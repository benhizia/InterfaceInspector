#!/usr/bin/env python3
"""
inspect_header.py
One-shot pipeline:  header → Doxygen XML + CastXML → merged metadata
Produces:
  <basename>.meta.json          (human readable)
  <basename>.meta.hpp           (C++ reflection helpers)
"""

import os
import shutil
import subprocess
import tempfile
import json
import platform
from pathlib import Path
from lxml import etree
from jinja2 import Template

# Import the comment processing module
from comment_processor import preprocess_comments

# ------------------------------------------------------------------ #
# 2. Doxygen
# ------------------------------------------------------------------ #
DOXYFILE_TMPL = """
OUTPUT_DIRECTORY = {out}
GENERATE_XML     = YES
XML_OUTPUT       = xml
INPUT            = {header}
QUIET            = YES
EXTRACT_ALL      = YES
EXTRACT_PRIVATE  = YES
MACRO_EXPANSION  = YES
GENERATE_LATEX   = NO
GENERATE_HTML    = NO
HAVE_DOT         = NO
"""

def run_doxygen(header: Path, out_dir: Path):
    # Create output directory if it doesn't exist
    out_dir.mkdir(parents=True, exist_ok=True)
    cfg = DOXYFILE_TMPL.format(header=header, out=out_dir)
    cfg_path = out_dir / "Doxyfile"
    cfg_path.write_text(cfg)
    subprocess.run(["doxygen", str(cfg_path)], check=True)
    return out_dir / "xml"

# ------------------------------------------------------------------ #
# 3. CastXML  (MSVC alignment hints)
# ------------------------------------------------------------------ #
def run_castxml(header: Path, out_xml: Path, include_paths: list[str]):
    # Platform-specific flags
    system = platform.system()
    if system == "Windows":
        # Use MSVC compatibility flags on Windows
        flags = [
            "-std=c++17",
            "-fms-compatibility",
            "-fms-extensions",
            "-Wno-static-float-init"
        ]
    else:
        # Use standard flags on macOS/Linux
        flags = [
            "-std=c++17",
            "-stdlib=libc++",
            "-Wno-static-float-init"
        ]
    
    cmd = [
        "castxml",
        "--castxml-output=1"
    ] + flags + [
        "-o", str(out_xml),
        str(header)
    ]
    for path in include_paths:
        cmd.insert(2, f"-I{path}")
    subprocess.run(cmd, check=True)
    return out_xml

# ------------------------------------------------------------------ #
# 4. XML Parsers
# ------------------------------------------------------------------ #
def parse_castxml(xml_path: Path, header_file: Path):
    """Parse CastXML output using reliable XML parsing."""
    tree = etree.parse(str(xml_path))
    classes = {}

    # Find the file ID for the header file
    header_file_id = None
    for file_node in tree.xpath(f"//File"):
        if header_file.name == Path(file_node.get("name")).name:
            header_file_id = file_node.get("id")
            break

    # Create a map of all fields by ID
    fields_by_id = {}
    for field in tree.xpath("//Field"):
        fields_by_id[field.get("id")] = field
    
    # Look for both Struct and Class elements
    for element_type in ["Struct", "Class"]:
        xpath_query = f"//{element_type}[@name]"
        if header_file_id:
            xpath_query = f"//{element_type}[@name and @file='{header_file_id}']"

        for rec in tree.xpath(xpath_query):
            name = rec.get("name")
            members = []
            
            # Get member IDs from members attribute
            members_attr = rec.get("members", "")
            member_ids = members_attr.split() if members_attr else []
            
            for member_id in member_ids:
                if member_id in fields_by_id:
                    f = fields_by_id[member_id]
                    offset_bits = int(f.get("offset", "0"))
                    members.append({
                        "name": f.get("name"),
                        "offset": offset_bits // 8,
                        "type": f.get("type"),
                        "size": int(f.get("size", "0")) // 8,
                        "align": int(f.get("align", "0")) // 8
                    })
            
            classes[name] = {
                "size": int(rec.get("size", "0")) // 8,
                "align": int(rec.get("align", "0")) // 8,
                "members": members
            }
    
    return classes

def parse_doxygen_comments(xml_dir: Path):
    comments = {}
    idx = xml_dir / "index.xml"
    if not idx.exists():
        return comments
    
    # Parse index to find all compounds (classes and structs)
    tree = etree.parse(str(idx))
    
    # Look for both classes and structs
    for kind in ['class', 'struct']:
        for c in tree.xpath(f"//compound[@kind='{kind}']"):
            ref = c.get("refid")
            
            # Try multiple XML file naming patterns
            xml_candidates = [
                xml_dir / f"{ref}.xml",
                xml_dir / f"{kind}_{c.get('name', '').lower().replace(' ', '_')}.xml" if c.get('name') else None
            ]
            
            cls_xml = None
            for xml_file in xml_candidates:
                if xml_file and xml_file.exists():
                    cls_xml = xml_file
                    break
            
            if not cls_xml:
                continue
            
            # Parse the compound XML file
            cls_tree = etree.parse(str(cls_xml))
            
            # Find all member variables
            for m in cls_tree.xpath(".//memberdef[@kind='variable']"):
                name = m.findtext("name")
                if not name:
                    continue
                
                # Get description from multiple sources
                doc_parts = []
                
                # Brief description
                brief = m.findtext("briefdescription/para") or ""
                if brief.strip():
                    doc_parts.append(brief.strip())
                
                # Detailed description
                detailed = m.findtext("detaileddescription/para") or ""
                if detailed.strip():
                    doc_parts.append(detailed.strip())
                
                # Get all text content from descriptions
                for desc_type in ["briefdescription", "detaileddescription"]:
                    desc_elem = m.find(desc_type)
                    if desc_elem is not None:
                        text_content = etree.tostring(desc_elem, encoding='unicode', method='text').strip()
                        if text_content and text_content not in doc_parts:
                            doc_parts.append(text_content)
                
                # Combine all documentation and deduplicate
                seen = set()
                unique_parts = []
                for part in doc_parts:
                    if part and part not in seen:
                        seen.add(part)
                        unique_parts.append(part)
                
                doc = " ".join(unique_parts).strip()
                if doc:
                    comments[name] = doc
    
    return comments

# ------------------------------------------------------------------ #
# 5. Merge + Generate artefacts
# ------------------------------------------------------------------ #
CPP_TMPL = """
#pragma once
#include <cstddef>
#include <type_traits>
#include <tuple>

struct MemberInfo {
    const char* name;
    size_t      offset;
    size_t      size;
    size_t      align;
};

{% for cls, data in classes.items() %}
constexpr std::array<MemberInfo, {{ data.members|length }}> {{ cls }}_meta = {
    {% for m in data.members %}
    { "{{ m.name }}", {{ m.offset }}, {{ m.size }}, {{ m.align }} }{% if not loop.last %},{% endif %}
    {% endfor %}
};
{% endfor %}
"""

def generate_files(header: Path, classes: dict, comments: dict, out_dir: Path):
    base = header.with_suffix("")
    out_dir.mkdir(parents=True, exist_ok=True)
    # 1. JSON
    enriched = {
        cls: {
            **meta,
            "members": [
                {**m, "doc": comments.get(m["name"], "")}
                for m in meta["members"]
            ]
        }
        for cls, meta in classes.items()
    }
    (out_dir / base.with_suffix(".meta.json").name).write_text(
        json.dumps(enriched, indent=2), encoding="utf-8"
    )

    # 2. C++ helpers
    cpp = Template(CPP_TMPL, trim_blocks=True, lstrip_blocks=True)
    (out_dir / base.with_suffix(".meta.hpp").name).write_text(
        cpp.render(classes=enriched), encoding="utf-8"
    )

# ------------------------------------------------------------------ #
# 6. Main driver
# ------------------------------------------------------------------ #
def main(header: str, include_paths: list[str]):
    header = Path(header).expanduser().resolve()
    out_dir = Path("MetaDataGen")
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        print("🔍 Pre-processing comments...")
        tmp_header = preprocess_comments(header)

        print("📗 Running Doxygen...")
        doxy_xml = run_doxygen(tmp_header, tmp / "doxy")

        print("⚙️ Running CastXML...")
        castxml_xml = tmp / "cast.xml"
        run_castxml(tmp_header, castxml_xml, include_paths)

        print("📊 Parsing & merging...")
        classes = parse_castxml(castxml_xml, tmp_header)
        comments = parse_doxygen_comments(doxy_xml)

        print("🧩 Generating artefacts...")
        generate_files(header, classes, comments, out_dir)
        print(f"✅ Done → {out_dir}/{header.with_suffix('').name}.meta.*")

if __name__ == "__main__":
    import sys
    if len(sys.argv) < 2:
        print("Usage: inspect_header.py <header.hpp> [-I<path>...]")
        sys.exit(1)
    
    header_file = sys.argv[1]
    include_paths = [arg[2:] for arg in sys.argv[2:] if arg.startswith("-I")]
    main(header_file, include_paths)

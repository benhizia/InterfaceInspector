import os
import shutil
import subprocess
import tempfile
import json
import platform
import sys
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
# 3. CastXML  (for names and structure)
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
def parse_castxml_for_names(xml_path: Path, header_file: Path):
    """Parse CastXML to get struct and member names."""
    tree = etree.parse(str(xml_path))
    classes = {}

    header_file_id = None
    for file_node in tree.xpath(f"//File"):
        if header_file.name == Path(file_node.get("name")).name:
            header_file_id = file_node.get("id")
            break

    fields_by_id = {}
    for field in tree.xpath("//Field"):
        fields_by_id[field.get("id")] = field

    for element_type in ["Struct", "Class"]:
        xpath_query = f"//{element_type}[@name and @file='{header_file_id}']"
        for rec in tree.xpath(xpath_query):
            name = rec.get("name")
            members = []
            members_attr = rec.get("members", "")
            member_ids = members_attr.split() if members_attr else []
            for member_id in member_ids:
                if member_id in fields_by_id:
                    f = fields_by_id[member_id]
                    members.append({"name": f.get("name")})
            classes[name] = {"members": members}
    return classes

def parse_doxygen_comments(xml_dir: Path):
    comments = {}
    idx = xml_dir / "index.xml"
    if not idx.exists():
        return comments
    
    tree = etree.parse(str(idx))
    for kind in ['class', 'struct']:
        for c in tree.xpath(f"//compound[@kind='{kind}']"):
            ref = c.get("refid")
            cls_xml = xml_dir / f"{ref}.xml"
            if not cls_xml.exists():
                continue
            
            cls_tree = etree.parse(str(cls_xml))
            for m in cls_tree.xpath(".//memberdef[@kind='variable']"):
                name = m.findtext("name")
                if not name:
                    continue
                
                doc_parts = []
                brief = m.findtext("briefdescription/para") or ""
                if brief.strip():
                    doc_parts.append(brief.strip())
                
                detailed = m.findtext("detaileddescription/para") or ""
                if detailed.strip():
                    doc_parts.append(detailed.strip())
                
                doc = " ".join(doc_parts).strip()
                if doc:
                    comments[name] = doc
    return comments

# ------------------------------------------------------------------ #
# 5. C++ Introspection
# ------------------------------------------------------------------ #
CPP_BUILDER_TMPL = """
#include <iostream>
#include <vector>
#include <string>
#include <cstddef>
#include <nlohmann/json.hpp>
#include "{header_path}"

using json = nlohmann::json;

std::string get_compiler_info() {{
#if defined(__clang__)
    return "Clang " + std::to_string(__clang_major__) + "." + std::to_string(__clang_minor__) + "." + std::to_string(__clang_patchlevel__);
#elif defined(__GNUC__)
    return "GCC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__) + "." + std::to_string(__GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
    return "MSVC " + std::to_string(_MSC_VER);
#else
    return "Unknown";
#endif
}}

int main() {{
    json output;
    output["system"] = {{
        {{ "compiler", get_compiler_info() }}
    }};
    {introspection_code}
    std::cout << output.dump(2);
    return 0;
}}
"""

INTROSPECTION_CODE_TMPL = """
    output["{class_name}"] = {{
        {{ "size", sizeof({class_name}) }},
        {{ "align", alignof({class_name}) }},
        {{ "members", json::array() }}
    }};
    {member_code}
"""

MEMBER_CODE_TMPL = """
    output["{class_name}"]["members"].push_back({{
        {{ "name", "{member_name}" }},
        {{ "offset", offsetof({class_name}, {member_name}) }},
        {{ "size", sizeof(decltype(std::declval<{class_name}>().{member_name})) }},
        {{ "align", alignof(decltype(std::declval<{class_name}>().{member_name})) }}
    }});
"""

def run_cpp_introspection(header: Path, classes: dict, tmp_dir: Path, include_paths: list[str]):
    introspection_code = ""
    for class_name, data in classes.items():
        member_code = ""
        for member in data["members"]:
            member_code += MEMBER_CODE_TMPL.format(class_name=class_name, member_name=member["name"])
        introspection_code += INTROSPECTION_CODE_TMPL.format(class_name=class_name, member_code=member_code)

    builder_cpp = tmp_dir / "meta_builder.cpp"
    builder_cpp.write_text(CPP_BUILDER_TMPL.format(
        header_path=header.resolve(),
        introspection_code=introspection_code
    ))

    builder_exe = tmp_dir / "meta_builder"
    
    sdk_root = None
    final_include_paths = []
    if platform.system() == "Darwin":
        for path in include_paths:
            if "SDKs/MacOSX.sdk/usr/include" in path:
                if not sdk_root:
                    sdk_root = path.removesuffix("/usr/include")
            elif "llvm/include/c++/v1" in path:
                # This can conflict with the system's C++ library, so we skip it
                # when using -isysroot.
                pass
            else:
                final_include_paths.append(path)
    else:
        final_include_paths = include_paths

    compile_cmd = ["clang++", "-std=c++17"]
    if sdk_root:
        compile_cmd.extend(["-isysroot", sdk_root])

    compile_cmd.extend([str(builder_cpp), "-o", str(builder_exe)])

    for path in final_include_paths:
        compile_cmd.append(f"-I{path}")
    
    compile_cmd.append(f"-I{Path(__file__).parent / 'vendor'}")

    subprocess.run(compile_cmd, check=True)
    
    result = subprocess.run([str(builder_exe)], capture_output=True, text=True, check=True)
    return json.loads(result.stdout)

# ------------------------------------------------------------------ #
# 6. Merge + Generate artefacts
# ------------------------------------------------------------------ #
def generate_files(header: Path, classes: dict, comments: dict, out_dir: Path):
    base = header.with_suffix("")
    out_dir.mkdir(parents=True, exist_ok=True)

    # 1. JSON
    system_info = classes.pop("system", {})
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
    enriched["system"] = {
        "architecture": platform.machine(),
        "endianness": sys.byteorder,
        "compiler": system_info.get("compiler", "Unknown")
    }

    (out_dir / base.with_suffix(".meta.json").name).write_text(
        json.dumps(enriched, indent=2), encoding="utf-8"
    )

    # 2. C++ helpers
    cpp_content = "#pragma once\n#include <cstddef>\n#include <type_traits>\n#include <tuple>\n\n"
    cpp_content += "struct MemberInfo {\n    const char* name;\n    size_t      offset;\n    size_t      size;\n    size_t      align;\n};\n\n"

    for cls, data in enriched.items():
        if cls == "system":
            continue
        cpp_content += f"constexpr std::array<MemberInfo, {len(data['members'])}> {cls}_meta = {{\n"
        for i, m in enumerate(data["members"]):
            cpp_content += f'    {{ "{m["name"]}", {m["offset"]}, {m["size"]}, {m["align"]} }}'
            if i < len(data["members"]) - 1:
                cpp_content += ","
            cpp_content += "\n"
        cpp_content += "}}};\n\n"

    (out_dir / base.with_suffix(".meta.hpp").name).write_text(cpp_content, encoding="utf-8")


# ------------------------------------------------------------------ #
# 7. Main driver
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

        print("📊 Parsing for names...")
        classes_with_names = parse_castxml_for_names(castxml_xml, tmp_header)
        comments = parse_doxygen_comments(doxy_xml)

        print("🤖 Generating and running C++ introspection tool...")
        layout_data = run_cpp_introspection(header, classes_with_names, tmp, include_paths)

        # Merge layout data with names and comments
        final_classes = {}
        system_info = layout_data.pop("system", {})
        for class_name, data in layout_data.items():
            final_classes[class_name] = {
                "size": data["size"],
                "align": data["align"],
                "members": data["members"]
            }

        final_classes["system"] = system_info

        print("🧩 Generating artefacts...")
        generate_files(header, final_classes, comments, out_dir)
        print(f"✅ Done → {out_dir}/{header.with_suffix('').name}.meta.*")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: inspect_header.py <header.hpp> [-I<path>...]")
        sys.exit(1)
    
    header_file = sys.argv[1]
    include_paths = [arg[2:] for arg in sys.argv[2:] if arg.startswith("-I")]
    main(header_file, include_paths)
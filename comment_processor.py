#!/usr/bin/env python3
"""
comment_processor.py
Module for handling all comment-related regex operations
"""

import re
import tempfile
from pathlib import Path
from typing import List, Tuple


class CommentProcessor:
    """Handles all comment processing and transformation operations"""
    
    def __init__(self):
        # String protection pattern
        self.string_pattern = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])\'')
        
        # Struct/class boundary patterns
        self.struct_start_pattern = re.compile(r'(?:struct|class)\s+\w+\s*\{')
        self.struct_end_pattern = re.compile(r'\}')
    
    def protect_strings(self, text: str) -> Tuple[str, List[str]]:
        """Replace strings with placeholders and return protected text and original strings"""
        strings = []
        def replace_with_placeholder(match):
            strings.append(match.group(0))
            return f"__STRING_PLACEHOLDER_{len(strings)-1}__"
        
        protected = self.string_pattern.sub(replace_with_placeholder, text)
        return protected, strings
    
    def restore_strings(self, text: str, strings: List[str]) -> str:
        """Restore original strings from placeholders"""
        for i, original in enumerate(strings):
            text = text.replace(f"__STRING_PLACEHOLDER_{i}__", original)
        return text
    
    def process_struct_body(self, lines: List[str], start_idx: int, end_idx: int) -> List[str]:
        """Process a struct/class body for member comments with concatenation support"""
        processed = []
        i = start_idx
        
        # Collect comments that appear before members
        pending_comments = []
        
        while i <= end_idx:
            line = lines[i].rstrip()
            
            # Skip empty lines
            if not line.strip():
                processed.append(line)
                i += 1
                continue
            
            # Check for standalone comments (before members)
            standalone_comment = re.match(r'^(\s*)(//(?!/)(.*))$', line)
            if standalone_comment:
                indent, full_comment, comment_text = standalone_comment.groups()
                pending_comments.append(('//', indent, comment_text.strip()))
                i += 1
                continue
            
            standalone_block = re.match(r'^(\s*)(/\*(.*?)\*/)$', line)
            if standalone_block:
                indent, full_comment, comment_text = standalone_block.groups()
                pending_comments.append(('/*', indent, comment_text.strip()))
                i += 1
                continue
            
            # Check for standalone multi-line comment start
            standalone_multiline = re.match(r'^(\s*)(/\*(.*))$', line)
            if standalone_multiline and not re.search(r'\*/', line):
                indent, full_comment, comment_start = standalone_multiline.groups()
                
                # Collect multi-line standalone comment
                comment_parts = [comment_start.strip()]
                j = i + 1
                
                while j <= end_idx:
                    next_line = lines[j]
                    if '*/' in next_line:
                        end_pos = next_line.find('*/')
                        comment_part = next_line[:end_pos].strip()
                        if comment_part:
                            comment_parts.append(comment_part)
                        pending_comments.append(('/*', indent, ' '.join(comment_parts).replace('*/', '').strip()))
                        i = j + 1  # Skip the closing line
                        break
                    else:
                        comment_parts.append(next_line.strip())
                    j += 1
                else:
                    # No closing, treat as regular line
                    # Regular line - flush pending comments if any
                    if pending_comments:
                        for comment_type, indent, text in pending_comments:
                            if comment_type == '//':
                                processed.append(indent + '/// ' + text)
                            else:
                                processed.append(indent + '/** ' + text + ' */')
                        pending_comments.clear()
                    processed.append(line)
                    i += 1
                continue
            
            # Check for member declarations with comments
            # First, check for inline comments on the same line
            inline_comment = re.search(r'^(\s*)(\w+(?:\s+\w+)*\s+\w+(?:\s*=\s*[^;]+)?\s*;)\s*//(?!/)(.*)$', line)
            if inline_comment:
                indent, member_decl, inline_text = inline_comment.groups()
                
                # Combine pending comments with inline
                combined_comment = self._combine_comments(pending_comments, inline_text.strip())
                if combined_comment:
                    processed.append(indent + combined_comment)
                pending_comments.clear()
                processed.append(indent + member_decl)
                i += 1
                continue
            
            inline_block = re.search(r'^(\s*)(\w+(?:\s+\w+)*\s+\w+(?:\s*=\s*[^;]+)?\s*;)\s*/\*(.*?)\*/', line)
            if inline_block:
                indent, member_decl, inline_text = inline_block.groups()
                
                # Combine pending comments with inline block
                combined_comment = self._combine_comments(pending_comments, inline_text.strip())
                if combined_comment:
                    processed.append(indent + combined_comment)
                pending_comments.clear()
                processed.append(indent + member_decl)
                i += 1
                continue
            
            # Check for multi-line inline comment
            inline_multiline = re.search(r'^(\s*)(\w+(?:\s+\w+)*\s+\w+(?:\s*=\s*[^;]+)?\s*;)\s*/\*(.*)$', line)
            if inline_multiline and not re.search(r'\*/', line):
                indent, member_decl, comment_start = inline_multiline.groups()
                
                # Collect multi-line inline comment
                comment_parts = [comment_start.strip()]
                j = i + 1
                
                while j <= end_idx:
                    next_line = lines[j]
                    if '*/' in next_line:
                        end_pos = next_line.find('*/')
                        comment_part = next_line[:end_pos].strip()
                        if comment_part:
                            comment_parts.append(comment_part)
                        
                        # Combine pending comments with multi-line inline
                        full_comment = ' '.join(comment_parts).replace('*/', '').strip()
                        combined_comment = self._combine_comments(pending_comments, full_comment)
                        if combined_comment:
                            processed.append(indent + combined_comment)
                        pending_comments.clear()
                        processed.append(indent + member_decl)
                        
                        # Check for remaining content after the comment
                        remaining = next_line[end_pos+2:].strip()
                        if remaining and not remaining.startswith('*/'):
                            # This might be another member, handle it properly
                            processed.append(remaining)
                        
                        i = j + 1  # Skip the closing line
                        break
                    else:
                        comment_parts.append(next_line.strip())
                    j += 1
                else:
                    # No closing, treat as regular line
                    # Regular line - flush pending comments if any
                    if pending_comments:
                        for comment_type, indent, text in pending_comments:
                            if comment_type == '//':
                                processed.append(indent + '/// ' + text)
                            else:
                                processed.append(indent + '/** ' + text + ' */')
                        pending_comments.clear()
                    processed.append(line)
                    i += 1
                continue
            
            # Check for member declaration without inline comment
            member_decl_match = re.match(r'^(\s*)(\w+(?:\s+\w+)*\s+\w+(?:\s*=\s*[^;]+)?\s*;)$', line)
            if member_decl_match:
                indent, member_decl = member_decl_match.groups()
                
                # Use pending comments for this member
                combined_comment = self._combine_comments(pending_comments)
                if combined_comment:
                    processed.append(indent + combined_comment)
                pending_comments.clear()
                processed.append(indent + member_decl)
                i += 1
                continue
            
            # Regular line - flush pending comments if any
            if pending_comments:
                for comment_type, indent, text in pending_comments:
                    if comment_type == '//':
                        processed.append(indent + '/// ' + text)
                    else:
                        processed.append(indent + '/** ' + text + ' */')
                pending_comments.clear()
            
            processed.append(line)
            i += 1
        
        # Flush any remaining pending comments
        for comment_type, indent, text in pending_comments:
            if comment_type == '//':
                processed.append(indent + '/// ' + text)
            else:
                processed.append(indent + '/** ' + text + ' */')
        
        return processed
    
    def _combine_comments(self, pending_comments: List[Tuple[str, str, str]], inline_text: str = None) -> str:
        """Combine before-line comments with end-of-line comments"""
        if not pending_comments and not inline_text:
            return None
        
        all_parts = []
        
        # Add pending comments
        for comment_type, _, text in pending_comments:
            all_parts.append(text)
        
        # Add inline comment
        if inline_text:
            all_parts.append(inline_text)
        
        if not all_parts:
            return None
        
        # Combine all parts
        combined = ' '.join(all_parts).strip()
        
        # Return appropriate Doxygen format
        if len(all_parts) == 1 and not inline_text and pending_comments and pending_comments[0][0] == '/*':
            return '/** ' + combined + ' */'
        else:
            return '/// ' + combined
    
    def find_struct_boundaries(self, lines: List[str], start_idx: int) -> Tuple[int, int]:
        """Find the start and end indices of a struct/class"""
        brace_count = 0
        start_line = lines[start_idx]
        
        # Find opening brace
        brace_pos = start_line.find('{')
        if brace_pos == -1:
            # Look for brace on next lines
            for j in range(start_idx + 1, len(lines)):
                if '{' in lines[j]:
                    brace_pos = lines[j].find('{')
                    start_idx = j
                    break
        
        # Count braces to find closing
        for j in range(start_idx, len(lines)):
            line = lines[j]
            brace_count += line.count('{')
            brace_count -= line.count('}')
            if brace_count <= 0:
                return start_idx, j
        
        return start_idx, len(lines) - 1
    
    def process_comments(self, code: str) -> str:
        """Main entry point for comment processing"""
        lines = code.splitlines()
        processed_lines = []
        i = 0
        
        # Protect strings
        protected_code, strings = self.protect_strings(code)
        protected_lines = protected_code.splitlines()
        
        def find_struct_start(lines, start_idx):
            for j in range(start_idx, len(lines)):
                if self.struct_start_pattern.search(lines[j]):
                    return j
            return -1
        
        i = 0
        while i < len(protected_lines):
            line = protected_lines[i]
            
            struct_start = find_struct_start(protected_lines, i)
            if struct_start != -1 and struct_start >= i:
                # Found struct start
                processed_lines.extend(protected_lines[i:struct_start])
                
                # Find the opening brace
                brace_line = protected_lines[struct_start]
                brace_pos = brace_line.find('{')
                if brace_pos == -1:
                    # Look on next line
                    for j in range(struct_start + 1, len(protected_lines)):
                        if '{' in protected_lines[j]:
                            brace_pos = protected_lines[j].find('{')
                            struct_start = j
                            break
                
                # Find struct boundaries
                start_line = struct_start
                end_line = start_line
                
                # Count braces
                brace_count = 0
                for j in range(start_line, len(protected_lines)):
                    brace_count += protected_lines[j].count('{')
                    brace_count -= protected_lines[j].count('}')
                    if brace_count <= 0:
                        end_line = j
                        break
                
                # Process the struct
                if start_line < len(protected_lines):
                    # Add struct declaration line
                    struct_decl = protected_lines[start_line]
                    processed_lines.append(struct_decl)
                    
                    # Process body
                    if start_line + 1 < end_line:
                        body_lines = self.process_struct_body(
                            protected_lines, start_line + 1, end_line - 1
                        )
                        processed_lines.extend(body_lines)
                    
                    # Add closing brace
                    if end_line < len(protected_lines):
                        processed_lines.append(protected_lines[end_line])
                
                i = end_line + 1
            else:
                processed_lines.append(line)
                i += 1
        
        result = '\n'.join(processed_lines)
        return self.restore_strings(result, strings)


def preprocess_comments(src: Path) -> Path:
    """Entry point for comment preprocessing"""
    processor = CommentProcessor()
    code = src.read_text(encoding="utf-8")
    processed_code = processor.process_comments(code)
    
    tmp = Path(tempfile.mktemp(suffix=".hpp"))
    tmp.write_text(processed_code, encoding="utf-8")
    return tmp
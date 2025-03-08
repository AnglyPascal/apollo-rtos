#!/usr/bin/env python3

import os
import sys
import re

def parse_su_files(build_directory):
    """
    Parse all .su files in the given build directory.
    
    :param build_directory: Path to the CMake build directory
    :return: List of stack usage entries
    """
    stack_entries = []
    
    # Find all .su files in the build directory and its subdirectories
    for root, _, files in os.walk(build_directory):
        for file in files:
            if file.endswith('.su'):
                filepath = os.path.join(root, file)
                with open(filepath, 'r') as f:
                    for line in f:
                        # Parse each line 
                        # Format: path/to/file:line:column:function_name    stack_usage    static/dynamic
                        match = re.match(r'^(.*):(\d+):(\d+):(.+)\s+(\d+)\s+(static|dynamic)', line.strip())
                        if match:
                            filepath, line_num, col_num, func_name, stack_usage, kind = match.groups()
                            stack_entries.append({
                                'filepath': filepath,
                                'line_num': int(line_num),
                                'col_num': int(col_num),
                                'func_name': func_name.strip(),
                                'stack_usage': int(stack_usage),
                                'kind': kind
                            })
    
    return stack_entries

def format_output(stack_entries):
    """
    Format stack usage entries for readability.
    
    :param stack_entries: List of stack usage dictionaries
    :return: Formatted output string
    """
    # Sort entries by stack usage in descending order
    sorted_entries = sorted(stack_entries, key=lambda x: x['stack_usage'], reverse=True)
    
    # Format output
    output_lines = [
        "Stack Usage Analysis",
        "=" * 40,
        f"{'Function':60} {'Stack Usage':>12} {'Location':>30}",
        "-" * 102
    ]
    
    for entry in sorted_entries:
        if entry['func_name'] == "cpp)": # or entry['func_name'] == ")":
            continue

        # Truncate filepath to show only last two components if too long
        filepath_parts = entry['filepath'].split(os.path.sep)
        if len(filepath_parts) > 2:
            filepath_display = os.path.join(*filepath_parts[-2:])
        else:
            filepath_display = entry['filepath']
        
        output_lines.append(
            f"{entry['func_name'][:60]:60} "
            f"{entry['stack_usage']:>12} "
            f"{filepath_display:>30}"
        )
    
    return "\n".join(output_lines)

def main():
    # Check if build directory is provided as argument
    if len(sys.argv) < 2:
        print("Usage: stack_usage_analysis.py <build_directory>")
        sys.exit(1)
    
    build_directory = sys.argv[1]
    
    # Parse stack usage files
    stack_entries = parse_su_files(build_directory)
    
    # Format and generate output
    output = format_output(stack_entries)
    
    # Write to output file in the build directory
    output_filepath = os.path.join(build_directory, 'stack_usage_analysis.txt')
    with open(output_filepath, 'w') as f:
        f.write(output)
    
    print(f"Stack usage analysis written to {output_filepath}")

if __name__ == "__main__":
    main()

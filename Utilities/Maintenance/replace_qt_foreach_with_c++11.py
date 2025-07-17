import re
import sys
import os

def convert_foreach_to_range_for(line):
    # Match: foreach (Type var, container)
    pattern = r"\b(?:foreach|Q_FOREACH)\s*\(\s*([\w]+)\s*([\w]+),\s*(.+?)\s*\)"
    match = re.search(pattern, line)
    if match:
        type_name = match.group(1).strip()
        var = match.group(2).strip()
        container = match.group(3).strip()
        line=re.sub(pattern, f"for (const {type_name} & {var}: {container})", line)
    # Match: foreach (const Type &var, container)
    pattern = r"\b(?:foreach|Q_FOREACH)\s*\(\s*(const)+\s*([\w]+\s*&)\s*([\w]+)\s*,\s*(.+?)\s*\)"
    match = re.search(pattern, line)
    if match:
        const = match.group(1).strip()
        type_name = match.group(2).strip()
        var = match.group(3).strip()
        container = match.group(4).strip()
        line=re.sub(pattern, f"for ({const} {type_name} {var}: {container})", line)
    # Match: foreach (Type* var, container)
    pattern = r"\b(?:foreach|Q_FOREACH)\s*\(\s*([\w]+\s*\*)\s*([\w]+)\s*,\s*(.+?)\s*\)"
    match = re.search(pattern, line)
    if match:
        type_name = match.group(1).strip()
        var = match.group(2).strip()
        container = match.group(3).strip()
        line=re.sub(pattern, f"for ({type_name} const {var}: {container})", line)

    return line

def process_file(filename):
    with open(filename) as f:
        lines = f.readlines()

    modified = False
    new_lines = []

    for line in lines:
        new_line = convert_foreach_to_range_for(line)
        if new_line != line:
            modified = True
        new_lines.append(new_line)

    if modified:
        with open(filename, "w") as f:
            f.writelines(new_lines)
        print(f"Updated: {filename}")
    else:
        print(f"No changes: {filename}")

def process_directory(root_dir):
    for dirpath, _, filenames in os.walk(root_dir):
        for file in filenames:
            if file.endswith((".cpp", ".h", ".cxx", ".hpp", ".txx")):
                process_file(os.path.join(dirpath, file))

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python convert_foreach.py <file-or-directory>")
        sys.exit(1)

    target = sys.argv[1]
    if os.path.isdir(target):
        process_directory(target)
    elif os.path.isfile(target):
        process_file(target)
    else:
        print(f"Invalid path: {target}")

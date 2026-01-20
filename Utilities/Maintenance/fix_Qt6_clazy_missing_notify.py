#!/usr/bin/env python3
import re
import sys

prop_pattern = re.compile(
    r"^(?P<indent>\s*)Q_PROPERTY\([\s\n]*"
    r"(?P<type>[\w:<>]+)[\s\n]+"
    r"(?P<prop>[A-Za-z_]\w*)[\s\n]+"
    r"READ[\s\n]+(?P<readfunction>[A-Za-z_]\w*)[\s\n]+"
    r"WRITE[\s\n]+(?P<writefunction>\w+)[\s\n]*\)[\s\n]*;*[\s\n]*$",
)

signal_pattern = re.compile(
    r"[\s\n]*void[\s\n]+"
    r"(?P<prop>[A-Za-z_]\w*)Changed[\s\n]*\([\s\n]*[\w:<>]+[\s\n]*\)[\s\n]*;",
)

def process_lines(lines):
    out = []
    existing_signals = set()
    # First pass: collect existing signal names
    for line in lines:
        m = signal_pattern.match(line)
        if m:
            existing_signals.add(m.group("prop"))

    # Second pass: transform properties and insert signal declaration
    q_signals_block = ["Q_SIGNALS:\n"]
    for line in lines:
        pm = prop_pattern.match(line)
        if pm:
            indent = pm.group("indent")
            type_ = pm.group("type")
            prop = pm.group("prop")
            readfunction = pm.group("readfunction")
            writefunction = pm.group("writefunction")
            #notify = f' NOTIFY {prop}Changed'
            #new_prop_line = re.sub(r'\)[\s\n]*;', notify + ');', line)
            new_prop_line = f"{indent}Q_PROPERTY({type_} {prop} READ {readfunction} WRITE {writefunction} NOTIFY {prop}Changed);\n"
            out.append(new_prop_line)
            if prop not in existing_signals:
                # insert signal declaration
                q_signals_block.append(f"{indent}    void {prop}Changed({type_});\n")
                existing_signals.add(prop)
        else:
           out.append(line)
    if len(q_signals_block) > 1:
        for signal_function_prototypes in q_signals_block:
            out.append(signal_function_prototypes)
    return out

def convert_file(src_path, dst_path=None):
    with open(src_path) as f:
        lines = f.readlines()
    transformed = process_lines(lines)
    out = open(dst_path, "w") if dst_path else sys.stdout
    out.writelines(transformed)
    if dst_path:
        out.close()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <input-file> [<output-file>]")
        sys.exit(1)
    convert_file(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else None)

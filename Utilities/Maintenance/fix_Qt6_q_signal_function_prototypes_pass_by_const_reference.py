#!/usr/bin/env python3
import re
import sys

PRIMITIVE = {
    "bool","char","signed char","unsigned char",
    "short","unsigned short","int","unsigned int",
    "long","unsigned long","long long","unsigned long long",
    "float","double","long double",
    "char16_t","char32_t","wchar_t",
}

signal_proto = re.compile(
    r"^(?P<indent>\s*)void\s+(?P<function_name>[\w:<>]+)\s*\(\s*(?P<variable_type>[\w:<>]+)\s*(?P<variable_name>[\w:<>]*)\)\s*;\s*$",
)

def is_primitive(typ: str) -> bool:
    t = typ.strip()
    # ignore parameter names
    t = re.sub(r"\b\w+\s*$", "", t).strip()
    return t in PRIMITIVE

def process(lines):
    out = []
    in_signals = False
    for line in lines:
        if re.match(r"^\s*Q_SIGNALS\s*:", line): # Q_SIGNALS indicates the start of a block
            in_signals = True
            out.append(line)
            continue
        if re.match(r"^\s*.*:\s*$", line):
            in_signals = False
            out.append(line)
            continue
        if in_signals:
            pm = signal_proto.match(line)
            if not pm:
                out.append(line)
                continue
            indent = pm.group("indent")
            function_name = pm.group("function_name")
            variable_type = pm.group("variable_type").strip()
            variable_name = pm.group("variable_name").strip()
            if len(variable_name) > 1:
              variable_name = f" {variable_name}" # insert space before name
            if variable_type.strip() in PRIMITIVE:
              out.append(line)
              continue
            if "*" in variable_type: # Skip pointers
              out.append(line)
              continue
            if "&" in variable_type: # Skip references
              out.append(line)
              continue
            if pm:
              new = f"{indent}void {function_name}(const {variable_type} &{variable_name});\n"
              out.append(new)
              continue
        out.append(line)
    return out

def convert(src, dst=None):
    with open(src) as f:
        lines = f.readlines()
    transformed = process(lines)
    out = open(dst, "w") if dst else sys.stdout
    out.writelines(transformed)
    if dst:
        out.close()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <header.h> [<out.h>]")
        sys.exit(1)
    convert(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else None)

#!/usr/bin/env python3
import sys
import yaml
from pathlib import Path
import os

def yaml_to_gcc(path):
    with open(path) as f:
        doc = yaml.safe_load(f)

    diagnostics = doc.get("Diagnostics", [])
    for d in diagnostics:
        level = d.get("Level", "Warning").lower()
        msg = d["DiagnosticMessage"]["Message"].strip().strip("'")
        file_name = d["DiagnosticMessage"]["FilePath"]
        # Optional: FileOffset if exists
        offset = d["DiagnosticMessage"].get("FileOffset", 0)
        file_path: Path = Path(file_name)
        if file_path.exists():
            with open(file_path) as fid:
                lines=fid.readlines()
        bytes_consumed=0
        line_number=0
        column_number=0
        for line in lines:
            line_length=len(line)
            if line_length < (offset - bytes_consumed):
                bytes_consumed += line_length
                line_number += 1
            else:
                column_number=(offset - bytes_consumed)
                line_number +=1
                break

        # filename:line:column: error: message
        print(f"{file_path}:{line_number}:{column_number}: {level}: {msg}")

def main():
    if len(sys.argv) != 2:
        print(f"Usage: {os.path.basename(sys.argv[0])} <diagnostics.yaml>")
        sys.exit(1)
    yaml_to_gcc(sys.argv[1])

if __name__ == "__main__":
    main()

#!/usr/bin/env python

import argparse
import base64
import subprocess
import sys

_header = """
import base64

from __main__ import slicer

qt_resource_handle = None
qt_resource_data = base64.decodestring(\"\"\"
"""

_footer = """
\"\"\")

def qInitResources():
    global qt_resource_handle
    qt_resource_handle = slicer.app.registerResource(qt_resource_data)

def qCleanupResources():
    slicer.app.unregisterResource(qt_resource_handle)

qInitResources()
"""

def compileResources(in_path, out_file, args):
  command = [args.rcc, "-binary", in_path]
  proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=sys.stderr)
  data, err = proc.communicate()

  if proc.returncode != 0:
    sys.exit(proc.returncode)

  out_file.write(_header)
  out_file.write(base64.encodestring(data).rstrip())
  out_file.write(_footer)

def main(argv):
  parser = argparse.ArgumentParser(description="PythonQt Resource Compiler")

  parser.add_argument("--rcc", default="rcc",
                      help="location of the Qt resource compiler executable")
  parser.add_argument("-o", "--output", dest="out_path", metavar="PATH",
                      default="-",
                      help="location to which to write the output Python"
                           " script (default=stdout)")
  parser.add_argument("in_path", metavar="resource_script",
                      help="input resource script to compile")

  args = parser.parse_args(argv)

  if args.out_path == "-":
    compileResources(args.in_path, sys.stdout, args)
  else:
    with open(args.out_path, "w") as f:
      compileResources(args.in_path, f, args)

if __name__ == "__main__":
  main(sys.argv[1:])

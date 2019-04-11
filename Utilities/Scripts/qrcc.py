#!/usr/bin/env python

import argparse
import base64
import os
import subprocess
import sys

_header = """
import base64

import slicer

qt_resource_handle = None
qt_resource_data = base64.decodebytes(b\"\"\"
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
  # Determine command line for rcc
  if sys.platform.startswith("win") or sys.platform.startswith("cygwin"):
    # On Windows, rcc performs LF -> CRLF conversion when writing to stdout,
    # resulting in corrupt data that can cause Qt to crash when loading the
    # resources. To work around this, we must instead write to a temporary
    # file.
    if args.out_path == "-":
      import tempfile
      tmp_file, tmp_path = tempfile.mkstemp()
      os.close(tmp_file)

    else:
      tmp_path = args.out_path + ".rcctmp"

    command = [args.rcc, "-binary", "-o", tmp_path, in_path]

  else:
    tmp_path = None
    command = [args.rcc, "-binary", in_path]

  # Run rcc
  proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=sys.stderr)
  data, err = proc.communicate()

  # Check that rcc ran successfully
  if proc.returncode != 0:
    sys.exit(proc.returncode)

  # Read data, if using a temporary file (see above)
  if tmp_path is not None:
    with open(tmp_path,"rb") as tmp_file:
      data = tmp_file.read()

    os.remove(tmp_path)

  _data = base64.encodestring(data).rstrip().decode()

  # Write output script
  out_file.write(_header)
  out_file.write(_data)
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

"""Generate a lazily-loading drop-in replacement for the ``vtk`` module.

The ``vtk.py`` shim distributed with VTK eagerly star-imports more than a
hundred binary modules, which costs close to a second at every application
startup even though a typical session touches only a fraction of them.

This script, run at build time, imports every VTK module once, records which
module provides each public attribute, and writes a replacement ``vtk``
module that resolves attributes on demand (PEP 562 module __getattr__):
``vtk.vtkPolyData`` imports only ``vtkmodules.vtkCommonDataModel``.
The generated file is placed on the Python path ahead of the eager shim.

The module is generated from a template (``Base/Python/vtk.py.in``)
by replacing the ``@VTK_ATTRIBUTE_MODULE@`` placeholder with the attribute
index, in the manner of CMake's configure_file. The template documents the
behavior of the generated module.

Usage: GenerateLazyVtkModule.py --template /path/to/vtk.py.in --output /path/to/vtk.py
"""

import argparse
import importlib
import os
import re
import sys


def module_order():
    """Return the vtkmodules submodule names in the order used by
    vtkmodules.all, so that attribute collisions resolve to the same module
    as with eager star-imports (the last import wins).
    """
    import vtkmodules.all
    source = open(vtkmodules.all.__file__).read()
    return re.findall(r"^from \.(\w+) import \*", source, re.MULTILINE)


def build_index(names):
    index = {}
    failed = []
    for name in names:
        try:
            module = importlib.import_module("vtkmodules." + name)
        except ImportError:
            failed.append(name)
            continue
        for attribute in dir(module):
            if not attribute.startswith("_"):
                index[attribute] = name
    return index, failed


def format_index(index):
    """Format the attribute index as a Python dict literal."""
    entries = ",\n    ".join(f'"{attribute}": "{index[attribute]}"'
                             for attribute in sorted(index))
    return "{\n    %s,\n}" % entries


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--template", required=True)
    parser.add_argument("--output", required=True)
    arguments = parser.parse_args(argv)

    placeholder = "@VTK_ATTRIBUTE_MODULE@"
    with open(arguments.template) as stream:
        template = stream.read()
    if placeholder not in template:
        print(f"error: {arguments.template} does not contain {placeholder}", file=sys.stderr)
        return 1

    names = module_order()
    if not names:
        print("error: could not determine vtkmodules import order", file=sys.stderr)
        return 1
    index, failed = build_index(names)
    if failed:
        print("note: skipped unimportable modules: %s" % ", ".join(failed))

    # On a fresh build this rule may run before anything else has created
    # the destination directory.
    os.makedirs(os.path.dirname(os.path.abspath(arguments.output)), exist_ok=True)
    with open(arguments.output, "w") as stream:
        stream.write(template.replace(placeholder, format_index(index)))
    print("generated %s: %d attributes from %d modules"
          % (arguments.output, len(index), len(names)))
    return 0


if __name__ == "__main__":
    status = main(sys.argv[1:])
    sys.stdout.flush()
    sys.stderr.flush()
    # Exit without interpreter teardown: unloading a hundred freshly built
    # VTK modules can crash in static destructors in the bare build
    # environment, and the generated file is already written and closed.
    os._exit(status)

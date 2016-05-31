""" This module sets up root logging and loads the Slicer library modules into its namespace."""

#-----------------------------------------------------------------------------
def _createModule(name, globals, docstring):
  import imp
  import sys
  moduleName = name.split('.')[-1]
  module = imp.new_module( moduleName )
  module.__file__ = __file__
  module.__doc__ = docstring
  sys.modules[name] = module
  globals[moduleName] = module

#-----------------------------------------------------------------------------
# Create slicer.modules and slicer.moduleNames

_createModule('slicer.modules', globals(),
"""This module provides an access to all instantiated Slicer modules.

The module attributes are the lower-cased Slicer module names, the
associated value is an instance of ``qSlicerAbstractCoreModule``.
""")

_createModule('slicer.moduleNames', globals(),
"""This module provides an access to all instantiated Slicer module names.

The module attributes are the Slicer modules names, the associated
value is the module name.
""")

#-----------------------------------------------------------------------------
# Load modules: Add VTK and PythonQt python module attributes into slicer namespace

try:
  from kits import available_kits
except ImportError:
  available_kits = []

for kit in available_kits:
   try:
     exec "from %s import *" % (kit)
   except ImportError as detail:
     print detail

#-----------------------------------------------------------------------------
# Cleanup: Removing things the user shouldn't have to see.

del _createModule
del available_kits
del kit

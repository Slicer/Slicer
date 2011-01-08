""" This module loads the Slicer Module Logic vtk classes into its namespace."""

import glob, os

# Retrieve list of modules
for fname in glob.glob(__path__[0] + "/vtkSlicer*ModuleLogic.py"):

  # Extract module logic name
  module_logic_name = os.path.splitext(os.path.basename(fname))[0]
  
  # Import logic library
  try:
    exec "import %s" % (module_logic_name)
  except ImportError as detail:
    # TODO: this message should go in the application error log (how?)
    print detail
    continue
  
  # ... and obtain a reference to the associated module
  module_logic = eval(module_logic_name)
  
  # Loop over content of the python module associated with the 'Module logic library'
  for item_name in dir(module_logic):
  
    # Obtain a reference associated with the current object
    item = eval("%s.%s" % (module_logic_name, item_name))
    
    # Import the logic vtk class if any
    if type(item).__name__ == 'vtkclass':
      exec("from %s import %s" % (module_logic_name, item_name))
    
    # Removing things the user shouldn't have to see.  
    del item, item_name
  
  # Removing things the user shouldn't have to see.
  exec("del %s" % (module_logic_name))
  del module_logic, module_logic_name

# Removing things the user shouldn't have to see.
del fname, glob, os

""" This module loads the Slicer Module MRML vtk classes into its namespace."""

import glob, os

# Retrieve list of modules
for fname in glob.glob(__path__[0] + "/vtkSlicer*ModuleMRML.py"):

  # Extract module mrml name
  module_mrml_name = os.path.splitext(os.path.basename(fname))[0]
  
  # Import mrml library
  try:
    exec "import %s" % (module_mrml_name)
  except ImportError as detail:
    # TODO: this message should go in the application error log (how?)
    print detail
    continue
  
  # ... and obtain a reference to the associated module
  module_mrml = eval(module_mrml_name)
  
  # Loop over content of the python module associated with the 'Module mrml library'
  for item_name in dir(module_mrml):
  
    # Obtain a reference associated with the current object
    item = eval("%s.%s" % (module_mrml_name, item_name))
    
    # Import the mrml vtk class if any
    if type(item).__name__ == 'vtkclass':
      exec("from %s import %s" % (module_mrml_name, item_name))
    
    # Removing things the user shouldn't have to see.  
    del item, item_name
  
  # Removing things the user shouldn't have to see.
  exec("del %s" % (module_mrml_name))
  del module_mrml, module_mrml_name

# Removing things the user shouldn't have to see.
del fname, glob, os


#
# General
#

EXIT_SUCCESS = 0
EXIT_FAILURE = 1

def quit():
  exit(EXIT_SUCCESS)

def exit(status=EXIT_SUCCESS):
  from slicer import app
  app.exit(status)

def restart():
  from slicer import app
  app.restart()

#
# Custom Import
#

def importVTKClassesFromDirectory(directory, dest_module_name, filematch = '*'):
  import glob, os
  for fname in glob.glob(directory + '/' + filematch):
    try:
      vtk_module_name = os.path.splitext(os.path.basename(fname))[0]
      importVTKClasses(vtk_module_name, dest_module_name)
    except ImportError as detail:
      # TODO: this message should go in the application error log (how?)
      print detail

def importVTKClasses(vtk_module_name, dest_module_name):
  """Import VTK classes from module identified by vtk_module_name into
  the module identified by 'dest_module_name'."""

  # Obtain a reference to the module idenitifed by 'dest_module_name'
  import sys
  dest_module = sys.modules[dest_module_name]

  exec "import %s" % (vtk_module_name)

  # Obtain a reference to the associated VTK module
  module = eval(vtk_module_name)

  # Loop over content of the python module associated with the given VTK python library
  for item_name in dir(module):

    # Obtain a reference associated with the current object
    item = eval("%s.%s" % (vtk_module_name, item_name))

    # Add the vtk class to dest_module_globals_dict if any
    if type(item).__name__ == 'vtkclass':
      exec("from %s import %s" % (vtk_module_name, item_name))
      exec("dest_module.%s = %s"%(item_name, item_name))

#
# UI
#

def lookupTopLevelWidget(objectName, verbose = True):
  """Loop over all top level widget associated with 'slicer.app' and 
  return the one matching 'objectName'"""
  from slicer import app
  for w in app.topLevelWidgets():
    if w.objectName == objectName: return w
  if verbose:
    print "Failed to obtain reference to '%s'" % objectName
  return None

def mainWindow(verbose = True):
  return lookupTopLevelWidget('qSlicerMainWindow', verbose)

def pythonShell(verbose = True):
  return lookupTopLevelWidget('pythonConsole', verbose)

def showStatusMessage(message, duration = 0):
  mw = mainWindow(verbose=False)
  if mw:
    mw.statusBar().showMessage(message, duration)

def findChildren(widget=None,name=""):
  """ return a list of child widgets that match the passed name """
  # TODO: figure out why the native QWidget.findChildren method
  # does not seem to work from PythonQt
  if not widget:
    widget = mainWindow()
  children = []
  parents = [widget]
  while parents != []:
    p = parents.pop()
    parents += p.children()
    if p.name == name:
      children.append(p)
  return children

#
# IO
#

def loadColorTable(filename):
  from slicer import app, qSlicerIO
  filetype = qSlicerIO.ColorTableFile
  properties = {}
  return app.coreIOManager().loadNodes(filetype, properties)

def loadFiberBundle(filename):
  from slicer import app, qSlicerIO
  filetype = qSlicerIO.FiberBundleFile
  properties = {}
  return app.coreIOManager().loadNodes(filetype, properties)

def loadFiducialList(filename):
  from slicer import app, qSlicerIO
  filetype = qSlicerIO.FiducialListFile
  properties = {'fileName':filename}
  return app.coreIOManager().loadNodes(filetype, properties)

def loadModel(filename):
  from slicer import app, qSlicerIO
  filetype = qSlicerIO.ModelFile
  properties = {'fileName':filename}
  return app.coreIOManager().loadNodes(filetype, properties)

def loadScalarOverlay(filename):
  from slicer import app, qSlicerIO
  filetype = qSlicerIO.ScalarOverlayFile
  properties = {'fileName':filename}
  return app.coreIOManager().loadNodes(filetype, properties)

def loadScene(filename, clear = True):
  from slicer import app, qSlicerIO
  filetype = qSlicerIO.SceneFile
  properties = {'fileName':filename}
  return app.coreIOManager().loadNodes(filetype, properties)

def loadTransform(filename):
  from slicer import app, qSlicerIO
  filetype = qSlicerIO.TransformFile
  properties = {'fileName':filename}
  return app.coreIOManager().loadNodes(filetype, properties)

def loadVolume(filename):
  from slicer import app, qSlicerIO
  filetype = qSlicerIO.VolumeFile
  properties = {'fileName':filename}
  return app.coreIOManager().loadNodes(filetype, properties)
def openData():
  from slicer import app, qSlicerIO
  return app.coreIOManager().openAddDataDialog()

def openVolume():
  from slicer import app, qSlicerIO
  return app.coreIOManager().openAddVolumeDialog()

def openModel():
  from slicer import app, qSlicerIO
  return app.coreIOManager().openAddModelDialog()

def openScalarOverlay():
  from slicer import app, qSlicerIO
  return app.coreIOManager().openAddScalarOverlayDialog()

def openTransform():
  from slicer import app, qSlicerIO
  return app.coreIOManager().openAddTransformDialog()

def openColorTable():
  from slicer import app, qSlicerIO
  return app.coreIOManager().openAddColorTableDialog()

def openFiducial():
  from slicer import app, qSlicerIO
  return app.coreIOManager().openAddFiducialDialog()

def openFiberBundleFile():
  from slicer import app, qSlicerIO
  return app.coreIOManager().openAddFiberBundleDialog()

def openSaveData():
  from slicer import app, qSlicerIO
  return app.coreIOManager().openSaveDataDialog()
  
#
# Module
#

def selectModule(module):
  moduleName = module
  if not isinstance(module, basestring):
    moduleName = module.name
  w = mainWindow()
  if not w: return
  w.moduleSelector().selectModule(moduleName)

def moduleNames():
  from slicer import app
  return app.moduleManager().factoryManager().moduleNames()

def getModule(moduleName):
  from slicer import app
  module = app.moduleManager().module(moduleName);
  if not module:
    print "Could not find module with name '%s" % moduleName
    return None
  return module

def getModuleGui(module):
  if isinstance(module, basestring):
    module = getModule(module)
  if not module:
    return None
  widgetRepr = module.widgetRepresentation()
  if not widgetRepr:
    print "Could not find module widget representation with name '%s" % moduleName
    return None
  return widgetRepr

#
# MRML
#

def getNodes(pattern = ""):
    """Return a dictionary of nodes where the name or id matches the 'pattern'.
    Providing an empty 'pattern' string will return all nodes.
    """
    import slicer, fnmatch
    nodes = {}
    scene = slicer.mrmlScene
    count = scene.GetNumberOfNodes()
    for idx in range(count):
      node = scene.GetNthNode(idx)
      name = node.GetName()
      id = node.GetID()
      if fnmatch.fnmatch(name, pattern) or fnmatch.fnmatch(id, pattern):
        nodes[node.GetName()] = node
    return nodes

def getNode(pattern = "", index = 0):
    """Return the indexth node where name or id matches 'pattern'.
    Providing an empty 'pattern' string will return all nodes.
    """
    nodes = getNodes(pattern)
    try:
      if nodes.keys():
        return nodes.values()[index]
    except IndexError:
      return None

#
# MRML-numpy
#

def array(pattern = "", index = 0):
  """Return the array you are "most likely to want" from the indexth
  MRML node that matches the pattern.  Meant to be used in the python
  console for quick debugging/testing.  More specific API should be
  used in scripts to be sure you get exactly what you want.
  """
  import vtk.util.numpy_support
  n = getNode(pattern=pattern, index=index)
  if n.GetClassName() == 'vtkMRMLScalarVolumeNode':
    i = n.GetImageData()
    a = vtk.util.numpy_support.vtk_to_numpy(i.GetPointData().GetScalars()).reshape(n.GetImageData().GetDimensions())
    return a
  # TODO: accessors for other node types: tensors, polydata (verts, polys...), colors



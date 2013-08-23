
from __future__ import print_function

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
  importClassesFromDirectory(directory, dest_module_name, 'vtkclass', filematch)

def importQtClassesFromDirectory(directory, dest_module_name, filematch = '*'):
  importClassesFromDirectory(directory, dest_module_name, 'PythonQtClassWrapper', filematch)

def importClassesFromDirectory(directory, dest_module_name, type_name, filematch = '*'):
  import glob, os, re, fnmatch
  for fname in glob.glob(os.path.join(directory, filematch)):
    if not re.compile(fnmatch.translate(filematch)).match(os.path.basename(fname)):
      continue
    try:
      from_module_name = os.path.splitext(os.path.basename(fname))[0]
      importModuleObjects(from_module_name, dest_module_name, type_name)
    except ImportError as detail:
      import sys
      print(detail, file=sys.stderr)

def importModuleObjects(from_module_name, dest_module_name, type_name):
  """Import object of type 'type_name' from module identified
  by 'from_module_name' into the module identified by 'dest_module_name'."""

  # Obtain a reference to the module identifed by 'dest_module_name'
  import sys
  dest_module = sys.modules[dest_module_name]

  exec "import %s" % (from_module_name)

  # Obtain a reference to the associated VTK module
  module = eval(from_module_name)

  # Loop over content of the python module associated with the given VTK python library
  for item_name in dir(module):

    # Obtain a reference associated with the current object
    item = eval("%s.%s" % (from_module_name, item_name))

    # Add the object to dest_module_globals_dict if any
    if type(item).__name__ == type_name:
      exec("from %s import %s" % (from_module_name, item_name))
      exec("dest_module.%s = %s"%(item_name, item_name))

#
# UI
#

def lookupTopLevelWidget(objectName, verbose = True):
  """Loop over all top level widget associated with 'slicer.app' and
  return the one matching 'objectName'"""
  from slicer import app
  for w in app.topLevelWidgets():
    if hasattr(w,'objectName'):
      if w.objectName == objectName: return w
  if verbose:
    import sys
    print("Failed to obtain reference to '%s'" % objectName, file=sys.stderr)
  return None

def mainWindow(verbose = True):
  return lookupTopLevelWidget('qSlicerAppMainWindow', verbose)

def pythonShell(verbose = True):
  return lookupTopLevelWidget('pythonConsole', verbose)

def showStatusMessage(message, duration = 0):
  mw = mainWindow(verbose=False)
  if mw:
    mw.statusBar().showMessage(message, duration)

def findChildren(widget=None,name="",text="",title="",className=""):
  """ return a list of child widgets that match the passed name """
  # TODO: figure out why the native QWidget.findChildren method
  # does not seem to work from PythonQt
  import slicer, fnmatch
  if not widget:
    widget = mainWindow()
  if not widget:
    return []
  children = []
  parents = [widget]
  while parents != []:
    p = parents.pop()
    # sometimes, p is null, f.e. when using --python-script or --python-code
    if not p:
      break
    if not hasattr(p,'children'):
      continue
    parents += p.children()
    if name and fnmatch.fnmatch(p.name, name):
      children.append(p)
    elif text:
      try:
        p.text
        if fnmatch.fnmatch(p.text, text):
          children.append(p)
      except (AttributeError, TypeError):
        pass
    elif title:
      try:
        p.title
        if fnmatch.fnmatch(p.title, title):
          children.append(p)
      except AttributeError:
        pass
    elif className:
      try:
        p.className()
        if fnmatch.fnmatch(p.className(), className):
          children.append(p)
      except AttributeError:
        pass
  return children

#
# IO
#

def loadNodeFromFile(filename, filetype, properties={}, returnNode=False):
  from slicer import app
  from vtk import vtkCollection
  properties['fileName'] = filename

  if returnNode:
      loadedNodes = vtkCollection()
      success = app.coreIOManager().loadNodes(filetype, properties, loadedNodes)
      return success, loadedNodes.GetItemAsObject(0)
  else:
      success = app.coreIOManager().loadNodes(filetype, properties)
      return success

def loadColorTable(filename, returnNode=False):
  filetype = 'ColorTableFile'
  return loadNodeFromFile(filename, filetype, {}, returnNode)

def loadFiberBundle(filename, returnNode=False):
  filetype = 'FiberBundleFile'
  return loadNodeFromFile(filename, filetype, {}, returnNode)

def loadFiducialList(filename, returnNode=False):
  filetype = 'FiducialListFile'
  return loadNodeFromFile(filename, filetype, {}, returnNode)

def loadAnnotationFiducial(filename, returnNode=False):
  filetype = 'AnnotationFile'
  properties = {}
  properties['fiducial'] = 1
  return loadNodeFromFile(filename, filetype, properties, returnNode)

def loadMarkupsFiducialList(filename, returnNode=False):
  filetype = 'MarkupsFiducials'
  properties = {}
  return loadNodeFromFile(filename, filetype, properties, returnNode)

def loadModel(filename, returnNode=False):
  filetype = 'ModelFile'
  return loadNodeFromFile(filename, filetype, {}, returnNode)

def loadScalarOverlay(filename, returnNode=False):
  filetype = 'ScalarOverlayFile'
  return loadNodeFromFile(filename, filetype, {}, returnNode)

def loadTransform(filename, returnNode=False):
  filetype = 'TransformFile'
  return loadNodeFromFile(filename, filetype, {}, returnNode)

def loadLabelVolume(filename, properties, returnNode=False):
  filetype = 'VolumeFile'
  properties['labelmap'] = True
  return loadNodeFromFile(filename, filetype, properties, returnNode)

def loadVolume(filename, properties={}, returnNode=False):
  filetype = 'VolumeFile'
  return loadNodeFromFile(filename, filetype, properties, returnNode)

def loadScene(filename, properties={}):
  filetype = 'SceneFile'
  return loadNodeFromFile(filename, filetype, properties, returnNode=False)

def openAddDataDialog():
  from slicer import app
  return app.coreIOManager().openAddDataDialog()

def openAddVolumeDialog():
  from slicer import app
  return app.coreIOManager().openAddVolumeDialog()

def openAddModelDialog():
  from slicer import app
  return app.coreIOManager().openAddModelDialog()

def openAddScalarOverlayDialog():
  from slicer import app
  return app.coreIOManager().openAddScalarOverlayDialog()

def openAddTransformDialog():
  from slicer import app
  return app.coreIOManager().openAddTransformDialog()

def openAddColorTableDialog():
  from slicer import app
  return app.coreIOManager().openAddColorTableDialog()

def openAddFiducialDialog():
  from slicer import app
  return app.coreIOManager().openAddFiducialDialog()

def openAddFiberBundleDialog():
  from slicer import app
  return app.coreIOManager().openAddFiberBundleDialog()

def openSaveDataDialog():
  from slicer import app
  return app.coreIOManager().openSaveDataDialog()

def saveNode(node, filename, properties={}):
  """Save 'node' data into 'filename'.

  It is the user responsability to provide the appropriate file extension.

  User has also the possibility to overwrite the fileType internally retrieved using
  method 'qSlicerCoreIOManager::fileWriterFileType(vtkObject*)'. This can be done
  by specifiying a 'fileType'attribute to the optional 'properties' dictionary.
  """
  from slicer import app
  properties["nodeID"] = node.GetID();
  properties["fileName"] = filename
  if hasattr(properties, "fileType"):
    filetype = properties["fileType"]
  else:
    filetype = app.coreIOManager().fileWriterFileType(node)
  return app.coreIOManager().saveNodes(filetype, properties)

def saveScene(filename, properties={}):
  """Save the current scene.

  Based on the value of 'filename', the current scene is saved either
  as a MRML file, MRB file or directory.

  If filename ends with '.mrml', the scene is saved as a single file
  without associated data.

  If filename ends with '.mrb', the scene is saved as a MRML bundle (Zip
  archive with scene and data files).

  In every other case, the scene is saved in the directory
  specified by 'filename'. Both MRML scene file and data
  will be written to disk. If needed, directories and sub-directories
  will be created.
  """
  from slicer import app
  filetype = 'SceneFile'
  properties['fileName'] = filename
  return app.coreIOManager().saveNodes(filetype, properties)

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
  return app.moduleManager().factoryManager().loadedModuleNames()

def getModule(moduleName):
  from slicer import app
  module = app.moduleManager().module(moduleName);
  if not module:
    import sys
    print("Could not find module with name '%s" % moduleName, file=sys.stderr)
    return None
  return module

def getModuleGui(module):
  if isinstance(module, basestring):
    module = getModule(module)
  widgetRepr = module.widgetRepresentation()
  if not widgetRepr:
    import sys
    print("Could not find module widget representation with name '%s" % module.name, file=sys.stderr)
  return widgetRepr

def getNewModuleGui(module):
  if isinstance(module, basestring):
    module = getModule(module)
  widgetRepr = module.createNewWidgetRepresentation()
  if not widgetRepr:
    import sys
    print("Could not find module widget representation with name '%s" % module.name, file=sys.stderr)
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
  vectorTypes = ('vtkMRMLVectorVolumeNode', 'vtkMRMLMultiVolumeNode')
  tensorTypes = ('vtkMRMLDiffusionTensorVolumeNode',)
  import vtk.util.numpy_support
  n = getNode(pattern=pattern, index=index)
  if n.GetClassName() == 'vtkMRMLScalarVolumeNode':
    i = n.GetImageData()
    shape = list(n.GetImageData().GetDimensions())
    shape.reverse()
    a = vtk.util.numpy_support.vtk_to_numpy(i.GetPointData().GetScalars()).reshape(shape)
    return a
  elif n.GetClassName() in vectorTypes:
    i = n.GetImageData()
    shape = list(n.GetImageData().GetDimensions())
    shape.reverse()
    components = i.GetNumberOfScalarComponents()
    if components > 1:
      shape.append(components)
    a = vtk.util.numpy_support.vtk_to_numpy(i.GetPointData().GetScalars()).reshape(shape)
    return a
  elif n.GetClassName() in tensorTypes:
    i = n.GetImageData()
    shape = list(n.GetImageData().GetDimensions())
    shape.reverse()
    a = vtk.util.numpy_support.vtk_to_numpy(i.GetPointData().GetTensors()).reshape(shape+[3,3])
    return a
  # TODO: accessors for other node types: polydata (verts, polys...), colors


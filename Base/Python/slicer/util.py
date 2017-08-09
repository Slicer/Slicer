
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
  app.commandOptions().runPythonAndExit = False
  app.exit(status)

def restart():
  from slicer import app
  app.restart()

def _readCMakeCache(var):
  import os
  from slicer import app

  prefix = var + ":"

  try:
    with open(os.path.join(app.slicerHome, "CMakeCache.txt")) as cache:
      for line in cache:
        if line.startswith(prefix):
          return line.split("=", 1)[1].rstrip()

  except:
    pass

  return None

def sourceDir():
  """Location of the Slicer source directory.

  :type: :class:`str` or ``None``

  This provides the location of the Slicer source directory, if Slicer is being
  run from a CMake build directory. If the Slicer home directory does not
  contain a ``CMakeCache.txt`` (e.g. for an installed Slicer), the property
  will have the value ``None``.
  """
  return _readCMakeCache('Slicer_SOURCE_DIR')

#
# Custom Import
#

def importVTKClassesFromDirectory(directory, dest_module_name, filematch = '*'):
  from vtk import vtkObjectBase
  importClassesFromDirectory(directory, dest_module_name, vtkObjectBase, filematch)

def importQtClassesFromDirectory(directory, dest_module_name, filematch = '*'):
  importClassesFromDirectory(directory, dest_module_name, 'PythonQtClassWrapper', filematch)

# To avoid globbing multiple times the same directory, successful
# call to ``importClassesFromDirectory()`` will be indicated by
# adding an entry to the ``__import_classes_cache`` set.
#
# Each entry is a tuple of form (directory, dest_module_name, type_info, filematch)
__import_classes_cache = set()

def importClassesFromDirectory(directory, dest_module_name, type_info, filematch = '*'):
  # Create entry for __import_classes_cache
  cache_key = ",".join([str(arg) for arg in [directory, dest_module_name, type_info, filematch]])
  # Check if function has already been called with this set of parameters
  if cache_key in __import_classes_cache:
    return

  import glob, os, re, fnmatch
  re_filematch = re.compile(fnmatch.translate(filematch))
  for fname in glob.glob(os.path.join(directory, filematch)):
    if not re_filematch.match(os.path.basename(fname)):
      continue
    try:
      from_module_name = os.path.splitext(os.path.basename(fname))[0]
      importModuleObjects(from_module_name, dest_module_name, type_info)
    except ImportError as detail:
      import sys
      print(detail, file=sys.stderr)

  __import_classes_cache.add(cache_key)

def importModuleObjects(from_module_name, dest_module_name, type_info):
  """Import object of type 'type_info' (str or type) from module identified
  by 'from_module_name' into the module identified by 'dest_module_name'."""

  # Obtain a reference to the module identifed by 'dest_module_name'
  import sys
  dest_module = sys.modules[dest_module_name]

  # Skip if module has already been loaded
  if from_module_name in sys.modules:
    return

  # Obtain a reference to the module identified by 'from_module_name'
  import imp
  fp, pathname, description = imp.find_module(from_module_name)
  module = imp.load_module(from_module_name, fp, pathname, description)

  # Loop over content of the python module associated with the given python library
  for item_name in dir(module):

    # Obtain a reference associated with the current object
    item = getattr(module, item_name)

    # Check type match by type or type name
    match = False
    if isinstance(type_info, type):
      try:
        match = issubclass(item, type_info)
      except TypeError as e:
        pass
    else:
      match = type(item).__name__ == type_info

    if match:
      setattr(dest_module, item_name, item)

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
  console = slicer.app.pythonConsole()
  if not console and verbose:
    print("Failed to obtain reference to python shell", file=sys.stderr)
  return console

def showStatusMessage(message, duration = 0):
  mw = mainWindow(verbose=False)
  if not mw or not mw.statusBar:
    return False
  mw.statusBar().showMessage(message, duration)
  return True

def findChildren(widget=None, name="", text="", title="", className=""):
  """ Return a list of child widgets that meet all the given criteria.
  If no criteria are provided, the function will return all widgets descendants.
  If no widget is provided, slicer.util.mainWindow() is used.
  :param widget: parent widget where the widgets will be searched
  :param name: name attribute of the widget
  :param text: text attribute of the widget
  :param title: title attribute of the widget
  :param className: className() attribute of the widget
  :return: list with all the widgets that meet all the given criteria.
  """
  # TODO: figure out why the native QWidget.findChildren method does not seem to work from PythonQt
  import slicer, fnmatch
  if not widget:
    widget = mainWindow()
  if not widget:
    return []
  children = []
  parents = [widget]
  kwargs = {'name': name, 'text': text, 'title': title, 'className': className}
  expected_matches = []
  for kwarg in kwargs.iterkeys():
    if kwargs[kwarg]:
      expected_matches.append(kwarg)
  while parents:
    p = parents.pop()
    # sometimes, p is null, f.e. when using --python-script or --python-code
    if not p:
      break
    if not hasattr(p,'children'):
      continue
    parents += p.children()
    matched_filter_criteria = 0
    for attribute in expected_matches:
      if hasattr(p, attribute):
        attr_name = getattr(p, attribute)
        if attribute == 'className':
          # className is a method, not a direct attribute. Invoke the method
          attr_name = attr_name()
        # Objects may have text attributes with non-string value (for example,
        # QUndoStack objects have text attribute of 'builtin_qt_slot' type.
        # We only consider string type attributes.
        if isinstance(attr_name, basestring):
          if fnmatch.fnmatchcase(attr_name, kwargs[attribute]):
            matched_filter_criteria = matched_filter_criteria + 1
    if matched_filter_criteria == len(expected_matches):
      children.append(p)
  return children

def findChild(widget, name):
  """
  Convenience method to access a widget by its ``name``.
  A ``RuntimeError`` exception is raised if the widget with the
  given ``name`` does not exist.
  """
  errorMessage = "Widget named " + str(name) + " does not exists."
  child = None
  try:
    child = findChildren(widget, name=name)[0]
    if not child:
      raise RuntimeError(errorMessage)
  except IndexError:
    raise RuntimeError(errorMessage)
  return child

def loadUI(path):
  """ Load UI file ``path`` and return the corresponding widget.
  Raises a ``RuntimeError`` exception if the UI file is not found or if no
  widget was instantiated.
  """
  import qt
  qfile = qt.QFile(path)
  if not qfile.exists():
    errorMessage = "Could not load UI file: file not found " + str(path) + "\n\n"
    raise RuntimeError(errorMessage)
  qfile.open(qt.QFile.ReadOnly)
  loader = qt.QUiLoader()
  widget = loader.load(qfile)
  if not widget:
    errorMessage = "Could not load UI file: " + str(path) + "\n\n"
    raise RuntimeError(errorMessage)
  return widget

def setSliceViewerLayers(background='keep-current', foreground='keep-current', label='keep-current',
                         foregroundOpacity=None, labelOpacity=None):
  """ Set the slice views with the given nodes.
  If node ID is not specified (or value is 'keep-current') then the layer will not be modified.
  :param background: node or node ID to be used for the background layer
  :param foreground: node or node ID to be used for the foreground layer
  :param label: node or node ID to be used for the label layer
  :param foregroundOpacity: opacity of the foreground layer
  :param labelOpacity: opacity of the label layer
  """
  import slicer
  def _nodeID(nodeOrID):
    nodeID = nodeOrID
    if isinstance(nodeOrID, slicer.vtkMRMLNode):
      nodeID = nodeOrID.GetID()
    return nodeID

  num = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLSliceCompositeNode')
  for i in range(num):
      sliceViewer = slicer.mrmlScene.GetNthNodeByClass(i, 'vtkMRMLSliceCompositeNode')
      if background is not 'keep-current':
          sliceViewer.SetBackgroundVolumeID(_nodeID(background))
      if foreground is not 'keep-current':
          sliceViewer.SetForegroundVolumeID(_nodeID(foreground))
      if foregroundOpacity is not None:
          sliceViewer.SetForegroundOpacity(foregroundOpacity)
      if label is not 'keep-current':
          sliceViewer.SetLabelVolumeID(_nodeID(label))
      if labelOpacity is not None:
          sliceViewer.SetLabelOpacity(labelOpacity)

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

def loadAnnotationRuler(filename, returnNode=False):
  filetype = 'AnnotationFile'
  properties = {}
  properties['ruler'] = 1
  return loadNodeFromFile(filename, filetype, properties, returnNode)

def loadAnnotationROI(filename, returnNode=False):
  filetype = 'AnnotationFile'
  properties = {}
  properties['roi'] = 1
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

def loadSegmentation(filename, returnNode=False):
  filetype = 'SegmentationFile'
  return loadNodeFromFile(filename, filetype, {}, returnNode)

def loadTransform(filename, returnNode=False):
  filetype = 'TransformFile'
  return loadNodeFromFile(filename, filetype, {}, returnNode)

def loadLabelVolume(filename, properties={}, returnNode=False):
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

def openAddSegmentationDialog():
  from slicer import app, qSlicerFileDialog
  return app.coreIOManager().openDialog('SegmentationFile', qSlicerFileDialog.Read)

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

def moduleSelector():
  w = mainWindow()
  if not w:
    import sys
    print("Could not find main window", file=sys.stderr)
    return None
  return w.moduleSelector()

def selectModule(module):
  moduleName = module
  if not isinstance(module, basestring):
    moduleName = module.name
  selector = moduleSelector()
  if not selector:
    import sys
    print("Could not find moduleSelector in the main window", file=sys.stderr)
    return None
  moduleSelector().selectModule(moduleName)

def selectedModule():
  selector = moduleSelector()
  if not selector:
    import sys
    print("Could not find moduleSelector in the main window", file=sys.stderr)
    return None
  return selector.selectedModule

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

def modulePath(moduleName):
  import slicer
  return eval('slicer.modules.%s.path' % moduleName.lower())

def reloadScriptedModule(moduleName):
  """Generic reload method for any scripted module.
  """
  import imp, sys, os
  import slicer

  widgetName = moduleName + "Widget"

  # reload the source code
  filePath = modulePath(moduleName)
  p = os.path.dirname(filePath)
  if not p in sys.path:
    sys.path.insert(0,p)
  with open(filePath, "r") as fp:
    reloaded_module = imp.load_module(
        moduleName, fp, filePath, ('.py', 'r', imp.PY_SOURCE))

  # find and hide the existing widget
  parent = eval('slicer.modules.%s.widgetRepresentation()' % moduleName.lower())
  for child in parent.children():
    try:
      child.hide()
    except AttributeError:
      pass

  # remove spacer items
  item = parent.layout().itemAt(0)
  while item:
    parent.layout().removeItem(item)
    item = parent.layout().itemAt(0)

  # delete the old widget instance
  if hasattr(slicer.modules, widgetName):
    w = getattr(slicer.modules, widgetName)
    if hasattr(w, 'cleanup'):
      w.cleanup()

  # create new widget inside existing parent
  widget = eval('reloaded_module.%s(parent)' % widgetName)
  widget.setup()
  setattr(slicer.modules, widgetName, widget)

  return reloaded_module

#
# Layout
#

def resetThreeDViews():
  """Reset focal view around volumes
  """
  import slicer
  slicer.app.layoutManager().resetThreeDViews()

def resetSliceViews():
  """Reset focal view around volumes
  """
  import slicer
  manager = slicer.app.layoutManager().resetSliceViews()

#
# MRML
#

def getNodes(pattern="*", scene=None, useLists=False):
  """Return a dictionary of nodes where the name or id matches the ``pattern``.
  By default, ``pattern`` is a wildcard and it returns all nodes associated
  with ``slicer.mrmlScene``.
  If multiple node share the same name, using ``useLists=False`` (default behavior)
  returns only the last node with that name. If ``useLists=True``, it returns
  a dictionary of lists of nodes.
  """
  import slicer, collections, fnmatch
  nodes = collections.OrderedDict()
  if scene is None:
    scene = slicer.mrmlScene
  count = scene.GetNumberOfNodes()
  for idx in range(count):
    node = scene.GetNthNode(idx)
    name = node.GetName()
    id = node.GetID()
    if (fnmatch.fnmatchcase(name, pattern) or
        fnmatch.fnmatchcase(id, pattern)):
      if useLists:
        nodes.setdefault(node.GetName(), []).append(node)
      else:
        nodes[node.GetName()] = node
  return nodes

def getNode(pattern="*", index=0, scene=None):
  """Return the indexth node where name or id matches ``pattern``.
  By default, ``pattern`` is a wildcard and it returns the first node
  associated with ``slicer.mrmlScene``.
  """
  nodes = getNodes(pattern, scene)
  try:
    return nodes.values()[index]
  except IndexError:
    return None

def getNodesByClass(className, scene=None):
  """Return all nodes in the scene of the specified class.
  """
  import slicer
  if scene is None:
    scene = slicer.mrmlScene
  nodes = slicer.mrmlScene.GetNodesByClass(className)
  nodes.UnRegister(slicer.mrmlScene)
  nodeList = []
  nodes.InitTraversal()
  node = nodes.GetNextItemAsObject()
  while node:
    nodeList.append(node)
    node = nodes.GetNextItemAsObject()
  return nodeList

def getFirstNodeByClassByName(className, name, scene=None):
  """Return the frist node in the scene that matches the specified node name and node class.
  """
  import slicer
  if scene is None:
    scene = slicer.mrmlScene
  nodes = scene.GetNodesByClassByName(className, name)
  nodes.UnRegister(nodes)
  if nodes.GetNumberOfItems() > 0:
    return nodes.GetItemAsObject(0)
  return None

def getFirstNodeByName(name, className=None):
  """get the first MRML node that has the given name
  - use a regular expression to match names post-pended with addition characters
  - optionally specify a classname that must match
  """
  import slicer
  scene = slicer.mrmlScene
  return scene.GetFirstNode(name, className, None, False)

class NodeModify:
  """Context manager to conveniently compress mrml node modified event.
  """
  def __init__(self, node):
    self.node = node
  def __enter__(self):
    self.wasModifying = self.node.StartModify()
    return self.node
  def __exit__(self, type, value, traceback):
    self.node.EndModify(self.wasModifying)

#
# MRML-numpy
#

def array(pattern = "", index = 0):
  """Return the array you are "most likely to want" from the indexth
  MRML node that matches the pattern.  Meant to be used in the python
  console for quick debugging/testing.  More specific API should be
  used in scripts to be sure you get exactly what you want.
  """
  node = getNode(pattern=pattern, index=index)
  import slicer
  if isinstance(node, slicer.vtkMRMLVolumeNode):
    return arrayFromVolume(node)
  elif isinstance(node, slicer.vtkMRMLModelNode):
    return arrayFromModelPoints(node)
  elif isinstance(node, slicer.vtkMRMLGridTransformNode):
    return arrayFromGridTransform(node)

  # TODO: accessors for other node types: polydata (verts, polys...), colors
  return None

def arrayFromVolume(volumeNode):
  """Return voxel array from volume node as numpy array.
  Voxels values are not copied. Voxel values in the volume node can be modified
  by changing values in the numpy array.
  After all modifications has been completed, call volumeNode.Modified().
  """
  scalarTypes = ['vtkMRMLScalarVolumeNode', 'vtkMRMLLabelMapVolumeNode']
  vectorTypes = ['vtkMRMLVectorVolumeNode', 'vtkMRMLMultiVolumeNode']
  tensorTypes = ['vtkMRMLDiffusionTensorVolumeNode']
  vimage = volumeNode.GetImageData()
  nshape = tuple(reversed(volumeNode.GetImageData().GetDimensions()))
  import vtk.util.numpy_support
  narray = None
  if volumeNode.GetClassName() in scalarTypes:
    narray = vtk.util.numpy_support.vtk_to_numpy(vimage.GetPointData().GetScalars()).reshape(nshape)
  elif volumeNode.GetClassName() in vectorTypes:
    components = vimage.GetNumberOfScalarComponents()
    if components > 1:
      nshape = nshape + (components,)
    narray = vtk.util.numpy_support.vtk_to_numpy(vimage.GetPointData().GetScalars()).reshape(nshape)
  elif volumeNode.GetClassName() in tensorTypes:
    narray = vtk.util.numpy_support.vtk_to_numpy(vimage.GetPointData().GetTensors()).reshape(nshape+(3,3))
  else:
    raise RuntimeError("Unsupported volume type: "+volumeNode.GetClassName())
  return narray

def arrayFromModelPoints(modelNode):
  """Return point positions of a model node as numpy array.
  Voxels values in the volume node can be modified by modfying the numpy array.
  After all modifications has been completed, call modelNode.Modified().
  """
  import vtk.util.numpy_support
  pointData = modelNode.GetPolyData().GetPoints().GetData()
  narray = vtk.util.numpy_support.vtk_to_numpy(pointData)
  return narray

def arrayFromGridTransform(gridTransformNode):
  """Return voxel array from transform node as numpy array.
  Vector values are not copied. Values in the transform node can be modified
  by changing values in the numpy array.
  After all modifications has been completed, call gridTransformNode.Modified().
  """
  transformGrid = gridTransformNode.GetTransformFromParent()
  displacementGrid = transformGrid.GetDisplacementGrid()
  nshape = tuple(reversed(displacementGrid.GetDimensions()))
  import vtk.util.numpy_support
  nshape = nshape + (3,)
  narray = vtk.util.numpy_support.vtk_to_numpy(displacementGrid.GetPointData().GetScalars()).reshape(nshape)
  return narray

def updateVolumeFromArray(volumeNode, narray):
  """Sets voxels of a volume node from a numpy array.
  Voxels values are copied, therefore if the numpy array
  is modified after calling this method, voxel values in the volume node will not change.
  """

  vshape = tuple(reversed(narray.shape))
  if len(vshape) == 3:
    # Scalar volume
    vcomponents = 1
  elif len(vshape) == 4:
    # Vector volume
    vcomponents = vshape[0]
    vshape = vshape[1:4]
  else:
    # TODO: add support for tensor volumes
    raise RuntimeError("Unsupported numpy array shape: "+str(narray.shape))

  vimage = volumeNode.GetImageData()
  if not vimage:
    import vtk
    vimage = vtk.vtkImageData()
    volumeNode.SetAndObserveImageData(vimage)
  import vtk.util.numpy_support
  vtype = vtk.util.numpy_support.get_vtk_array_type(narray.dtype)
  vimage.SetDimensions(vshape)
  vimage.AllocateScalars(vtype, vcomponents)

  narrayTarget = arrayFromVolume(volumeNode)
  narrayTarget[:] = narray

  # Notify the application that image data is changed
  # (same notifications as in vtkMRMLVolumeNode.SetImageDataConnection)
  import slicer
  volumeNode.StorableModified()
  volumeNode.Modified()
  volumeNode.InvokeEvent(slicer.vtkMRMLVolumeNode.ImageDataModifiedEvent, volumeNode)

#
# VTK
#

class VTKObservationMixin(object):
  def __init__(self):
    super(VTKObservationMixin, self).__init__()
    self.Observations = []

  def removeObservers(self, method=None):
    for o, e, m, g, t in list(self.Observations):
      if method == m or method is None:
        o.RemoveObserver(t)
        self.Observations.remove([o, e, m, g, t])

  def addObserver(self, object, event, method, group = 'none'):
    if self.hasObserver(object, event, method):
      print('already has observer')
      return
    tag = object.AddObserver(event, method)
    self.Observations.append([object, event, method, group, tag])

  def removeObserver(self, object, event, method):
    for o, e, m, g, t in self.Observations:
      if o == object and e == event and m == method:
        o.RemoveObserver(t)
        self.Observations.remove([o, e, m, g, t])

  def hasObserver(self, object, event, method):
    for o, e, m, g, t in self.Observations:
      if o == object and e == event and m == method:
        return True
    return False

  def observer(self, event, method):
    for o, e, m, g, t in self.Observations:
      if e == event and m == method:
        return o
    return None

def toVTKString(str):
  """Convert unicode string into 8-bit encoded ascii string.
  Unicode characters without ascii equivalent will be stripped out.
  """
  vtkStr = ""
  for c in str:
    try:
      cc = c.encode("latin1", "ignore")
    except (UnicodeDecodeError):
      cc = "?"
    vtkStr = vtkStr + cc
  return vtkStr

#
# File Utlities
#

def tempDirectory(key='__SlicerTemp__',tempDir=None,includeDateTime=True):
  """Come up with a unique directory name in the temp dir and make it and return it
  # TODO: switch to QTemporaryDir in Qt5.
  Note: this directory is not automatically cleaned up
  """
  import qt, slicer
  if not tempDir:
    tempDir = qt.QDir(slicer.app.temporaryPath)
  tempDirName = key
  if includeDateTime:
    key += qt.QDateTime().currentDateTime().toString("yyyy-MM-dd_hh+mm+ss.zzz")
  fileInfo = qt.QFileInfo(qt.QDir(tempDir), tempDirName)
  dirPath = fileInfo.absoluteFilePath()
  qt.QDir().mkpath(dirPath)
  return dirPath

#
# Misc. Utility methods
#
def unicodeify(s):
  """
  Avoid UnicodeEncodeErrors using the technique described here:
  http://stackoverflow.com/questions/9942594/unicodeencodeerror-ascii-codec-cant-encode-character-u-xa0-in-position-20
  """
  return u' '.join(s).encode('utf-8').strip()

def delayDisplay(message,autoCloseMsec=1000):
  """Display an information message in a popup window for a short time.
  If autoCloseMsec>0 then the window is closed after waiting for autoCloseMsec milliseconds
  If autoCloseMsec=0 then the window is not closed until the user clicks on it.
  """
  import qt, slicer
  import logging
  logging.info(message)
  messagePopup = qt.QDialog()
  layout = qt.QVBoxLayout()
  messagePopup.setLayout(layout)
  label = qt.QLabel(message,messagePopup)
  layout.addWidget(label)
  if autoCloseMsec>0:
    qt.QTimer.singleShot(autoCloseMsec, messagePopup.close)
  else:
    okButton = qt.QPushButton("OK")
    layout.addWidget(okButton)
    okButton.connect('clicked()', messagePopup.close)
  messagePopup.exec_()

def infoDisplay(text, windowTitle="Slicer information", parent=None, standardButtons=None, **kwargs):
  """Display popup with a info message.
  """
  import qt
  import logging
  logging.info(text)
  if mainWindow(verbose=False):
    standardButtons = standardButtons if standardButtons else qt.QMessageBox.Ok
    messageBox(text, parent, windowTitle=windowTitle, icon=qt.QMessageBox.Information, standardButtons=standardButtons,
               **kwargs)

def warningDisplay(text, windowTitle="Slicer warning", parent=None, standardButtons=None, **kwargs):
  """Display popup with a warning message.
  """
  import qt
  import logging
  logging.warning(text)
  if mainWindow(verbose=False):
    standardButtons = standardButtons if standardButtons else qt.QMessageBox.Ok
    messageBox(text, parent, windowTitle=windowTitle, icon=qt.QMessageBox.Warning, standardButtons=standardButtons,
               **kwargs)

def errorDisplay(text, windowTitle="Slicer error", parent=None, standardButtons=None, **kwargs):
  """Display an error popup.
  """
  import qt
  import logging
  logging.error(text)
  if mainWindow(verbose=False):
    standardButtons = standardButtons if standardButtons else qt.QMessageBox.Ok
    messageBox(text, parent, windowTitle=windowTitle, icon=qt.QMessageBox.Critical, standardButtons=standardButtons,
               **kwargs)

def confirmOkCancelDisplay(text, windowTitle="Slicer confirmation", parent=None, **kwargs):
  """Display an confirmation popup. Return if confirmed with OK.
  """
  import qt
  result = messageBox(text, parent=parent, windowTitle=windowTitle, icon=qt.QMessageBox.Question,
                       standardButtons=qt.QMessageBox.Ok | qt.QMessageBox.Cancel, **kwargs)
  return result == qt.QMessageBox.Ok

def confirmYesNoDisplay(text, windowTitle="Slicer confirmation", parent=None, **kwargs):
  """Display an confirmation popup. Return if confirmed with Yes.
  """
  import qt
  result = messageBox(text, parent=parent, windowTitle=windowTitle, icon=qt.QMessageBox.Question,
                       standardButtons=qt.QMessageBox.Yes | qt.QMessageBox.No, **kwargs)
  return result == qt.QMessageBox.Yes

def confirmRetryCloseDisplay(text, windowTitle="Slicer error", parent=None, **kwargs):
  """Display an confirmation popup. Return if confirmed with Retry.
  """
  import qt
  result = messageBox(text, parent=parent, windowTitle=windowTitle, icon=qt.QMessageBox.Critical,
                       standardButtons=qt.QMessageBox.Retry | qt.QMessageBox.Close, **kwargs)
  return result == qt.QMessageBox.Retry

def messageBox(text, parent=None, **kwargs):
  import qt
  mbox = qt.QMessageBox(parent if parent else mainWindow())
  mbox.text = text
  for key, value in kwargs.iteritems():
    if hasattr(mbox, key):
      setattr(mbox, key, value)
  return mbox.exec_()

def createProgressDialog(parent=None, value=0, maximum=100, labelText="", windowTitle="Processing...", **kwargs):
  """Display a modal QProgressDialog. Go to QProgressDialog documentation
  http://pyqt.sourceforge.net/Docs/PyQt4/qprogressdialog.html for more keyword arguments, that could be used.
  E.g. progressbar = createProgressIndicator(autoClose=False) if you don't want the progress dialog to automatically
  close.
  Updating progress value with progressbar.value = 50
  Updating label text with progressbar.labelText = "processing XYZ"
  """
  import qt
  progressIndicator = qt.QProgressDialog(parent if parent else mainWindow())
  progressIndicator.minimumDuration = 0
  progressIndicator.maximum = maximum
  progressIndicator.value = value
  progressIndicator.windowTitle = windowTitle
  progressIndicator.labelText = labelText
  for key, value in kwargs.iteritems():
    if hasattr(progressIndicator, key):
      setattr(progressIndicator, key, value)
  return progressIndicator

def toBool(value):
  """Convert any type of value to a boolean.
  The function uses the following heuristic:
  1) If the value can be converted to an integer, the integer is then
  converted to a boolean.
  2) If the value is a string, return True if it is equal to 'true'. False otherwise.
  Note that the comparison is case insensitive.
  3) If the value is neither an integer or a string, the bool() function is applied.

  >>> [toBool(x) for x in range(-2, 2)]
  [True, True, False, True]
  >>> [toBool(x) for x in ['-2', '-1', '0', '1', '2', 'Hello']]
  [True, True, False, True, True, False]
  >>> toBool(object())
  True
  >>> toBool(None)
  False
  """
  try:
    return bool(int(value))
  except (ValueError, TypeError):
    return value.lower() in ['true'] if isinstance(value, basestring) else bool(value)

def settingsValue(key, default, converter=lambda v: v, settings=None):
  """Return settings value associated with key if it exists or the provided default otherwise.
  ``settings`` parameter is expected to be a valid ``qt.Settings`` object.
  """
  import qt
  settings = qt.QSettings() if settings is None else settings
  return converter(settings.value(key)) if settings.contains(key) else default

def clickAndDrag(widget,button='Left',start=(10,10),end=(10,40),steps=20,modifiers=[]):
  """
  Send synthetic mouse events to the specified widget (qMRMLSliceWidget or qMRMLThreeDView)
  button : "Left", "Middle", "Right", or "None"
  start, end : window coordinates for action
  steps : number of steps to move in, if <2 then mouse jumps to the end position
  modifiers : list containing zero or more of "Shift" or "Control"

  Hint: for generating test data you can use this snippet of code:

layoutManager = slicer.app.layoutManager()
threeDView = layoutManager.threeDWidget(0).threeDView()
style = threeDView.interactorStyle()
interactor = style.GetInteractor()
def onClick(caller,event):
    print(interactor.GetEventPosition())

interactor.AddObserver(vtk.vtkCommand.LeftButtonPressEvent, onClick)

  """
  style = widget.interactorStyle()
  interactor = style.GetInteractor()
  if button == 'Left':
    down = interactor.LeftButtonPressEvent
    up = interactor.LeftButtonReleaseEvent
  elif button == 'Right':
    down = interactor.RightButtonPressEvent
    up = interactor.RightButtonReleaseEvent
  elif button == 'Middle':
    down = interactor.MiddleButtonPressEvent
    up = interactor.MiddleButtonReleaseEvent
  elif button == 'None' or not button:
    down = lambda : None
    up = lambda : None
  else:
    raise Exception("Bad button - should be Left or Right, not %s" % button)
  if 'Shift' in modifiers:
    interactor.SetShiftKey(1)
  if 'Control' in modifiers:
    interactor.SetControlKey(1)
  interactor.SetEventPosition(*start)
  down()
  if (steps<2):
    interactor.SetEventPosition(end[0], end[1])
    interactor.MouseMoveEvent()
  else:
    for step in xrange(steps):
      frac = float(step)/(steps-1)
      x = int(start[0] + frac*(end[0]-start[0]))
      y = int(start[1] + frac*(end[1]-start[1]))
      interactor.SetEventPosition(x,y)
      interactor.MouseMoveEvent()
  up()
  interactor.SetShiftKey(0)
  interactor.SetControlKey(0)

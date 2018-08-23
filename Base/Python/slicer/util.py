
from __future__ import print_function

#
# General
#

EXIT_SUCCESS = 0
EXIT_FAILURE = 1

def quit():
  exit(EXIT_SUCCESS)

def exit(status=EXIT_SUCCESS):
  """Exits the application with the specified exit code.
  The method does not stops the process immediately but lets
  pending events to be processed.
  If exit() is called again while processing pending events,
  the error code will be overwritten.

  To make the application exit immediately, this code can be used.
  Note that forcing the application to exit may result in
  improperly released files and other resources.

  .. code-block:: python

    import sys
    sys.exit(status)

  """

  from slicer import app
  # Prevent automatic application exit (for example, triggered by starting Slicer
  # with "--testing" argument) from overwriting the exit code that we set now.
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

def startupEnvironment():
  """Returns the environment without the Slicer specific values.

  Path environment variables like `PATH`, `LD_LIBRARY_PATH` or `PYTHONPATH`
  will not contain values found in the launcher settings.

  Similarly `key=value` environment variables also found in the launcher
  settings are excluded. Note that if a value was associated with a key prior
  starting Slicer, it will not be set in the environment returned by this
  function.

  The function excludes both the Slicer launcher settings and the revision
  specific launcher settings.
  """
  import slicer
  startupEnv = slicer.app.startupEnvironment()
  import os
  if os.name == 'nt':
    # On Windows, subprocess functions expect environment to contain strings
    # and Qt provide us unicode strings, so we need to convert them.
    return {str(varname): str(startupEnv.value(varname)) for varname in startupEnv.keys()}
  else:
    return {varname: startupEnv.value(varname) for varname in startupEnv.keys()}

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
  return lookupTopLevelWidget('qSlicerMainWindow', verbose)

def pythonShell(verbose = True):
  from slicer import app
  console = app.pythonConsole()
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
                         foregroundOpacity=None, labelOpacity=None, fit=False):
  """ Set the slice views with the given nodes.
  If node ID is not specified (or value is 'keep-current') then the layer will not be modified.
  :param background: node or node ID to be used for the background layer
  :param foreground: node or node ID to be used for the foreground layer
  :param label: node or node ID to be used for the label layer
  :param foregroundOpacity: opacity of the foreground layer
  :param labelOpacity: opacity of the label layer
  :param fit: fit slice views to their content (position&zoom to show all visible layers)
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
      if background != 'keep-current':
          sliceViewer.SetBackgroundVolumeID(_nodeID(background))
      if foreground != 'keep-current':
          sliceViewer.SetForegroundVolumeID(_nodeID(foreground))
      if foregroundOpacity is not None:
          sliceViewer.SetForegroundOpacity(foregroundOpacity)
      if label != 'keep-current':
          sliceViewer.SetLabelVolumeID(_nodeID(label))
      if labelOpacity is not None:
          sliceViewer.SetLabelOpacity(labelOpacity)

  if fit:
    layoutManager = slicer.app.layoutManager()
    if layoutManager is not None:
      sliceLogics = layoutManager.mrmlSliceLogics()
      for i in xrange(sliceLogics.GetNumberOfItems()):
        sliceLogic = sliceLogics.GetItemAsObject(i)
        if sliceLogic:
          sliceLogic.FitSliceToAll()

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
  """Properties:
  - name: this name will be used as node name for the loaded volume
  - labelmap: interpret volume as labelmap
  - singleFile: ignore all other files in the directory
  - center: ignore image position
  - discardOrientation: ignore image axis directions
  - autoWindowLevel: compute window/level automatically
  - show: display volume in slice viewers after loading is completed
  - fileNames: list of filenames to load the volume from
  """
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

class MRMLNodeNotFoundException(Exception):
  """Exception raised when a requested MRML node was not found.
  """
  pass

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
  Throws MRMLNodeNotFoundException exception if no node is found
  that matches the specified pattern.
  """
  nodes = getNodes(pattern, scene)
  if not nodes:
    raise MRMLNodeNotFoundException("could not find nodes in the scene by name or id '%s'" % (pattern if (type(pattern) == str) else ""))
  return nodes.values()[index]

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
  """Return the first node in the scene that matches the specified node name and node class.
  """
  import slicer
  if scene is None:
    scene = slicer.mrmlScene
  return scene.GetFirstNode(name, className)

def getFirstNodeByName(name, className=None):
  """Get the first MRML node that name starts with the specified name.
  Optionally specify a classname that must also match.
  """
  import slicer
  scene = slicer.mrmlScene
  return scene.GetFirstNode(name, className, False, False)

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
  used in scripts to be sure you get exactly what you want, such as
  arrayFromVolume, arrayFromModelPoints, and arrayFromGridTransform.
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
  After all modifications has been completed, call :py:meth:`arrayFromVolumeModified`.

  .. warning:: Memory area of the returned array is managed by VTK, therefore
    values in the array may be changed, but the array must not be reallocated
    (change array size, shallow-copy content from other array most likely causes
    application crash). To allow arbitrary numpy operations on a volume array:

      1. Make a deep-copy of the returned VTK-managed array using :func:`numpy.copy`.
      2. Perform any computations using the copied array.
      3. Write results back to the image data using :py:meth:`updateVolumeFromArray`.
  """
  scalarTypes = ['vtkMRMLScalarVolumeNode', 'vtkMRMLLabelMapVolumeNode']
  vectorTypes = ['vtkMRMLVectorVolumeNode', 'vtkMRMLMultiVolumeNode', 'vtkMRMLDiffusionWeightedVolumeNode']
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

def arrayFromVolumeModified(volumeNode):
  """Indicate that modification of a numpy array returned by :py:meth:`arrayFromVolume` has been completed."""
  imageData = volumeNode.GetImageData()
  pointData = imageData.GetPointData() if imageData else None
  if pointData:
    if pointData.GetScalars():
      pointData.GetScalars().Modified()
    if pointData.GetTensors():
      pointData.GetTensors().Modified()
  volumeNode.Modified()

def arrayFromModelPoints(modelNode):
  """Return point positions of a model node as numpy array.
  Point coordinates can be modified by modfying the numpy array.
  After all modifications has been completed, call :py:meth:`arrayFromModelPointsModified`.

  .. warning:: Important: memory area of the returned array is managed by VTK,
    therefore values in the array may be changed, but the array must not be reallocated.
    See :py:meth:`arrayFromVolume` for details.
  """
  import vtk.util.numpy_support
  pointData = modelNode.GetPolyData().GetPoints().GetData()
  narray = vtk.util.numpy_support.vtk_to_numpy(pointData)
  return narray

def arrayFromModelPointsModified(modelNode):
  """Indicate that modification of a numpy array returned by :py:meth:`arrayFromModelPoints` has been completed."""
  if modelNode.GetPolyData():
    modelNode.GetPolyData().GetPoints().GetData().Modified()
  # Trigger re-render
  modelNode.GetDisplayNode().Modified()

def arrayFromModelPointData(modelNode, arrayName):
  """Return point data array of a model node as numpy array.

  .. warning:: Important: memory area of the returned array is managed by VTK,
    therefore values in the array may be changed, but the array must not be reallocated.
    See :py:meth:`arrayFromVolume` for details.
  """
  import vtk.util.numpy_support
  arrayVtk = modelNode.GetPolyData().GetPointData().GetArray(arrayName)
  narray = vtk.util.numpy_support.vtk_to_numpy(arrayVtk)
  return narray

def arrayFromGridTransform(gridTransformNode):
  """Return voxel array from transform node as numpy array.
  Vector values are not copied. Values in the transform node can be modified
  by changing values in the numpy array.
  After all modifications has been completed, call :py:meth:`arrayFromGridTransformModified`.

  .. warning:: Important: memory area of the returned array is managed by VTK,
    therefore values in the array may be changed, but the array must not be reallocated.
    See :py:meth:`arrayFromVolume` for details.
  """
  transformGrid = gridTransformNode.GetTransformFromParent()
  displacementGrid = transformGrid.GetDisplacementGrid()
  nshape = tuple(reversed(displacementGrid.GetDimensions()))
  import vtk.util.numpy_support
  nshape = nshape + (3,)
  narray = vtk.util.numpy_support.vtk_to_numpy(displacementGrid.GetPointData().GetScalars()).reshape(nshape)
  return narray

def arrayFromGridTransformModified(gridTransformNode):
  """Indicate that modification of a numpy array returned by :py:meth:`arrayFromModelPoints` has been completed."""
  transformGrid = gridTransformNode.GetTransformFromParent()
  displacementGrid = transformGrid.GetDisplacementGrid()
  displacementGrid.GetPointData().GetScalars().Modified()
  displacementGrid.Modified()

def arrayFromSegment(segmentationNode, segmentId):
  """Return voxel array of a segment's binary labelmap representation as numpy array.
  Voxels values are not copied.
  If binary labelmap is the master representation then voxel values in the volume node can be modified
  by changing values in the numpy array. After all modifications has been completed, call:
  segmentationNode.GetSegmentation().GetSegment(segmentID).Modified()

  .. warning:: Important: memory area of the returned array is managed by VTK,
    therefore values in the array may be changed, but the array must not be reallocated.
    See :py:meth:`arrayFromVolume` for details.
  """
  vimage = segmentationNode.GetBinaryLabelmapRepresentation(segmentId)
  nshape = tuple(reversed(vimage.GetDimensions()))
  import vtk.util.numpy_support
  narray = vtk.util.numpy_support.vtk_to_numpy(vimage.GetPointData().GetScalars()).reshape(nshape)
  return narray

def updateVolumeFromArray(volumeNode, narray):
  """Sets voxels of a volume node from a numpy array.
  Voxels values are deep-copied, therefore if the numpy array
  is modified after calling this method, voxel values in the volume node will not change.
  Dimensions and data size of the source numpy array does not have to match the current
  content of the volume node.
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

def updateTableFromArray(tableNode, narrays, columnNames=None):
  """Sets values in a table node from a numpy array.
  columnNames may contain a string or list of strings that will be used as column name(s).
  Values are copied, therefore if the numpy array  is modified after calling this method,
  values in the table node will not change.
  All previous content of the table is deleted.

  Example:

      import numpy as np
      histogram = np.histogram(arrayFromVolume(getNode('MRHead')))
      tableNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTableNode")
      updateTableFromArray(tableNode, histogram, ["Count", "Intensity"])

  """
  import numpy as np
  import vtk.util.numpy_support
  import slicer

  if tableNode is None:
    tableNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTableNode")
  if type(narrays) == np.ndarray and len(narrays.shape) == 1:
    ncolumns = [narrays]
  elif type(narrays) == np.ndarray and len(narrays.shape) == 2:
    ncolumns = narrays.T
  elif type(narrays) == tuple or type(narrays) == list:
    ncolumns = narrays
  else:
    raise ValueError('Expected narrays is a numpy ndarray, or tuple or list of numpy ndarrays, got %s instead.' % (str(type(narrays))))
  tableNode.RemoveAllColumns()
  # Convert single string to a single-element string list
  if columnNames is None:
    columnNames = []
  if isinstance(columnNames, basestring):
    columnNames = [columnNames]
  for columnIndex, ncolumn in enumerate(ncolumns):
    vcolumn = vtk.util.numpy_support.numpy_to_vtk(num_array=ncolumn.ravel(),deep=True,array_type=vtk.VTK_FLOAT)
    if (columnNames is not None) and (columnIndex < len(columnNames)):
      vcolumn.SetName(columnNames[columnIndex])
    tableNode.AddColumn(vcolumn)
  return tableNode

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

def infoDisplay(text, windowTitle=None, parent=None, standardButtons=None, **kwargs):
  """Display popup with a info message.
  """
  import qt, slicer
  import logging
  if not windowTitle:
    windowTitle = slicer.app.applicationName + " information"
  logging.info(text)
  if mainWindow(verbose=False):
    standardButtons = standardButtons if standardButtons else qt.QMessageBox.Ok
    messageBox(text, parent, windowTitle=windowTitle, icon=qt.QMessageBox.Information, standardButtons=standardButtons,
               **kwargs)

def warningDisplay(text, windowTitle=None, parent=None, standardButtons=None, **kwargs):
  """Display popup with a warning message.
  """
  import qt, slicer
  import logging
  if not windowTitle:
    windowTitle = slicer.app.applicationName + " warning"
  logging.warning(text)
  if mainWindow(verbose=False):
    standardButtons = standardButtons if standardButtons else qt.QMessageBox.Ok
    messageBox(text, parent, windowTitle=windowTitle, icon=qt.QMessageBox.Warning, standardButtons=standardButtons,
               **kwargs)

def errorDisplay(text, windowTitle=None, parent=None, standardButtons=None, **kwargs):
  """Display an error popup.
  """
  import qt, slicer
  import logging
  if not windowTitle:
    windowTitle = slicer.app.applicationName + " error"
  logging.error(text)
  if mainWindow(verbose=False):
    standardButtons = standardButtons if standardButtons else qt.QMessageBox.Ok
    messageBox(text, parent, windowTitle=windowTitle, icon=qt.QMessageBox.Critical, standardButtons=standardButtons,
               **kwargs)

def confirmOkCancelDisplay(text, windowTitle=None, parent=None, **kwargs):
  """Display an confirmation popup. Return if confirmed with OK.
  """
  import qt, slicer
  if not windowTitle:
    windowTitle = slicer.app.applicationName + " confirmation"
  result = messageBox(text, parent=parent, windowTitle=windowTitle, icon=qt.QMessageBox.Question,
                       standardButtons=qt.QMessageBox.Ok | qt.QMessageBox.Cancel, **kwargs)
  return result == qt.QMessageBox.Ok

def confirmYesNoDisplay(text, windowTitle=None, parent=None, **kwargs):
  """Display an confirmation popup. Return if confirmed with Yes.
  """
  import qt, slicer
  if not windowTitle:
    windowTitle = slicer.app.applicationName + " confirmation"
  result = messageBox(text, parent=parent, windowTitle=windowTitle, icon=qt.QMessageBox.Question,
                       standardButtons=qt.QMessageBox.Yes | qt.QMessageBox.No, **kwargs)
  return result == qt.QMessageBox.Yes

def confirmRetryCloseDisplay(text, windowTitle=None, parent=None, **kwargs):
  """Display an confirmation popup. Return if confirmed with Retry.
  """
  import qt, slicer
  if not windowTitle:
    windowTitle = slicer.app.applicationName + " error"
  result = messageBox(text, parent=parent, windowTitle=windowTitle, icon=qt.QMessageBox.Critical,
                       standardButtons=qt.QMessageBox.Retry | qt.QMessageBox.Close, **kwargs)
  return result == qt.QMessageBox.Retry

def messageBox(text, parent=None, **kwargs):
  """Displays a messagebox.
  ctkMessageBox is used instead of a default qMessageBox to provide "Don't show again" checkbox.
  For example: slicer.util.messageBox("Some message", dontShowAgainSettingsKey = "MainWindow/DontShowSomeMessage")
  """
  import qt
  import ctk
  mbox = ctk.ctkMessageBox(parent if parent else mainWindow())
  mbox.text = text
  for key, value in kwargs.iteritems():
    if hasattr(mbox, key):
      setattr(mbox, key, value)
  # Windows 10 peek feature in taskbar shows all hidden but not destroyed windows
  # (after creating and closing a messagebox, hovering over the mouse on Slicer icon, moving up the
  # mouse to the peek thumbnail would show it again).
  # Popup windows in other Qt applications often show closed popups (such as
  # Paraview's Edit / Find data dialog, MeshMixer's File/Preferences dialog).
  # By calling deleteLater, the messagebox is permanently deleted when the current call is completed.
  mbox.deleteLater()
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

def downloadFile(url, targetFilePath):
  """ Download ``url`` to local storage as ``targetFilePath``

  Target file path needs to indicate the file name and extension as well
  """
  import os
  import logging
  if not os.path.exists(targetFilePath) or os.stat(targetFilePath).st_size == 0:
    logging.info('Downloading from\n  %s\nas file\n  %s\nIt may take a few minutes...' % (url,targetFilePath))
    try:
      import urllib
      urllib.urlretrieve(url, targetFilePath)
    except Exception, e:
      import traceback
      traceback.print_exc()
      logging.error('Failed to download file from ' + url)
      return False
  else:
    logging.info('Requested file has been found: ' + targetFilePath)
  return True

def extractArchive(archiveFilePath, outputDir, expectedNumberOfExtractedFiles=None):
  """ Extract file ``archiveFilePath`` into folder ``outputDir``.

  Number of expected files unzipped may be specified in ``expectedNumberOfExtractedFiles``.
  If folder contains the same number of files as expected (if specified), then it will be
  assumed that unzipping has been successfully done earlier.
  """
  import os
  import logging
  from slicer import app
  if not os.path.exists(archiveFilePath):
    logging.error('Specified file %s does not exist' % (archiveFilePath))
    return False
  fileName, fileExtension = os.path.splitext(archiveFilePath)
  if fileExtension.lower() != '.zip':
    #TODO: Support other archive types
    logging.error('Only zip archives are supported now, got ' + fileExtension)
    return False

  numOfFilesInOutputDir = len(getFilesInDirectory(outputDir, False))
  if expectedNumberOfExtractedFiles is not None \
      and numOfFilesInOutputDir == expectedNumberOfExtractedFiles:
    logging.info('File %s already unzipped into %s' % (archiveFilePath, outputDir))
    return True

  extractSuccessful = app.applicationLogic().Unzip(archiveFilePath, outputDir)
  numOfFilesInOutputDirTest = len(getFilesInDirectory(outputDir, False))
  if extractSuccessful is False or (expectedNumberOfExtractedFiles is not None \
      and numOfFilesInOutputDirTest != expectedNumberOfExtractedFiles):
    logging.error('Unzipping %s into %s failed' % (archiveFilePath, outputDir))
    return False
  logging.info('Unzipping %s into %s successful' % (archiveFilePath, outputDir))
  return True

def downloadAndExtractArchive(url, archiveFilePath, outputDir, \
                              expectedNumberOfExtractedFiles=None, numberOfTrials=3):
  """ Downloads an archive from ``url`` as ``archiveFilePath``, and extracts it to ``outputDir``.

  This combined function tests the success of the download by the extraction step,
  and re-downloads if extraction failed.
  """
  import os
  import shutil
  import logging

  maxNumberOfTrials = numberOfTrials

  def _cleanup():
    # If there was a failure, delete downloaded file and empty output folder
    logging.warning('Download and extract failed, removing archive and destination folder and retrying. Attempt #%d...' % (maxNumberOfTrials - numberOfTrials))
    os.remove(archiveFilePath)
    shutil.rmtree(outputDir)
    os.mkdir(outputDir)

  while numberOfTrials:
    if not downloadFile(url, archiveFilePath):
      numberOfTrials -= 1
      _cleanup()
      continue
    if not extractArchive(archiveFilePath, outputDir, expectedNumberOfExtractedFiles):
      numberOfTrials -= 1
      _cleanup()
      continue
    return True

  _cleanup()
  return False

def getFilesInDirectory(directory, absolutePath=True):
  """ Collect all files in a directory and its subdirectories in a list
  """
  import os
  allFiles=[]
  for root, subdirs, files in os.walk(directory):
    for fileName in files:
      if absolutePath:
        fileAbsolutePath = os.path.abspath(os.path.join(root, fileName)).replace('\\','/')
        allFiles.append(fileAbsolutePath)
      else:
        allFiles.append(fileName)
  return allFiles

def plot(narray, xColumnIndex = -1, columnNames = None, title = None, show = True, nodes = None):
  """ Create a plot from a numpy array that contains two or more columns.
  :param narray: input numpy array containing data series in columns.
  :param xColumnIndex: index of column that will be used as x axis.
    If it is set to negative number (by default) then row index will be used as x coordinate.
  :param columnNames: names of each column of the input array.
  :param title: title of the chart. Plot node names are set based on this value.
  :param nodes: plot chart, table, and list of plot series nodes.
    Specified in a dictionary, with keys: 'chart', 'table', 'series'.
    Series contains a list of plot series nodes (one for each table column).
    The parameter is used both as an input and output.
  :return: plot chart node. Plot chart node provides access to chart properties and plot series nodes.

  Example 1: simple plot

  .. code-block:: python

    # Get sample data
    import numpy as np
    import SampleData
    volumeNode = SampleData.SampleDataLogic().downloadMRHead()

    # Create new plot
    histogram = np.histogram(arrayFromVolume(volumeNode), bins=50)
    chartNode = plot(histogram, xColumnIndex = 1)

    # Change some plot properties
    chartNode.SetTitle("My histogram")
    chartNode.GetNthPlotSeriesNode(0).SetPlotType(slicer.vtkMRMLPlotSeriesNode.PlotTypeScatterBar)

  Example 2: plot with multiple updates

  .. code-block:: python

    # Get sample data
    import numpy as np
    import SampleData
    volumeNode = SampleData.SampleDataLogic().downloadMRHead()

    # Create variable that will store plot nodes (chart, table, series)
    plotNodes = {}

    # Create new plot
    histogram = np.histogram(arrayFromVolume(volumeNode), bins=80)
    plot(histogram, xColumnIndex = 1, nodes = plotNodes)

    # Update plot
    histogram = np.histogram(arrayFromVolume(volumeNode), bins=40)
    plot(histogram, xColumnIndex = 1, nodes = plotNodes)

  """
  import slicer

  chartNode = None
  tableNode = None
  seriesNodes = []

  # Retrieve nodes that must be reused
  if nodes is not None:
    if nodes.has_key('chart'):
      chartNode = nodes['chart']
    if nodes.has_key('table'):
      tableNode = nodes['table']
    if nodes.has_key('series'):
      seriesNodes = nodes['series']

  # Create table node
  if tableNode is None:
    tableNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTableNode")

  if title is not None:
    tableNode.SetName(title+' table')
  updateTableFromArray(tableNode, narray)
  # Update column names
  numberOfColumns = tableNode.GetTable().GetNumberOfColumns()
  yColumnIndex = 0
  for columnIndex in range(numberOfColumns):
    if (columnNames is not None) and (len(columnNames) > columnIndex):
      columnName = columnNames[columnIndex]
    else:
      if columnIndex == xColumnIndex:
        columnName = "X"
      elif yColumnIndex == 0:
        columnName = "Y"
        yColumnIndex += 1
      else:
        columnName = "Y"+str(yColumnIndex)
        yColumnIndex += 1
    tableNode.GetTable().GetColumn(columnIndex).SetName(columnName)

  # Create chart and add plot
  if chartNode is None:
    chartNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLPlotChartNode")
  if title is not None:
    chartNode.SetName(title + ' chart')
    chartNode.SetTitle(title)

  # Create plot series node(s)
  xColumnName = columnNames[xColumnIndex] if (columnNames is not None) and (len(columnNames) > 0) else "X"
  seriesIndex = -1
  for columnIndex in range(numberOfColumns):
    if columnIndex == xColumnIndex:
      continue
    seriesIndex += 1
    if len(seriesNodes) > seriesIndex:
      seriesNode = seriesNodes[seriesIndex]
    else:
      seriesNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLPlotSeriesNode")
      seriesNodes.append(seriesNode)
      seriesNode.SetUniqueColor()
    seriesNode.SetAndObserveTableNodeID(tableNode.GetID())
    if xColumnIndex < 0:
      seriesNode.SetXColumnName("")
      seriesNode.SetPlotType(seriesNode.PlotTypeLine)
    else:
      seriesNode.SetXColumnName(xColumnName)
      seriesNode.SetPlotType(seriesNode.PlotTypeScatter)
    yColumnName = tableNode.GetTable().GetColumn(columnIndex).GetName()
    seriesNode.SetYColumnName(yColumnName)
    if title:
      seriesNode.SetName(title + " " + yColumnName)
    if not chartNode.HasPlotSeriesNodeID(seriesNode.GetID()):
      chartNode.AddAndObservePlotSeriesNodeID(seriesNode.GetID())

  # Show plot in layout
  if show:
    slicer.modules.plots.logic().ShowChartInLayout(chartNode)

  # Without this, chart view may show up completely empty when the same nodes are updated
  # (this is probably due to a bug in plotting nodes or widgets).
  chartNode.Modified()

  if nodes is not None:
    nodes['table'] = tableNode
    nodes['chart'] = chartNode
    nodes['series'] = seriesNodes

  return chartNode


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
    if name and fnmatch.fnmatchcase(p.name, name):
      children.append(p)
    elif text:
      try:
        p.text
        if fnmatch.fnmatchcase(p.text, text):
          children.append(p)
      except (AttributeError, TypeError):
        pass
    elif title:
      try:
        p.title
        if fnmatch.fnmatchcase(p.title, title):
          children.append(p)
      except AttributeError:
        pass
    elif className:
      try:
        p.className()
        if fnmatch.fnmatchcase(p.className(), className):
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

def getNodes(pattern = "", scene=None):
    """Return a dictionary of nodes where the name or id matches the 'pattern'.
    Providing an empty 'pattern' string will return all nodes.
    """
    import slicer, fnmatch
    nodes = {}
    if scene is None:
      scene = slicer.mrmlScene
    count = scene.GetNumberOfNodes()
    for idx in range(count):
      node = scene.GetNthNode(idx)
      name = node.GetName()
      id = node.GetID()
      if fnmatch.fnmatchcase(name, pattern) or fnmatch.fnmatchcase(id, pattern):
        nodes[node.GetName()] = node
    return nodes

def getNode(pattern = "", index = 0, scene=None):
    """Return the indexth node where name or id matches 'pattern'.
    Providing an empty 'pattern' string will return all nodes.
    """
    nodes = getNodes(pattern, scene)
    try:
      if nodes.keys():
        return nodes.values()[index]
    except IndexError:
      return None

def getFirstNodeByClassByName(className, name, scene=None):
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
  nodes = getNodes(name+'*')
  for nodeName in nodes.keys():
    if not className:
      return (nodes[nodeName]) # return the first one
    else:
      if nodes[nodeName].IsA(className):
        return (nodes[nodeName])
  return None

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
  scalarTypes = ('vtkMRMLScalarVolumeNode', 'vtkMRMLLabelMapVolumeNode')
  vectorTypes = ('vtkMRMLVectorVolumeNode', 'vtkMRMLMultiVolumeNode')
  tensorTypes = ('vtkMRMLDiffusionTensorVolumeNode',)
  pointTypes = ('vtkMRMLModelNode',)
  import vtk.util.numpy_support
  n = getNode(pattern=pattern, index=index)
  if n.GetClassName() in scalarTypes:
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
  elif n.GetClassName() in pointTypes:
    p = n.GetPolyData().GetPoints().GetData()
    a = vtk.util.numpy_support.vtk_to_numpy(p)
    return a
  # TODO: accessors for other node types: polydata (verts, polys...), colors


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
  return str.encode('latin1', 'ignore')


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

def warningDisplay(message,autoCloseMsec=1000,windowTitle="Slicer warning"):
  """Display popup with a warning message.
  """
  import qt, slicer
  import logging
  logging.warning(message)
  if mainWindow(verbose=False):
    qt.QMessageBox.warning(slicer.util.mainWindow(), windowTitle, message)

def errorDisplay(message,autoCloseMsec=1000,windowTitle="Slicer error"):
  """Display an error popup.
  """
  import qt, slicer
  import logging
  logging.error(message)
  if mainWindow(verbose=False):
    qt.QMessageBox.critical(slicer.util.mainWindow(), windowTitle, message)

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

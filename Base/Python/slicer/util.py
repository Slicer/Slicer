
from __future__ import print_function

#
# General
#

EXIT_SUCCESS = 0
EXIT_FAILURE = 1

TESTING_DATA_URL = "https://github.com/Slicer/SlicerTestingData/releases/download/"
"""Base URL for downloading testing data.

Datasets can be downloaded using URL of the form ``TESTING_DATA_URL + "SHA256/" + sha256ofDataSet``
"""

DATA_STORE_URL = "https://github.com/Slicer/SlicerDataStore/releases/download/"
"""Base URL for downloading data from Slicer Data Store. Data store contains atlases,
registration case library images, and various sample data sets.

Datasets can be downloaded using URL of the form ``DATA_STORE_URL + "SHA256/" + sha256ofDataSet``
"""


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
  # "if varname" is added to reject empty key (it is invalid)
  if os.name == 'nt':
    # On Windows, subprocess functions expect environment to contain strings
    # and Qt provide us unicode strings, so we need to convert them.
    return {str(varname): str(startupEnv.value(varname)) for varname in list(startupEnv.keys()) if varname}
  else:
    return {varname: startupEnv.value(varname) for varname in list(startupEnv.keys()) if varname}

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
  for kwarg in kwargs.keys():
    if kwargs[kwarg]:
      expected_matches.append(kwarg)
  while parents:
    p = parents.pop()
    # sometimes, p is null, f.e. when using --python-script or --python-code
    if not p:
      continue
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
        if isinstance(attr_name, str):
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

  :raises RuntimeError: if the UI file is not found or if no
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

def startQtDesigner(args = None):
  """ Start Qt Designer application to allow editing UI files.
  """
  import slicer
  cmdLineArguments = []
  if args is not None:
    if isinstance(args, str):
      cmdLineArguments.append(args)
    else:
      cmdLineArguments.extend(args)
  return slicer.app.launchDesigner(cmdLineArguments)

def childWidgetVariables(widget):
  """ Get child widgets as attributes of an object.

  Each named child widget is accessible as an attribute of the returned object,
  with the attribute name matching the child widget name.
  This function provides convenient access to widgets in a loaded UI file.

  Example::

    uiWidget = slicer.util.loadUI(myUiFilePath)
    self.ui = slicer.util.childWidgetVariables(uiWidget)
    self.ui.inputSelector.setMRMLScene(slicer.mrmlScene)
    self.ui.outputSelector.setMRMLScene(slicer.mrmlScene)

  """
  ui = type('', (), {})() # empty object
  childWidgets = findChildren(widget)
  for childWidget in childWidgets:
    if hasattr(childWidget, "name"):
      setattr(ui, childWidget.name, childWidget)
  return ui

def addParameterEditWidgetConnections(parameterEditWidgets, updateParameterNodeFromGUI):
  """ Add connections to get notification of a widget change.

  The function is useful for calling updateParameterNodeFromGUI method in scripted module widgets.

  Note: Only a few widget classes are supported now. More will be added later. Report any missing classes at discourse.slicer.org.

  Example::

    class SurfaceToolboxWidget(ScriptedLoadableModuleWidget, VTKObservationMixin):
      ...
      def setup(self):
        ...
        self.parameterEditWidgets = [
          (self.ui.inputModelSelector, "inputModel"),
          (self.ui.outputModelSelector, "outputModel"),
          (self.ui.decimationButton, "decimation"),
          ...]
        slicer.util.addParameterEditWidgetConnections(self.parameterEditWidgets, self.updateParameterNodeFromGUI)

      def updateGUIFromParameterNode(self, caller=None, event=None):
        if self._parameterNode is None or self._updatingGUIFromParameterNode:
          return
        self._updatingGUIFromParameterNode = True
        slicer.util.updateParameterEditWidgetsFromNode(self.parameterEditWidgets, self._parameterNode)
        self._updatingGUIFromParameterNode = False

      def updateParameterNodeFromGUI(self, caller=None, event=None):
        if self._parameterNode is None or self._updatingGUIFromParameterNode:
          return
        wasModified = self._parameterNode.StartModify()  # Modify all properties in a single batch
        slicer.util.updateNodeFromParameterEditWidgets(self.parameterEditWidgets, self._parameterNode)
        self._parameterNode.EndModify(wasModified)
  """

  for (widget, parameterName) in parameterEditWidgets:
    widgetClassName = widget.className()
    if widgetClassName=="QSpinBox":
      widget.connect("valueChanged(int)", updateParameterNodeFromGUI)
    elif widgetClassName=="QCheckBox":
      widget.connect("clicked()", updateParameterNodeFromGUI)
    elif widgetClassName=="QPushButton":
      widget.connect("toggled(bool)", updateParameterNodeFromGUI)
    elif widgetClassName=="qMRMLNodeComboBox":
      widget.connect("currentNodeIDChanged(QString)", updateParameterNodeFromGUI)
    elif widgetClassName=="QComboBox":
      widget.connect("currentIndexChanged(int)", updateParameterNodeFromGUI)
    elif widgetClassName=="ctkSliderWidget":
      widget.connect("valueChanged(double)", updateParameterNodeFromGUI)

def removeParameterEditWidgetConnections(parameterEditWidgets, updateParameterNodeFromGUI):
  """ Remove connections created by :py:meth:`addParameterEditWidgetConnections`.
  """

  for (widget, parameterName) in parameterEditWidgets:
    widgetClassName = widget.className()
    if widgetClassName=="QSpinBox":
      widget.disconnect("valueChanged(int)", updateParameterNodeFromGUI)
    elif widgetClassName=="QPushButton":
      widget.disconnect("toggled(bool)", updateParameterNodeFromGUI)
    elif widgetClassName=="qMRMLNodeComboBox":
      widget.disconnect("currentNodeIDChanged(QString)", updateParameterNodeFromGUI)
    elif widgetClassName=="QComboBox":
      widget.disconnect("currentIndexChanged(int)", updateParameterNodeFromGUI)
    elif widgetClassName=="ctkSliderWidget":
      widget.disconnect("valueChanged(double)", updateParameterNodeFromGUI)

def updateParameterEditWidgetsFromNode(parameterEditWidgets, parameterNode):
  """ Update widgets from values stored in a vtkMRMLScriptedModuleNode.

  The function is useful for implementing updateGUIFromParameterNode.

  Note: Only a few widget classes are supported now. More will be added later. Report any missing classes at discourse.slicer.org.

  See example in :py:meth:`addParameterEditWidgetConnections` documentation.
  """

  for (widget, parameterName) in parameterEditWidgets:
    widgetClassName = widget.className()
    if widgetClassName=="QSpinBox":
      widget.value = int(float(parameterNode.GetParameter(parameterName)))
    if widgetClassName=="ctkSliderWidget":
      widget.value = float(parameterNode.GetParameter(parameterName))
    elif widgetClassName=="QCheckBox" or widgetClassName=="QPushButton":
      widget.checked = parameterNode.GetParameter(parameterName) == "true"
    elif widgetClassName=="QComboBox":
      widget.setCurrentText(parameterNode.GetParameter(parameterName))
    elif widgetClassName=="qMRMLNodeComboBox":
      widget.currentNodeID = parameterNode.GetNodeReferenceID(parameterName)

def updateNodeFromParameterEditWidgets(parameterEditWidgets, parameterNode):
  """ Update vtkMRMLScriptedModuleNode from widgets.

  The function is useful for implementing updateParameterNodeFromGUI.

  Note: Only a few widget classes are supported now. More will be added later. Report any missing classes at discourse.slicer.org.

  See example in :py:meth:`addParameterEditWidgetConnections` documentation.
  """

  for (widget, parameterName) in parameterEditWidgets:
    widgetClassName = widget.className()
    if widgetClassName=="QSpinBox" or widgetClassName=="ctkSliderWidget":
      parameterNode.SetParameter(parameterName, str(widget.value))
    elif widgetClassName=="QCheckBox" or widgetClassName=="QPushButton":
      parameterNode.SetParameter(parameterName, "true" if widget.checked else "false")
    elif widgetClassName=="QComboBox":
      parameterNode.SetParameter(parameterName, widget.currentText)
    elif widgetClassName=="qMRMLNodeComboBox":
      parameterNode.SetNodeReferenceID(parameterName, widget.currentNodeID)

def setSliceViewerLayers(background='keep-current', foreground='keep-current', label='keep-current',
                         foregroundOpacity=None, labelOpacity=None, fit=False, rotateToVolumePlane=False):
  """ Set the slice views with the given nodes.

  If node ID is not specified (or value is 'keep-current') then the layer will not be modified.

  :param background: node or node ID to be used for the background layer
  :param foreground: node or node ID to be used for the foreground layer
  :param label: node or node ID to be used for the label layer
  :param foregroundOpacity: opacity of the foreground layer
  :param labelOpacity: opacity of the label layer
  :param rotateToVolumePlane: rotate views to closest axis of the selected background, foreground, or label volume
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

  if rotateToVolumePlane:
    if background != 'keep-current':
      volumeNode = slicer.mrmlScene.GetNodeByID(_nodeID(background))
    elif foreground != 'keep-current':
      volumeNode = slicer.mrmlScene.GetNodeByID(_nodeID(foreground))
    elif label != 'keep-current':
      volumeNode = slicer.mrmlScene.GetNodeByID(_nodeID(label))
    else:
      volumeNode = None
    if volumeNode:
      layoutManager = slicer.app.layoutManager()
      for sliceViewName in layoutManager.sliceViewNames():
        layoutManager.sliceWidget(sliceViewName).mrmlSliceNode().RotateToVolumePlane(volumeNode)

  if fit:
    layoutManager = slicer.app.layoutManager()
    if layoutManager is not None:
      sliceLogics = layoutManager.mrmlSliceLogics()
      for i in range(sliceLogics.GetNumberOfItems()):
        sliceLogic = sliceLogics.GetItemAsObject(i)
        if sliceLogic:
          sliceLogic.FitSliceToAll()

def setToolbarsVisible(visible, ignore=None):
  """Show/hide all existing toolbars, except those listed in
  ignore list.
  """

  for toolbar in mainWindow().findChildren('QToolBar'):
    if ignore is not None and toolbar in ignore:
      continue
    toolbar.setVisible(visible)

  # Prevent sequence browser toolbar showing up automatically
  # when a sequence is loaded.
  # (put in try block because Sequence Browser module is not always installed)
  try:
    import slicer
    slicer.modules.sequences.autoShowToolBar = visible
  except:
    # Sequences module is not installed
    pass

def setMenuBarsVisible(visible, ignore=None):
  """Show/hide all menu bars, except those listed in
  ignore list."""
  for menubar in mainWindow().findChildren('QMenuBar'):
    if ignore is not None and menubar in ignore:
      continue
    menubar.setVisible(visible)

def setPythonConsoleVisible(visible):
  """Show/hide Python console."""
  mainWindow().pythonConsole().parent().setVisible(visible)

def setStatusBarVisible(visible):
  """Show/hide status bar"""
  mainWindow(verbose=False).statusBar().setVisible(visible)

def setViewControllersVisible(visible):
  """Show/hide view controller toolbar at the top of slice and 3D views"""
  import slicer
  lm = slicer.app.layoutManager()
  for viewIndex in range(lm.threeDViewCount):
    lm.threeDWidget(viewIndex).threeDController().setVisible(visible)
  for sliceViewName in lm.sliceViewNames():
    lm.sliceWidget(sliceViewName).sliceController().setVisible(visible)
  for viewIndex in range(lm.tableViewCount):
    lm.tableWidget(viewIndex).tableController().setVisible(visible)
  for viewIndex in range(lm.plotViewCount):
    lm.plotWidget(viewIndex).plotController().setVisible(visible)

def forceRenderAllViews():
  """Force rendering of all views"""
  import slicer
  lm = slicer.app.layoutManager()
  for viewIndex in range(lm.threeDViewCount):
    lm.threeDWidget(viewIndex).threeDView().forceRender()
  for sliceViewName in lm.sliceViewNames():
    lm.sliceWidget(sliceViewName).sliceView().forceRender()
  for viewIndex in range(lm.tableViewCount):
    lm.tableWidget(viewIndex).tableView().repaint()
  for viewIndex in range(lm.plotViewCount):
    lm.plotWidget(viewIndex).plotView().repaint()

#
# IO
#

def loadNodeFromFile(filename, filetype, properties={}, returnNode=False):
  """Load node into the scene from a file.

  :param filename: full path of the file to load.
  :param filetype: specifies the file type, which determines which IO class will load the file.
  :param properties: map containing additional parameters for the loading.
  :param returnNode: Deprecated. If set to true then the method returns status flag and node
    instead of signalling error by throwing an exception.
  :return: loaded node (if multiple nodes are loaded then a list of nodes).
    If returnNode is True then a status flag and loaded node are returned.
  """
  from slicer import app
  from vtk import vtkCollection
  properties['fileName'] = filename

  loadedNodesCollection = vtkCollection()
  success = app.coreIOManager().loadNodes(filetype, properties, loadedNodesCollection)
  loadedNode = loadedNodesCollection.GetItemAsObject(0) if loadedNodesCollection.GetNumberOfItems() > 0 else None

  # Deprecated way of returning status and node
  if returnNode:
    import logging
    logging.warning("loadNodeFromFile `returnNode` argument is deprecated. Loaded node is now returned directly if `returnNode` is not specified.")
    return success, loadedNode

  if not success:
    errorMessage = "Failed to load node from file: " + str(filename)
    raise RuntimeError(errorMessage)

  return loadedNode

def loadNodesFromFile(filename, filetype, properties={}, returnNode=False):
  """Load nodes into the scene from a file.

  It differs from `loadNodeFromFile` in that it returns loaded node(s) in an iterator.

  :param filename: full path of the file to load.
  :param filetype: specifies the file type, which determines which IO class will load the file.
  :param properties: map containing additional parameters for the loading.
  :return: loaded node(s) in an iterator object.
  """
  from slicer import app
  from vtk import vtkCollection
  properties['fileName'] = filename

  loadedNodesCollection = vtkCollection()
  success = app.coreIOManager().loadNodes(filetype, properties, loadedNodesCollection)
  if not success:
    errorMessage = "Failed to load nodes from file: " + str(filename)
    raise RuntimeError(errorMessage)

  return iter(loadedNodesCollection)

def loadColorTable(filename, returnNode=False):
  """Load node from file.

  :param filename: full path of the file to load.
  :param returnNode: Deprecated.
  :return: loaded node (if multiple nodes are loaded then a list of nodes).
    If returnNode is True then a status flag and loaded node are returned.
  """
  return loadNodeFromFile(filename, 'ColorTableFile', {}, returnNode)

def loadFiberBundle(filename, returnNode=False):
  """Load node from file.

  :param filename: full path of the file to load.
  :param returnNode: Deprecated.
  :return: loaded node (if multiple nodes are loaded then a list of nodes).
    If returnNode is True then a status flag and loaded node are returned.
  """
  return loadNodeFromFile(filename, 'FiberBundleFile', {}, returnNode)

def loadFiducialList(filename, returnNode=False):
  """Load node from file.

  :param filename: full path of the file to load.
  :param returnNode: Deprecated.
  :return: loaded node (if multiple nodes are loaded then a list of nodes).
    If returnNode is True then a status flag and loaded node are returned.
  """
  return loadNodeFromFile(filename, 'FiducialListFile', {}, returnNode)

def loadAnnotationFiducial(filename, returnNode=False):
  """Load node from file.

  :param filename: full path of the file to load.
  :param returnNode: Deprecated.
  :return: loaded node (if multiple nodes are loaded then a list of nodes).
    If returnNode is True then a status flag and loaded node are returned.
  """
  return loadNodeFromFile(filename, 'AnnotationFile', {'fiducial': 1}, returnNode)

def loadAnnotationRuler(filename, returnNode=False):
  """Load node from file.

  :param filename: full path of the file to load.
  :param returnNode: Deprecated.
  :return: loaded node (if multiple nodes are loaded then a list of nodes).
    If returnNode is True then a status flag and loaded node are returned.
  """
  return loadNodeFromFile(filename, 'AnnotationFile', {'ruler': 1}, returnNode)

def loadAnnotationROI(filename, returnNode=False):
  """Load node from file.

  :param filename: full path of the file to load.
  :param returnNode: Deprecated.
  :return: loaded node (if multiple nodes are loaded then a list of nodes).
    If returnNode is True then a status flag and loaded node are returned.
  """
  return loadNodeFromFile(filename, 'AnnotationFile', {'roi': 1}, returnNode)

def loadMarkupsFiducialList(filename, returnNode=False):
  """Deprecated. Use loadMarkups function instead.
  """
  if returnNode:
    return loadMarkups(filename)
  else:
    node = loadMarkups(filename)
    return [node is not None, node]

def loadMarkupsCurve(filename):
  """Deprecated. Use loadMarkups function instead.
  """
  return loadMarkups(filename)

def loadMarkupsClosedCurve(filename):
  """Deprecated. Use loadMarkups function instead.
  """
  return loadMarkups(filename)

def loadMarkups(filename):
  """Load node from file.

  :param filename: full path of the file to load.
  :return: loaded node (if multiple nodes are loaded then a list of nodes).
  """
  return loadNodeFromFile(filename, 'MarkupsFile')

def loadModel(filename, returnNode=False):
  """Load node from file.

  :param filename: full path of the file to load.
  :param returnNode: Deprecated.
  :return: loaded node (if multiple nodes are loaded then a list of nodes).
    If returnNode is True then a status flag and loaded node are returned.
  """
  return loadNodeFromFile(filename, 'ModelFile', {}, returnNode)

def loadScalarOverlay(filename, modelNodeID, returnNode=False):
  """Load node from file.

  :param filename: full path of the file to load.
  :param returnNode: Deprecated.
  :return: loaded node (if multiple nodes are loaded then a list of nodes).
    If returnNode is True then a status flag and loaded node are returned.
  """
  return loadNodeFromFile(filename, 'ScalarOverlayFile', {'modelNodeId': modelNodeID }, returnNode)

def loadSegmentation(filename, returnNode=False):
  """Load node from file.

  :param filename: full path of the file to load.
  :param returnNode: Deprecated.
  :return: loaded node (if multiple nodes are loaded then a list of nodes).
    If returnNode is True then a status flag and loaded node are returned.
  """
  return loadNodeFromFile(filename, 'SegmentationFile', {}, returnNode)

def loadTransform(filename, returnNode=False):
  """Load node from file.

  :param filename: full path of the file to load.
  :param returnNode: Deprecated.
  :return: loaded node (if multiple nodes are loaded then a list of nodes).
    If returnNode is True then a status flag and loaded node are returned.
  """
  return loadNodeFromFile(filename, 'TransformFile', {}, returnNode)

def loadTable(filename):
  """Load table node from file.
  :param filename: full path of the file to load.
  :return: loaded table node
  """
  return loadNodeFromFile(filename, 'TableFile')

def loadLabelVolume(filename, properties={}, returnNode=False):
  """Load node from file.

  :param filename: full path of the file to load.
  :param returnNode: Deprecated.
  :return: loaded node (if multiple nodes are loaded then a list of nodes).
    If returnNode is True then a status flag and loaded node are returned.
  """
  properties['labelmap'] = True
  return loadNodeFromFile(filename, 'VolumeFile', properties, returnNode)

def loadShaderProperty(filename, returnNode=False):
  """Load node from file.

  :param filename: full path of the file to load.
  :param returnNode: Deprecated.
  :return: loaded node (if multiple nodes are loaded then a list of nodes).
    If returnNode is True then a status flag and loaded node are returned.
  """
  return loadNodeFromFile(filename, 'ShaderPropertyFile', {}, returnNode)

def loadText(filename):
  """Load node from file.

  :param filename: full path of the text file to load.
  :return: loaded text node.
  """
  return loadNodeFromFile(filename, 'TextFile')

def loadVolume(filename, properties={}, returnNode=False):
  """Load node from file.

  :param filename: full path of the file to load.
  :param properties:
    - name: this name will be used as node name for the loaded volume
    - labelmap: interpret volume as labelmap
    - singleFile: ignore all other files in the directory
    - center: ignore image position
    - discardOrientation: ignore image axis directions
    - autoWindowLevel: compute window/level automatically
    - show: display volume in slice viewers after loading is completed
    - fileNames: list of filenames to load the volume from
  :param returnNode: Deprecated.
  :return: loaded node (if multiple nodes are loaded then a list of nodes).
    If returnNode is True then a status flag and loaded node are returned.
  """
  filetype = 'VolumeFile'
  return loadNodeFromFile(filename, filetype, properties, returnNode)

def loadSequence(filename, properties={}):
  """Load sequence (4D data set) from file.

  :param filename: full path of the file to load.
  :param properties:
    - name: this name will be used as node name for the loaded volume
    - show: display volume in slice viewers after loading is completed
    - colorNodeID: color node to set in the proxy nodes's display node
  :return: loaded sequence node.
  """
  filetype = 'SequenceFile'
  return loadNodeFromFile(filename, filetype, properties)

def loadScene(filename, properties={}):
  """Load node from file.

  :param filename: full path of the file to load.
  :param returnNode: Deprecated.
  :return: loaded node (if multiple nodes are loaded then a list of nodes).
    If returnNode is True then a status flag and loaded node are returned.
  """
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

def openAddMarkupsDialog():
  from slicer import app
  return app.coreIOManager().openAddMarkupsDialog()

def openAddFiberBundleDialog():
  from slicer import app
  return app.coreIOManager().openAddFiberBundleDialog()

def openAddShaderPropertyDialog():
  from slicer import app, qSlicerFileDialog
  return app.coreIOManager().openDialog('ShaderPropertyFile', qSlicerFileDialog.Read)

def openSaveDataDialog():
  from slicer import app
  return app.coreIOManager().openSaveDataDialog()

def saveNode(node, filename, properties={}):
  """Save 'node' data into 'filename'.

  It is the user responsibility to provide the appropriate file extension.

  User has also the possibility to overwrite the fileType internally retrieved using
  method 'qSlicerCoreIOManager::fileWriterFileType(vtkObject*)'. This can be done
  by specifying a 'fileType'attribute to the optional 'properties' dictionary.
  """
  from slicer import app
  properties["nodeID"] = node.GetID()
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
  if not isinstance(module, str):
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
  module = app.moduleManager().module(moduleName)
  if not module:
    import sys
    print("Could not find module with name '%s" % moduleName, file=sys.stderr)
    return None
  return module

def getModuleGui(module):
  if isinstance(module, str):
    module = getModule(module)
  widgetRepr = module.widgetRepresentation()
  if not widgetRepr:
    import sys
    print("Could not find module widget representation with name '%s" % module.name, file=sys.stderr)
  return widgetRepr

def getNewModuleGui(module):
  if isinstance(module, str):
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

  The function performs the following:

  * Ensure ``sys.path`` includes the module path and use ``imp.load_module``
    to load the associated script.
  * For the current module widget representation:

    * Hide all children widgets
    * Call ``cleanup()`` function and disconnect ``ScriptedLoadableModuleWidget_onModuleAboutToBeUnloaded``
    * Remove layout items

  * Instantiate new widget representation
  * Call ``setup()`` function
  * Update ``slicer.modules.<moduleName>Widget`` attribute
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

  # if the module widget has been instantiated, call cleanup function and
  # disconnect "_onModuleAboutToBeUnloaded" (this avoids double-cleanup on
  # application exit)
  if hasattr(slicer.modules, widgetName):
    widget = getattr(slicer.modules, widgetName)
    widget.cleanup()

    if hasattr(widget, '_onModuleAboutToBeUnloaded'):
      slicer.app.moduleManager().disconnect('moduleAboutToBeUnloaded(QString)', widget._onModuleAboutToBeUnloaded)

  # remove layout items (remaining spacer items would add space above the widget)
  items = []
  for itemIndex in range(parent.layout().count()):
    items.append(parent.layout().itemAt(itemIndex))
  for item in items:
    parent.layout().removeItem(item)

  # create new widget inside existing parent
  widget = eval('reloaded_module.%s(parent)' % widgetName)
  widget.setup()
  setattr(slicer.modules, widgetName, widget)

  return reloaded_module

def setModulePanelTitleVisible(visible):
  """Show/hide module panel title bar at the top of module panel.
  If the title bar is not visible then it is not possible to drag and dock the
  module panel to a different location."""
  modulePanelDockWidget = mainWindow().findChildren('QDockWidget','PanelDockWidget')[0]
  if visible:
    modulePanelDockWidget.setTitleBarWidget(None)
  else:
    import qt
    modulePanelDockWidget.setTitleBarWidget(qt.QWidget(modulePanelDockWidget))

def setApplicationLogoVisible(visible):
  """Show/hide application logo at the top of module panel."""
  widget = findChild(mainWindow(), "LogoLabel")
  widget.setVisible(visible)

def setModuleHelpSectionVisible(visible):
  """Show/hide Help section at the top of module panel."""
  modulePanel = findChild(mainWindow(), "ModulePanel")
  modulePanel.helpAndAcknowledgmentVisible = visible

def setDataProbeVisible(visible):
  """Show/hide Data probe at the bottom of module panel."""
  widget = findChild(mainWindow(), "DataProbeCollapsibleWidget")
  widget.setVisible(visible)

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

  :raises MRMLNodeNotFoundException: if no node is found
   that matches the specified pattern.
  """
  nodes = getNodes(pattern, scene)
  if not nodes:
    raise MRMLNodeNotFoundException("could not find nodes in the scene by name or id '%s'" % (pattern if (isinstance(pattern, str)) else ""))
  return list(nodes.values())[index]

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

class NodeModify(object):
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
  MRML node that matches the pattern.

  .. warning::

    Meant to be used in the python console for quick debugging/testing.

  More specific API should be used in scripts to be sure you get exactly
  what you want, such as :py:meth:`arrayFromVolume`, :py:meth:`arrayFromModelPoints`,
  and :py:meth:`arrayFromGridTransform`.
  """
  node = getNode(pattern=pattern, index=index)
  import slicer
  if isinstance(node, slicer.vtkMRMLVolumeNode):
    return arrayFromVolume(node)
  elif isinstance(node, slicer.vtkMRMLModelNode):
    return arrayFromModelPoints(node)
  elif isinstance(node, slicer.vtkMRMLGridTransformNode):
    return arrayFromGridTransform(node)
  elif isinstance(node, slicer.vtkMRMLMarkupsNode):
    return arrayFromMarkupsControlPoints(node)
  elif isinstance(node, slicer.vtkMRMLTransformNode):
    return arrayFromTransformMatrix(node)

  # TODO: accessors for other node types: polydata (verts, polys...), colors
  raise RuntimeError("Cannot get node "+node.GetID()+" as array")

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
  Point coordinates can be modified by modifying the numpy array.
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

def arrayFromModelPointDataModified(modelNode, arrayName):
  """Indicate that modification of a numpy array returned by :py:meth:`arrayFromModelPoints` has been completed."""
  arrayVtk = modelNode.GetPolyData().GetPointData().GetArray(arrayName)
  arrayVtk.Modified()

def arrayFromModelPolyIds(modelNode):
  """Return poly id array of a model node as numpy array.

  These ids are the following format:
  [ n(0), i(0,0), i(0,1), ... i(0,n(00),..., n(j), i(j,0), ... i(j,n(j))...]
  where n(j) is the number of vertices in polygon j
  and i(j,k) is the index into the vertex array for vertex k of poly j.

  As described here:
  https://vtk.org/wp-content/uploads/2015/04/file-formats.pdf

  Typically in Slicer n(j) will always be 3 because a model node's
  polygons will be triangles.

  .. warning:: Important: memory area of the returned array is managed by VTK,
    therefore values in the array may be changed, but the array must not be reallocated.
    See :py:meth:`arrayFromVolume` for details.
  """
  import vtk.util.numpy_support
  arrayVtk = modelNode.GetPolyData().GetPolys().GetData()
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

def arrayFromVTKMatrix(vmatrix):
  """Return vtkMatrix4x4 or vtkMatrix3x3 elements as numpy array.
  The returned array is just a copy and so any modification in the array will not affect the input matrix.
  To set VTK matrix from a numpy array, use :py:meth:`vtkMatrixFromArray` or
  :py:meth:`updateVTKMatrixFromArray`.
  """
  from vtk import vtkMatrix4x4
  from vtk import vtkMatrix3x3
  import numpy as np
  if isinstance(vmatrix, vtkMatrix4x4):
    matrixSize = 4
  elif isinstance(vmatrix, vtkMatrix3x3):
    matrixSize = 3
  else:
    raise RuntimeError("Input must be vtk.vtkMatrix3x3 or vtk.vtkMatrix4x4")
  narray = np.eye(matrixSize)
  vmatrix.DeepCopy(narray.ravel(), vmatrix)
  return narray

def vtkMatrixFromArray(narray):
  """Create VTK matrix from a 3x3 or 4x4 numpy array.
  :param narray: input numpy array
  The returned matrix is just a copy and so any modification in the array will not affect the output matrix.
  To set numpy array from VTK matrix, use :py:meth:`arrayFromVTKMatrix`.
  """
  from vtk import vtkMatrix4x4
  from vtk import vtkMatrix3x3
  narrayshape = narray.shape
  if narrayshape == (4,4):
    vmatrix = vtkMatrix4x4()
    updateVTKMatrixFromArray(vmatrix, narray)
    return vmatrix
  elif narrayshape == (3,3):
    vmatrix = vtkMatrix3x3()
    updateVTKMatrixFromArray(vmatrix, narray)
    return vmatrix
  else:
    raise RuntimeError("Unsupported numpy array shape: "+str(narrayshape)+" expected (4,4)")

def updateVTKMatrixFromArray(vmatrix, narray):
  """Update VTK matrix values from a numpy array.
  :param vmatrix: VTK matrix (vtkMatrix4x4 or vtkMatrix3x3) that will be update
  :param narray: input numpy array
  To set numpy array from VTK matrix, use :py:meth:`arrayFromVTKMatrix`.
  """
  from vtk import vtkMatrix4x4
  from vtk import vtkMatrix3x3
  if isinstance(vmatrix, vtkMatrix4x4):
    matrixSize = 4
  elif isinstance(vmatrix, vtkMatrix3x3):
    matrixSize = 3
  else:
    raise RuntimeError("Output vmatrix must be vtk.vtkMatrix3x3 or vtk.vtkMatrix4x4")
  if narray.shape != (matrixSize, matrixSize):
    raise RuntimeError("Input narray size must match output vmatrix size ({0}x{0})".format(matrixSize))
  vmatrix.DeepCopy(narray.ravel())

def arrayFromTransformMatrix(transformNode, toWorld=False):
  """Return 4x4 transformation matrix as numpy array.

  :param toWorld: if set to True then the transform to world coordinate system is returned
    (effect of parent transform to the node is applied), otherwise transform to parent transform is returned.

  The returned array is just a copy and so any modification in the array will not affect the transform node.

  To set transformation matrix from a numpy array, use :py:meth:`updateTransformMatrixFromArray`.
  """
  import numpy as np
  from vtk import vtkMatrix4x4
  vmatrix = vtkMatrix4x4()
  if toWorld:
    success = transformNode.GetMatrixTransformToWorld(vmatrix)
  else:
    success = transformNode.GetMatrixTransformToParent(vmatrix)
  if not success:
    raise RuntimeError("Failed to get transformation matrix from node "+transformNode.GetID())
  return arrayFromVTKMatrix(vmatrix)

def updateTransformMatrixFromArray(transformNode, narray, toWorld = False):
  """Set transformation matrix from a numpy array of size 4x4 (toParent).

  :param world: if set to True then the transform will be set so that transform
    to world matrix will be equal to narray; otherwise transform to parent will be
    set as narray.
  """
  import numpy as np
  from vtk import vtkMatrix4x4
  narrayshape = narray.shape
  if narrayshape != (4,4):
    raise RuntimeError("Unsupported numpy array shape: "+str(narrayshape)+" expected (4,4)")
  if toWorld and transformNode.GetParentTransformNode():
    # thisToParent = worldToParent * thisToWorld = inv(parentToWorld) * toWorld
    narrayParentToWorld = arrayFromTransformMatrix(transformNode.GetParentTransformNode())
    thisToParent = np.dot(np.linalg.inv(narrayParentToWorld), narray)
    updateTransformMatrixFromArray(transformNode, thisToParent, toWorld = False)
  else:
    vmatrix = vtkMatrix4x4()
    updateVTKMatrixFromArray(vmatrix, narray)
    transformNode.SetMatrixTransformToParent(vmatrix)

def arrayFromGridTransformModified(gridTransformNode):
  """Indicate that modification of a numpy array returned by :py:meth:`arrayFromModelPoints` has been completed."""
  transformGrid = gridTransformNode.GetTransformFromParent()
  displacementGrid = transformGrid.GetDisplacementGrid()
  displacementGrid.GetPointData().GetScalars().Modified()
  displacementGrid.Modified()

def arrayFromSegment(segmentationNode, segmentId):
  """
  """
  import logging
  logging.warning("arrayFromSegment is deprecated! Binary labelmap representation may be shared between multiple segments!"
                  " Use arrayFromSegmentBinaryLabelmap to access a copy of the binary labelmap that will not modify the original labelmap."
                  " Use arrayFromSegmentInternalBinaryLabelmap to access a modifiable internal lablemap representation that may be shared"
                  " between multiple segments.")
  return arrayFromSegmentBinaryLabelmap(segmentationNode, segmentId)

def arrayFromSegmentInternalBinaryLabelmap(segmentationNode, segmentId):
  """Return voxel array of a segment's binary labelmap representation as numpy array.
  Voxels values are not copied.

  The labelmap containing the specified segment may be a shared labelmap containing multiple segments.

  To get and modify the array for a single segment, calling::

    segmentationNode->GetSegmentation()->SeparateSegment(segmentId)

  will transfer the segment from a shared labelmap into a new layer.

  Layers can be merged by calling::

    segmentationNode->GetSegmentation()->CollapseBinaryLabelmaps()

  If binary labelmap is the master representation then voxel values in the volume node can be modified
  by changing values in the numpy array. After all modifications has been completed, call::

    segmentationNode.GetSegmentation().GetSegment(segmentID).Modified()

  .. warning:: Important: memory area of the returned array is managed by VTK,
    therefore values in the array may be changed, but the array must not be reallocated.
    See :py:meth:`arrayFromVolume` for details.
  """
  vimage = segmentationNode.GetBinaryLabelmapInternalRepresentation(segmentId)
  nshape = tuple(reversed(vimage.GetDimensions()))
  import vtk.util.numpy_support
  narray = vtk.util.numpy_support.vtk_to_numpy(vimage.GetPointData().GetScalars()).reshape(nshape)
  return narray

def arrayFromSegmentBinaryLabelmap(segmentationNode, segmentId):
  """Return voxel array of a segment's binary labelmap representation as numpy array.

  Voxels values are copied.

  If binary labelmap is the master representation then voxel values in the volume node can be modified
  by changing values in the numpy array.

  After all modifications have been completed, call::

    segmentationNode.GetSegmentation().GetSegment(segmentID).Modified()

  .. warning:: Important: memory area of the returned array is managed by VTK,
    therefore values in the array may be changed, but the array must not be reallocated.
    See :py:meth:`arrayFromVolume` for details.
  """
  import slicer
  vimage = slicer.vtkOrientedImageData()
  segmentationNode.GetBinaryLabelmapRepresentation(segmentId, vimage)
  nshape = tuple(reversed(vimage.GetDimensions()))
  import vtk.util.numpy_support
  narray = vtk.util.numpy_support.vtk_to_numpy(vimage.GetPointData().GetScalars()).reshape(nshape)
  return narray

def arrayFromMarkupsControlPoints(markupsNode, world = False):
  """Return control point positions of a markups node as rows in a numpy array (of size Nx3).

  :param world: if set to True then the control points coordinates are returned in world coordinate system
    (effect of parent transform to the node is applied).

  The returned array is just a copy and so any modification in the array will not affect the markup node.

  To modify markup control points based on a numpy array, use :py:meth:`updateMarkupsControlPointsFromArray`.
  """
  numberOfControlPoints = markupsNode.GetNumberOfControlPoints()
  import numpy as np
  narray = np.zeros([numberOfControlPoints, 3])
  for controlPointIndex in range(numberOfControlPoints):
    if world:
      markupsNode.GetNthControlPointPositionWorld(controlPointIndex, narray[controlPointIndex,:])
    else:
      markupsNode.GetNthControlPointPosition(controlPointIndex, narray[controlPointIndex,:])
  return narray

def updateMarkupsControlPointsFromArray(markupsNode, narray, world = False):
  """Sets control point positions in a markups node from a numpy array of size Nx3.

  :param world: if set to True then the control point coordinates are expected in world coordinate system.

  All previous content of the node is deleted.
  """
  narrayshape = narray.shape
  if narrayshape == (0,):
    markupsNode.RemoveAllControlPoints()
    return
  if len(narrayshape) != 2 or narrayshape[1] != 3:
    raise RuntimeError("Unsupported numpy array shape: "+str(narrayshape)+" expected (N,3)")
  numberOfControlPoints = narrayshape[0]
  oldNumberOfControlPoints = markupsNode.GetNumberOfControlPoints()
  # Update existing control points
  for controlPointIndex in range(min(numberOfControlPoints, oldNumberOfControlPoints)):
    if world:
      markupsNode.SetNthControlPointPositionWorldFromArray(controlPointIndex, narray[controlPointIndex,:])
    else:
      markupsNode.SetNthControlPointPositionFromArray(controlPointIndex, narray[controlPointIndex,:])
  if numberOfControlPoints >= oldNumberOfControlPoints:
    # Add new points to the markup node
    from vtk import vtkVector3d
    for controlPointIndex in range(oldNumberOfControlPoints, numberOfControlPoints):
      if world:
        markupsNode.AddControlPointWorld(vtkVector3d(narray[controlPointIndex,:]))
      else:
        markupsNode.AddControlPoint(vtkVector3d(narray[controlPointIndex,:]))
  else:
    # Remove extra point from the markup node
    for controlPointIndex in range(oldNumberOfControlPoints, numberOfControlPoints, -1):
      markupsNode.RemoveNthControlPoint(controlPointIndex-1)

def arrayFromMarkupsCurvePoints(markupsNode, world = False):
  """Return interpolated curve point positions of a markups node as rows in a numpy array (of size Nx3).

  :param world: if set to True then the point coordinates are returned in world coordinate system
    (effect of parent transform to the node is applied).

  The returned array is just a copy and so any modification in the array will not affect the markup node.
  """
  import numpy as np
  import vtk.util.numpy_support
  if world:
    pointData = markupsNode.GetCurvePointsWorld().GetData()
  else:
    pointData = markupsNode.GetCurvePoints().GetData()
  narray = vtk.util.numpy_support.vtk_to_numpy(pointData)
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

  # Volumes with "long long" scalar type are not rendered corectly.
  # Probably this could be fixed in VTK or Slicer but for now just reject it.
  if vtype == vtk.VTK_LONG_LONG:
    raise RuntimeError("Unsupported numpy array type: long long")

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

def addVolumeFromArray(narray, ijkToRAS=None, name=None, nodeClassName=None):
  """Create a new volume node from content of a numpy array and add it to the scene.
  Voxels values are deep-copied, therefore if the numpy array
  is modified after calling this method, voxel values in the volume node will not change.

  :param narray: numpy array containing volume voxels.
  :param ijkToRAS: 4x4 numpy array or vtk.vtkMatrix4x4 that defines mapping from IJK to RAS coordinate system (specifying origin, spacing, directions)
  :param name: volume node name
  :param nodeClassName: type of created volume, default: ``vtkMRMLScalarVolumeNode``.
    Use ``vtkMRMLLabelMapVolumeNode`` for labelmap volume, ``vtkMRMLVectorVolumeNode`` for vector volume.
  :return: created new volume node

  Example::

    # create zero-filled volume
    import numpy as np
    volumeNode = slicer.util.addVolumeFromArray(np.zeros((30, 40, 50)))

  Example::

    # create labelmap volume filled with voxel value of 120
    import numpy as np
    volumeNode = slicer.util.addVolumeFromArray(np.ones((30, 40, 50), 'int8') * 120,
      np.diag([0.2, 0.2, 0.5, 1.0]), nodeClassName="vtkMRMLLabelMapVolumeNode")

  """

  import slicer
  from vtk import vtkMatrix4x4
  import numpy as np

  if name is None:
    name = ""
  if nodeClassName is None:
    nodeClassName = "vtkMRMLScalarVolumeNode"

  volumeNode = slicer.mrmlScene.AddNewNodeByClass(nodeClassName, name)
  if ijkToRAS is not None:
    if not isinstance(ijkToRAS, vtkMatrix4x4):
      ijkToRAS = vtkMatrixFromArray(ijkToRAS)
    volumeNode.SetIJKToRASMatrix(ijkToRAS)
  updateVolumeFromArray(volumeNode, narray)
  volumeNode.CreateDefaultDisplayNodes()

  return volumeNode

def arrayFromTableColumn(tableNode, columnName):
  """Return values of a table node's column as numpy array.
  Values can be modified by modifying the numpy array.
  After all modifications has been completed, call :py:meth:`arrayFromTableColumnModified`.

  .. warning:: Important: memory area of the returned array is managed by VTK,
    therefore values in the array may be changed, but the array must not be reallocated.
    See :py:meth:`arrayFromVolume` for details.
  """
  import vtk.util.numpy_support
  columnData = tableNode.GetTable().GetColumnByName(columnName)
  narray = vtk.util.numpy_support.vtk_to_numpy(columnData)
  return narray

def arrayFromTableColumnModified(tableNode, columnName):
  """Indicate that modification of a numpy array returned by :py:meth:`arrayFromModelPoints` has been completed."""
  import vtk.util.numpy_support
  columnData = tableNode.GetTable().GetColumnByName(columnName)
  columnData.Modified()
  tableNode.GetTable().Modified()

def updateTableFromArray(tableNode, narrays, columnNames=None):
  """Set values in a table node from a numpy array.

  columnNames may contain a string or list of strings that will be used as column name(s).
  Values are copied, therefore if the numpy array  is modified after calling this method,
  values in the table node will not change.
  All previous content of the table is deleted.

  Example::

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
  if isinstance(narrays, np.ndarray) and len(narrays.shape) == 1:
    ncolumns = [narrays]
  elif isinstance(narrays, np.ndarray) and len(narrays.shape) == 2:
    ncolumns = narrays.T
  elif isinstance(narrays, tuple) or isinstance(narrays, list):
    ncolumns = narrays
  else:
    raise ValueError('Expected narrays is a numpy ndarray, or tuple or list of numpy ndarrays, got %s instead.' % (str(type(narrays))))
  tableNode.RemoveAllColumns()
  # Convert single string to a single-element string list
  if columnNames is None:
    columnNames = []
  if isinstance(columnNames, str):
    columnNames = [columnNames]
  for columnIndex, ncolumn in enumerate(ncolumns):
    vcolumn = vtk.util.numpy_support.numpy_to_vtk(num_array=ncolumn.ravel(),deep=True,array_type=vtk.VTK_FLOAT)
    if (columnNames is not None) and (columnIndex < len(columnNames)):
      vcolumn.SetName(columnNames[columnIndex])
    tableNode.AddColumn(vcolumn)
  return tableNode

def dataframeFromTable(tableNode):
  """Convert table node content to pandas dataframe.
  Table content is copied. Therefore, changes in table node do not affect the dataframe,
  and dataframe changes do not affect the original table node.
  """
  try:
    # Suppress "lzma compression not available" UserWarning when loading pandas
    import warnings
    with warnings.catch_warnings():
      warnings.simplefilter(action='ignore', category=UserWarning)
      import pandas as pd
  except ImportError:
    raise ImportError("Failed to convert to pandas dataframe. Please install pandas by running `slicer.util.pip_install('pandas')`")
  dataframe = pd.DataFrame()
  vtable = tableNode.GetTable()
  for columnIndex in range (vtable.GetNumberOfColumns()):
    vcolumn = vtable.GetColumn(columnIndex)
    column = []
    numberOfComponents = vcolumn.GetNumberOfComponents()
    if numberOfComponents==1:
      # most common, simple case
      for rowIndex in range(vcolumn.GetNumberOfValues()):
        column.append(vcolumn.GetValue(rowIndex))
    else:
      # rare case: column contains multiple components
      valueIndex = 0
      for rowIndex in range(vcolumn.GetNumberOfTuples()):
        item = []
        for componentIndex in range(numberOfComponents):
          item.append(vcolumn.GetValue(valueIndex))
          valueIndex += 1
        column.append(item)
    dataframe[vcolumn.GetName()] = column
  return dataframe

def dataframeFromMarkups(markupsNode):
  """Convert table node content to pandas dataframe.
  Table content is copied. Therefore, changes in table node do not affect the dataframe,
  and dataframe changes do not affect the original table node.
  """
  try:
    # Suppress "lzma compression not available" UserWarning when loading pandas
    import warnings
    with warnings.catch_warnings():
      warnings.simplefilter(action='ignore', category=UserWarning)
      import pandas as pd
  except ImportError:
    raise ImportError("Failed to convert to pandas dataframe. Please install pandas by running `slicer.util.pip_install('pandas')`")

  label = []
  description = []
  positionWorldR = []
  positionWorldA = []
  positionWorldS = []
  selected = []
  visible = []

  numberOfControlPoints = markupsNode.GetNumberOfControlPoints()
  for controlPointIndex in range(numberOfControlPoints):
    label.append(markupsNode.GetNthControlPointLabel(controlPointIndex))
    description.append(markupsNode.GetNthControlPointDescription(controlPointIndex))
    p=[0,0,0]
    markupsNode.GetNthControlPointPositionWorld(controlPointIndex, p)
    positionWorldR.append(p[0])
    positionWorldA.append(p[1])
    positionWorldS.append(p[2])
    selected.append(markupsNode.GetNthControlPointSelected(controlPointIndex) != 0)
    visible.append(markupsNode.GetNthControlPointVisibility(controlPointIndex) != 0)

  dataframe = pd.DataFrame({
    'label': label,
    'position.R': positionWorldR,
    'position.A': positionWorldA,
    'position.S': positionWorldS,
    'selected': selected,
    'visible': visible,
    'description': description})
  return dataframe

#
# VTK
#

class VTKObservationMixin(object):
  def __init__(self):
    super(VTKObservationMixin, self).__init__()
    self.Observations = []

  def removeObservers(self, method=None):
    for o, e, m, g, t, p in list(self.Observations):
      if method == m or method is None:
        o.RemoveObserver(t)
        self.Observations.remove([o, e, m, g, t, p])

  def addObserver(self, object, event, method, group = 'none', priority = 0.0):
    if self.hasObserver(object, event, method):
      print('already has observer')
      return
    tag = object.AddObserver(event, method, priority)
    self.Observations.append([object, event, method, group, tag, priority])

  def removeObserver(self, object, event, method):
    for o, e, m, g, t, p in self.Observations:
      if o == object and e == event and m == method:
        o.RemoveObserver(t)
        self.Observations.remove([o, e, m, g, t, p])

  def hasObserver(self, object, event, method):
    for o, e, m, g, t, p in self.Observations:
      if o == object and e == event and m == method:
        return True
    return False

  def observer(self, event, method):
    for o, e, m, g, t, p in self.Observations:
      if e == event and m == method:
        return o
    return None

def toVTKString(text):
  """This method is deprecated. It converted unicode string into VTK string, but
  since now VTK assumes that all strings are in UTF-8 and all strings in Slicer are UTF-8, too,
  conversion is no longer necessary.
  The method is only kept for backward compatibility and will be removed in the future.
  """
  import logging
  logging.warning("toVTKString is deprecated! Conversion is no longer necessary.")
  return text

def toLatin1String(text):
  """Convert string to latin1 encoding.
  """
  vtkStr = ""
  for c in text:
    try:
      cc = c.encode("latin1", "ignore").decode()
    except (UnicodeDecodeError):
      cc = "?"
    vtkStr = vtkStr + cc
  return vtkStr

#
# File Utilities
#

def tempDirectory(key='__SlicerTemp__',tempDir=None,includeDateTime=True):
  """Come up with a unique directory name in the temp dir and make it and return it
  # TODO: switch to QTemporaryDir in Qt5.
  Note: this directory is not automatically cleaned up
  """
  import qt, slicer
  if not tempDir:
    tempDir = qt.QDir(slicer.app.temporaryPath)
  if includeDateTime:
    tempDirName = key + qt.QDateTime().currentDateTime().toString("yyyy-MM-dd_hh+mm+ss.zzz")
  else:
    tempDirName = key
  fileInfo = qt.QFileInfo(qt.QDir(tempDir), tempDirName)
  dirPath = fileInfo.absoluteFilePath()
  qt.QDir().mkpath(dirPath)
  return dirPath

def delayDisplay(message, autoCloseMsec=1000):
  """Display an information message in a popup window for a short time.

  If ``autoCloseMsec < 0`` then the window is not closed until the user clicks on it

  If ``0 <= autoCloseMsec < 400`` then only ``slicer.app.processEvents()`` is called.

  If ``autoCloseMsec >= 400`` then the window is closed after waiting for autoCloseMsec milliseconds
  """
  import qt, slicer
  import logging
  logging.info(message)
  if 0 <= autoCloseMsec < 400:
    slicer.app.processEvents()
    return
  messagePopup = qt.QDialog()
  layout = qt.QVBoxLayout()
  messagePopup.setLayout(layout)
  label = qt.QLabel(message,messagePopup)
  layout.addWidget(label)
  if autoCloseMsec >= 0:
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

  For example::

    slicer.util.messageBox("Some message", dontShowAgainSettingsKey = "MainWindow/DontShowSomeMessage")
  """
  import qt
  import ctk
  mbox = ctk.ctkMessageBox(parent if parent else mainWindow())
  mbox.text = text
  for key, value in kwargs.items():
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
  """Display a modal QProgressDialog.

  Go to `QProgressDialog documentation <http://pyqt.sourceforge.net/Docs/PyQt4/qprogressdialog.html>`_ to
  learn about the available keyword arguments.

  Examples::

    # Prevent progress dialog from automatically closing
    progressbar = createProgressIndicator(autoClose=False)

    # Update progress value
    progressbar.value = 50

    # Update label text
    progressbar.labelText = "processing XYZ"
  """
  import qt
  progressIndicator = qt.QProgressDialog(parent if parent else mainWindow())
  progressIndicator.minimumDuration = 0
  progressIndicator.maximum = maximum
  progressIndicator.value = value
  progressIndicator.windowTitle = windowTitle
  progressIndicator.labelText = labelText
  for key, value in kwargs.items():
    if hasattr(progressIndicator, key):
      setattr(progressIndicator, key, value)
  return progressIndicator

def toBool(value):
  """Convert any type of value to a boolean.

  The function uses the following heuristic:

  1. If the value can be converted to an integer, the integer is then
     converted to a boolean.
  2. If the value is a string, return True if it is equal to 'true'. False otherwise.
     Note that the comparison is case insensitive.
  3. If the value is neither an integer or a string, the bool() function is applied.

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
    return value.lower() in ['true'] if isinstance(value, str) else bool(value)

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

  :param button: "Left", "Middle", "Right", or "None"
   start, end : window coordinates for action
  :param steps: number of steps to move in, if <2 then mouse jumps to the end position
  :param modifiers: list containing zero or more of "Shift" or "Control"

  .. hint::

    For generating test data you can use this snippet of code::

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
    for step in range(steps):
      frac = float(step)/(steps-1)
      x = int(start[0] + frac*(end[0]-start[0]))
      y = int(start[1] + frac*(end[1]-start[1]))
      interactor.SetEventPosition(x,y)
      interactor.MouseMoveEvent()
  up()
  interactor.SetShiftKey(0)
  interactor.SetControlKey(0)

def downloadFile(url, targetFilePath, checksum=None, reDownloadIfChecksumInvalid=True):
  """ Download ``url`` to local storage as ``targetFilePath``

  Target file path needs to indicate the file name and extension as well

  If specified, the ``checksum`` is used to verify that the downloaded file is the expected one.
  It must be specified as ``<algo>:<digest>``. For example, ``SHA256:cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93``.
  """
  import os
  import logging
  try:
    (algo, digest) = extractAlgoAndDigest(checksum)
  except ValueError as excinfo:
    logging.error('Failed to parse checksum: ' + excinfo.message)
    return False
  if not os.path.exists(targetFilePath) or os.stat(targetFilePath).st_size == 0:
    logging.info('Downloading from\n  %s\nas file\n  %s\nIt may take a few minutes...' % (url,targetFilePath))
    try:
      import urllib.request, urllib.parse, urllib.error
      urllib.request.urlretrieve(url, targetFilePath)
    except Exception as e:
      import traceback
      traceback.print_exc()
      logging.error('Failed to download file from ' + url)
      return False
    if algo is not None:
      logging.info('Verifying checksum\n  %s' % targetFilePath)
      current_digest = computeChecksum(algo, targetFilePath)
      if current_digest != digest:
        logging.error('Downloaded file does not have expected checksum.'
          '\n   current checksum: %s'
          '\n  expected checksum: %s' % (current_digest, digest))
        return False
      else:
        logging.info('Checksum OK')
  else:
    if algo is not None:
      current_digest = computeChecksum(algo, targetFilePath)
      if current_digest != digest:
        if reDownloadIfChecksumInvalid:
          logging.info('Requested file has been found but its checksum is different: deleting and re-downloading')
          os.remove(targetFilePath)
          return downloadFile(url, targetFilePath, checksum, reDownloadIfChecksumInvalid=False)
        else:
          logging.error('Requested file has been found but its checksum is different:'
            '\n   current checksum: %s'
            '\n  expected checksum: %s' % (current_digest, digest))
          return False
      else:
        logging.info('Requested file has been found and checksum is OK: ' + targetFilePath)
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

def computeChecksum(algo, filePath):
  """Compute digest of ``filePath`` using ``algo``.

  Supported hashing algorithms are SHA256, SHA512, and MD5.

  It internally reads the file by chunk of 8192 bytes.

  :raises ValueError: if algo is unknown.
  :raises IOError: if filePath does not exist.
  """
  import hashlib

  if algo not in ['SHA256', 'SHA512', 'MD5']:
    raise ValueError("unsupported hashing algorithm %s" % algo)

  with open(filePath, 'rb') as content:
    hash = hashlib.new(algo)
    while True:
        chunk = content.read(8192)
        if not chunk:
            break
        hash.update(chunk)
    return hash.hexdigest()

def extractAlgoAndDigest(checksum):
  """Given a checksum string formatted as ``<algo>:<digest>`` returns
  the tuple ``(algo, digest)``.

  ``<algo>`` is expected to be `SHA256`, `SHA512`, or `MD5`.
  ``<digest>`` is expected to be the full length hexdecimal digest.

  :raises ValueError: if checksum is incorrectly formatted.
  """
  if checksum is None:
    return None, None
  if len(checksum.split(':')) != 2:
    raise ValueError("invalid checksum '%s'. Expected format is '<algo>:<digest>'." % checksum)
  (algo, digest) = checksum.split(':')
  expected_algos = ['SHA256', 'SHA512', 'MD5']
  if algo not in expected_algos:
    raise ValueError("invalid algo '%s'. Algo must be one of %s" % (algo, ", ".join(expected_algos)))
  expected_digest_length = {'SHA256': 64, 'SHA512': 128, 'MD5': 32}
  if len(digest) != expected_digest_length[algo]:
    raise ValueError("invalid digest length %d. Expected digest length for %s is %d" % (len(digest), algo, expected_digest_length[algo]))
  return algo, digest

def downloadAndExtractArchive(url, archiveFilePath, outputDir, \
                              expectedNumberOfExtractedFiles=None, numberOfTrials=3, checksum=None):
  """ Downloads an archive from ``url`` as ``archiveFilePath``, and extracts it to ``outputDir``.

  This combined function tests the success of the download by the extraction step,
  and re-downloads if extraction failed.

  If specified, the ``checksum`` is used to verify that the downloaded file is the expected one.
  It must be specified as ``<algo>:<digest>``. For example, ``SHA256:cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93``.
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
    if not downloadFile(url, archiveFilePath, checksum):
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
    volumeNode = SampleData.downloadSample("MRHead")

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
    volumeNode = SampleData.downloadSample("MRHead")

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
    if 'chart' in nodes:
      chartNode = nodes['chart']
    if 'table' in nodes:
      tableNode = nodes['table']
    if 'series' in nodes:
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

def launchConsoleProcess(args, useStartupEnvironment=True, cwd=None):
  """Launch a process. Hiding the console and captures the process output.
  The console window is hidden when running on Windows.
  :param args: executable name, followed by command-line arguments
  :param useStartupEnvironment: launch the process in the original environment as the original Slicer process
  :param cwd: current working directory
  :return: process object.
  """
  import subprocess
  if useStartupEnvironment:
    startupEnv = startupEnvironment()
  else:
    startupEnv = None
  import os
  if os.name == 'nt':
    # Hide console window (only needed on Windows)
    info = subprocess.STARTUPINFO()
    info.dwFlags = 1
    info.wShowWindow = 0
    proc = subprocess.Popen(args, env=startupEnv, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True, startupinfo=info, cwd=cwd)
  else:
    proc = subprocess.Popen(args, env=startupEnv, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True, cwd=cwd)
  return proc

def logProcessOutput(proc):
  """Continuously write process output to the application log and the Python console.
  :param proc: process object.
  """
  from subprocess import Popen, PIPE, CalledProcessError
  import logging
  try:
    from slicer import app
    guiApp = app
  except ImportError:
    # Running from console
    guiApp = None
  for line in proc.stdout:
    if guiApp:
      logging.info(line.rstrip())
      guiApp.processEvents()  # give a chance the application to refresh GUI
    else:
      print(line.rstrip())
  proc.wait()
  retcode=proc.returncode
  if retcode != 0:
    raise CalledProcessError(retcode, proc.args, output=proc.stdout, stderr=proc.stderr)

def _executePythonModule(module, args):
  """Execute a Python module as a script in Slicer's Python environment.
  Internally python -m is called with the module name and additional arguments.
  """
  # Determine pythonSlicerExecutablePath
  try:
    from slicer import app
    # If we get to this line then import from "app" is succeeded,
    # which means that we run this function from Slicer Python interpreter.
    # PythonSlicer is added to PATH environment variable in Slicer
    # therefore shutil.which will be able to find it.
    import shutil
    import subprocess
    pythonSlicerExecutablePath = shutil.which('PythonSlicer')
    if not pythonSlicerExecutablePath:
      raise RuntimeError("PythonSlicer executable not found")
  except ImportError:
    # Running from console
    import os
    import sys
    pythonSlicerExecutablePath = os.path.dirname(sys.executable)+"/PythonSlicer"
    if os.name == 'nt':
      pythonSlicerExecutablePath += ".exe"

  commandLine = [pythonSlicerExecutablePath, "-m", module, *args]
  proc = launchConsoleProcess(commandLine, useStartupEnvironment=False)
  logProcessOutput(proc)

def pip_install(requirements):
  """Install python packages.
  Currently, the method simply calls ``python -m pip install`` but in the future further checks, optimizations,
  user confirmation may be implemented, therefore it is recommended to use this method call instead of a plain
  pip install.
  :param requirements: requirement specifier, same format as used by pip (https://docs.python.org/3/installing/index.html)

  Example: calling from Slicer GUI

  .. code-block:: python

    pip_install("tensorflow keras scikit-learn ipywidgets")

  Example: calling from PythonSlicer console

  .. code-block:: python

    from slicer.util import pip_install
    pip_install("tensorflow")

  """
  args = 'install', *requirements.split()
  _executePythonModule('pip', args)

def pip_uninstall(requirements):
  """Uninstall python packages.
  Currently, the method simply calls ``python -m pip uninstall`` but in the future further checks, optimizations,
  user confirmation may be implemented, therefore it is recommended to use this method call instead of a plain
  pip uninstall.
  :param requirements: requirement specifier, same format as used by pip (https://docs.python.org/3/installing/index.html)

  Example: calling from Slicer GUI

  .. code-block:: python

    pip_uninstall("tensorflow keras scikit-learn ipywidgets")

  Example: calling from PythonSlicer console

  .. code-block:: python

    from slicer.util import pip_uninstall
    pip_uninstall("tensorflow")

  """
  args = 'uninstall', *requirements.split(), '--yes'
  _executePythonModule('pip', args)

def longPath(path):
  """
  Make long paths work on Windows, where the maximum path length is 260 characters.
  For example, the files in the DICOM database may have paths longer than this limit.
  Accessing these can be made safe by prefixing it with the UNC prefix ('\\?\').

  :param string path: Path to be made safe if too long

  :return string: Safe path
  """
  # Return path as is if conversion is disabled
  longPathConversionEnabled = settingsValue('General/LongPathConversionEnabled', True, converter=toBool)
  if not longPathConversionEnabled:
    return path
  # Return path as is on operating systems other than Windows
  import qt
  sysInfo = qt.QSysInfo()
  if sysInfo.productType() != 'windows':
    return path
  # Skip prefixing relative paths as UNC prefix wors only on absolute paths
  if not qt.QDir.isAbsolutePath(path):
    return path
  # Return path as is if UNC prefix is already applied
  if path[:4] == '\\\\?\\':
    return path
  return u"\\\\?\\" + path.replace('/', '\\')

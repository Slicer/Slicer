from __future__ import print_function
import os, string
import unittest
import vtk, qt, ctk, slicer
import logging
import importlib

__all__ = ['ScriptedLoadableModule', 'ScriptedLoadableModuleWidget', 'ScriptedLoadableModuleLogic', 'ScriptedLoadableModuleTest']

class ScriptedLoadableModule(object):
  def __init__(self, parent):
    self.parent = parent
    self.moduleName = self.__class__.__name__

    parent.title = ""
    parent.categories = []
    parent.dependencies = []
    parent.contributors = ["Andras Lasso (PerkLab, Queen's University), Steve Pieper (Isomics)"]
    parent.helpText = """
This module was created from a template and the help section has not yet been updated.
"""

    parent.acknowledgementText = """
This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.
This work is partially supported by PAR-07-249: R01CA131718 NA-MIC Virtual Colonoscopy (See <a href=http://www.slicer.org>http://www.na-mic.org/Wiki/index.php/NA-MIC_NCBC_Collaboration:NA-MIC_virtual_colonoscopy</a>).
"""

    # Set module icon from Resources/Icons/<ModuleName>.png
    moduleDir = os.path.dirname(self.parent.path)
    iconPath = os.path.join(moduleDir, 'Resources/Icons', self.moduleName+'.png')
    if os.path.isfile(iconPath):
      parent.icon = qt.QIcon(iconPath)

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests[self.moduleName] = self.runTest

  def getDefaultModuleDocumentationLink(self, docPage=None):
    """Return string that can be inserted into the application help text that contains
    link to the module's documentation in current Slicer version's documentation.
    Currently the text is "See the documentation for more information."
    If docPage is not specified then the link points to Modules/(ModuleName).
    """
    if not docPage:
      docPage = "Modules/"+self.moduleName
    linkText = '<p>For more information see the <a href="{0}/Documentation/{1}.{2}/{3}">online documentation</a>.</p>'.format(
      self.parent.slicerWikiUrl, slicer.app.majorVersion, slicer.app.minorVersion, docPage)
    return linkText

  def runTest(self, msec=100, **kwargs):
    """
    :param msec: delay to associate with :func:`ScriptedLoadableModuleTest.delayDisplay()`.
    """
    # Name of the test case class is expected to be <ModuleName>Test
    module = importlib.import_module(self.__module__)
    className = self.moduleName + 'Test'
    try:
      TestCaseClass = getattr(module, className)
    except AttributeError:
      # Treat missing test case class as a failure; provide useful error message
      raise AssertionError('Test case class not found: %s.%s ' % (self.__module__, className))

    testCase = TestCaseClass()
    testCase.messageDelay = msec
    testCase.runTest(**kwargs)

class ScriptedLoadableModuleWidget(object):
  def __init__(self, parent = None):
    """If parent widget is not specified: a top-level widget is created automatically;
    the application has to delete this widget (by calling widget.parent.deleteLater() to avoid memory leaks.
    """
    # Get module name by stripping 'Widget' from the class name
    self.moduleName = self.__class__.__name__
    if self.moduleName.endswith('Widget'):
      self.moduleName = self.moduleName[:-6]
    self.developerMode = slicer.util.settingsValue('Developer/DeveloperMode', False, converter=slicer.util.toBool)
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parent.show()
    slicer.app.moduleManager().connect(
      'moduleAboutToBeUnloaded(QString)', self._onModuleAboutToBeUnloaded)

  def resourcePath(self, filename):
    scriptedModulesPath = os.path.dirname(slicer.util.modulePath(self.moduleName))
    return os.path.join(scriptedModulesPath, 'Resources', filename)

  def cleanup(self):
    """Override this function to implement module widget specific cleanup.

    It is invoked when the signal `qSlicerModuleManager::moduleAboutToBeUnloaded(QString)`
    corresponding to the current module is emitted and just before a module is
    effectively unloaded.
    """
    pass

  def _onModuleAboutToBeUnloaded(self, moduleName):
    """This slot calls `cleanup()` if the module about to be unloaded is the
    current one.
    """
    if moduleName == self.moduleName:
      self.cleanup()
      slicer.app.moduleManager().disconnect(
        'moduleAboutToBeUnloaded(QString)', self._onModuleAboutToBeUnloaded)

  def setupDeveloperSection(self):
    if not self.developerMode:
      return

    def createHLayout(elements):
      rowLayout = qt.QHBoxLayout()
      for element in elements:
        rowLayout.addWidget(element)
      return rowLayout

    #
    # Reload and Test area
    # Used during development, but hidden when delivering
    # developer mode is turned off.

    self.reloadCollapsibleButton = ctk.ctkCollapsibleButton()
    self.reloadCollapsibleButton.text = "Reload && Test"
    self.layout.addWidget(self.reloadCollapsibleButton)
    reloadFormLayout = qt.QFormLayout(self.reloadCollapsibleButton)

    # reload button
    self.reloadButton = qt.QPushButton("Reload")
    self.reloadButton.toolTip = "Reload this module."
    self.reloadButton.name = "ScriptedLoadableModuleTemplate Reload"
    self.reloadButton.connect('clicked()', self.onReload)

    # reload and test button
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    # edit python source code
    self.editSourceButton = qt.QPushButton("Edit")
    self.editSourceButton.toolTip = "Edit the module's source code."
    self.editSourceButton.connect('clicked()', self.onEditSource)

    self.editModuleUiButton = None
    moduleUiFileName = self.resourcePath('UI/%s.ui' % self.moduleName)
    import os.path
    if os.path.isfile(moduleUiFileName):
      # Module UI file exists
      self.editModuleUiButton = qt.QPushButton("Edit UI")
      self.editModuleUiButton.toolTip = "Edit the module's .ui file."
      self.editModuleUiButton.connect('clicked()', lambda filename=moduleUiFileName: slicer.util.startQtDesigner(moduleUiFileName))

    # restart Slicer button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.restartButton = qt.QPushButton("Restart Slicer")
    self.restartButton.toolTip = "Restart Slicer"
    self.restartButton.name = "ScriptedLoadableModuleTemplate Restart"
    self.restartButton.connect('clicked()', slicer.app.restart)

    if self.editModuleUiButton:
      # There are many buttons, distribute them in two rows
      reloadFormLayout.addRow(createHLayout([self.reloadButton, self.reloadAndTestButton, self.restartButton]))
      reloadFormLayout.addRow(createHLayout([self.editSourceButton, self.editModuleUiButton]))
    else:
      reloadFormLayout.addRow(createHLayout([self.reloadButton, self.reloadAndTestButton, self.editSourceButton, self.restartButton]))

  def setup(self):
    # Instantiate and connect default widgets ...
    self.setupDeveloperSection()

  def onReload(self):
    """
    Reload scripted module widget representation.
    """

    # Print a clearly visible separator to make it easier
    # to distinguish new error messages (during/after reload)
    # from old ones.
    print('\n' * 2)
    print('-' * 30)
    print('Reloading module: '+self.moduleName)
    print('-' * 30)
    print('\n' * 2)

    slicer.util.reloadScriptedModule(self.moduleName)

  def onReloadAndTest(self, **kwargs):
    """Reload scripted module widget representation and call :func:`ScriptedLoadableModuleTest.runTest()`
    passing ``kwargs``.
    """
    try:
      self.onReload()
      test = slicer.selfTests[self.moduleName]
      test(msec=int(slicer.app.userSettings().value("Developer/SelfTestDisplayMessageDelay")), **kwargs)
    except Exception as e:
      import traceback
      traceback.print_exc()
      errorMessage = "Reload and Test: Exception!\n\n" + str(e) + "\n\nSee Python Console for Stack Trace"
      slicer.util.errorDisplay(errorMessage)

  def onEditSource(self):
    filePath = slicer.util.modulePath(self.moduleName)
    qt.QDesktopServices.openUrl(qt.QUrl("file:///"+filePath, qt.QUrl.TolerantMode))

class ScriptedLoadableModuleLogic(object):
  def __init__(self, parent = None):
    # Get module name by stripping 'Logic' from the class name
    self.moduleName = self.__class__.__name__
    if self.moduleName.endswith('Logic'):
      self.moduleName = self.moduleName[:-5]

    # If parameter node is singleton then only one parameter node
    # is allowed in a scene.
    # Derived classes can set self.isSingletonParameterNode = False
    # to allow having multiple parameter nodes in the scene.
    self.isSingletonParameterNode = True

    # takeScreenshot default parameters
    self.enableScreenshots = False
    self.screenshotScaleFactor = 1.0

  def getParameterNode(self):
    """
    Return the first available parameter node for this module
    If no parameter nodes are available for this module then a new one is created.
    """
    if self.isSingletonParameterNode:
      parameterNode = slicer.mrmlScene.GetSingletonNode(self.moduleName, "vtkMRMLScriptedModuleNode")
      if parameterNode:
        # After close scene, ModuleName attribute may be removed, restore it now
        if parameterNode.GetAttribute("ModuleName") != self.moduleName:
          parameterNode.SetAttribute("ModuleName", self.moduleName)
        return parameterNode
    else:
      numberOfScriptedModuleNodes =  slicer.mrmlScene.GetNumberOfNodesByClass("vtkMRMLScriptedModuleNode")
      for nodeIndex in range(numberOfScriptedModuleNodes):
        parameterNode  = slicer.mrmlScene.GetNthNodeByClass( nodeIndex, "vtkMRMLScriptedModuleNode" )
        if parameterNode.GetAttribute("ModuleName") == self.moduleName:
          return parameterNode
    # no parameter node was found for this module, therefore we add a new one now
    parameterNode = slicer.mrmlScene.AddNode(self.createParameterNode())
    return parameterNode

  def getAllParameterNodes(self):
    """
    Return a list of all parameter nodes for this module
    Multiple parameter nodes are useful for storing multiple parameter sets in a single scene.
    """
    foundParameterNodes = []
    numberOfScriptedModuleNodes =  slicer.mrmlScene.GetNumberOfNodesByClass("vtkMRMLScriptedModuleNode")
    for nodeIndex in range(numberOfScriptedModuleNodes):
      parameterNode  = slicer.mrmlScene.GetNthNodeByClass( nodeIndex, "vtkMRMLScriptedModuleNode" )
      if parameterNode.GetAttribute("ModuleName") == self.moduleName:
        foundParameterNodes.append(parameterNode)
    return foundParameterNodes

  def createParameterNode(self):
    """
    Create a new parameter node
    The node is of vtkMRMLScriptedModuleNode class. Module name is added as an attribute to allow filtering
    in node selector widgets (attribute name: ModuleName, attribute value: the module's name).
    This method can be overridden in derived classes to create a default parameter node with all
    parameter values set to their default.
    """
    if slicer.mrmlScene is None:
      return

    node = slicer.mrmlScene.CreateNodeByClass("vtkMRMLScriptedModuleNode")
    node.UnRegister(None) # object is owned by the Python variable now
    if self.isSingletonParameterNode:
      node.SetSingletonTag( self.moduleName )
    # Add module name in an attribute to allow filtering in node selector widgets
    # Note that SetModuleName is not used anymore as it would be redundant with the ModuleName attribute.
    node.SetAttribute( "ModuleName", self.moduleName )
    node.SetName(slicer.mrmlScene.GenerateUniqueName(self.moduleName))
    return node

  def delayDisplay(self,message,msec=1000):
    """
    Display a message in a popup window for a short time.
    It is recommended to directly use slicer.util.delayDisplay function.
    This method is only kept for backward compatibility and may be removed in the future.
    """
    slicer.util.delayDisplay(message, msec)

  def clickAndDrag(self,widget,button='Left',start=(10,10),end=(10,40),steps=20,modifiers=[]):
    """
    Send synthetic mouse events to the specified widget (qMRMLSliceWidget or qMRMLThreeDView).
    It is recommended to directly use slicer.util.clickAndDrag function.
    This method is only kept for backward compatibility and may be removed in the future.
    """
    slicer.util.clickAndDrag(widget,button=button,start=start,end=end,steps=steps,modifiers=modifiers)

  def takeScreenshot(self,name,description,type=-1):
    """ Take a screenshot of the selected viewport and store as and
    annotation snapshot node. Convenience method for automated testing.

    If self.enableScreenshots is False then only a message is displayed but screenshot
    is not stored. Screenshots are scaled by self.screenshotScaleFactor.

    :param name: snapshot node name
    :param description: description of the node
    :param type: which viewport to capture. If not specified then captures the entire window.
      Valid values: slicer.qMRMLScreenShotDialog.FullLayout,
      slicer.qMRMLScreenShotDialog.ThreeD, slicer.qMRMLScreenShotDialog.Red,
      slicer.qMRMLScreenShotDialog.Yellow, slicer.qMRMLScreenShotDialog.Green.
    """

    # show the message even if not taking a screen shot
    slicer.util.delayDisplay(description)

    if not self.enableScreenshots:
      return

    lm = slicer.app.layoutManager()
    # switch on the type to get the requested window
    widget = 0
    if type == slicer.qMRMLScreenShotDialog.FullLayout:
      # full layout
      widget = lm.viewport()
    elif type == slicer.qMRMLScreenShotDialog.ThreeD:
      # just the 3D window
      widget = lm.threeDWidget(0).threeDView()
    elif type == slicer.qMRMLScreenShotDialog.Red:
      # red slice window
      widget = lm.sliceWidget("Red")
    elif type == slicer.qMRMLScreenShotDialog.Yellow:
      # yellow slice window
      widget = lm.sliceWidget("Yellow")
    elif type == slicer.qMRMLScreenShotDialog.Green:
      # green slice window
      widget = lm.sliceWidget("Green")
    else:
      # default to using the full window
      widget = slicer.util.mainWindow()
      # reset the type so that the node is set correctly
      type = slicer.qMRMLScreenShotDialog.FullLayout

    # grab and convert to vtk image data
    qimage = ctk.ctkWidgetsUtils.grabWidget(widget)
    imageData = vtk.vtkImageData()
    slicer.qMRMLUtils().qImageToVtkImageData(qimage,imageData)

    annotationLogic = slicer.modules.annotations.logic()
    annotationLogic.CreateSnapShot(name, description, type, self.screenshotScaleFactor, imageData)

class ScriptedLoadableModuleTest(unittest.TestCase):
  """
  Base class for module tester class.
  Setting messageDelay to something small, like 50ms allows
  faster development time.
  """

  def __init__(self, *args, **kwargs):
    super(ScriptedLoadableModuleTest, self).__init__(*args, **kwargs)

  def delayDisplay(self,message,requestedDelay=None,msec=None):
    """
    Display messages to the user/tester during testing.

    By default, the delay is 50ms.

    The function accepts the keyword arguments ``requestedDelay`` or ``msec``. If both
    are specified, the value associated with ``msec`` is used.

    This method can be temporarily overridden to allow tests running
    with longer or shorter message display time.

    Displaying a dialog and waiting does two things:
    1) it lets the event loop catch up to the state of the test so
    that rendering and widget updates have all taken place before
    the test continues and
    2) it shows the user/developer/tester the state of the test
    so that we'll know when it breaks.

    Note:
    Information that might be useful (but not important enough to show
    to the user) can be logged using logging.info() function
    (printed to console and application log) or logging.debug()
    function (printed to application log only).
    Error messages should be logged by logging.error() function
    and displayed to user by slicer.util.errorDisplay function.
    """
    if hasattr(self, "messageDelay"):
      msec = self.messageDelay
    if msec is None:
      msec = requestedDelay
    if msec is None:
      msec = 100

    slicer.util.delayDisplay(message, msec)

  def runTest(self):
    """
    Run a default selection of tests here.
    """
    logging.warning('No test is defined in '+self.__class__.__name__)

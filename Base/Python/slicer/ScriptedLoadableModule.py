import os, string
import unittest
import qt, ctk, slicer
import logging
import importlib

__all__ = ['ScriptedLoadableModule', 'ScriptedLoadableModuleWidget', 'ScriptedLoadableModuleLogic', 'ScriptedLoadableModuleTest']

class ScriptedLoadableModule:
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
    linkText = 'See <a href="{0}/Documentation/{1}.{2}/{3}">the documentation</a> for more information.'.format(
      self.parent.slicerWikiUrl, slicer.app.majorVersion, slicer.app.minorVersion, docPage)
    return linkText

  def runTest(self):
    # Name of the test case class is expected to be <ModuleName>Test
    module = importlib.import_module(self.__module__)
    className = self.moduleName + 'Test'
    try:
      TestCaseClass = getattr(module, className)
    except AttributeError:
      # Treat missing test case class as a failure; provide useful error message
      raise AssertionError('Test case class not found: %s.%s ' % (self.__module__, className))

    testCase = TestCaseClass()
    testCase.runTest()

class ScriptedLoadableModuleWidget:
  def __init__(self, parent = None):
    # Get module name by stripping 'Widget' from the class name
    self.moduleName = self.__class__.__name__
    if self.moduleName.endswith('Widget'):
      self.moduleName = self.moduleName[:-6]
    settings = qt.QSettings()
    self.developerMode = settings.value('Developer/DeveloperMode').lower() == 'true'
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

  def setupDeveloperSection(self):
    if not self.developerMode:
      return

    def createHLayout(elements):
      widget = qt.QWidget()
      rowLayout = qt.QHBoxLayout()
      widget.setLayout(rowLayout)
      for element in elements:
        rowLayout.addWidget(element)
      return widget

    #
    # Reload and Test area
    #
    self.reloadCollapsibleButton = ctk.ctkCollapsibleButton()
    self.reloadCollapsibleButton.text = "Reload && Test"
    self.layout.addWidget(self.reloadCollapsibleButton)
    reloadFormLayout = qt.QFormLayout(self.reloadCollapsibleButton)

    # reload button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadButton = qt.QPushButton("Reload")
    self.reloadButton.toolTip = "Reload this module."
    self.reloadButton.name = "ScriptedLoadableModuleTemplate Reload"
    self.reloadButton.connect('clicked()', self.onReload)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    # restart Slicer button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.restartButton = qt.QPushButton("Restart Slicer")
    self.restartButton.toolTip = "Restart Slicer"
    self.restartButton.name = "ScriptedLoadableModuleTemplate Restart"
    self.restartButton.connect('clicked()', slicer.app.restart)

    reloadFormLayout.addWidget(createHLayout([self.reloadButton, self.reloadAndTestButton, self.restartButton]))


  def setup(self):
    # Instantiate and connect default widgets ...
    self.setupDeveloperSection()

  def cleanup(self):
    pass

  def onReload(self):
    """
    ModuleWizard will substitute correct default moduleName.
    Generic reload method for any scripted module.
    """
    slicer.util.reloadScriptedModule(self.moduleName)

  def onReloadAndTest(self):
    try:
      self.onReload()
      test = slicer.selfTests[self.moduleName]
      test()
    except Exception, e:
      import traceback
      traceback.print_exc()
      errorMessage = "Reload and Test: Exception!\n\n" + str(e) + "\n\nSee Python Console for Stack Trace"
      slicer.util.errorDisplay(errorMessage)

class ScriptedLoadableModuleLogic():
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

  def getParameterNode(self):
    """
    Return the first available parameter node for this module
    If no parameter nodes are available for this module then a new one is created.
    """
    numberOfScriptedModuleNodes =  slicer.mrmlScene.GetNumberOfNodesByClass("vtkMRMLScriptedModuleNode")
    for nodeIndex in xrange(numberOfScriptedModuleNodes):
      parameterNode  = slicer.mrmlScene.GetNthNodeByClass( nodeIndex, "vtkMRMLScriptedModuleNode" )
      if parameterNode.GetAttribute("ModuleName") == self.moduleName:
        return parameterNode
    # no parameter node was found for this module, therefore we add a new one now
    parameterNode = self.createParameterNode()
    slicer.mrmlScene.AddNode(parameterNode)
    return parameterNode

  def getAllParameterNodes(self):
    """
    Return a list of all parameter nodes for this module
    Multiple parameter nodes are useful for storing multiple parameter sets in a single scene.
    """
    foundParameterNodes = []
    numberOfScriptedModuleNodes =  slicer.mrmlScene.GetNumberOfNodesByClass("vtkMRMLScriptedModuleNode")
    for nodeIndex in xrange(numberOfScriptedModuleNodes):
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
    node = slicer.vtkMRMLScriptedModuleNode()
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
    Send synthetic mouse events to the specified widget (qMRMLSliceWidget or qMRMLThreeDView)
    button : "Left", "Middle", "Right", or "None"
    start, end : window coordinates for action
    steps : number of steps to move in
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
    for step in xrange(steps):
      frac = float(step)/steps
      x = int(start[0] + frac*(end[0]-start[0]))
      y = int(start[1] + frac*(end[1]-start[1]))
      interactor.SetEventPosition(x,y)
      interactor.MouseMoveEvent()
    up()
    interactor.SetShiftKey(0)
    interactor.SetControlKey(0)

class ScriptedLoadableModuleTest(unittest.TestCase):
  """
  Base class for module tester class.
  Setting messageDelay to something small, like 50 allows
  faster development time.
  """

  def __init__(self, *args, **kwargs):
    super(ScriptedLoadableModuleTest, self).__init__(*args, **kwargs)

  def delayDisplay(self,message,requestedDelay=None):
    """
    Display messages to the user/tester during testing.
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
    else:
        msec = 1000
    if requestedDelay:
      msec = requestedDelay
    slicer.util.delayDisplay(message, msec)

  def runTest(self):
    """
    Run a default selection of tests here.
    """
    logging.warning('No test is defined in '+self.__class__.__name__)

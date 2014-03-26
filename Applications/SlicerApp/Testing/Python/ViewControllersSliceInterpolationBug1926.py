import os
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# ViewControllersSliceInterpolationBug1926
#

class ViewControllersSliceInterpolationBug1926:
  def __init__(self, parent):
    parent.title = "ViewControllers Slice Interpolation Bug 1926" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Jim Miller (GE)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    Test case for the interaction between the ViewControllers module, linking, slice interpolation, and the selection of background, foreground, and label images. When entering the ViewControllers module, extra View Controllers are created and configured.  If linking is on, then foreground, backgroud, and label selection can be propagated to the other views incorrectly. If the node selectors are bocked from emitting signals, then the viewers maintain their proper volumes. However the slice interpolation widget is then not managed properly.
    """
    parent.acknowledgementText = """
    This file was originally developed by Jim Miller, GE.  and was partially funded by NIH grant U54EB005149.
""" # replace with organization, grant and thanks.
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['ViewControllersSliceInterpolationBug1926'] = self.runTest

  def runTest(self):
    tester = ViewControllersSliceInterpolationBug1926Test()
    tester.runTest()

#
# qViewControllersSliceInterpolationBug1926Widget
#

class ViewControllersSliceInterpolationBug1926Widget:
  def __init__(self, parent = None):
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

  def setup(self):
    # Instantiate and connect widgets ...

    # reload button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadButton = qt.QPushButton("Reload")
    self.reloadButton.toolTip = "Reload this module."
    self.reloadButton.name = "ViewControllersSliceInterpolationBug1926 Reload"
    self.layout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    self.layout.addWidget(self.reloadAndTestButton)
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    # Collapsible button
    dummyCollapsibleButton = ctk.ctkCollapsibleButton()
    dummyCollapsibleButton.text = "A collapsible button"
    self.layout.addWidget(dummyCollapsibleButton)

    # Layout within the dummy collapsible button
    dummyFormLayout = qt.QFormLayout(dummyCollapsibleButton)

    # HelloWorld button
    helloWorldButton = qt.QPushButton("Hello world")
    helloWorldButton.toolTip = "Print 'Hello world' in standard ouput."
    dummyFormLayout.addWidget(helloWorldButton)
    helloWorldButton.connect('clicked(bool)', self.onHelloWorldButtonClicked)

    # Add vertical spacer
    self.layout.addStretch(1)

    # Set local var as instance attribute
    self.helloWorldButton = helloWorldButton

  def onHelloWorldButtonClicked(self):
    print "Hello World !"

  def onReload(self,moduleName="ViewControllersSliceInterpolationBug1926"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="ViewControllersSliceInterpolationBug1926"):
    self.onReload()
    evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
    tester = eval(evalString)
    tester.runTest()

#
# ViewControllersSliceInterpolationBug1926Logic
#

class ViewControllersSliceInterpolationBug1926Logic:
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass

  def hasImageData(self,volumeNode):
    """This is a dummy logic method that
    returns true if the passed in volume
    node has valid image data
    """
    if not volumeNode:
      print('no volume node')
      return False
    if volumeNode.GetImageData() == None:
      print('no image data')
      return False
    return True


class ViewControllersSliceInterpolationBug1926Test(unittest.TestCase):
  """
  This is the test case for your scripted module.
  """

  def delayDisplay(self,message,msec=1000):
    """This utility method displays a small dialog and waits.
    This does two things: 1) it lets the event loop catch up
    to the state of the test so that rendering and widget updates
    have all taken place before the test continues and 2) it
    shows the user/developer/tester the state of the test
    so that we'll know when it breaks.
    """
    print(message)
    self.info = qt.QDialog()
    self.infoLayout = qt.QVBoxLayout()
    self.info.setLayout(self.infoLayout)
    self.label = qt.QLabel(message,self.info)
    self.infoLayout.addWidget(self.label)
    qt.QTimer.singleShot(msec, self.info.close)
    self.info.exec_()

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_ViewControllersSliceInterpolationBug19261()

  def test_ViewControllersSliceInterpolationBug19261(self):
    """ Ideally you should have several levels of tests.  At the lowest level
    tests sould exercise the functionality of the logic with different inputs
    (both valid and invalid).  At higher levels your tests should emulate the
    way the user would interact with your code and confirm that it still works
    the way you intended.
    One of the most important features of the tests is that it should alert other
    developers when their changes will have an impact on the behavior of your
    module.  For example, if a developer removes a feature that you depend on,
    your test should break so they know that the feature is needed.
    """

    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    self.delayDisplay("Getting Data")
    head = sampleDataLogic.downloadMRHead()

    tumor = sampleDataLogic.downloadMRBrainTumor1()

    # Change to a CompareView
    ln = slicer.util.getNode(pattern='vtkMRMLLayoutNode*')
    ln.SetNumberOfCompareViewRows(2)
    ln.SetNumberOfCompareViewLightboxColumns(4)
    ln.SetViewArrangement(12)
    self.delayDisplay('Compare View')

    # Get the slice logic, slice node and slice composite node for the
    # first compare viewer
    logic = slicer.app.layoutManager().sliceWidget('Compare1').sliceLogic()
    compareNode = logic.GetSliceNode()
    compareCNode = logic.GetSliceCompositeNode()

    # Link the viewers
    compareCNode.SetLinkedControl(1)
    self.delayDisplay('Linked the viewers (first Compare View)')

    # Set the data to be same on all viewers
    logic.StartSliceCompositeNodeInteraction(2)  #BackgroundVolumeFlag
    compareCNode.SetBackgroundVolumeID(tumor.GetID())
    logic.EndSliceCompositeNodeInteraction()
    self.assertTrue( compareCNode.GetBackgroundVolumeID() == tumor.GetID())
    self.delayDisplay('Compare views configured')

    # Get handles to the Red viewer
    redLogic = slicer.app.layoutManager().sliceWidget('Red').sliceLogic()
    redNode = redLogic.GetSliceNode()
    redCNode = redLogic.GetSliceCompositeNode()

    # Configure the red viewer with a different dataset
    redCNode.SetLinkedControl(0)
    redLogic.StartSliceCompositeNodeInteraction(2)  #BackgroundVolumeFlag
    redCNode.SetBackgroundVolumeID(head.GetID())
    redLogic.EndSliceCompositeNodeInteraction()
    redCNode.SetLinkedControl(1)
    self.delayDisplay('Red viewer configured')

    # Get handles to the second compare view
    compareNode2 = slicer.util.getNode('vtkMRMLSliceNodeCompare2')
    compareCNode2 = slicer.util.getNode('vtkMRMLSliceCompositeNodeCompare2')

    # Check whether the viewers have the proper data initially
    self.assertTrue( redCNode.GetBackgroundVolumeID() == head.GetID())
    self.assertTrue( compareCNode.GetBackgroundVolumeID() == tumor.GetID())
    self.assertTrue( compareCNode2.GetBackgroundVolumeID() == tumor.GetID())
    self.delayDisplay('All viewers configured properly')

    # Switch to the View Controllers module
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModule('ViewControllers')
    self.delayDisplay("Entered View Controllers module")

    # Check the volume selectors
    self.assertTrue( redCNode.GetBackgroundVolumeID() == head.GetID())
    self.assertTrue( compareCNode.GetBackgroundVolumeID() == tumor.GetID())
    self.assertTrue( compareCNode2.GetBackgroundVolumeID() == tumor.GetID())
    self.delayDisplay('All viewers still configured properly')

    # Check whether we can change the interpolation (needs to check gui)
    redWidget = slicer.app.layoutManager().sliceWidget('Red')
    redController = redWidget.sliceController()


    self.delayDisplay('Test passed!')

import os
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# Slicer4Minute
#

class Slicer4Minute:
  def __init__(self, parent):
    parent.title = "Slicer4Minute" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Jim Miller (GE)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    Test suite for the Slicer 4 Minute tutorial
    """
    parent.acknowledgementText = """
    This file was originally developed by Jim Miller, GE and was partially funded by NIH grant U54EB005149.
""" # replace with organization, grant and thanks.
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['Slicer4Minute'] = self.runTest

  def runTest(self):
    tester = Slicer4MinuteTest()
    tester.runTest()

#
# qSlicer4MinuteWidget
#

class Slicer4MinuteWidget:
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
    self.reloadButton.name = "Slicer4Minute Reload"
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

  def onReload(self,moduleName="Slicer4Minute"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="Slicer4Minute"):
    self.onReload()
    evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
    tester = eval(evalString)
    tester.runTest()

#
# Slicer4MinuteLogic
#

class Slicer4MinuteLogic:
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


class Slicer4MinuteTest(unittest.TestCase):
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
    self.test_Slicer4Minute1()

  def test_Slicer4Minute1(self):
    """ Tests parts of the Slicer4Minute tutorial.

    Currently testing 'Part 2' which covers volumes, models, visibility and clipping.
    """

    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=8466', 'slicer4minute.mrb', slicer.util.loadScene),
        )

    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        print('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        print('Loading %s...\n' % (name,))
        loader(filePath)
    self.delayDisplay('Finished with download and loading')

    # Testing "Part 2" of Tutorial
    #
    #
    self.delayDisplay('Testing Part 2 of the Tutorial')

    # check volume is loaded out of scene
    volumeNode = slicer.util.getNode(pattern="grayscale")
    logic = Slicer4MinuteLogic()
    self.assertTrue( logic.hasImageData(volumeNode) )

    # check the slice planes
    red = slicer.util.getNode(pattern="vtkMRMLSliceNode1")
    red.SetSliceVisible(1)

    green = slicer.util.getNode(pattern="vtkMRMLSliceNode3")
    green.SetSliceVisible(1)

    # rotate a bit
    cam = slicer.util.getNode(pattern='vtkMRMLCameraNode1')
    cam.GetCamera().Azimuth(90)
    cam.GetCamera().Elevation(20)

    # turn off skin and skull
    skin = slicer.util.getNode(pattern='Skin.vtk')
    skin.GetDisplayNode().SetVisibility(0)

    skull = slicer.util.getNode(pattern='skull_bone.vtk')
    skull.GetDisplayNode().SetVisibility(0)

    # clip the model hemispheric_white_matter.vtk
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModule('Models')

    models = slicer.util.getModule('Models')
    logic = models.logic()

    hemispheric_white_matter = slicer.util.getNode(pattern='hemispheric_white_matter.vtk')
    hemispheric_white_matter.GetDisplayNode().SetClipping(1)

    clip = slicer.util.getNode(pattern='vtkMRMLClipModelsNode1')
    clip.SetRedSliceClipState(0)
    clip.SetYellowSliceClipState(0)
    clip.SetGreenSliceClipState(2)

    # Can we make this more than just a Smoke Test?
    self.delayDisplay('Optic chiasm should be visible. Front part of white matter should be clipped.')

    # Done
    #
    #
    self.delayDisplay('Test passed!')

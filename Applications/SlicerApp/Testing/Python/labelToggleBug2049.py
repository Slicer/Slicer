import os
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# labelToggleBug2049
#

class labelToggleBug2049:
  def __init__(self, parent):
    parent.title = "labelToggleBug2049" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Jean-Christophe Fillion-Robin (Kitware), Steve Pieper (Isomics)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    This is an example of scripted loadable module bundled in an extension.
    """
    parent.acknowledgementText = """
    This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc. and Steve Pieper, Isomics, Inc.  and was partially funded by NIH grant 3P41RR013218-12S1.
""" # replace with organization, grant and thanks.
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['labelToggleBug2049'] = self.runTest

  def runTest(self):
    tester = labelToggleBug2049Test()
    tester.runTest()

#
# qlabelToggleBug2049Widget
#

class labelToggleBug2049Widget:
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
    self.reloadButton.name = "labelToggleBug2049 Reload"
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

  def onReload(self,moduleName="labelToggleBug2049"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="labelToggleBug2049"):
    self.onReload()
    evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
    tester = eval(evalString)
    tester.runTest()

#
# labelToggleBug2049Logic
#

class labelToggleBug2049Logic:
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


class labelToggleBug2049Test(unittest.TestCase):
  """
  This is the test case for your scripted module.
  """

  def delayDisplay(self,message,msec=1000):
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
    self.test_labelToggleBug20491()

  def test_labelToggleBug20491(self):
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

    #
    # this module exists to implement a test for bug #2409
    # http://na-mic.org/Bug/view.php?id=2409
    #

    self.delayDisplay("Running the test")
    #
    # first, get the data
    #
    import urllib
    downloads = (
        ('http://www.slicer.org/slicerWiki/images/4/43/MR-head.nrrd', 'MR-head.nrrd', slicer.util.loadVolume),
        ('http://www.slicer.org/slicerWiki/images/4/43/MR-head.nrrd', 'MR-head_1.nrrd', slicer.util.loadVolume),
        )

    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        print('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        print('Loading %s...\n' % (name,))
        loader(filePath)
    self.delayDisplay('Finished with download and loading\n')

    # check the data
    logic = labelToggleBug2049Logic()
    volumeNode = slicer.util.getNode(pattern="MR-head")
    self.assertTrue( logic.hasImageData(volumeNode) )
    volumeNode_1 = slicer.util.getNode(pattern="MR-head_1")
    self.assertTrue( logic.hasImageData(volumeNode_1) )
    self.delayDisplay("Volumes check out okay")

    # use the volumes module to replicate the bug report
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModule('Volumes')
    self.delayDisplay("Entered Volumes module")

    selector = slicer.util.findChildren(name='ActiveVolumeNodeSelector')[0]
    checkBox = slicer.util.findChildren(name='LabelMapCheckBox')[0]

    selector.setCurrentNode(volumeNode)
    checkBox.checked = True
    self.delayDisplay("Made the volumeNode into a label map")

    # set the label map layer
    appLogic = slicer.app.applicationLogic()
    selectionNode = appLogic.GetSelectionNode()
    selectionNode.SetReferenceActiveVolumeID( volumeNode_1.GetID() )
    selectionNode.SetReferenceActiveLabelVolumeID( volumeNode.GetID() )
    appLogic.PropagateVolumeSelection(0)
    self.delayDisplay("Put volumes into layers")

    # turn off the label map attribue
    selector.setCurrentNode(volumeNode)
    checkBox.checked = False
    self.delayDisplay("Made the volumeNode into a non-label map")

    self.delayDisplay('Did we crash?')

    self.delayDisplay('Test passed!')

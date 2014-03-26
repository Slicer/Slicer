import os
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# AtlasTests
#

class AtlasTests:
  def __init__(self, parent):
    parent.title = "AtlasTests" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Steve Pieper (Isomics)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    This is a self test that downloads and displays volumetric atlases from the NA-MIC publication database.

    For more information:

    Abdominal Atlas: <a>http://www.slicer.org/publications/item/view/1918</a>
    Brain Atlas: <a>http://www.slicer.org/publications/item/view/2037</a>
    Knee Atlas: <a>http://www.slicer.org/publications/item/view/1953</a>

    """
    parent.acknowledgementText = """
    This file was originally developed by Steve Pieper, Isomics, Inc. and was partially funded by NIH grant 3P41RR013218-12S1.
""" # replace with organization, grant and thanks.
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['AtlasTests'] = self.runTest

  def runTest(self):
    tester = AtlasTestsTest()
    tester.runTest()

#
# qAtlasTestsWidget
#

class AtlasTestsWidget:
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
    self.reloadButton.name = "AtlasTests Reload"
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
    atlasTests = ctk.ctkCollapsibleButton()
    atlasTests.text = "Atlas Tests"
    self.layout.addWidget(atlasTests)

    # Layout within the dummy collapsible button
    dummyFormLayout = qt.QFormLayout(atlasTests)

    # run Abdominal Test
    self.abdominalAtlas = qt.QPushButton("Run Abdominal Test")
    self.abdominalAtlas.toolTip = "Downloads abdominal atlas and loads it."
    dummyFormLayout.addWidget(self.abdominalAtlas)
    self.abdominalAtlas.connect('clicked(bool)', self.onAbdominalAtlas)

    # run brain Test
    self.brainAtlas = qt.QPushButton("Run Brain Test")
    self.brainAtlas.toolTip = "Downloads brain atlas and loads it."
    dummyFormLayout.addWidget(self.brainAtlas)
    self.brainAtlas.connect('clicked(bool)', self.onBrainAtlas)

    # run knee Test
    self.kneeAtlas = qt.QPushButton("Run Knee Test")
    self.kneeAtlas.toolTip = "Downloads knee atlas and loads it."
    dummyFormLayout.addWidget(self.kneeAtlas)
    self.kneeAtlas.connect('clicked(bool)', self.onKneeAtlas)

    # Add vertical spacer
    self.layout.addStretch(1)

  def onAbdominalAtlas(self):
    tester = AtlasTestsTest()
    tester.runAbdominalTest()

  def onBrainAtlas(self):
    tester = AtlasTestsTest()
    tester.runBrainTest()

  def onKneeAtlas(self):
    tester = AtlasTestsTest()
    tester.runKneeTest()

  def onReload(self,moduleName="AtlasTests"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="AtlasTests"):
    self.onReload()
    evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
    tester = eval(evalString)
    tester.runTest()

#
# AtlasTestsLogic
#

class AtlasTestsLogic:
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


class AtlasTestsTest(unittest.TestCase):
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
    self.test_AbdominalAtlasTest()
    self.setUp()
    self.test_BrainAtlasTest()
    self.setUp()
    self.test_KneeAtlasTest()

  def runAbdominalTest(self):
    self.setUp()
    self.test_AbdominalAtlasTest()

  def runBrainTest(self):
    self.setUp()
    self.test_BrainAtlasTest()

  def runKneeTest(self):
    self.setUp()
    self.test_KneeAtlasTest()

  def test_AbdominalAtlasTest(self):
    self.delayDisplay('Running Abdominal Atlas Test')
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=8301', 'Abdominal_Atlas_2012.mrb', slicer.util.loadScene),
        )
    self.perform_AtlasTest(downloads,'I')

  def test_BrainAtlasTest(self):
    self.delayDisplay('Running Brain Atlas Test')
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=10937', 'BrainAtlas2012.mrb', slicer.util.loadScene),
        )
    self.perform_AtlasTest(downloads,'A1_grayT1')

  def test_KneeAtlasTest(self):
    self.delayDisplay('Running Knee Atlas Test')
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=9912', 'KneeAtlas2012.mrb', slicer.util.loadScene),
        )
    self.perform_AtlasTest(downloads,'I')

  def perform_AtlasTest(self, downloads,testVolumePattern):
    """ Perform the actual atlas test.
    This includes: download and load the given data, touch all
    model hierarchies, and restore all scene views.
    downloads : list of lists of: url, file save name, load callable
    testVolumePattern : volume name/id that is tested for valid load
    """

    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import urllib

    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        print('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        print('Loading %s...\n' % (name,))
        loader(filePath)
    self.delayDisplay('Finished with download and loading\n')

    volumeNode = slicer.util.getNode(pattern=testVolumePattern)
    logic = AtlasTestsLogic()
    self.assertTrue( logic.hasImageData(volumeNode) )

    m = slicer.util.mainWindow()

    # go to the models module
    m.moduleSelector().selectModule('Models')
    self.delayDisplay("Entered Models module")

    # get model hierarchy nodes that have children hierarchies
    numModelHierarchies = slicer.mrmlScene.GetNumberOfNodesByClass("vtkMRMLModelHierarchyNode")
    # get the number that we'll be manipulating
    numModelHierarchiesToManipulate = 0
    for h in range(numModelHierarchies):
      mh = slicer.mrmlScene.GetNthNodeByClass(h, "vtkMRMLModelHierarchyNode")
      if mh.GetNumberOfChildrenNodes() > 0 and mh.GetDisplayNode() != None:
        numModelHierarchiesToManipulate += 1
    # iterate over all the hierarchies
    hierarchyManipulating = 0
    for h in range(numModelHierarchies):
      mh = slicer.mrmlScene.GetNthNodeByClass(h, "vtkMRMLModelHierarchyNode")
      numChildren = mh.GetNumberOfChildrenNodes()
      if numChildren > 0:
        mhd = mh.GetDisplayNode()
        # manually added hierarchies may not have display nodes, skip
        if mhd == None:
          self.delayDisplay("Skipping model hierarchy with no display node " + mh.GetName())
        else:
          hierarchyManipulating += 1
          self.delayDisplay("Manipulating model hierarchy " + mh.GetName() + " (" + str(hierarchyManipulating) + "/" + str(numModelHierarchiesToManipulate) + ")")
          hierarchyOriginalColour = mhd.GetColor()
          hierarchyOriginalVisibility = mhd.GetVisibility()
          hierarchyOriginalExpanded = mh.GetExpanded()
          # collapse and change the colour on the hierarchy to full red
          mh.SetExpanded(0)
          self.delayDisplay("Model hierarchy " + mh.GetName() + ": expanded = false",msec=10)
          mhd.SetColor(1,0,0)
          self.delayDisplay("Model hierarchy " + mh.GetName() + ": color = red",msec=10)
          # set the collapsed visibility to 0
          mhd.SetVisibility(0)
          self.delayDisplay("Model hierarchy " + mh.GetName() + ": visibility = off",msec=10)
          # expand, should see all models in correct colour
          mh.SetExpanded(1)
          self.delayDisplay("Model hierarchy " + mh.GetName() + ": expanded = true",msec=10)
          # reset the hierarchy
          mhd.SetVisibility(hierarchyOriginalVisibility)
          mhd.SetColor(hierarchyOriginalColour)
          mh.SetExpanded(hierarchyOriginalExpanded)

    # go to the scene views module
    m.moduleSelector().selectModule('SceneViews')
    self.delayDisplay("Entered Scene Views module")

    # iterate over the scene views and restore them
    numSceneViews = slicer.mrmlScene.GetNumberOfNodesByClass("vtkMRMLSceneViewNode")
    for s in range(numSceneViews):
      sv = slicer.mrmlScene.GetNthNodeByClass(s, "vtkMRMLSceneViewNode")
      self.delayDisplay("Restoring scene " + sv.GetName() + " (" + str(s+1) + "/" + str(numSceneViews) + ")",msec=500)
      sv.RestoreScene()

    self.delayDisplay('Test passed!')

from __future__ import print_function
import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
from slicer.util import TESTING_DATA_URL

#
# AtlasTests
#

class AtlasTests(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
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

#
# qAtlasTestsWidget
#

class AtlasTestsWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    # Instantiate and connect widgets ...

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

#
# AtlasTestsLogic
#

class AtlasTestsLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget.
  Uses ScriptedLoadableModuleLogic base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def hasImageData(self,volumeNode):
    """This is a dummy logic method that
    returns true if the passed in volume
    node has valid image data
    """
    if not volumeNode:
      print('no volume node')
      return False
    if volumeNode.GetImageData() is None:
      print('no image data')
      return False
    return True


class AtlasTestsTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

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
    downloads = {
      'fileNames': 'Abdominal_Atlas_2012.mrb',
      'loadFiles': True,
      'uris': TESTING_DATA_URL + 'SHA256/5d315abf7d303326669c6075f9eea927eeda2e531a5b1662cfa505806cb498ea',
      'checksums': 'SHA256:5d315abf7d303326669c6075f9eea927eeda2e531a5b1662cfa505806cb498ea',
      }
    self.perform_AtlasTest(downloads,'I')

  def test_BrainAtlasTest(self):
    self.delayDisplay('Running Brain Atlas Test')
    downloads = {
      'fileNames': 'BrainAtlas2012.mrb',
      'loadFiles': True,
      'uris': TESTING_DATA_URL + 'SHA256/688ebcc6f45989795be2bcdc6b8b5bfc461f1656d677ed3ddef8c313532687f1',
      'checksums': 'SHA256:688ebcc6f45989795be2bcdc6b8b5bfc461f1656d677ed3ddef8c313532687f1',
      }
    self.perform_AtlasTest(downloads,'A1_grayT1')

  def test_KneeAtlasTest(self):
    self.delayDisplay('Running Knee Atlas Test')
    downloads = {
      'fileNames': 'KneeAtlas2012.mrb',
      'loadFiles': True,
      'uris': TESTING_DATA_URL + 'SHA256/5d5506c07c238918d0c892e7b04c26ad7f43684d89580780bb207d1d860b0b33',
      'checksums': 'SHA256:5d5506c07c238918d0c892e7b04c26ad7f43684d89580780bb207d1d860b0b33',
      }
    self.perform_AtlasTest(downloads,'I')

  def perform_AtlasTest(self, downloads, testVolumePattern):
    """ Perform the actual atlas test.
    This includes: download and load the given data, touch all
    model hierarchies, and restore all scene views.
    downloads : dictionnary of URIs and fileNames
    testVolumePattern : volume name/id that is tested for valid load
    """

    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import SampleData
    SampleData.downloadFromURL(**downloads)
    self.delayDisplay('Finished with download and loading\n')

    volumeNode = slicer.util.getNode(pattern=testVolumePattern)
    logic = AtlasTestsLogic()
    self.assertIsNotNone( logic.hasImageData(volumeNode) )

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
      if mh.GetNumberOfChildrenNodes() > 0 and mh.GetDisplayNode() is not None:
        numModelHierarchiesToManipulate += 1
    # iterate over all the hierarchies
    hierarchyManipulating = 0
    for h in range(numModelHierarchies):
      mh = slicer.mrmlScene.GetNthNodeByClass(h, "vtkMRMLModelHierarchyNode")
      numChildren = mh.GetNumberOfChildrenNodes()
      if numChildren > 0:
        mhd = mh.GetDisplayNode()
        # manually added hierarchies may not have display nodes, skip
        if mhd is None:
          self.delayDisplay("Skipping model hierarchy with no display node " + mh.GetName())
        else:
          hierarchyManipulating += 1
          self.delayDisplay("Manipulating model hierarchy " + mh.GetName() + " (" + str(hierarchyManipulating) + "/" + str(numModelHierarchiesToManipulate) + ")")
          hierarchyOriginalColour = mhd.GetColor()
          hierarchyOriginalVisibility = mhd.GetVisibility()
          hierarchyOriginalExpanded = mh.GetExpanded()
          # collapse and change the colour on the hierarchy to full red
          mh.SetExpanded(0)
          self.delayDisplay("Model hierarchy " + mh.GetName() + ": expanded = false")
          mhd.SetColor(1,0,0)
          self.delayDisplay("Model hierarchy " + mh.GetName() + ": color = red")
          # set the collapsed visibility to 0
          mhd.SetVisibility(0)
          self.delayDisplay("Model hierarchy " + mh.GetName() + ": visibility = off")
          # expand, should see all models in correct colour
          mh.SetExpanded(1)
          self.delayDisplay("Model hierarchy " + mh.GetName() + ": expanded = true")
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
      self.delayDisplay("Restoring scene " + sv.GetName() + " (" + str(s+1) + "/" + str(numSceneViews) + ")")
      sv.RestoreScene()

    self.delayDisplay('Test passed!')

import os
import unittest
import vtk, qt, ctk, slicer
import logging
from slicer.ScriptedLoadableModule import *
from slicer.util import DATA_STORE_URL

class SubjectHierarchyFoldersTest1(unittest.TestCase):

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_SubjectHierarchyFoldersTest1()

  #------------------------------------------------------------------------------
  def test_SubjectHierarchyFoldersTest1(self):
    # Check for modules
    self.assertIsNotNone( slicer.modules.subjecthierarchy )

    self.TestSection_InitializeTest()
    self.TestSection_LoadTestData()
    self.TestSection_FolderVisibility()
    self.TestSection_ApplyDisplayPropertiesOnBranch()
    self.TestSection_FolderDisplayOverrideAllowed()

    logging.info('Test finished')

  #------------------------------------------------------------------------------
  def TestSection_InitializeTest(self):
    #
    # Define variables
    #

    # A certain model to test that is both in the brain and the midbrain folders
    self.testModelNodeName = 'Model_79_left_red_nucleus'
    self.testModelNode = None
    self.overrideColor = [255, 255, 0]

    # Get subject hierarchy node
    self.shNode = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
    self.assertIsNotNone(self.shNode)
    # Get folder plugin
    pluginHandler = slicer.qSlicerSubjectHierarchyPluginHandler().instance()
    self.assertIsNotNone(pluginHandler)
    self.folderPlugin = pluginHandler.pluginByName('Folder')
    self.assertIsNotNone(self.folderPlugin)

    #
    # Initialize test
    #

    # Create 3D view
    self.layoutName = "Test3DView"
    # ownerNode manages this view instead of the layout manager (it can be any node in the scene)
    self.viewOwnerNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScriptedModuleNode")
    self.viewNode = slicer.vtkMRMLViewNode()
    self.viewNode.SetName(self.layoutName)
    self.viewNode.SetLayoutName(self.layoutName)
    self.viewNode.SetLayoutColor(1, 1, 0)
    self.viewNode.SetAndObserveParentLayoutNodeID(self.viewOwnerNode.GetID())
    self.viewNode = slicer.mrmlScene.AddNode(self.viewNode)
    self.viewWidget = slicer.qMRMLThreeDWidget()
    self.viewWidget.setMRMLScene(slicer.mrmlScene)
    self.viewWidget.setMRMLViewNode(self.viewNode)
    self.viewWidget.show()

    # Get model displayable manager for the 3D view
    self.modelDisplayableManager = self.getModelDisplayableManager()
    self.assertIsNotNone(self.modelDisplayableManager)

  #------------------------------------------------------------------------------
  def TestSection_LoadTestData(self):
    # Load NAC Brain Atlas 2015 (https://slicer.kitware.com/midas3/item/121588)
    logging.info('Test section: Load NAC Brain Atlas 2015')

    import SampleData
    sceneFile = SampleData.downloadFromURL(
      fileNames='NACBrainAtlas2015.mrb',
      # Note: this data set is from SlicerDataStore (not from SlicerTestingData) repository
      uris=DATA_STORE_URL + 'SHA256/d69d0331d4fd2574be1459b7734921f64f5872d3cb9589ec01b2f53dadc7112f',
      checksums='SHA256:d69d0331d4fd2574be1459b7734921f64f5872d3cb9589ec01b2f53dadc7112f')[0]

    ioManager = slicer.app.ioManager()
    ioManager.loadFile(sceneFile)

    # Check number of models to see if atlas was fully loaded
    self.assertEqual(298, slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLModelNode')) # 301 with main window due to the slice views

    # Check number of model hierarchy nodes to make sure all of them were converted
    self.assertEqual(0, slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLModelHierarchyNode'))
    # Check number of folder display nodes, which is zero until branch display related functions are used
    self.assertEqual(0, slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLFolderDisplayNode'))

    # Check number of folder items
    numberOfFolderItems = 0
    allItems = vtk.vtkIdList()
    self.shNode.GetItemChildren(self.shNode.GetSceneItemID(), allItems, True)
    for index in range(allItems.GetNumberOfIds()):
      currentItem = allItems.GetId(index)
      if self.shNode.IsItemLevel(currentItem, slicer.vtkMRMLSubjectHierarchyConstants.GetSubjectHierarchyLevelFolder()):
        numberOfFolderItems += 1
    self.assertEqual(80, numberOfFolderItems)

  #------------------------------------------------------------------------------
  def TestSection_FolderVisibility(self):
    # Test apply display properties on branch feature
    logging.info('Test section: Folder visibility')

    # Get folder that contains the whole brain
    brainFolderItem = self.shNode.GetItemByName('Brain')
    self.assertNotEqual(brainFolderItem, 0)

    # Check number of visible models
    modelNodes = slicer.util.getNodes('vtkMRMLModelNode*')
    modelNodesList = list(modelNodes.values())
    numberOfVisibleModels = 0
    for modelNode in modelNodesList:
      displayNode = modelNode.GetDisplayNode()
      actor = self.modelDisplayableManager.GetActorByID(displayNode.GetID())
      if actor.GetVisibility() > 0:
        numberOfVisibleModels += 1
    self.assertEqual(225, numberOfVisibleModels)

    # Check model node hierarchy visibility
    self.testModelNode = slicer.util.getNode(self.testModelNodeName)
    self.assertIsNotNone(self.testModelNode)

    testModelHierarchyVisibility = slicer.vtkMRMLFolderDisplayNode.GetHierarchyVisibility(self.testModelNode)
    self.assertTrue(testModelHierarchyVisibility)

    # Hide branch using the folder plugin
    self.startTiming()
    self.folderPlugin.setDisplayVisibility(brainFolderItem, 0)
    logging.info('Time of hiding whole brain: ' + str(self.stopTiming()/1000) + ' s')

    # Check if a folder display node was indeed created when changing display property on the folder
    self.assertEqual(1, slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLFolderDisplayNode'))
    brainFolderDisplayNode = self.shNode.GetItemDataNode(brainFolderItem)
    self.assertIsNotNone(brainFolderDisplayNode)

    # Check model node hierarchy visibility
    testModelHierarchyVisibility = slicer.vtkMRMLFolderDisplayNode.GetHierarchyVisibility(self.testModelNode)
    self.assertFalse(testModelHierarchyVisibility)

    # Check if brain models were indeed hidden
    numberOfVisibleModels = 0
    for modelNode in modelNodesList:
      displayNode = modelNode.GetDisplayNode()
      actor = self.modelDisplayableManager.GetActorByID(displayNode.GetID())
      if actor.GetVisibility() > 0:
        numberOfVisibleModels += 1
    self.assertEqual(3, numberOfVisibleModels)

    # Show folder again
    self.startTiming()
    self.folderPlugin.setDisplayVisibility(brainFolderItem, 1)
    logging.info('Time of showing whole brain: ' + str(self.stopTiming()/1000) + ' s')

    # Check number of visible models
    numberOfVisibleModels = 0
    for modelNode in modelNodesList:
      displayNode = modelNode.GetDisplayNode()
      actor = self.modelDisplayableManager.GetActorByID(displayNode.GetID())
      if actor.GetVisibility() > 0:
        numberOfVisibleModels += 1
    self.assertEqual(225, numberOfVisibleModels)

    # Check model node hierarchy visibility
    testModelHierarchyVisibility = slicer.vtkMRMLFolderDisplayNode.GetHierarchyVisibility(self.testModelNode)
    self.assertTrue(testModelHierarchyVisibility)

  #------------------------------------------------------------------------------
  def TestSection_ApplyDisplayPropertiesOnBranch(self):
    # Test apply display properties on branch feature
    logging.info('Test section: Apply display properties on branch')

    # Get folder that contains the midbrain
    midbrainFolderItem = self.shNode.GetItemByName('midbrain')
    self.assertNotEqual(midbrainFolderItem, 0)

    # Test simple color override, check actor color
    overrideColorQt = qt.QColor(self.overrideColor[0], self.overrideColor[1], self.overrideColor[2])
    self.startTiming()
    self.folderPlugin.setDisplayColor(midbrainFolderItem, overrideColorQt, {})
    logging.info('Time of setting override color on midbrain branch: ' + str(self.stopTiming()/1000) + ' s')

    # Check number of models with overridden color
    numberOfOverriddenMidbrainModels = 0
    testModelNodeOverridden = False
    midbrainModelItems = vtk.vtkIdList()
    self.shNode.GetItemChildren(midbrainFolderItem, midbrainModelItems, True)
    for index in range(midbrainModelItems.GetNumberOfIds()):
      currentMidbrainModelItem = midbrainModelItems.GetId(index)
      currentMidbrainModelNode = self.shNode.GetItemDataNode(currentMidbrainModelItem)
      if currentMidbrainModelNode: # The child item can be a folder as well, in which case there is no model node
        displayNode = currentMidbrainModelNode.GetDisplayNode()
        actor = self.modelDisplayableManager.GetActorByID(displayNode.GetID())
        currentColor = actor.GetProperty().GetColor()
        if (currentColor[0] == self.overrideColor[0]/255 and
            currentColor[1] == self.overrideColor[1]/255 and
            currentColor[2] == self.overrideColor[2]/255):
          if currentMidbrainModelNode is self.testModelNode:
            testModelNodeOverridden = True
          numberOfOverriddenMidbrainModels += 1
    self.assertEqual(6, numberOfOverriddenMidbrainModels)
    self.assertTrue(testModelNodeOverridden)

    # Test hierarchy opacity
    testModelHierarchyOpacity = slicer.vtkMRMLFolderDisplayNode.GetHierarchyOpacity(self.testModelNode)
    self.assertEqual(testModelHierarchyOpacity, 1.0)

    midbrainFolderDisplayNode = self.shNode.GetItemDataNode(midbrainFolderItem)
    self.assertIsNotNone(midbrainFolderDisplayNode)
    midbrainFolderDisplayNode.SetOpacity(0.5)

    testModelHierarchyOpacity = slicer.vtkMRMLFolderDisplayNode.GetHierarchyOpacity(self.testModelNode)
    self.assertEqual(testModelHierarchyOpacity, 0.5)

    brainFolderItem = self.shNode.GetItemByName('Brain')
    self.assertNotEqual(brainFolderItem, 0)
    brainFolderDisplayNode = self.shNode.GetItemDataNode(brainFolderItem)
    self.assertIsNotNone(brainFolderDisplayNode)
    brainFolderDisplayNode.SetOpacity(0.5)

    testModelHierarchyOpacity = slicer.vtkMRMLFolderDisplayNode.GetHierarchyOpacity(self.testModelNode)
    self.assertEqual(testModelHierarchyOpacity, 0.25)

  #------------------------------------------------------------------------------
  def TestSection_FolderDisplayOverrideAllowed(self):
    # Test exclusion of a node from the apply display properties feature
    logging.info('Test section: Disable apply display properties using FolderDisplayOverrideAllowed')

    testModelDisplayNode = self.testModelNode.GetDisplayNode()
    self.assertTrue(testModelDisplayNode.GetFolderDisplayOverrideAllowed())

    # Turn of override allowed
    testModelDisplayNode.SetFolderDisplayOverrideAllowed(False)

    # Check that color and opacity are not overridden
    testModelActor = self.modelDisplayableManager.GetActorByID(testModelDisplayNode.GetID())

    testModelCurrentColor = testModelActor.GetProperty().GetColor()
    colorOverridden = False
    if (testModelCurrentColor[0] == self.overrideColor[0]/255 and
        testModelCurrentColor[1] == self.overrideColor[1]/255 and
        testModelCurrentColor[2] == self.overrideColor[2]/255):
      colorOverridden = True
    self.assertFalse(colorOverridden)

    testModelCurrentOpacity = testModelActor.GetProperty().GetOpacity()
    self.assertEqual(testModelCurrentOpacity, 1.0)

  #------------------------------------------------------------------------------
  def startTiming(self):
    self.timer = qt.QTime()
    self.timer.start()

  #------------------------------------------------------------------------------
  def stopTiming(self):
    return self.timer.elapsed()

  #------------------------------------------------------------------------------
  def getModelDisplayableManager(self):
    if self.viewWidget is None:
      logging.error('View widget is not created')
      return None
    managers = vtk.vtkCollection()
    self.viewWidget.getDisplayableManagers(managers)
    for i in range(managers.GetNumberOfItems()):
      obj = managers.GetItemAsObject(i)
      if obj.IsA('vtkMRMLModelDisplayableManager'):
        return obj
    logging.error('Failed to find the model displayable manager')
    return None

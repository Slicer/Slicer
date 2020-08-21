from __future__ import print_function
import logging
import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
from DICOMLib import DICOMUtils
from slicer.util import TESTING_DATA_URL

#
# SubjectHierarchyGenericSelfTest
#

class SubjectHierarchyGenericSelfTest(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "SubjectHierarchyGenericSelfTest"
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = ["SubjectHierarchy", "DICOM"]
    parent.contributors = ["Csaba Pinter (Queen's)"]
    parent.helpText = """
    This is a self test for the Subject hierarchy module generic features.
    """
    parent.acknowledgementText = """
This file was originally developed by Csaba Pinter, PerkLab, Queen's University and was supported through the Applied Cancer
 Research Unit program of Cancer Care Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care"""
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['SubjectHierarchyGenericSelfTest'] = self.runTest

  def runTest(self, msec=100, **kwargs):
    tester = SubjectHierarchyGenericSelfTestTest()
    tester.runTest()

#
# SubjectHierarchyGenericSelfTestWidget
#

class SubjectHierarchyGenericSelfTestWidget(ScriptedLoadableModuleWidget):
  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

#
# SubjectHierarchyGenericSelfTestLogic
#

class SubjectHierarchyGenericSelfTestLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass


class SubjectHierarchyGenericSelfTestTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

    self.delayMs = 700

    #TODO: Comment out (sample code for debugging)
    #logFile = open('d:/pyTestLog.txt', 'w')
    #logFile.write(repr(slicer.modules.subjecthierarchygenericselftest) + '\n')
    #logFile.write(repr(slicer.modules.subjecthierarchy) + '\n')
    #logFile.close()

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_SubjectHierarchyGenericSelfTest_FullTest1()

  # ------------------------------------------------------------------------------
  def test_SubjectHierarchyGenericSelfTest_FullTest1(self):
    # Check for SubjectHierarchy module
    self.assertIsNotNone( slicer.modules.subjecthierarchy )

    # Switch to subject hierarchy module so that the changes can be seen as the test goes
    slicer.util.selectModule('SubjectHierarchy')

    self.section_SetupPathsAndNames()
    self.section_ClearScene()
    self.section_LoadDicomDataWitchBatchProcessing()
    self.section_SaveScene()
    self.section_AddNodeToSubjectHierarchy()
    self.section_CLI()
    self.section_CreateSecondBranch()
    self.section_AttributeFilter()
    self.section_ReparentNodeInSubjectHierarchy()
    self.section_LoadScene()
    self.section_TestCircularParenthood()

    logging.info('Test finished')


  # ------------------------------------------------------------------------------
  def section_SetupPathsAndNames(self):
    # Set constants
    subjectHierarchyGenericSelfTestDir = slicer.app.temporaryPath + '/SubjectHierarchyGenericSelfTest'
    print('Test directory: ' + subjectHierarchyGenericSelfTestDir)
    if not os.access(subjectHierarchyGenericSelfTestDir, os.F_OK):
      os.mkdir(subjectHierarchyGenericSelfTestDir)

    self.dicomDataDir = subjectHierarchyGenericSelfTestDir + '/DicomData'
    if not os.access(self.dicomDataDir, os.F_OK):
      os.mkdir(self.dicomDataDir)

    self.dicomDatabaseDir = subjectHierarchyGenericSelfTestDir + '/CtkDicomDatabase'
    self.dicomZipFileUrl = TESTING_DATA_URL + 'SHA256/1aa0bb177bbf6471ca5f2192340a6cecdedb81b33506b03ff316c6b5f624e863'
    self.dicomZipChecksum = 'SHA256:1aa0bb177bbf6471ca5f2192340a6cecdedb81b33506b03ff316c6b5f624e863'
    self.dicomZipFilePath = subjectHierarchyGenericSelfTestDir + '/TestDicomCT.zip'
    self.expectedNumOfFilesInDicomDataDir = 10
    self.tempDir = subjectHierarchyGenericSelfTestDir + '/Temp'
    self.sceneFileName = self.tempDir + '/SubjectHierarchyGenericSelfTestScene.mrml'

    self.invalidItemID = slicer.vtkMRMLSubjectHierarchyNode.GetInvalidItemID()

    self.loadedDicomStudyName = 'No study description (20110101)'
    self.loadedDicomVolumeName = '303: Unnamed Series'
    self.patientItemID = self.invalidItemID # To be filled in after adding
    self.patientOriginalName = ''
    self.patientNewName = 'TestPatient_1'
    self.studyItemID = self.invalidItemID
    self.studyOriginalName = ''
    self.studyNewName = 'No study description (20170107)'
    self.ctVolumeShItemID = self.invalidItemID
    self.ctVolumeOriginalName = ''
    self.ctVolumeNewName = '404: Unnamed Series'
    self.sampleLabelmapName = 'SampleLabelmap'
    self.sampleLabelmapNode = None
    self.sampleLabelmapShItemID = self.invalidItemID
    self.sampleModelName = 'SampleModel'
    self.sampleModelNode = None
    self.sampleModelShItemID = self.invalidItemID
    self.patient2Name = 'Patient2'
    self.patient2ItemID = self.invalidItemID
    self.study2Name = 'Study2'
    self.study2ItemID = self.invalidItemID
    self.folderName = 'Folder'
    self.folderItemID = self.invalidItemID

  # ------------------------------------------------------------------------------
  def section_ClearScene(self):
    self.delayDisplay("Clear scene",self.delayMs)

    # Clear the scene to make sure there is no crash (closing scene is a sensitive operation)
    slicer.mrmlScene.Clear(0)

    # Make sure there is only one subject hierarchy node after closing the scene
    self.assertEqual( slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLSubjectHierarchyNode'), 1 )

    shNode = slicer.mrmlScene.GetSubjectHierarchyNode()
    self.assertIsNotNone( shNode )

  # ------------------------------------------------------------------------------
  def section_LoadDicomDataWitchBatchProcessing(self):
    try:
      # Open Data module so that a subject hierarchy scene model is active
      # (which caused problems with batch processing)
      slicer.util.selectModule('Data')

      # Open test database and empty it
      with DICOMUtils.TemporaryDICOMDatabase(self.dicomDatabaseDir) as db:
        self.assertTrue( db.isOpen )
        self.assertEqual( slicer.dicomDatabase, db)

        slicer.mrmlScene.StartState(slicer.vtkMRMLScene.BatchProcessState)

        # Download, unzip, import, and load data. Verify loaded nodes.
        loadedNodes = {'vtkMRMLScalarVolumeNode':1}
        with DICOMUtils.LoadDICOMFilesToDatabase( \
            self.dicomZipFileUrl, self.dicomZipFilePath, \
            self.dicomDataDir, self.expectedNumOfFilesInDicomDataDir, \
            {}, loadedNodes, checksum=self.dicomZipChecksum) as success:
          self.assertTrue(success)

        slicer.mrmlScene.EndState(slicer.vtkMRMLScene.BatchProcessState)

      self.assertEqual( len( slicer.util.getNodes('vtkMRMLSubjectHierarchyNode*') ), 1 )

      shNode = slicer.mrmlScene.GetSubjectHierarchyNode()
      self.assertIsNotNone( shNode )
      loadedDicomVolumeItemID = shNode.GetItemByName(self.loadedDicomVolumeName)
      loadedDicomStudyItemID = shNode.GetItemByName(self.loadedDicomStudyName)
      self.assertEqual( shNode.GetItemParent(loadedDicomVolumeItemID), loadedDicomStudyItemID )

    except Exception as e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e),self.delayMs*2)

  # ------------------------------------------------------------------------------
  def section_SaveScene(self):
    self.delayDisplay("Save scene",self.delayMs)

    if not os.access(self.tempDir, os.F_OK):
      os.mkdir(self.tempDir)

    if os.access(self.sceneFileName, os.F_OK):
      os.remove(self.sceneFileName)

    # Save MRML scene into file
    slicer.mrmlScene.Commit(self.sceneFileName)
    logging.info('Scene saved into ' + self.sceneFileName)

    readable = os.access(self.sceneFileName, os.R_OK)
    self.assertTrue( readable )

  # ------------------------------------------------------------------------------
  def section_AddNodeToSubjectHierarchy(self):
    self.delayDisplay("Add node to subject hierarchy",self.delayMs)

    # Get volume previously loaded from DICOM
    volumeNodes = list(slicer.util.getNodes('vtkMRMLScalarVolumeNode*').values())
    ctVolumeNode = volumeNodes[len(volumeNodes)-1]
    self.assertIsNotNone( ctVolumeNode )

    # Create sample labelmap and model and add them in subject hierarchy
    self.sampleLabelmapNode = self.createSampleLabelmapVolumeNode(ctVolumeNode, self.sampleLabelmapName, 2)
    sampleModelColor = [0.0, 1.0, 0.0]
    self.sampleModelNode = self.createSampleModelNode(self.sampleModelName, sampleModelColor, ctVolumeNode)

    # Get subject hierarchy scene model and node
    dataWidget = slicer.modules.data.widgetRepresentation()
    self.assertIsNotNone( dataWidget )
    shTreeView = slicer.util.findChild(dataWidget, name='SubjectHierarchyTreeView')
    self.assertIsNotNone( shTreeView )
    shModel = shTreeView.model()
    self.assertIsNotNone( shModel )
    shNode = slicer.mrmlScene.GetSubjectHierarchyNode()
    self.assertIsNotNone( shNode )

    # Get and check subject hierarchy items for the data nodes
    self.ctVolumeShItemID = shNode.GetItemByDataNode(ctVolumeNode)
    self.ctVolumeOriginalName = shNode.GetItemName(self.ctVolumeShItemID)
    self.assertIsNotNone( self.ctVolumeShItemID )

    self.sampleLabelmapShItemID = shNode.GetItemByDataNode(self.sampleLabelmapNode)
    self.assertIsNotNone( self.sampleLabelmapShItemID )
    self.assertEqual( shNode.GetItemOwnerPluginName(self.sampleLabelmapShItemID), 'LabelMaps' )

    self.sampleModelShItemID = shNode.GetItemByDataNode(self.sampleModelNode)
    self.assertIsNotNone( self.sampleModelShItemID )
    self.assertEqual( shNode.GetItemOwnerPluginName(self.sampleModelShItemID), 'Models' )

    # Save item IDs for scene load testing
    self.studyItemID = shNode.GetItemParent(self.ctVolumeShItemID)
    self.studyOriginalName = shNode.GetItemName(self.studyItemID)
    self.assertIsNotNone( self.studyItemID )

    self.patientItemID = shNode.GetItemParent(self.studyItemID)
    self.patientOriginalName = shNode.GetItemName(self.patientItemID)
    self.assertIsNotNone( self.patientItemID )

    # Verify DICOM levels
    self.assertEqual( shNode.GetItemLevel(self.patientItemID), slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMLevelPatient() )
    self.assertEqual( shNode.GetItemLevel(self.studyItemID), slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMLevelStudy() )
    self.assertEqual( shNode.GetItemLevel(self.ctVolumeShItemID), "" )

    # Add model and labelmap to the created study
    retVal1 = shModel.reparent(self.sampleLabelmapShItemID, self.studyItemID)
    self.assertTrue(retVal1)
    retVal2 = shModel.reparent(self.sampleModelShItemID, self.studyItemID)
    self.assertTrue(retVal2)
    qt.QApplication.processEvents()

  # ------------------------------------------------------------------------------
  def section_CLI(self):
    self.delayDisplay("Test command-line interface",self.delayMs)

    shNode = slicer.mrmlScene.GetSubjectHierarchyNode()
    self.assertIsNotNone( shNode )

    # Get CT volume
    ctVolumeNode = shNode.GetItemDataNode(self.ctVolumeShItemID)
    self.assertIsNotNone( ctVolumeNode )

    # Create output volume
    resampledVolumeNode = slicer.vtkMRMLScalarVolumeNode()
    resampledVolumeNode.SetName(ctVolumeNode.GetName() + '_Resampled_10x10x10mm')
    slicer.mrmlScene.AddNode(resampledVolumeNode)

    # Resample
    resampleParameters = { 'outputPixelSpacing':'24.5,24.5,11.5', 'interpolationType':'lanczos',
      'InputVolume':ctVolumeNode.GetID(), 'OutputVolume':resampledVolumeNode.GetID() }
    slicer.cli.run(slicer.modules.resamplescalarvolume, None, resampleParameters, wait_for_completion=True)
    self.delayDisplay("Wait for CLI logic to add result to same branch",self.delayMs)

    # Check if output is also under the same study node
    resampledVolumeItemID = shNode.GetItemByDataNode(resampledVolumeNode)
    self.assertIsNotNone( resampledVolumeItemID )
    self.assertEqual( shNode.GetItemParent(resampledVolumeItemID), self.studyItemID )

  # ------------------------------------------------------------------------------
  def section_CreateSecondBranch(self):
    self.delayDisplay("Create second branch in subject hierarchy",self.delayMs)

    shNode = slicer.mrmlScene.GetSubjectHierarchyNode()
    self.assertIsNotNone( shNode )

    # Create second patient, study, and a folder
    self.patient2ItemID = shNode.CreateSubjectItem(shNode.GetSceneItemID(), self.patient2Name)
    self.study2ItemID = shNode.CreateStudyItem(self.patient2ItemID, self.study2Name)
    self.folderItemID = shNode.CreateFolderItem(self.study2ItemID, self.folderName)

    # Check if the items have the right parents
    self.assertEqual( shNode.GetItemParent(self.patient2ItemID), shNode.GetSceneItemID() )
    self.assertEqual( shNode.GetItemParent(self.study2ItemID), self.patient2ItemID )
    self.assertEqual( shNode.GetItemParent(self.folderItemID), self.study2ItemID )

  # ------------------------------------------------------------------------------
  def section_AttributeFilter(self):
    self.delayDisplay("Attribute filter",self.delayMs)

    # Get subject hierarchy tree view and model
    dataWidget = slicer.modules.data.widgetRepresentation()
    self.assertIsNotNone( dataWidget )
    shTreeView = slicer.util.findChild(dataWidget, name='SubjectHierarchyTreeView')
    self.assertIsNotNone( shTreeView )
    shModel = shTreeView.model()
    self.assertIsNotNone( shModel )

    self.assertEqual(shTreeView.displayedItemCount(), 9)
    shTreeView.setAttributeFilter('DICOM.Modality')
    self.assertEqual(shTreeView.displayedItemCount(), 3)
    shTreeView.setAttributeFilter('DICOM.Modality','IncorrectValue')
    self.assertEqual(shTreeView.displayedItemCount(), 0)
    shTreeView.setAttributeFilter('DICOM.Modality','CT')
    self.assertEqual(shTreeView.displayedItemCount(), 3)
    shTreeView.removeAttributeFilter()
    self.assertEqual(shTreeView.displayedItemCount(), 9)

  # ------------------------------------------------------------------------------
  def section_ReparentNodeInSubjectHierarchy(self):
    self.delayDisplay("Reparent node in subject hierarchy",self.delayMs)

    shNode = slicer.mrmlScene.GetSubjectHierarchyNode()
    self.assertIsNotNone( shNode )

    # Get subject hierarchy scene model
    dataWidget = slicer.modules.data.widgetRepresentation()
    self.assertIsNotNone( dataWidget )
    shTreeView = slicer.util.findChild(dataWidget, name='SubjectHierarchyTreeView')
    self.assertIsNotNone( shTreeView )
    shModel = shTreeView.model()
    self.assertIsNotNone( shModel )

    # Reparent using the item model
    shModel.reparent(self.sampleLabelmapShItemID, self.studyItemID)
    self.assertEqual( shNode.GetItemParent(self.sampleLabelmapShItemID), self.studyItemID )
    self.assertEqual( shNode.GetItemOwnerPluginName(self.sampleLabelmapShItemID), 'LabelMaps' )

    # Reparent using the node's set parent function
    shNode.SetItemParent(self.ctVolumeShItemID, self.study2ItemID)
    self.assertEqual( shNode.GetItemParent(self.ctVolumeShItemID), self.study2ItemID )
    self.assertEqual( shNode.GetItemOwnerPluginName(self.ctVolumeShItemID), 'Volumes' )

    # Reparent using the node's create item function
    shNode.CreateItem(self.folderItemID, self.sampleModelNode)
    self.assertEqual( shNode.GetItemParent(self.sampleModelShItemID), self.folderItemID )
    self.assertEqual( shNode.GetItemOwnerPluginName(self.sampleModelShItemID), 'Models' )

  # ------------------------------------------------------------------------------
  def section_LoadScene(self):
    self.delayDisplay("Load scene",self.delayMs)

    shNode = slicer.mrmlScene.GetSubjectHierarchyNode()
    self.assertIsNotNone( shNode )

    # Rename existing items so that when the scene is loaded again they are different
    shNode.SetItemName(self.patientItemID, self.patientNewName)
    shNode.SetItemName(self.studyItemID, self.studyNewName)
    shNode.SetItemName(self.ctVolumeShItemID, self.ctVolumeNewName)

    # Load the saved scene
    slicer.util.loadScene(self.sceneFileName)

    # Check number of nodes in the scene
    self.assertEqual( slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLScalarVolumeNode'), 4 )
    self.assertEqual( slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLModelNode'), 4 ) # Including the three slice view models
    self.assertEqual( slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLSubjectHierarchyNode'), 1 )

    # Check if the items are in the right hierarchy with the right names
    self.assertEqual( shNode.GetItemChildWithName(shNode.GetSceneItemID(), self.patientNewName), self.patientItemID )
    self.assertEqual( shNode.GetItemChildWithName(self.patientItemID, self.studyNewName), self.studyItemID )
    self.assertEqual( shNode.GetItemChildWithName(self.studyItemID, self.sampleLabelmapName), self.sampleLabelmapShItemID )

    self.assertEqual( shNode.GetItemChildWithName(shNode.GetSceneItemID(), self.patient2Name), self.patient2ItemID )
    self.assertEqual( shNode.GetItemChildWithName(self.patient2ItemID, self.study2Name), self.study2ItemID )
    self.assertEqual( shNode.GetItemChildWithName(self.study2ItemID, self.folderName), self.folderItemID )
    self.assertEqual( shNode.GetItemChildWithName(self.folderItemID, self.sampleModelName), self.sampleModelShItemID )
    self.assertEqual( shNode.GetItemChildWithName(self.study2ItemID, self.ctVolumeNewName), self.ctVolumeShItemID )

    loadedPatientItemID = shNode.GetItemChildWithName(shNode.GetSceneItemID(), self.patientOriginalName)
    self.assertIsNotNone( loadedPatientItemID )
    loadedStudyItemID = shNode.GetItemChildWithName(loadedPatientItemID, self.studyOriginalName)
    self.assertIsNotNone( loadedStudyItemID )
    loadedCtVolumeShItemID = shNode.GetItemChildWithName(loadedStudyItemID, self.ctVolumeOriginalName)
    self.assertIsNotNone( loadedCtVolumeShItemID )

    # Print subject hierarchy after the test
    logging.info(shNode)

  # ------------------------------------------------------------------------------
  def section_TestCircularParenthood(self):
    # Test case for https://issues.slicer.org/view.php?id=4713
    self.delayDisplay("Test circular parenthood",self.delayMs)

    shNode = slicer.mrmlScene.GetSubjectHierarchyNode()
    self.assertIsNotNone( shNode )

    sceneItemID = shNode.GetSceneItemID()
    mainfolder_ID = shNode.CreateFolderItem(sceneItemID, "Main Folder")
    subfolder_ID = shNode.CreateFolderItem(sceneItemID, "Sub Folder")
    shNode.SetItemParent(subfolder_ID, mainfolder_ID) # Regular hiearchy setting
    shNode.SetItemParent(mainfolder_ID, subfolder_ID) # Makes slicer crash instead of returning an error

  # ------------------------------------------------------------------------------
  # Utility functions

  # ------------------------------------------------------------------------------
  # Create sample labelmap with same geometry as input volume
  def createSampleLabelmapVolumeNode(self, volumeNode, name, label, colorNode=None):
    self.assertIsNotNone( volumeNode )
    self.assertTrue( volumeNode.IsA('vtkMRMLScalarVolumeNode') )
    self.assertTrue( label > 0 )

    sampleLabelmapNode = slicer.vtkMRMLLabelMapVolumeNode()
    sampleLabelmapNode.SetName(name)
    sampleLabelmapNode = slicer.mrmlScene.AddNode(sampleLabelmapNode)
    sampleLabelmapNode.Copy(volumeNode)
    imageData = sampleLabelmapNode.GetImageData()
    extent = imageData.GetExtent()
    for x in range(extent[0], extent[1]+1):
      for y in range(extent[2], extent[3]+1):
        for z in range(extent[4], extent[5]+1):
          if ( (x >= (extent[1]/4) and x <= (extent[1]/4) * 3) and
               (y >= (extent[3]/4) and y <= (extent[3]/4) * 3) and
               (z >= (extent[5]/4) and z <= (extent[5]/4) * 3) ):
            imageData.SetScalarComponentFromDouble(x,y,z,0,label)
          else:
            imageData.SetScalarComponentFromDouble(x,y,z,0,0)

    # Display labelmap
    labelmapVolumeDisplayNode = slicer.vtkMRMLLabelMapVolumeDisplayNode()
    slicer.mrmlScene.AddNode(labelmapVolumeDisplayNode)
    if colorNode is None:
      colorNode = slicer.util.getNode('GenericAnatomyColors')
      self.assertIsNotNone( colorNode )
    labelmapVolumeDisplayNode.SetAndObserveColorNodeID(colorNode.GetID())
    labelmapVolumeDisplayNode.VisibilityOn()
    sampleLabelmapName = slicer.mrmlScene.GenerateUniqueName(name)
    sampleLabelmapNode.SetName(sampleLabelmapName)
    sampleLabelmapNode.SetAndObserveDisplayNodeID(labelmapVolumeDisplayNode.GetID())

    return sampleLabelmapNode

  #------------------------------------------------------------------------------
  # Create sphere model at the centre of an input volume
  def createSampleModelNode(self, name, color, volumeNode=None):
    if volumeNode:
      self.assertTrue( volumeNode.IsA('vtkMRMLScalarVolumeNode') )
      bounds = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
      volumeNode.GetRASBounds(bounds)
      x = (bounds[0] + bounds[1])/2
      y = (bounds[2] + bounds[3])/2
      z = (bounds[4] + bounds[5])/2
      radius = min(bounds[1]-bounds[0],bounds[3]-bounds[2],bounds[5]-bounds[4]) / 3.0
    else:
      radius = 50
      x = y = z = 0

    # Taken from: http://www.na-mic.org/Bug/view.php?id=1536
    sphere = vtk.vtkSphereSource()
    sphere.SetCenter(x, y, z)
    sphere.SetRadius(radius)

    modelNode = slicer.vtkMRMLModelNode()
    modelNode.SetName(name)
    modelNode = slicer.mrmlScene.AddNode(modelNode)
    modelNode.SetPolyDataConnection(sphere.GetOutputPort())
    modelNode.SetHideFromEditors(0)

    displayNode = slicer.vtkMRMLModelDisplayNode()
    slicer.mrmlScene.AddNode(displayNode)
    displayNode.Visibility2DOn()
    displayNode.VisibilityOn()
    displayNode.SetColor(color[0], color[1], color[2])
    modelNode.SetAndObserveDisplayNodeID(displayNode.GetID())

    return modelNode

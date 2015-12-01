import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
from DICOMLib import DICOMUtils

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
    parent.acknowledgementText = """This file was originally developed by Csaba Pinter, PerkLab, Queen's University and was supported through the Applied Cancer Research Unit program of Cancer Care Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care""" # replace with organization, grant and thanks.
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['SubjectHierarchyGenericSelfTest'] = self.runTest

  def runTest(self):
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

    self.section_SetupPathsAndNames()
    self.section_LoadDicomData()
    self.section_AddNodeToSubjectHierarchy()
    self.section_CreateSecondBranch()
    self.section_ReparentNodeInSubjectHierarchy()
    self.section_SaveScene()
    self.section_LoadScene()


  # ------------------------------------------------------------------------------
  def section_SetupPathsAndNames(self):
    # Make sure subject hierarchy auto-creation is on for this test
    subjectHierarchyWidget = slicer.modules.subjecthierarchy.widgetRepresentation()
    subjectHierarchyPluginLogic = subjectHierarchyWidget.pluginLogic()
    self.assertIsNotNone( subjectHierarchyWidget )
    self.assertIsNotNone( subjectHierarchyPluginLogic )
    subjectHierarchyPluginLogic.autoCreateSubjectHierarchy = True

    # Set constants
    subjectHierarchyGenericSelfTestDir = slicer.app.temporaryPath + '/SubjectHierarchyGenericSelfTest'
    print('Test directory: ' + subjectHierarchyGenericSelfTestDir)
    if not os.access(subjectHierarchyGenericSelfTestDir, os.F_OK):
      os.mkdir(subjectHierarchyGenericSelfTestDir)

    self.dicomDataDir = subjectHierarchyGenericSelfTestDir + '/DicomData'
    if not os.access(self.dicomDataDir, os.F_OK):
      os.mkdir(self.dicomDataDir)

    self.dicomDatabaseDir = subjectHierarchyGenericSelfTestDir + '/CtkDicomDatabase'
    self.dicomZipFilePath = subjectHierarchyGenericSelfTestDir + '/TestDicomCT.zip'
    self.expectedNumOfFilesInDicomDataDir = 10
    self.tempDir = subjectHierarchyGenericSelfTestDir + '/Temp'
    self.sceneFileName = self.tempDir + '/SubjectHierarchyGenericSelfTestScene.mrml'

    self.patientNodeName = '' # To be filled in after loading
    self.studyNodeName = '' # To be filled in after loading
    self.ctVolumeShNodeName = '' # To be filled in after loading
    self.sampleLabelmapName = 'SampleLabelmap'
    self.sampleModelNoAutoCreateName = 'SampleModelNoAutoCreate'
    self.sampleModelName = 'SampleModel'
    self.patient2Name = 'Patient2'
    self.study2Name = 'Study2'
    self.testSeriesName = 'TestSeries_Empty'
    self.testSubseriesName = 'TestSuberies_Empty'

  # ------------------------------------------------------------------------------
  def section_LoadDicomData(self):
    try:
      # Download and unzip test CT DICOM data
      import urllib
      downloads = (
          ('http://slicer.kitware.com/midas3/download/item/137843/TestDicomCT.zip', self.dicomZipFilePath),
          )

      downloaded = 0
      for url,filePath in downloads:
        if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
          if downloaded == 0:
            self.delayDisplay('Downloading input data to folder\n' + self.dicomZipFilePath + '.\n\n  It may take a few minutes...',self.delayMs)
          print('Requesting download from %s...' % (url))
          urllib.urlretrieve(url, filePath)
          downloaded += 1
        else:
          self.delayDisplay('Input data has been found in folder ' + self.dicomZipFilePath, self.delayMs)
      if downloaded > 0:
        self.delayDisplay('Downloading input data finished',self.delayMs)

      numOfFilesInDicomDataDir = len([name for name in os.listdir(self.dicomDataDir) if os.path.isfile(self.dicomDataDir + '/' + name)])
      if (numOfFilesInDicomDataDir != self.expectedNumOfFilesInDicomDataDir):
        slicer.app.applicationLogic().Unzip(self.dicomZipFilePath, self.dicomDataDir)
        self.delayDisplay("Unzipping done",self.delayMs)

      numOfFilesInDicomDataDirTest = len([name for name in os.listdir(self.dicomDataDir) if os.path.isfile(self.dicomDataDir + '/' + name)])
      self.assertEqual( numOfFilesInDicomDataDirTest, self.expectedNumOfFilesInDicomDataDir )

      # Open test database and empty it
      with DICOMUtils.TemporaryDICOMDatabase(self.dicomDatabaseDir, True) as db:
        self.assertTrue( db.isOpen )
        self.assertEqual( slicer.dicomDatabase, db)

        # Import test data in database
        indexer = ctk.ctkDICOMIndexer()
        self.assertIsNotNone( indexer )

        indexer.addDirectory( slicer.dicomDatabase, self.dicomDataDir )

        self.assertEqual( len(slicer.dicomDatabase.patients()), 1 )
        self.assertIsNotNone( slicer.dicomDatabase.patients()[0] )

        # Load test data
        numOfScalarVolumeNodesBeforeLoad = len( slicer.util.getNodes('vtkMRMLScalarVolumeNode*') )
        numOfSubjectHierarchyNodesBeforeLoad = len( slicer.util.getNodes('vtkMRMLSubjectHierarchyNode*') )

        patient = slicer.dicomDatabase.patients()[0]
        DICOMUtils.loadPatientByUID(patient)

        self.assertEqual( len( slicer.util.getNodes('vtkMRMLScalarVolumeNode*') ),  numOfScalarVolumeNodesBeforeLoad + 1 )
        self.assertEqual( len( slicer.util.getNodes('vtkMRMLSubjectHierarchyNode*') ), numOfSubjectHierarchyNodesBeforeLoad + 3 )
      
    except Exception, e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e),self.delayMs*2)

  # ------------------------------------------------------------------------------
  def section_AddNodeToSubjectHierarchy(self):
    self.delayDisplay("Add node to subject hierarchy",self.delayMs)

    import qSlicerSubjectHierarchyModuleWidgetsPythonQt

    # Get volume previously loaded from DICOM
    ctVolumeNode = slicer.util.getNode('vtkMRMLScalarVolumeNode*')
    self.assertIsNotNone( ctVolumeNode )

    # Create sample labelmap and model and add them in subject hierarchy
    sampleLabelmapNode = self.createSampleLabelmapVolumeNode(ctVolumeNode, self.sampleLabelmapName, 2)
    sampleModelColor = [0.0, 1.0, 0.0]
    sampleModelNode = self.createSampleModelVolume(self.sampleModelName, sampleModelColor, ctVolumeNode)

    # Get subject hierarchy scene model
    subjectHierarchyWidget = slicer.modules.subjecthierarchy.widgetRepresentation()
    self.assertIsNotNone( subjectHierarchyWidget )
    subjectHierarchySceneModel = subjectHierarchyWidget.subjectHierarchySceneModel()
    self.assertIsNotNone( subjectHierarchySceneModel )

    # Get subject hierarchy nodes and study node
    ctVolumeShNode = slicer.vtkMRMLSubjectHierarchyNode.GetAssociatedSubjectHierarchyNode(ctVolumeNode)
    self.assertIsNotNone( ctVolumeShNode )
    sampleLabelmapShNode = slicer.vtkMRMLSubjectHierarchyNode.GetAssociatedSubjectHierarchyNode(sampleLabelmapNode)
    self.assertIsNotNone( sampleLabelmapShNode )
    sampleModelShNode = slicer.vtkMRMLSubjectHierarchyNode.GetAssociatedSubjectHierarchyNode(sampleModelNode)
    self.assertIsNotNone( sampleModelShNode )
    studyNode = ctVolumeShNode.GetParentNode()
    self.assertIsNotNone( studyNode )

    # Save node names for scene load testing
    self.patientNodeName = studyNode.GetParentNode().GetName()
    self.studyNodeName = studyNode.GetName()
    self.ctVolumeShNodeName = ctVolumeShNode.GetName()

    # Verify DICOM levels
    self.assertEqual( studyNode.GetParentNode().GetLevel(), slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMLevelPatient() )
    self.assertEqual( studyNode.GetLevel(), slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMLevelStudy() )
    self.assertEqual( ctVolumeShNode.GetLevel(), slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMLevelSeries() )

    # Add model and labelmap to the created study
    subjectHierarchySceneModel.reparent(sampleLabelmapShNode, studyNode)
    subjectHierarchySceneModel.reparent(sampleModelShNode, studyNode)
    qt.QApplication.processEvents()

    self.assertEqual( sampleLabelmapShNode.GetParentNode(), studyNode )
    self.assertEqual( sampleLabelmapShNode.GetOwnerPluginName(), 'LabelMaps' )

    self.assertEqual( sampleModelShNode.GetParentNode(), studyNode )
    self.assertEqual( sampleModelShNode.GetOwnerPluginName(), 'Models' )

  # ------------------------------------------------------------------------------
  def section_CreateSecondBranch(self):
    self.delayDisplay("Create second branch in subject hierarchy",self.delayMs)

    patient2Node = slicer.vtkMRMLSubjectHierarchyNode.CreateSubjectHierarchyNode(slicer.mrmlScene, None, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMLevelPatient(), self.patient2Name)
    study2Node = slicer.vtkMRMLSubjectHierarchyNode.CreateSubjectHierarchyNode(slicer.mrmlScene, patient2Node, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMLevelStudy(), self.study2Name)
    seriesNode = slicer.vtkMRMLSubjectHierarchyNode.CreateSubjectHierarchyNode(slicer.mrmlScene, study2Node, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMLevelSeries(), self.testSeriesName)
    subseriesNode = slicer.vtkMRMLSubjectHierarchyNode.CreateSubjectHierarchyNode(slicer.mrmlScene, seriesNode, slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMLevelSubseries(), self.testSubseriesName)

    # Check if the created nodes have the right parents
    self.assertEqual( study2Node.GetParentNode(), patient2Node )
    self.assertEqual( seriesNode.GetParentNode(), study2Node )
    self.assertEqual( subseriesNode.GetParentNode(), seriesNode )

  # ------------------------------------------------------------------------------
  def section_ReparentNodeInSubjectHierarchy(self):
    self.delayDisplay("Reparent node in subject hierarchy",self.delayMs)

    # Get second study node and labelmap node to reparent
    study2Node = slicer.util.getNode(self.study2Name + slicer.vtkMRMLSubjectHierarchyConstants.GetSubjectHierarchyNodeNamePostfix())
    self.assertIsNotNone( study2Node )
    sampleLabelmapShNode = slicer.util.getNode(self.sampleLabelmapName + slicer.vtkMRMLSubjectHierarchyConstants.GetSubjectHierarchyNodeNamePostfix())
    self.assertIsNotNone( sampleLabelmapShNode )

    # Get subject hierarchy scene model
    import qSlicerSubjectHierarchyModuleWidgetsPythonQt
    subjectHierarchyWidget = slicer.modules.subjecthierarchy.widgetRepresentation()
    self.assertIsNotNone( subjectHierarchyWidget )
    subjectHierarchySceneModel = subjectHierarchyWidget.subjectHierarchySceneModel()
    self.assertIsNotNone( subjectHierarchySceneModel )

    # Reparent
    subjectHierarchySceneModel.reparent(sampleLabelmapShNode, study2Node)
    self.assertEqual( sampleLabelmapShNode.GetParentNode(), study2Node )
    self.assertEqual( sampleLabelmapShNode.GetOwnerPluginName(), 'LabelMaps' )

  # ------------------------------------------------------------------------------
  def section_SaveScene(self):
    self.delayDisplay("Save scene",self.delayMs)

    if not os.access(self.tempDir, os.F_OK):
      os.mkdir(self.tempDir)

    if os.access(self.sceneFileName, os.F_OK):
      os.remove(self.sceneFileName)

    # Save MRML scene into file
    slicer.mrmlScene.Commit(self.sceneFileName)

    readable = os.access(self.sceneFileName, os.R_OK)
    self.assertTrue( readable )

  # ------------------------------------------------------------------------------
  def section_LoadScene(self):
    self.delayDisplay("Load scene",self.delayMs)

    slicer.mrmlScene.Clear(0)

    slicer.util.loadScene(self.sceneFileName)

    # Check number of nodes in the scene
    self.assertEqual( slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLScalarVolumeNode'), 2 )
    self.assertEqual( slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLModelNode'), 4 ) # 1 + slice models
    self.assertEqual( slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLSubjectHierarchyNode'), 9 )
    
    # Check if the nodes have the right parents
    patientNode = slicer.util.getNode(self.patientNodeName)
    self.assertIsNotNone( patientNode )
    self.assertIsNone( patientNode.GetParentNode() )
    
    studyNode = slicer.util.getNode(self.studyNodeName)
    self.assertIsNotNone( studyNode )
    self.assertEqual( studyNode.GetParentNode(), patientNode )
    
    ctVolumeShNode = slicer.util.getNode(self.ctVolumeShNodeName)
    self.assertIsNotNone( ctVolumeShNode )
    self.assertEqual( ctVolumeShNode.GetParentNode(), studyNode )
    
    sampleModelShNode = slicer.util.getNode(self.sampleModelName + slicer.vtkMRMLSubjectHierarchyConstants.GetSubjectHierarchyNodeNamePostfix())
    self.assertIsNotNone( sampleModelShNode )
    self.assertEqual( sampleModelShNode.GetParentNode(), studyNode )
    
    patient2Node = slicer.util.getNode(self.patient2Name + slicer.vtkMRMLSubjectHierarchyConstants.GetSubjectHierarchyNodeNamePostfix())
    self.assertIsNotNone( patient2Node )
    self.assertIsNone( patient2Node.GetParentNode() )

    study2Node = slicer.util.getNode(self.study2Name + slicer.vtkMRMLSubjectHierarchyConstants.GetSubjectHierarchyNodeNamePostfix())
    self.assertIsNotNone( study2Node )
    self.assertEqual( study2Node.GetParentNode(), patient2Node )

    sampleLabelmapShNode = slicer.util.getNode(self.sampleLabelmapName + slicer.vtkMRMLSubjectHierarchyConstants.GetSubjectHierarchyNodeNamePostfix())
    self.assertIsNotNone( sampleLabelmapShNode )
    self.assertEqual( sampleLabelmapShNode.GetParentNode(), study2Node )

    testSeriesNode = slicer.util.getNode(self.testSeriesName + slicer.vtkMRMLSubjectHierarchyConstants.GetSubjectHierarchyNodeNamePostfix())
    self.assertIsNotNone( testSeriesNode )
    self.assertEqual( testSeriesNode.GetParentNode(), study2Node )

    testSubseriesNode = slicer.util.getNode(self.testSubseriesName + slicer.vtkMRMLSubjectHierarchyConstants.GetSubjectHierarchyNodeNamePostfix())
    self.assertIsNotNone( testSubseriesNode )
    self.assertEqual( testSubseriesNode.GetParentNode(), testSeriesNode )

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
    imageData = vtk.vtkImageData()
    imageData.DeepCopy(volumeNode.GetImageData())
    sampleLabelmapNode.SetAndObserveImageData(imageData)

    extent = imageData.GetExtent()
    for x in xrange(extent[0], extent[1]+1):
      for y in xrange(extent[2], extent[3]+1):
        for z in xrange(extent[4], extent[5]+1):
          if (x >= (extent[1]/4) and x <= (extent[1]/4) * 3) and (y >= (extent[3]/4) and y <= (extent[3]/4) * 3) and (z >= (extent[5]/4) and z <= (extent[5]/4) * 3):
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
    sampleLabelmapNodeName = slicer.mrmlScene.GenerateUniqueName(name)
    sampleLabelmapNode.SetName(sampleLabelmapNodeName)
    sampleLabelmapNode.SetAndObserveDisplayNodeID(labelmapVolumeDisplayNode.GetID())

    return sampleLabelmapNode

  #------------------------------------------------------------------------------
  # Create sphere model at the centre of an input volume
  def createSampleModelVolume(self, name, color, volumeNode=None):
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
    displayNode.SliceIntersectionVisibilityOn()
    displayNode.VisibilityOn()
    displayNode.SetColor(color[0], color[1], color[2])
    modelNode.SetAndObserveDisplayNodeID(displayNode.GetID())

    return modelNode

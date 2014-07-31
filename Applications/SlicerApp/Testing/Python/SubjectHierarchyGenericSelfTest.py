import os
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# SubjectHierarchyGenericSelfTest
#

class SubjectHierarchyGenericSelfTest:
  def __init__(self, parent):
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

class SubjectHierarchyGenericSelfTestWidget:
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
    self.reloadButton.name = "SubjectHierarchyGenericSelfTest Reload"
    self.layout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    self.layout.addWidget(self.reloadAndTestButton)
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    # Add vertical spacer
    self.layout.addStretch(1)

  def onReload(self,moduleName="SubjectHierarchyGenericSelfTest"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="SubjectHierarchyGenericSelfTest"):
    self.onReload()
    evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
    tester = eval(evalString)
    tester.runTest()

#
# SubjectHierarchyGenericSelfTestLogic
#

class SubjectHierarchyGenericSelfTestLogic:
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass


class SubjectHierarchyGenericSelfTestTest(unittest.TestCase):
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
    self.assertTrue( slicer.modules.subjecthierarchy )

    # TODO: Uncomment when #598 is fixed
    slicer.util.selectModule('SubjectHierarchy')

    self.section_SetupPathsAndNames()
    self.section_LoadInputData()
    self.section_AddNodeToSubjectHierarchy()
    self.section_CreateSecondBranch()
    self.section_ReparentNodeInSubjectHierarchy()
    self.section_SaveScene()
    self.section_LoadScene()


  # ------------------------------------------------------------------------------
  def section_SetupPathsAndNames(self):
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
    self.sampleModelName = 'SampleModel'
    self.patient2Name = 'Patient2'
    self.study2Name = 'Study2'
    self.testSeriesName = 'TestSeries_Empty'
    self.testSubseriesName = 'TestSuberies_Empty'

  # ------------------------------------------------------------------------------
  def section_LoadInputData(self):
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
      self.assertTrue( numOfFilesInDicomDataDirTest == self.expectedNumOfFilesInDicomDataDir )

      # Open test database and empty it
      qt.QDir().mkpath(self.dicomDatabaseDir)

      if slicer.dicomDatabase:
        self.originalDatabaseDirectory = os.path.split(slicer.dicomDatabase.databaseFilename)[0]
      else:
        self.originalDatabaseDirectory = None
        settings = qt.QSettings()
        settings.setValue('DatabaseDirectory', self.dicomDatabaseDir)

      dicomWidget = slicer.modules.dicom.widgetRepresentation().self()
      dicomWidget.onDatabaseDirectoryChanged(self.dicomDatabaseDir)
      self.assertTrue( slicer.dicomDatabase.isOpen )

      # Import test data in database
      indexer = ctk.ctkDICOMIndexer()
      self.assertTrue( indexer )

      indexer.addDirectory( slicer.dicomDatabase, self.dicomDataDir )

      self.assertTrue( len(slicer.dicomDatabase.patients()) == 1 )
      self.assertTrue( slicer.dicomDatabase.patients()[0] )

      # Load test data
      numOfScalarVolumeNodesBeforeLoad = len( slicer.util.getNodes('vtkMRMLScalarVolumeNode*') )
      numOfSubjectHierarchyNodesBeforeLoad = len( slicer.util.getNodes('vtkMRMLSubjectHierarchyNode*') )

      patient = slicer.dicomDatabase.patients()[0]
      studies = slicer.dicomDatabase.studiesForPatient(patient)
      series = [slicer.dicomDatabase.seriesForStudy(study) for study in studies]
      seriesUIDs = [uid for uidList in series for uid in uidList]
      dicomWidget.detailsPopup.offerLoadables(seriesUIDs, 'SeriesUIDList')
      dicomWidget.detailsPopup.loadCheckedLoadables()

      self.assertTrue( len( slicer.util.getNodes('vtkMRMLScalarVolumeNode*') ) == numOfScalarVolumeNodesBeforeLoad + 1 )
      self.assertTrue( len( slicer.util.getNodes('vtkMRMLSubjectHierarchyNode*') ) == numOfSubjectHierarchyNodesBeforeLoad + 3 )

    except Exception, e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e),self.delayMs*2)

  # ------------------------------------------------------------------------------
  def section_AddNodeToSubjectHierarchy(self):
    self.delayDisplay("Add node to subject hierarchy",self.delayMs)

    from vtkSlicerSubjectHierarchyModuleMRML import vtkMRMLSubjectHierarchyNode
    import qSlicerSubjectHierarchyModuleWidgetsPythonQt

    # Get volume previously loaded from DICOM
    ctVolumeNode = slicer.util.getNode('vtkMRMLScalarVolumeNode*')
    self.assertTrue( ctVolumeNode != None )

    # Create sample labelmap and model and add them in subject hierarchy
    sampleLabelmapNode = self.createSampleLabelmapVolumeNode(ctVolumeNode, self.sampleLabelmapName, 2)
    sampleModelColor = [0.0, 1.0, 0.0]
    sampleModelNode = self.createSampleModelVolume(ctVolumeNode, self.sampleModelName, sampleModelColor)

    # Get subject hierarchy scene model
    subjectHierarchyWidget = slicer.modules.subjecthierarchy.widgetRepresentation()
    self.assertTrue( subjectHierarchyWidget != None )
    subjectHierarchySceneModel = subjectHierarchyWidget.subjectHierarchySceneModel()
    self.assertTrue( subjectHierarchySceneModel != None )

    # Get study node
    ctVolumeShNode = vtkMRMLSubjectHierarchyNode.GetAssociatedSubjectHierarchyNode(ctVolumeNode)
    self.assertTrue( ctVolumeShNode != None )
    studyNode = ctVolumeShNode.GetParentNode()
    self.assertTrue( studyNode != None )

    # Save node names for scene load testing
    self.patientNodeName = studyNode.GetParentNode().GetName()
    self.studyNodeName = studyNode.GetName()
    self.ctVolumeShNodeName = ctVolumeShNode.GetName()
    
    # Verify DICOM levels
    self.assertTrue( studyNode.GetParentNode().GetLevel() == 'Subject' )
    self.assertTrue( studyNode.GetLevel() == 'Study' )
    self.assertTrue( ctVolumeShNode.GetLevel() == 'Series' )

    # Add model and labelmap to the created study
    subjectHierarchySceneModel.reparent(sampleLabelmapNode, studyNode)
    subjectHierarchySceneModel.reparent(sampleModelNode, studyNode)

    sampleLabelmapShNode = vtkMRMLSubjectHierarchyNode.GetAssociatedSubjectHierarchyNode(sampleLabelmapNode)
    self.assertTrue( sampleLabelmapShNode != None )
    self.assertTrue( sampleLabelmapShNode.GetParentNode() == studyNode )
    self.assertTrue( sampleLabelmapShNode.GetOwnerPluginName() == 'Volumes' )

    sampleModelShNode = vtkMRMLSubjectHierarchyNode.GetAssociatedSubjectHierarchyNode(sampleModelNode)
    self.assertTrue( sampleModelShNode != None )
    self.assertTrue( sampleModelShNode.GetParentNode() == studyNode )
    self.assertTrue( sampleModelShNode.GetOwnerPluginName() == 'Models' )

  # ------------------------------------------------------------------------------
  def section_CreateSecondBranch(self):
    self.delayDisplay("Create second branch in subject hierarchy",self.delayMs)

    from vtkSlicerSubjectHierarchyModuleMRML import vtkMRMLSubjectHierarchyNode

    patient2Node = vtkMRMLSubjectHierarchyNode.CreateSubjectHierarchyNode(slicer.mrmlScene, None, 'Subject', self.patient2Name)
    study2Node = vtkMRMLSubjectHierarchyNode.CreateSubjectHierarchyNode(slicer.mrmlScene, patient2Node, 'Study', self.study2Name)
    seriesNode = vtkMRMLSubjectHierarchyNode.CreateSubjectHierarchyNode(slicer.mrmlScene, study2Node, 'Series', self.testSeriesName)
    subseriesNode = vtkMRMLSubjectHierarchyNode.CreateSubjectHierarchyNode(slicer.mrmlScene, seriesNode, 'Subseries', self.testSubseriesName)

    # Check if the created nodes have the right parents
    self.assertTrue( study2Node.GetParentNode() == patient2Node )
    self.assertTrue( seriesNode.GetParentNode() == study2Node )
    self.assertTrue( subseriesNode.GetParentNode() == seriesNode )

  # ------------------------------------------------------------------------------
  def section_ReparentNodeInSubjectHierarchy(self):
    self.delayDisplay("Reparent node in subject hierarchy",self.delayMs)

    # Get second study node and labelmap node to reparent
    study2Node = slicer.util.getNode(self.study2Name + '_SubjectHierarchy')
    self.assertTrue( study2Node != None )
    sampleLabelmapShNode = slicer.util.getNode(self.sampleLabelmapName + '_SubjectHierarchy')
    self.assertTrue( sampleLabelmapShNode != None )

    # Get subject hierarchy scene model
    import qSlicerSubjectHierarchyModuleWidgetsPythonQt
    subjectHierarchyWidget = slicer.modules.subjecthierarchy.widgetRepresentation()
    self.assertTrue( subjectHierarchyWidget != None )
    subjectHierarchySceneModel = subjectHierarchyWidget.subjectHierarchySceneModel()
    self.assertTrue( subjectHierarchySceneModel != None )

    # Reparent
    subjectHierarchySceneModel.reparent(sampleLabelmapShNode, study2Node)
    self.assertTrue( sampleLabelmapShNode.GetParentNode() == study2Node )
    self.assertTrue( sampleLabelmapShNode.GetOwnerPluginName() == 'Volumes' )

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
    self.assertTrue( slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLScalarVolumeNode') == 2 )
    self.assertTrue( slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLModelNode') == 4 ) # 1 + slice models
    self.assertTrue( slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLSubjectHierarchyNode') == 9 )
    
    # Check if the nodes have the right parents
    patientNode = slicer.util.getNode(self.patientNodeName)
    self.assertTrue( patientNode != None )
    self.assertTrue( patientNode.GetParentNode() == None )
    
    studyNode = slicer.util.getNode(self.studyNodeName)
    self.assertTrue( studyNode != None )
    self.assertTrue( studyNode.GetParentNode() == patientNode )
    
    ctVolumeShNode = slicer.util.getNode(self.ctVolumeShNodeName)
    self.assertTrue( ctVolumeShNode != None )
    self.assertTrue( ctVolumeShNode.GetParentNode() == studyNode )
    
    sampleModelShNode = slicer.util.getNode(self.sampleModelName + '_SubjectHierarchy')
    self.assertTrue( sampleModelShNode != None )
    self.assertTrue( sampleModelShNode.GetParentNode() == studyNode )
    
    patient2Node = slicer.util.getNode(self.patient2Name + '_SubjectHierarchy')
    self.assertTrue( patient2Node != None )
    self.assertTrue( patient2Node.GetParentNode() == None )

    study2Node = slicer.util.getNode(self.study2Name + '_SubjectHierarchy')
    self.assertTrue( study2Node != None )
    self.assertTrue( study2Node.GetParentNode() == patient2Node )

    sampleLabelmapShNode = slicer.util.getNode(self.sampleLabelmapName + '_SubjectHierarchy')
    self.assertTrue( sampleLabelmapShNode != None )
    self.assertTrue( sampleLabelmapShNode.GetParentNode() == study2Node )

    testSeriesNode = slicer.util.getNode(self.testSeriesName + '_SubjectHierarchy')
    self.assertTrue( testSeriesNode != None )
    self.assertTrue( testSeriesNode.GetParentNode() == study2Node )

    testSubseriesNode = slicer.util.getNode(self.testSubseriesName + '_SubjectHierarchy')
    self.assertTrue( testSubseriesNode != None )
    self.assertTrue( testSubseriesNode.GetParentNode() == testSeriesNode )

  # ------------------------------------------------------------------------------
  # Create sample labelmap with same geometry as input volume
  def createSampleLabelmapVolumeNode(self, volumeNode, name, label, colorNode=None):
    self.assertTrue( volumeNode != None )
    self.assertTrue( volumeNode.IsA('vtkMRMLScalarVolumeNode') )
    self.assertTrue( label > 0 )

    sampleLabelmapNode = slicer.vtkMRMLScalarVolumeNode()
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
    if colorNode == None:
      colorNode = slicer.util.getNode('GenericAnatomyColors')
      self.assertTrue( colorNode != None )
    labelmapVolumeDisplayNode.SetAndObserveColorNodeID(colorNode.GetID())
    labelmapVolumeDisplayNode.VisibilityOn()
    sampleLabelmapNodeName = slicer.mrmlScene.GenerateUniqueName(name)
    sampleLabelmapNode.SetName(sampleLabelmapNodeName)
    sampleLabelmapNode.SetLabelMap(1)
    sampleLabelmapNode.SetAndObserveDisplayNodeID(labelmapVolumeDisplayNode.GetID())
    sampleLabelmapNode.SetHideFromEditors(0)

    return sampleLabelmapNode

  #------------------------------------------------------------------------------
  # Create sphere model at the centre of an input volume
  def createSampleModelVolume(self, volumeNode, name, color):
    self.assertTrue( volumeNode != None )
    self.assertTrue( volumeNode.IsA('vtkMRMLScalarVolumeNode') )

    bounds = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    volumeNode.GetRASBounds(bounds)
    x = (bounds[0] + bounds[1])/2
    y = (bounds[2] + bounds[3])/2
    z = (bounds[4] + bounds[5])/2
    radius = min(bounds[1]-bounds[0],bounds[3]-bounds[2],bounds[5]-bounds[4]) / 3.0

    # Taken from: http://www.na-mic.org/Bug/view.php?id=1536
    sphere = vtk.vtkSphereSource()
    sphere.SetCenter(x, y, z)
    sphere.SetRadius(radius)

    modelNode = slicer.vtkMRMLModelNode()
    modelNode = slicer.mrmlScene.AddNode(modelNode)
    modelNodeName = slicer.mrmlScene.GenerateUniqueName(name)
    modelNode.SetName(modelNodeName)
    if vtk.VTK_MAJOR_VERSION <= 5:
      modelNode.SetAndObservePolyData(sphere.GetOutput())
    else:
      modelNode.SetPolyDataConnection(sphere.GetOutputPort())
    modelNode.SetHideFromEditors(0)

    displayNode = slicer.vtkMRMLModelDisplayNode()
    slicer.mrmlScene.AddNode(displayNode)
    displayNode.SliceIntersectionVisibilityOn()
    displayNode.VisibilityOn()
    displayNode.SetColor(color[0], color[1], color[2])
    modelNode.SetAndObserveDisplayNodeID(displayNode.GetID())

    return modelNode

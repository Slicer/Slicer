from __future__ import print_function
import os
import unittest
import vtk, qt, ctk, slicer
from DICOMLib import DICOMUtils
from slicer.ScriptedLoadableModule import *
from slicer.util import TESTING_DATA_URL

#
# JRC2013Vis
#

class JRC2013Vis(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "JRC2013Vis" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Nicholas Herlambang (AZE R&D)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    This module was developed as a self test to perform the operations needed for the JRC 2013 Visualization Tutorial
    """
    parent.acknowledgementText = """
    This file was originally developed by Steve Pieper, Isomics, Inc.  and was partially funded by NIH grant 3P41RR013218-12S1.
""" # replace with organization, grant and thanks.

#
# qJRC2013VisWidget
#

class JRC2013VisWidget(ScriptedLoadableModuleWidget):

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    # Instantiate and connect widgets ...

    # start/stop DICOM peer
    self.startStopDicomPeerButton = qt.QPushButton("Start/Stop DICOM peer")
    self.startStopDicomPeerButton.setCheckable(True)
    self.layout.addWidget(self.startStopDicomPeerButton)
    self.startStopDicomPeerButton.connect('toggled(bool)', self.onStartStopDicomPeer)

    # Collapsible button
    testsCollapsibleButton = ctk.ctkCollapsibleButton()
    testsCollapsibleButton.text = "A collapsible button"
    self.layout.addWidget(testsCollapsibleButton)

    # Layout within the collapsible button
    formLayout = qt.QFormLayout(testsCollapsibleButton)

    # test buttons
    tests = ( ("Part 1: DICOM",self.onPart1DICOM),("Part 2: Head", self.onPart2Head),("Part 3: Liver", self.onPart3Liver),("Part 4: Lung", self.onPart4Lung),)
    for text,slot in tests:
      testButton = qt.QPushButton(text)
      testButton.toolTip = "Run the test."
      formLayout.addWidget(testButton)
      testButton.connect('clicked(bool)', slot)

    # Add vertical spacer
    self.layout.addStretch(1)

  def onPart1DICOM(self):
    tester = JRC2013VisTest()
    tester.setUp()
    tester.test_Part1DICOM()

  def onPart2Head(self):
    tester = JRC2013VisTest()
    tester.setUp()
    tester.test_Part2Head()

  def onPart3Liver(self):
    tester = JRC2013VisTest()
    tester.setUp()
    tester.test_Part3Liver()

  def onPart4Lung(self):
    tester = JRC2013VisTest()
    tester.setUp()
    tester.test_Part4Lung()

  def onStartStopDicomPeer(self,flag):
    if flag:
      import os
      self.startStopDicomPeerButton.setEnabled(False)
      dicomFilesDirectory = slicer.app.temporaryPath
      configFilePath = dicomFilesDirectory + '/Dcmtk-db/dcmqrscp.cfg'
      processCurrentPath = dicomFilesDirectory + '/Dcmtk-db/'

      if slicer.util.confirmYesNoDisplay('Do you want to choose local DCMTK database folder?'):
        print('Yes')
        dicomFilesDirectory = qt.QFileDialog.getExistingDirectory(None, 'Select DCMTK database folder')
        configFilePath = dicomFilesDirectory + '/dcmqrscp.cfg'
        processCurrentPath = dicomFilesDirectory
      else:
        import SampleData
        SampleData.downloadFromURL(
          fileNames='Dcmtk-db.zip',
          uris=TESTING_DATA_URL + 'MD5/6bfb01cf5ffb8e3af9b1c0c9556f0c6b45f0ec40305a9539ed7a9f0dcfe378e3',
          checksums='SHA256:6bfb01cf5ffb8e3af9b1c0c9556f0c6b45f0ec40305a9539ed7a9f0dcfe378e3')[0]

      import subprocess
      dcmqrscpExeOptions = (
        '/bin',
        '/../CTK-build/CMakeExternals/Install/bin',
        '/../DCMTK-install/bin',
        '/../DCMTK-build/bin',
        '/../DCMTK-build/bin/Release'
        '/../DCMTK-build/bin/Debug'
        '/../DCMTK-build/bin/RelWithDebInfo'
        '/../DCMTK-build/bin/MinSizeRel'
        )

      dcmqrscpExePath = None
      dcmqrscpExeName = '/dcmqrscp'
      if slicer.app.os == 'win':
        dcmqrscpExeName = dcmqrscpExeName + '.exe'
      for path in dcmqrscpExeOptions:
        testPath = slicer.app.slicerHome + path + dcmqrscpExeName
        if os.path.exists(testPath):
          dcmqrscpExePath = testPath
          break
      if not dcmqrscpExePath:
        raise UserWarning("Could not find dcmqrscp executable")

      args = (dcmqrscpExePath, '-c', configFilePath)
      print('Start DICOM peer')
      self.popen = subprocess.Popen(args, stdout=subprocess.PIPE, cwd=processCurrentPath)
      self.startStopDicomPeerButton.setEnabled(True)
    else:
      print('Stop DICOM peer')
      self.popen.kill()

#
# JRC2013VisLogic
#

class JRC2013VisLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  pass


class JRC2013VisTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    self.delayDisplay("Closing the scene")
    layoutManager = slicer.app.layoutManager()
    layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_Part1DICOM()
    self.setUp()
    self.test_Part2Head()
    self.setUp()
    self.test_Part3Liver()
    self.setUp()
    self.test_Part4Lung()

  def test_Part1DICOM(self):
    """ Test the DICOM part of the test using the head atlas
    """
    import os
    self.delayDisplay("Starting the DICOM test")
    #
    # first, get the data - a zip file of dicom data
    #
    import SampleData
    dicomFilesDirectory = SampleData.downloadFromURL(
      fileNames='Dcmtk-db.zip',
      uris=TESTING_DATA_URL + 'MD5/7a43d121a51a631ab0df02071e5ba6ed',
      checksums='MD5:7a43d121a51a631ab0df02071e5ba6ed')[0]

    try:
      self.delayDisplay("Switching to temp database directory")
      originalDatabaseDirectory = DICOMUtils.openTemporaryDatabase('tempDICOMDatbase')

      self.delayDisplay('Start Local DICOM Q/R SCP')
      import subprocess
      import os
      configFilePath = dicomFilesDirectory + '/Dcmtk-db/dcmqrscp.cfg'
      processCurrentPath = dicomFilesDirectory + '/Dcmtk-db/'
      print("configFilePath: "+os.path.abspath(configFilePath))
      print("processCurrentPath: "+os.path.abspath(processCurrentPath))

      dcmqrscpExeOptions = (
        '/bin',
        '/../CTK-build/CMakeExternals/Install/bin',
        '/../DCMTK-install/bin',
        '/../DCMTK-build/bin',
        '/../DCMTK-build/bin/Release',
        '/../DCMTK-build/bin/Debug',
        '/../DCMTK-build/bin/RelWithDebInfo'
        '/../DCMTK-build/bin/MinSizeRel'
        )

      dcmqrscpExePath = None
      dcmqrscpExeName = '/dcmqrscp'
      if slicer.app.os == 'win':
        dcmqrscpExeName = dcmqrscpExeName + '.exe'
      for path in dcmqrscpExeOptions:
        testPath = slicer.app.slicerHome + path + dcmqrscpExeName
        if os.path.exists(testPath):
          dcmqrscpExePath = testPath
          break
      if not dcmqrscpExePath:
        raise UserWarning("Could not find dcmqrscp executable")

      args = (dcmqrscpExePath, '-c', configFilePath)
      popen = subprocess.Popen(args, stdout=subprocess.PIPE, cwd=processCurrentPath)

      self.delayDisplay('Retrieve DICOM')
      slicer.util.selectModule('DICOM')
      dicomRetrieve = ctk.ctkDICOMRetrieve()
      dicomRetrieve.setKeepAssociationOpen(True)
      dicomRetrieve.setDatabase(slicer.dicomDatabase)
      dicomRetrieve.setCallingAETitle('SlicerAE')
      dicomRetrieve.setCalledAETitle('DCMTK')
      dicomRetrieve.setPort(12345)
      dicomRetrieve.setHost('localhost')
      dicomRetrieve.getStudy('1.2.124.113932.1.170.223.162.178.20050502.160340.12640015');
      popen.kill()

      # Select first patient
      browserWidget = slicer.modules.DICOMWidget.browserWidget
      browserWidget.dicomBrowser.dicomTableManager().patientsTable().selectFirst()
      browserWidget.examineForLoading()

      self.delayDisplay('Loading Selection')
      browserWidget.loadCheckedLoadables()

      self.delayDisplay('Change Level')
      layoutManager = slicer.app.layoutManager()
      redWidget = layoutManager.sliceWidget('Red')
      slicer.util.clickAndDrag(redWidget,start=(10,10),end=(10,40))

      self.delayDisplay('Change Window')
      slicer.util.clickAndDrag(redWidget,start=(10,10),end=(40,10))

      self.delayDisplay('Change Layout')
      layoutManager = slicer.app.layoutManager()
      layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpRedSliceView)

      self.delayDisplay('Zoom')
      slicer.util.clickAndDrag(redWidget,button='Right',start=(10,10),end=(10,40))

      self.delayDisplay('Pan')
      slicer.util.clickAndDrag(redWidget,button='Middle',start=(10,10),end=(40,40))

      self.delayDisplay('Center')
      redWidget.sliceController().fitSliceToBackground()

      self.delayDisplay('Lightbox')
      redWidget.sliceController().setLightboxTo6x6()

      self.delayDisplay('Conventional Layout')
      layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)

      self.delayDisplay('No Lightbox')
      redWidget.sliceController().setLightboxTo1x1()

      self.delayDisplay('Four Up Layout')
      layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutFourUpView)

      self.delayDisplay('Shift Mouse')
      slicer.util.clickAndDrag(redWidget,button='None',start=(100,100),end=(140,140),modifiers=['Shift'])

      self.delayDisplay('Conventional, Link, Slice Model')
      layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
      redWidget.sliceController().setSliceLink(True)
      redWidget.sliceController().setSliceVisible(True);

      self.delayDisplay('Rotate')
      threeDView = layoutManager.threeDWidget(0).threeDView()
      slicer.util.clickAndDrag(threeDView)

      self.delayDisplay('Zoom')
      threeDView = layoutManager.threeDWidget(0).threeDView()
      slicer.util.clickAndDrag(threeDView,button='Right')

      self.delayDisplay('Test passed!')
    except Exception as e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e))

    self.delayDisplay("Restoring original database directory")
    DICOMUtils.closeTemporaryDatabase(originalDatabaseDirectory)

  def test_Part2Head(self):
    """ Test using the head atlas - may not be needed - Slicer4Minute is already tested
    """
    self.delayDisplay("Starting the test")
    #
    # first, get some data
    # TODO: This is a very old scene with missing scene view screenshots
    #   that is why error is reported while attempting to load it.
    #   It would be better to replace with a new scene.
    #
    import SampleData
    SampleData.downloadFromURL(
      fileNames='3DHeadData.mrb',
      loadFiles=True,
      uris=TESTING_DATA_URL + 'SHA256/e2c7944095dd92be7961bed37f3c8f49e6f40c7f31d4fe865753b6efddae7993',
      checksums='SHA256:e2c7944095dd92be7961bed37f3c8f49e6f40c7f31d4fe865753b6efddae7993')
    self.delayDisplay('Finished with download and loading\n')

    try:
      logic = JRC2013VisLogic()
      mainWindow = slicer.util.mainWindow()
      layoutManager = slicer.app.layoutManager()
      threeDView = layoutManager.threeDWidget(0).threeDView()
      redWidget = layoutManager.sliceWidget('vtkMRMLSliceNode1') # it would be 'Red' in a recent scene
      redController = redWidget.sliceController()
      greenWidget = layoutManager.sliceWidget('vtkMRMLSliceNode2') # it would be 'Green' in a recent scene
      greenController = greenWidget.sliceController()

      self.delayDisplay('Models and Slice Model')
      mainWindow.moduleSelector().selectModule('Models')
      redWidget.sliceController().setSliceVisible(True);

      self.delayDisplay('Scroll Slices')
      for offset in range(-20,20,2):
        redController.setSliceOffsetValue(offset)

      self.delayDisplay('Skin Opacity')
      # turn off skin and skull
      skin = slicer.util.getNode(pattern='Skin.vtk')
      skin.GetDisplayNode().SetOpacity(0.5)

      self.delayDisplay('Skin and Skull Visibility')
      skin.GetDisplayNode().SetVisibility(0)
      skull = slicer.util.getNode(pattern='skull_bone.vtk')
      skull.GetDisplayNode().SetVisibility(0)

      self.delayDisplay('Green slice and Clipping')
      greenWidget.sliceController().setSliceVisible(True);
      hemispheric_white_matter = slicer.util.getNode(pattern='hemispheric_white_matter.vtk')
      hemispheric_white_matter.GetDisplayNode().SetClipping(1)
      clip = slicer.mrmlScene.GetFirstNodeByClass('vtkMRMLClipModelsNode')
      clip.SetRedSliceClipState(0)
      clip.SetYellowSliceClipState(0)
      clip.SetGreenSliceClipState(2)

      viewNode = threeDView.mrmlViewNode()
      cameras = slicer.util.getNodes('vtkMRMLCameraNode*')
      for cameraNode in cameras.values():
        if cameraNode.GetActiveTag() == viewNode.GetID():
          break
      cameraNode.GetCamera().Azimuth(90)
      cameraNode.GetCamera().Elevation(20)

      self.delayDisplay('Rotate')
      slicer.util.clickAndDrag(threeDView)

      self.delayDisplay('Zoom')
      threeDView = layoutManager.threeDWidget(0).threeDView()
      slicer.util.clickAndDrag(threeDView,button='Right')

      self.delayDisplay('Test passed!')
    except Exception as e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e))


  def test_Part3Liver(self):
    """ Test using the liver example data
    """
    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import SampleData
    SampleData.downloadFromURL(
      fileNames='LiverData.mrb',
      loadFiles=True,
      uris=TESTING_DATA_URL + 'SHA256/a39075d3e87f80bbf8eba1e0222ee68c60036e57c3db830db08f3022f424e221',
      checksums='SHA256:a39075d3e87f80bbf8eba1e0222ee68c60036e57c3db830db08f3022f424e221')
    self.delayDisplay('Finished with download and loading\n')

    try:
      logic = JRC2013VisLogic()
      mainWindow = slicer.util.mainWindow()
      layoutManager = slicer.app.layoutManager()
      threeDView = layoutManager.threeDWidget(0).threeDView()
      redWidget = layoutManager.sliceWidget('Red')
      redController = redWidget.sliceController()
      viewNode = threeDView.mrmlViewNode()
      cameras = slicer.util.getNodes('vtkMRMLCameraNode*')
      for cameraNode in cameras.values():
        if cameraNode.GetActiveTag() == viewNode.GetID():
          break

      self.delayDisplay('Segment II invisible')
      mainWindow.moduleSelector().selectModule('Models')
      segmentII = slicer.util.getNode('LiverSegment_II')
      segmentII.GetDisplayNode().SetVisibility(0)
      slicer.util.clickAndDrag(threeDView,start=(10,200),end=(10,10))

      self.delayDisplay('Segment II visible')
      segmentII.GetDisplayNode().SetVisibility(1)
      cameraNode.GetCamera().Azimuth(0)
      cameraNode.GetCamera().Elevation(0)

      self.delayDisplay('View Adrenal')
      segmentII.GetDisplayNode().SetVisibility(1)
      cameraNode.GetCamera().Azimuth(180)
      cameraNode.GetCamera().Elevation(-30)

      segmentVII = slicer.util.getNode('LiverSegment_II')
      redWidget.sliceController().setSliceVisible(True);

      self.delayDisplay('Middle Hepatic')
      models = slicer.util.getNodes('vtkMRMLModelNode*')
      for modelNode in models.values():
        modelNode.GetDisplayNode().SetVisibility(0)

      segmentVII = slicer.util.getNode('LiverSegment_II')
      transparentNodes = ('MiddleHepaticVein_and_Branches','LiverSegment_IVb','LiverSegmentV',)
      for nodeName in transparentNodes:
        modelNode = slicer.util.getNode(nodeName)
        modelNode.GetDisplayNode().SetOpacity(0.5)
        modelNode.GetDisplayNode().SetVisibility(1)
      redWidget.sliceController().setSliceVisible(True);
      cameraNode.GetCamera().Azimuth(30)
      cameraNode.GetCamera().Elevation(-20)

      self.delayDisplay('Test passed!')
    except Exception as e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e))

  def test_Part4Lung(self):
    """ Test using the lung data
    """

    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import SampleData
    SampleData.downloadFromURL(
      fileNames='LungData.mrb',
      loadFiles=True,
      uris=TESTING_DATA_URL + 'SHA256/9da091065aa42edbba2d436a2ef21a093792e8a76455c28e5b80590b04f5a73e',
      checksums='SHA256:9da091065aa42edbba2d436a2ef21a093792e8a76455c28e5b80590b04f5a73e')
    self.delayDisplay('Finished with download and loading\n')

    try:
      mainWindow = slicer.util.mainWindow()
      layoutManager = slicer.app.layoutManager()
      threeDView = layoutManager.threeDWidget(0).threeDView()
      redWidget = layoutManager.sliceWidget('Red')
      redController = redWidget.sliceController()
      viewNode = threeDView.mrmlViewNode()
      cameras = slicer.util.getNodes('vtkMRMLCameraNode*')
      for cameraNode in cameras.values():
        if cameraNode.GetActiveTag() == viewNode.GetID():
          break

      self.delayDisplay('Reset view')
      threeDView.resetFocalPoint()
      mainWindow.moduleSelector().selectModule('Models')

      self.delayDisplay('View Question 1')
      cameraNode.GetCamera().Azimuth(-100)
      cameraNode.GetCamera().Elevation(-40)
      redWidget.sliceController().setSliceVisible(True);
      lungs = slicer.util.getNode('chestCT_lungs')
      lungs.GetDisplayNode().SetVisibility(0)

      self.delayDisplay('View Question 2')
      cameraNode.GetCamera().Azimuth(-65)
      cameraNode.GetCamera().Elevation(-20)
      lungs.GetDisplayNode().SetVisibility(1)
      lungs.GetDisplayNode().SetOpacity(0.24)
      redController.setSliceOffsetValue(-50)

      self.delayDisplay('View Question 3')
      cameraNode.GetCamera().Azimuth(-165)
      cameraNode.GetCamera().Elevation(-10)
      redWidget.sliceController().setSliceVisible(False);

      self.delayDisplay('View Question 4')
      cameraNode.GetCamera().Azimuth(20)
      cameraNode.GetCamera().Elevation(-10)
      lowerLobeNodes = slicer.util.getNodes('*LowerLobe*')
      for showNode in lowerLobeNodes:
        self.delayDisplay('Showing Node %s' % showNode, 300)
        for node in lowerLobeNodes:
          displayNode = lowerLobeNodes[node].GetDisplayNode()
          if displayNode:
            displayNode.SetVisibility(1 if node == showNode else 0)

      self.delayDisplay('Test passed!')
    except Exception as e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e))

import os
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# RSNAQuantTutorial
#

class RSNAQuantTutorial:
  def __init__(self, parent):
    parent.title = "RSNAQuantTutorial" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Steve Pieper (Isomics)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    This module was developed as a self test to perform the operations needed for the RSNA 2012 Quantitative Imaging Tutorial
    """
    parent.acknowledgementText = """
    This file was originally developed by Steve Pieper, Isomics, Inc.  and was partially funded by NIH grant 3P41RR013218-12S1.
""" # replace with organization, grant and thanks.
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['RSNAQuantTutorial'] = self.runTest

  def runTest(self):
    tester = RSNAQuantTutorialTest()
    tester.runTest()

#
# qRSNAQuantTutorialWidget
#

class RSNAQuantTutorialWidget:
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
    self.reloadButton.name = "RSNAQuantTutorial Reload"
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
    testsCollapsibleButton = ctk.ctkCollapsibleButton()
    testsCollapsibleButton.text = "Tests"
    self.layout.addWidget(testsCollapsibleButton)

    # Layout within the collapsible button
    formLayout = qt.QFormLayout(testsCollapsibleButton)

    # test buttons
    tests = ( ("Part 1 : Ruler", self.onPart1Ruler),("Part 2: ChangeTracker", self.onPart2ChangeTracker),("Part 3 : PETCT", self.onPart3PETCT) )
    for text,slot in tests:
      testButton = qt.QPushButton(text)
      testButton.toolTip = "Run the test."
      formLayout.addWidget(testButton)
      testButton.connect('clicked(bool)', slot)

    # A collapsible button to hide screen shot options
    screenShotsCollapsibleButton = ctk.ctkCollapsibleButton()
    screenShotsCollapsibleButton.text = "Screen shot options"
    self.layout.addWidget(screenShotsCollapsibleButton)

    # layout within the collapsible button
    screenShotsFormLayout = qt.QFormLayout(screenShotsCollapsibleButton)

    #
    # check box to trigger taking screen shots for later use in tutorials
    #
    self.enableScreenshotsFlagCheckBox = qt.QCheckBox()
    self.enableScreenshotsFlagCheckBox.checked = 0
    self.enableScreenshotsFlagCheckBox.setToolTip("If checked, take screen shots for tutorials. Use Save Data to write them to disk.")
    screenShotsFormLayout.addRow("Enable Screenshots", self.enableScreenshotsFlagCheckBox)

    #
    # scale factor for screen shots
    #
    self.screenshotScaleFactorSliderWidget = ctk.ctkSliderWidget()
    self.screenshotScaleFactorSliderWidget.singleStep = 1.0
    self.screenshotScaleFactorSliderWidget.minimum = 1.0
    self.screenshotScaleFactorSliderWidget.maximum = 50.0
    self.screenshotScaleFactorSliderWidget.value = 1.0
    self.screenshotScaleFactorSliderWidget.setToolTip("Set scale factor for the screen shots.")
    screenShotsFormLayout.addRow("Screenshot scale factor", self.screenshotScaleFactorSliderWidget)

    # Add vertical spacer
    self.layout.addStretch(1)

  def onPart1Ruler(self):
    enableScreenshotsFlag = self.enableScreenshotsFlagCheckBox.checked
    screenshotScaleFactor = int(self.screenshotScaleFactorSliderWidget.value)

    tester = RSNAQuantTutorialTest()
    tester.setUp()
    tester.test_Part1Ruler(enableScreenshotsFlag,screenshotScaleFactor)

  def onPart2ChangeTracker(self):
    enableScreenshotsFlag = self.enableScreenshotsFlagCheckBox.checked
    screenshotScaleFactor = int(self.screenshotScaleFactorSliderWidget.value)

    tester = RSNAQuantTutorialTest()
    tester.setUp()
    tester.test_Part2ChangeTracker(enableScreenshotsFlag,screenshotScaleFactor)

  def onPart3PETCT(self):
    enableScreenshotsFlag = self.enableScreenshotsFlagCheckBox.checked
    screenshotScaleFactor = int(self.screenshotScaleFactorSliderWidget.value)

    tester = RSNAQuantTutorialTest()
    tester.setUp()
    tester.test_Part3PETCT(enableScreenshotsFlag,screenshotScaleFactor)

  def onReload(self,moduleName="RSNAQuantTutorial"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="RSNAQuantTutorial"):
    self.onReload()
    evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
    tester = eval(evalString)
    tester.runTest()

#
# RSNAQuantTutorialLogic
#

class RSNAQuantTutorialLogic:
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


class RSNAQuantTutorialTest(unittest.TestCase):
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

  def takeScreenshot(self,name,description,type=-1):
    # show the message even if not taking a screen shot
    self.delayDisplay(description)

    if self.enableScreenshots == 0:
      return

    lm = slicer.app.layoutManager()
    # switch on the type to get the requested window
    widget = 0
    if type == -1:
      # full window
      widget = slicer.util.mainWindow()
    elif type == slicer.qMRMLScreenShotDialog().FullLayout:
      # full layout
      widget = lm.viewport()
    elif type == slicer.qMRMLScreenShotDialog().ThreeD:
      # just the 3D window
      widget = lm.threeDWidget(0).threeDView()
    elif type == slicer.qMRMLScreenShotDialog().Red:
      # red slice window
      widget = lm.sliceWidget("Red")
    elif type == slicer.qMRMLScreenShotDialog().Yellow:
      # yellow slice window
      widget = lm.sliceWidget("Yellow")
    elif type == slicer.qMRMLScreenShotDialog().Green:
      # green slice window
      widget = lm.sliceWidget("Green")

    # grab and convert to vtk image data
    qpixMap = qt.QPixmap().grabWidget(widget)
    qimage = qpixMap.toImage()
    imageData = vtk.vtkImageData()
    slicer.qMRMLUtils().qImageToVtkImageData(qimage,imageData)

    annotationLogic = slicer.modules.annotations.logic()
    annotationLogic.CreateSnapShot(name, description, type, self.screenshotScaleFactor, imageData)

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    self.delayDisplay("Closing the scene")
    layoutManager = slicer.app.layoutManager()
    layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
    slicer.mrmlScene.Clear(0)

  def clickAndDrag(self,widget,button='Left',start=(10,10),end=(10,40),steps=20,modifiers=[]):
    """Send synthetic mouse events to the specified widget (qMRMLSliceWidget or qMRMLThreeDView)
    button : "Left", "Middle", "Right", or "None"
    start, end : window coordinates for action
    steps : number of steps to move in
    modifiers : list containing zero or more of "Shift" or "Control"
    """
    style = widget.interactorStyle()
    interator = style.GetInteractor()
    if button == 'Left':
      down = style.OnLeftButtonDown
      up = style.OnLeftButtonUp
    elif button == 'Right':
      down = style.OnRightButtonDown
      up = style.OnRightButtonUp
    elif button == 'Middle':
      down = style.OnMiddleButtonDown
      up = style.OnMiddleButtonUp
    elif button == 'None' or not button:
      down = lambda : None
      up = lambda : None
    else:
      raise Exception("Bad button - should be Left or Right, not %s" % button)
    if 'Shift' in modifiers:
      interator.SetShiftKey(1)
    if 'Control' in modifiers:
      interator.SetControlKey(1)
    interator.SetEventPosition(*start)
    down()
    for step in xrange(steps):
      frac = float(step)/steps
      x = int(start[0] + frac*(end[0]-start[0]))
      y = int(start[1] + frac*(end[1]-start[1]))
      interator.SetEventPosition(x,y)
      style.OnMouseMove()
    up()
    interator.SetShiftKey(0)
    interator.SetControlKey(0)


  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_Part1Ruler()
    self.setUp()
    self.test_Part2ChangeTracker()
    self.setUp()
    self.test_Part3PETCT()

  def test_Part1Ruler(self,enableScreenshotsFlag=0,screenshotScaleFactor=1):
    """ Test using rulers
    """
    self.enableScreenshots = enableScreenshotsFlag
    self.screenshotScaleFactor = screenshotScaleFactor

    self.delayDisplay("Starting the test")

    #
    # first, get some data
    #
    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    tumor = sampleDataLogic.downloadMRBrainTumor1()

    try:
      # four up view
      layoutManager = slicer.app.layoutManager()
      layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutFourUpView)

      # annotations module
      m = slicer.util.mainWindow()
      m.moduleSelector().selectModule('Annotations')

      # add ruler 1
      rulerNode1 = slicer.vtkMRMLAnnotationRulerNode()
      rulerNode1.SetName("d1")
      rulerNode1.SetPosition1(-7.59519,43.544,28.6)
      rulerNode1.SetPosition2(-5.56987,14.177,28.6)
      rulerNode1.Initialize(slicer.mrmlScene)
      self.delayDisplay("Ruler 1")

      # add ruler 2
      rulerNode2 = slicer.vtkMRMLAnnotationRulerNode()
      rulerNode2.SetName("d2")
      rulerNode2.SetPosition1(-3.54455,27.656,13.1646)
      rulerNode2.SetPosition2(-2.5319,27.656,47.5949)
      rulerNode2.Initialize(slicer.mrmlScene)
      self.delayDisplay("Ruler 2")

      # scroll
      annotLogic = slicer.modules.annotations.logic()
      annotLogic.JumpSlicesToAnnotationCoordinate(rulerNode1.GetID())

      # show slices
      redWidget = layoutManager.sliceWidget('Red')
      redWidget.sliceController().setSliceLink(True)
      redWidget.sliceController().setSliceVisible(True);


      self.takeScreenshot('Ruler','Ruler used to measure tumor diameter',-1)

    except Exception, e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e))


  def test_Part3PETCT(self,enableScreenshotsFlag=0,screenshotScaleFactor=1):
    """ Test using the PETCT module
    """

    self.enableScreenshots = enableScreenshotsFlag
    self.screenshotScaleFactor = screenshotScaleFactor

    self.delayDisplay("Starting the test")

    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=124185', 'dataset3_PETCT.zip'),
        )

    self.delayDisplay("Downloading")

    for url,name in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        print('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
    self.delayDisplay('Finished with download\n')

    self.delayDisplay("Unzipping to  %s" % (slicer.app.temporaryPath))
    zipFilePath = slicer.app.temporaryPath + '/' + 'dataset3_PETCT.zip'
    extractPath = slicer.app.temporaryPath + '/' + 'dataset3_PETCT'
    qt.QDir().mkpath(extractPath)
    self.delayDisplay("Using extract path  %s" % (extractPath))
    applicationLogic = slicer.app.applicationLogic()
    applicationLogic.Unzip(zipFilePath, extractPath)

    self.delayDisplay("Loading PET_CT_pre-treatment.mrb")
    preTreatmentPath = extractPath + '/PET_CT_pre-treatment.mrb'
    slicer.util.loadScene(preTreatmentPath)
    self.takeScreenshot('PETCT-LoadedPre','Loaded pre-treatement scene',-1)

    try:
      logic = RSNAQuantTutorialLogic()
      mainWindow = slicer.util.mainWindow()
      layoutManager = slicer.app.layoutManager()
      threeDView = layoutManager.threeDWidget(0).threeDView()
      redWidget = layoutManager.sliceWidget('Red')
      redController = redWidget.sliceController()
      greenWidget = layoutManager.sliceWidget('Green')
      greenController = greenWidget.sliceController()
      yellowWidget = layoutManager.sliceWidget('Yellow')
      yellowController = yellowWidget.sliceController()
      viewNode = threeDView.mrmlViewNode()
      cameras = slicer.util.getNodes('vtkMRMLCameraNode*')
      for cameraNode in cameras.values():
        if cameraNode.GetActiveTag() == viewNode.GetID():
          break

      threeDView.resetFocalPoint()
      self.clickAndDrag(threeDView,button='Right')
      redWidget.sliceController().setSliceVisible(True)
      yellowWidget.sliceController().setSliceVisible(True)
      self.takeScreenshot('PETCT-ConfigureView','Configure View',-1)

      mainWindow.moduleSelector().selectModule('Volumes')
      compositNode = redWidget.mrmlSliceCompositeNode()
      compositNode.SetForegroundOpacity(0.2)
      self.takeScreenshot('PETCT-ShowVolumes','Show Volumes with lesion',-1)

      compositNode.SetForegroundOpacity(0.5)
      self.takeScreenshot('PETCT-CTOpacity','CT1 volume opacity to 0.5',-1)

      yellowWidget.sliceController().setSliceVisible(False)
      greenWidget.sliceController().setSliceVisible(True)
      self.takeScreenshot('PETCT-ShowSlices','Show axial and sagittal slices',-1)

      self.delayDisplay('SUV Computation')
      if not hasattr(slicer.modules, 'petstandarduptakevaluecomputation'):
        self.delayDisplay("PET SUV Computation not available, skipping the test.")
        return

      slicer.util.selectModule('PETStandardUptakeValueComputation')

      parameters = {
          "PETDICOMPath": extractPath + '/' + 'PET1',
          "PETVolume": slicer.util.getNode('PET1'),
          "VOIVolume": slicer.util.getNode('PET1-label'),
          }

      suvComputation = slicer.modules.petstandarduptakevaluecomputation
      self.CLINode1 = None
      self.CLINode1 = slicer.cli.run(suvComputation, self.CLINode1, parameters, delete_temporary_files=False)
      waitCount = 0
      while self.CLINode1.GetStatusString() != 'Completed' and waitCount < 100:
        self.delayDisplay( "Running SUV Computation... %d" % waitCount )
        waitCount += 1

      # close the scene
      slicer.mrmlScene.Clear(0)

      self.delayDisplay("Loading PET_CT_post-treatment.mrb")
      postTreatmentPath = extractPath + '/PET_CT_post-treatment.mrb'
      slicer.util.loadScene(postTreatmentPath)
      self.takeScreenshot('PETCT-LoadedPost','Loaded post-treatement scene',-1)

      compositNode.SetForegroundOpacity(0.5)
      self.takeScreenshot('PETCT-CT2Opacity','CT2 volume opacity to 0.5',-1)

      redController.setSliceOffsetValue(-165.01)
      self.takeScreenshot('PETCT-LarynxUptake','Mild uptake in the larynx and pharynx',-1)

      redController.setSliceOffsetValue(-106.15)
      self.takeScreenshot('PETCT-TumorUptake','No uptake in the area of the primary tumor',-1)

      self.delayDisplay('Test passed!')
    except Exception, e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e))


  def test_Part2ChangeTracker(self,enableScreenshotsFlag=0,screenshotScaleFactor=1):
    """ Test the ChangeTracker module
    """

    self.enableScreenshots = enableScreenshotsFlag
    self.screenshotScaleFactor = screenshotScaleFactor

    self.delayDisplay("Starting the test")

    if not hasattr(slicer.modules, 'changetracker'):
      self.delayDisplay("ChangeTracker not available, skipping the test.")
      return

    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=124184', 'ChangeTrackerScene.mrb', slicer.util.loadScene),
        )

    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        print('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        print('Loading %s...\n' % (name,))
        loader(filePath)
    self.takeScreenshot('ChangeTracker-Loaded','Finished with download and loading',-1)

    try:
      logic = RSNAQuantTutorialLogic()
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

      self.delayDisplay('Configure Module')
      mainWindow.moduleSelector().selectModule('ChangeTracker')
      self.takeScreenshot('ChangeTracker-ModuleGUI','Select the ChangeTracker module',-1)

      changeTracker = slicer.modules.changetracker.widgetRepresentation().self()

      baselineNode = slicer.util.getNode('2006-spgr1')
      followupNode = slicer.util.getNode('2007-spgr1')
      changeTracker.selectScansStep._ChangeTrackerSelectScansStep__baselineVolumeSelector.setCurrentNode(baselineNode)
      changeTracker.selectScansStep._ChangeTrackerSelectScansStep__followupVolumeSelector.setCurrentNode(followupNode)
      self.takeScreenshot('ChangeTracker-SetInputs','Select input scans',-1)

      changeTracker.workflow.goForward()
      self.takeScreenshot('ChangeTracker-GoForward','Go Forward',-1)

      self.clickAndDrag(redWidget,button='Right')
      self.takeScreenshot('ChangeTracker-Zoom','Inspect - zoom',-1)

      self.clickAndDrag(redWidget,button='Middle')
      self.takeScreenshot('ChangeTracker-Pan','Inspect - pan',-1)

      for offset in xrange(-20,20,2):
        redController.setSliceOffsetValue(offset)
      self.takeScreenshot('ChangeTracker-Scroll','Inspect - scroll',-1)

      self.delayDisplay('Set ROI')
      roi = changeTracker.defineROIStep._ChangeTrackerDefineROIStep__roi
      roi.SetXYZ(-2.81037, 28.7629, 28.4536)
      self.takeScreenshot('ChangeTracker-SetROICenter','Center VOI',-1)
      roi.SetRadiusXYZ(22.6467, 22.6804, 22.9897)
      self.takeScreenshot('ChangeTracker-SetROIExtent','Resize the VOI',-1)

      layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalWidescreenView)
      self.takeScreenshot('ChangeTracker-ConventionalWidescreen','Select the viewing mode Conventional Widescreen',-1)

      self.clickAndDrag(redWidget,button='Right')
      self.takeScreenshot('ChangeTracker-ZoomVOI','Zoom',-1)

      layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutFourUpView)
      self.takeScreenshot('ChangeTracker-FourUpLayout','Go back to Four-Up layout',-1)

      changeTracker.workflow.goForward()
      self.takeScreenshot('ChangeTracker-GoForward','Go Forward',-1)

      changeTracker.segmentROIStep._ChangeTrackerSegmentROIStep__threshRange.minimumValue = 120
      self.takeScreenshot('ChangeTracker-Threshold','Set threshold',-1)

      changeTracker.workflow.goForward()
      self.takeScreenshot('ChangeTracker-GoForward','Go Forward',-1)

      checkList = changeTracker.analyzeROIStep._ChangeTrackerAnalyzeROIStep__metricCheckboxList
      index = checkList.values().index('IntensityDifferenceMetric')
      checkList.keys()[index].checked = True
      self.takeScreenshot('ChangeTracker-PickMetric','Select the ROI analysis method',-1)

      changeTracker.workflow.goForward()
      self.takeScreenshot('ChangeTracker-GoForward','Go Forward',-1)

      self.delayDisplay('Look!')
      redWidget.sliceController().setSliceVisible(True);


      self.delayDisplay('Crosshairs')
      compareWidget = layoutManager.sliceWidget('Compare1')
      style = compareWidget.interactorStyle()
      interator = style.GetInteractor()
      for step in xrange(100):
        interator.SetEventPosition(10,step)
        style.OnMouseMove()

      self.delayDisplay('Zoom')
      self.clickAndDrag(compareWidget,button='Right')

      self.delayDisplay('Pan')
      self.clickAndDrag(compareWidget,button='Middle')

      self.delayDisplay('Inspect - scroll')
      compareController = redWidget.sliceController()
      for offset in xrange(10,30,2):
        compareController.setSliceOffsetValue(offset)

      self.takeScreenshot('ChangeTracker-InspectResults','Inspected results',-1)

      self.delayDisplay('Test passed!')
    except Exception, e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e))

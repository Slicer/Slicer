import os
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# RSNAVisTutorial
#

class RSNAVisTutorial:
  def __init__(self, parent):
    parent.title = "RSNAVisTutorial" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Steve Pieper (Isomics)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    This module was developed as a self test to perform the operations needed for the RSNA 2012 Visualization Tutorial
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
    slicer.selfTests['RSNAVisTutorial'] = self.runTest

  def runTest(self):
    tester = RSNAVisTutorialTest()
    tester.runTest()

#
# qRSNAVisTutorialWidget
#

class RSNAVisTutorialWidget:
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
    self.reloadButton.name = "RSNAVisTutorial Reload"
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
    tests = ( ("Part 1: DICOM",self.onPart1DICOM),("Part 2: Head", self.onPart2Head),("Part 3: Liver", self.onPart3Liver),("Part 4: Lung", self.onPart4Lung),)
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

  def onPart1DICOM(self):
    enableScreenshotsFlag = self.enableScreenshotsFlagCheckBox.checked
    screenshotScaleFactor = int(self.screenshotScaleFactorSliderWidget.value)

    tester = RSNAVisTutorialTest()
    tester.setUp()
    tester.test_Part1DICOM(enableScreenshotsFlag,screenshotScaleFactor)

  def onPart2Head(self):
    enableScreenshotsFlag = self.enableScreenshotsFlagCheckBox.checked
    screenshotScaleFactor = int(self.screenshotScaleFactorSliderWidget.value)

    tester = RSNAVisTutorialTest()
    tester.setUp()
    tester.test_Part2Head(enableScreenshotsFlag,screenshotScaleFactor)

  def onPart3Liver(self):
    enableScreenshotsFlag = self.enableScreenshotsFlagCheckBox.checked
    screenshotScaleFactor = int(self.screenshotScaleFactorSliderWidget.value)

    tester = RSNAVisTutorialTest()
    tester.setUp()
    tester.test_Part3Liver(enableScreenshotsFlag,screenshotScaleFactor)

  def onPart4Lung(self):
    enableScreenshotsFlag = self.enableScreenshotsFlagCheckBox.checked
    screenshotScaleFactor = int(self.screenshotScaleFactorSliderWidget.value)

    tester = RSNAVisTutorialTest()
    tester.setUp()
    tester.test_Part4Lung(enableScreenshotsFlag,screenshotScaleFactor)

  def onReload(self,moduleName="RSNAVisTutorial"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="RSNAVisTutorial"):
    self.onReload()
    evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
    tester = eval(evalString)
    tester.runTest()

#
# RSNAVisTutorialLogic
#

class RSNAVisTutorialLogic:
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


class RSNAVisTutorialTest(unittest.TestCase):
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
    self.test_Part1DICOM()
    self.setUp()
    self.test_Part2Head()
    self.setUp()
    self.test_Part3Liver()
    self.setUp()
    self.test_Part4Lung()

  def test_Part1DICOM(self,enableScreenshotsFlag=0,screenshotScaleFactor=1):
    """ Test the DICOM part of the test using the head atlas
    """

    self.enableScreenshots = enableScreenshotsFlag
    self.screenshotScaleFactor = screenshotScaleFactor

    import os
    self.delayDisplay("Starting the DICOM test")
    #
    # first, get the data - a zip file of dicom data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=124183', 'dataset1_Thorax_Abdomen.zip'),
        )

    self.delayDisplay("Downloading")
    for url,name in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        self.delayDisplay('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
    self.delayDisplay('Finished with download\n')

    self.delayDisplay("Unzipping")
    dicomFilesDirectory = slicer.app.temporaryPath + '/dicomFiles'
    qt.QDir().mkpath(dicomFilesDirectory)
    slicer.app.applicationLogic().Unzip(filePath, dicomFilesDirectory)

    try:
      self.delayDisplay("Switching to temp database directory")
      tempDatabaseDirectory = slicer.app.temporaryPath + '/tempDICOMDatabase'
      qt.QDir().mkpath(tempDatabaseDirectory)
      if slicer.dicomDatabase:
        originalDatabaseDirectory = os.path.split(slicer.dicomDatabase.databaseFilename)[0]
      else:
        originalDatabaseDirectory = None
        settings = qt.QSettings()
        settings.setValue('DatabaseDirectory', tempDatabaseDirectory)
      dicomWidget = slicer.modules.dicom.widgetRepresentation().self()
      dicomWidget.onDatabaseDirectoryChanged(tempDatabaseDirectory)

      self.delayDisplay('Importing DICOM')
      mainWindow = slicer.util.mainWindow()
      mainWindow.moduleSelector().selectModule('DICOM')

      indexer = ctk.ctkDICOMIndexer()
      indexer.addDirectory(slicer.dicomDatabase, dicomFilesDirectory, None)
      indexer.waitForImportFinished()

      dicomWidget.detailsPopup.open()

      # load the data by series UID
      dicomWidget.detailsPopup.offerLoadables('1.3.12.2.1107.5.1.4.50025.30000005060811542834300000776','Series')
      dicomWidget.detailsPopup.examineForLoading()

      self.delayDisplay('Loading Selection')
      dicomWidget.detailsPopup.loadCheckedLoadables()

      self.takeScreenshot('LoadingADICOMVolume-Loaded','Loaded DICOM Volume',-1)

      layoutManager = slicer.app.layoutManager()
      redWidget = layoutManager.sliceWidget('Red')
      self.clickAndDrag(redWidget,start=(10,10),end=(10,40))
      self.clickAndDrag(redWidget,start=(10,10),end=(40,10))

      self.takeScreenshot('LoadingADICOMVolume-WL','Changed level and window',-1)

      redWidget.sliceController().setSliceLink(True)
      redWidget.sliceController().setSliceVisible(True);
      self.takeScreenshot('LoadingADICOMVolume-LinkView','Linked and visible',-1)

      self.clickAndDrag(redWidget,button='Right',start=(10,10),end=(10,40))
      self.takeScreenshot('LoadingADICOMVolume-Zoom','Zoom',-1)

      threeDView = layoutManager.threeDWidget(0).threeDView()
      self.clickAndDrag(threeDView)
      self.takeScreenshot('LoadingADICOMVolume-Rotate','Rotate',-1)

      threeDView.resetFocalPoint()
      self.takeScreenshot('LoadingADICOMVolume-Center','Center the view',-1)

      layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalWidescreenView)
      self.takeScreenshot('LoadingADICOMVolume-ConventionalWidescreen','Conventional Widescreen Layout',-1)

      slicer.util.mainWindow().moduleSelector().selectModule('VolumeRendering')
      self.takeScreenshot('VolumeRendering-Module','Volume Rendering',-1)

      volumeRenderingWidgetRep = slicer.modules.volumerendering.widgetRepresentation()
      abdomenVolume = slicer.mrmlScene.GetFirstNodeByName('6: CT_Thorax_Abdomen')
      volumeRenderingWidgetRep.setMRMLVolumeNode(abdomenVolume)
      self.takeScreenshot('VolumeRendering-SelectVolume','Select the volume 6: CT_Thorax_Abdomen',-1)

      presetsScene = slicer.modules.volumerendering.logic().GetPresetsScene()
      ctCardiac3 = presetsScene.GetFirstNodeByName('CT-Cardiac3')
      volumeRenderingWidgetRep.applyPreset(ctCardiac3)
      self.takeScreenshot('VolumeRendering-SelectPreset','Select the Preset CT-Cardiac-3')

      self.delayDisplay('Skipping: Select VTK CPU Ray Casting')

      volumeRenderingNode = slicer.mrmlScene.GetFirstNodeByName('VolumeRendering')
      volumeRenderingNode.SetVisibility(1)
      self.takeScreenshot('VolumeRendering-ViewRendering','View Volume Rendering',-1)

      self.delayDisplay('Skipping Move the Shift slider')

      redWidget.sliceController().setSliceVisible(False);
      self.takeScreenshot('VolumeRendering-SlicesOff','Turn off visibility of slices in 3D',-1)

      threeDView = layoutManager.threeDWidget(0).threeDView()
      self.clickAndDrag(threeDView)
      self.takeScreenshot('VolumeRendering-RotateVolumeRendering','Rotate volume rendered image',-1)

      volumeRenderingNode.SetVisibility(0)
      self.takeScreenshot('VolumeRendering-TurnOffVolumeRendering','Turn off volume rendered image',-1)

      volumeRenderingNode.SetCroppingEnabled(1)
      annotationROI = slicer.mrmlScene.GetFirstNodeByName('AnnotationROI')
      annotationROI.SetDisplayVisibility(1)
      self.takeScreenshot('VolumeRendering-DisplayROI','Enable cropping and display ROI',-1)

      redWidget.sliceController().setSliceVisible(True)
      self.takeScreenshot('VolumeRendering-SlicesOn','Turn on visibility of slices in 3D',-1)

      annotationROI.SetXYZ(-79.61,154.16,-232.591)
      annotationROI.SetRadiusXYZ(43.4,65.19,70.5)
      self.takeScreenshot('VolumeRendering-SizedROI','Position the ROI over a kidney',-1)

      volumeRenderingNode.SetVisibility(1)
      self.takeScreenshot('VolumeRendering-ROIRendering','ROI volume rendered',-1)

      annotationROI.SetXYZ(15,146,-186)
      annotationROI.SetRadiusXYZ(138,57,61)
      self.takeScreenshot('VolumeRendering-BothKidneys','Rendered both kidneys',-1)

      self.delayDisplay('Test passed!')
    except Exception, e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e))

    self.delayDisplay("Restoring original database directory")
    if originalDatabaseDirectory:
      dicomWidget.onDatabaseDirectoryChanged(originalDatabaseDirectory)

  def test_Part2Head(self,enableScreenshotsFlag=0,screenshotScaleFactor=1):
    """ Test using the head atlas - may not be needed - Slicer4Minute is already tested
    """
    self.enableScreenshots = enableScreenshotsFlag
    self.screenshotScaleFactor = screenshotScaleFactor

    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=124180', 'Head_Scene.mrb', slicer.util.loadScene),
        )

    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        print('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        print('Loading %s...\n' % (name,))
        loader(filePath)
    self.takeScreenshot('Head-Downloaded','Finished with download and loading',-1)

    try:
      logic = RSNAVisTutorialLogic()
      mainWindow = slicer.util.mainWindow()
      layoutManager = slicer.app.layoutManager()
      threeDView = layoutManager.threeDWidget(0).threeDView()
      redWidget = layoutManager.sliceWidget('Red')
      redController = redWidget.sliceController()
      greenWidget = layoutManager.sliceWidget('Green')
      greenController = greenWidget.sliceController()

      mainWindow.moduleSelector().selectModule('Models')
      redWidget.sliceController().setSliceVisible(True);
      self.takeScreenshot('Head-ModelsAndSliceModel','Models and Slice Model',-1)

      self.clickAndDrag(threeDView)
      self.takeScreenshot('Head-Rotate','Rotate',-1)

      redController.setSliceVisible(True);
      self.takeScreenshot('Head-AxialSlice','Display Axial Slice',-1)

      layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
      self.takeScreenshot('Head-ConventionalView','Conventional Layout',-1)

      viewNode = threeDView.mrmlViewNode()
      cameras = slicer.util.getNodes('vtkMRMLCameraNode*')
      for cameraNode in cameras.values():
        if cameraNode.GetActiveTag() == viewNode.GetID():
          break
      cameraNode.GetCamera().Azimuth(90)
      cameraNode.GetCamera().Elevation(20)

      # turn off skin and skull
      skin = slicer.util.getNode(pattern='Skin.vtk')
      skin.GetDisplayNode().SetOpacity(0.5)
      self.takeScreenshot('Head-SkinOpacity','Skin Opacity to 0.5',-1)

      skin.GetDisplayNode().SetVisibility(0)
      self.takeScreenshot('Head-SkinOpacityZero','Skin Opacity to 0',-1)

      skull = slicer.util.getNode(pattern='skull_bone.vtk')

      greenWidget.sliceController().setSliceVisible(True);
      self.takeScreenshot('Head-GreenSlice','Display Coronal Slice',-1)

      # hemispheric_white_matter.GetDisplayNode().SetClipping(1)
      skull.GetDisplayNode().SetClipping(1)
      clip = slicer.util.getNode(pattern='vtkMRMLClipModelsNode1')
      clip.SetRedSliceClipState(0)
      clip.SetYellowSliceClipState(0)
      clip.SetGreenSliceClipState(2)
      self.takeScreenshot('Head-SkullClipping','Turn on clipping for skull model',-1)

      for offset in xrange(-20,20,2):
        greenController.setSliceOffsetValue(offset)
      self.takeScreenshot('Head-ScrollCoronal','Scroll through coronal slices',-1)

      skull.GetDisplayNode().SetVisibility(0)
      self.takeScreenshot('Head-HideSkull','Make the skull invisible',-1)

      for offset in xrange(-40,-20,2):
        greenController.setSliceOffsetValue(offset)
      self.takeScreenshot('Head-ScrollCoronalWhiteMatter','Scroll through coronal slices to show white matter',-1)

      hemispheric_white_matter = slicer.util.getNode(pattern='hemispheric_white_matter.vtk')
      hemispheric_white_matter.GetDisplayNode().SetVisibility(0)
      self.takeScreenshot('Head-HideWhiteMatter','Turn off white matter',-1)

      self.delayDisplay('Rotate')
      self.clickAndDrag(threeDView)

      self.delayDisplay('Zoom')
      threeDView = layoutManager.threeDWidget(0).threeDView()
      self.clickAndDrag(threeDView,button='Right')
      self.takeScreenshot('Head-Zoom','Zoom',-1)

      self.delayDisplay('Test passed!')
    except Exception, e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e))


  def test_Part3Liver(self,enableScreenshotsFlag=0,screenshotScaleFactor=1):
    """ Test using the liver example data
    """
    self.enableScreenshots = enableScreenshotsFlag
    self.screenshotScaleFactor = screenshotScaleFactor

    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=124181', 'LiverSegments_Scene.mrb', slicer.util.loadScene),
        )

    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        print('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        print('Loading %s...\n' % (name,))
        loader(filePath)
    self.takeScreenshot('Liver-Loaded','Loaded Liver scene',-1)


    try:
      logic = RSNAVisTutorialLogic()
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

      mainWindow.moduleSelector().selectModule('Models')
      self.takeScreenshot('Liver-Models','Models module',-1)

      segmentII = slicer.util.getNode('LiverSegment_II')
      segmentII.GetDisplayNode().SetVisibility(0)
      self.clickAndDrag(threeDView,start=(10,200),end=(10,10))
      self.takeScreenshot('Liver-SegmentII','Segment II invisible',-1)

      segmentII.GetDisplayNode().SetVisibility(1)
      self.takeScreenshot('Liver-SegmentII','Segment II visible',-1)

      cameraNode.GetCamera().Azimuth(0)
      cameraNode.GetCamera().Elevation(0)
      self.takeScreenshot('Liver-SuperiorView','Superior view',-1)

      segmentII.GetDisplayNode().SetVisibility(0)
      cameraNode.GetCamera().Azimuth(180)
      cameraNode.GetCamera().Elevation(-30)
      redWidget.sliceController().setSliceVisible(True);
      self.takeScreenshot('Liver-ViewAdrenal','View Adrenal',-1)

      models = slicer.util.getNodes('vtkMRMLModelNode*')
      for modelNode in models.values():
        modelNode.GetDisplayNode().SetVisibility(0)

      transparentNodes = ('MiddleHepaticVein_and_Branches','LiverSegment_IVb','LiverSegmentV',)
      for nodeName in transparentNodes:
        modelNode = slicer.util.getNode(nodeName)
        modelNode.GetDisplayNode().SetOpacity(0.5)
        modelNode.GetDisplayNode().SetVisibility(1)
      cameraNode.GetCamera().Azimuth(30)
      cameraNode.GetCamera().Elevation(-20)
      redWidget.sliceController().setSliceVisible(True);
      self.takeScreenshot('Liver-MiddleHepatic','Middle Hepatic',-1)

      self.delayDisplay('Test passed!')
    except Exception, e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e))

  def test_Part4Lung(self,enableScreenshotsFlag=0,screenshotScaleFactor=1):
    """ Test using the lung data
    """
    self.enableScreenshots = enableScreenshotsFlag
    self.screenshotScaleFactor = screenshotScaleFactor

    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=124182', 'LungSegments_Scene.mrb', slicer.util.loadScene),
        )

    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        print('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        print('Loading %s...\n' % (name,))
        loader(filePath)
    self.takeScreenshot('Lung-Loaded','Finished with download and loading',-1)

    try:
      logic = RSNAVisTutorialLogic()
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

      threeDView.resetFocalPoint()
      self.takeScreenshot('Lung-ResetView','Reset view',-1)

      mainWindow.moduleSelector().selectModule('Models')
      self.takeScreenshot('Lung-Models','Models module',-1)

      cameraNode.GetCamera().Azimuth(-100)
      cameraNode.GetCamera().Elevation(-40)
      redWidget.sliceController().setSliceVisible(True);
      lungs = slicer.util.getNode('chestCT_lungs')
      lungs.GetDisplayNode().SetVisibility(0)
      self.takeScreenshot('Lung-Question1','View Question 1',-1)

      cameraNode.GetCamera().Azimuth(-65)
      cameraNode.GetCamera().Elevation(-20)
      lungs.GetDisplayNode().SetVisibility(1)
      lungs.GetDisplayNode().SetOpacity(0.24)
      redController.setSliceOffsetValue(-50)
      self.takeScreenshot('Lung-Question2','View Question 2',-1)

      cameraNode.GetCamera().Azimuth(-165)
      cameraNode.GetCamera().Elevation(-10)
      redWidget.sliceController().setSliceVisible(False);
      self.takeScreenshot('Lung-Question3','View Question 3',-1)

      cameraNode.GetCamera().Azimuth(20)
      cameraNode.GetCamera().Elevation(-10)
      lowerLobeNodes = slicer.util.getNodes('*LowerLobe*')
      for showNode in lowerLobeNodes:
        self.delayDisplay('Showing Node %s' % showNode, 300)
        for node in lowerLobeNodes:
          displayNode = lowerLobeNodes[node].GetDisplayNode()
          if displayNode:
            displayNode.SetVisibility(1 if node == showNode else 0)
      self.takeScreenshot('Lung-Question4','View Question 4',-1)

      self.delayDisplay('Test passed!')
    except Exception, e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e))

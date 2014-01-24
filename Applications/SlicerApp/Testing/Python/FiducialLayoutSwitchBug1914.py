import os
import unittest
import math
from __main__ import vtk, qt, ctk, slicer

#
# FiducialLayoutSwitchBug1914
#

class FiducialLayoutSwitchBug1914:
  def __init__(self, parent):
    parent.title = "FiducialLayoutSwitchBug1914"
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Nicole Aucoin (BWH)"]
    parent.helpText = """
    Test for bug 1914, misplaced fiducial after switching layouts.
    """
    parent.acknowledgementText = """
    This file was originally developed by Nicole Aucoin, BWH  and was partially funded by NIH grant 3P41RR013218-12S1.
"""
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['FiducialLayoutSwitchBug1914'] = self.runTest

  def runTest(self):
    tester = FiducialLayoutSwitchBug1914Test()
    tester.runTest()

#
# qFiducialLayoutSwitchBug1914Widget
#

class FiducialLayoutSwitchBug1914Widget:
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

    #
    # Reload and Test area
    #
    reloadCollapsibleButton = ctk.ctkCollapsibleButton()
    reloadCollapsibleButton.text = "Reload && Test"
    self.layout.addWidget(reloadCollapsibleButton)
    reloadFormLayout = qt.QFormLayout(reloadCollapsibleButton)

    # reload button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadButton = qt.QPushButton("Reload")
    self.reloadButton.toolTip = "Reload this module."
    self.reloadButton.name = "FiducialLayoutSwitchBug1914 Reload"
    reloadFormLayout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    reloadFormLayout.addWidget(self.reloadAndTestButton)
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    #
    # Parameters Area
    #
    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Parameters"
    self.layout.addWidget(parametersCollapsibleButton)

    # Layout within the dummy collapsible button
    parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)

    #
    # check box to trigger taking screen shots for later use in tutorials
    #
    self.enableScreenshotsFlagCheckBox = qt.QCheckBox()
    self.enableScreenshotsFlagCheckBox.checked = 1
    self.enableScreenshotsFlagCheckBox.setToolTip("If checked, take screen shots for tutorials. Use Save Data to write them to disk.")
    parametersFormLayout.addRow("Enable Screenshots", self.enableScreenshotsFlagCheckBox)

    #
    # scale factor for screen shots
    #
    self.screenshotScaleFactorSliderWidget = ctk.ctkSliderWidget()
    self.screenshotScaleFactorSliderWidget.singleStep = 1.0
    self.screenshotScaleFactorSliderWidget.minimum = 1.0
    self.screenshotScaleFactorSliderWidget.maximum = 50.0
    self.screenshotScaleFactorSliderWidget.value = 1.0
    self.screenshotScaleFactorSliderWidget.setToolTip("Set scale factor for the screen shots.")
    parametersFormLayout.addRow("Screenshot scale factor", self.screenshotScaleFactorSliderWidget)

    #
    # Apply Button
    #
    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.toolTip = "Run the algorithm."
    self.applyButton.enabled = True
    parametersFormLayout.addRow(self.applyButton)

    # connections
    self.applyButton.connect('clicked(bool)', self.onApplyButton)

    # Add vertical spacer
    self.layout.addStretch(1)

  def cleanup(self):
    pass

  def onApplyButton(self):
    logic = FiducialLayoutSwitchBug1914Logic()
    enableScreenshotsFlag = self.enableScreenshotsFlagCheckBox.checked
    screenshotScaleFactor = int(self.screenshotScaleFactorSliderWidget.value)
    print("Run the algorithm")
    logic.run(enableScreenshotsFlag,screenshotScaleFactor)

  def onReload(self,moduleName="FiducialLayoutSwitchBug1914"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="FiducialLayoutSwitchBug1914"):
    try:
      self.onReload()
      evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
      tester = eval(evalString)
      tester.runTest()
    except Exception, e:
      import traceback
      traceback.print_exc()
      qt.QMessageBox.warning(slicer.util.mainWindow(),
          "Reload and Test", 'Exception!\n\n' + str(e) + "\n\nSee Python Console for Stack Trace")


#
# FiducialLayoutSwitchBug1914Logic
#

class FiducialLayoutSwitchBug1914Logic:
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass

  def delayDisplay(self,message,msec=1000):
    #
    # logic version of delay display
    #
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

  def getFiducialSliceDisplayableManagerHelper(self,sliceName='Red'):
    sliceWidget = slicer.app.layoutManager().sliceWidget(sliceName)
    sliceView = sliceWidget.sliceView()
    collection = vtk.vtkCollection()
    sliceView.getDisplayableManagers(collection)
    for i in range(collection.GetNumberOfItems()):
     m = collection.GetItemAsObject(i)
     if m.GetClassName() == "vtkMRMLMarkupsFiducialDisplayableManager2D":
       return m.GetHelper()
    return None

  def run(self,enableScreenshots=0,screenshotScaleFactor=1):
    """
    Run the actual algorithm
    """

    self.delayDisplay('Running the aglorithm')

    self.enableScreenshots = enableScreenshots
    self.screenshotScaleFactor = screenshotScaleFactor

    # Start in conventional layout
    lm = slicer.app.layoutManager()
    lm.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
    # without this delayed display, when running from the cmd line Slicer starts
    # up in a different layout and the seed won't get rendered in the right spot
    self.delayDisplay("Conventional view")

    # Download MRHead from sample data
    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    print("Getting MR Head Volume")
    mrHeadVolume = sampleDataLogic.downloadMRHead()

    # Place a fiducial on the red slice
    markupsLogic = slicer.modules.markups.logic()
    eye = [33.4975, 79.4042, -10.2143]
    fidIndex = markupsLogic.AddFiducial(eye[0], eye[1], eye[2])
    fidID = markupsLogic.GetActiveListID()
    fidNode = slicer.mrmlScene.GetNodeByID(fidID)
    self.delayDisplay("Placed a fiducial")

    # Pan and zoom
    sliceWidget = slicer.app.layoutManager().sliceWidget('Red')
    sliceLogic = sliceWidget.sliceLogic()
    compositeNode = sliceLogic.GetSliceCompositeNode()
    sliceNode = sliceLogic.GetSliceNode()
    sliceNode.SetXYZOrigin(-71.7, 129.7, 0.0)
    sliceNode.SetFieldOfView(98.3, 130.5, 1.0)
    self.delayDisplay("Panned and zoomed")

    # Get the seed widget seed location
    startingSeedDisplayCoords = [0.0, 0.0, 0.0]
    helper = self.getFiducialSliceDisplayableManagerHelper('Red')
    if helper != None:
     seedWidget = helper.GetWidget(fidNode)
     seedRepresentation = seedWidget.GetSeedRepresentation()
     handleRep = seedRepresentation.GetHandleRepresentation(fidIndex)
     startingSeedDisplayCoords = handleRep.GetDisplayPosition()
     print('Starting seed display coords = %d, %d, %d' % (startingSeedDisplayCoords[0], startingSeedDisplayCoords[1], startingSeedDisplayCoords[2]))
    self.takeScreenshot('FiducialLayoutSwitchBug1914-StartingPosition','Fiducial starting position',slicer.qMRMLScreenShotDialog().Red)

    # Switch to red slice only
    lm.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpRedSliceView)
    self.delayDisplay("Red Slice only")

    # Switch to conventional layout
    lm.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
    self.delayDisplay("Conventional layout")

    # Get the current seed widget seed location
    endingSeedDisplayCoords = [0.0, 0.0, 0.0]
    helper = self.getFiducialSliceDisplayableManagerHelper('Red')
    if helper != None:
     seedWidget = helper.GetWidget(fidNode)
     seedRepresentation = seedWidget.GetSeedRepresentation()
     handleRep = seedRepresentation.GetHandleRepresentation(fidIndex)
     endingSeedDisplayCoords = handleRep.GetDisplayPosition()
     print('Ending seed display coords = %d, %d, %d' % (endingSeedDisplayCoords[0], endingSeedDisplayCoords[1], endingSeedDisplayCoords[2]))
    self.takeScreenshot('FiducialLayoutSwitchBug1914-EndingPosition','Fiducial ending position',slicer.qMRMLScreenShotDialog().Red)

    # Compare to original seed widget location
    diff = math.pow((startingSeedDisplayCoords[0] - endingSeedDisplayCoords[0]),2) + math.pow((startingSeedDisplayCoords[1] - endingSeedDisplayCoords[1]),2) + math.pow((startingSeedDisplayCoords[2] - endingSeedDisplayCoords[2]),2)
    if diff != 0.0:
      diff = math.sqrt(diff)
    self.delayDisplay("Difference between starting and ending seed display coordinates = %g" % diff)

    if diff > 1.0:
      # raise Exception("Display coordinate difference is too large!\nExpected < 1.0 but got %g" % (diff))
      print("Display coordinate difference is too large!\nExpected < 1.0 but got %g" % (diff))
      return False

    if enableScreenshots == 1:
      # compare the screen snapshots
      startView = slicer.mrmlScene.GetFirstNodeByName('FiducialLayoutSwitchBug1914-StartingPosition')
      startShot = startView.GetScreenShot()
      endView = slicer.mrmlScene.GetFirstNodeByName('FiducialLayoutSwitchBug1914-EndingPosition')
      endShot = endView.GetScreenShot()
      imageMath = vtk.vtkImageMathematics()
      imageMath.SetOperationToSubtract()
      imageMath.SetInput1(startShot)
      imageMath.SetInput2(endShot)
      imageMath.Update()
      shotDiff = imageMath.GetOutput()
      # save it as a scene view
      annotationLogic = slicer.modules.annotations.logic()
      annotationLogic.CreateSnapShot("FiducialLayoutSwitchBug1914-Diff", "Difference between starting and ending fiducial seed positions",slicer.qMRMLScreenShotDialog().Red, screenshotScaleFactor, shotDiff)
      # calculate the image difference
      imageStats = vtk.vtkImageHistogramStatistics()
      imageStats.SetInput(shotDiff)
      imageStats.GenerateHistogramImageOff()
      imageStats.Update()
      meanVal = imageStats.GetMean()
      self.delayDisplay("Mean of image difference = %g" % meanVal)
      if meanVal > 5.0:
        # raise Exception("Image difference is too great!\nExpected <= 5.0, but got %g" % (meanVal))
        print("Image difference is too great!\nExpected <= 5.0, but got %g" % (meanVal))
        return False

    self.delayDisplay('Test passed!')
    return True


class FiducialLayoutSwitchBug1914Test(unittest.TestCase):
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
    self.test_FiducialLayoutSwitchBug19141()

  def test_FiducialLayoutSwitchBug19141(self):
    logic = FiducialLayoutSwitchBug1914Logic()
    returnFlag = logic.run()
    if returnFlag == True:
      self.delayDisplay('Test passed!')
    else:
      self.delayDisplay('Test failed!')



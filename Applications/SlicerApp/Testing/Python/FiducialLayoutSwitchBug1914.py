from __future__ import print_function
import os
import unittest
import math
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

#
# FiducialLayoutSwitchBug1914
#

class FiducialLayoutSwitchBug1914(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
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

#
# qFiducialLayoutSwitchBug1914Widget
#

class FiducialLayoutSwitchBug1914Widget(ScriptedLoadableModuleWidget):

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    # Instantiate and connect widgets ...

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


#
# FiducialLayoutSwitchBug1914Logic
#

class FiducialLayoutSwitchBug1914Logic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    ScriptedLoadableModuleLogic.__init__(self)
    # test difference in display location and then in RAS if this is too fine
    self.maximumDisplayDifference = 1.0
    # for future testing: take into account the volume voxel size
    self.maximumRASDifference = 1.0;

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

    self.delayDisplay('Running the algorithm')

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
    mrHeadVolume = SampleData.downloadSample("MRHead")

    # Place a fiducial on the red slice
    markupsLogic = slicer.modules.markups.logic()
    eye = [33.4975, 79.4042, -10.2143]
    fidIndex = markupsLogic.AddFiducial(eye[0], eye[1], eye[2])
    fidID = markupsLogic.GetActiveListID()
    fidNode = slicer.mrmlScene.GetNodeByID(fidID)
    self.delayDisplay("Placed a fiducial at %g, %g, %g" % (eye[0], eye[1], eye[2]))

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
    if helper is not None:
     seedWidget = helper.GetWidget(fidNode)
     seedRepresentation = seedWidget.GetSeedRepresentation()
     handleRep = seedRepresentation.GetHandleRepresentation(fidIndex)
     startingSeedDisplayCoords = handleRep.GetDisplayPosition()
     print('Starting seed display coords = %d, %d, %d' % (startingSeedDisplayCoords[0], startingSeedDisplayCoords[1], startingSeedDisplayCoords[2]))
    self.takeScreenshot('FiducialLayoutSwitchBug1914-StartingPosition','Fiducial starting position',slicer.qMRMLScreenShotDialog.Red)

    # Switch to red slice only
    lm.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpRedSliceView)
    self.delayDisplay("Red Slice only")

    # Switch to conventional layout
    print('Calling set layout back to conventional')
    lm.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
    print('Done calling set layout back to conventional')
    self.delayDisplay("Conventional layout")

    # Get the current seed widget seed location
    endingSeedDisplayCoords = [0.0, 0.0, 0.0]
    helper = self.getFiducialSliceDisplayableManagerHelper('Red')
    if helper is not None:
     seedWidget = helper.GetWidget(fidNode)
     seedRepresentation = seedWidget.GetSeedRepresentation()
     handleRep = seedRepresentation.GetHandleRepresentation(fidIndex)
     endingSeedDisplayCoords = handleRep.GetDisplayPosition()
     print('Ending seed display coords = %d, %d, %d' % (endingSeedDisplayCoords[0], endingSeedDisplayCoords[1], endingSeedDisplayCoords[2]))
    self.takeScreenshot('FiducialLayoutSwitchBug1914-EndingPosition','Fiducial ending position',slicer.qMRMLScreenShotDialog.Red)

    # Compare to original seed widget location
    diff = math.pow((startingSeedDisplayCoords[0] - endingSeedDisplayCoords[0]),2) + math.pow((startingSeedDisplayCoords[1] - endingSeedDisplayCoords[1]),2) + math.pow((startingSeedDisplayCoords[2] - endingSeedDisplayCoords[2]),2)
    if diff != 0.0:
      diff = math.sqrt(diff)
    self.delayDisplay("Difference between starting and ending seed display coordinates = %g" % diff)

    if diff > self.maximumDisplayDifference:
      # double check against the RAS coordinates of the underlying volume since the display could have changed with a FOV adjustment.
      sliceView = sliceWidget.sliceView()
      volumeRAS = sliceView.convertXYZToRAS(endingSeedDisplayCoords)
      seedRAS = [0,0,0]
      fidNode.GetNthFiducialPosition(0,seedRAS)
      rasDiff = math.pow((seedRAS[0] - volumeRAS[0]),2) + math.pow((seedRAS[1] - volumeRAS[1]),2) + math.pow((seedRAS[2] - volumeRAS[2]),2)
      if rasDiff != 0.0:
        rasDiff = math.sqrt(rasDiff)
      print('Checking the difference between fiducial RAS position',seedRAS,'and volume RAS as derived from the fiducial display position',volumeRAS,': ',rasDiff)
      if rasDiff > self.maximumRASDifference:
        self.delayDisplay("RAS coordinate difference is too large as well!\nExpected < %g but got %g" % (self.maximumRASDifference, rasDiff))
        return False
      else:
        self.delayDisplay("RAS coordinate difference is %g which is < %g, test passes." % (rasDiff, self.maximumRASDifference))

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
      annotationLogic.CreateSnapShot("FiducialLayoutSwitchBug1914-Diff", "Difference between starting and ending fiducial seed positions",slicer.qMRMLScreenShotDialog.Red, screenshotScaleFactor, shotDiff)
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


class FiducialLayoutSwitchBug1914Test(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  """

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
    logging.info("ctest, please don't truncate my output: CTEST_FULL_OUTPUT")
    logic = FiducialLayoutSwitchBug1914Logic()
    returnFlag = logic.run()
    if returnFlag == True:
      self.delayDisplay('Test passed!')
    else:
      self.delayDisplay('Test failed!')
    self.assertTrue(returnFlag)



import logging
import math

import vtk

import slicer
from slicer.ScriptedLoadableModule import *


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
    Test for bug 1914, misplaced control point after switching layouts.
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

    def getPointSliceDisplayableManagerHelper(self, sliceName='Red'):
        sliceWidget = slicer.app.layoutManager().sliceWidget(sliceName)
        sliceView = sliceWidget.sliceView()
        collection = vtk.vtkCollection()
        sliceView.getDisplayableManagers(collection)
        for i in range(collection.GetNumberOfItems()):
            m = collection.GetItemAsObject(i)
            if m.GetClassName() == "vtkMRMLMarkupsFiducialDisplayableManager2D":
                return m.GetHelper()
        return None


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
        self.test_FiducialLayoutSwitchBug1914()

    def test_FiducialLayoutSwitchBug1914(self):
        # test difference in display location and then in RAS if this is too fine
        maximumDisplayDifference = 1.0
        # for future testing: take into account the volume voxel size
        maximumRASDifference = 1.0

        enableScreenshots = 0
        screenshotScaleFactor = 1

        logic = FiducialLayoutSwitchBug1914Logic()
        logging.info("ctest, please don't truncate my output: CTEST_FULL_OUTPUT")

        self.delayDisplay('Running the algorithm')
        # Start in conventional layout
        lm = slicer.app.layoutManager()
        lm.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
        # without this delayed display, when running from the cmd line Slicer starts
        # up in a different layout and the seed won't get rendered in the right spot
        self.delayDisplay("Conventional view")

        # Download MRHead from sample data
        import SampleData
        mrHeadVolume = SampleData.downloadSample("MRHead")

        # Place a point on the red slice
        eye = [33.4975, 79.4042, -10.2143]
        markupNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode")
        markupNode.AddControlPoint(eye)
        self.delayDisplay(f"Placed a point at {eye[0]:g}, {eye[1]:g}, {eye[2]:g}")

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
        helper = logic.getPointSliceDisplayableManagerHelper('Red')
        if helper is not None:
            seedWidget = helper.GetWidget(markupNode)
            seedRepresentation = seedWidget.GetSeedRepresentation()
            handleRep = seedRepresentation.GetHandleRepresentation(fidIndex)
            startingSeedDisplayCoords = handleRep.GetDisplayPosition()
            print('Starting seed display coords = %d, %d, %d' % (startingSeedDisplayCoords[0], startingSeedDisplayCoords[1], startingSeedDisplayCoords[2]))
        self.takeScreenshot('FiducialLayoutSwitchBug1914-StartingPosition', 'Point starting position', slicer.qMRMLScreenShotDialog.Red)

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
        helper = logic.getPointSliceDisplayableManagerHelper('Red')
        if helper is not None:
            seedWidget = helper.GetWidget(markupNode)
            seedRepresentation = seedWidget.GetSeedRepresentation()
            handleRep = seedRepresentation.GetHandleRepresentation(fidIndex)
            endingSeedDisplayCoords = handleRep.GetDisplayPosition()
            print('Ending seed display coords = %d, %d, %d' % (endingSeedDisplayCoords[0], endingSeedDisplayCoords[1], endingSeedDisplayCoords[2]))
        self.takeScreenshot('FiducialLayoutSwitchBug1914-EndingPosition', 'Point ending position', slicer.qMRMLScreenShotDialog.Red)

        # Compare to original seed widget location
        diff = math.pow((startingSeedDisplayCoords[0] - endingSeedDisplayCoords[0]), 2) + math.pow((startingSeedDisplayCoords[1] - endingSeedDisplayCoords[1]), 2) + math.pow((startingSeedDisplayCoords[2] - endingSeedDisplayCoords[2]), 2)
        if diff != 0.0:
            diff = math.sqrt(diff)
        self.delayDisplay("Difference between starting and ending seed display coordinates = %g" % diff)

        if diff > maximumDisplayDifference:
            # double check against the RAS coordinates of the underlying volume since the display could have changed with a FOV adjustment.
            sliceView = sliceWidget.sliceView()
            volumeRAS = sliceView.convertXYZToRAS(endingSeedDisplayCoords)
            seedRAS = [0, 0, 0]
            markupNode.GetNthControlPointPosition(0, seedRAS)
            rasDiff = math.pow((seedRAS[0] - volumeRAS[0]), 2) + math.pow((seedRAS[1] - volumeRAS[1]), 2) + math.pow((seedRAS[2] - volumeRAS[2]), 2)
            if rasDiff != 0.0:
                rasDiff = math.sqrt(rasDiff)
            print('Checking the difference between point RAS position', seedRAS,
                  'and volume RAS as derived from the point display position', volumeRAS, ': ', rasDiff)
            if rasDiff > maximumRASDifference:
                raise Exception(f"RAS coordinate difference is too large as well!\nExpected < {maximumRASDifference:g} but got {rasDiff:g}")
            else:
                self.delayDisplay(f"RAS coordinate difference is {rasDiff:g} which is < {maximumRASDifference:g}, test passes.")

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
            annotationLogic.CreateSnapShot("FiducialLayoutSwitchBug1914-Diff", "Difference between starting and ending point positions",
                                           slicer.qMRMLScreenShotDialog.Red, screenshotScaleFactor, shotDiff)
            # calculate the image difference
            imageStats = vtk.vtkImageHistogramStatistics()
            imageStats.SetInput(shotDiff)
            imageStats.GenerateHistogramImageOff()
            imageStats.Update()
            meanVal = imageStats.GetMean()
            self.delayDisplay("Mean of image difference = %g" % meanVal)
            if meanVal > 5.0:
                raise Exception("Image difference is too great!\nExpected <= 5.0, but got %g" % (meanVal))

        self.delayDisplay('Test passed!')

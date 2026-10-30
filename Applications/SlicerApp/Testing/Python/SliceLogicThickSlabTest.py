import os
import time

import numpy as np
import vtk
from vtk.util import numpy_support

import slicer
from slicer.ScriptedLoadableModule import *


#
# SliceLogicThickSlabTest
#
class SliceLogicThickSlabTest(ScriptedLoadableModule):
    """Uses ScriptedLoadableModule base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "SliceLogicThickSlabTest"
        self.parent.categories = ["Testing.TestCases"]
        self.parent.dependencies = ["SampleData"]
        self.parent.contributors = ["Mauro I. Dominguez"]
        self.parent.helpText = """
      This test checks the thick slab reconstruction performed by vtkMRMLSliceLogic.

      Once the slab of the red slice is as thick as the extent of the displayed volume along
      the normal of the slice, every voxel of the volume already contributes to the
      reconstruction. Making the slab any thicker than that can only add samples that fall
      outside the volume, so the rendered slice must stay the same no matter how much thicker
      the slab is made. This is checked both in the default axial plane and in a tilted one,
      as a tilted plane is resliced by a different code path.
      """
        self.parent.acknowledgementText = """
    This file was originally developed by Mauro I. Dominguez.
    """


#
# SliceLogicThickSlabTestWidget
#
class SliceLogicThickSlabTestWidget(ScriptedLoadableModuleWidget):
    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)


#
# SliceLogicThickSlabTestLogic
#
class SliceLogicThickSlabTestLogic(ScriptedLoadableModuleLogic):
    pass


class SliceLogicThickSlabTestTest(ScriptedLoadableModuleTest):
    # How much thicker than the volume the slab is made. The reconstruction is expected to be
    # unaffected by all of them, so these are just a few representative values.
    thicknessFactors = [1.01, 1.2, 1.5, 2.0, 5.0, 10.0]

    # Largest color component difference, out of 255, still considered to be the same image.
    # Every factor above reconstructs a pixel perfect copy of the base screenshot, so this only
    # leaves room for the jitter that a different renderer might introduce.
    maximumAllowedDifference = 2

    def setUp(self):
        """Do whatever is needed to reset the state - typically a scene clear will be enough."""
        slicer.mrmlScene.Clear(0)
        self.savedLayout = None
        self.savedCornerAnnotationVisibility = None

    def tearDown(self):
        """Restore the view state that the test had to change to be able to compare screenshots."""
        if self.savedCornerAnnotationVisibility is not None:
            self.redSliceView().cornerAnnotation().SetVisibility(self.savedCornerAnnotationVisibility)
            self.savedCornerAnnotationVisibility = None
        if self.savedLayout is not None:
            slicer.app.layoutManager().setLayout(self.savedLayout)
            self.savedLayout = None

    def runTest(self):
        """Run as few or as many tests as needed here."""
        for test in (self.test_ThickSlabBeyondVolumeExtentInAxialPlane,
                     self.test_ThickSlabBeyondVolumeExtentInTiltedPlane):
            self.setUp()
            try:
                test()
            finally:
                self.tearDown()
        self.delayDisplay("Test completed.")

    def test_ThickSlabBeyondVolumeExtentInAxialPlane(self):
        """Check the slab in the default plane of the red slice."""
        self.checkThickSlabBeyondVolumeExtent("axial", tiltAnglesDegrees=None)

    def test_ThickSlabBeyondVolumeExtentInTiltedPlane(self):
        """Check the slab in a plane that is not aligned with any of the volume axes.

        A tilted plane is worth checking on its own because vtkMRMLSliceLayerLogic only snaps the
        reslice transform to a permute matrix when the plane is axis aligned, so the slab of a
        tilted plane is reconstructed by a different vtkImageReslice code path.
        """
        self.checkThickSlabBeyondVolumeExtent("tilted", tiltAnglesDegrees=(30.0, 20.0))

    def checkThickSlabBeyondVolumeExtent(self, planeDescription, tiltAnglesDegrees):
        """Growing the slab past the extent of the volume must not change the reconstruction."""
        self.delayDisplay(f"Starting the {planeDescription} plane check")

        # Download the "CBCT-MR Head" sample data set and show its CBCT volume in the slice views.
        import SampleData

        volumeNodes = SampleData.downloadSamples("CBCTMRHead")
        self.assertEqual(len(volumeNodes), 2)
        volumeNode = next(node for node in volumeNodes if node.GetName().startswith("DZ-CBCT"))
        slicer.util.setSliceViewerLayers(background=volumeNode, foreground=None, label=None)

        layoutManager = slicer.app.layoutManager()
        self.savedLayout = layoutManager.layout
        layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpRedSliceView)

        sliceWidget = layoutManager.sliceWidget("Red")
        self.assertIsNotNone(sliceWidget)
        sliceLogic = sliceWidget.sliceLogic()
        sliceNode = sliceWidget.mrmlSliceNode()

        sliceNode.SetOrientationToAxial()
        if tiltAnglesDegrees:
            self.tiltSlice(sliceNode, *tiltAnglesDegrees)
            self.assertEqual(sliceNode.GetOrientation(), "Reformat")
        sliceLogic.FitSliceToBackground()

        # The slab is reconstructed along the normal of the slice, so it is the extent of the
        # volume along that normal that the slab has to cover, whatever the orientation is.
        rasBounds = [0.0] * 6
        volumeNode.GetRASBounds(rasBounds)
        extentAlongSliceNormal = self.volumeExtentAlongSliceNormal(rasBounds, sliceNode)
        self.assertGreater(extentAlongSliceNormal, 0.0)

        # The slab is centered on the slice, so move the slice onto the center of the volume.
        # Otherwise a slab as thick as the volume would still stick out on one side only.
        volumeCenter = [0.5 * (rasBounds[2 * i] + rasBounds[2 * i + 1]) for i in range(3)]
        sliceNode.JumpSliceByOffsetting(*volumeCenter)

        self.delayDisplay(f"Enabling thick slab reconstruction on the {planeDescription} red slice")
        sliceNode.SetSlabReconstructionEnabled(True)

        # The DataProbe slice view annotations report the slab thickness in the corner of the
        # view, which would make any two screenshots taken with different thicknesses differ.
        # Hide them so that only the reconstructed image is compared.
        sliceView = self.redSliceView()
        self.savedCornerAnnotationVisibility = sliceView.cornerAnnotation().GetVisibility()
        sliceView.cornerAnnotation().SetVisibility(False)

        self.waitForRedSliceViewSizeToSettle()

        self.delayDisplay(f"Setting the slab thickness to the volume extent ({extentAlongSliceNormal:.3f} mm)")
        sliceNode.SetSlabReconstructionThickness(extentAlongSliceNormal)
        baseScreenshot = self.captureRedSliceView()

        # A slab thicker than the volume must reconstruct the very same image, whatever the excess is.
        for thicknessFactor in self.thicknessFactors:
            thickness = thicknessFactor * extentAlongSliceNormal
            self.delayDisplay(f"Growing the slab thickness to {thicknessFactor:g} times the volume extent ({thickness:.3f} mm)")
            sliceNode.SetSlabReconstructionThickness(thickness)
            self.assertScreenshotsSimilar(baseScreenshot, self.captureRedSliceView(), planeDescription, thicknessFactor)

        self.delayDisplay(f"The {planeDescription} plane check passed!")

    #
    # Helpers
    #
    @staticmethod
    def tiltSlice(sliceNode, aroundFirstAxisDegrees, aroundSecondAxisDegrees):
        """Rotate the plane of the slice out of its current orientation."""
        sliceToRAS = vtk.vtkTransform()
        sliceToRAS.SetMatrix(sliceNode.GetSliceToRAS())
        sliceToRAS.RotateX(aroundFirstAxisDegrees)
        sliceToRAS.RotateY(aroundSecondAxisDegrees)
        sliceNode.GetSliceToRAS().DeepCopy(sliceToRAS.GetMatrix())
        sliceNode.UpdateMatrices()

    @staticmethod
    def volumeExtentAlongSliceNormal(rasBounds, sliceNode):
        """Return how much of the normal of the slice the bounding box of the volume spans.

        This is the width of the bounding box measured along the normal, which is the superior
        inferior extent of the volume for an axial slice.
        """
        sliceNormal = [sliceNode.GetSliceToRAS().GetElement(i, 2) for i in range(3)]
        vtk.vtkMath.Normalize(sliceNormal)
        return sum(abs((rasBounds[2 * i + 1] - rasBounds[2 * i]) * sliceNormal[i]) for i in range(3))

    @staticmethod
    def redSliceView():
        return slicer.app.layoutManager().sliceWidget("Red").sliceView()

    def waitForRedSliceViewSizeToSettle(self, stableChecksCount=5, timeoutSeconds=30.0):
        """Wait until the main window stops resizing the red slice view.

        The screenshots can only be compared pixel by pixel if the view has the same size
        when all of them are taken.
        """
        sliceView = self.redSliceView()
        previousSize = None
        stableChecks = 0
        deadline = time.time() + timeoutSeconds
        while time.time() < deadline:
            slicer.app.processEvents()
            currentSize = (sliceView.width, sliceView.height)
            stableChecks = stableChecks + 1 if currentSize == previousSize else 0
            if stableChecks >= stableChecksCount:
                return
            previousSize = currentSize
            time.sleep(0.1)
        raise AssertionError(f"The size of the red slice view did not settle in {timeoutSeconds} seconds")

    def captureRedSliceView(self):
        """Return a screenshot of the red slice view as a vtkImageData."""
        sliceView = self.redSliceView()
        sliceView.forceRender()
        slicer.app.processEvents()

        windowToImageFilter = vtk.vtkWindowToImageFilter()
        windowToImageFilter.SetInput(sliceView.renderWindow())
        windowToImageFilter.Update()

        # The filter reuses its output, so keep a copy of it.
        screenshot = vtk.vtkImageData()
        screenshot.DeepCopy(windowToImageFilter.GetOutput())

        dimensions = screenshot.GetDimensions()
        self.assertGreater(dimensions[0], 1)
        self.assertGreater(dimensions[1], 1)
        return screenshot

    def assertScreenshotsSimilar(self, baseScreenshot, screenshot, planeDescription, thicknessFactor):
        """Fail if the two screenshots differ, writing both of them out for inspection."""
        self.assertEqual(baseScreenshot.GetDimensions(), screenshot.GetDimensions())

        baseArray = self.screenshotAsArray(baseScreenshot).astype(np.int32)
        array = self.screenshotAsArray(screenshot).astype(np.int32)
        difference = np.abs(baseArray - array)
        if difference.max() <= self.maximumAllowedDifference:
            return

        outputDirectory = slicer.util.tempDirectory("SliceLogicThickSlabTest")
        self.writeScreenshot(baseScreenshot, os.path.join(outputDirectory, f"{planeDescription}-slab-at-volume-extent.png"))
        self.writeScreenshot(screenshot, os.path.join(outputDirectory, f"{planeDescription}-slab-{thicknessFactor:g}-times-volume-extent.png"))
        self.fail(
            f"The {planeDescription} red slice changed when the slab was made {thicknessFactor:g} times thicker than the volume: "
            f"{np.count_nonzero(difference.any(axis=1))} of {difference.shape[0]} pixels differ, "
            f"largest component difference is {difference.max()} but at most {self.maximumAllowedDifference} is allowed. "
            f"Both screenshots were written to {outputDirectory}.")

    @staticmethod
    def screenshotAsArray(screenshot):
        scalars = screenshot.GetPointData().GetScalars()
        return numpy_support.vtk_to_numpy(scalars).reshape(-1, scalars.GetNumberOfComponents())

    @staticmethod
    def writeScreenshot(screenshot, filePath):
        writer = vtk.vtkPNGWriter()
        writer.SetFileName(filePath)
        writer.SetInputData(screenshot)
        writer.Write()

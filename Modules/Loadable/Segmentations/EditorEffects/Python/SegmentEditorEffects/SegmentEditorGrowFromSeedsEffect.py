import logging
import os
import time
import qt
import vtk

import slicer
from slicer.i18n import tr as _

from SegmentEditorEffects import *

try:
    import warp as wp
    import numpy
    warpAvailable = True

    @wp.func
    def setNeighbors(
        volume: wp.array3d(dtype=wp.int16),
        slice: wp.int32,
        row: wp.int32,
        column: wp.int32,
        value: wp.int16,
        shape: wp.vec3i,
    ):
        """Sets the 3x3x3 neighborhood around a voxel to a given value."""
        neighborhoodSize = 1
        for sliceOff in range(-neighborhoodSize, neighborhoodSize + 1):
            for rowOff in range(-neighborhoodSize, neighborhoodSize + 1):
                for colOff in range(-neighborhoodSize, neighborhoodSize + 1):
                    s = slice + sliceOff
                    r = row + rowOff
                    c = column + colOff

                    if s >= 0 and s < shape[0] and r >= 0 and r < shape[1] and c >= 0 and c < shape[2]:
                        volume[s, r, c] = value

    @wp.kernel
    def initialCandidatesKernel(
        labels: wp.array3d(dtype=wp.int16),
        candidates: wp.array3d(dtype=wp.int16),
        shape: wp.vec3i,
    ):
        """Initializes the candidate voxels for the GrowCut algorithm."""
        slice, row, column = wp.tid()

        if labels[slice, row, column] != 0:
            setNeighbors(candidates, slice, row, column, wp.int16(1), shape)

    @wp.kernel
    def growCutKernel(
        volume: wp.array3d(dtype=wp.int16),
        label: wp.array3d(dtype=wp.int16),
        theta: wp.array3d(dtype=wp.float32),
        labelNext: wp.array3d(dtype=wp.int16),
        thetaNext: wp.array3d(dtype=wp.float32),
        candidates: wp.array3d(dtype=wp.int16),
        candidatesNext: wp.array3d(dtype=wp.int16),
        volumeMax: wp.float32,
        changedFlag: wp.array(dtype=wp.int32),
        shape: wp.vec3i,
    ):
        """Performs one iteration of the GrowCut algorithm."""
        slice, row, column = wp.tid()

        # Copy current state to next state
        labelNext[slice, row, column] = label[slice, row, column]
        thetaNow = theta[slice, row, column]
        thetaNext[slice, row, column] = thetaNow

        if candidates[slice, row, column] == 0:
            return

        sample = wp.float(volume[slice, row, column])

        neighborhoodSize = 1
        for sliceOff in range(-neighborhoodSize, neighborhoodSize + 1):
            for rowOff in range(-neighborhoodSize, neighborhoodSize + 1):
                for colOff in range(-neighborhoodSize, neighborhoodSize + 1):
                    s = slice + sliceOff
                    r = row + rowOff
                    c = column + colOff

                    if s >= 0 and s < shape[0] and r >= 0 and r < shape[1] and c >= 0 and c < shape[2]:
                        otherLabel = label[s, r, c]
                        if otherLabel != 0:
                            otherSample = wp.float(volume[s, r, c])
                            otherTheta = theta[s, r, c]

                            sampleDiff = wp.abs(sample - otherSample)
                            attackStrength = otherTheta * (1.0 - (sampleDiff / volumeMax))

                            if attackStrength > thetaNow:
                                labelNext[slice, row, column] = otherLabel
                                thetaNext[slice, row, column] = attackStrength
                                thetaNow = attackStrength
                                wp.atomic_add(changedFlag, 0, 1)
                                setNeighbors(candidatesNext, slice, row, column, wp.int16(1), shape)


    class GrowCutWarpLogic:
        def __init__(self, backgroundArray, labelArray, device="cpu"):
            wp.init()
            self.device = device
            self.shape = backgroundArray.shape
            self.wpShape = wp.vec3i(self.shape[0], self.shape[1], self.shape[2])

            self.backgroundWp = wp.array(backgroundArray, dtype=wp.int16, device=self.device)
            self.labelWp = wp.array(labelArray, dtype=wp.int16, device=self.device)
            self.labelNextWp = wp.zeros_like(self.labelWp)

            self.volumeMax = float(backgroundArray.max())

            binaryLabels = (labelArray != 0).astype(numpy.float32)
            theta = (2**15) * binaryLabels
            self.thetaWp = wp.array(theta, dtype=wp.float32, device=self.device)
            self.thetaNextWp = wp.zeros_like(self.thetaWp)

            self.candidatesWp = wp.zeros_like(self.labelWp)
            self.candidatesNextWp = wp.zeros_like(self.labelWp)

            self.changedFlagWp = wp.zeros(1, dtype=wp.int32, device=self.device)

            self.candidatesInitialized = False
            self.progressCallback = None

        def _initializeCandidates(self):
            if not self.candidatesInitialized:
                wp.launch(
                    kernel=initialCandidatesKernel,
                    dim=self.shape,
                    inputs=[self.labelWp, self.candidatesWp, self.wpShape],
                    device=self.device,
                )
                self.candidatesInitialized = True

        def run(self):
            """Run the algorithm until convergence."""
            self._initializeCandidates()
            iterationCount = 0
            while True:
                iterationCount += 1
                changedCount = self.step(1)

                if self.progressCallback and (iterationCount % 100 == 0):
                    self.progressCallback(iterationCount, changedCount)

                if changedCount == 0:
                    logging.info(f"Warp Grow-cut converged after {iterationCount} iterations.")
                    # One final progress update on completion
                    if self.progressCallback:
                        self.progressCallback(iterationCount, changedCount)
                    break

            return iterationCount

        def step(self, iterations=1):
            self._initializeCandidates()

            for _ in range(iterations):
                self.changedFlagWp.zero_()
                self.candidatesNextWp.zero_()
                wp.launch(
                    kernel=growCutKernel,
                    dim=self.shape,
                    inputs=[
                        self.backgroundWp, self.labelWp, self.thetaWp,
                        self.labelNextWp, self.thetaNextWp,
                        self.candidatesWp, self.candidatesNextWp,
                        self.volumeMax, self.changedFlagWp, self.wpShape,
                    ],
                    device=self.device,
                )
                wp.copy(self.labelWp, self.labelNextWp)
                wp.copy(self.thetaWp, self.thetaNextWp)
                wp.copy(self.candidatesWp, self.candidatesNextWp)

            # Return the number of pixels that changed
            return self.changedFlagWp.numpy()[0]

        def getResult(self):
            return self.labelWp.numpy()

except ImportError:
    warpAvailable = False

def _arrayFromOrientedImageData(imageData):
    """
    Get numpy array from a vtkOrientedImageData object.
    The vtkOrientedImageData is similar to vtkImageData, but it also contains
    an image to world matrix.
    Return a numpy array with shape (depth, height, width).
    This is a local implementation to support older Slicer versions
    where slicer.util.arrayFromOrientedImageData is not available.
    """
    import vtk.util.numpy_support
    pointData = imageData.GetPointData().GetScalars()
    numpyArray = vtk.util.numpy_support.vtk_to_numpy(pointData)
    dims = imageData.GetDimensions()
    # The numpy array shape is reversed from vtk image dimensions (k, j, i)
    return numpyArray.reshape(dims[2], dims[1], dims[0])

def _updateOrientedImageDataFromArray(imageData, numpyArray):
    """
    Update vtkOrientedImageData from a numpy array.
    This is a local implementation to support older Slicer versions.
    """
    import vtk.util.numpy_support
    if imageData.GetPointData().GetScalars() is None:
        # If the image data is not allocated, allocate it
        imageData.AllocateScalars(vtk.util.numpy_support.get_vtk_array_type(numpyArray.dtype), 1)

    # Get the VTK array from the image data
    vtk_data_array = imageData.GetPointData().GetScalars()
    # Get the numpy array from the VTK array
    numpy_data_array = vtk.util.numpy_support.vtk_to_numpy(vtk_data_array)
    # Reshape and copy data
    numpy_data_array[:] = numpyArray.ravel()
    imageData.Modified()

class SegmentEditorGrowFromSeedsEffect(AbstractScriptedSegmentEditorAutoCompleteEffect):
    """AutoCompleteEffect is an effect that can create a full segmentation
    from a partial segmentation (not all slices are segmented or only
    part of the target structures are painted).
    """

    def __init__(self, scriptedEffect):
        AbstractScriptedSegmentEditorAutoCompleteEffect.__init__(self, scriptedEffect)
        scriptedEffect.name = "Grow from seeds"  # no tr (don't translate it because modules find effects by name)
        scriptedEffect.title = _("Grow from seeds")
        self.minimumNumberOfSegments = 2
        self.minimumNumberOfSegmentsWithEditableArea = 1  # if mask is specified then one input segment is sufficient
        self.clippedMasterImageDataRequired = True  # source volume intensities are used by this effect
        self.clippedMaskImageDataRequired = True  # masking is used
        self.logic = None

    def clone(self):
        import qSlicerSegmentationsEditorEffectsPythonQt as effects

        clonedEffect = effects.qSlicerSegmentEditorScriptedEffect(None)
        clonedEffect.setPythonSource(__file__.replace("\\", "/"))
        return clonedEffect

    def icon(self):
        iconPath = os.path.join(os.path.dirname(__file__), "Resources/Icons/GrowFromSeeds.png")
        if os.path.exists(iconPath):
            return qt.QIcon(iconPath)
        return qt.QIcon()

    def helpText(self):
        return "<html>" + _("""Growing segments to create complete segmentation<br>.
        Location, size, and shape of initial segments and content of source volume are taken into account.
        Final segment boundaries will be placed where source volume brightness changes abruptly. Instructions:<p>
        <ul style="margin: 0">
        <li>Use Paint or other effects to draw seeds in each region that should belong to a separate segment.
        Paint each seed with a different segment. Minimum two segments are required.
        <li>Click <dfn>Initialize</dfn> to compute preview of full segmentation.
        <li>Browse through image slices. If previewed segmentation result is not correct then switch to
        Paint or other effects and add more seeds in the misclassified region. Full segmentation will be
        updated automatically within a few seconds
        <li>Click <dfn>Apply</dfn> to update segmentation with the previewed result.
        </ul><p>
        If segments overlap, segment higher in the segments table will have priority.
        The effect uses <a href="http://interactivemedical.org/imic2014/CameraReadyPapers/Paper%204/IMIC_ID4_FastGrowCut.pdf">fast grow-cut method</a>.
        <p>""")

    def reset(self):
        self.logic = None
        AbstractScriptedSegmentEditorAutoCompleteEffect.reset(self)
        self.updateGUIFromMRML()

    def setupOptionsFrame(self):
        AbstractScriptedSegmentEditorAutoCompleteEffect.setupOptionsFrame(self)

        self.backendComboBox = qt.QComboBox()
        self.backendComboBox.addItem(_("CPU (vtkITK)"), "VTK")

        backendWidget = qt.QWidget()
        backendLayout = qt.QHBoxLayout(backendWidget)
        backendLayout.setContentsMargins(0, 0, 0, 0)
        backendLayout.addWidget(self.backendComboBox)

        if warpAvailable:
            self.backendComboBox.addItem(_("CPU (Warp, slower)"), "WarpCPU")
            gpuOptionIndex = self.backendComboBox.count
            self.backendComboBox.addItem(_("GPU (Warp/CUDA, faster if GPU available)"), "WarpGPU")
            if not wp.is_cuda_available():
                self.backendComboBox.model().item(gpuOptionIndex).setEnabled(False)
        else:
            self.installGpuBackendButton = qt.QPushButton(_("Install GPU backend"))
            self.installGpuBackendButton.toolTip = _("Install 'warp-lang' Python package to enable GPU-accelerated processing.")
            self.installGpuBackendButton.connect("clicked()", self.onInstallGpuBackend)
            backendLayout.addWidget(self.installGpuBackendButton)

        self.scriptedEffect.addLabeledOptionsWidget(_("Backend:"), backendWidget)
        self.backendComboBox.connect("currentIndexChanged(int)", self.updateAlgorithmParameterFromGUI)

        # Object scale slider
        self.seedLocalityFactorSlider = slicer.qMRMLSliderWidget()
        self.seedLocalityFactorSlider.setMRMLScene(slicer.mrmlScene)
        self.seedLocalityFactorSlider.minimum = 0
        self.seedLocalityFactorSlider.maximum = 10
        self.seedLocalityFactorSlider.value = 0.0
        self.seedLocalityFactorSlider.decimals = 1
        self.seedLocalityFactorSlider.singleStep = 0.1
        self.seedLocalityFactorSlider.pageStep = 1.0
        self.seedLocalityFactorSlider.setToolTip(_('Increasing this value makes the effect of seeds more localized,'
                                                   ' thereby reducing leaks, but requires seed regions to be more evenly distributed in the image.'
                                                   ' The value is specified as an additional "intensity level difference" per "unit distance."'))
        self.scriptedEffect.addLabeledOptionsWidget(_("Seed locality:"), self.seedLocalityFactorSlider)
        self.seedLocalityFactorSlider.connect("valueChanged(double)", self.updateAlgorithmParameterFromGUI)

    def onInstallGpuBackend(self):
        with slicer.util.tryWithErrorDisplay(_("Failed to install 'warp-lang' package."), waitCursor=True):
            slicer.util.pip_install("warp-lang[extras]")
            if slicer.util.confirmOkCancelDisplay(
                _("GPU backend installation is complete. Restart Slicer to use this feature."),
                _("Restart required"),
            ):
                slicer.util.restart()

    def setMRMLDefaults(self):
        AbstractScriptedSegmentEditorAutoCompleteEffect.setMRMLDefaults(self)
        settings = qt.QSettings()
        backend = settings.value("SegmentEditor/GrowFromSeeds/Backend", "VTK")
        self.scriptedEffect.setParameterDefault("Backend", backend)
        self.scriptedEffect.setParameterDefault("SeedLocalityFactor", 0.0)

    def updateGUIFromMRML(self):
        AbstractScriptedSegmentEditorAutoCompleteEffect.updateGUIFromMRML(self)

        seedLocalityFactor = self.scriptedEffect.doubleParameter("SeedLocalityFactor")
        # TODO: next line seems to be a bug, as it re-reads the parameter as a string
        if not self.scriptedEffect.parameterDefined("SeedLocalityFactor"):
            seedLocalityFactor = 0.0
        else:
            # legacy case, parameter was stored as string
            try:
                seedLocalityFactor = float(seedLocalityFactor)
            except ValueError:
                seedLocalityFactor = 0.0
        wasBlocked = self.seedLocalityFactorSlider.blockSignals(True)
        self.seedLocalityFactorSlider.value = abs(seedLocalityFactor)
        self.seedLocalityFactorSlider.blockSignals(wasBlocked)

        backend = self.scriptedEffect.parameter("Backend") if self.scriptedEffect.parameterDefined("Backend") else "VTK"
        wasBlocked = self.backendComboBox.blockSignals(True)
        self.backendComboBox.setCurrentIndex(self.backendComboBox.findData(backend))
        self.backendComboBox.blockSignals(wasBlocked)

        # Seed locality is only used for the VTK backend
        self.seedLocalityFactorSlider.enabled = (backend == "VTK")

    def updateMRMLFromGUI(self):
        AbstractScriptedSegmentEditorAutoCompleteEffect.updateMRMLFromGUI(self)
        backend = self.backendComboBox.currentData
        self.scriptedEffect.setParameter("Backend", backend)
        settings = qt.QSettings()
        settings.setValue("SegmentEditor/GrowFromSeeds/Backend", backend)
        self.scriptedEffect.setParameter("SeedLocalityFactor", self.seedLocalityFactorSlider.value)

    def updateAlgorithmParameterFromGUI(self):
        self.updateMRMLFromGUI()

        # Changing a parameter requires re-initialization
        self.logic = None

        # Trigger preview update
        if self.getPreviewNode():
            self.delayedAutoUpdateTimer.start()

    def onWarpProgress(self, iteration, changedCount):
        slicer.util.showStatusMessage(f"Warp Grow-cut iteration {iteration}: {changedCount} pixels changed", 100)
        slicer.app.processEvents(qt.QEventLoop.ExcludeUserInputEvents)

    def computePreviewLabelmap(self, mergedImage, outputLabelmap):
        backend = self.scriptedEffect.parameter("Backend")

        if backend.startswith("Warp") and warpAvailable:
            self.computePreviewLabelmapWarp(mergedImage, outputLabelmap)
        else:
            self.computePreviewLabelmapVtk(mergedImage, outputLabelmap)

    def computePreviewLabelmapWarp(self, mergedImage, outputLabelmap):
        backend = self.scriptedEffect.parameter("Backend")
        if not self.logic:
            intensityArray = _arrayFromOrientedImageData(self.clippedMasterImageData)
            labelArray = _arrayFromOrientedImageData(mergedImage)
            if backend == "WarpGPU":
                device = "cuda" if wp.is_cuda_available() else "cpu"
                if not wp.is_cuda_available():
                    logging.warning("Warp GPU backend selected, but CUDA is not available. Falling back to CPU.")
            else:  # WarpCPU
                device = "cpu"
            self.logic = GrowCutWarpLogic(intensityArray, labelArray, device=device)
            self.logic.progress_callback = self.onWarpProgress

        startTime = time.time()
        self.logic.run()

        logging.info("Warp Grow-cut on volume of {}x{}x{} voxels was completed in {:3.1f} seconds.".format(
            *self.clippedMasterImageData.GetDimensions(), time.time() - startTime))

        # Ensure the output labelmap has the correct geometry and is allocated
        outputLabelmap.DeepCopy(mergedImage)

        resultArray = self.logic.getResult()
        _updateOrientedImageDataFromArray(outputLabelmap, resultArray)

        imageToWorld = vtk.vtkMatrix4x4()
        mergedImage.GetImageToWorldMatrix(imageToWorld)
        outputLabelmap.SetImageToWorldMatrix(imageToWorld)

    def computePreviewLabelmapVtk(self, mergedImage, outputLabelmap):
        import vtkITK

        if not self.logic:
            self.logic = vtkITK.vtkITKGrowCut()
            self.logic.SetIntensityVolume(self.clippedMasterImageData)
            self.logic.SetMaskVolume(self.clippedMaskImageData)
            maskExtent = self.clippedMaskImageData.GetExtent() if self.clippedMaskImageData else None
            if (maskExtent is not None and maskExtent[0] <= maskExtent[1] and maskExtent[2] <= maskExtent[3] and maskExtent[4] <= maskExtent[5]):
                self.extentGrowthRatio = 0.50
            else:
                self.extentGrowthRatio = 0.20

        seedLocalityFactor = self.scriptedEffect.doubleParameter("SeedLocalityFactor")
        self.logic.SetDistancePenalty(seedLocalityFactor)
        self.logic.SetSeedLabelVolume(mergedImage)
        startTime = time.time()
        self.logic.Update()
        logging.info("Grow-cut operation on volume of {}x{}x{} voxels was completed in {:3.1f} seconds.".format(
            self.clippedMasterImageData.GetDimensions()[0],
            self.clippedMasterImageData.GetDimensions()[1],
            self.clippedMasterImageData.GetDimensions()[2],
            time.time() - startTime))

        outputLabelmap.DeepCopy(self.logic.GetOutput())
        imageToWorld = vtk.vtkMatrix4x4()
        mergedImage.GetImageToWorldMatrix(imageToWorld)
        outputLabelmap.SetImageToWorldMatrix(imageToWorld)

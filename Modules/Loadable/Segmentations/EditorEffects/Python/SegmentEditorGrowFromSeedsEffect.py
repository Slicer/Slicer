import logging
import os
import time

import qt
import vtk

import slicer

from SegmentEditorEffects import *


class SegmentEditorGrowFromSeedsEffect(AbstractScriptedSegmentEditorAutoCompleteEffect):
    """ AutoCompleteEffect is an effect that can create a full segmentation
        from a partial segmentation (not all slices are segmented or only
        part of the target structures are painted).
    """

    def __init__(self, scriptedEffect):
        AbstractScriptedSegmentEditorAutoCompleteEffect.__init__(self, scriptedEffect)
        scriptedEffect.name = 'Grow from seeds'
        self.minimumNumberOfSegments = 2
        self.clippedMasterImageDataRequired = True  # source volume intensities are used by this effect
        self.clippedMaskImageDataRequired = True  # masking is used
        self.growCutFilter = None

    def clone(self):
        import qSlicerSegmentationsEditorEffectsPythonQt as effects
        clonedEffect = effects.qSlicerSegmentEditorScriptedEffect(None)
        clonedEffect.setPythonSource(__file__.replace('\\', '/'))
        return clonedEffect

    def icon(self):
        iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/GrowFromSeeds.png')
        if os.path.exists(iconPath):
            return qt.QIcon(iconPath)
        return qt.QIcon()

    def helpText(self):
        return """<html>Growing segments to create complete segmentation<br>.
Location, size, and shape of initial segments and content of source volume are taken into account.
Final segment boundaries will be placed where source volume brightness changes abruptly. Instructions:<p>
<ul style="margin: 0">
<li>Use Paint or other offects to draw seeds in each region that should belong to a separate segment.
Paint each seed with a different segment. Minimum two segments are required.</li>
<li>Click <dfn>Initialize</dfn> to compute preview of full segmentation.</li>
<li>Browse through image slices. If previewed segmentation result is not correct then switch to
Paint or other effects and add more seeds in the misclassified region. Full segmentation will be
updated automatically within a few seconds</li>
<li>Click <dfn>Apply</dfn> to update segmentation with the previewed result.</li>
</ul><p>
If segments overlap, segment higher in the segments table will have priority.
The effect uses <a href="http://interactivemedical.org/imic2014/CameraReadyPapers/Paper%204/IMIC_ID4_FastGrowCut.pdf">fast grow-cut method</a>.
<p></html>"""

    def reset(self):
        self.growCutFilter = None
        AbstractScriptedSegmentEditorAutoCompleteEffect.reset(self)
        self.updateGUIFromMRML()

    def setupOptionsFrame(self):
        AbstractScriptedSegmentEditorAutoCompleteEffect.setupOptionsFrame(self)

        # Object scale slider
        self.seedLocalityFactorSlider = slicer.qMRMLSliderWidget()
        self.seedLocalityFactorSlider.setMRMLScene(slicer.mrmlScene)
        self.seedLocalityFactorSlider.minimum = 0
        self.seedLocalityFactorSlider.maximum = 10
        self.seedLocalityFactorSlider.value = 0.0
        self.seedLocalityFactorSlider.decimals = 1
        self.seedLocalityFactorSlider.singleStep = 0.1
        self.seedLocalityFactorSlider.pageStep = 1.0
        self.seedLocalityFactorSlider.setToolTip('Increasing this value makes the effect of seeds more localized,'
                                                 ' thereby reducing leaks, but requires seed regions to be more evenly distributed in the image.'
                                                 ' The value is specified as an additional "intensity level difference" per "unit distance."')
        self.scriptedEffect.addLabeledOptionsWidget("Seed locality:", self.seedLocalityFactorSlider)
        self.seedLocalityFactorSlider.connect('valueChanged(double)', self.updateAlgorithmParameterFromGUI)

    def setMRMLDefaults(self):
        AbstractScriptedSegmentEditorAutoCompleteEffect.setMRMLDefaults(self)
        self.scriptedEffect.setParameterDefault("SeedLocalityFactor", 0.0)

    def updateGUIFromMRML(self):
        AbstractScriptedSegmentEditorAutoCompleteEffect.updateGUIFromMRML(self)
        if self.scriptedEffect.parameterDefined("SeedLocalityFactor"):
            seedLocalityFactor = self.scriptedEffect.doubleParameter("SeedLocalityFactor")
        else:
            seedLocalityFactor = 0.0
        wasBlocked = self.seedLocalityFactorSlider.blockSignals(True)
        self.seedLocalityFactorSlider.value = abs(seedLocalityFactor)
        self.seedLocalityFactorSlider.blockSignals(wasBlocked)

    def updateMRMLFromGUI(self):
        AbstractScriptedSegmentEditorAutoCompleteEffect.updateMRMLFromGUI(self)
        self.scriptedEffect.setParameter("SeedLocalityFactor", self.seedLocalityFactorSlider.value)

    def updateAlgorithmParameterFromGUI(self):
        self.updateMRMLFromGUI()

        # Trigger preview update
        if self.getPreviewNode():
            self.delayedAutoUpdateTimer.start()

    def computePreviewLabelmap(self, mergedImage, outputLabelmap):
        import vtkSlicerSegmentationsModuleLogicPython as vtkSlicerSegmentationsModuleLogic

        if not self.growCutFilter:
            self.growCutFilter = vtkSlicerSegmentationsModuleLogic.vtkImageGrowCutSegment()
            self.growCutFilter.SetIntensityVolume(self.clippedMasterImageData)
            self.growCutFilter.SetMaskVolume(self.clippedMaskImageData)
            maskExtent = self.clippedMaskImageData.GetExtent() if self.clippedMaskImageData else None
            if maskExtent is not None and maskExtent[0] <= maskExtent[1] and maskExtent[2] <= maskExtent[3] and maskExtent[4] <= maskExtent[5]:
                # Mask is used.
                # Grow the extent more, as background segment does not surround region of interest.
                self.extentGrowthRatio = 0.50
            else:
                # No masking is used.
                # Background segment is expected to surround region of interest, so narrower margin is enough.
                self.extentGrowthRatio = 0.20

        if self.scriptedEffect.parameterDefined("SeedLocalityFactor"):
            seedLocalityFactor = self.scriptedEffect.doubleParameter("SeedLocalityFactor")
        else:
            seedLocalityFactor = 0.0
        self.growCutFilter.SetDistancePenalty(seedLocalityFactor)
        self.growCutFilter.SetSeedLabelVolume(mergedImage)
        startTime = time.time()
        self.growCutFilter.Update()
        logging.info('Grow-cut operation on volume of {}x{}x{} voxels was completed in {:3.1f} seconds.'.format(
            self.clippedMasterImageData.GetDimensions()[0],
            self.clippedMasterImageData.GetDimensions()[1],
            self.clippedMasterImageData.GetDimensions()[2],
            time.time() - startTime))

        outputLabelmap.DeepCopy(self.growCutFilter.GetOutput())
        imageToWorld = vtk.vtkMatrix4x4()
        mergedImage.GetImageToWorldMatrix(imageToWorld)
        outputLabelmap.SetImageToWorldMatrix(imageToWorld)

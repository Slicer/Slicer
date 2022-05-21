import os

import qt
import vtk

from SegmentEditorEffects import *


class SegmentEditorFillBetweenSlicesEffect(AbstractScriptedSegmentEditorAutoCompleteEffect):
    """ AutoCompleteEffect is an effect that can create a full segmentation
        from a partial segmentation (not all slices are segmented or only
        part of the target structures are painted).
    """

    def __init__(self, scriptedEffect):
        AbstractScriptedSegmentEditorAutoCompleteEffect.__init__(self, scriptedEffect)
        scriptedEffect.name = 'Fill between slices'

    def clone(self):
        import qSlicerSegmentationsEditorEffectsPythonQt as effects
        clonedEffect = effects.qSlicerSegmentEditorScriptedEffect(None)
        clonedEffect.setPythonSource(__file__.replace('\\', '/'))
        return clonedEffect

    def icon(self):
        iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/FillBetweenSlices.png')
        if os.path.exists(iconPath):
            return qt.QIcon(iconPath)
        return qt.QIcon()

    def helpText(self):
        return """<html>Interpolate segmentation between slices<br>. Instructions:
<p><ul>
<li>Create complete segmentation on selected slices using any editor effect.
Segmentation will only expanded if a slice is segmented but none of the direct neighbors are segmented, therefore
do not use sphere brush with Paint effect and always leave at least one empty slice between segmented slices.</li>
<li>All visible segments will be interpolated, not just the selected segment.</li>
<li>The complete segmentation will be created by interpolating segmentations in empty slices.</li>
</ul><p>
Masking settings are ignored. If segments overlap, segment higher in the segments table will have priority.
The effect uses  <a href="https://insight-journal.org/browse/publication/977">morphological contour interpolation method</a>.
<p></html>"""

    def computePreviewLabelmap(self, mergedImage, outputLabelmap):
        import vtkITK
        interpolator = vtkITK.vtkITKMorphologicalContourInterpolator()
        interpolator.SetInputData(mergedImage)
        interpolator.Update()
        outputLabelmap.DeepCopy(interpolator.GetOutput())
        imageToWorld = vtk.vtkMatrix4x4()
        mergedImage.GetImageToWorldMatrix(imageToWorld)
        outputLabelmap.SetImageToWorldMatrix(imageToWorld)

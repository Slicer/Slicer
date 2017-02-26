import os
import vtk, qt, ctk, slicer
import logging
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
    clonedEffect.setPythonSource(__file__.replace('\\','/'))
    return clonedEffect

  def icon(self):
    iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/AutoComplete.png')
    if os.path.exists(iconPath):
      return qt.QIcon(iconPath)
    return qt.QIcon()

  def helpText(self):
    return """Create complete segmentation on selected slices using any editor effect. \
The complete segmentation will be created by interpolating segmentations on slices that were skipped. \
Masking settings are bypassed. If segments overlap, segment higher in the segments table will have priority. \
See http://insight-journal.org/browse/publication/977 for more details."""

  def computePreviewLabelmap(self, mergedImage, outputLabelmap):
    import vtkITK
    interpolator = vtkITK.vtkITKMorphologicalContourInterpolator()
    interpolator.SetInputData(mergedImage)
    interpolator.Update()
    outputLabelmap.DeepCopy(interpolator.GetOutput())

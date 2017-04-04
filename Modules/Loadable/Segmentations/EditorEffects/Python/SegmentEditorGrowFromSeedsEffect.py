import os
import vtk, qt, ctk, slicer
import logging
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
    self.clippedMasterImageDataRequired = True # master volume intensities are used by this effect
    self.growCutFilter = None

  def clone(self):
    import qSlicerSegmentationsEditorEffectsPythonQt as effects
    clonedEffect = effects.qSlicerSegmentEditorScriptedEffect(None)
    clonedEffect.setPythonSource(__file__.replace('\\','/'))
    return clonedEffect

  def icon(self):
    iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/GrowFromSeeds.png')
    if os.path.exists(iconPath):
      return qt.QIcon(iconPath)
    return qt.QIcon()

  def helpText(self):
    return """<html>Growing segments to create complete segmentation<br>.
Location, size, and shape of initial segments and content of master volume are taken into account.
Final segment boundaries will be placed where master volume brightness changes abruptly. Instructions:<p>
<ul style="margin: 0">
<li>Use Paint or other offects to draw seeds in each region that should belong to a separate segment.
Paint each seed with a different segment. Minimum two segments are required.</li>
<li>Click <dfn>Initialize</dfn> to compute preview of full segmentation.</li>
<li>Browse through image slices. If previewed segmentation result is not correct then switch to
Paint or other effects and add more seeds in the misclassified region. Full segmentation will be
updated automatically within a few seconds</li>
<li>Click <dfn>Apply</dfn> to update segmentation with the previewed result.</li>
</ul><p>
Masking settings are bypassed. If segments overlap, segment higher in the segments table will have priority.
The effect uses <a href="https://www.spl.harvard.edu/publications/item/view/2761">fast grow-cut method</a>.
<p></html>"""


  def reset(self):
    self.growCutFilter = None
    AbstractScriptedSegmentEditorAutoCompleteEffect.reset(self)
    self.updateGUIFromMRML()

  def computePreviewLabelmap(self, mergedImage, outputLabelmap):
    import vtkSlicerSegmentationsModuleLogicPython as vtkSlicerSegmentationsModuleLogic

    if not self.growCutFilter:
      self.growCutFilter = vtkSlicerSegmentationsModuleLogic.vtkImageGrowCutSegment()
      self.growCutFilter.SetIntensityVolume(self.clippedMasterImageData)

    self.growCutFilter.SetSeedLabelVolume(mergedImage)
    self.growCutFilter.Update()

    outputLabelmap.DeepCopy( self.growCutFilter.GetOutput() )

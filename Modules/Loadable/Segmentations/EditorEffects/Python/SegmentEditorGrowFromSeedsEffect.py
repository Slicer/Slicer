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
    iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/AutoComplete.png')
    if os.path.exists(iconPath):
      return qt.QIcon(iconPath)
    return qt.QIcon()

  def helpText(self):
    return """Create a complete segmentation by growing from existing segmentation. \
Paint in each region that should belong to a separate segment. Segments will be expanded to create \
a complete segmentation, taking into account the master volume content. Minimum two segments are required. \
Masking settings are bypassed. If segments overlap, segment higher in the segments table will have priority."""

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

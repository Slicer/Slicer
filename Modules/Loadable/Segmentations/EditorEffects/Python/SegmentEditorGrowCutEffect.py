import os
import vtk, qt, ctk, slicer
import logging
from SegmentEditorEffects import *

class SegmentEditorGrowCutEffect(AbstractScriptedSegmentEditorEffect):
  """ GrowCutEffect is an Effect that implements the
      GrowCut segmentation in segment editor
  """

  def __init__(self, scriptedEffect):
    scriptedEffect.name = 'GrowCut'
    # Indicates that effect does not operate on one segment, but the whole segmentation.
    # This means that while this effect is active, no segment can be selected
    scriptedEffect.perSegment = False
    AbstractScriptedSegmentEditorEffect.__init__(self, scriptedEffect)

  def clone(self):
    import qSlicerSegmentationsEditorEffectsPythonQt as effects
    clonedEffect = effects.qSlicerSegmentEditorScriptedEffect(None)
    clonedEffect.setPythonSource(__file__.replace('\\','/'))
    return clonedEffect

  def icon(self):
    iconPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons/GrowCut.png')
    if os.path.exists(iconPath):
      return qt.QIcon(iconPath)
    return qt.QIcon()

  def helpText(self):
    return """Create complete segmentation by expanding existing segments.
Create segments using any editor effect: one segment inside each each region that should belong to a separate segment, then click Apply.
Minimum two segments are required. If segments overlap, segment higher in the segments table will have priority."""

  def setupOptionsFrame(self):
    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.objectName = self.__class__.__name__ + 'Apply'
    self.applyButton.setToolTip("GrowCut selected segment")
    self.scriptedEffect.addOptionsWidget(self.applyButton)

    self.applyButton.connect('clicked()', self.onApply)

  def createCursor(self, widget):
    # Turn off effect-specific cursor for this effect
    return slicer.util.mainWindow().cursor

  def onApply(self):
    # Get master volume image data
    import vtkSegmentationCorePython as vtkSegmentationCore
    masterImageData = self.scriptedEffect.masterVolumeImageData()

    # Check validity of master volume: whether scalar type is supported
    if masterImageData.GetScalarType() != vtk.VTK_SHORT:
      logging.warning("GrowCut is attempted with image type '{0}', which is not directly supported".format(masterImageData.GetScalarTypeAsString()))
      if not slicer.util.confirmOkCancelDisplay("Current image type is '{0}', which is not supported by GrowCut. "
          "The image scalars will be temporarily casted to short.\n\nIf the segmentation result is not satisfacory, "
          "then it may mean the scalar range of the master image is out of the range covered by short."
          .format(masterImageData.GetScalarTypeAsString()), windowTitle='Segment editor'):
        logging.warning('GrowCut is cancelled by the user')
        return

    slicer.util.showStatusMessage("Running GrowCut...", 2000)
    self.scriptedEffect.saveStateForUndo()

    self.growCut()
    slicer.util.showStatusMessage("GrowCut Finished", 2000)

  def growCut(self):
    # Get master volume image data
    import vtkSegmentationCorePython as vtkSegmentationCore
    masterImageData = self.scriptedEffect.masterVolumeImageData()
    # Get segmentation
    segmentationNode = self.scriptedEffect.parameterSetNode().GetSegmentationNode()

    # Cast master image if not short
    if masterImageData.GetScalarType() != vtk.VTK_SHORT:
      imageCast = vtk.vtkImageCast()
      imageCast.SetInputData(masterImageData)
      imageCast.SetOutputScalarTypeToShort()
      imageCast.ClampOverflowOn()
      imageCast.Update()
      masterImageDataShort = vtkSegmentationCore.vtkOrientedImageData()
      masterImageDataShort.DeepCopy(imageCast.GetOutput()) # Copy image data
      masterImageDataShort.CopyDirections(masterImageData) # Copy geometry
      masterImageData = masterImageDataShort

    # Generate merged labelmap as input to GrowCut
    mergedImage = vtkSegmentationCore.vtkOrientedImageData()
    segmentationNode.GenerateMergedLabelmapForAllSegments(mergedImage, vtkSegmentationCore.vtkSegmentation.EXTENT_UNION_OF_SEGMENTS, masterImageData)

    # Make a zero-valued volume for the output
    outputLabelmap = vtkSegmentationCore.vtkOrientedImageData()
    thresh = vtk.vtkImageThreshold()
    thresh.ReplaceInOn()
    thresh.ReplaceOutOn()
    thresh.SetInValue(0)
    thresh.SetOutValue(0)
    thresh.SetOutputScalarType( vtk.VTK_SHORT )
    thresh.SetInputData( mergedImage )
    thresh.SetOutput( outputLabelmap )
    thresh.Update()
    outputLabelmap.DeepCopy( mergedImage ) #TODO: It was thresholded just above, why deep copy now?

    # Perform grow cut
    import vtkITK
    growCutFilter = vtkITK.vtkITKGrowCutSegmentationImageFilter()
    growCutFilter.SetInputData( 0, masterImageData )
    growCutFilter.SetInputData( 1, mergedImage )
    #TODO: This call sets an empty image for the optional "previous segmentation", and
    #      is apparently needed for the first segmentation too. Why?
    growCutFilter.SetInputConnection( 2, thresh.GetOutputPort() )

    #TODO: These are magic numbers inherited from EditorLib/GrowCut.py
    objectSize = 5.
    contrastNoiseRatio = 0.8
    priorStrength = 0.003
    segmented = 2
    conversion = 1000

    spacing = mergedImage.GetSpacing()
    voxelVolume = reduce(lambda x,y: x*y, spacing)
    voxelAmount = objectSize / voxelVolume
    voxelNumber = round(voxelAmount) * conversion

    cubeRoot = 1./3.
    oSize = int(round(pow(voxelNumber,cubeRoot)))

    growCutFilter.SetObjectSize( oSize )
    growCutFilter.SetContrastNoiseRatio( contrastNoiseRatio )
    growCutFilter.SetPriorSegmentConfidence( priorStrength )
    growCutFilter.Update()

    outputLabelmap.DeepCopy( growCutFilter.GetOutput() )

    # Write output segmentation results in segments
    segmentIDs = vtk.vtkStringArray()
    segmentationNode.GetSegmentation().GetSegmentIDs(segmentIDs)
    for index in xrange(segmentIDs.GetNumberOfValues()):
      segmentID = segmentIDs.GetValue(index)
      segment = segmentationNode.GetSegmentation().GetSegment(segmentID)

      # Get label corresponding to segment in merged labelmap (and so GrowCut output)
      colorIndexStr = vtk.mutable("")
      tagFound = segment.GetTag(slicer.vtkMRMLSegmentationDisplayNode.GetColorIndexTag(), colorIndexStr);
      if not tagFound:
        logging.error('Failed to apply GrowCut result on segment ' + segmentID)
        continue
      colorIndex = int(colorIndexStr.get())

      # Get only the label of the current segment from the output image
      thresh = vtk.vtkImageThreshold()
      thresh.ReplaceInOn()
      thresh.ReplaceOutOn()
      thresh.SetInValue(1)
      thresh.SetOutValue(0)
      thresh.ThresholdBetween(colorIndex, colorIndex);
      thresh.SetOutputScalarType(vtk.VTK_UNSIGNED_CHAR)
      thresh.SetInputData(outputLabelmap)
      thresh.Update()

      # Write label to segment
      newSegmentLabelmap = vtkSegmentationCore.vtkOrientedImageData()
      newSegmentLabelmap.ShallowCopy(thresh.GetOutput())
      newSegmentLabelmap.CopyDirections(mergedImage)
      slicer.vtkSlicerSegmentationsModuleLogic.SetBinaryLabelmapToSegment(newSegmentLabelmap, segmentationNode, segmentID, slicer.vtkSlicerSegmentationsModuleLogic.MODE_REPLACE, newSegmentLabelmap.GetExtent())

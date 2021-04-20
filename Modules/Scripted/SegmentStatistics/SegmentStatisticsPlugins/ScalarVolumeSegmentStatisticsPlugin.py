import vtk, slicer
from vtk.util.numpy_support import vtk_to_numpy
from SegmentStatisticsPlugins import SegmentStatisticsPluginBase
from functools import reduce
import numpy as np
import qt


class ScalarVolumeSegmentStatisticsPlugin(SegmentStatisticsPluginBase):
  """Statistical plugin for segmentations with scalar volumes"""

  def __init__(self):
    super(ScalarVolumeSegmentStatisticsPlugin,self).__init__()
    self.name = "Scalar Volume"
    self.keys = ["voxel_count", "volume_mm3", "volume_cm3", "min", "max", "mean", "median", "stdev"]
    self.defaultKeys = self.keys # calculate all measurements by default
    #... developer may add extra options to configure other parameters

  def computeStatistics(self, segmentID):
    import vtkSegmentationCorePython as vtkSegmentationCore
    requestedKeys = self.getRequestedKeys()

    segmentationNode = slicer.mrmlScene.GetNodeByID(self.getParameterNode().GetParameter("Segmentation"))
    grayscaleNode = slicer.mrmlScene.GetNodeByID(self.getParameterNode().GetParameter("ScalarVolume"))

    if len(requestedKeys)==0:
      return {}

    containsLabelmapRepresentation = segmentationNode.GetSegmentation().ContainsRepresentation(
      vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationBinaryLabelmapRepresentationName())
    if not containsLabelmapRepresentation:
      return {}

    if (not grayscaleNode
      or not grayscaleNode.GetImageData()
      or not grayscaleNode.GetImageData().GetPointData()
      or not grayscaleNode.GetImageData().GetPointData().GetScalars()):
      # Input grayscale node does not contain valid image data
      return {}

    voxelsInSegment = self.getVoxelsInSegment(segmentationNode, segmentID, grayscaleNode)

    # If option enabled, compute stats using voxels within the display node thresholds for the volume
    pluginName = self.__class__.__name__
    if self.parameterNode.GetParameter(pluginName+'.ApplyThresholds.enabled') == "True":
      threshold_min = grayscaleNode.GetDisplayNode().GetLowerThreshold()
      threshold_max = grayscaleNode.GetDisplayNode().GetUpperThreshold()
      arrayThresholded = voxelsInSegment[voxelsInSegment >= threshold_min]
      arrayThresholded = arrayThresholded[arrayThresholded <= threshold_max]
    else:
      arrayThresholded = voxelsInSegment

    cubicMMPerVoxel = reduce(lambda x,y: x*y, grayscaleNode.GetSpacing())
    ccPerCubicMM = 0.001

    # create statistics list
    stats = {}
    if "voxel_count" in requestedKeys:
      stats["voxel_count"] = len(voxelsInSegment)
    if "volume_mm3" in requestedKeys:
      stats["volume_mm3"] = len(voxelsInSegment) * cubicMMPerVoxel
    if "volume_cm3" in requestedKeys:
      stats["volume_cm3"] = len(voxelsInSegment) * cubicMMPerVoxel * ccPerCubicMM
    if len(arrayThresholded)>0:
      if "min" in requestedKeys:
        stats["min"] = arrayThresholded.min()
      if "max" in requestedKeys:
        stats["max"] = arrayThresholded.max()
      if "mean" in requestedKeys:
        stats["mean"] = arrayThresholded.mean()
      if "stdev" in requestedKeys:
        stats["stdev"] = np.std(arrayThresholded)
      if "median" in requestedKeys:
        stats["median"] = np.median(arrayThresholded)
    return stats

  def getVoxelsInSegment(self, segmentationNode, segmentID, grayscaleNode):
    import vtkSegmentationCorePython as vtkSegmentationCore
    # Get geometry of grayscale volume node as oriented image data
    # reference geometry in reference node coordinate system
    referenceGeometry_Reference = vtkSegmentationCore.vtkOrientedImageData()
    referenceGeometry_Reference.SetExtent(grayscaleNode.GetImageData().GetExtent())
    ijkToRasMatrix = vtk.vtkMatrix4x4()
    grayscaleNode.GetIJKToRASMatrix(ijkToRasMatrix)
    referenceGeometry_Reference.SetGeometryFromImageToWorldMatrix(ijkToRasMatrix)

    # Get transform between grayscale volume and segmentation
    segmentationToReferenceGeometryTransform = vtk.vtkGeneralTransform()
    slicer.vtkMRMLTransformNode.GetTransformBetweenNodes(segmentationNode.GetParentTransformNode(),
      grayscaleNode.GetParentTransformNode(), segmentationToReferenceGeometryTransform)

    segmentLabelmap = vtkSegmentationCore.vtkOrientedImageData()
    segmentationNode.GetBinaryLabelmapRepresentation(segmentID, segmentLabelmap)
    if (not segmentLabelmap
      or not segmentLabelmap.GetPointData()
      or not segmentLabelmap.GetPointData().GetScalars()):
      # No input label data
      return {}

    segmentLabelmap_Reference = vtkSegmentationCore.vtkOrientedImageData()
    vtkSegmentationCore.vtkOrientedImageDataResample.ResampleOrientedImageToReferenceOrientedImage(
      segmentLabelmap, referenceGeometry_Reference, segmentLabelmap_Reference,
      False, # nearest neighbor interpolation
      False, # no padding
      segmentationToReferenceGeometryTransform)

    # We need to know exactly the value of the segment voxels, apply threshold to make force the selected label value
    labelValue = 1
    backgroundValue = 0
    thresh = vtk.vtkImageThreshold()
    thresh.SetInputData(segmentLabelmap_Reference)
    thresh.ThresholdByLower(0)
    thresh.SetInValue(backgroundValue)
    thresh.SetOutValue(labelValue)
    thresh.SetOutputScalarType(vtk.VTK_UNSIGNED_CHAR)
    thresh.Update()

    #  Use binary labelmap as a stencil
    stencil = vtk.vtkImageToImageStencil()
    stencil.SetInputData(thresh.GetOutput())
    stencil.ThresholdByUpper(labelValue)
    stencil.Update()

    # Apply stencil mask to the volume, everything outside of the stencil is set to -inf
    image_cast = vtk.vtkImageCast()
    image_cast.SetOutputScalarTypeToDouble()
    image_cast.SetInputData(grayscaleNode.GetImageData())
    image_cast.Update()

    reslice = vtk.vtkImageReslice()
    reslice.SetStencilData(stencil.GetOutput())
    reslice.SetBackgroundLevel(float('-inf'))
    reslice.SetInputData(image_cast.GetOutput())
    reslice.Update()

    array = vtk_to_numpy(reslice.GetOutput().GetPointData().GetScalars())
    array = array[array > float('-inf')]

    return array

  def createDefaultOptionsWidget(self):
    super().createDefaultOptionsWidget()
    self.enableThresholdCheckbox = qt.QCheckBox("Apply volume thresholds to statistics")
    self.optionsWidget.layout().insertRow(1, self.enableThresholdCheckbox)
    self.enableThresholdCheckbox.connect('stateChanged(int)', self.updateParameterNodeFromGui)

  def updateParameterNodeFromGui(self):
    super().updateParameterNodeFromGui()
    pluginName = self.__class__.__name__
    self.parameterNode.SetParameter(pluginName+'.ApplyThresholds.enabled', str(self.enableThresholdCheckbox.checked))

  def updateGuiFromParameterNode(self, caller=None, event=None):
    super().updateGuiFromParameterNode(caller=None, event=None)
    pluginName = self.__class__.__name__
    value = self.parameterNode.GetParameter(pluginName+'.ApplyThresholds.enabled')=='True'
    previousState = self.enableThresholdCheckbox.blockSignals(True)
    self.enableThresholdCheckbox.checked = value
    self.enableThresholdCheckbox.blockSignals(previousState)

  def getMeasurementInfo(self, key):
    """Get information (name, description, units, ...) about the measurement for the given key"""

    scalarVolumeNode = slicer.mrmlScene.GetNodeByID(self.getParameterNode().GetParameter("ScalarVolume"))

    scalarVolumeQuantity = scalarVolumeNode.GetVoxelValueQuantity() if scalarVolumeNode else self.createCodedEntry("", "", "")
    scalarVolumeUnits = scalarVolumeNode.GetVoxelValueUnits() if scalarVolumeNode else self.createCodedEntry("", "", "")
    if not scalarVolumeQuantity:
      scalarVolumeQuantity = self.createCodedEntry("", "", "")
    if not scalarVolumeUnits:
      scalarVolumeUnits = self.createCodedEntry("", "", "")

    info = dict()

    # @fedorov could not find any suitable DICOM quantity code for "number of voxels".
    # DCM has "Number of needles" etc., so probably "Number of voxels"
    # should be added too. Need to discuss with @dclunie. For now, a
    # QIICR private scheme placeholder.
    # @moselhy also could not find DICOM quantity code for "median"

    info["voxel_count"] = \
      self.createMeasurementInfo(name="Voxel count", description="Number of voxels", units="voxels",
                                   quantityDicomCode=self.createCodedEntry("nvoxels", "99QIICR", "Number of voxels", True),
                                   unitsDicomCode=self.createCodedEntry("voxels", "UCUM", "voxels", True))

    info["volume_mm3"] = \
      self.createMeasurementInfo(name="Volume mm3", description="Volume in mm3", units="mm3",
                                   quantityDicomCode=self.createCodedEntry("118565006", "SCT", "Volume", True),
                                   unitsDicomCode=self.createCodedEntry("mm3", "UCUM", "cubic millimeter", True))

    info["volume_cm3"] = \
      self.createMeasurementInfo(name="Volume cm3", description="Volume in cm3", units="cm3",
                                   quantityDicomCode=self.createCodedEntry("118565006", "SCT", "Volume", True),
                                   unitsDicomCode=self.createCodedEntry("cm3","UCUM","cubic centimeter", True),
                                   measurementMethodDicomCode=self.createCodedEntry("126030", "DCM",
                                                                             "Sum of segmented voxel volumes", True))

    info["min"] = \
      self.createMeasurementInfo(name="Minimum", description="Minimum scalar value",
                                   units=scalarVolumeUnits.GetCodeMeaning(),
                                   quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                   unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                   derivationDicomCode=self.createCodedEntry("255605001", "SCT", "Minimum", True))

    info["max"] = \
      self.createMeasurementInfo(name="Maximum", description="Maximum scalar value",
                                   units=scalarVolumeUnits.GetCodeMeaning(),
                                   quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                   unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                   derivationDicomCode=self.createCodedEntry("56851009","SCT","Maximum", True))

    info["mean"] = \
      self.createMeasurementInfo(name="Mean", description="Mean scalar value",
                                   units=scalarVolumeUnits.GetCodeMeaning(),
                                   quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                   unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                   derivationDicomCode=self.createCodedEntry("373098007","SCT","Mean", True))

    info["median"] = \
      self.createMeasurementInfo(name="Median", description="Median scalar value",
                                   units=scalarVolumeUnits.GetCodeMeaning(),
                                   quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                   unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                   derivationDicomCode=self.createCodedEntry("median","SCT","Median", True))

    info["stdev"] = \
      self.createMeasurementInfo(name="Standard deviation", description="Standard deviation of scalar values",
                                   units=scalarVolumeUnits.GetCodeMeaning(),
                                   quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                   unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                   derivationDicomCode=self.createCodedEntry('386136009','SCT','Standard Deviation', True))

    return info[key] if key in info else None

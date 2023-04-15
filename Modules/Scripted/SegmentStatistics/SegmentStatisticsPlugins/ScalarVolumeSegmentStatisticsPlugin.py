import vtk, slicer
from slicer.i18n import tr as _
from SegmentStatisticsPlugins import SegmentStatisticsPluginBase
from functools import reduce


class ScalarVolumeSegmentStatisticsPlugin(SegmentStatisticsPluginBase):
    """Statistical plugin for segmentations with scalar volumes"""

    def __init__(self):
        super().__init__()
        self.name = "Scalar Volume"
        self.keys = ["voxel_count", "volume_mm3", "volume_cm3", "min", "max", "mean", "median", "stdev"]
        self.defaultKeys = self.keys  # calculate all measurements by default
        # ... developer may add extra options to configure other parameters

    def computeStatistics(self, segmentID):
        requestedKeys = self.getRequestedKeys()

        segmentationNode = slicer.mrmlScene.GetNodeByID(self.getParameterNode().GetParameter("Segmentation"))
        grayscaleNode = slicer.mrmlScene.GetNodeByID(self.getParameterNode().GetParameter("ScalarVolume"))

        if len(requestedKeys) == 0:
            return {}

        stencil = self.getStencilForVolume(segmentationNode, segmentID, grayscaleNode)
        if not stencil:
            return {}

        cubicMMPerVoxel = reduce(lambda x, y: x * y, grayscaleNode.GetSpacing())
        ccPerCubicMM = 0.001

        stat = vtk.vtkImageAccumulate()
        stat.SetInputData(grayscaleNode.GetImageData())
        stat.SetStencilData(stencil.GetOutput())
        stat.Update()

        medians = vtk.vtkImageHistogramStatistics()
        medians.SetInputData(grayscaleNode.GetImageData())
        medians.SetStencilData(stencil.GetOutput())
        medians.Update()

        # create statistics list
        stats = {}
        if "voxel_count" in requestedKeys:
            stats["voxel_count"] = stat.GetVoxelCount()
        if "volume_mm3" in requestedKeys:
            stats["volume_mm3"] = stat.GetVoxelCount() * cubicMMPerVoxel
        if "volume_cm3" in requestedKeys:
            stats["volume_cm3"] = stat.GetVoxelCount() * cubicMMPerVoxel * ccPerCubicMM
        if stat.GetVoxelCount() > 0:
            if "min" in requestedKeys:
                stats["min"] = stat.GetMin()[0]
            if "max" in requestedKeys:
                stats["max"] = stat.GetMax()[0]
            if "mean" in requestedKeys:
                stats["mean"] = stat.GetMean()[0]
            if "stdev" in requestedKeys:
                stats["stdev"] = stat.GetStandardDeviation()[0]
            if "median" in requestedKeys:
                stats["median"] = medians.GetMedian()
        return stats

    def getStencilForVolume(self, segmentationNode, segmentID, grayscaleNode):
        import vtkSegmentationCorePython as vtkSegmentationCore

        containsLabelmapRepresentation = segmentationNode.GetSegmentation().ContainsRepresentation(
            vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationBinaryLabelmapRepresentationName())
        if not containsLabelmapRepresentation:
            return None

        if (not grayscaleNode
            or not grayscaleNode.GetImageData()
            or not grayscaleNode.GetImageData().GetPointData()
                or not grayscaleNode.GetImageData().GetPointData().GetScalars()):
            # Input grayscale node does not contain valid image data
            return None

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
            return None

        segmentLabelmap_Reference = vtkSegmentationCore.vtkOrientedImageData()
        vtkSegmentationCore.vtkOrientedImageDataResample.ResampleOrientedImageToReferenceOrientedImage(
            segmentLabelmap, referenceGeometry_Reference, segmentLabelmap_Reference,
            False,  # nearest neighbor interpolation
            False,  # no padding
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

        return stencil

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
            self.createMeasurementInfo(name="Voxel count", description=_("Number of voxels"), units="voxels",
                                       quantityDicomCode=self.createCodedEntry("nvoxels", "99QIICR", _("Number of voxels"), True),
                                       unitsDicomCode=self.createCodedEntry("voxels", "UCUM", _("voxels"), True))

        info["volume_mm3"] = \
            self.createMeasurementInfo(name="Volume mm3", description=_("Volume in mm3"), units="mm3",
                                       quantityDicomCode=self.createCodedEntry("118565006", "SCT", _("Volume"), True),
                                       unitsDicomCode=self.createCodedEntry("mm3", "UCUM", _("cubic millimeter"), True))

        info["volume_cm3"] = \
            self.createMeasurementInfo(name="Volume cm3", description=_("Volume in cm3"), units="cm3",
                                       quantityDicomCode=self.createCodedEntry("118565006", "SCT", _("Volume"), True),
                                       unitsDicomCode=self.createCodedEntry("cm3", "UCUM", _("cubic centimeter"), True),
                                       measurementMethodDicomCode=self.createCodedEntry("126030", "DCM",
                                                                                        _("Sum of segmented voxel volumes"), True))

        info["min"] = \
            self.createMeasurementInfo(name="Minimum", description=_("Minimum scalar value"),
                                       units=scalarVolumeUnits.GetCodeMeaning(),
                                       quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                       unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                       derivationDicomCode=self.createCodedEntry("255605001", "SCT", _("Minimum"), True))

        info["max"] = \
            self.createMeasurementInfo(name="Maximum", description=_("Maximum scalar value"),
                                       units=scalarVolumeUnits.GetCodeMeaning(),
                                       quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                       unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                       derivationDicomCode=self.createCodedEntry("56851009", "SCT", _("Maximum"), True))

        info["mean"] = \
            self.createMeasurementInfo(name="Mean", description=_("Mean scalar value"),
                                       units=scalarVolumeUnits.GetCodeMeaning(),
                                       quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                       unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                       derivationDicomCode=self.createCodedEntry("373098007", "SCT", _("Mean"), True))

        info["median"] = \
            self.createMeasurementInfo(name="Median", description=_("Median scalar value"),
                                       units=scalarVolumeUnits.GetCodeMeaning(),
                                       quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                       unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                       derivationDicomCode=self.createCodedEntry("median", "SCT", _("Median"), True))

        info["stdev"] = \
            self.createMeasurementInfo(name="Standard deviation", description=_("Standard deviation of scalar values"),
                                       units=scalarVolumeUnits.GetCodeMeaning(),
                                       quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                       unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                       derivationDicomCode=self.createCodedEntry('386136009', 'SCT', _('Standard Deviation'), True))

        return info[key] if key in info else None

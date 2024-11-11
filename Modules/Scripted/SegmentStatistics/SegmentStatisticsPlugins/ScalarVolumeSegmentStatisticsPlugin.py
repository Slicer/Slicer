import vtk, slicer
from slicer.i18n import tr as _
from SegmentStatisticsPlugins import SegmentStatisticsPluginBase
from functools import reduce


class ScalarVolumeSegmentStatisticsPlugin(SegmentStatisticsPluginBase):
    """Statistical plugin for segmentations with scalar volumes"""

    def __init__(self):
        super().__init__()
        self.name = "Scalar Volume"
        self.title = _("Scalar Volume")
        self.keys = [
            "voxel_count", "volume_mm3", "volume_cm3", "min", "max", "mean", "stdev",
            "percentile_05", "percentile_10", "percentile_90", "percentile_95", "median",
        ]
        # skip 10th and 90th percentiles by default to keep the table compact
        self.defaultKeys = [
            "voxel_count", "volume_mm3", "volume_cm3", "min", "max", "mean", "stdev",
            "percentile_05", "percentile_95", "median",
        ]
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

        histogram = vtk.vtkImageHistogramStatistics()
        histogram.SetInputData(grayscaleNode.GetImageData())
        histogram.SetStencilData(stencil.GetOutput())
        histogram.SetAutoRangePercentiles(5, 95)
        histogram.SetAutoRangeExpansionFactors(0, 0)  # compute exact percentiles (do not add margin)
        histogram.Update()

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
                stats["median"] = histogram.GetMedian()
            if "percentile_05" or "percentile_95" in requestedKeys:
                # percentiles for 5 and 95 are already computed
                if "percentile_05" in requestedKeys:
                    stats["percentile_05"] = histogram.GetAutoRange()[0]
                if "percentile_95" in requestedKeys:
                    stats["percentile_95"] = histogram.GetAutoRange()[1]
            if "percentile_10" or "percentile_90" in requestedKeys:
                histogram.SetAutoRangePercentiles(10, 90)
                histogram.Update()
                if "percentile_10" in requestedKeys:
                    stats["percentile_10"] = histogram.GetAutoRange()[0]
                if "percentile_90" in requestedKeys:
                    stats["percentile_90"] = histogram.GetAutoRange()[1]
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

        # @fedorov could not find any suitable DICOM quantity code for "number of voxels".
        # DCM has "Number of needles" etc., so probably "Number of voxels"
        # should be added too. Need to discuss with @dclunie. For now, a
        # QIICR private scheme placeholder.
        # @moselhy also could not find DICOM quantity code for "median"

        if key == "voxel_count":
            return self.createMeasurementInfo(name="Voxel count",
                                       title=_("Voxel count"),
                                       description=_("Number of voxels. Computed from region of the binary labelmap representation of the segment"
                                                     " that overlaps with the input scalar volume."),
                                       units="",
                                       quantityDicomCode=self.createCodedEntry("nvoxels", "99QIICR", "Number of voxels", True),
                                       unitsDicomCode=self.createCodedEntry("voxels", "UCUM", "voxels", True))

        elif key == "volume_mm3":
            return self.createMeasurementInfo(name="Volume mm3",
                                       title=_("Volume"),
                                       description=_("Volume of the region of the binary labelmap representation that overlaps with the input scalar volume."),
                                       units=_("mm3"),
                                       quantityDicomCode=self.createCodedEntry("118565006", "SCT", "Volume", True),
                                       unitsDicomCode=self.createCodedEntry("mm3", "UCUM", "cubic millimeter", True))

        elif key == "volume_cm3":
            return self.createMeasurementInfo(name="Volume cm3",
                                       title=_("Volume"),
                                       description=_("Volume of the region of the binary labelmap representation that overlaps with the input scalar volume."),
                                       units=_("cm3"),
                                       quantityDicomCode=self.createCodedEntry("118565006", "SCT", "Volume", True),
                                       unitsDicomCode=self.createCodedEntry("cm3", "UCUM", "cubic centimeter", True),
                                       measurementMethodDicomCode=self.createCodedEntry("126030", "DCM", "Sum of segmented voxel volumes", True))

        elif key == "min":
            return self.createMeasurementInfo(name="Minimum", title=_("Minimum"), description=_("Minimum input scalar volume voxel value within the segment."),
                                       units=scalarVolumeUnits.GetCodeMeaning(),
                                       quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                       unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                       derivationDicomCode=self.createCodedEntry("255605001", "SCT", "Minimum", True))

        elif key == "max":
            return self.createMeasurementInfo(name="Maximum", title=_("Maximum"), description=_("Maximum input scalar volume voxel value within the segment."),
                                       units=scalarVolumeUnits.GetCodeMeaning(),
                                       quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                       unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                       derivationDicomCode=self.createCodedEntry("56851009", "SCT", "Maximum", True))

        elif key == "mean":
            return self.createMeasurementInfo(name="Mean", title=_("Mean"), description=_("Mean input scalar volume voxel value within the segment."),
                                       units=scalarVolumeUnits.GetCodeMeaning(),
                                       quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                       unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                       derivationDicomCode=self.createCodedEntry("373098007", "SCT", "Mean", True))

        elif key == "stdev":
            return self.createMeasurementInfo(name="Standard deviation",
                                       title=_("Standard Deviation"),
                                       description=_("Standard deviation of input scalar volume voxel values within the segment."),
                                       units=scalarVolumeUnits.GetCodeMeaning(),
                                       quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                       unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                       derivationDicomCode=self.createCodedEntry("386136009", "SCT", "Standard Deviation", True))

        elif key == "percentile_05":
            return self.createMeasurementInfo(name="Percentile 5",
                                       title=_("Percentile 5"),
                                       description=_("5th percentile of input scalar volume voxel values within the segment."),
                                       units=scalarVolumeUnits.GetCodeMeaning(),
                                       quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                       unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                       derivationDicomCode=self.createCodedEntry("371888009", "SCT", "Fifth percentile", True))
        elif key == "percentile_10":
            return self.createMeasurementInfo(name="Percentile 10",
                                       title=_("Percentile 10"),
                                       description=_("10th percentile of input scalar volume voxel values within the segment."),
                                       units=scalarVolumeUnits.GetCodeMeaning(),
                                       quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                       unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                       derivationDicomCode=self.createCodedEntry("371890005", "SCT", "Tenth percentile", True))
        elif key == "percentile_90":
            return self.createMeasurementInfo(name="Percentile 90",
                                       title=_("Percentile 90"),
                                       description=_("90th percentile of input scalar volume voxel values within the segment."),
                                       units=scalarVolumeUnits.GetCodeMeaning(),
                                       quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                       unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                       derivationDicomCode=self.createCodedEntry("371887004", "SCT", "Ninetieth percentile", True))
        elif key == "percentile_95":
            return self.createMeasurementInfo(name="Percentile 95",
                                       title=_("Percentile 95"),
                                       description=_("95th percentile of input scalar volume voxel values within the segment."),
                                       units=scalarVolumeUnits.GetCodeMeaning(),
                                       quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                       unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                       derivationDicomCode=self.createCodedEntry("371889001", "SCT", "Ninety-fifth percentile", True))

        elif key == "median":
            return self.createMeasurementInfo(name="Median",
                                       title=_("Median"),
                                       description=_("Median input scalar volume voxel value within the segment."),
                                       units=scalarVolumeUnits.GetCodeMeaning(),
                                       quantityDicomCode=scalarVolumeQuantity.GetAsString(),
                                       unitsDicomCode=scalarVolumeUnits.GetAsString(),
                                       derivationDicomCode=self.createCodedEntry("373099004", "SCT", "Median", True))

        return None

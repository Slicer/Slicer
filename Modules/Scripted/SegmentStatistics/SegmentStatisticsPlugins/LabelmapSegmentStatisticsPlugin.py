import logging
from functools import reduce

import vtk

import slicer
import vtkITK
from SegmentStatisticsPlugins import SegmentStatisticsPluginBase
from slicer.i18n import tr as _


class LabelmapSegmentStatisticsPlugin(SegmentStatisticsPluginBase):
    """Statistical plugin for Labelmaps"""

    def __init__(self):
        super().__init__()
        self.name = "Label Map"
        self.title = _("Label Map")
        self.obbKeys = ["obb_origin_ras", "obb_diameter_mm", "obb_direction_ras_x", "obb_direction_ras_y", "obb_direction_ras_z"]
        self.principalAxisKeys = ["principal_axis_x", "principal_axis_y", "principal_axis_z"]
        self.shapeKeys = [
            "centroid_ras", "feret_diameter_mm", "surface_area_mm2", "roundness", "flatness", "elongation",
            "principal_moments",
        ] + self.principalAxisKeys + self.obbKeys

        self.defaultKeys = ["voxel_count", "volume_mm3", "volume_cm3"]  # Don't calculate label shape statistics by default since they take longer to compute
        self.keys = self.defaultKeys + self.shapeKeys
        self.keyToShapeStatisticNames = {
            "centroid_ras": vtkITK.vtkITKLabelShapeStatistics.GetShapeStatisticAsString(vtkITK.vtkITKLabelShapeStatistics.Centroid),
            "feret_diameter_mm": vtkITK.vtkITKLabelShapeStatistics.GetShapeStatisticAsString(vtkITK.vtkITKLabelShapeStatistics.FeretDiameter),
            "surface_area_mm2": vtkITK.vtkITKLabelShapeStatistics.GetShapeStatisticAsString(vtkITK.vtkITKLabelShapeStatistics.Perimeter),
            "roundness": vtkITK.vtkITKLabelShapeStatistics.GetShapeStatisticAsString(vtkITK.vtkITKLabelShapeStatistics.Roundness),
            "flatness": vtkITK.vtkITKLabelShapeStatistics.GetShapeStatisticAsString(vtkITK.vtkITKLabelShapeStatistics.Flatness),
            "elongation": vtkITK.vtkITKLabelShapeStatistics.GetShapeStatisticAsString(vtkITK.vtkITKLabelShapeStatistics.Elongation),
            "oriented_bounding_box": vtkITK.vtkITKLabelShapeStatistics.GetShapeStatisticAsString(vtkITK.vtkITKLabelShapeStatistics.OrientedBoundingBox),
            "obb_origin_ras": "OrientedBoundingBoxOrigin",
            "obb_diameter_mm": "OrientedBoundingBoxSize",
            "obb_direction_ras_x": "OrientedBoundingBoxDirectionX",
            "obb_direction_ras_y": "OrientedBoundingBoxDirectionY",
            "obb_direction_ras_z": "OrientedBoundingBoxDirectionZ",
            "principal_moments": vtkITK.vtkITKLabelShapeStatistics.GetShapeStatisticAsString(vtkITK.vtkITKLabelShapeStatistics.PrincipalMoments),
            "principal_axes": vtkITK.vtkITKLabelShapeStatistics.GetShapeStatisticAsString(vtkITK.vtkITKLabelShapeStatistics.PrincipalAxes),
            "principal_axis_x": "PrincipalAxisX",
            "principal_axis_y": "PrincipalAxisY",
            "principal_axis_z": "PrincipalAxisZ",
        }
        # ... developer may add extra options to configure other parameters

    def computeStatistics(self, segmentID):
        import vtkSegmentationCorePython as vtkSegmentationCore

        requestedKeys = self.getRequestedKeys()

        segmentationNode = slicer.mrmlScene.GetNodeByID(self.getParameterNode().GetParameter("Segmentation"))

        if len(requestedKeys) == 0:
            return {}

        containsLabelmapRepresentation = segmentationNode.GetSegmentation().ContainsRepresentation(
            vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationBinaryLabelmapRepresentationName())
        if not containsLabelmapRepresentation:
            return {}

        segmentLabelmap = slicer.vtkOrientedImageData()
        segmentationNode.GetBinaryLabelmapRepresentation(segmentID, segmentLabelmap)
        if (not segmentLabelmap
            or not segmentLabelmap.GetPointData()
                or not segmentLabelmap.GetPointData().GetScalars()):
            # No input label data
            return {}

        # We need to know exactly the value of the segment voxels, apply threshold to make force the selected label value
        labelValue = 1
        backgroundValue = 0
        thresh = vtk.vtkImageThreshold()
        thresh.SetInputData(segmentLabelmap)
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

        stat = vtk.vtkImageAccumulate()
        stat.SetInputData(thresh.GetOutput())
        stat.SetStencilData(stencil.GetOutput())
        stat.Update()

        # Add data to statistics list
        cubicMMPerVoxel = reduce(lambda x, y: x * y, segmentLabelmap.GetSpacing())
        ccPerCubicMM = 0.001
        stats = {}
        if "voxel_count" in requestedKeys:
            stats["voxel_count"] = stat.GetVoxelCount()
        if "volume_mm3" in requestedKeys:
            stats["volume_mm3"] = stat.GetVoxelCount() * cubicMMPerVoxel
        if "volume_cm3" in requestedKeys:
            stats["volume_cm3"] = stat.GetVoxelCount() * cubicMMPerVoxel * ccPerCubicMM

        calculateShapeStats = False
        for shapeKey in self.shapeKeys:
            if shapeKey in requestedKeys:
                calculateShapeStats = True
                break

        if calculateShapeStats:
            directions = vtk.vtkMatrix4x4()
            segmentLabelmap.GetDirectionMatrix(directions)

            # Remove oriented bounding box from requested keys and replace with individual keys
            requestedOptions = requestedKeys
            statFilterOptions = self.shapeKeys
            calculateOBB = (
                "obb_diameter_mm" in requestedKeys
                or "obb_origin_ras" in requestedKeys
                or "obb_direction_ras_x" in requestedKeys
                or "obb_direction_ras_y" in requestedKeys
                or "obb_direction_ras_z" in requestedKeys
            )

            if calculateOBB:
                temp = statFilterOptions
                statFilterOptions = []
                for option in temp:
                    if option not in self.obbKeys:
                        statFilterOptions.append(option)
                statFilterOptions.append("oriented_bounding_box")

                temp = requestedOptions
                requestedOptions = []
                for option in temp:
                    if option not in self.obbKeys:
                        requestedOptions.append(option)
                requestedOptions.append("oriented_bounding_box")

            calculatePrincipalAxis = (
                "principal_axis_x" in requestedKeys
                or "principal_axis_y" in requestedKeys
                or "principal_axis_z" in requestedKeys
            )
            if calculatePrincipalAxis:
                temp = statFilterOptions
                statFilterOptions = []
                for option in temp:
                    if option not in self.principalAxisKeys:
                        statFilterOptions.append(option)
                statFilterOptions.append("principal_axes")

                temp = requestedOptions
                requestedOptions = []
                for option in temp:
                    if option not in self.principalAxisKeys:
                        requestedOptions.append(option)
                requestedOptions.append("principal_axes")
                requestedOptions.append("centroid_ras")

            shapeStat = vtkITK.vtkITKLabelShapeStatistics()
            shapeStat.SetInputData(thresh.GetOutput())
            shapeStat.SetDirections(directions)
            for shapeKey in statFilterOptions:
                shapeStat.SetComputeShapeStatistic(self.keyToShapeStatisticNames[shapeKey], shapeKey in requestedOptions)
            shapeStat.Update()

            # If segmentation node is transformed, apply that transform to get RAS coordinates
            transformSegmentToRas = vtk.vtkGeneralTransform()
            slicer.vtkMRMLTransformNode.GetTransformBetweenNodes(segmentationNode.GetParentTransformNode(), None, transformSegmentToRas)

            statTable = shapeStat.GetOutput()
            if "centroid_ras" in requestedKeys:
                centroidRAS = [0, 0, 0]
                centroidTuple = None
                centroidArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["centroid_ras"])
                if centroidArray is None:
                    logging.error("Could not calculate centroid_ras!")
                else:
                    centroidTuple = centroidArray.GetTuple(0)
                if centroidTuple is not None:
                    transformSegmentToRas.TransformPoint(centroidTuple, centroidRAS)
                    stats["centroid_ras"] = centroidRAS

            if "roundness" in requestedKeys:
                roundnessTuple = None
                roundnessArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["roundness"])
                if roundnessArray is None:
                    logging.error("Could not calculate roundness!")
                else:
                    roundnessTuple = roundnessArray.GetTuple(0)
                if roundnessTuple is not None:
                    roundness = roundnessTuple[0]
                    stats["roundness"] = roundness

            if "flatness" in requestedKeys:
                flatnessTuple = None
                flatnessArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["flatness"])
                if flatnessArray is None:
                    logging.error("Could not calculate flatness!")
                else:
                    flatnessTuple = flatnessArray.GetTuple(0)
                if flatnessTuple is not None:
                    flatness = flatnessTuple[0]
                    stats["flatness"] = flatness

            if "elongation" in requestedKeys:
                elongationTuple = None
                elongationArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["elongation"])
                if elongationArray is None:
                    logging.error("Could not calculate elongation!")
                else:
                    elongationTuple = elongationArray.GetTuple(0)
                if elongationTuple is not None:
                    elongation = elongationTuple[0]
                    stats["elongation"] = elongation

            if "feret_diameter_mm" in requestedKeys:
                feretDiameterTuple = None
                feretDiameterArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["feret_diameter_mm"])
                if feretDiameterArray is None:
                    logging.error("Could not calculate feret_diameter_mm!")
                else:
                    feretDiameterTuple = feretDiameterArray.GetTuple(0)
                if feretDiameterTuple is not None:
                    feretDiameter = feretDiameterTuple[0]
                    stats["feret_diameter_mm"] = feretDiameter

            if "surface_area_mm2" in requestedKeys:
                perimeterTuple = None
                perimeterArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["surface_area_mm2"])
                if perimeterArray is None:
                    logging.error("Could not calculate surface_area_mm2!")
                else:
                    perimeterTuple = perimeterArray.GetTuple(0)
                if perimeterTuple is not None:
                    perimeter = perimeterTuple[0]
                    stats["surface_area_mm2"] = perimeter

            if "obb_origin_ras" in requestedKeys:
                obbOriginTuple = None
                obbOriginRAS = [0, 0, 0]
                obbOriginArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["obb_origin_ras"])
                if obbOriginArray is None:
                    logging.error("Could not calculate obb_origin_ras!")
                else:
                    obbOriginTuple = obbOriginArray.GetTuple(0)
                if obbOriginTuple is not None:
                    transformSegmentToRas.TransformPoint(obbOriginTuple, obbOriginRAS)
                    stats["obb_origin_ras"] = obbOriginRAS

            if "obb_diameter_mm" in requestedKeys:
                obbDiameterMMTuple = None
                obbDiameterArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["obb_diameter_mm"])
                if obbDiameterArray is None:
                    logging.error("Could not calculate obb_diameter_mm!")
                else:
                    obbDiameterMMTuple = obbDiameterArray.GetTuple(0)
                if obbDiameterMMTuple is not None:
                    obbDiameterMM = list(obbDiameterMMTuple)
                    stats["obb_diameter_mm"] = obbDiameterMM

            if "obb_direction_ras_x" in requestedKeys:
                obbOriginTuple = None
                obbOriginArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["obb_origin_ras"])
                if obbOriginArray is None:
                    logging.error("Could not calculate obb_direction_ras_x!")
                else:
                    obbOriginTuple = obbOriginArray.GetTuple(0)

                obbDirectionXTuple = None
                obbDirectionXArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["obb_direction_ras_x"])
                if obbDirectionXArray is None:
                    logging.error("Could not calculate obb_direction_ras_x!")
                else:
                    obbDirectionXTuple = obbDirectionXArray.GetTuple(0)

                if obbOriginTuple is not None and obbDirectionXTuple is not None:
                    obbDirectionX = list(obbDirectionXTuple)
                    transformSegmentToRas.TransformVectorAtPoint(obbOriginTuple, obbDirectionX, obbDirectionX)
                    stats["obb_direction_ras_x"] = obbDirectionX

            if "obb_direction_ras_y" in requestedKeys:
                obbOriginTuple = None
                obbOriginArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["obb_origin_ras"])
                if obbOriginArray is None:
                    logging.error("Could not calculate obb_direction_ras_y!")
                else:
                    obbOriginTuple = obbOriginArray.GetTuple(0)

                obbDirectionYTuple = None
                obbDirectionYArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["obb_direction_ras_y"])
                if obbDirectionYArray is None:
                    logging.error("Could not calculate obb_direction_ras_y!")
                else:
                    obbDirectionYTuple = obbDirectionYArray.GetTuple(0)

                if obbOriginTuple is not None and obbDirectionYTuple is not None:
                    obbDirectionY = list(obbDirectionYTuple)
                    transformSegmentToRas.TransformVectorAtPoint(obbOriginTuple, obbDirectionY, obbDirectionY)
                    stats["obb_direction_ras_y"] = obbDirectionY

            if "obb_direction_ras_z" in requestedKeys:
                obbOriginTuple = None
                obbOriginArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["obb_origin_ras"])
                if obbOriginArray is None:
                    logging.error("Could not calculate obb_direction_ras_z!")
                else:
                    obbOriginTuple = obbOriginArray.GetTuple(0)

                obbDirectionZTuple = None
                obbDirectionZArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["obb_direction_ras_z"])
                if obbDirectionZArray is None:
                    logging.error("Could not calculate obb_direction_ras_z!")
                else:
                    obbDirectionZTuple = obbDirectionZArray.GetTuple(0)

                if obbOriginTuple is not None and obbDirectionZTuple is not None:
                    obbDirectionZ = list(obbDirectionZTuple)
                    transformSegmentToRas.TransformVectorAtPoint(obbOriginTuple, obbDirectionZ, obbDirectionZ)
                    stats["obb_direction_ras_z"] = obbDirectionZ

            if "principal_moments" in requestedKeys:
                principalMomentsTuple = None
                principalMomentsArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["principal_moments"])
                if principalMomentsArray is None:
                    logging.error("Could not calculate principal_moments!")
                else:
                    principalMomentsTuple = principalMomentsArray.GetTuple(0)
                if principalMomentsTuple is not None:
                    principalMoments = list(principalMomentsTuple)
                    stats["principal_moments"] = principalMoments

            if "principal_axis_x" in requestedKeys:
                centroidRASTuple = None
                centroidRASArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["centroid_ras"])
                if centroidRASArray is None:
                    logging.error("Could not calculate principal_axis_x!")
                else:
                    centroidRASTuple = centroidRASArray.GetTuple(0)

                principalAxisXTuple = None
                principalAxisXArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["principal_axis_x"])
                if principalAxisXArray is None:
                    logging.error("Could not calculate principal_axis_x!")
                else:
                    principalAxisXTuple = principalAxisXArray.GetTuple(0)

                if centroidRASTuple is not None and principalAxisXTuple is not None:
                    principalAxisX = list(principalAxisXTuple)
                    transformSegmentToRas.TransformVectorAtPoint(centroidRASTuple, principalAxisX, principalAxisX)
                    stats["principal_axis_x"] = principalAxisX

            if "principal_axis_y" in requestedKeys:
                centroidRASTuple = None
                centroidRASArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["centroid_ras"])
                if centroidRASArray is None:
                    logging.error("Could not calculate principal_axis_y!")
                else:
                    centroidRASTuple = centroidRASArray.GetTuple(0)

                principalAxisYTuple = None
                principalAxisYArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["principal_axis_y"])
                if principalAxisYArray is None:
                    logging.error("Could not calculate principal_axis_y!")
                else:
                    principalAxisYTuple = principalAxisYArray.GetTuple(0)

                if centroidRASTuple is not None and principalAxisYTuple is not None:
                    principalAxisY = list(principalAxisYTuple)
                    transformSegmentToRas.TransformVectorAtPoint(centroidRASTuple, principalAxisY, principalAxisY)
                    stats["principal_axis_y"] = principalAxisY

            if "principal_axis_z" in requestedKeys:
                centroidRASTuple = None
                centroidRASArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["centroid_ras"])
                if centroidRASArray is None:
                    logging.error("Could not calculate principal_axis_z!")
                else:
                    centroidRASTuple = centroidRASArray.GetTuple(0)

                principalAxisZTuple = None
                principalAxisZArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["principal_axis_z"])
                if principalAxisZArray is None:
                    logging.error("Could not calculate principal_axis_z!")
                else:
                    principalAxisZTuple = principalAxisZArray.GetTuple(0)

                if centroidRASTuple is not None and principalAxisZTuple is not None:
                    principalAxisZ = list(principalAxisZTuple)
                    transformSegmentToRas.TransformVectorAtPoint(centroidRASTuple, principalAxisZ, principalAxisZ)
                    stats["principal_axis_z"] = principalAxisZ

        return stats

    def getMeasurementInfo(self, key):
        """Get information (name, description, units, ...) about the measurement for the given key"""

        # @fedorov could not find any suitable DICOM quantity code for "number of voxels".
        # DCM has "Number of needles" etc., so probably "Number of voxels"
        # should be added too. Need to discuss with @dclunie. For now, a
        # QIICR private scheme placeholder.
        if key == "voxel_count":
            return self.createMeasurementInfo(name="Voxel count",
                                       title=_("Voxel count"),
                                       description=_("Number of voxels in the binary labelmap representation of the segment."),
                                       units="",
                                       quantityDicomCode=self.createCodedEntry("nvoxels", "99QIICR", "Number of voxels", True),
                                       unitsDicomCode=self.createCodedEntry("voxels", "UCUM", "voxels", True))

        elif key == "volume_mm3":
            return self.createMeasurementInfo(name="Volume mm3",
                                       title=_("Volume"),
                                       description=_("Volume of the segment computed from binary labelmap representation."),
                                       units=_("mm3"),
                                       quantityDicomCode=self.createCodedEntry("118565006", "SCT", "Volume", True),
                                       unitsDicomCode=self.createCodedEntry("mm3", "UCUM", "cubic millimeter", True))

        elif key == "volume_cm3":
            return self.createMeasurementInfo(name="Volume cm3",
                                       title=_("Volume"),
                                       description=_("Volume of the segment computed from binary labelmap representation."),
                                       units=_("cm3"),
                                       quantityDicomCode=self.createCodedEntry("118565006", "SCT", "Volume", True),
                                       unitsDicomCode=self.createCodedEntry("cm3", "UCUM", "cubic centimeter", True),
                                       measurementMethodDicomCode=self.createCodedEntry("126030", "DCM", "Sum of segmented voxel volumes", True))

        elif key == "centroid_ras":
            return self.createMeasurementInfo(name="Centroid",
                                       title=_("Centroid"),
                                       description=_("Location of the centroid in RAS. Computed from binary labelmap representation."),
                                       units="", componentNames=["r", "a", "s"])

        elif key == "feret_diameter_mm":
            return self.createMeasurementInfo(name="Feret diameter mm",
                                       title=_("Feret diameter"),
                                       description=_("Feret diameter of the segment computed from binary labelmap representation."), units=_("mm"))

        elif key == "surface_area_mm2":
            return self.createMeasurementInfo(name="Surface mm2",
                                       title=_("Surface area"),
                                       description=_("Surface area of the segment computed from binary labelmap representation."), units=_("mm2"),
                                       quantityDicomCode=self.createCodedEntry("000247", "99CHEMINF", "Surface area", True),
                                       unitsDicomCode=self.createCodedEntry("mm2", "UCUM", "square millimeter", True))

        elif key == "roundness":
            return self.createMeasurementInfo(name="Roundness",
                                      title=_("Roundness"),
                                      description=_("Segment roundness. Calculated from ratio of the area of the hypersphere by the actual area. "
                                                    "Value of 1 represents a spherical structure. Computed from binary labelmap representation."),
                                      units="")

        elif key == "flatness":
            return self.createMeasurementInfo(name="Flatness",
                                       title=_("Flatness"),
                                       description=_("Segment flatness. Calculated from square root of the ratio of the second smallest principal moment by "
                                                     "the smallest. Value of 0 represents a flat structure. Computed from binary labelmap representation.")
                                                     + " ( https://hdl.handle.net/1926/584 )",
                                       units="")

        elif key == "elongation":
            return self.createMeasurementInfo(name="Elongation",
                                       title=_("Elongation"),
                                       description=_("Segment elongation. Calculated from square root of the ratio of the second largest principal moment "
                                                     "by the second smallest. Computed from binary labelmap representation.")
                                                     + " ( https://hdl.handle.net/1926/584 )",
                                       units="")

        elif key == "oriented_bounding_box":
            return self.createMeasurementInfo(name="Oriented bounding box",
                                       title=_("Oriented bounding box"),
                                       description=_("Oriented bounding box. Computed from binary labelmap representation of the segment."),
                                       units="")

        elif key == "obb_origin_ras":
            return self.createMeasurementInfo(name="OBB origin",
                                       title=_("OBB origin"),
                                       description=_("Oriented bounding box origin in RAS coordinates."
                                                     " Computed from binary labelmap representation of the segment."),
                                       units="",
                                       componentNames=["r", "a", "s"])

        elif key == "obb_diameter_mm":
            return self.createMeasurementInfo(name="OBB diameter",
                                       title=_("OBB diameter"),
                                       description=_("Oriented bounding box diameter. Computed from binary labelmap representation of the segment."),
                                       units=_("mm"),
                                       componentNames=["x", "y", "z"])

        elif key == "obb_direction_ras_x":
            return self.createMeasurementInfo(name="OBB X direction",
                title=_("OBB X direction"),
                description=_("Oriented bounding box X direction in RAS coordinates. Computed from binary labelmap representation of the segment."),
                units="", componentNames=["r", "a", "s"])

        elif key == "obb_direction_ras_y":
            return self.createMeasurementInfo(name="OBB Y direction",
                                       title=_("OBB Y direction"),
                                       description=_("Oriented bounding box Y direction in RAS coordinates."
                                                     " Computed from binary labelmap representation of the segment."),
                                       units="",
                                       componentNames=["r", "a", "s"])

        elif key == "obb_direction_ras_z":
            return self.createMeasurementInfo(name="OBB Z direction",
                                       title=_("OBB Z direction"),
                                       description=_("Oriented bounding box Z direction in RAS coordinates."
                                                     " Computed from binary labelmap representation of the segment."),
                                       units="",
                                       componentNames=["r", "a", "s"])

        elif key == "principal_moments":
            return self.createMeasurementInfo(name="Principal moments",
                                       title=_("Principal moments"),
                                       description=_("Principal moments of inertia for x, y and z axes."
                                                     " Computed from binary labelmap representation of the segment."),
                                       units="",
                                       componentNames=["x", "y", "z"])

        elif key == "principal_axis_x":
            return self.createMeasurementInfo(name="Principal X axis", title=_("Principal X axis"),
                                       description=_("Principal X axis of rotation in RAS coordinates."
                                                     " Computed from binary labelmap representation of the segment."),
                                       units="",
                                       componentNames=["r", "a", "s"])

        elif key == "principal_axis_y":
            return self.createMeasurementInfo(name="Principal Y axis", title=_("Principal Y axis"),
                                       description=_("Principal Y axis of rotation in RAS coordinates."
                                                     " Computed from binary labelmap representation of the segment."),
                                       units="",
                                       componentNames=["r", "a", "s"])

        elif key == "principal_axis_z":
            return self.createMeasurementInfo(name="Principal Z axis", title=_("Principal Z axis"),
                                       description=_("Principal Z axis of rotation in RAS coordinates."
                                                     " Computed from binary labelmap representation of the segment."),
                                       units="",
                                       componentNames=["r", "a", "s"])

        return None

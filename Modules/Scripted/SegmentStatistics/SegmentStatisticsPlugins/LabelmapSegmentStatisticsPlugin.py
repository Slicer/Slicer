import vtk
import slicer
import vtkITK
from SegmentStatisticsPlugins import SegmentStatisticsPluginBase
from functools import reduce

class LabelmapSegmentStatisticsPlugin(SegmentStatisticsPluginBase):
  """Statistical plugin for Labelmaps"""

  def __init__(self):
    super(LabelmapSegmentStatisticsPlugin,self).__init__()
    self.name = "Labelmap"
    self.obbKeys = ["obb_origin_ras", "obb_diameter_mm", "obb_direction_ras_x", "obb_direction_ras_y", "obb_direction_ras_z"]
    self.shapeKeys = [
      "centroid_ras", "feret_diameter_mm", "surface_area_mm2", "roundness", "flatness",
      ] + self.obbKeys

    self.defaultKeys = ["voxel_count", "volume_mm3", "volume_cm3"] # Don't calculate label shape statistics by default since they take longer to compute
    self.keys = self.defaultKeys + self.shapeKeys
    self.keyToShapeStatisticNames = {
      "centroid_ras" : vtkITK.vtkITKLabelShapeStatistics.GetShapeStatisticAsString(vtkITK.vtkITKLabelShapeStatistics.Centroid),
      "feret_diameter_mm": vtkITK.vtkITKLabelShapeStatistics.GetShapeStatisticAsString(vtkITK.vtkITKLabelShapeStatistics.FeretDiameter),
      "surface_area_mm2" : vtkITK.vtkITKLabelShapeStatistics.GetShapeStatisticAsString(vtkITK.vtkITKLabelShapeStatistics.Perimeter),
      "roundness" : vtkITK.vtkITKLabelShapeStatistics.GetShapeStatisticAsString(vtkITK.vtkITKLabelShapeStatistics.Roundness),
      "flatness" : vtkITK.vtkITKLabelShapeStatistics.GetShapeStatisticAsString(vtkITK.vtkITKLabelShapeStatistics.Flatness),
      "oriented_bounding_box" : vtkITK.vtkITKLabelShapeStatistics.GetShapeStatisticAsString(vtkITK.vtkITKLabelShapeStatistics.OrientedBoundingBox),
      "obb_origin_ras" : "OrientedBoundingBoxOrigin",
      "obb_diameter_mm" : "OrientedBoundingBoxSize",
      "obb_direction_ras_x" : "OrientedBoundingBoxDirectionX",
      "obb_direction_ras_y" : "OrientedBoundingBoxDirectionY",
      "obb_direction_ras_z" : "OrientedBoundingBoxDirectionZ",
      }
    #... developer may add extra options to configure other parameters

  def computeStatistics(self, segmentID):
    import vtkSegmentationCorePython as vtkSegmentationCore
    requestedKeys = self.getRequestedKeys()

    segmentationNode = slicer.mrmlScene.GetNodeByID(self.getParameterNode().GetParameter("Segmentation"))

    if len(requestedKeys)==0:
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
    cubicMMPerVoxel = reduce(lambda x,y: x*y, segmentLabelmap.GetSpacing())
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
        "obb_diameter_mm" in requestedKeys or
        "obb_origin_ras" in requestedKeys or
        "obb_direction_ras_x" in requestedKeys or
        "obb_direction_ras_y" in requestedKeys or
        "obb_direction_ras_z" in requestedKeys
        )

      if calculateOBB:
        temp = statFilterOptions
        statFilterOptions = []
        for option in temp:
          if not option in self.obbKeys:
            statFilterOptions.append(option)
        statFilterOptions.append("oriented_bounding_box")

        temp = requestedOptions
        requestedOptions = []
        for option in temp:
          if not option in self.obbKeys:
            requestedOptions.append(option)
        requestedOptions.append("oriented_bounding_box")

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
        centroidRAS = [0,0,0]
        centroidArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["centroid_ras"])
        centroid = centroidArray.GetTuple(0)
        transformSegmentToRas.TransformPoint(centroid, centroidRAS)
        stats["centroid_ras"] = centroidRAS

      if "roundness" in requestedKeys:
        roundnessArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["roundness"])
        roundness = roundnessArray.GetTuple(0)[0]
        stats["roundness"] = roundness

      if "flatness" in requestedKeys:
        flatnessArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["flatness"])
        flatness = flatnessArray.GetTuple(0)[0]
        stats["flatness"] = flatness

      if "feret_diameter_mm" in requestedKeys:
        feretDiameterArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["feret_diameter_mm"])
        feretDiameter = feretDiameterArray.GetTuple(0)[0]
        stats["feret_diameter_mm"] = feretDiameter

      if "surface_area_mm2" in requestedKeys:
        perimeterArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["surface_area_mm2"])
        perimeter = perimeterArray.GetTuple(0)[0]
        stats["surface_area_mm2"] = perimeter

      if "obb_origin_ras" in requestedKeys:
        obbOriginRAS = [0,0,0]
        obbOriginArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["obb_origin_ras"])
        obbOrigin = obbOriginArray.GetTuple(0)
        transformSegmentToRas.TransformPoint(obbOrigin, obbOriginRAS)
        stats["obb_origin_ras"] = obbOriginRAS

      if "obb_diameter_mm" in requestedKeys:
        obbDiameterArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["obb_diameter_mm"])
        obbDiameterMM = list(obbDiameterArray.GetTuple(0))
        stats["obb_diameter_mm"] = obbDiameterMM

      if "obb_direction_ras_x" in requestedKeys:
        obbOriginArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["obb_origin_ras"])
        obbOrigin = obbOriginArray.GetTuple(0)
        obbDirectionXArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["obb_direction_ras_x"])
        obbDirectionX = list(obbDirectionXArray.GetTuple(0))
        transformSegmentToRas.TransformVectorAtPoint(obbOrigin, obbDirectionX, obbDirectionX)
        stats["obb_direction_ras_x"] = obbDirectionX

      if "obb_direction_ras_y" in requestedKeys:
        obbOriginArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["obb_origin_ras"])
        obbOrigin = obbOriginArray.GetTuple(0)
        obbDirectionYArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["obb_direction_ras_y"])
        obbDirectionY = list(obbDirectionYArray.GetTuple(0))
        transformSegmentToRas.TransformVectorAtPoint(obbOrigin, obbDirectionY, obbDirectionY)
        stats["obb_direction_ras_y"] = obbDirectionY

      if "obb_direction_ras_z" in requestedKeys:
        obbOriginArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["obb_origin_ras"])
        obbOrigin = obbOriginArray.GetTuple(0)
        obbDirectionZArray = statTable.GetColumnByName(self.keyToShapeStatisticNames["obb_direction_ras_z"])
        obbDirectionZ = list(obbDirectionZArray.GetTuple(0))
        transformSegmentToRas.TransformVectorAtPoint(obbOrigin, obbDirectionZ, obbDirectionZ)
        stats["obb_direction_ras_z"] = obbDirectionZ

    return stats

  def getMeasurementInfo(self, key):
    """Get information (name, description, units, ...) about the measurement for the given key"""
    info = {}

    # @fedorov could not find any suitable DICOM quantity code for "number of voxels".
    # DCM has "Number of needles" etc., so probably "Number of voxels"
    # should be added too. Need to discuss with @dclunie. For now, a
    # QIICR private scheme placeholder.
    info["voxel_count"] = \
      self.createMeasurementInfo(name="Voxel count", description="Number of voxels", units="voxels",
                                   quantityDicomCode=self.createCodedEntry("nvoxels", "99QIICR", "Number of voxels", True),
                                   unitsDicomCode=self.createCodedEntry("voxels", "UCUM", "voxels", True))

    info["volume_mm3"] = \
      self.createMeasurementInfo(name="Volume mm3", description="Volume in mm3", units="mm3",
                                   quantityDicomCode=self.createCodedEntry("G-D705", "SRT", "Volume", True),
                                   unitsDicomCode=self.createCodedEntry("mm3", "UCUM", "cubic millimeter", True))

    info["volume_cm3"] = \
      self.createMeasurementInfo(name="Volume cm3", description="Volume in cm3", units="cm3",
                                   quantityDicomCode=self.createCodedEntry("G-D705", "SRT", "Volume", True),
                                   unitsDicomCode=self.createCodedEntry("cm3", "UCUM", "cubic centimeter", True),
                                   measurementMethodDicomCode=self.createCodedEntry("126030", "DCM",
                                                                             "Sum of segmented voxel volumes", True))

    info["centroid_ras"] = \
      self.createMeasurementInfo(name="Centroid (RAS)", description="Location of the centroid in RAS", units="", componentNames=["r", "a", "s"])

    info["feret_diameter_mm"] = \
      self.createMeasurementInfo(name="Feret Diameter mm", description="Feret diameter in mm", units="mm")

    info["surface_area_mm2"] = \
      self.createMeasurementInfo(name="Surface mm2", description="Surface area in mm2", units="mm2",
                                   quantityDicomCode=self.createCodedEntry("000247", "99CHEMINF", "Surface area", True),
                                   unitsDicomCode=self.createCodedEntry("mm2", "UCUM", "squared millimeters", True))

    info["roundness"] = \
      self.createMeasurementInfo(name="Roundness", description="Roundness", units="")

    info["flatness"] = \
      self.createMeasurementInfo(name="Flatness", description="Flatness", units="")

    info["oriented_bounding_box"] = \
      self.createMeasurementInfo(name="Oriented bounding box", description="Oriented bounding box", units="")

    info["obb_origin_ras"] = \
      self.createMeasurementInfo(name="OBB origin (RAS)", description="Oriented bounding box origin", units="", componentNames=["r", "a", "s"])

    info["obb_diameter_mm"] = \
      self.createMeasurementInfo(name="OBB diameter", description="Oriented bounding box diameter", units="mm", componentNames=["x", "y", "z"])

    info["obb_direction_ras_x"] = \
      self.createMeasurementInfo(name="OBB X direction (RAS)", description="Oriented bounding box X direction", units="", componentNames=["r", "a", "s"])

    info["obb_direction_ras_y"] = \
      self.createMeasurementInfo(name="OBB Y direction (RAS)", description="Oriented bounding box Y direction", units="", componentNames=["r", "a", "s"])

    info["obb_direction_ras_z"] = \
      self.createMeasurementInfo(name="OBB Z direction (RAS)", description="Oriented bounding box Z direction", units="", componentNames=["r", "a", "s"])

    return info[key] if key in info else None

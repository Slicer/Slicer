import vtk
import slicer
from SegmentStatisticsPlugins import SegmentStatisticsPluginBase


class ClosedSurfaceSegmentStatisticsPlugin(SegmentStatisticsPluginBase):
  """Statistical plugin for closed surfaces"""

  def __init__(self):
    super(ClosedSurfaceSegmentStatisticsPlugin,self).__init__()
    self.name = "Closed Surface"
    self.keys = ["surface_mm2", "volume_mm3", "volume_cm3"]
    self.defaultKeys = self.keys # calculate all measurements by default
    #... developer may add extra options to configure other parameters

  def computeStatistics(self, segmentID):
    import vtkSegmentationCorePython as vtkSegmentationCore
    requestedKeys = self.getRequestedKeys()

    segmentationNode = slicer.mrmlScene.GetNodeByID(self.getParameterNode().GetParameter("Segmentation"))

    if len(requestedKeys)==0:
      return {}

    containsClosedSurfaceRepresentation = segmentationNode.GetSegmentation().ContainsRepresentation(
      vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationClosedSurfaceRepresentationName())
    if not containsClosedSurfaceRepresentation:
      return {}

    segmentClosedSurface = vtk.vtkPolyData()
    segmentationNode.GetClosedSurfaceRepresentation(segmentID, segmentClosedSurface)

    # Compute statistics
    massProperties = vtk.vtkMassProperties()
    massProperties.SetInputData(segmentClosedSurface)

    # Add data to statistics list
    ccPerCubicMM = 0.001
    stats = {}
    if "surface_mm2" in requestedKeys:
      stats["surface_mm2"] = massProperties.GetSurfaceArea()
    if "volume_mm3" in requestedKeys:
      stats["volume_mm3"] = massProperties.GetVolume()
    if "volume_cm3" in requestedKeys:
      stats["volume_cm3"] = massProperties.GetVolume() * ccPerCubicMM
    return stats

  def getMeasurementInfo(self, key):
    """Get information (name, description, units, ...) about the measurement for the given key"""
    info = dict()

    # I searched BioPortal, and found seemingly most suitable code.
    # Prefixed with "99" since CHEMINF is not a recognized DICOM coding scheme.
    # See https://bioportal.bioontology.org/ontologies/CHEMINF?p=classes&conceptid=http%3A%2F%2Fsemanticscience.org%2Fresource%2FCHEMINF_000247
    #
    info["surface_mm2"] = \
      self.createMeasurementInfo(name="Surface mm2", description="Surface area in mm2", units="mm2",
                                   quantityDicomCode=self.createCodedEntry("000247", "99CHEMINF", "Surface area", True),
                                   unitsDicomCode=self.createCodedEntry("mm2", "UCUM", "squared millimeters", True))

    info["volume_mm3"] = \
      self.createMeasurementInfo(name="Volume mm3", description="Volume in mm3", units="mm3",
                                   quantityDicomCode=self.createCodedEntry("118565006", "SCT", "Volume", True),
                                   unitsDicomCode=self.createCodedEntry("mm3", "UCUM", "cubic millimeter", True))

    info["volume_cm3"] = \
      self.createMeasurementInfo(name="Volume cm3", description="Volume in cm3", units="cm3",
                                   quantityDicomCode=self.createCodedEntry("118565006", "SCT", "Volume", True),
                                   unitsDicomCode=self.createCodedEntry("cm3", "UCUM", "cubic centimeter", True))

    return info[key] if key in info else None

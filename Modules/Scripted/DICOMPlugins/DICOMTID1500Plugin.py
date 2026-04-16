import json
import logging
import os
import subprocess
import vtk
import datetime
from collections import Counter

import numpy
import numpy as np
import highdicom as hd
import pydicom
from pydicom.sr.codedict import codes

import slicer
from DICOMLib import DICOMLoadable, DICOMPlugin


class DICOMTID1500PluginClass(DICOMPlugin):

  UID_EnhancedSRStorage = pydicom.uid.EnhancedSRStorage
  UID_ComprehensiveSRStorage = pydicom.uid.ComprehensiveSRStorage
  UID_Comprehensive3DSRStorage = pydicom.uid.Comprehensive3DSRStorage
  UID_SegmentationStorage = pydicom.uid.SegmentationStorage
  UID_RealWorldValueMappingStorage = pydicom.uid.RealWorldValueMappingStorage

  def __init__(self):
    super().__init__()
    self.loadType = "DICOM Structured Report TID1500"

    self.codings = {
      "imagingMeasurementReport": { "scheme": "DCM", "value": "126000" },
      "personObserver": { "scheme": "DCM", "value": "121008" },
      "imagingMeasurements": { "scheme": "DCM", "value": "126010" },
      "measurementGroup": { "scheme": "DCM", "value": "125007" },
      "trackingIdentifier": { "scheme": "DCM", "value": "112039" },
      "trackingUniqueIdentifier": { "scheme": "DCM", "value": "112040" },
      "findingSite": { "scheme": "SRT", "value": "G-C0E3" },
      "length": { "scheme": "SRT", "value": "G-D7FE" },
    }

    self.tags = {}
    self.tags["PatientID"] = "0010,0020"
    self.tags["StudyDate"] = "0008,0020"
    self.tags["StudyInstanceUID"] = "0020,000D"
    self.tags["SeriesInstanceUID"] = "0020,000E"
    self.tags["SeriesNumber"] = "0020,0011"
    self.tags["SeriesDescription"] = "0008,103E"
    self.tags["SOPInstanceUID"] = "0008,0018"
    self.tags["ReferencedSeriesSequence"] = "0008,1115"
    self.tags["Modality"] = "0008,0060"
    self.tags["FrameOfReferenceUID"] = "0020,0052"
    self.tags["PixelSpacing"] = "0028,0030"
    self.tags["Rows"] = "0028,0010"
    self.tags["Columns"] = "0028,0011"

  def getDICOMValue(self, dataset, attribute, default=""):
    """Get an attribute value from a pydicom Dataset, returning default if not present."""
    try:
      return getattr(dataset, attribute)
    except AttributeError:
      return default

  def examineFiles(self, files):

    loadables = []

    for cFile in files:
      dataset = pydicom.dcmread(cFile)

      uid = self.getDICOMValue(dataset, "SOPInstanceUID")
      if uid == "":
        return []

      seriesDescription = self.getDICOMValue(dataset, "SeriesDescription", "Unknown")

      isDicomTID1500 = self.isDICOMTID1500(dataset)

      if isDicomTID1500:
        loadable = self.createLoadableAndAddReferences([dataset])
        loadable.files = [cFile]
        loadable.name = seriesDescription + " - as a DICOM SR TID1500 object"
        loadable.tooltip = loadable.name
        loadable.selected = True
        loadable.confidence = 0.95
        loadable.uids = [uid]
        refName = self.referencedSeriesName(loadable)
        if refName != "":
          loadable.name = refName + " " + seriesDescription + " - SR TID1500"

        loadables.append(loadable)

        logging.debug("DICOM SR TID1500 modality found")

    return loadables

  def isDICOMTID1500(self, dataset):
    """"
    This function checks if the dataset is a TID1500 or not.
    """

    try:
      isDicomTID1500 = self.getDICOMValue(dataset, "Modality") == "SR" and \
                       (self.getDICOMValue(dataset, "SOPClassUID") == self.UID_EnhancedSRStorage or
                        self.getDICOMValue(dataset, "SOPClassUID") == self.UID_ComprehensiveSRStorage or
                        self.getDICOMValue(dataset, "SOPClassUID") == self.UID_Comprehensive3DSRStorage) and \
                       self.getDICOMValue(dataset, "ContentTemplateSequence")[0].TemplateIdentifier == "1500"
    except (AttributeError, IndexError):
      isDicomTID1500 = False
    return isDicomTID1500

  def referencedSeriesName(self, loadable):
    """Returns the default series name for the given loadable."""

    referencedName = "Unnamed Reference"
    if hasattr(loadable, "referencedSOPInstanceUID"):
      referencedName = self.defaultSeriesNodeName(loadable.referencedSOPInstanceUID)
    return referencedName

  def getSRCode(self, code_meaning):
    """
    This function takes as input a standardized string, and returns
    the highdicom code, either using pydicom or defining it.
    """

    if (code_meaning == "Image Region"):
      highdicom_code = codes.DCM.ImageRegion # instead of using hd.sr.value_types.Code(value='111030',scheme_designator='DCM',meaning='Image Region')
    elif (code_meaning == "Geometric purpose of region"):
      highdicom_code = hd.sr.value_types.Code(
                        value="130400",
                        scheme_designator="DCM",
                        meaning="Geometric purpose of region",
                      )
    elif (code_meaning == "Bounded by"):
      highdicom_code = hd.sr.value_types.Code(
                        value="75958009",
                        scheme_designator="SCT",
                        meaning="Bounded by",
                      ) # codes.SCT.BoundedBy from pydicom does not exist.
    else:
      highdicom_code = ""

    return highdicom_code

  def getSOPInstanceUIDsForSeries(self, SeriesInstanceUID):
    """Gets the list of SOPInstanceUIDs from a SeriesInstanceUID."""

    db = slicer.dicomDatabase
    SOPInstanceUIDs = []
    fileList = db.filesForSeries(SeriesInstanceUID)
    for file in fileList:
      SOPInstanceUID = db.fileValue(file, self.tags["SOPInstanceUID"])
      SOPInstanceUIDs.append(SOPInstanceUID)

    return SOPInstanceUIDs

  def createLoadableAndAddReferences(self, datasets):
    """Main function to create the loadable and add the necessary references."""

    loadable = DICOMLoadable()
    loadable.selected = True
    loadable.confidence = 0.95

    loadable.referencedSegInstanceUIDs = []
    # store lists of UIDs separately to avoid re-parsing later
    loadable.ReferencedSegmentationInstanceUIDs = {}
    loadable.ReferencedRWVMSeriesInstanceUIDs = []
    loadable.ReferencedOtherInstanceUIDs = []
    loadable.referencedInstanceUIDs = []

    segPlugin = slicer.modules.dicomPlugins["DICOMSegmentationPlugin"]()

    for dataset in datasets:

      ### First we convert from dataset to sr ###
      sr_class_map = {
          pydicom.uid.EnhancedSRStorage: hd.sr.EnhancedSR,
          pydicom.uid.ComprehensiveSRStorage: hd.sr.ComprehensiveSR,
          pydicom.uid.Comprehensive3DSRStorage: hd.sr.Comprehensive3DSR,
      }
      # Get the SOP Class UID from the dataset
      sop_class_uid = dataset.SOPClassUID
      # Retrieve the correct SR class from the dictionary
      sr_class = sr_class_map.get(sop_class_uid)
      # Instantiate the correct SR object
      if sr_class is not None:
        sr = sr_class.from_dataset(dataset, copy=True)
      else:
        logging.error(f"Cannot create SR from dataset. Unsupported SOP Class UID: {sop_class_uid}")

      ### First we check if the SR contains planar annotations ###
      containsPlanarAnnotations = self.containsPlanarAnnotations(sr)

      ### If it doesn't contain planar annotations, and instead references a seg, we use the original code ###
      if not containsPlanarAnnotations:

        uid = self.getDICOMValue(dataset, "SOPInstanceUID")
        loadable.ReferencedSegmentationInstanceUIDs[uid] = []
        if hasattr(dataset, "CurrentRequestedProcedureEvidenceSequence"):
          for refSeriesSequence in dataset.CurrentRequestedProcedureEvidenceSequence:
            for referencedSeriesSequence in refSeriesSequence.ReferencedSeriesSequence:
              for refSOPSequence in referencedSeriesSequence.ReferencedSOPSequence:
                if refSOPSequence.ReferencedSOPClassUID == self.UID_SegmentationStorage:
                  logging.debug("Found referenced segmentation")
                  loadable.ReferencedSegmentationInstanceUIDs[uid].append(referencedSeriesSequence.SeriesInstanceUID)

                elif refSOPSequence.ReferencedSOPClassUID == self.UID_RealWorldValueMappingStorage: # handle SUV mapping
                  logging.debug("Found referenced RWVM")
                  loadable.ReferencedRWVMSeriesInstanceUIDs.append(referencedSeriesSequence.SeriesInstanceUID)
                else:
                  # TODO: those are not used at all
                  logging.debug( "Found other reference")
                  loadable.ReferencedOtherInstanceUIDs.append(refSOPSequence.ReferencedSOPInstanceUID)

        for segSeriesInstanceUID in loadable.ReferencedSegmentationInstanceUIDs[uid]:
          segLoadables = segPlugin.examine([slicer.dicomDatabase.filesForSeries(segSeriesInstanceUID)])
          for segLoadable in segLoadables:
            loadable.referencedInstanceUIDs += segLoadable.referencedInstanceUIDs

        if len(loadable.ReferencedSegmentationInstanceUIDs[uid])>1:
          logging.warning("SR references more than one SEG. This has not been tested!")
        for segUID in loadable.ReferencedSegmentationInstanceUIDs:
          loadable.referencedSegInstanceUIDs.append(segUID)

        if len(loadable.ReferencedRWVMSeriesInstanceUIDs)>1:
          logging.warning("SR references more than one RWVM. This has not been tested!")
            # not adding RWVM instances to referencedSeriesInstanceUIDs

      ### Additions for handling planar annotations ###
      else:

        #### Then we check if the SR contains a point3d, line, or bbox ###
        checkIfSRContainsBbox = self.checkIfSRContainsGeometry(sr, geometry_type="bbox2D")
        checkIfSRContains3DPoint = self.checkIfSRContainsGeometry(sr, geometry_type="point3D")
        checkIfSRContainsPolyline = self.checkIfSRContainsGeometry(sr, geometry_type="polyline2D")

        ### checkIfSRContains3DPoint ###
        # We only check for FrameOfReferenceUID for 3Dpoint
        # We check for FrameOfReferenceUID, and add to the list of loadable.referencedInstanceUIDs
        # Please refer to David Clunie's paper for more information about searching for the FrameOfReferenceUIDs
        # http://dx.doi.org/10.13140/RG.2.2.34520.62725
        if checkIfSRContains3DPoint:

          # print('In checkIfSRContains3DPoint - check for FrameOfReferenceUID')
          # get the ImageRegion code
          image_region_code = self.getSRCode("Image Region")
          # First get the planar roi measurement groups
          groups = sr.content.get_planar_roi_measurement_groups(
                    graphic_type=hd.sr.GraphicTypeValues3D.POINT,
                    reference_type=codes.DCM.ImageRegion,
          )

          # Iterate through the groups, and get unique list of FrameOfReferenceUIDs
          FrameOfReferenceUIDs = []
          for group in groups:
            FrameOfReferenceUIDs.append(group.roi.frame_of_reference_uid)
          FrameOfReferenceUIDs = list(set(FrameOfReferenceUIDs))

          # Now we get the SeriesInstanceUIDs in the dicom database that have these FrameOfReferenceUIDs
          # We know that the StudyInstanceUID must be the same, so we only check those patients.
          StudyInstanceUID = dataset.StudyInstanceUID
          # Get the list of possible series in the database for this study
          SeriesInstanceUIDs = slicer.dicomDatabase.seriesForStudy(StudyInstanceUID)

          # Now make sure that the modality of these possible_SeriesInstanceUIDs are not SR or SEG.
          # Don't want them included in the list
          possible_SeriesInstanceUIDs = []
          db = slicer.dicomDatabase
          for series in SeriesInstanceUIDs:
            modality = slicer.dicomDatabase.fileValue(
              slicer.dicomDatabase.filesForSeries(series)[0], self.tags["Modality"],
            )
            if (modality != "SR") and (modality != "SEG"):
              possible_SeriesInstanceUIDs.append(series)

          # Now, we don't need to check if possible_SeriesInstanceUIDs are actually in the DICOM database
          # We know they are, because we retrieved the list of series using slicer.dicomDatabase.seriesForStudy(StudyInstanceUID)
          # However, we do need to make sure that possible_SeriesInstanceUIDs is not empty.
          # If empty, then display a popup
          if not possible_SeriesInstanceUIDs:
            logging.error("ERROR: no referenced series found in the DICOM database, cannot load any associated image data")
            slicer.util.errorDisplay("No referenced series found in the DICOM database, cannot load any associated image data")

          else:

            # Now get the FrameOfReferenceUIDs for these SeriesInstanceUIDs
            FrameOfReferenceUIDs_forSeries = []
            for SeriesInstanceUID in possible_SeriesInstanceUIDs:
              FrameOfReferenceUID_forSeries = slicer.dicomDatabase.fileValue(
                                                        slicer.dicomDatabase.filesForSeries(SeriesInstanceUID)[0], self.tags["FrameOfReferenceUID"])
              FrameOfReferenceUIDs_forSeries.append(FrameOfReferenceUID_forSeries)

            # iterate over the list of FrameOfReferenceUIDs_forSeries and see if any are in the actual SR.
            keep_SeriesInstanceUIDs = []
            for SeriesInstanceUID,FrameOfReferenceUID_forSeries in zip(possible_SeriesInstanceUIDs,FrameOfReferenceUIDs_forSeries, strict=False):
              if FrameOfReferenceUID_forSeries in FrameOfReferenceUIDs:
                keep_SeriesInstanceUIDs.append(SeriesInstanceUID)

            # Get the unique list of possible series
            keep_SeriesInstanceUIDs = list(set(keep_SeriesInstanceUIDs))

            SOPInstanceUIDs = [self.getSOPInstanceUIDsForSeries(series) for series in keep_SeriesInstanceUIDs]
            SOPInstanceUIDs = [item for sublist in SOPInstanceUIDs for item in sublist]
            # Now add to the loadable.referencedInstanceUIDs
            if (SOPInstanceUIDs):
              loadable.referencedInstanceUIDs += SOPInstanceUIDs


        #### checkIfSRContainsBbox or checkIfSRContainsPolyline ###
        # Here we get the referenced SeriesInstanceUID
        if (checkIfSRContainsBbox or checkIfSRContainsPolyline):

          # print('In checkIfSRContainsBbox/checkIfSRContainsPolyline - get referenced series')
          # Get the referenced SeriesInstanceUID
          referenced_series_instance_uid = str(sr.CurrentRequestedProcedureEvidenceSequence[0].ReferencedSeriesSequence[0].SeriesInstanceUID)
          # Now we get all of the SOPInstanceUIDs of this series
          db = slicer.dicomDatabase
          fileList = db.filesForSeries(referenced_series_instance_uid)
          if not fileList:
            logging.error("ERROR: no referenced series found in the DICOM database, cannot load any associated image data")
            slicer.util.errorDisplay("No referenced series found in the DICOM database, cannot load any associated image data")
          else:
            SOPInstanceUIDs = [db.fileValue(file, self.tags["SOPInstanceUID"]) for file in fileList]
            if (SOPInstanceUIDs):
              loadable.referencedInstanceUIDs += SOPInstanceUIDs


    return loadable

  def sortReportsByDateTime(self, uids):
    return sorted(uids, key=self.getDateTime)

  def getDateTime(self, uid):
    filename = slicer.dicomDatabase.fileForInstance(uid)
    dataset = pydicom.dcmread(filename)
    if hasattr(dataset, "SeriesDate") and hasattr(dataset, "SeriesTime"):
      date = dataset.SeriesDate
      time = dataset.SeriesTime
    elif hasattr(dataset, "StudyDate") and hasattr(dataset, "StudyTime"):
      date = dataset.StudyDate
      time = dataset.StudyDate
    else:
      date = ""
      time = ""
    try:
      dateTime = datetime.datetime.strptime(date+time, "%Y%m%d%H%M%S")
    except ValueError:
      dateTime = ""
    return dateTime

  def containsPlanarAnnotations(self, sr):
    """
    Checks if the original plugin should be used (tid1500reader) to read the SR, or if specialized
    highdicom code should be utilized for reading planar annotations, in the case of point,
    bounding box, etc.
    """

    # default to use the plugin
    containsPlanarAnnotations = False

    # get the image region code
    image_region_code = self.getSRCode("Image Region")

    planar_roi_measurement_groups = sr.content.get_planar_roi_measurement_groups()
    for planar_roi_measurement_group in planar_roi_measurement_groups:
      # Here we check if it is an Image Region
      if (planar_roi_measurement_group.reference_type == image_region_code):
          containsPlanarAnnotations = True

    return containsPlanarAnnotations

  def checkIfSRContainsGeometry(self, sr, geometry_type="bbox"):
    """Checks if the SR contains a bbox, polyline, or point3D."""

    # If SR contains a bounding box
    if (geometry_type == "bbox2D"):
      for group in sr.content.get_planar_roi_measurement_groups(reference_type=codes.DCM.ImageRegion, graphic_type=hd.sr.GraphicTypeValues.POLYLINE):
        for eval in group.get_qualitative_evaluations(
              name=self.getSRCode("Geometric purpose of region"),
        ):
            if eval.value == self.getSRCode("Bounded by"):
                return True
      return False

    # If SR contains a POLYLINE
    elif (geometry_type == "polyline2D"):
      groups = sr.content.get_planar_roi_measurement_groups(
        reference_type=self.getSRCode("Image Region"),
        graphic_type=hd.sr.GraphicTypeValues.POLYLINE,
      )
      return len(groups) >= 1

    # If SR contains SCOORD3D
    elif (geometry_type == "point3D"):
      groups = sr.content.get_planar_roi_measurement_groups(reference_type=codes.DCM.ImageRegion,graphic_type=hd.sr.GraphicTypeValues3D.POINT)
      return len(groups) >= 1

    # If SR contains unknown geometry
    else:
      logging.info("Cannot read SR with geometry: " + str(geometry_type))

    return False

  def getIPPFromSOP(self, referenced_sop_instance_uid, referenced_series_instance_uid):
    """
    In order to display the bounding box markups in Slicer, we need the IPP corresponding
    to the referenced SOPInstanceUID. We get this from the Slicer DICOM database.
    """

    # Get the dicom database
    db = slicer.dicomDatabase

    # Get the files for the series
    fileList = db.filesForSeries(referenced_series_instance_uid)

    # Use pydicom to read the files, get the SOPInstanceUID and the IPP for each
    num_files = len(fileList)
    SOPInstanceUID_list = []
    IPP_list = []
    for file in fileList:
      ds = pydicom.dcmread(file)
      SOPInstanceUID_list.append(ds.SOPInstanceUID)
      IPP_list.append(ds.ImagePositionPatient)

    # Get the index of the SOPInstanceUID we want
    index = SOPInstanceUID_list.index(referenced_sop_instance_uid)

    # Now get the IPP for the corresponding SOPInstanceUID
    ipp = IPP_list[index]

    return ipp

  def showTable(self, table):
    """Display a table in the scene."""

    currentLayout = slicer.app.layoutManager().layout
    layoutWithTable = slicer.modules.tables.logic().GetLayoutWithTable(currentLayout)
    slicer.mrmlScene.AddNode(table)
    slicer.app.layoutManager().setLayout(layoutWithTable)
    slicer.app.applicationLogic().GetSelectionNode().SetActiveTableID(table.GetID())
    slicer.app.applicationLogic().PropagateTableSelection()

    return

  def createBboxTable(self, poly_infos, table_name):
    """Create and display a table for the bbox info."""

    tableNode = slicer.vtkMRMLTableNode()
    tableNode.SetAttribute("readonly", "Yes")
    tableNode.SetName(table_name)

    # Add columns
    col = tableNode.AddColumn()
    col.SetName("Tracking Identifier")
    col = tableNode.AddColumn()
    col.SetName("FindingType")
    col = tableNode.AddColumn()
    col.SetName("FindingSite")
    col = tableNode.AddColumn()
    col.SetName("Bounding box points")

    # Order by IPP2
    poly_infos = sorted(poly_infos, key=lambda x: x["center_z"])

    for i,p in enumerate(poly_infos):
      # get values
      tracking_identifier = p["TrackingIdentifier"]
      finding_type = p["FindingType"]
      finding_site = p["FindingSite"][0] # check this later.
      polyline = p["polyline"]
      polyline_str = ", ".join(f"({np.round(a,2)}, {np.round(b,2)})" for a, b in polyline)
      width = np.round(p["width"],2)
      height = np.round(p["height"],2)
      center_x = np.round(p["center_x"],2)
      center_y = np.round(p["center_y"],2)
      center_z = np.round(p["center_z"],2)
      center = [str(center_x), str(center_y), str(center_z)]
      # add tracking info and finding site info
      rowIndex = tableNode.AddEmptyRow()
      tableNode.SetCellText(rowIndex, 0, tracking_identifier)
      tableNode.SetCellText(rowIndex, 1, finding_type[2])
      tableNode.SetCellText(rowIndex, 2, finding_site[2])
      # add bbox points
      tableNode.SetCellText(rowIndex, 3, polyline_str)

    return tableNode


  def createPointTable(self, point_infos, table_name):
    """Create and display a table for the point info."""

    tableNode = slicer.vtkMRMLTableNode()
    tableNode.SetAttribute("readonly", "Yes")
    tableNode.SetName(table_name)

    # Add columns
    col = tableNode.AddColumn()
    col.SetName("Tracking Identifier")
    col = tableNode.AddColumn()
    col.SetName("FindingType")
    col = tableNode.AddColumn()
    col.SetName("FindingSite")
    col = tableNode.AddColumn()
    col.SetName("Point")

    # Order by IPP2
    point_infos = sorted(point_infos, key=lambda x: x["point"][2])

    for i,p in enumerate(point_infos):
      # get values
      tracking_identifier = p["TrackingIdentifier"]
      tracking_uid = p["TrackingUID"]
      finding_type = p["FindingType"]
      finding_site = p["FindingSite"][0] # check this later.
      point = p["point"]
      point = [str(np.round(f,2)) for f in point]
      point_str = f"({', '.join(point)})"
      content_sequence_names = p["ContentSequenceNames"]
      content_sequence_values = p["ContentSequenceValues"]
      # add tracking info and finding site info
      rowIndex = tableNode.AddEmptyRow()
      tableNode.SetCellText(rowIndex, 0, tracking_identifier)
      tableNode.SetCellText(rowIndex, 1, finding_type[2]) # CodeMeaning
      tableNode.SetCellText(rowIndex, 2, finding_site[2]) # CodeMeaning
      # add point
      tableNode.SetCellText(rowIndex, 3, point_str)
      # now add the names CodeMeaning as the column name, and the values CodeMeaning as the actual value
      # first do for qualitative evaluations
      num_rows = 4
      # then add for content sequence
      # num_rows = num_rows + len(qual_eval_names)
      for j, content_sequence_name in enumerate(content_sequence_names):
        rowIndexValue = num_rows + j
        colName = str(content_sequence_name[2])
        colValue = str(content_sequence_values[j][2])
        # can't add column each time - if there is more than 1 point.
        # therefore, only add column when on first point_info.
        if (i==0):
          col = tableNode.AddColumn()
          col.SetName(colName)
        tableNode.SetCellText(rowIndex, rowIndexValue, colValue)


    return tableNode

  def createPolylineTable(self, point_infos, table_name):
    """Create and display a table for the polyline info."""

    tableNode = slicer.vtkMRMLTableNode()
    tableNode.SetAttribute("readonly", "Yes")
    tableNode.SetName(table_name)

    # Add columns
    col = tableNode.AddColumn()
    col.SetName("Tracking Identifier")
    col = tableNode.AddColumn()
    col.SetName("FindingType")
    col = tableNode.AddColumn()
    col.SetName("FindingSite")
    col = tableNode.AddColumn()
    col.SetName("PolyLine")

    for i,p in enumerate(point_infos):
      # get values
      tracking_identifier = p["TrackingIdentifier"]
      tracking_uid = p["TrackingUID"]
      finding_type = p["FindingType"]
      finding_site = p["FindingSite"][0] # check this later.
      polyline = p["polyline"]
      polyline = [str(np.round(f,2)) for f in polyline]
      polyline_str = f"({', '.join(polyline)})"
      # add tracking info and finding site info
      rowIndex = tableNode.AddEmptyRow()
      tableNode.SetCellText(rowIndex, 0, tracking_identifier)
      tableNode.SetCellText(rowIndex, 1, finding_type[2]) # CodeMeaning
      tableNode.SetCellText(rowIndex, 2, finding_site[2]) # CodeMeaning
      # add polyline
      tableNode.SetCellText(rowIndex, 3, polyline_str)

    return tableNode

  def extractBboxMetadataToVtkTableNode(self, sr):
    """
    Extracts the bounding box metadata from the SR using highdicom,
    and creates a table node.
    """

    # We use the SeriesNumber and the SeriesDescription of the SR to name the table
    SeriesNumber = sr.SeriesNumber
    SeriesDescription = sr.SeriesDescription
    table_name = str(SeriesNumber) + ": " + SeriesDescription

    # get the referenced SeriesInstanceUID
    referenced_series_instance_uid = str(sr.CurrentRequestedProcedureEvidenceSequence[0].ReferencedSeriesSequence[0].SeriesInstanceUID)

    # get the image_region_code
    image_region_code = self.getSRCode("Image Region")

    # will store the info needed for table too.
    poly_infos = []

    # First get the planar roi measurement groups
    groups = sr.content.get_planar_roi_measurement_groups()

    for group in groups:

      # Get the tracking ids
      tracking_identifier = group.tracking_identifier
      tracking_uid = group.tracking_uid

      # Get the findings and finding_sites
      finding_type = [group.finding_type.CodeValue, group.finding_type.CodingSchemeDesignator, group.finding_type.CodeMeaning]
      finding_sites = []
      for finding_site in group.finding_sites:
        finding_sites.append([finding_site.value.CodeValue,
                              finding_site.value.CodingSchemeDesignator,
                              finding_site.value.CodeMeaning])
      # Get the Image Region
      referenced_sop_instance_uid = group.roi.ContentSequence[0].referenced_sop_instance_uid
      bbox = group.roi.value

      # calculate the width, height and center, as these are needed for display
      min_x = np.min([bbox[0,0], bbox[1,0], bbox[2,0], bbox[3,0]]) # using roi.GraphicData: min_x = np.min([bbox[0], bbox[2], bbox[4], bbox[6]])
      max_x = np.max([bbox[0,0], bbox[1,0], bbox[2,0], bbox[3,0]]) # using roi.GraphicData: max_x = np.max([bbox[0], bbox[2], bbox[4], bbox[6]])
      min_y = np.min([bbox[0,1], bbox[1,1], bbox[2,1], bbox[3,1]]) # using roi.GraphicData: min_y = np.min([bbox[1], bbox[3], bbox[5], bbox[7]])
      max_y = np.max([bbox[0,1], bbox[1,1], bbox[2,1], bbox[3,1]]) # using roi.GraphicData: max_y = np.max([bbox[1], bbox[3], bbox[5], bbox[7]])
      width = max_x - min_x
      height = max_y - min_y
      # in pixel coordinates
      center_x = min_x + width/2
      center_y = min_y + height/2
      # get ipp
      ipp = self.getIPPFromSOP(referenced_sop_instance_uid,
                                       referenced_series_instance_uid)
      # in mm
      center_z = ipp[2]

      # append to poly_infos
      poly_infos.append({
                         "TrackingIdentifier": tracking_identifier,
                         "TrackingUID" : tracking_uid,
                         "SOPInstanceUID" : referenced_sop_instance_uid,
                         "FindingType": finding_type,
                         "FindingSite": finding_sites,
                         "polyline": bbox, # using roi.GraphicData: [[bbox[0],bbox[1]], [bbox[2],bbox[3]], [bbox[4],bbox[5]], [bbox[6],bbox[7]]],
                         "width": width,
                         "height": height,
                         "center_x": center_x,
                         "center_y": center_y,
                         "center_z": center_z,
                        })

      # create and display tableNode
      tableNode = self.createBboxTable(poly_infos, table_name)

    return poly_infos, tableNode

  def extractPointMetadataToVtkTableNode(self, sr):
    """
    Extracts the point metadata from the SR using highdicom,
    and creates a table node.
    """

    # We use the SeriesNumber and the SeriesDescription of the SR to name the table
    SeriesNumber = sr.SeriesNumber
    SeriesDescription = sr.SeriesDescription
    table_name = str(SeriesNumber) + ": " + SeriesDescription

    # get image region code
    image_region_code = self.getSRCode("Image Region")

    # will store the info needed for table too.
    point_infos = []

    # First get the planar roi measurement groups
    groups = sr.content.get_planar_roi_measurement_groups(graphic_type=hd.sr.GraphicTypeValues3D.POINT,reference_type=codes.DCM.ImageRegion)

    # Iterate through each group
    for group in groups:

      # Get the tracking ids
      tracking_identifier = group.tracking_identifier
      tracking_uid = group.tracking_uid
      # Get the finding type
      finding_type = [group.finding_type.CodeValue,
                      group.finding_type.CodingSchemeDesignator,
                      group.finding_type.CodeMeaning]
      # Get the finding sites
      finding_sites = []
      for finding_site in group.finding_sites:
        finding_sites.append([finding_site.value.CodeValue,
                              finding_site.value.CodingSchemeDesignator,
                              finding_site.value.CodeMeaning])

      # Get the point info
      referenced_frame_of_reference_uid = group.roi.ReferencedFrameOfReferenceUID
      extracted_data = group.roi.value[0]

      # Get the content items
      # if (len(group)>0):
      code_items = hd.sr.utils.find_content_items(group[0],
                                                  relationship_type=hd.sr.RelationshipTypeValues.CONTAINS,
                                                  value_type = hd.sr.ValueTypeValues.CODE)
      content_sequence_names = []
      content_sequence_values = []
      for code_item in code_items:
        # Skip image region code here, will get in a different way.
        if (code_item.name == image_region_code):
          break
        else:
          content_sequence_names.append([code_item.name.CodeValue,
                                        code_item.name.CodingSchemeDesignator,
                                        code_item.name.CodeMeaning])
          content_sequence_values.append([code_item.value.CodeValue,
                                          code_item.value.CodingSchemeDesignator,
                                          code_item.value.CodeMeaning])
      # append to poly_infos
      point_infos.append({
                         "TrackingIdentifier": tracking_identifier,
                         "TrackingUID" : tracking_uid,
                         "FindingType": finding_type,
                         "FindingSite": finding_sites,
                         "ReferencedFrameOfReferenceUID": referenced_frame_of_reference_uid,
                         "ContentSequenceNames": content_sequence_names,
                         "ContentSequenceValues": content_sequence_values,
                         "point": extracted_data,
                        })

    # create and display tableNode
    tableNode = self.createPointTable(point_infos, table_name)

    return point_infos, tableNode

  def extractLineMetadataToVtkTableNode(self, sr):
    """
    Extracts the point metadata from the SR using highdicom,
    and creates a table node.
    """

    # We use the SeriesNumber and the SeriesDescription of the SR to name the table
    SeriesNumber = sr.SeriesNumber
    SeriesDescription = sr.SeriesDescription
    table_name = str(SeriesNumber) + ": " + SeriesDescription

    # get the referenced SeriesInstanceUID
    referenced_series_instance_uid = str(sr.CurrentRequestedProcedureEvidenceSequence[0].ReferencedSeriesSequence[0].SeriesInstanceUID)

    # will store the info needed for table too.
    line_infos = []

    # First get the planar roi measurement groups
    groups = sr.content.get_planar_roi_measurement_groups(
        reference_type=self.getSRCode("Image Region"),
        graphic_type=hd.sr.GraphicTypeValues.POLYLINE,
      )

    for group in groups:

      # Get the tracking ids
      tracking_identifier = group.tracking_identifier
      tracking_uid = group.tracking_uid

      # Get the findings and finding_sites
      finding_type = [group.finding_type.CodeValue, group.finding_type.CodingSchemeDesignator, group.finding_type.CodeMeaning]
      finding_sites = []
      for finding_site in group.finding_sites:
        finding_sites.append([finding_site.value.CodeValue,
                              finding_site.value.CodingSchemeDesignator,
                              finding_site.value.CodeMeaning])

      # Get the reference
      referenced_sop_instance_uid = group.roi.ContentSequence[0].referenced_sop_instance_uid
      # Get the polyline
      polyline = group.roi.value
      # get the points
      num_points = np.int32(len(polyline))
      point_line = []
      for n in range(0,num_points):
        pointx = polyline[n,0] # pixel coord space
        pointy = polyline[n,1] # pixel coord space
        pointz = self.getIPPFromSOP(referenced_sop_instance_uid,
                                    referenced_series_instance_uid)[2] # mm space
        point_line.append([pointx, pointy, pointz])
      # append to poly_infos
      line_infos.append({
                        "TrackingIdentifier": tracking_identifier,
                        "TrackingUID" : tracking_uid,
                        "SOPInstanceUID" : referenced_sop_instance_uid,
                        "FindingType": finding_type,
                        "FindingSite": finding_sites,
                        "polyline": point_line,
                        })

    tableNode = self.createPolylineTable(line_infos, table_name)

    return line_infos, tableNode

  def create_2d_roi(self, loadable, center_ras, width, height, slice_normal=(0, 0, 1), thickness=1.0, bbox_name="2D_BoundingBox"):
    """
    Create a Markups ROI as a thin 2D bounding box on a specified slice plane.

    Args:
        loadable             - loadable of the corresponding SR
        center_ras (tuple)   - (x, y, z) center in RAS coordinates.
        width (float)        - Width of the box (in mm).
        height (float)       - Height of the box (in mm).
        slice_normal (tuple) - Normal vector of the slice plane (e.g., (0,0,1) for axial).
        thickness (float)    - Depth of the box (in mm), small for a 2D box.

    Returns:
        vtkMRMLMarkupsROINode - The ROI node created.
    """
    # Create ROI node
    roi_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsROINode", bbox_name)
    # Do not lock the node
    roi_node.SetLocked(False)

    # Set the size (width, height, thickness)
    size = [width, height, thickness]
    roi_node.SetSize(size)

    # Set the center position
    roi_node.SetCenter(center_ras)

    # Align ROI orientation to slice normal
    transform_matrix = vtk.vtkMatrix4x4()

    # Build a local coordinate system with normal as Z
    z = np.array(slice_normal)
    z = z / np.linalg.norm(z)
    x = np.cross([0, 1, 0], z)
    if np.linalg.norm(x) < 1e-3:
        x = np.cross([1, 0, 0], z)
    x = x / np.linalg.norm(x)
    y = np.cross(z, x)

    for i in range(3):
        transform_matrix.SetElement(i, 0, x[i])
        transform_matrix.SetElement(i, 1, y[i])
        transform_matrix.SetElement(i, 2, z[i])
        transform_matrix.SetElement(i, 3, center_ras[i])
    roi_node.SetAndObserveObjectToNodeMatrix(transform_matrix)

    display_node = roi_node.GetDisplayNode()
    if (display_node):
      # Change size of glyph
      display_node.SetGlyphScale(1.3)
      # Change size of interaction handles
      display_node.SetInteractionHandleScale(1)
      # Reduce the opacity of the box
      display_node.SetFillOpacity(0)

    roi_node.GetDisplayNode().SetHandlesInteractive(False)
    for controlPointIndex in range(roi_node.GetNumberOfControlPoints()):
      roi_node.SetNthControlPointLocked(controlPointIndex, True)

    return roi_node

  def displayBboxMarkups(self, sr, loadable, poly_infos):
    """Displays the bounding box markups."""

    # Get the subject hierarchy
    shNode = slicer.modules.subjecthierarchy.logic().GetSubjectHierarchyNode()

    # We use the SeriesNumber and the SeriesDescription of the SR to name the folder
    SeriesNumber = sr.SeriesNumber
    SeriesDescription = sr.SeriesDescription

    # We get the StudyInstanceUID for creating nodes in the subject hierarchy
    StudyInstanceUID = sr.StudyInstanceUID

    # Get the studyNode
    studyNode = shNode.GetItemByUID(slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMUIDName(), StudyInstanceUID)
    # Now create the folder and set the name
    bboxFolderID = shNode.CreateFolderItem(studyNode, str(SeriesNumber) + ": " + SeriesDescription)

    # Order by IPP2
    poly_infos = sorted(poly_infos, key=lambda x: x["center_z"])

    # We need the pixel spacing, in order to convert the coordinates from pixel space to mm space
    referenced_series_instance_uid = str(sr.CurrentRequestedProcedureEvidenceSequence[0].ReferencedSeriesSequence[0].SeriesInstanceUID)
    db = slicer.dicomDatabase
    fileList = db.filesForSeries(referenced_series_instance_uid)
    pixel_spacing = db.fileValue(fileList[0], self.tags["PixelSpacing"])
    pixel_spacing_x = np.float32(pixel_spacing.split("\\")[0])
    pixel_spacing_y = np.float32(pixel_spacing.split("\\")[1])
    num_rows = np.float32(db.fileValue(fileList[0], self.tags["Rows"]))
    num_columns = np.float32(db.fileValue(fileList[0], self.tags["Columns"]))

    for i,p in enumerate(poly_infos):
      # get values
      polyline = p["polyline"]
      tracking_identifier = p["TrackingIdentifier"]
      width = p["width"] # in pixel coord
      height = p["height"] # in pixel coord
      center_x = p["center_x"] # in pixel coord
      center_y = p["center_y"] # in pixel coord
      center_z = p["center_z"] # in mm
      # convert pixel coordinates to mm
      referenced_sop_instance_uid = p["SOPInstanceUID"]
      ipp = self.getIPPFromSOP(referenced_sop_instance_uid,
                               referenced_series_instance_uid)
      ipp_0 = ipp[0]
      ipp_1 = ipp[1]
      center_x_mm = -((center_x * pixel_spacing_x) + ipp_0)
      center_y_mm = -((center_y * pixel_spacing_y) + ipp_1)
      center_ras = np.asarray([center_x_mm, center_y_mm, center_z])
      width_mm = width * pixel_spacing_x
      height_mm = height * pixel_spacing_y
      bbox_name = tracking_identifier # for now
      # create roi
      bboxNode = self.create_2d_roi(loadable, center_ras, width_mm, height_mm, slice_normal=(0, 0, 1), thickness=0.01, bbox_name=bbox_name)
      markupItemID = shNode.GetItemByDataNode(bboxNode)
      # Set the parent to the folder
      shNode.SetItemParent(markupItemID, bboxFolderID)
      if (i==0):
        slicer.modules.markups.logic().JumpSlicesToLocation(center_x_mm, center_y_mm, center_z, True)

    return


  def create_3d_point(self, loadable, point_index, point_x, point_y, point_z, point_text):
    """Create the point markup"""

    markupsNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode", point_text)
    markupsNode.CreateDefaultDisplayNodes()
    # if markupsNode.SetLocked(True) - cannot move, edit, or delete points, but also cannot jump between control points.
    # if markupsNode.SetLocked(False) - can move, edit, delete and jump between control points.
    markupsNode.SetLocked(False)
    markupsNode.AddControlPoint([point_x, point_y, point_z])
    markupsNode.SetName(point_text)
    markupsNode.SetNthControlPointLabel(point_index, point_text)

    # Make sure control points are locked and cannot be moved
    markupsNode.GetDisplayNode().SetHandlesInteractive(False)
    for controlPointIndex in range(markupsNode.GetNumberOfControlPoints()):
      markupsNode.SetNthControlPointLocked(controlPointIndex, True)

    return markupsNode

  def displayPointMarkups(self, sr, loadable, point_infos):
    """Display the point markups."""

    # Get the subject hierarchy
    shNode = slicer.modules.subjecthierarchy.logic().GetSubjectHierarchyNode()

    # We use the SeriesNumber and the SeriesDescription of the SR to name the folder
    SeriesNumber = sr.SeriesNumber
    SeriesDescription = sr.SeriesDescription

    # We get the StudyInstanceUID for creating nodes in the subject hierarchy
    StudyInstanceUID = sr.StudyInstanceUID

    # Get the studyNode
    studyNode = shNode.GetItemByUID(slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMUIDName(), StudyInstanceUID)
    # Now create the folder and set the name
    pointsFolderID = shNode.CreateFolderItem(studyNode, str(SeriesNumber) + ": " + SeriesDescription)

    for i,p in enumerate(point_infos):
      point_text = p["TrackingIdentifier"]
      point_x = -p["point"][0]
      point_y = -p["point"][1]
      point_z = p["point"][2]
      # index should always be 0. not i.
      pointsNode = self.create_3d_point(loadable, 0, point_x, point_y, point_z, point_text)
      # change size of glyph
      display_node = pointsNode.GetDisplayNode()
      if (display_node):
        display_node.SetGlyphScale(0.75)

      markupItemID = shNode.GetItemByDataNode(pointsNode)
      # Set the parent to the folder
      shNode.SetItemParent(markupItemID, pointsFolderID)
      # jump to the first point
      if (i==0):
        slicer.modules.markups.logic().JumpSlicesToLocation(point_x, point_y, point_z, True)

    return

  def displayLineMarkups(self, sr, loadable, line_infos):
    """Display the line markups."""

    # Get the subject hierarchy
    shNode = slicer.modules.subjecthierarchy.logic().GetSubjectHierarchyNode()

    # We use the SeriesNumber and the SeriesDescription of the SR to name the folder
    SeriesNumber = sr.SeriesNumber
    SeriesDescription = sr.SeriesDescription

    # We get the StudyInstanceUID for creating nodes in the subject hierarchy
    StudyInstanceUID = sr.StudyInstanceUID

    # Get the studyNode
    studyNode = shNode.GetItemByUID(slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMUIDName(), StudyInstanceUID)
    # Now create the folder and set the name
    linesFolderID = shNode.CreateFolderItem(studyNode, str(SeriesNumber) + ": " + SeriesDescription)

    # Get the referenced series instance uid
    # Needed for later getting the IPP
    referenced_series_instance_uid = str(sr.CurrentRequestedProcedureEvidenceSequence[0].ReferencedSeriesSequence[0].SeriesInstanceUID)
    db = slicer.dicomDatabase
    fileList = db.filesForSeries(referenced_series_instance_uid)
    pixel_spacing = db.fileValue(fileList[0], self.tags["PixelSpacing"])
    pixel_spacing_x = np.float32(pixel_spacing.split("\\")[0])
    pixel_spacing_y = np.float32(pixel_spacing.split("\\")[1])

    # Create all the line nodes
    for i,p in enumerate(line_infos):
      line_text = p["TrackingIdentifier"]
      polyline = p["polyline"]
      # add new node
      lineNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsLineNode", line_text)
      # Do not lock node
      lineNode.SetLocked(False)
      # get number of points
      num_points = len(polyline)
      # add each as a control point
      for n in range(0,num_points):
        point_x = polyline[n][0] # pixel coord space
        point_y = polyline[n][1] # pixel coord space
        # convert pixel coordinates to mm
        referenced_sop_instance_uid = p["SOPInstanceUID"]
        ipp = self.getIPPFromSOP(referenced_sop_instance_uid,
                                 referenced_series_instance_uid)
        ipp_0 = ipp[0]
        ipp_1 = ipp[1]
        point_x_mm = -((point_x * pixel_spacing_x) + ipp_0)
        point_y_mm = -((point_y * pixel_spacing_y) + ipp_1)
        point_z = polyline[n][2]
        lineNode.AddControlPoint(point_x_mm, point_y_mm, point_z)
        # jump to the first line
        if (i==0 and n==0):
          slicer.modules.markups.logic().JumpSlicesToLocation(point_x_mm, point_y_mm, point_z, True)
      # do not display the length measurement
      lineNode.GetMeasurement("length").SetEnabled(False)
      # change size of glyph
      display_node = lineNode.GetDisplayNode()
      # lock the display
      lineNode.GetDisplayNode().SetHandlesInteractive(False)
      for controlPointIndex in range(lineNode.GetNumberOfControlPoints()):
          lineNode.SetNthControlPointLocked(controlPointIndex, True)

      if (display_node):
        display_node.SetGlyphScale(0.75)
      # Add to subject hierarchy
      # Get the Subject Hierarchy item ID for the markup node
      markupItemID = shNode.GetItemByDataNode(lineNode)
      # Set the parent to the folder
      shNode.SetItemParent(markupItemID, linesFolderID)

    return

  def load(self, loadable):
    """Loads the SR and checks for planar annotations."""

    logging.debug("DICOM SR TID1500 load()")

    logging.debug("before sorting: %s" % loadable.uids)
    sortedUIDs = self.sortReportsByDateTime(loadable.uids)
    logging.debug("after sorting: %s" % sortedUIDs)

    # if there is a RWVM object referenced from SEG, assume it contains the
    # scaling that needs to be applied to the referenced series. Assign
    # referencedSeriesUID from the image series, but load using the RWVM plugin
    segPlugin = slicer.modules.dicomPlugins["DICOMSegmentationPlugin"]()

    ### Iterate over the uids ###
    for idx, uid in enumerate(sortedUIDs):

      # Get SR filename
      srFileName = slicer.dicomDatabase.fileForInstance(uid)
      if srFileName is None:
        logging.debug("Failed to get the filename from the DICOM database for %s", uid)
        return False

      # Read the SR once
      sr = hd.sr.srread(srFileName)

      # Check if the plugin (tid1500reader) should be used or specialized highdicom code
      # to read the planar annotations
      # sets the self.containsPlanarAnnotations = True or False
      containsPlanarAnnotations = self.containsPlanarAnnotations(sr)
      logging.info("containsPlanarAnnotations: " + str(containsPlanarAnnotations))

      ### If it does not contain planar annotations ###
      if not containsPlanarAnnotations:

        tables = []

        for segSeriesInstanceUID in loadable.ReferencedSegmentationInstanceUIDs[uid]:
          segLoadables = segPlugin.examine([slicer.dicomDatabase.filesForSeries(segSeriesInstanceUID)])
          for segLoadable in segLoadables:
            if hasattr(segLoadable, "referencedSegInstanceUIDs"):
              segLoadable.referencedSegInstanceUIDs = list(set(segLoadable.referencedSegInstanceUIDs) -
                                                          set(loadable.referencedInstanceUIDs))
            segPlugin.load(segLoadable)
            if hasattr(segLoadable, "referencedSeriesUID") and len(loadable.ReferencedRWVMSeriesInstanceUIDs) > 0:
              self.determineAndApplyRWVMToReferencedSeries(loadable, segLoadable)

        self.tempDir = os.path.join(slicer.app.temporaryPath, "QIICR", "SR", self.currentDateTime, uid)
        try:
          os.makedirs(self.tempDir)
        except OSError:
          pass

        outputFile = os.path.join(self.tempDir, uid+".json")

        param = {
          "inputSRFileName": srFileName,
          "metaDataFileName": outputFile,
          }

        result = subprocess.run(
            [self._dcmqi_binary("tid1500reader"),
             "--inputDICOM", param["inputSRFileName"],
             "--outputMetadata", param["metaDataFileName"]],
            capture_output=True, text=True, check=False)
        if result.returncode != 0:
          logging.debug("tid1500reader failed, unable to load DICOM SR TID1500:\n" + result.stderr)
          self.cleanup()
          return False

        table = self.metadata2vtkTableNode(outputFile)
        if table:
          self.addSeriesInSubjectHierarchy(loadable, table)
          with open(outputFile) as srMetadataFile:
            srMetadataJSON = json.loads(srMetadataFile.read())
            table.SetName(srMetadataJSON["SeriesDescription"])

          # TODO: think about the following...
          if len(slicer.util.getNodesByClass("vtkMRMLSegmentationNode")) > 0:
            segmentationNode = slicer.util.getNodesByClass("vtkMRMLSegmentationNode")[-1]
            segmentationNodeID = segmentationNode.GetID()
            table.SetAttribute("ReferencedSegmentationNodeID", segmentationNodeID)

            # TODO: think about a better solution for finding related reports
            if idx-1 > -1:
              table.SetAttribute("PriorReportUID", sortedUIDs[idx-1])
              tables[idx-1].SetAttribute("FollowUpReportUID", uid)
            table.SetAttribute("SOPInstanceUID", uid)
            self.assignTrackingUniqueIdentifier(outputFile, segmentationNode)

        tables.append(table)

        self.loadAdditionalMeasurements(uid, loadable)

        self.cleanup()

      ### If it does contains planar annotations - use highdicom code to read the SR ###
      else:

        # check if contains a point, bbox, polyline
        checkIfSRContainsBbox = self.checkIfSRContainsGeometry(sr, geometry_type="bbox2D")
        checkIfSRContains3DPoint = self.checkIfSRContainsGeometry(sr, geometry_type="point3D")
        checkIfSRContainsPolyline = self.checkIfSRContainsGeometry(sr, geometry_type="polyline2D")

        tables = []

        # if bbox
        if (checkIfSRContainsBbox):
          logging.info("SR contains 2D bounding box")
          bboxInfo, bboxTableNode = self.extractBboxMetadataToVtkTableNode(sr)
          self.showTable(bboxTableNode)
          self.addSeriesInSubjectHierarchy(loadable, bboxTableNode)
          self.displayBboxMarkups(sr, loadable, bboxInfo)
          tables.append(bboxTableNode)

        # if point
        if (checkIfSRContains3DPoint):
            logging.info("SR contains 3D point")
            pointInfo, pointTableNode = self.extractPointMetadataToVtkTableNode(sr)
            self.showTable(pointTableNode)
            self.addSeriesInSubjectHierarchy(loadable, pointTableNode)
            self.displayPointMarkups(sr, loadable, pointInfo)
            tables.append(pointTableNode)

        # if polyline but not bbox
        if (checkIfSRContainsPolyline and not checkIfSRContainsBbox):
            logging.info("SR contains a 2D polyline, but not a bbox")
            lineInfo, lineTableNode = self.extractLineMetadataToVtkTableNode(sr)
            self.showTable(lineTableNode)
            self.addSeriesInSubjectHierarchy(loadable, lineTableNode)
            self.displayLineMarkups(sr, loadable, lineInfo)
            tables.append(lineTableNode)

    return len(tables) > 0

  def getSegmentIDs(self, segmentationNode):
    segmentIDs = vtk.vtkStringArray()
    segmentation = segmentationNode.GetSegmentation()
    segmentation.GetSegmentIDs(segmentIDs)
    return [segmentIDs.GetValue(idx) for idx in range(segmentIDs.GetNumberOfValues())]

  def assignTrackingUniqueIdentifier(self, metafile, segmentationNode):

    with open(metafile) as datafile:
      data = json.load(datafile)

      segmentation = segmentationNode.GetSegmentation()
      segments = [segmentation.GetSegment(segmentID) for segmentID in self.getSegmentIDs(segmentationNode)]

      for idx, measurement in enumerate(data["Measurements"]):
        tagName = "TrackingUniqueIdentifier"
        trackingUID = measurement[tagName]
        segment = segments[idx]
        segment.SetTag(tagName, trackingUID)
        logging.debug("Setting tag '{}' to {} for segment with name {}".format(tagName, trackingUID, segment.GetName()))

  def determineAndApplyRWVMToReferencedSeries(self, loadable, segLoadable):
    rwvmUID = loadable.ReferencedRWVMSeriesInstanceUIDs[0]
    logging.debug("Looking up series %s from database" % rwvmUID)
    rwvmFiles = slicer.dicomDatabase.filesForSeries(rwvmUID)
    if len(rwvmFiles) > 0:
      # consider only the first item on the list - there should be only
      # one anyway, for the cases we are handling at the moment
      rwvmPlugin = slicer.modules.dicomPlugins["DICOMRWVMPlugin"]()
      rwvmFile = rwvmFiles[0]
      logging.debug("Reading RWVM from " + rwvmFile)
      rwvmDataset = pydicom.dcmread(rwvmFile)
      if hasattr(rwvmDataset, "ReferencedSeriesSequence"):
        if hasattr(rwvmDataset.ReferencedSeriesSequence[0], "SeriesInstanceUID"):
          if rwvmDataset.ReferencedSeriesSequence[0].SeriesInstanceUID == segLoadable.referencedSeriesUID:
            logging.debug("SEG references the same image series that is referenced by the RWVM series referenced from "
                          "SR. Will load via RWVM.")
            logging.debug("Examining " + rwvmFile)
            rwvmLoadables = rwvmPlugin.examine([[rwvmFile]])
            rwvmPlugin.load(rwvmLoadables[0])
    else:
      logging.warning("RWVM is referenced from SR, but is not found in the DICOM database!")

  def metadata2vtkTableNode(self, metafile):
    with open(metafile) as datafile:
      data = json.load(datafile)
      if "Measurements" not in data:
        # Invalid file, just return instead of throw an exception to allow loading
        # other data.
        return None

      measurement = data["Measurements"][0]

      table = self.createAndConfigureTable()

      tableWasModified = table.StartModify()
      self.setupTableInformation(measurement, table)
      self.addMeasurementsToTable(data, table)
      table.EndModify(tableWasModified)

      slicer.app.applicationLogic().GetSelectionNode().SetReferenceActiveTableID(table.GetID())
      slicer.app.applicationLogic().PropagateTableSelection()
    return table

  def addMeasurementsToTable(self, data, table):
    for measurement in data["Measurements"]:
      name = measurement["TrackingIdentifier"]
      rowIndex = table.AddEmptyRow()
      table.SetCellText(rowIndex, 0, name)
      for columnIndex, measurementItem in enumerate(measurement["measurementItems"]):
        table.SetCellText(rowIndex, columnIndex + 1, measurementItem["value"])

  def createAndConfigureTable(self):
    table = slicer.vtkMRMLTableNode()
    slicer.mrmlScene.AddNode(table)
    table.SetAttribute("QuantitativeReporting", "Yes")
    table.SetAttribute("readonly", "Yes")
    table.SetUseColumnNameAsColumnHeader(True)
    return table

  def setupTableInformation(self, measurement, table):
    col = table.AddColumn()
    col.SetName("Tracking Identifier")

    infoItems = self.enumerateDuplicateNames(self.generateMeasurementInformation(measurement["measurementItems"]))

    for info in infoItems:
      col = table.AddColumn()
      col.SetName(info["name"])
      table.SetColumnLongName(info["name"], info["name"])
      table.SetColumnUnitLabel(info["name"], info["unit"])
      table.SetColumnDescription(info["name"], info["description"])

  def generateMeasurementInformation(self, measurementItems):
    infoItems = []
    for measurementItem in measurementItems:

      crntInfo = dict()

      unit = measurementItem["units"]["CodeValue"]
      crntInfo["unit"] = measurementItem["units"]["CodeMeaning"]

      if "derivationModifier" in measurementItem.keys():
        description = crntInfo["name"] = measurementItem["derivationModifier"]["CodeMeaning"]
      else:
        description = measurementItem["quantity"]["CodeMeaning"]

      crntInfo["name"] = "{} [{}]".format(description, unit.replace("[", "").replace("]", ""))
      crntInfo["description"] = description

      infoItems.append(crntInfo)
    return infoItems

  def enumerateDuplicateNames(self, items):
    names = [item["name"] for item in items]
    counts = {k: v for k, v in Counter(names).items() if v > 1}
    nameListCopy = names[:]

    for i in reversed(range(len(names))):
      item = names[i]
      if counts.get(item):
        nameListCopy[i] += " (%s)" % str(counts[item])
        counts[item] -= 1

    for idx, item in enumerate(nameListCopy):
      items[idx]["name"] = item
    return items

  def isConcept(self, item, coding):
    code = item.ConceptNameCodeSequence[0]
    return code.CodingSchemeDesignator == self.codings[coding]["scheme"] and code.CodeValue == self.codings[coding]["value"]

  def loadAdditionalMeasurements(self, srUID, loadable):
    """
    Loads length measements as annotation rulers
    TODO: need to generalize to other report contents
    """
    srFilePath = slicer.dicomDatabase.fileForInstance(srUID)
    sr = pydicom.dcmread(srFilePath)

    if not self.isConcept(sr, "imagingMeasurementReport"):
      return sr

    contents = {}
    measurements = []
    contents["measurements"] = measurements
    for item in sr.ContentSequence:
      if self.isConcept(item, "personObserver"):
        contents["personObserver"] = item.PersonName
      if self.isConcept(item, "imagingMeasurements"):
        for contentItem in item.ContentSequence:
          if self.isConcept(contentItem, "measurementGroup"):
            measurement = {}
            for measurementItem in contentItem.ContentSequence:
              if self.isConcept(measurementItem, "trackingIdentifier"):
                measurement["trackingIdentifier"] = measurementItem.TextValue
              if self.isConcept(measurementItem, "trackingUniqueIdentifier"):
                measurement["trackingUniqueIdentifier"] = measurementItem.UID
              if self.isConcept(measurementItem, "findingSite"):
                measurement["findingSite"] = measurementItem.ConceptCodeSequence[0].CodeMeaning
              if self.isConcept(measurementItem, "length"):
                for lengthItem in measurementItem.ContentSequence:
                  measurement["polyline"] = lengthItem.GraphicData
                  for selectionItem in lengthItem.ContentSequence:
                    if selectionItem.RelationshipType == "SELECTED FROM":
                      for reference in selectionItem.ReferencedSOPSequence:
                        measurement["referencedSOPInstanceUID"] = reference.ReferencedSOPInstanceUID
                        if hasattr(reference, "ReferencedFrameNumber") and reference.ReferencedFrameNumber != "1":
                          print("Error - only single frame references supported")
            measurements.append(measurement)

    for measurement in contents["measurements"]:
      if not "polyline" in measurement:
        # only polyline measurements are loaded as nodes
        continue

      markupsNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsLineNode")
      markupsNode.SetName(str(contents["personObserver"]))
      self.addSeriesInSubjectHierarchy(loadable, markupsNode)

      # Instead of calling markupsNode.SetLocked(True), lock each control point.
      # This allows interacting with the points but not change their position.
      slicer.modules.markups.logic().SetAllMarkupsLocked(markupsNode, True)

      colorIndex = 1 + slicer.mrmlScene.GetNumberOfNodesByClass("vtkMRMLMarkupsLineNode")
      colorNode = slicer.mrmlScene.GetNodeByID("vtkMRMLColorTableNodeFileGenericAnatomyColors.txt")
      color = numpy.zeros(4)
      colorNode.GetColor(colorIndex, color)
      markupsNode.GetDisplayNode().SetSelectedColor(*color[:3])

      referenceFilePath = slicer.dicomDatabase.fileForInstance(measurement["referencedSOPInstanceUID"])
      if not referenceFilePath:
        raise Exception(f"Referenced image is not found in the database (referencedSOPInstanceUID={measurement['referencedSOPInstanceUID']}). Polyline point positions cannot be determined in 3D.")

      reference = pydicom.dcmread(referenceFilePath)
      origin = numpy.array(reference.ImagePositionPatient)
      alongColumnVector = numpy.array(reference.ImageOrientationPatient[:3])
      alongRowVector = numpy.array(reference.ImageOrientationPatient[3:])
      alongColumnVector *= reference.PixelSpacing[1]
      alongRowVector *= reference.PixelSpacing[0]
      col1,row1,col2,row2 = measurement["polyline"]
      lpsToRAS = numpy.array([-1,-1,1])
      p1 = (origin + col1 * alongColumnVector + row1 * alongRowVector) * lpsToRAS
      p2 = (origin + col2 * alongColumnVector + row2 * alongRowVector) * lpsToRAS
      markupsNode.AddControlPoint(vtk.vtkVector3d(p1))
      markupsNode.AddControlPoint(vtk.vtkVector3d(p2))

class DICOMLongitudinalTID1500PluginClass(DICOMTID1500PluginClass):

  def __init__(self):
    super().__init__()
    self.loadType = "Longitudinal DICOM Structured Report TID1500"

  def examineFiles(self, files):
    loadables = []

    for cFile in files:
      dataset = pydicom.dcmread(cFile)

      uid = self.getDICOMValue(dataset, "SOPInstanceUID")
      if uid == "":
        return []

      if self.isDICOMTID1500(dataset):
        otherSRDatasets, otherSRFiles = self.getRelatedSRs(dataset)

        if len(otherSRFiles):
          allDatasets = otherSRDatasets + [dataset]
          loadable = self.createLoadableAndAddReferences(allDatasets)
          loadable.files = [cFile]+otherSRFiles
          seriesDescription = self.getDICOMValue(dataset, "SeriesDescription", "Unknown")
          loadable.name = seriesDescription + " - as a Longitudinal DICOM SR TID1500 object"
          loadable.tooltip = loadable.name
          loadable.selected = True
          loadable.confidence = 0.96
          loadable.uids = [self.getDICOMValue(d, "SOPInstanceUID") for d in allDatasets]
          refName = self.referencedSeriesName(loadable)
          if refName != "":
            loadable.name = refName + " " + seriesDescription + " - SR TID1500"

          loadables.append(loadable)

          logging.debug("DICOM SR Longitudinal TID1500 modality found")

    return loadables

  def getRelatedSRs(self, dataset):
    otherSRFiles = []
    otherSRDatasets = []
    studyInstanceUID = self.getDICOMValue(dataset, "StudyInstanceUID")
    patient = slicer.dicomDatabase.patientForStudy(studyInstanceUID)
    studies = [s for s in slicer.dicomDatabase.studiesForPatient(patient) if studyInstanceUID not in s]
    for study in studies:
      series = slicer.dicomDatabase.seriesForStudy(study)
      foundSRs = []
      for s in series:
        srFile = self.fileForSeries(s)
        tempDCM = pydicom.dcmread(srFile)
        if self.isDICOMTID1500(tempDCM):
          foundSRs.append(srFile)
          otherSRDatasets.append(tempDCM)

      if len(foundSRs) > 1:
        logging.warn("Found more than one SR per study!! This is not supported right now")
      otherSRFiles += foundSRs
    return otherSRDatasets, otherSRFiles

  def fileForSeries(self, series):
    instance = slicer.dicomDatabase.instancesForSeries(series)
    return slicer.dicomDatabase.fileForInstance(instance[0])


class DICOMTID1500Plugin:
  """
  This class is the 'hook' for slicer to detect and recognize the plugin
  as a loadable scripted module
  """

  def __init__(self, parent):
    parent.title = "DICOM SR TID1500 Object Import Plugin"
    parent.categories = ["Developer Tools.DICOM Plugins"]
    parent.contributors = ["Christian Herz (BWH), Andrey Fedorov (BWH)"]
    parent.helpText = """
    Plugin to the DICOM Module to parse and load DICOM SR TID1500 instances.
    No module interface here, only in the DICOM module
    """
    parent.dependencies = ["DICOM", "Colors"]
    parent.acknowledgementText = """
    This DICOM Plugin was developed by
    Christian Herz and Andrey Fedorov, BWH.
    and was partially funded by NIH grant U24 CA180918 (QIICR).
    """

    # Add this extension to the DICOM module's list for discovery when the module
    # is created.  Since this module may be discovered before DICOM itself,
    # create the list if it doesn't already exist.
    try:
      slicer.modules.dicomPlugins
    except AttributeError:
      slicer.modules.dicomPlugins = {}
    slicer.modules.dicomPlugins["DICOMTID1500Plugin"] = DICOMTID1500PluginClass
    # slicer.modules.dicomPlugins['DICOMLongitudinalTID1500Plugin'] = DICOMLongitudinalTID1500PluginClass

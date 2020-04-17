import os
import string
from __main__ import vtk, qt, ctk, slicer
import logging
import numpy
if slicer.app.majorVersion >= 5 or (slicer.app.majorVersion == 4 and slicer.app.minorVersion >= 11):
  import pydicom
else:
  import dicom
from DICOMLib import DICOMPlugin
from DICOMLib import DICOMLoadable
from DICOMLib import DICOMExportScalarVolume

#
# This plugin can export sequence node that contains volumes
# to DICOM, using DICOM module's plugin infrastructure.
#

class DICOMVolumeSequencePluginClass(DICOMPlugin):
  """ Volume sequence export plugin
  """

  def __init__(self):
    super(DICOMVolumeSequencePluginClass,self).__init__()
    self.loadType = "Volume Sequence"

    self.tags['studyID'] = '0020,0010'
    self.tags['seriesDescription'] = "0008,103e"
    self.tags['seriesUID'] = "0020,000E"
    self.tags['seriesNumber'] = "0020,0011"
    self.tags['seriesDate'] = "0008,0021"
    self.tags['seriesTime'] = "0020,0031"
    self.tags['position'] = "0020,0032"
    self.tags['orientation'] = "0020,0037"
    self.tags['pixelData'] = "7fe0,0010"
    self.tags['seriesInstanceUID'] = "0020,000E"
    self.tags['contentTime'] = "0008,0033"
    self.tags['triggerTime'] = "0018,1060"
    self.tags['diffusionGradientOrientation'] = "0018,9089"
    self.tags['imageOrientationPatient'] = "0020,0037"
    self.tags['numberOfFrames'] = "0028,0008"
    self.tags['instanceUID'] = "0008,0018"
    self.tags['windowCenter'] = "0028,1050"
    self.tags['windowWidth'] = "0028,1051"
    self.tags['classUID'] = "0008,0016"

  def getSequenceBrowserNodeForMasterOutputNode(self, masterOutputNode):
    browserNodes = slicer.mrmlScene.GetNodesByClass('vtkMRMLSequenceBrowserNode')
    browserNodes.UnRegister(None)
    for itemIndex in range(browserNodes.GetNumberOfItems()):
      sequenceBrowserNode = browserNodes.GetItemAsObject(itemIndex)
      if sequenceBrowserNode.GetProxyNode(sequenceBrowserNode.GetMasterSequenceNode()) == masterOutputNode:
        return sequenceBrowserNode
    return None

  def examineForExport(self, subjectHierarchyItemID):
    """Return a list of DICOMExportable instances that describe the
    available techniques that this plugin offers to convert MRML
    data into DICOM data
    """

    # Check if setting of DICOM UIDs is suported (if not, then we cannot export to sequence)
    dicomUIDSettingSupported = False
    createDicomSeriesParameterNode = slicer.modules.createdicomseries.cliModuleLogic().CreateNode()
    for groupIndex in range(createDicomSeriesParameterNode.GetNumberOfParameterGroups()):
      if createDicomSeriesParameterNode.GetParameterGroupLabel(groupIndex) == "Unique Identifiers (UIDs)":
        dicomUIDSettingSupported = True
    if not dicomUIDSettingSupported:
      # This version of Slicer does not allow setting DICOM UIDs for export
      return []

    # cannot export if there is no data node or the data node is not a volume
    shn = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
    dataNode = shn.GetItemDataNode(subjectHierarchyItemID)
    if dataNode is None or not dataNode.IsA('vtkMRMLScalarVolumeNode'):
      # not a volume node
      return []

    sequenceBrowserNode = self.getSequenceBrowserNodeForMasterOutputNode(dataNode)
    if not sequenceBrowserNode:
      # this seems to be a simple volume node (not a proxy node of a volume
      # sequence). This plugin only deals with volume sequences.
      return []

    sequenceItemCount = sequenceBrowserNode.GetMasterSequenceNode().GetNumberOfDataNodes()
    if sequenceItemCount <= 1:
      # this plugin is only relevant if there are multiple items in the sequence
      return []

    # Define basic properties of the exportable
    exportable = slicer.qSlicerDICOMExportable()
    exportable.name = self.loadType
    exportable.tooltip = "Creates a series of DICOM files from volume sequences"
    exportable.subjectHierarchyItemID = subjectHierarchyItemID
    exportable.pluginClass = self.__module__
    exportable.confidence = 0.6 # Simple volume has confidence of 0.5, use a slightly higher value here

    # Define required tags and default values
    exportable.setTag('SeriesDescription', 'Volume sequence of {0} frames'.format(sequenceItemCount))
    exportable.setTag('Modality', 'CT')
    exportable.setTag('Manufacturer', 'Unknown manufacturer')
    exportable.setTag('Model', 'Unknown model')
    exportable.setTag('StudyID', '1')
    exportable.setTag('SeriesNumber', '1')
    exportable.setTag('SeriesDate', '')
    exportable.setTag('SeriesTime', '')

    return [exportable]

  def datetimeFromDicom(self, dt, tm):
    year = 0
    month = 0
    day = 0
    if len(dt)==8: # YYYYMMDD
      year = int(dt[0:4])
      month = int(dt[4:6])
      day = int(dt[6:8])
    else:
      raise IOError("Invalid DICOM date string: "+tm+" (failed to parse YYYYMMDD)")

    hour = 0
    minute = 0
    second = 0
    microsecond = 0
    if len(tm)>=6:
      try:
        hhmmss = string.split(tm,'.')[0]
      except:
        hhmmss = tm
      try:
        microsecond = int(float('0.'+string.split(tm,'.')[1]) * 1e6)
      except:
        microsecond = 0
      if len(hhmmss)==6: # HHMMSS
        hour = int(hhmmss[0:2])
        minute = int(hhmmss[2:4])
        second = int(hhmmss[4:6])
      elif len(hhmmss)==4: # HHMM
        hour = int(hhmmss[0:2])
        minute = int(hhmmss[2:4])
      elif len(hhmmss)==2: # HH
        hour = int(hhmmss[0:2])
      else:
        raise IOError("Invalid DICOM time string: "+tm+" (failed to parse HHMMSS)")

    import datetime
    return datetime.datetime(year, month, day, hour, minute, second, microsecond)

  def export(self,exportables):
    for exportable in exportables:
      # Get volume node to export
      shNode = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
      if shNode is None:
        error = "Invalid subject hierarchy"
        logging.error(error)
        return error
      volumeNode = shNode.GetItemDataNode(exportable.subjectHierarchyItemID)
      if volumeNode is None or not volumeNode.IsA('vtkMRMLScalarVolumeNode'):
        error = "Series '" + shNode.GetItemName(exportable.subjectHierarchyItemID) + "' cannot be exported  as volume sequence"
        logging.error(error)
        return error

      sequenceBrowserNode = self.getSequenceBrowserNodeForMasterOutputNode(volumeNode)
      if not sequenceBrowserNode:
        error = "Series '" + shNode.GetItemName(exportable.subjectHierarchyItemID) + "' cannot be exported as volume sequence"
        logging.error(error)
        return error

      volumeSequenceNode =  sequenceBrowserNode.GetSequenceNode(volumeNode)
      if not volumeSequenceNode:
        error = "Series '" + shNode.GetItemName(exportable.subjectHierarchyItemID) + "' cannot be exported as volume sequence"
        logging.error(error)
        return error

      # Get study and patient items
      studyItemID = shNode.GetItemParent(exportable.subjectHierarchyItemID)
      if not studyItemID:
        error = "Unable to get study for series '" + volumeNode.GetName() + "'"
        logging.error(error)
        return error
      patientItemID = shNode.GetItemParent(studyItemID)
      if not patientItemID:
        error = "Unable to get patient for series '" + volumeNode.GetName() + "'"
        logging.error(error)
        return error

      # Assemble tags dictionary for volume export

      tags = {}
      tags['Patient Name'] = exportable.tag(slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMPatientNameTagName())
      tags['Patient ID'] = exportable.tag(slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMPatientIDTagName())
      tags['Patient Comments'] = exportable.tag(slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMPatientCommentsTagName())
      if slicer.app.majorVersion >= 5 or (slicer.app.majorVersion == 4 and slicer.app.minorVersion >= 11):
        tags['Study Instance UID'] = pydicom.uid.generate_uid()
      else:
        tags['Study Instance UID'] = dicom.UID.generate_uid()
      tags['Study ID'] = exportable.tag(slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMStudyIDTagName())
      tags['Study Date'] = exportable.tag(slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMStudyDateTagName())
      tags['Study Time'] = exportable.tag(slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMStudyTimeTagName())
      tags['Study Description'] = exportable.tag(slicer.vtkMRMLSubjectHierarchyConstants.GetDICOMStudyDescriptionTagName())
      tags['Modality'] = exportable.tag('Modality')
      tags['Manufacturer'] = exportable.tag('Manufacturer')
      tags['Model'] = exportable.tag('Model')
      tags['Series Description'] = exportable.tag('SeriesDescription')
      tags['Series Number'] = exportable.tag('SeriesNumber')
      tags['Series Date'] = exportable.tag("SeriesDate")
      tags['Series Time'] = exportable.tag("SeriesTime")
      if slicer.app.majorVersion >= 5 or (slicer.app.majorVersion == 4 and slicer.app.minorVersion >= 11):
        tags['Series Instance UID'] = pydicom.uid.generate_uid()
        tags['Frame of Reference Instance UID'] = pydicom.uid.generate_uid()
      else:
        tags['Series Instance UID'] = dicom.UID.generate_uid()
        tags['Frame of Reference Instance UID'] = dicom.UID.generate_uid()

      # Validate tags
      if tags['Modality'] == "":
        error = "Empty modality for series '" + volumeNode.GetName() + "'"
        logging.error(error)
        return error
      #TODO: more tag checks

      sequenceItemCount = sequenceBrowserNode.GetMasterSequenceNode().GetNumberOfDataNodes()
      originalSelectedSequenceItemNumber = sequenceBrowserNode.GetSelectedItemNumber()
      masterVolumeNode = sequenceBrowserNode.GetMasterSequenceNode()

      # initialize content datetime from series datetime
      contentStartDate = exportable.tag("SeriesDate")
      contentStartTime = exportable.tag("SeriesTime")
      import datetime
      datetimeNow = datetime.datetime.now()
      if not contentStartDate:
        contentStartDate = datetimeNow.strftime("%Y%m%d")
      if not contentStartTime:
        contentStartTime = datetimeNow.strftime("%H%M%S.%f")
      contentStartDatetime = self.datetimeFromDicom(contentStartDate, contentStartTime)

      # Get output directory and create a subdirectory. This is necessary
      # to avoid overwriting the files in case of multiple exportables, as
      # naming of the DICOM files is static
      directoryName = 'VolumeSequence_' + str(exportable.subjectHierarchyItemID)
      directoryDir = qt.QDir(exportable.directory)
      directoryDir.mkdir(directoryName)
      directoryDir.cd(directoryName)
      directory = directoryDir.absolutePath()
      logging.info("Export scalar volume '" + volumeNode.GetName() + "' to directory " + directory)

      for sequenceItemIndex in range(sequenceItemCount):

        # Switch to next item in the series
        sequenceBrowserNode.SetSelectedItemNumber(sequenceItemIndex)
        slicer.app.processEvents()
        # Compute content date&time
        # TODO: verify that unit in sequence node is "second" (and convert to seconds if not)
        timeOffsetSec = float(masterVolumeNode.GetNthIndexValue(sequenceItemIndex))-float(masterVolumeNode.GetNthIndexValue(0))
        contentDatetime = contentStartDatetime + datetime.timedelta(seconds=timeOffsetSec)
        tags['Content Date'] = contentDatetime.strftime("%Y%m%d")
        tags['Content Time'] = contentDatetime.strftime("%H%M%S.%f")
        # Perform export
        filenamePrefix = "IMG_{0:04d}_".format(sequenceItemIndex)
        exporter = DICOMExportScalarVolume(tags['Study ID'], volumeNode, tags, directory, filenamePrefix)
        exporter.export()

    # Success
    return ""

#
# DICOMVolumeSequencePlugin
#

class DICOMVolumeSequencePlugin:
  """
  This class is the 'hook' for slicer to detect and recognize the plugin
  as a loadable scripted module
  """
  def __init__(self, parent):
    parent.title = "DICOM Volume Sequence Export Plugin"
    parent.categories = ["Developer Tools.DICOM Plugins"]
    parent.contributors = ["Andras Lasso (PerkLab)"]
    parent.helpText = """
    Plugin to the DICOM Module to export volume sequence to DICOM file.
    No module interface here, only in the DICOM module.
    """
    parent.acknowledgementText = """
    Originally developed by Andras Lasso (PekLab).
    """

    # don't show this module - it only appears in the DICOM module
    parent.hidden = True

    # Add this extension to the DICOM module's list for discovery when the module
    # is created.  Since this module may be discovered before DICOM itself,
    # create the list if it doesn't already exist.
    try:
      slicer.modules.dicomPlugins
    except AttributeError:
      slicer.modules.dicomPlugins = {}
    slicer.modules.dicomPlugins['DICOMVolumeSequencePlugin'] = DICOMVolumeSequencePluginClass

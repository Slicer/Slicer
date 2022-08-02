import os
import re
import vtk, qt, ctk, slicer
import vtk.util.numpy_support
import DICOMLib
from DICOMLib import DICOMPlugin
from DICOMLib import DICOMLoadable
import logging
from slicer.util import settingsValue, toBool

#
# This is the plugin to handle translation of DICOM objects
# that can be represented as multivolume objects
# from DICOM files into MRML nodes.  It follows the DICOM module's
# plugin architecture.
#

class MultiVolumeImporterPluginClass(DICOMPlugin):
  """ MV specific interpretation code
  """

  def __init__(self,epsilon=0.01):
    super().__init__()
    self.loadType = "MultiVolume"

    self.tags['seriesInstanceUID'] = "0020,000E"
    self.tags['seriesDescription'] = "0008,103E"
    self.tags['instanceUID'] = "0008,0018"
    self.tags['position'] = "0020,0032"
    self.tags['studyDescription'] = "0008,1030"
    self.tags['seriesNumber'] = "0020,0011"
    self.tags['instanceNumber'] = "0020,0013"
    self.tags['repetitionTime'] = "0018,0080"

    # tags used to identify multivolumes
    self.multiVolumeTags = {}
    self.multiVolumeTags['TriggerTime'] = "0018,1060"
    self.multiVolumeTags['EchoTime'] = "0018,0081"
    self.multiVolumeTags['FlipAngle'] = "0018,1314"
    self.multiVolumeTags['RepetitionTime'] = "0018,0080"
    self.multiVolumeTags['AcquisitionTime'] = "0008,0032"
    self.multiVolumeTags['SeriesTime'] = "0008,0031"
    self.multiVolumeTags['ContentTime'] = "0008,0033"
    # Siemens Somatom Cardiac CT 'ScanOptions' tag contains info on cardiac cycle
    self.multiVolumeTags['CardiacCycle'] = "0018,0022"
    # GE Revolution CT uses 'NominalPercentageOfCardiacPhase' tag to identify cardiac cycle
    self.multiVolumeTags['NominalPercentageOfCardiacPhase'] = "0020,9241"
    # this one is GE-specific using the private tag
    self.multiVolumeTags['Siemens.B-value'] = "0019,100c"
    self.multiVolumeTags['GE.B-value'] = "0043,1039"
    # used on some GE systems, with 2D acquisitions
    self.multiVolumeTags['TemporalPositionIdentifier'] = "0020,0100"
    # Philips DWI
    self.multiVolumeTags['Philips.B-value'] = "2001,1003"
    self.multiVolumeTags['Standard.B-value'] = "0018,9087"
    # GE Revolution CT Kinematics protocol
    self.multiVolumeTags['DeltaStartTime'] = "0043,101e"

    for tagName,tagVal in self.multiVolumeTags.items():
      self.tags[tagName] = tagVal

    self.multiVolumeTagsUnits = {}
    self.multiVolumeTagsUnits['TriggerTime'] = "ms"
    self.multiVolumeTagsUnits['EchoTime'] = "ms"
    self.multiVolumeTagsUnits['FlipAngle'] = "degrees"
    self.multiVolumeTagsUnits['RepetitionTime'] = "ms"
    self.multiVolumeTagsUnits['AcquisitionTime'] = "ms"
    self.multiVolumeTagsUnits['SeriesTime'] = "ms"
    self.multiVolumeTagsUnits['ContentTime'] = "ms"
    self.multiVolumeTagsUnits['TemporalPositionIdentifier'] = "count"
    self.multiVolumeTagsUnits['Siemens.B-value'] = "sec/mm2"
    self.multiVolumeTagsUnits['GE.B-value'] = "sec/mm2"
    self.multiVolumeTagsUnits['Philips.B-value'] = "sec/mm2"
    self.multiVolumeTagsUnits['Standard.B-value'] = "sec/mm2"
    self.multiVolumeTagsUnits['CardiacCycle'] = "%"
    self.multiVolumeTagsUnits['NominalPercentageOfCardiacPhase'] = "%"
    self.multiVolumeTagsUnits['DeltaStartTime'] = "sec"
    self.epsilon = epsilon

    self.detailedLogging = False

  @staticmethod
  def settingsPanelEntry(panel, parent):
    """Create a settings panel entry for this plugin class.
    It is added to the DICOM panel of the application settings
    by the DICOM module.
    """

    formLayout = qt.QFormLayout(parent)
    importFormatsComboBox = ctk.ctkComboBox()
    importFormatsComboBox.toolTip = "Preferred format for imported volume sequences. It determines what MRML node type volume sequences will be loaded into."
    importFormatsComboBox.addItem("default (multi-volume)", "default")
    importFormatsComboBox.addItem("volume sequence", "sequence")
    importFormatsComboBox.addItem("multi-volume", "multivolume")
    importFormatsComboBox.currentIndex = 0
    formLayout.addRow("Preferred multi-volume import format:", importFormatsComboBox)
    panel.registerProperty(
      "DICOM/PreferredMultiVolumeImportFormat", importFormatsComboBox,
      "currentUserDataAsString", str(qt.SIGNAL("currentIndexChanged(int)")))

  def examine(self,fileLists):
    """ Returns a list of DICOMLoadable instances
    corresponding to ways of interpreting the
    fileLists parameter.

    Top-level examine() calls various individual strategies implemented in examineFiles*().
    """

    self.detailedLogging = settingsValue('DICOM/detailedLogging', False, converter=toBool)
    timer = vtk.vtkTimerLog()
    timer.StartTimer()

    loadables = []
    allfiles = []
    for files in fileLists:
      loadables += self.examineFiles(files)

      # this strategy sorts the files into groups
      loadables += self.examineFilesIPPAcqTime(files)

      allfiles += files

    # here all files are lumped into one list for the situations when
    # individual frames should be parsed from series
    loadables += self.examineFilesMultiseries(allfiles)
    if len(allfiles)>len(files):
      # only examineFilesIPPAcqTime again if there are multiple file groups
      loadables += self.examineFilesIPPAcqTime(allfiles)

    # this strategy sorts the files into groups
    loadables += self.examineFilesIPPInstanceNumber(allfiles)

    # If Sequences module is available then duplicate all the loadables
    # for loading them as volume sequence.
    # A slightly higher confidence value is set for volume sequence loadables,
    # therefore by default data will be loaded as volume sequence.

    if hasattr(slicer.modules, 'sequences'):

      seqLoadables = []
      for loadable in loadables:
        seqLoadable = DICOMLib.DICOMLoadable()
        seqLoadable.files = loadable.files
        seqLoadable.tooltip = loadable.tooltip.replace(' frames MultiVolume by ', ' frames Volume Sequence by ')
        seqLoadable.name = loadable.name.replace(' frames MultiVolume by ', ' frames Volume Sequence by ')
        seqLoadable.multivolume = loadable.multivolume
        seqLoadable.selected = loadable.selected

        seqLoadable.confidence = loadable.confidence

        seqLoadable.loadAsVolumeSequence = True
        seqLoadables.append(seqLoadable)

      # Among all selected loadables, the ones that are listed first will be selected by default,
      # therefore we need to prepend loadables if sequence format is preferred.
      # Determine from settings loading into sequence node should have higher confidence (selected by default).
      settings = qt.QSettings()
      sequenceFormatPreferred = (settings.value("DICOM/PreferredMultiVolumeImportFormat", "default") == "sequence")
      if sequenceFormatPreferred:
       # prepend
       loadables[0:0] = seqLoadables
      else:
       # append
       loadables += seqLoadables

    timer.StopTimer()
    if self.detailedLogging:
      logging.debug(f"MultiVolumeImporterPlugin: found {len(loadables)} loadables in {len(allfiles)} files in {timer.GetElapsedTime():.1f}sec.")

    return loadables

  def examineFilesMultiseries(self,files):
    """
    This strategy is similar to examineFiles(), but
    does not separate the files by individual series before
    parsing multivolumes out.
    """

    if self.detailedLogging:
      logging.debug('MultiVolumeImporterPlugin: examineMultiseries')

    loadables = []

    mvNodes = self.initMultiVolumes(files,prescribedTags=['SeriesTime','AcquisitionTime','FlipAngle','CardiacCycle'])

    if self.detailedLogging:
      logging.debug('MultiVolumeImporterPlugin: found {} multivolumes!'.format(len(mvNodes)))

    for mvNode in mvNodes:
      tagName = mvNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagName')
      nFrames = mvNode.GetNumberOfFrames()
      orderedFiles = mvNode.GetAttribute('MultiVolume.FrameFileList').split(',')

      desc = slicer.dicomDatabase.fileValue(orderedFiles[0],self.tags['studyDescription']) # SeriesDescription
      num = slicer.dicomDatabase.fileValue(orderedFiles[0],self.tags['seriesNumber'])
      if num != "":
        name = num+": "+desc
      else:
        name = desc

      if self.isFrameOriginConsistent(orderedFiles, mvNode) == False:
        continue

      loadable = DICOMLib.DICOMLoadable()
      loadable.files = orderedFiles
      loadable.tooltip =  name+' - '+str(nFrames) + ' frames MultiVolume by ' + tagName
      loadable.name = name
      loadable.selected = True
      loadable.multivolume = mvNode
      if tagName == 'TemporalPositionIdentifier':
        loadable.confidence = 0.9
      else:
        loadable.confidence = 1.
      loadables.append(loadable)

    return loadables

  def emptyTagValueFound(self,files,tags):
    for f in files:
      for tag in tags:
        value = slicer.dicomDatabase.fileValue(f,self.tags[tag])
        if value == None or value == "":
          return True
    return False

  def examineFilesIPPInstanceNumber(self,files):
    """
    This strategy first orders files into lists, where each list is
    indexed by ImagePositionPatient (IPP). Next, files within each
    list are ordered by InstanceNumber attribute. Finally, loadable
    frames are indexed by InstanceNumber + RepetitionTime, and files within each
    frame are ordered by IPP.
    This strategy was required to handle DSC MRI data collected on
    some GE platforms.
    """

    if self.emptyTagValueFound(files,['instanceNumber','position','repetitionTime']):
      return []

    loadables = []
    subseriesLists = {}
    orderedFiles = []

    desc = slicer.dicomDatabase.fileValue(files[0],self.tags['seriesDescription']) # SeriesDescription

    minTime = int(slicer.dicomDatabase.fileValue(files[0],self.tags['instanceNumber']))
    for file in files:
      ipp = slicer.dicomDatabase.fileValue(file,self.tags['position'])
      time = int(slicer.dicomDatabase.fileValue(file,self.tags['instanceNumber']))
      if time<minTime:
        minTime = time
      if ipp not in subseriesLists:
        subseriesLists[ipp] = {}
      subseriesLists[ipp][time] = file

    nSlicesEqual = True
    allIPPs = list(subseriesLists.keys())
    for ipp in subseriesLists.keys():
      if len(subseriesLists[allIPPs[0]].keys()) != len(subseriesLists[ipp].keys()):
        nSlicesEqual = False
        break

    if len(subseriesLists[allIPPs[0]].keys())<2 or not nSlicesEqual:
      return []

    if nSlicesEqual:
      nFrames = len(subseriesLists[allIPPs[0]].keys())
      nSlices = len(allIPPs)

      orderedFiles = [0] * nFrames * nSlices

      frameLabelsStr=""
      frameFileListStr = ""
      frameLabelsArray = vtk.vtkDoubleArray()

      ippPositionCnt = 0
      for ipp in subseriesLists.keys():
        timesSorted = sorted(subseriesLists[ipp].keys())
        timeCnt = 0
        for time in timesSorted:
          orderedFiles[timeCnt*nSlices+ippPositionCnt] = subseriesLists[ipp][time]
          timeCnt = timeCnt+1
          if ippPositionCnt == 0:
            frameLabelsStr = frameLabelsStr+str(time-minTime)+','
            frameLabelsArray.InsertNextValue(time-minTime)
        ippPositionCnt = ippPositionCnt+1

      scalarVolumePlugin = slicer.modules.dicomPlugins['DICOMScalarVolumePlugin']()
      for f in range(nFrames):
        frameFileList = orderedFiles[f*nSlices:(f+1)*nSlices]
        svs = scalarVolumePlugin.examine([frameFileList])
        if len(svs)==0:
          print('Failed to parse one of the multivolume frames as scalar volume!')
          break
        time = float(slicer.dicomDatabase.fileValue(svs[0].files[0],self.tags['repetitionTime']))*f
        if f==0:
            frameLabelsStr = '0,'
            frameLabelsArray.InsertNextValue(0)
        else:
            frameLabelsStr = frameLabelsStr+str(time)+','
            frameLabelsArray.InsertNextValue(time)

      for file in orderedFiles:
        frameFileListStr = frameFileListStr+str(file)+','

      frameLabelsStr = frameLabelsStr[:-1]
      frameFileListStr = frameFileListStr[:-1]

      mvNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLMultiVolumeNode')
      mvNode.SetReferenceCount(mvNode.GetReferenceCount()-1)
      mvNode.SetScene(slicer.mrmlScene)
      mvNode.SetAttribute("MultiVolume.FrameLabels",frameLabelsStr)
      mvNode.SetAttribute("MultiVolume.FrameIdentifyingDICOMTagName","Time")
      mvNode.SetAttribute("MultiVolume.ParseStrategy","TemporalPosition_via_InstanceNumber*RepetitionTime")
      mvNode.SetAttribute('MultiVolume.NumberOfFrames',str(nFrames))
      mvNode.SetAttribute('MultiVolume.FrameIdentifyingDICOMTagUnits',"ms")
      # keep the files in the order by the detected tag
      # files are not ordered within the individual frames -- this will be
      # done by ScalarVolumePlugin later
      mvNode.SetAttribute('MultiVolume.FrameFileList', frameFileListStr)

      self.addAcquisitionAttributes(mvNode, frameFileList)

      mvNode.SetNumberOfFrames(nFrames)
      mvNode.SetLabelName("Time")
      mvNode.SetLabelArray(frameLabelsArray)

      loadable = DICOMLib.DICOMLoadable()
      loadable.files = orderedFiles
      loadable.name = desc + ' - as a ' + str(nFrames) + ' frames MultiVolume by ImagePositionPatient+InstanceNumber'
      mvNode.SetName(desc)
      loadable.tooltip = loadable.name
      loadable.selected = True
      loadable.multivolume = mvNode
      loadable.confidence = 1.
      loadables.append(loadable)

    return loadables


  def examineFilesIPPAcqTime(self,files):
    """
    This strategy first orders files into lists, where each list is
    indexed by ImagePositionPatient (IPP). Next, files within each
    list are ordered by AcquisitionTime attribute. Finally, loadable
    frames are indexed by AcquisitionTime, and files within each
    frame are ordered by IPP.
    This strategy was required to handle DSC MRI data collected on
    Siemens, tested with a DSC sequence obtained using software
    version "syngo MR B15"
    """

    loadables = []
    subseriesLists = {}
    orderedFiles = []

    desc = slicer.dicomDatabase.fileValue(files[0],self.tags['seriesDescription']) # SeriesDescription

    minTime = self.tm2ms(slicer.dicomDatabase.fileValue(files[0],self.tags['AcquisitionTime']))
    for file in files:
      ipp = slicer.dicomDatabase.fileValue(file,self.tags['position'])
      time = self.tm2ms(slicer.dicomDatabase.fileValue(file,self.tags['AcquisitionTime']))
      if time<minTime:
        minTime = time
      if ipp not in subseriesLists:
        subseriesLists[ipp] = {}
      subseriesLists[ipp][time] = file

    nSlicesEqual = True
    allIPPs = list(subseriesLists.keys())
    for ipp in subseriesLists.keys():
      if len(subseriesLists[allIPPs[0]].keys()) != len(subseriesLists[ipp].keys()):
        nSlicesEqual = False
        break

    if len(subseriesLists[allIPPs[0]].keys())<2 or not nSlicesEqual:
      return []

    if nSlicesEqual:
      nFrames = len(subseriesLists[allIPPs[0]].keys())
      nSlices = len(allIPPs)

      orderedFiles = [0] * nFrames * nSlices

      frameLabelsStr=""
      frameFileListStr = ""
      frameLabelsArray = vtk.vtkDoubleArray()

      ippPositionCnt = 0
      for ipp in subseriesLists.keys():
        timesSorted = sorted(subseriesLists[ipp].keys())
        timeCnt = 0
        for time in timesSorted:
          orderedFiles[timeCnt*nSlices+ippPositionCnt] = subseriesLists[ipp][time]
          timeCnt = timeCnt+1
          if ippPositionCnt == 0:
            frameLabelsStr = frameLabelsStr+str(time-minTime)+','
            frameLabelsArray.InsertNextValue(time-minTime)
        ippPositionCnt = ippPositionCnt+1

      scalarVolumePlugin = slicer.modules.dicomPlugins['DICOMScalarVolumePlugin']()
      firstFrameTime = 0
      for f in range(nFrames):
        frameFileList = orderedFiles[f*nSlices:(f+1)*nSlices]
        svs = scalarVolumePlugin.examine([frameFileList])
        if len(svs)==0:
          print('Failed to parse one of the multivolume frames as scalar volume!')
          break
        time = self.tm2ms(slicer.dicomDatabase.fileValue(svs[0].files[0],self.tags['AcquisitionTime']))
        if f==0:
            frameLabelsStr = '0,'
            frameLabelsArray.InsertNextValue(0)
            firstFrameTime = time
        else:
            frameLabelsStr = frameLabelsStr+str(time-firstFrameTime)+','
            frameLabelsArray.InsertNextValue(time)

      for file in orderedFiles:
        frameFileListStr = frameFileListStr+str(file)+','

      frameLabelsStr = frameLabelsStr[:-1]
      frameFileListStr = frameFileListStr[:-1]

      mvNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLMultiVolumeNode')
      mvNode.SetReferenceCount(mvNode.GetReferenceCount()-1)
      mvNode.SetScene(slicer.mrmlScene)
      mvNode.SetAttribute("MultiVolume.FrameLabels",frameLabelsStr)
      mvNode.SetAttribute("MultiVolume.FrameIdentifyingDICOMTagName","AcquisitionTime")
      mvNode.SetAttribute("MultiVolume.ParseStrategy","AcquisitionTime+ImagePositionPatient")
      mvNode.SetAttribute('MultiVolume.NumberOfFrames',str(nFrames))
      mvNode.SetAttribute('MultiVolume.FrameIdentifyingDICOMTagUnits',"ms")
      # keep the files in the order by the detected tag
      # files are not ordered within the individual frames -- this will be
      # done by ScalarVolumePlugin later
      mvNode.SetAttribute('MultiVolume.FrameFileList', frameFileListStr)

      self.addAcquisitionAttributes(mvNode, frameFileList)

      mvNode.SetNumberOfFrames(nFrames)
      mvNode.SetLabelName("AcquisitionTime")
      mvNode.SetLabelArray(frameLabelsArray)

      loadable = DICOMLib.DICOMLoadable()
      loadable.files = orderedFiles
      loadable.name = desc + ' - as a ' + str(nFrames) + ' frames MultiVolume by ImagePositionPatient+AcquisitionTime'
      mvNode.SetName(desc)
      loadable.tooltip = loadable.name
      loadable.selected = True
      loadable.multivolume = mvNode
      loadable.confidence = 1.
      loadables.append(loadable)

    return loadables

  def addAcquisitionAttributes(self,mvNode,frameFileList):
    frameTag = mvNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagName')

    for tag in ['EchoTime','RepetitionTime','FlipAngle']:
      if tag != frameTag:
        tagValue = slicer.dicomDatabase.fileValue(frameFileList[0],self.tags[tag])
        mvNode.SetAttribute('MultiVolume.DICOM.'+tag,tagValue)

  def examineFiles(self,files):

    """
    This is the main strategy that assumes all files (instances) belong
    to the same series, and all instances within the same frame have the same value for one of the attributes defined in self.multiVolumeTags
    """

    if self.detailedLogging:
      logging.debug("MultiVolumeImporterPlugin: examine")

    """ Returns a list of DICOMLoadable instances
    corresponding to ways of interpreting the
    files parameter.
    """
    loadables = []

    # Look for series with several values in either of the volume-identifying
    #  tags in files

    # first separate individual series, then try to find multivolume in each
    # of the series (code from DICOMScalarVolumePlugin)
    subseriesLists = {}
    subseriesDescriptions = {}

    for file in files:

      value = slicer.dicomDatabase.fileValue(file,self.tags['seriesInstanceUID']) # SeriesInstanceUID
      desc = slicer.dicomDatabase.fileValue(file,self.tags['seriesDescription']) # SeriesDescription

      if value == "":
        value = "Unknown"

      if desc == "":
        desc = "Unknown"

      if value not in subseriesLists:
        subseriesLists[value] = []
      subseriesLists[value].append(file)
      subseriesDescriptions[value] = desc

    # now iterate over all subseries file lists and try to parse the
    # multivolumes


    for key in subseriesLists.keys():

      mvNodes = self.initMultiVolumes(subseriesLists[key])

      if self.detailedLogging:
        logging.debug('MultiVolumeImporterPlugin: found '+str(len(mvNodes))+' multivolumes!')

      for mvNode in mvNodes:
        tagName = mvNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagName')
        nFrames = mvNode.GetNumberOfFrames()
        orderedFiles = mvNode.GetAttribute('MultiVolume.FrameFileList').split(',')

        if self.isFrameOriginConsistent(orderedFiles, mvNode) == False:
          continue

        loadable = DICOMLib.DICOMLoadable()
        loadable.files = files
        loadable.name = subseriesDescriptions[key] + ' - as a ' + str(nFrames) + ' frames MultiVolume by ' + tagName
        mvNode.SetName(subseriesDescriptions[key])
        loadable.tooltip = loadable.name
        loadable.selected = True
        loadable.multivolume = mvNode
        if tagName == 'TemporalPositionIdentifier':
          loadable.confidence = 0.9
        else:
          loadable.confidence = 1.
        loadables.append(loadable)

    return loadables

  # return true is the origins for the individual frames are within
  # self.epsilon apart
  def isFrameOriginConsistent(self, files, mvNode):

    nFrames = mvNode.GetNumberOfFrames()

    # sort files for each frame
    nFiles = len(files)
    filesPerFrame = int(nFiles/nFrames)
    frameOrigins = []

    scalarVolumePlugin = slicer.modules.dicomPlugins['DICOMScalarVolumePlugin']()
    firstFrameOrigin = None
    for frameNumber in range(nFrames):
      frameFileList = files[frameNumber*filesPerFrame:(frameNumber+1)*filesPerFrame]

      # sv plugin will sort the filenames by geometric order
      svs = scalarVolumePlugin.examine([frameFileList])
      if len(svs) == 0:
        return False

      positionTag = slicer.dicomDatabase.fileValue(svs[0].files[0], self.tags['position'])
      if positionTag == '':
        return False
      origin = [float(zz) for zz in positionTag.split('\\')]

      if firstFrameOrigin is None:
        # this is the first frame, just record the origin
        firstFrameOrigin = origin
      else:
        # compare this frame's origin to the origin of the first frame
        if abs(origin[0]-firstFrameOrigin[0])>self.epsilon or abs(origin[1]-firstFrameOrigin[1])>self.epsilon or abs(origin[2]-firstFrameOrigin[2])>self.epsilon:
          # frames have mismatching origins
          return False

    return True

  def load(self,loadable):
    """Load the selection as a MultiVolume, if multivolume attribute is
    present
    """

    mvNode = ''
    try:
      mvNode = loadable.multivolume
    except AttributeError:
      return None

    nFrames = int(mvNode.GetAttribute('MultiVolume.NumberOfFrames'))
    files = mvNode.GetAttribute('MultiVolume.FrameFileList').split(',')
    nFiles = len(files)
    filesPerFrame = int(nFiles/nFrames)
    frames = []

    baseName = loadable.name

    loadAsVolumeSequence = hasattr(loadable, 'loadAsVolumeSequence') and loadable.loadAsVolumeSequence
    if loadAsVolumeSequence:
      volumeSequenceNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSequenceNode",
        slicer.mrmlScene.GenerateUniqueName(baseName))
      volumeSequenceNode.SetIndexName("")
      volumeSequenceNode.SetIndexUnit("")
      # Transfer all attributes from multivolume node to volume sequence node
      for attrName in mvNode.GetAttributeNames():
        volumeSequenceNode.SetAttribute(attrName, mvNode.GetAttribute(attrName))
    else:
      mvImage = vtk.vtkImageData()
      mvImageArray = None

    scalarVolumePlugin = slicer.modules.dicomPlugins['DICOMScalarVolumePlugin']()
    instanceUIDs = ""
    for file in files:
      uid = slicer.dicomDatabase.fileValue(file,self.tags['instanceUID'])
      if uid == "":
        uid = "Unknown"
      instanceUIDs += uid+" "
    instanceUIDs = instanceUIDs[:-1]
    mvNode.SetAttribute("DICOM.instanceUIDs", instanceUIDs)

    progressbar = slicer.util.createProgressDialog(labelText="Loading "+baseName,
                                                   value=0, maximum=nFrames,
                                                   windowModality = qt.Qt.WindowModal)

    try:
      # read each frame into scalar volume
      for frameNumber in range(nFrames):

        progressbar.value = frameNumber
        slicer.app.processEvents()
        if progressbar.wasCanceled:
          break

        sNode = slicer.vtkMRMLVolumeArchetypeStorageNode()
        sNode.ResetFileNameList()

        frameFileList = files[frameNumber*filesPerFrame:(frameNumber+1)*filesPerFrame]
        # sv plugin will sort the filenames by geometric order
        svLoadables = scalarVolumePlugin.examine([frameFileList])

        if len(svLoadables) == 0:
          raise OSError(f"volume frame {frameNumber} is invalid")

        frame = scalarVolumePlugin.load(svLoadables[0])

        if frame == None or frame.GetImageData() == None:
          raise OSError(f"Volume frame {frameNumber} is invalid - {svLoadables[0].warning}")
        if loadAsVolumeSequence:
          # Load into volume sequence

          # volumeSequenceNode.SetDataNodeAtValue would deep-copy the volume frame.
          # To avoid memory reallocation, add an empty node and shallow-copy the contents
          # of the volume frame.

          # Create an empty volume node in the sequence node
          proxyVolume = slicer.mrmlScene.AddNewNodeByClass(frame.GetClassName())
          indexValue = str(frameNumber)
          volumeSequenceNode.SetDataNodeAtValue(proxyVolume, indexValue)
          slicer.mrmlScene.RemoveNode(proxyVolume)

          # Update the data node
          shallowCopy = True
          volumeSequenceNode.UpdateDataNodeAtValue(frame, indexValue, shallowCopy)

        else:
          # Load into multi-volume

          if frameNumber == 0:
            frameImage = frame.GetImageData()
            frameExtent = frameImage.GetExtent()
            frameSize = frameExtent[1]*frameExtent[3]*frameExtent[5]

            mvImage.SetExtent(frameExtent)
            mvImage.AllocateScalars(frame.GetImageData().GetScalarType(), nFrames)

            mvImageArray = vtk.util.numpy_support.vtk_to_numpy(mvImage.GetPointData().GetScalars())

            mvNode.SetScene(slicer.mrmlScene)

            mat = vtk.vtkMatrix4x4()
            frame.GetRASToIJKMatrix(mat)
            mvNode.SetRASToIJKMatrix(mat)
            frame.GetIJKToRASMatrix(mat)
            mvNode.SetIJKToRASMatrix(mat)

          frameImage = frame.GetImageData()
          frameImageArray = vtk.util.numpy_support.vtk_to_numpy(frameImage.GetPointData().GetScalars())

          mvImageArray.T[frameNumber] = frameImageArray

        # Remove temporary volume node
        if frame.GetDisplayNode():
          slicer.mrmlScene.RemoveNode(frame.GetDisplayNode())
        if frame.GetStorageNode():
          slicer.mrmlScene.RemoveNode(frame.GetStorageNode())
        slicer.mrmlScene.RemoveNode(frame)

      if loadAsVolumeSequence:
        # Finalize volume sequence import
        # For user convenience, add a browser node and show the volume in the slice viewer.

        # Add browser node
        sequenceBrowserNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLSequenceBrowserNode',
          slicer.mrmlScene.GenerateUniqueName(baseName + " browser"))
        sequenceBrowserNode.SetAndObserveMasterSequenceNodeID(volumeSequenceNode.GetID())
        # If save changes are allowed then proxy nodes are updated using shallow copy, which is much
        # faster for images. Images are usually not modified, so the risk of accidentally modifying
        # data in the sequence is low.
        sequenceBrowserNode.SetSaveChanges(volumeSequenceNode, True)
        # Show frame number in proxy volume node name
        sequenceBrowserNode.SetOverwriteProxyName(volumeSequenceNode, True);

        # Automatically select the volume to display
        imageProxyVolumeNode = sequenceBrowserNode.GetProxyNode(volumeSequenceNode)
        appLogic = slicer.app.applicationLogic()
        selNode = appLogic.GetSelectionNode()
        selNode.SetReferenceActiveVolumeID(imageProxyVolumeNode.GetID())
        appLogic.PropagateVolumeSelection()

        # Show under the right patient/study in subject hierarchy
        self.addSeriesInSubjectHierarchy(loadable, imageProxyVolumeNode)

        # Show sequence browser toolbar
        sequencesModule = slicer.modules.sequences
        if sequencesModule.autoShowToolBar:
          sequencesModule.setToolBarActiveBrowserNode(sequenceBrowserNode)
          sequencesModule.setToolBarVisible(True)

      else:
        # Finalize multi-volume import

        mvDisplayNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMultiVolumeDisplayNode')
        mvDisplayNode.SetDefaultColorMap()

        mvNode.SetAndObserveDisplayNodeID(mvDisplayNode.GetID())
        mvNode.SetAndObserveImageData(mvImage)
        mvNode.SetNumberOfFrames(nFrames)
        mvNode.SetName(loadable.name)
        slicer.mrmlScene.AddNode(mvNode)

        # Show under the right patient/study in subject hierarchy
        self.addSeriesInSubjectHierarchy(loadable, mvNode)

        #
        # automatically select the volume to display
        #
        appLogic = slicer.app.applicationLogic()
        selNode = appLogic.GetSelectionNode()
        selNode.SetReferenceActiveVolumeID(mvNode.GetID())
        appLogic.PropagateVolumeSelection()

        # file list is no longer needed - remove the attribute
        mvNode.RemoveAttribute('MultiVolume.FrameFileList')

    except Exception as e:
      logging.error(f"Failed to read a multivolume: {str(e)}")
      import traceback
      traceback.print_exc()
      mvNode = None

    finally:
      progressbar.close()

    return mvNode

  def tm2ms(self,tm):

    if len(tm)<6:
      return 0

    try:
      hhmmss = tm.split('.')[0]
    except:
      hhmmss = tm

    try:
      ssfrac = float('0.'+tm.split('.')[1])
    except:
      ssfrac = 0.

    if len(hhmmss)==6: # HHMMSS
      sec = float(hhmmss[0:2])*60.*60.+float(hhmmss[2:4])*60.+float(hhmmss[4:6])
    elif len(hhmmss)==4: # HHMM
      sec = float(hhmmss[0:2])*60.*60.+float(hhmmss[2:4])*60.
    elif len(hhmmss)==2: # HH
      sec = float(hhmmss[0:2])*60.*60.
    else:
      raise OSError("Invalid DICOM time string: "+tm+" (failed to parse HHMMSS)")

    sec = sec+ssfrac

    return sec*1000.

  def initMultiVolumes(self, files, prescribedTags=None):
    tag2ValueFileList = {}
    multivolumes = []

    if prescribedTags == None:
      consideredTags = list(self.multiVolumeTags.keys())
    else:
      consideredTags = list(prescribedTags)

    # iterate over all files
    tagsToIgnore = []
    for file in files:

      # Remove tags that were not found in the previous iteration
      for frameTag in tagsToIgnore:
        consideredTags.remove(frameTag)
      tagsToIgnore = []

      # iterate over the tags that can be used to separate individual frames
      for frameTag in consideredTags:
        try:
          tagValue2FileList = tag2ValueFileList[frameTag]
        except:
          tagValue2FileList = {}
          tag2ValueFileList[frameTag] = tagValue2FileList

        tagValueStr = slicer.dicomDatabase.fileValue(file,self.tags[frameTag])
        if tagValueStr == '':
          # not found?
          tagsToIgnore.append(frameTag)
          continue

        if frameTag == 'AcquisitionTime' or frameTag == 'SeriesTime' or frameTag == 'ContentTime':
          # extra parsing is needed to convert from DICOM TM VR into ms
          tagValue = self.tm2ms(tagValueStr) # convert to ms
        elif frameTag == "GE.B-value":
          try:
            # Parse this:
            # (0043,1039) IS [1000001250\8\0\0] #  16, 4 Unknown Tag & Data
            # GE Discovery w750
            tagValue = float(int(tagValueStr.split('\\')[0]) % 100000)
          except:
            continue
        elif frameTag == "CardiacCycle":
          try:
            # Parse this:
            #  TP0PC0965, PULSTART_P0020PC, PULSEND_P0080PC...
            #  TP10PC0965, PULSTART_P0020PC, PULSEND_P0080PC...
            #  TP30PC0965, PULSTART_P0020PC, PULSEND_P0080PC...
            cardiacPhaseInfo = tagValueStr.split('\\')[0] # TP0PC0965
            matched = re.search(r"TP(\d+)PC(\d+)", cardiacPhaseInfo)
            tagValue = float(matched.groups()[0])
          except:
            continue
        else:
          try:
            tagValue = float(tagValueStr)
          except:
            continue

        try:
          tagValue2FileList[tagValue].append(file)
        except:
          tagValue2FileList[tagValue] = [file]

    # iterate over the parsed items and decide which ones can qualify as mv
    for frameTag in consideredTags:

      try:
        tagValue2FileList = tag2ValueFileList[frameTag]
      except:
        # didn't find the tag
        continue

      if len(tagValue2FileList)<2:
        # not enough frames for this tag to be a multivolume
        continue

      tagValues = sorted(tagValue2FileList.keys())

      # Check if the number of frames is the same in each volume
      slicesPerFrame = {}  # key: number of slices, value: list of tagValue values that has this many slices
      for tagValue in tagValues:
        numberOfSlices = len(tagValue2FileList[tagValue])
        if numberOfSlices in slicesPerFrame.keys():
          slicesPerFrame[numberOfSlices].append(tagValue)
        else:
          slicesPerFrame[numberOfSlices] = [tagValue]

      if len(slicesPerFrame) > 1:
        # We only accept volumes that has the same number of slices per frame.
        # There are multiple different slicesPerFrame values, therefore it is rejected.
        if self.detailedLogging:
          seriesNumber = slicer.dicomDatabase.fileValue(file, self.tags['seriesNumber'])
          seriesDescription = slicer.dicomDatabase.fileValue(file, self.tags['seriesDescription'])
          seriesInstanceUid = slicer.dicomDatabase.fileValue(file, self.tags['seriesInstanceUID'])
          msg = f"MultiVolumeImporterPlugin: series {seriesNumber}: {seriesDescription} ({seriesInstanceUid}) " + \
                 "is not accepted as multi-volume because number of slices varies across frames."
          for numberOfSlices in slicesPerFrame:
            msg += f" {numberOfSlices} slices are found for {frameTag}={slicesPerFrame[numberOfSlices]}."
          logging.debug(msg)
        continue

      # TODO: add a check to confirm individual frames have the same geometry
      # (check pixel dimensions, orientation, position)

      # now this looks like a serious mv!

      # initialize the needed attributes for a new mvNode
      frameFileListStr = ""
      frameLabelsStr = ""
      frameLabelsArray = vtk.vtkDoubleArray()
      tagValue0 = tagValues[0]
      for tagValue in tagValues:
        frameFileList = tagValue2FileList[tagValue]
        for file in frameFileList:
          frameFileListStr = frameFileListStr+file+','

        # if mv was parsed by series time, probably makes sense to start from 0
        if frameTag == 'SeriesTime' or frameTag == 'AcquisitionTime' or frameTag == 'ContentTime':
          frameLabelsArray.InsertNextValue(tagValue-tagValue0)
          frameLabelsStr = frameLabelsStr+str(tagValue-tagValue0)+','
        else:
          frameLabelsArray.InsertNextValue(tagValue)
          frameLabelsStr = frameLabelsStr+str(tagValue)+','

      frameFileListStr = frameFileListStr[:-1]
      frameLabelsStr = frameLabelsStr[:-1]

      mvNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLMultiVolumeNode')
      mvNode.UnRegister(None)
      mvNode.SetAttribute("MultiVolume.FrameLabels",frameLabelsStr)
      mvNode.SetAttribute("MultiVolume.FrameIdentifyingDICOMTagName",frameTag)
      mvNode.SetAttribute('MultiVolume.NumberOfFrames',str(len(tagValue2FileList)))
      mvNode.SetAttribute('MultiVolume.FrameIdentifyingDICOMTagUnits',self.multiVolumeTagsUnits[frameTag])
      # keep the files in the order by the detected tag
      # files are not ordered within the individual frames -- this will be
      # done by ScalarVolumePlugin later
      mvNode.SetAttribute('MultiVolume.FrameFileList', frameFileListStr)

      mvNode.SetNumberOfFrames(len(tagValue2FileList))
      mvNode.SetLabelName(self.multiVolumeTagsUnits[frameTag])
      mvNode.SetLabelArray(frameLabelsArray)

      self.addAcquisitionAttributes(mvNode, frameFileList)

      # add the node
      multivolumes.append(mvNode)

    return multivolumes

'''

* need to decide if import plugin should handle all types of MV
* separate functionality for parsing/detecting and loading?
* C++ code for reading and parsing DICOM header?
* once loadable is determined, need to pass the tag separating individual
* volumes ? !
'''

#
# MultiVolumeImporterPlugin
#

class MultiVolumeImporterPlugin:
  """
  This class is the 'hook' for slicer to detect and recognize the plugin
  as a loadable scripted module
  """
  def __init__(self, parent):
    parent.title = "DICOM MultiVolume Import Plugin"
    parent.categories = ["Developer Tools.DICOM Plugins"]
    parent.contributors = ["Andrey Fedorov, BWH"]
    parent.helpText = """
    Plugin to the DICOM Module to parse and load MultiVolume data from DICOM files.
    No module interface here, only in the DICOM module
    """
    parent.acknowledgementText = """
    This DICOM Plugin was developed by
    Andrey Fedorov, BWH.
    and was partially funded by NIH grant U01CA151261.
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
    slicer.modules.dicomPlugins['MultiVolumeImporterPlugin'] = MultiVolumeImporterPluginClass

#
#

class MultiVolumeImporterPluginWidget:
  def __init__(self, parent = None):
    self.parent = parent

  def setup(self):
    # don't display anything for this widget - it will be hidden anyway
    pass

  def enter(self):
    pass

  def exit(self):
    pass

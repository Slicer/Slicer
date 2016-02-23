import os
import string
import vtk, qt, ctk, slicer
import vtk.util.numpy_support
import DICOMLib
from DICOMLib import DICOMPlugin
from DICOMLib import DICOMLoadable
import logging

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
    super(MultiVolumeImporterPluginClass,self).__init__()
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
    # this one is GE-specific using the private tag
    self.multiVolumeTags['Siemens.B-value'] = "0019,100c"
    self.multiVolumeTags['GE.B-value'] = "0043,1039"
    # used on some GE systems, with 2D acquisitions
    self.multiVolumeTags['TemporalPositionIdentifier'] = "0020,0100"
    # Philips DWI
    self.multiVolumeTags['Philips.B-value'] = "2001,1003"
    self.multiVolumeTags['Standard.B-value'] = "0018,9087"

    for tagName,tagVal in self.multiVolumeTags.iteritems():
      self.tags[tagName] = tagVal

    self.multiVolumeTagsUnits = {}
    self.multiVolumeTagsUnits['TriggerTime'] = "ms"
    self.multiVolumeTagsUnits['EchoTime'] = "ms"
    self.multiVolumeTagsUnits['FlipAngle'] = "degrees"
    self.multiVolumeTagsUnits['RepetitionTime'] = "ms"
    self.multiVolumeTagsUnits['AcquisitionTime'] = "ms"
    self.multiVolumeTagsUnits['SeriesTime'] = "ms"
    self.multiVolumeTagsUnits['TemporalPositionIdentifier'] = "count"
    self.multiVolumeTagsUnits['Siemens.B-value'] = "sec/mm2"
    self.multiVolumeTagsUnits['GE.B-value'] = "sec/mm2"
    self.multiVolumeTagsUnits['Philips.B-value'] = "sec/mm2"
    self.multiVolumeTagsUnits['Standard.B-value'] = "sec/mm2"
    self.epsilon = epsilon

  def examine(self,fileLists):
    """ Returns a list of DICOMLoadable instances
    corresponding to ways of interpreting the
    fileLists parameter.

    Top-level examine() calls various individual strategies implemented in examineFiles*().
    """
    loadables = []
    allfiles = []
    for files in fileLists:
      loadables += self.examineFiles(files)
      allfiles += files

    # here all files are lumped into one list for the situations when
    # individual frames should be parsed from series
    loadables += self.examineFilesMultiseries(allfiles)

    # this strategy sorts the files into groups
    loadables += self.examineFilesIPPAcqTime(allfiles)

    # this strategy sorts the files into groups
    loadables += self.examineFilesIPPInstanceNumber(allfiles)

    return loadables


  def examineFilesMultiseries(self,files):
    """
    This strategy is similar to examineFiles(), but
    does not separate the files by individual series before
    parsing multivolumes out.
    """

    logging.debug('MultiVolumeImportPlugin:examineMultiseries')
    loadables = []

    mvNodes = self.initMultiVolumes(files,prescribedTags=['SeriesTime','AcquisitionTime','FlipAngle'])

    logging.debug('DICOMMultiVolumePlugin found '+str(len(mvNodes))+' multivolumes!')

    for mvNode in mvNodes:
      tagName = mvNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagName')
      nFrames = mvNode.GetNumberOfFrames()
      orderedFiles = string.split(mvNode.GetAttribute('MultiVolume.FrameFileList'),',')

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
      loadable.confidence = 0.9
      loadables.append(loadable)

    return loadables

  def emptyTagValueFound(self,files,tags):
    for f in files:
      for tag in tags:
        value = slicer.dicomDatabase.fileValue(f,tag)
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
      if not subseriesLists.has_key(ipp):
        subseriesLists[ipp] = {}
      subseriesLists[ipp][time] = file

    nSlicesEqual = True
    allIPPs = subseriesLists.keys()
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
        timesSorted = subseriesLists[ipp].keys()
        timesSorted.sort()
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
      loadable.confidence = 0.9
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
      if not subseriesLists.has_key(ipp):
        subseriesLists[ipp] = {}
      subseriesLists[ipp][time] = file

    nSlicesEqual = True
    allIPPs = subseriesLists.keys()
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
        timesSorted = subseriesLists[ipp].keys()
        timesSorted.sort()
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
      loadable.confidence = 0.9
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

    logging.debug("MultiVolumeImportPlugin::examine")

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

      if not subseriesLists.has_key(value):
        subseriesLists[value] = []
      subseriesLists[value].append(file)
      subseriesDescriptions[value] = desc

    # now iterate over all subseries file lists and try to parse the
    # multivolumes


    for key in subseriesLists.keys():

      mvNodes = self.initMultiVolumes(subseriesLists[key])

      logging.debug('DICOMMultiVolumePlugin found '+str(len(mvNodes))+' multivolumes!')

      for mvNode in mvNodes:
        tagName = mvNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagName')
        nFrames = mvNode.GetNumberOfFrames()
        orderedFiles = string.split(mvNode.GetAttribute('MultiVolume.FrameFileList'),',')

        if self.isFrameOriginConsistent(orderedFiles, mvNode) == False:
          continue

        loadable = DICOMLib.DICOMLoadable()
        loadable.files = files
        loadable.name = subseriesDescriptions[key] + ' - as a ' + str(nFrames) + ' frames MultiVolume by ' + tagName
        mvNode.SetName(subseriesDescriptions[key])
        loadable.tooltip = loadable.name
        loadable.selected = True
        loadable.multivolume = mvNode
        loadable.confidence = 1.
        loadables.append(loadable)

    return loadables

  # return true is the origins for the individual frames are within
  # self.epsilon apart
  def isFrameOriginConsistent(self, files, mvNode):

    nFrames = mvNode.GetNumberOfFrames()

    # sort files for each frame
    nFiles = len(files)
    filesPerFrame = nFiles/nFrames
    frameOrigins = []

    scalarVolumePlugin = slicer.modules.dicomPlugins['DICOMScalarVolumePlugin']()
    for frameNumber in range(nFrames):
      frameFileList = files[frameNumber*filesPerFrame:(frameNumber+1)*filesPerFrame]
      # sv plugin will sort the filenames by geometric order
      svs = scalarVolumePlugin.examine([frameFileList])
      if len(svs) == 0:
        return False

      positionTag = slicer.dicomDatabase.fileValue(svs[0].files[0], self.tags['position'])

      if positionTag == '':
        return False

      frameOrigins.append([float(zz) for zz in positionTag.split('\\')])

    # compare frame origins with the origin of the first frame
    firstO = frameOrigins[0]
    for o in frameOrigins[1:]:
      if abs(o[0]-firstO[0])>self.epsilon or abs(o[1]-firstO[1])>self.epsilon or abs(o[2]-firstO[2])>self.epsilon:
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
    files = string.split(mvNode.GetAttribute('MultiVolume.FrameFileList'),',')
    nFiles = len(files)
    filesPerFrame = nFiles/nFrames
    frames = []

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

    # read each frame into scalar volume
    for frameNumber in range(nFrames):

      sNode = slicer.vtkMRMLVolumeArchetypeStorageNode()
      sNode.ResetFileNameList();

      frameFileList = files[frameNumber*filesPerFrame:(frameNumber+1)*filesPerFrame]
      # sv plugin will sort the filenames by geometric order
      svLoadables = scalarVolumePlugin.examine([frameFileList])

      if len(svLoadables) == 0:
        return None
      for f in svLoadables[0].files:
        sNode.AddFileName(f)

      sNode.SetFileName(frameFileList[0]) # only used when num files/frame = 1
      sNode.SetSingleFile(0)
      frame = slicer.vtkMRMLScalarVolumeNode()
      sNode.ReadData(frame)

      if frame.GetImageData() == None:
        logging.error('Failed to read a multivolume frame!')
        return None

      if frameNumber == 0:
        frameImage = frame.GetImageData()
        frameExtent = frameImage.GetExtent()
        frameSize = frameExtent[1]*frameExtent[3]*frameExtent[5]

        mvImage.SetExtent(frameExtent)
        if vtk.VTK_MAJOR_VERSION <= 5:
          mvImage.SetNumberOfScalarComponents(nFrames)
          mvImage.SetScalarType(frame.GetImageData().GetScalarType())
          mvImage.AllocateScalars()
        else:
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

    mvDisplayNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLMultiVolumeDisplayNode')
    mvDisplayNode.SetReferenceCount(mvDisplayNode.GetReferenceCount()-1)
    mvDisplayNode.SetScene(slicer.mrmlScene)
    mvDisplayNode.SetDefaultColorMap()
    slicer.mrmlScene.AddNode(mvDisplayNode)

    mvNode.SetAndObserveDisplayNodeID(mvDisplayNode.GetID())
    mvNode.SetAndObserveImageData(mvImage)
    mvNode.SetNumberOfFrames(nFrames)
    mvNode.SetName(loadable.name)
    slicer.mrmlScene.AddNode(mvNode)

    #
    # automatically select the volume to display
    #
    appLogic = slicer.app.applicationLogic()
    selNode = appLogic.GetSelectionNode()
    selNode.SetReferenceActiveVolumeID(mvNode.GetID())
    appLogic.PropagateVolumeSelection()

    # file list is no longer needed - remove the attribute
    mvNode.RemoveAttribute('MultiVolume.FrameFileList')

    return mvNode

  def tm2ms(self,tm):

    if len(tm)<6:
      return 0

    try:
      hhmmss = string.split(tm,'.')[0]
    except:
      hhmmss = tm

    try:
      ssfrac = float('0.'+string.split(tm,'.')[1])
    except:
      ssfrac = 0.

    if len(hhmmss)==6: # HHMMSS
      sec = float(hhmmss[0:2])*60.*60.+float(hhmmss[2:4])*60.+float(hhmmss[4:6])
    elif len(hhmmss)==4: # HHMM
      sec = float(hhmmss[0:2])*60.*60.+float(hhmmss[2:4])*60.
    elif len(hhmmss)==2: # HH
      sec = float(hhmmss[0:2])*60.*60.

    sec = sec+ssfrac

    return sec*1000.

  def initMultiVolumes(self, files, prescribedTags=None):
    tag2ValueFileList = {}
    multivolumes = []

    if prescribedTags == None:
      consideredTags = self.multiVolumeTags.keys()
    else:
      consideredTags = prescribedTags

    # iterate over all files
    for file in files:

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
          continue

        if frameTag == 'AcquisitionTime' or frameTag == 'SeriesTime':
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
    for frameTag in self.multiVolumeTags.keys():

      try:
        tagValue2FileList = tag2ValueFileList[frameTag]
      except:
        # didn't find the tag
        continue

      if len(tagValue2FileList)<2:
        # not enough frames for this tag to be a multivolume
        continue

      tagValues = tagValue2FileList.keys()
      # sort the frames
      tagValues.sort()
      firstFrameSize = len(tagValue2FileList[tagValues[0]])
      frameInvalid = False
      for tagValue in tagValues:
        if len(tagValue2FileList[tagValue]) != firstFrameSize:
          # number of frames does not match

          frameInvalid = True
      if frameInvalid == True:
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
        if frameTag == 'SeriesTime' or frameTag == 'AcquisitionTime':
          frameLabelsArray.InsertNextValue(tagValue-tagValue0)
          frameLabelsStr = frameLabelsStr+str(tagValue-tagValue0)+','
        else:
          frameLabelsArray.InsertNextValue(tagValue)
          frameLabelsStr = frameLabelsStr+str(tagValue)+','

      frameFileListStr = frameFileListStr[:-1]
      frameLabelsStr = frameLabelsStr[:-1]

      mvNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLMultiVolumeNode')
      mvNode.SetReferenceCount(mvNode.GetReferenceCount()-1)
      mvNode.SetScene(slicer.mrmlScene)
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

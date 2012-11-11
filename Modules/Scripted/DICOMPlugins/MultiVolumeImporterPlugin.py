import os
import string
from __main__ import vtk, qt, ctk, slicer
from DICOMLib import DICOMPlugin
from DICOMLib import DICOMLoadable

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
    self.tags['position'] = "0020,0032"
    self.tags['studyDescription'] = "0008,1030"

    # tags used to identify multivolumes
    self.multiVolumeTags = {}
    self.multiVolumeTags['TriggerTime'] = "0018,1060"
    self.multiVolumeTags['EchoTime'] = "0018,0081"
    self.multiVolumeTags['FlipAngle'] = "0018,1314"
    self.multiVolumeTags['RepetitionTime'] = "0018,0080"
    self.multiVolumeTags['AcquisitionTime'] = "0008,0032"
    self.multiVolumeTags['SeriesTime'] = "0008,0031"

    for tagName,tagVal in self.multiVolumeTags.iteritems():
      self.tags[tagName] = tagVal

    self.multiVolumeTagsUnits = {}
    self.multiVolumeTagsUnits['TriggerTime'] = "ms"
    self.multiVolumeTagsUnits['EchoTime'] = "ms"
    self.multiVolumeTagsUnits['FlipAngle'] = "degrees"
    self.multiVolumeTagsUnits['RepetitionTime'] = "ms"
    self.multiVolumeTagsUnits['AcquisitionTime'] = "ms"
    self.multiVolumeTagsUnits['SeriesTime'] = "ms"

    self.epsilon = epsilon

  def examine(self,fileLists):
    """ Returns a list of DICOMLoadable instances
    corresponding to ways of interpreting the 
    fileLists parameter.
    """
    loadables = []
    allfiles = []
    for files in fileLists:
      loadables += self.examineFiles(files)
      allfiles += files

    # here all files are lumped into one list for the situations when
    # individual frames should be parsed from series
    loadables += self.examineFilesMultiseries(allfiles)

    return loadables

  def examineFilesMultiseries(self,files):

    print('MultiVolumeImportPlugin:examineMultiseries')
    loadables = []

    mvNodes = self.initMultiVolumes(files,prescribedTags=['SeriesTime'])

    print('DICOMMultiVolumePlugin found '+str(len(mvNodes))+' multivolumes!')

    for mvNode in mvNodes:
      tagName = mvNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagName')
      nFrames = mvNode.GetNumberOfFrames()
      orderedFiles = string.split(mvNode.GetAttribute('MultiVolume.FrameFileList'),',')

      desc = slicer.dicomDatabase.fileValue(orderedFiles[0],self.tags['studyDescription']) # SeriesDescription

      if self.isFrameOriginConsistent(orderedFiles, mvNode) == False:
        continue

      loadable = DICOMLib.DICOMLoadable()
      loadable.files = orderedFiles
      loadable.name =  str(nFrames) + ' frames MultiVolume by ' + tagName
      mvNode.SetName(desc)      
      loadable.tooltip = loadable.name
      loadable.selected = True
      loadable.multivolume = mvNode
      loadable.confidence = 0.9
      loadables.append(loadable)

    return loadables

  def examineFiles(self,files):

    print("MultiVolumeImportPlugin::examine")

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

      print('DICOMMultiVolumePlugin found '+str(len(mvNodes))+' multivolumes!')

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
      return

    nFrames = int(mvNode.GetAttribute('MultiVolume.NumberOfFrames'))
    files = string.split(mvNode.GetAttribute('MultiVolume.FrameFileList'),',')
    nFiles = len(files)
    filesPerFrame = nFiles/nFrames
    frames = []
    
    mvImage = vtk.vtkImageData()
    mvImageArray = None

    scalarVolumePlugin = slicer.modules.dicomPlugins['DICOMScalarVolumePlugin']()

    # read each frame into scalar volume
    for frameNumber in range(nFrames):
      
      sNode = slicer.vtkMRMLVolumeArchetypeStorageNode()
      sNode.ResetFileNameList();

      frameFileList = files[frameNumber*filesPerFrame:(frameNumber+1)*filesPerFrame]
      # sv plugin will sort the filenames by geometric order
      svLoadables = scalarVolumePlugin.examine([frameFileList])

      if len(svLoadables) == 0:
        return
      for f in svLoadables[0].files:
        sNode.AddFileName(f)

      sNode.SetFileName(frameFileList[0]) # only used when num files/frame = 1
      sNode.SetSingleFile(0)
      frame = slicer.vtkMRMLScalarVolumeNode()
      sNode.ReadData(frame)

      if frame == None:
        print('Failed to read a multivolume frame!')
        return False

      if frameNumber == 0:
        frameImage = frame.GetImageData()
        frameExtent = frameImage.GetExtent()
        frameSize = frameExtent[1]*frameExtent[3]*frameExtent[5]

        mvImage.SetExtent(frameExtent)
        mvImage.SetNumberOfScalarComponents(nFrames)

        mvImage.AllocateScalars()
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
    slicer.mrmlScene.AddNode(mvNode)

    return True

  def tm2ms(self,tm):
   
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
        else:
          tagValue = float(tagValueStr)
        
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

        frameLabelsStr = frameLabelsStr+str(tagValue)+','
        # if mv was parsed by series time, probably makes sense to start from
        # 0
        if frameTag == 'SeriesTime':
          frameLabelsArray.InsertNextValue(tagValue-tagValue0)
        else:
          frameLabelsArray.InsertNextValue(tagValue)

      #print 'File list: ',frameFileList
      #print 'Labels: ',frameLabelsStr

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

      if frameTag == 'TriggerTime' or frameTag == 'AcquisitionTime':
        # this is DCE, so let's keep the tag values that will be needed for
        # the analysis
        firstFile = frameFileList[0]
        echoTime = slicer.dicomDatabase.fileValue(firstFile, self.tags['EchoTime'])
        repetitionTime = slicer.dicomDatabase.fileValue(firstFile, self.tags['RepetitionTime'])
        flipAngle = slicer.dicomDatabase.fileValue(firstFile, self.tags['FlipAngle'])
        
        mvNode.SetAttribute('MultiVolume.DICOM.EchoTime',echoTime)
        mvNode.SetAttribute('MultiVolume.DICOM.RepetitionTime',repetitionTime)
        mvNode.SetAttribute('MultiVolume.DICOM.FlipAngle',flipAngle)

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

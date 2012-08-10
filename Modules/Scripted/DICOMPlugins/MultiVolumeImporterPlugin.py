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

    # tags used to identify multivolumes
    self.multiVolumeTags = {}
    self.multiVolumeTags['TriggerTime'] = "0018,1060"
    self.multiVolumeTags['EchoTime'] = "0018,0081"
    self.multiVolumeTags['FlipAngle'] = "0018,1314"
    self.multiVolumeTags['RepetitionTime'] = "0018,0080"

    for tagName,tagVal in self.multiVolumeTags.iteritems():
      self.tags[tagName] = tagVal

    self.multiVolumeTagsUnits = {}
    self.multiVolumeTagsUnits['TriggerTime'] = "ms"
    self.multiVolumeTagsUnits['EchoTime'] = "ms"
    self.multiVolumeTagsUnits['FlipAngle'] = "deg"
    self.multiVolumeTagsUnits['RepetitionTime'] = "ms"

  def examine(self,fileLists):
    """ Returns a list of DICOMLoadable instances
    corresponding to ways of interpreting the 
    fileLists parameter.
    """
    loadables = []
    for files in fileLists:
      loadables += self.examineFiles(files)
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
        
        loadable = DICOMLib.DICOMLoadable()
        loadable.files = files
        loadable.name = desc + ' - as a ' + str(nFrames) + ' frames MultiVolume by ' + tagName
        loadable.tooltip = loadable.name
        loadable.selected = True
        loadable.multivolume = mvNode
        loadables.append(loadable)

    return loadables

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
    volumesLogic = slicer.modules.volumes.logic()
    for frameNumber in range(nFrames):
      
      sNode = slicer.vtkMRMLVolumeArchetypeStorageNode()
      sNode.SetFileName(files[0])
      sNode.ResetFileNameList();

      frameFileList = files[frameNumber*filesPerFrame:(frameNumber+1)*filesPerFrame]
      # sv plugin will sort the filenames by geometric order
      svLoadables = scalarVolumePlugin.examine([frameFileList])

      if len(svLoadables) == 0:
        return
      for f in svLoadables[0].files:
        sNode.AddFileName(f)
      
      sNode.SetSingleFile(0)
      frame = slicer.vtkMRMLScalarVolumeNode()
      sNode.ReadData(frame)

      if frame == None:
        print('Failed to read a multivolume frame!')
        return False

      if frameNumber == 0:
        # initialize DWI node based on the parameters of the first frame
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
      self.annihilateScalarNode(frame)

    # create additional nodes that are needed for the DWI to be added to the scene
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

  # leave no trace of the temporary nodes
  def annihilateScalarNode(self, node):
    return
    dn = node.GetDisplayNode()
    sn = node.GetStorageNode()
    node.SetAndObserveDisplayNodeID(None)
    node.SetAndObserveStorageNodeID(None)
    slicer.mrmlScene.RemoveNode(dn)
    slicer.mrmlScene.RemoveNode(sn)
    slicer.mrmlScene.RemoveNode(node)

  def initMultiVolumes(self, files):
    tag2ValueFileList = {}
    multivolumes = []

    # iterate over all files
    for file in files:

      # iterate over the tags that can be used to separate individual frames
      for frameTag in self.multiVolumeTags.keys():
        try:
          tagValue2FileList = tag2ValueFileList[frameTag]
        except:
          tagValue2FileList = {}
          tag2ValueFileList[frameTag] = tagValue2FileList

        tagValue = slicer.dicomDatabase.fileValue(file,self.tags[frameTag])
        if tagValue == '':
          # not found?
          continue
        tagValue = float(tagValue)
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
        # not enough frames
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

      # now this looks like a serious mv!

      # initialize the needed attributes for a new mvNode
      frameFileListStr = ""
      frameLabelsStr = ""
      frameLabelsArray = vtk.vtkDoubleArray()
      for tagValue in tagValues:
        frameFileList = tagValue2FileList[tagValue]
        for file in frameFileList:
          frameFileListStr = frameFileListStr+file+','

        frameLabelsStr = frameLabelsStr+str(tagValue)+','
        frameLabelsArray.InsertNextValue(tagValue)

      print 'File list: ',frameFileList
      print 'Labels: ',frameLabelsStr

      frameFileListStr = frameFileListStr[:-1]
      frameLabelsStr = frameLabelsStr[:-1]

      mvNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLMultiVolumeNode')
      mvNode.SetReferenceCount(mvNode.GetReferenceCount()-1)
      mvNode.SetScene(slicer.mrmlScene)
      mvNode.SetAttribute("MultiVolume.FrameFileList",frameFileListStr)
      mvNode.SetAttribute("MultiVolume.FrameLabels",frameLabelsStr)
      mvNode.SetAttribute("MultiVolume.NumberOfFrames",str(len(tagValue2FileList)))
      mvNode.SetAttribute("MultiVolume.FrameIdentifyingDICOMTagName",frameTag)

      if frameTag == 'TriggerTime':
        # this is DCE, so let's keep the tag values that will be needed for
        # the analysis
        firstFile = frameFileList[0]
        echoTime = slicer.dicomDatabase.fileValue(firstFile, self.tags['EchoTime'])
        repetitionTime = slicer.dicomDatabase.fileValue(firstFile, self.tags['RepetitionTime'])
        flipAngle = slicer.dicomDatabase.fileValue(firstFile, self.tags['FlipAngle'])
        
        mvNode.SetAttribute('MultiVolume.DICOM.EchoTime',echoTime)
        mvNode.SetAttribute('MultiVolume.DICOM.RepetitionTime',repetitionTime)
        mvNode.SetAttribute('MultiVolume.DICOM.FlipAngle',flipAngle)
        mvNode.SetAttribute('MultiVolume.NumberOfFrames',str(len(tagValue2FileList)))
        
        mvNode.SetNumberOfFrames(len(tagValue2FileList))
        mvNode.SetLabelName(self.multiVolumeTagsUnits[frameTag])
        mvNode.SetLabelArray(frameLabelsArray)

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

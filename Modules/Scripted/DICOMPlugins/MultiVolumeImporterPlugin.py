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

    mvNode = None
    for key in subseriesLists.keys():
      if mvNode == None:
        # TODO: fix memory leaks here!
        mvNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLMultiVolumeNode')
        mvNode.SetName('MultiVolume node')
        mvNode.SetScene(slicer.mrmlScene)
      
      filevtkStringArray = vtk.vtkStringArray()
      for item in subseriesLists[key]:
        filevtkStringArray.InsertNextValue(item)

      mvNodes = self.initMultiVolumes(subseriesLists[key])

      if len(mvNodes) != 0:
        print 'Found ',len(mvNodes),' multivolumes! Here is the first one:'
        print mvNodes[0]

      return []
      
      # nFrames = slicer.modules.multivolumeexplorer.logic().InitializeMultivolumeNode(filevtkStringArray, mvNode)

      if nFrames > 1:
        tagName = mvNode.GetAttribute('MultiVolume.FrameIdentifyingDICOMTagName')
        loadable = DICOMLib.DICOMLoadable()
        loadable.files = files
        loadable.name = desc + ' - as a ' + str(nFrames) + ' frames MultiVolume by ' + tagName
        loadable.tooltip = loadable.name
        loadable.selected = True
        loadable.multivolume = mvNode
        loadables.append(loadable)

        mvNode = None
      else:
        print('No multivolumes found!')

    if mvNode != None:
      mvNode.SetReferenceCount(mvNode.GetReferenceCount()-1)

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

    print('MultiVolumeImportPlugin load()')
    # create a clean temporary directory
    # TODO: clean this up -- tmp dir is not used anymore!
    tmpDir = slicer.app.settings().value('Modules/TemporaryDirectory')
    if not os.path.exists(tmpDir):
      os.mkdir(tmpDir)
    tmpDir = tmpDir+'/MultiVolumeImportPlugin'
    if not os.path.exists(tmpDir):
      os.mkdir(tmpDir)
    else:
      # clean it up
      print 'tmpDir = '+tmpDir
      fileNames = os.listdir(tmpDir)
      for f in fileNames:
        os.unlink(tmpDir+'/'+f)

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

        # create and initialize a blank DWI node
        # TODO: need to clean up DWI-related junk!
        bValues = vtk.vtkDoubleArray()
        bValues.Allocate(nFrames)
        bValues.SetNumberOfComponents(1)
        bValues.SetNumberOfTuples(nFrames)
        gradients = vtk.vtkDoubleArray()
        gradients.Allocate(nFrames*3)
        gradients.SetNumberOfComponents(3)
        gradients.SetNumberOfTuples(nFrames)

        bValuesArray = vtk.util.numpy_support.vtk_to_numpy(bValues)
        gradientsArray = vtk.util.numpy_support.vtk_to_numpy(gradients)
        bValuesArray[:] = 0
        gradientsArray[:] = 1

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

    # create additional nodes that are needed for the DWI to be added to the
    # scene
    mvDisplayNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLMultiVolumeDisplayNode')
    mvDisplayNode.SetScene(slicer.mrmlScene)
    slicer.mrmlScene.AddNode(mvDisplayNode)
    mvDisplayNode.SetReferenceCount(mvDisplayNode.GetReferenceCount()-1)
    mvDisplayNode.SetDefaultColorMap()

    mvNode.SetAndObserveDisplayNodeID(mvDisplayNode.GetID())
    mvNode.SetAndObserveImageData(mvImage)
    mvNode.SetNumberOfFrames(nFrames)
    #mvNode.SetReferenceCount(mvNode.GetReferenceCount()-1)
    print("Number of frames :"+str(nFrames))

    slicer.mrmlScene.AddNode(mvNode)
    print('MV node added to the scene')

    mvNode.SetReferenceCount(mvNode.GetReferenceCount()-1)

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
  
      firstFrameSize = len(tagValue2FileList[tagValue2FileList.keys()[0]])
      frameInvalid = False
      for tagValue,frameFileList in tagValue2FileList.iteritems():
        if len(frameFileList) != firstFrameSize:
          # number of frames does not match
          frameInvalid = True
      if frameInvalid == True:
        continue

      # now this looks like a serious mv!
      print 'Found what seems to be a multivolume:'
      print tagValue2FileList

      # initialize the needed attributes for a new mvNode
      frameFileListStr = ""
      frameLabelsStr = ""
      frameLabelsArray = vtk.vtkDoubleArray()
      for tagValue,frameFileList in tagValue2FileList.iteritems():
        for file in frameFileList:
          frameFileListStr = frameFileListStr+file+','

        frameLabelsStr = frameLabelsStr+tagValue+','
        frameLabelsArray.InsertNextValue(float(tagValue))

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

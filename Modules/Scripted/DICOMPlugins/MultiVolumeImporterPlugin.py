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
    # AF TODO: run ProcessDICOMSeries with each of these tags?
    # AF TODO: need to re-read the header

    mvNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLMultiVolumeNode')
    mvNode.SetName('MultiVolume node')
    mvNode.SetScene(slicer.mrmlScene)
    nFrames = slicer.modules.multivolumeexplorer.logic().InitializeMultivolumeNode(os.path.dirname(files[0]), mvNode)

    if nFrames > 1:
      loadable = DICOMLib.DICOMLoadable()
      loadable.files = files
      loadable.name = str(nFrames) + ' frames Multi Volume'
      loadable.tooltip = str(nFrames) + ' frames Multi Volume'
      loadable.selected = True
      loadable.multivolume = mvNode
      loadables.append(loadable)
    else:
      print('No multivolumes found!')

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
    files = string.split(mvNode.GetAttribute('MultiVolume.FrameFileList'),' ')
    nFiles = len(files)
    filesPerFrame = nFiles/nFrames
    frames = []

    dwiImage = vtk.vtkImageData()
    dwiImageArray = None

    scalarVolumePlugin = slicer.modules.dicomPlugins['DICOMScalarVolumePlugin']()

    # read each frame into scalar volume
    volumesLogic = slicer.modules.volumes.logic()
    for frameNumber in range(nFrames):
      frameFileList = vtk.vtkStringArray()
      
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

        dwiImage.SetExtent(frameExtent)
        dwiImage.SetNumberOfScalarComponents(nFrames)

        dwiImage.AllocateScalars()
        dwiImageArray = vtk.util.numpy_support.vtk_to_numpy(dwiImage.GetPointData().GetScalars())

        # create and initialize a blank DWI node
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

        dwiNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLDiffusionWeightedVolumeNode')
        dwiNode.SetName('MultiVolume DisplayVolume')
        dwiNode.SetScene(slicer.mrmlScene)
        dwiNode.SetBValues(bValues)
        dwiNode.SetDiffusionGradients(gradients)

        mat = vtk.vtkMatrix4x4()
        frame.GetRASToIJKMatrix(mat)
        dwiNode.SetRASToIJKMatrix(mat)
        frame.GetIJKToRASMatrix(mat)
        dwiNode.SetIJKToRASMatrix(mat)

      frameImage = frame.GetImageData()
      frameImageArray = vtk.util.numpy_support.vtk_to_numpy(frameImage.GetPointData().GetScalars())
      dwiImageArray.T[frameNumber] = frameImageArray
      self.annihilateScalarNode(frame)

    # create additional nodes that are needed for the DWI to be added to the
    # scene
    dwiDisplayNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLDiffusionWeightedVolumeDisplayNode')
    dwiDisplayNode.SetScene(slicer.mrmlScene)
    slicer.mrmlScene.AddNode(dwiDisplayNode)
    dwiDisplayNode.SetReferenceCount(dwiDisplayNode.GetReferenceCount()-1)
    dwiDisplayNode.SetDefaultColorMap()

    dwiNode.SetAndObserveDisplayNodeID(dwiDisplayNode.GetID())
    dwiNode.SetAndObserveImageData(dwiImage)
    slicer.mrmlScene.AddNode(dwiNode)
    dwiNode.SetReferenceCount(dwiNode.GetReferenceCount()-1)
    print("Number of frames :"+str(nFrames))

    frameLabelsArray = vtk.vtkDoubleArray()
    frameLabelsArray.Allocate(nFrames)
    frameLabelsArray.SetNumberOfComponents(1)
    frameLabelsArray.SetNumberOfTuples(nFrames)
    i = 0
    '''
    frameLabelsStr = mvNode.GetAttribute('MultiVolume.FrameLabels')
    for label in string.split(frameLabelsStr, ' '):
      frameLabelsArray.SetComponent(i, 0, float(label))
      i = i+1
    '''
    mvNode.SetLabelArray(frameLabelsArray)
    mvNode.SetDWVNodeID(dwiNode.GetID())
    mvNode.SetLabelName('Label name')

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

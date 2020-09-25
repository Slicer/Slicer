import os
import string
from __main__ import vtk, qt, ctk, slicer
import logging
import numpy
try:
  import pydicom as dicom
except:
  # Slicer-4.10 backward compatibility
  import dicom

from DICOMLib import DICOMPlugin
from DICOMLib import DICOMLoadable

#
# This is the plugin to handle translation of DICOM objects
# that can be represented as multivolume objects
# from DICOM files into MRML nodes.  It follows the DICOM module's
# plugin architecture.
#

class DICOMImageSequencePluginClass(DICOMPlugin):
  """ 2D image sequence loader plugin.
  It supports X-ray angiography and ultrasound images.
  The main difference compared to plain scalar volume plugin is that it
  loads frames as a single-slice-volume sequence (and not as a 3D volume),
  it accepts color images, and handles multiple instances within a series
  (e.g., multiple independent acquisitions and synchronized biplane acquisitions).
  Limitation: ultrasound calibrated regions are not supported (each calibrated region
  would need to be split out to its own volume sequence).
  """

  def __init__(self):
    super(DICOMImageSequencePluginClass,self).__init__()
    self.loadType = "Image sequence"

    self.tags['sopClassUID'] = "0008,0016"

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
    """ Returns a list of DICOMLoadable instances
    corresponding to ways of interpreting the
    files parameter.
    """

    supportedSOPClassUIDs = [
      '1.2.840.10008.5.1.4.1.1.12.1',  # X-Ray Angiographic Image Storage
      '1.2.840.10008.5.1.4.1.1.3.1',  # Ultrasound Multiframe Image Storage
      ]

    # Each instance will be a loadable, that will result in one sequence browser node
    # and usually one sequence (except simultaneous biplane acquisition, which will
    # result in two sequences).
    # Each pedal press on the XA acquisition device creates a new instance number,
    # but if the device has two imaging planes (biplane) then two sequences
    # will be acquired, which have the same instance number. These two sequences
    # are synchronized in time, therefore they have to be assigned to the same
    # browser node.
    instanceNumberToLoadableIndex = {}

    loadables = []

    # Confidence is slightly larger than default scalar volume plugin's (0.5)
    # but still leaving room for more specialized plugins.
    confidence = 0.7

    for filePath in files:
      # Quick check of SOP class UID without parsing the file...
      try:
        sopClassUID = slicer.dicomDatabase.fileValue(filePath, self.tags['sopClassUID'])
        if not (sopClassUID in supportedSOPClassUIDs):
          # Unsupported class
          return []
      except Exception as e:
        # Quick check could not be completed (probably Slicer DICOM database is not initialized).
        # No problem, we'll try to parse the file and check the SOP class UID then.
        pass

      try:
        ds = dicom.read_file(filePath, stop_before_pixels=True)
      except Exception as e:
        logging.debug("Failed to parse DICOM file: {0}".format(str(e)))
        return []

      if not (ds.SOPClassUID in supportedSOPClassUIDs):
        # Unsupported class
        return []

      if not (ds.InstanceNumber in instanceNumberToLoadableIndex.keys()):
        # new instance number
        name = ''
        if hasattr(ds, 'SeriesNumber') and ds.SeriesNumber:
          name = '{0}:'.format(ds.SeriesNumber)
        if hasattr(ds, 'Modality') and ds.Modality:
          name = '{0} {1}'.format(name, ds.Modality)
        if hasattr(ds, 'SeriesDescription') and ds.SeriesDescription:
          name = '{0} {1}'.format(name, ds.SeriesDescription)
        if hasattr(ds, 'InstanceNumber') and ds.InstanceNumber:
          name = '{0} [{1}]'.format(name, ds.InstanceNumber)

        loadable = DICOMLoadable()
        loadable.files = [filePath]
        loadable.name = name.strip()  # remove leading and trailing spaces, if any
        loadable.warning = "Image spacing may need to be calibrated for accurate size measurements."
        loadable.tooltip = "{0} image sequence".format(ds.Modality)
        loadable.selected = True
        loadable.confidence = confidence
        loadable.grayscale = ('MONOCHROME' in ds.PhotometricInterpretation)

        # Add to loadables list
        loadables.append(loadable)
        instanceNumberToLoadableIndex[ds.InstanceNumber] = len(loadables)-1
      else:
        # existing instance number, add this file
        loadableIndex = instanceNumberToLoadableIndex[ds.InstanceNumber]
        loadables[loadableIndex].files.append(filePath)
        loadable.tooltip = "{0} image sequence ({1} planes)".format(ds.Modality, len(loadables[loadableIndex].files))

    return loadables

  def load(self,loadable):
    """Load the selection
    """

    outputSequenceNodes = []
    for filePath in loadable.files:

      import vtkITK
      if loadable.grayscale:
        reader = vtkITK.vtkITKArchetypeImageSeriesScalarReader()
      else:
        reader = vtkITK.vtkITKArchetypeImageSeriesVectorReaderFile()
      reader.SetArchetype(filePath)
      reader.AddFileName(filePath)
      reader.SetSingleFile(True)
      reader.SetOutputScalarTypeToNative()
      reader.SetDesiredCoordinateOrientationToNative()
      reader.SetUseNativeOriginOn()
      # GDCM is not particularly better in this than DCMTK, we just select one explicitly
      # so that we know which one is used
      reader.SetDICOMImageIOApproachToGDCM()
      reader.Update()
      if reader.GetErrorCode() != vtk.vtkErrorCode.NoError:
          errorString = vtk.vtkErrorCode.GetStringFromErrorCode(reader.GetErrorCode())
          raise ValueError("Could not read image {0} from file {1}. Error is: {2}".format(loadable.name, filePath, errorString))

      volume = reader.GetOutput()

      [spacingX, spacingY, frameTimeMsec] = volume.GetSpacing()

      volume.SetSpacing(1.0, 1.0, 1.0)

      outputSequenceNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSequenceNode")
      if frameTimeMsec == 1.0:
        # frame time is not found, set it to 1.0fps
        frameTime = 1
        outputSequenceNode.SetIndexName("frame")
        outputSequenceNode.SetIndexUnit("")
        playbackRateFps = 10
      else:
        frameTime = frameTimeMsec * 0.001
        outputSequenceNode.SetIndexName("time")
        outputSequenceNode.SetIndexUnit("s")
        playbackRateFps = 1.0/frameTime

      if loadable.grayscale:
        tempFrameVolume = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScalarVolumeNode")
      else:
        tempFrameVolume = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLVectorVolumeNode")

      # Rotate 180deg, otherwise the image would appear upside down
      ijkToRAS = vtk.vtkMatrix4x4()
      ijkToRAS.SetElement(0,0,-1.0)
      ijkToRAS.SetElement(1,1,-1.0)
      tempFrameVolume.SetIJKToRASMatrix(ijkToRAS)

      tempFrameVolume.SetSpacing(spacingX, spacingY, 1.0)

      numberOfFrames = volume.GetDimensions()[2]
      extent = volume.GetExtent()
      numberOfFrames = extent[5]-extent[4]+1
      for frame in range(numberOfFrames):

        crop = vtk.vtkImageClip()
        crop.SetInputData(volume)
        crop.SetOutputWholeExtent(extent[0], extent[1], extent[2], extent[3], extent[4]+frame, extent[4]+frame)
        crop.ClipDataOn()
        crop.Update()
        croppedOutput = crop.GetOutput()
        croppedOutput.SetExtent(extent[0], extent[1], extent[2], extent[3], 0, 0)
        tempFrameVolume.SetAndObserveImageData(croppedOutput)
        if type(frameTime)==int:
          timeStampSec = str(frame * frameTime)
        else:
          timeStampSec = "{:.3f}".format(frame * frameTime)
        outputSequenceNode.SetDataNodeAtValue(tempFrameVolume, timeStampSec)

      slicer.mrmlScene.RemoveNode(tempFrameVolume)

      if len(loadable.files) == 1:
        outputSequenceNode.SetName(loadable.name)
      else:
        ds = dicom.read_file(filePath, stop_before_pixels=True)
        if hasattr(ds, 'PositionerPrimaryAngle') and hasattr(ds, 'PositionerSecondaryAngle'):
          outputSequenceNode.SetName('{0} ({1}/{2})'.format(loadable.name, ds.PositionerPrimaryAngle,ds.PositionerSecondaryAngle))
        else:
          outputSequenceNode.SetName(loadable.name)

      slicer.mrmlScene.AddNode(outputSequenceNode)

      # Create storage node that allows saving node as nrrd
      outputSequenceStorageNode = slicer.vtkMRMLVolumeSequenceStorageNode()
      slicer.mrmlScene.AddNode(outputSequenceStorageNode)
      outputSequenceNode.SetAndObserveStorageNodeID(outputSequenceStorageNode.GetID())

      outputSequenceNodes.append(outputSequenceNode)

    if not hasattr(loadable, 'createBrowserNode') or loadable.createBrowserNode:
      # Add a browser node and show the volume in the slice viewer for user convenience
      outputSequenceBrowserNode = slicer.vtkMRMLSequenceBrowserNode()
      outputSequenceBrowserNode.SetName(slicer.mrmlScene.GenerateUniqueName(loadable.name+' browser'))
      outputSequenceBrowserNode.SetPlaybackRateFps(playbackRateFps)
      slicer.mrmlScene.AddNode(outputSequenceBrowserNode)

      # Add all sequences to the sequence browser
      first = True
      for outputSequenceNode in outputSequenceNodes:
        #outputSequenceBrowserNode.SetAndObserveMasterSequenceNodeID(outputSequenceNode.GetID())
        outputSequenceBrowserNode.AddSynchronizedSequenceNode(outputSequenceNode)
        proxyVolumeNode = outputSequenceBrowserNode.GetProxyNode(outputSequenceNode)
        # create Subject hierarchy nodes for the loaded series
        self.addSeriesInSubjectHierarchy(loadable, proxyVolumeNode)

        if first:
          first = False
          # Automatically select the volume to display
          appLogic = slicer.app.applicationLogic()
          selNode = appLogic.GetSelectionNode()
          selNode.SetReferenceActiveVolumeID(proxyVolumeNode.GetID())
          appLogic.PropagateVolumeSelection()
          appLogic.FitSliceToAll()
          slicer.modules.sequences.setToolBarActiveBrowserNode(outputSequenceBrowserNode)

      # Show sequence browser toolbar
      slicer.modules.sequences.showSequenceBrowser(outputSequenceBrowserNode)

    return outputSequenceNode


#
# DICOMImageSequencePlugin
#

class DICOMImageSequencePlugin:
  """
  This class is the 'hook' for slicer to detect and recognize the plugin
  as a loadable scripted module
  """
  def __init__(self, parent):
    parent.title = "DICOM Image Sequence Import Plugin"
    parent.categories = ["Developer Tools.DICOM Plugins"]
    parent.contributors = ["Andras Lasso (PerkLab)"]
    parent.helpText = """
    Plugin to the DICOM Module to parse and load 2D image sequences.
    No module interface here, only in the DICOM module.
    """
    parent.acknowledgementText = """
    The file was originally developed by Andras Lasso (PerkLab).
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
    slicer.modules.dicomPlugins['DICOMImageSequencePlugin'] = DICOMImageSequencePluginClass

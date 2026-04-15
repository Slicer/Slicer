from __future__ import absolute_import
import glob
import os
import json
import vtk
import io
import shutil
import vtkSegmentationCorePython as vtkSegmentationCore
import logging
import pydicom

import slicer
from DICOMLib import DICOMLoadable, DICOMPlugin


#
# This is the plugin to handle translation of DICOM M3D objects
# M3D stands for Model for 3D Manufacturing.
# DICOM M3D objects can be for example .STL files encoded into a DICOM M3D object.
#
class DICOMM3DPluginClass(DICOMPlugin):

  def __init__(self):
    super(DICOMM3DPluginClass, self).__init__()
    self.loadType = "DICOMM3D"

  def examineFiles(self, files):
    """ Returns a list of DICOMLoadable instances
    corresponding to ways of interpreting the
    files parameter.
    """
    loadables = []

    for candidateFile in files:
      #read modality type to flag M3D object.
      isDicomM3D = (slicer.dicomDatabase.fileValue(candidateFile, self.tags['modality']) == 'M3D')
      if isDicomM3D:
        uid = slicer.dicomDatabase.fileValue(candidateFile, self.tags['instanceUID'])
        if uid == '':
          return []

        desc = slicer.dicomDatabase.fileValue(candidateFile, self.tags['seriesDescription'])
        if desc == '':
          desc = "Unknown"

        loadable = DICOMLoadable()
        loadable.files = [candidateFile]
        loadable.name = desc
        loadable.tooltip = loadable.name + ' - as a DICOM M3D object'
        loadable.selected = True
        loadable.confidence = 0.95
        loadable.uid = uid
        loadables.append(loadable)

        logging.debug('DICOM M3D modality found')

    return loadables

  def getFrameOfReferenceUID(self, candidateFile):
    """Returns the frame of referenceUID for the given loadable"""
    dcm = pydicom.dcmread(candidateFile)
    if hasattr(dcm, "FrameOfReferenceUID"):
      return dcm.FrameOfReferenceUID
    else:
      return 'Unnamed FrameOfReferenceUID'

  def getEncapsulatedDocumentAttributes(self, candidateFile):
    dcm = pydicom.dcmread(candidateFile)
    encapsulatedDocument = b''
    encapsulatedDocumentLength = 0
    if hasattr(dcm, "EncapsulatedDocument"):
      encapsulatedDocument = dcm.EncapsulatedDocument
    if hasattr(dcm, "EncapsulatedDocumentLength"):
      encapsulatedDocumentLength = dcm.EncapsulatedDocumentLength
    return encapsulatedDocument, encapsulatedDocumentLength

  def load(self, loadable):
    """ Load the DICOM M3D object
    """
    logging.debug('DICOM M3D load()')
    if hasattr(loadable, "uid"):
      uid = loadable.uid
      logging.debug('in load(): uid = ' + uid)
    else:
      return False

    self.tempDir = slicer.util.tempDirectory()
    try:
      os.makedirs(self.tempDir)
    except OSError:
      pass

    stlFileName = slicer.dicomDatabase.fileForInstance(uid)
    if stlFileName is None:
      logging.error('Failed to get the filename from the DICOM database for ' + uid)
      self.cleanup()
      return False

    #Retrieve EncapsulatedDocument and EncapsulatedDocumentLength for candidateFile
    docFile, docLengthFile = self.getEncapsulatedDocumentAttributes(loadable.files[0])

    # Read Encapsulated Document attribute into binary IO buffer
    read_buffer = io.BytesIO(docFile)

    # Take care of trailing padding in Encapsulated Document
    # If EncapsulatedDocumentLength is odd,
    # the EncapsulatedDocument was padded to even length, otherwise it was already of even length.
    buffer_view = read_buffer.getbuffer()
    if (int(docLengthFile) % 2) != 0:
      buffer_view = buffer_view[0:(read_buffer.getbuffer().nbytes - 1)]

    # Save processed binary IO buffer into a temporary .STL file
    stlFilePath = os.path.join(self.tempDir, "temp.STL")
    with open(stlFilePath, 'wb') as file:
      shutil.copyfileobj(read_buffer, file)
    assert os.path.exists(stlFilePath)
    file.close()

    # Create Segmentation Node based on .STL file
    self._createSegmentationNode(loadable, stlFilePath)

    self.cleanup()

    return True

  def _createSegmentationNode(self, loadable, stlFilePath):
    #create modelNode
    modelNode = slicer.util.loadModel(stlFilePath)

    # Initialize Segmentation Node
    segmentationNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSegmentationNode")
    segmentationNode.SetName(loadable.name)

    # Initialize Segmentation Display Node
    segmentationDisplayNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSegmentationDisplayNode")
    segmentationNode.SetAndObserveDisplayNodeID(segmentationDisplayNode.GetID())

    #Setup converters for further export to labelmap or model representations
    vtkSegConverter = vtkSegmentationCore.vtkSegmentationConverter
    segmentation = vtkSegmentationCore.vtkSegmentation()
    segmentation.SetSourceRepresentationName(vtkSegConverter.GetSegmentationClosedSurfaceRepresentationName())
    segmentation.CreateRepresentation(vtkSegConverter.GetSegmentationBinaryLabelmapRepresentationName(), True)
    segmentationNode.SetAndObserveSegmentation(segmentation)

    #Load Model Node into Segmentation Node
    self._importModelToSegAndRemoveModel(modelNode, segmentationNode)
    self.addSeriesInSubjectHierarchy(loadable, segmentationNode)
    return segmentationNode

  def _importModelToSegAndRemoveModel(self, ModelNode, segmentationNode):
    segmentationsLogic = slicer.modules.segmentations.logic()
    segmentation = segmentationNode.GetSegmentation()
    numberOfSegmentsBeforeImport = segmentation.GetNumberOfSegments()
    success = segmentationsLogic.ImportModelToSegmentationNode(ModelNode, segmentationNode)
    if segmentation.GetNumberOfSegments() == 0:
      logging.warning("Empty segment loaded from DICOM SEG!")
    if segmentation.GetNumberOfSegments() - numberOfSegmentsBeforeImport > 1:
      logging.warning("Multiple segments were loaded from DICOM SEG labelmap. Only one label was expected.")
    if success and segmentation.GetNumberOfSegments() > 0:
      segment = segmentation.GetNthSegment(segmentation.GetNumberOfSegments() - 1)
      segment.SetName("Segment 1")
      segment.SetNameAutoGenerated(False)
      self._removeModelNode(ModelNode)
    return segmentation

  def _removeModelNode(self, modelNode):
    #Model Node being temporary, we remove here the node from the scene
    dNode = modelNode.GetDisplayNode()
    if dNode is not None:
      slicer.mrmlScene.RemoveNode(dNode)
    slicer.mrmlScene.RemoveNode(modelNode)


class DICOMM3DPlugin:
  """
  This class is the 'hook' for slicer to detect and recognize the plugin
  as a loadable scripted module
  """
  def __init__(self, parent):
    parent.title = "DICOM M3D Object Import Plugin"
    parent.categories = ["Developer Tools.DICOM Plugins"]
    parent.contributors = ["Cosmin Ciausu, BWH"]
    parent.helpText = """
    Plugin to the DICOM Module to parse and load DICOM M3D modality.
    No module interface here, only in the DICOM module
    """
    parent.dependencies = ['DICOM', 'Colors']
    parent.acknowledgementText = """
    This DICOM Plugin was developed by
    Cosmin Ciausu, BWH.
    """

    # Add this extension to the DICOM module's list for discovery when the module
    # is created.  Since this module may be discovered before DICOM itself,
    # create the list if it doesn't already exist.
    try:
      slicer.modules.dicomPlugins
    except AttributeError:
      slicer.modules.dicomPlugins = {}
    slicer.modules.dicomPlugins['DICOMM3DPlugin'] = DICOMM3DPluginClass

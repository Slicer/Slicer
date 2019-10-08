import os
import unittest
import vtk, qt, ctk, slicer
import logging
from slicer.ScriptedLoadableModule import *

import vtkSegmentationCore

'''
This class tests the shared labelmap behavior.
Edting segments with overwrite should keep all segments on the same labelmap.
Editing segments with overlap should split segments to separate layers.
Shared segment modification behavior is controlled through qSlicerSegmentEditorAbstractEffect::modifySelectedSegmentByLabelmap().
'''

class SegmentationsModuleTest2(unittest.TestCase):

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_SegmentationsModuleTest2()

  def test_SegmentationsModuleTest2(self):
    # Check for modules
    self.assertIsNotNone( slicer.modules.segmentations )
    self.assertIsNotNone( slicer.modules.segmenteditor )

    # Run tests
    self.TestSection_SetupPathsAndNames()
    self.TestSection_RetrieveInputData()
    self.TestSection_SetupScene()
    self.TestSection_TestSharedLabelmapMultipleLayerEditing()
    logging.info('Test finished')

  #------------------------------------------------------------------------------
  def TestSection_SetupPathsAndNames(self):
    # Set up paths used for this test
    self.segmentationsModuleTestDir = slicer.app.temporaryPath + '/SegmentationsModuleTest'
    if not os.access(self.segmentationsModuleTestDir, os.F_OK):
      os.mkdir(self.segmentationsModuleTestDir)

    self.dataDir = self.segmentationsModuleTestDir + '/TinyPatient_Seg'
    if not os.access(self.dataDir, os.F_OK):
      os.mkdir(self.dataDir)
    self.dataSegDir = self.dataDir + '/TinyPatient_Structures.seg'

    self.dataZipFilePath = self.segmentationsModuleTestDir + '/TinyPatient_Seg.zip'

    # Define variables
    self.expectedNumOfFilesInDataDir = 4
    self.expectedNumOfFilesInDataSegDir = 2
    self.inputSegmentationNode = None
    self.bodySegmentName = 'Body_Contour'
    self.tumorSegmentName = 'Tumor_Contour'
    self.secondSegmentationNode = None
    self.sphereSegment = None
    self.sphereSegmentName = 'Sphere'
    self.closedSurfaceReprName = vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationClosedSurfaceRepresentationName()
    self.binaryLabelmapReprName = vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationBinaryLabelmapRepresentationName()

  #------------------------------------------------------------------------------
  def TestSection_RetrieveInputData(self):
    try:
      slicer.util.downloadAndExtractArchive(
        'http://slicer.kitware.com/midas3/download/folder/3763/TinyPatient_Seg.zip', self.dataZipFilePath, self.segmentationsModuleTestDir,
        checksum='SHA256:b902f635ef2059cd3b4ba854c000b388e4a9e817a651f28be05c22511a317ec7')

      numOfFilesInDataDirTest = len([name for name in os.listdir(self.dataDir) if os.path.isfile(self.dataDir + '/' + name)])
      self.assertEqual( numOfFilesInDataDirTest, self.expectedNumOfFilesInDataDir )
      self.assertTrue( os.access(self.dataSegDir, os.F_OK) )
      numOfFilesInDataSegDirTest = len([name for name in os.listdir(self.dataSegDir) if os.path.isfile(self.dataSegDir + '/' + name)])
      self.assertEqual( numOfFilesInDataSegDirTest, self.expectedNumOfFilesInDataSegDir )

    except Exception as e:
      import traceback
      traceback.print_exc()
      logging.error('Test caused exception!\n' + str(e))

  #------------------------------------------------------------------------------
  def TestSection_SetupScene(self):

    self.paintEffect = slicer.modules.segmenteditor.widgetRepresentation().self().editor.effectByName("Paint")

    self.segmentEditorNode = slicer.util.getNode("SegmentEditor")
    self.assertIsNotNone(self.segmentEditorNode)

    self.segmentationNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSegmentationNode")
    self.assertIsNotNone(self.segmentationNode)
    self.segmentEditorNode.SetAndObserveSegmentationNode(self.segmentationNode)

    self.masterVolumeNode = slicer.util.loadVolume(self.dataDir + '/TinyPatient_CT.nrrd')
    self.assertIsNotNone(self.masterVolumeNode)
    self.segmentEditorNode.SetAndObserveMasterVolumeNode(self.masterVolumeNode)

    self.segmentation = self.segmentationNode.GetSegmentation()
    self.segmentation.SetMasterRepresentationName(self.binaryLabelmapReprName)
    self.assertIsNotNone(self.segmentation)

  #------------------------------------------------------------------------------
  def TestSection_TestSharedLabelmapMultipleLayerEditing(self):

    self.segmentation.RemoveAllSegments()
    self.segmentation.AddEmptySegment("Segment_1")
    self.segmentation.AddEmptySegment("Segment_2")

    mergedLabelmap = vtkSegmentationCore.vtkOrientedImageData()
    mergedLabelmap.SetExtent(0, 10, 0, 10, 0, 10)
    mergedLabelmap.AllocateScalars(vtk.VTK_UNSIGNED_CHAR, 1)

    threshold = vtk.vtkImageThreshold()
    threshold.SetInputData(mergedLabelmap)
    threshold.ThresholdBetween(vtk.VTK_UNSIGNED_CHAR_MIN, vtk.VTK_UNSIGNED_CHAR_MAX)
    threshold.SetInValue(1)
    threshold.SetOutValue(0)
    threshold.Update()
    mergedLabelmap.ShallowCopy(threshold.GetOutput())

    self.segmentEditorNode.SetSelectedSegmentID("Segment_1")
    self.paintEffect.modifySelectedSegmentByLabelmap(mergedLabelmap, self.paintEffect.ModificationModeAdd)
    self.segmentEditorNode.SetSelectedSegmentID("Segment_2")
    self.paintEffect.modifySelectedSegmentByLabelmap(mergedLabelmap, self.paintEffect.ModificationModeAdd)

    layerCount = self.segmentation.GetNumberOfLayers()
    self.assertEqual(layerCount, 1)

    self.segmentEditorNode.SetOverwriteMode(self.segmentEditorNode.OverwriteNone)
    self.segmentEditorNode.SetSelectedSegmentID("Segment_1")
    self.paintEffect.modifySelectedSegmentByLabelmap(mergedLabelmap, self.paintEffect.ModificationModeAdd)
    layerCount = self.segmentation.GetNumberOfLayers()
    self.assertEqual(layerCount, 2)

    logging.info('Multiple layer editing successful')

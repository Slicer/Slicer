import os
import unittest
import vtk, qt, ctk, slicer
import logging
from slicer.ScriptedLoadableModule import *
from slicer.util import TESTING_DATA_URL

import vtkSegmentationCore

'''
This class tests the shared labelmap behavior.
Edting segments with overwrite should keep all segments on the same labelmap.
Editing segments with overlap should split segments to separate layers.
Shared segment modification behavior is controlled through qSlicerSegmentEditorAbstractEffect::modifySelectedSegmentByLabelmap().
'''

class SegmentationsModuleTest2(unittest.TestCase):

  #------------------------------------------------------------------------------
  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  #------------------------------------------------------------------------------
  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_SegmentationsModuleTest2()

  #------------------------------------------------------------------------------
  def test_SegmentationsModuleTest2(self):
    # Check for modules
    self.assertIsNotNone( slicer.modules.segmentations )
    self.assertIsNotNone( slicer.modules.segmenteditor )

    # Run tests
    self.TestSection_SetupPathsAndNames()
    self.TestSection_RetrieveInputData()
    self.TestSection_SetupScene()
    self.TestSection_SharedLabelmapMultipleLayerEditing()
    self.TestSection_IslandEffects()
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
    self.closedSurfaceReprName = vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationClosedSurfaceRepresentationName()
    self.binaryLabelmapReprName = vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationBinaryLabelmapRepresentationName()

  #------------------------------------------------------------------------------
  def TestSection_RetrieveInputData(self):
    try:
      slicer.util.downloadAndExtractArchive(
        TESTING_DATA_URL + 'SHA256/b902f635ef2059cd3b4ba854c000b388e4a9e817a651f28be05c22511a317ec7',
        self.dataZipFilePath, self.segmentationsModuleTestDir,
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
    self.islandEffect = slicer.modules.segmenteditor.widgetRepresentation().self().editor.effectByName("Islands")

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
  def TestSection_SharedLabelmapMultipleLayerEditing(self):

    self.segmentation.RemoveAllSegments()
    self.segmentation.AddEmptySegment("Segment_1")
    self.segmentation.AddEmptySegment("Segment_2")

    defaultModifierLabelmap = self.paintEffect.defaultModifierLabelmap()
    self.ijkToRas = vtk.vtkMatrix4x4()
    defaultModifierLabelmap.GetImageToWorldMatrix(self.ijkToRas)


    mergedLabelmap = vtkSegmentationCore.vtkOrientedImageData()
    mergedLabelmap.SetImageToWorldMatrix(self.ijkToRas)
    mergedLabelmap.SetExtent(0, 10, 0, 10, 0, 10)
    mergedLabelmap.AllocateScalars(vtk.VTK_UNSIGNED_CHAR, 1)
    mergedLabelmap.GetPointData().GetScalars().Fill(1)

    oldOverwriteMode = self.segmentEditorNode.GetOverwriteMode()

    self.segmentEditorNode.SetOverwriteMode(self.segmentEditorNode.OverwriteAllSegments)
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

    self.segmentEditorNode.SetOverwriteMode(oldOverwriteMode)
    logging.info('Multiple layer editing successful')

  #------------------------------------------------------------------------------
  def TestSection_IslandEffects(self):

    islandSizes = [1, 26, 11, 6, 8, 6, 2]
    islandSizes.sort(reverse=True)

    minimumSize = 3
    self.resetIslandSegments(islandSizes)
    self.islandEffect.setParameter('MinimumSize', minimumSize)
    self.islandEffect.setParameter('Operation','KEEP_LARGEST_ISLAND')
    self.islandEffect.self().onApply()
    layerCount = self.segmentation.GetNumberOfLayers()
    self.assertEqual(layerCount, 1)

    voxelCount = 0
    for size in islandSizes:
      if size < minimumSize:
          continue
      voxelCount = max(voxelCount, size)
    self.checkSegmentVoxelCount(0, voxelCount)

    minimumSize = 7
    self.resetIslandSegments(islandSizes)
    self.islandEffect.setParameter('MinimumSize', minimumSize)
    self.islandEffect.setParameter('Operation','REMOVE_SMALL_ISLANDS')
    self.islandEffect.self().onApply()
    layerCount = self.segmentation.GetNumberOfLayers()
    self.assertEqual(layerCount, 1)

    voxelCount = 0
    for size in islandSizes:
      if size < minimumSize:
        continue
      voxelCount += size
    self.checkSegmentVoxelCount(0, voxelCount)

    self.resetIslandSegments(islandSizes)
    minimumSize = 3
    self.islandEffect.setParameter('MinimumSize', minimumSize)
    self.islandEffect.setParameter('Operation','SPLIT_ISLANDS_TO_SEGMENTS')
    self.islandEffect.self().onApply()
    layerCount = self.segmentation.GetNumberOfLayers()
    self.assertEqual(layerCount, 1)

    for i in range(len(islandSizes)):
      size = islandSizes[i]
      if size < minimumSize:
        continue
      self.checkSegmentVoxelCount(i, size)

  #------------------------------------------------------------------------------
  def resetIslandSegments(self, islandSizes):
    self.segmentation.RemoveAllSegments()

    totalSize = 0
    voxelSizeSum = 0
    for size in islandSizes:
      totalSize += size + 1
      voxelSizeSum += size

    mergedLabelmap = vtkSegmentationCore.vtkOrientedImageData()
    mergedLabelmap.SetImageToWorldMatrix(self.ijkToRas)
    mergedLabelmapExtent = [0, totalSize-1, 0, 0, 0, 0]
    self.setupIslandLabelmap(mergedLabelmap, mergedLabelmapExtent, 0)

    emptySegment = slicer.vtkSegment()
    emptySegment.SetName("Segment_1")
    emptySegment.AddRepresentation(self.binaryLabelmapReprName, mergedLabelmap)
    self.segmentation.AddSegment(emptySegment)
    self.segmentEditorNode.SetSelectedSegmentID("Segment_1")

    startExtent = 0
    for size in islandSizes:
      islandLabelmap = vtkSegmentationCore.vtkOrientedImageData()
      islandLabelmap.SetImageToWorldMatrix(self.ijkToRas)
      islandExtent = [startExtent, startExtent+size-1, 0, 0, 0, 0]
      self.setupIslandLabelmap(islandLabelmap, islandExtent)
      self.paintEffect.modifySelectedSegmentByLabelmap(islandLabelmap, self.paintEffect.ModificationModeAdd)
      startExtent += size + 1
    self.checkSegmentVoxelCount(0, voxelSizeSum)


    layerCount = self.segmentation.GetNumberOfLayers()
    self.assertEqual(layerCount, 1)

  #------------------------------------------------------------------------------
  def checkSegmentVoxelCount(self, segmentIndex, expectedVoxelCount):
    segment = self.segmentation.GetNthSegment(segmentIndex)
    self.assertIsNotNone(segment)

    labelmap = slicer.vtkOrientedImageData()
    labelmap.SetImageToWorldMatrix(self.ijkToRas)
    segmentID = self.segmentation.GetNthSegmentID(segmentIndex)
    self.segmentationNode.GetBinaryLabelmapRepresentation(segmentID, labelmap)

    imageStat = vtk.vtkImageAccumulate()
    imageStat.SetInputData(labelmap)
    imageStat.SetComponentExtent(0,4,0,0,0,0)
    imageStat.SetComponentOrigin(0,0,0)
    imageStat.SetComponentSpacing(1,1,1)
    imageStat.IgnoreZeroOn()
    imageStat.Update()

    self.assertEqual(imageStat.GetVoxelCount(), expectedVoxelCount)

  #------------------------------------------------------------------------------
  def setupIslandLabelmap(self, labelmap, extent, value=1):
    labelmap.SetExtent(extent)
    labelmap.AllocateScalars(vtk.VTK_UNSIGNED_CHAR, 1)
    labelmap.GetPointData().GetScalars().Fill(value)

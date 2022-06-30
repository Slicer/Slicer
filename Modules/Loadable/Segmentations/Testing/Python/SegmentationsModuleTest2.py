import logging
import os
import unittest

import vtk
import vtkSegmentationCore

import slicer
from slicer.util import TESTING_DATA_URL

'''
This class tests the shared labelmap behavior.
Edting segments with overwrite should keep all segments on the same labelmap.
Editing segments with overlap should split segments to separate layers.
Shared segment modification behavior is controlled through qSlicerSegmentEditorAbstractEffect::modifySelectedSegmentByLabelmap().
'''


class SegmentationsModuleTest2(unittest.TestCase):

    # ------------------------------------------------------------------------------
    def setUp(self):
        """ Do whatever is needed to reset the state - typically a scene clear will be enough.
        """
        slicer.mrmlScene.Clear(0)

    # ------------------------------------------------------------------------------
    def runTest(self):
        """Run as few or as many tests as needed here.
        """
        self.setUp()
        self.test_SegmentationsModuleTest2()

    # ------------------------------------------------------------------------------
    def test_SegmentationsModuleTest2(self):
        # Check for modules
        self.assertIsNotNone(slicer.modules.segmentations)
        self.assertIsNotNone(slicer.modules.segmenteditor)

        # Run tests
        self.TestSection_SetupPathsAndNames()
        self.TestSection_RetrieveInputData()
        self.TestSection_SetupScene()
        self.TestSection_SharedLabelmapMultipleLayerEditing()
        self.TestSection_IslandEffects()
        self.TestSection_MarginEffects()
        self.TestSection_MaskingSettings()
        logging.info('Test finished')

    # ------------------------------------------------------------------------------
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

    # ------------------------------------------------------------------------------
    def TestSection_RetrieveInputData(self):
        try:
            slicer.util.downloadAndExtractArchive(
                TESTING_DATA_URL + 'SHA256/b902f635ef2059cd3b4ba854c000b388e4a9e817a651f28be05c22511a317ec7',
                self.dataZipFilePath, self.segmentationsModuleTestDir,
                checksum='SHA256:b902f635ef2059cd3b4ba854c000b388e4a9e817a651f28be05c22511a317ec7')

            numOfFilesInDataDirTest = len([name for name in os.listdir(self.dataDir) if os.path.isfile(self.dataDir + '/' + name)])
            self.assertEqual(numOfFilesInDataDirTest, self.expectedNumOfFilesInDataDir)
            self.assertTrue(os.access(self.dataSegDir, os.F_OK))
            numOfFilesInDataSegDirTest = len([name for name in os.listdir(self.dataSegDir) if os.path.isfile(self.dataSegDir + '/' + name)])
            self.assertEqual(numOfFilesInDataSegDirTest, self.expectedNumOfFilesInDataSegDir)

        except Exception as e:
            import traceback
            traceback.print_exc()
            logging.error('Test caused exception!\n' + str(e))

    # ------------------------------------------------------------------------------
    def TestSection_SetupScene(self):
        self.paintEffect = slicer.modules.segmenteditor.widgetRepresentation().self().editor.effectByName("Paint")
        self.eraseEffect = slicer.modules.segmenteditor.widgetRepresentation().self().editor.effectByName("Erase")
        self.islandEffect = slicer.modules.segmenteditor.widgetRepresentation().self().editor.effectByName("Islands")
        self.thresholdEffect = slicer.modules.segmenteditor.widgetRepresentation().self().editor.effectByName("Threshold")

        self.segmentEditorNode = slicer.util.getNode("SegmentEditor")
        self.assertIsNotNone(self.segmentEditorNode)

        self.segmentationNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSegmentationNode")
        self.assertIsNotNone(self.segmentationNode)
        self.segmentEditorNode.SetAndObserveSegmentationNode(self.segmentationNode)

        self.sourceVolumeNode = slicer.util.loadVolume(self.dataDir + '/TinyPatient_CT.nrrd')
        self.assertIsNotNone(self.sourceVolumeNode)
        self.segmentEditorNode.SetAndObserveSourceVolumeNode(self.sourceVolumeNode)

        self.segmentation = self.segmentationNode.GetSegmentation()
        self.segmentation.SetMasterRepresentationName(self.binaryLabelmapReprName)
        self.assertIsNotNone(self.segmentation)

    # ------------------------------------------------------------------------------
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

    # ------------------------------------------------------------------------------
    def TestSection_IslandEffects(self):
        islandSizes = [1, 26, 11, 6, 8, 6, 2]
        islandSizes.sort(reverse=True)

        minimumSize = 3
        self.resetIslandSegments(islandSizes)
        self.islandEffect.setParameter('MinimumSize', minimumSize)
        self.islandEffect.setParameter('Operation', 'KEEP_LARGEST_ISLAND')
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
        self.islandEffect.setParameter('Operation', 'REMOVE_SMALL_ISLANDS')
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
        self.islandEffect.setParameter('Operation', 'SPLIT_ISLANDS_TO_SEGMENTS')
        self.islandEffect.self().onApply()
        layerCount = self.segmentation.GetNumberOfLayers()
        self.assertEqual(layerCount, 1)

        for i in range(len(islandSizes)):
            size = islandSizes[i]
            if size < minimumSize:
                continue
            self.checkSegmentVoxelCount(i, size)

    # ------------------------------------------------------------------------------
    def resetIslandSegments(self, islandSizes):
        self.segmentation.RemoveAllSegments()

        totalSize = 0
        voxelSizeSum = 0
        for size in islandSizes:
            totalSize += size + 1
            voxelSizeSum += size

        mergedLabelmap = vtkSegmentationCore.vtkOrientedImageData()
        mergedLabelmap.SetImageToWorldMatrix(self.ijkToRas)
        mergedLabelmapExtent = [0, totalSize - 1, 0, 0, 0, 0]
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
            islandExtent = [startExtent, startExtent + size - 1, 0, 0, 0, 0]
            self.setupIslandLabelmap(islandLabelmap, islandExtent)
            self.paintEffect.modifySelectedSegmentByLabelmap(islandLabelmap, self.paintEffect.ModificationModeAdd)
            startExtent += size + 1
        self.checkSegmentVoxelCount(0, voxelSizeSum)

        layerCount = self.segmentation.GetNumberOfLayers()
        self.assertEqual(layerCount, 1)

    # ------------------------------------------------------------------------------
    def checkSegmentVoxelCount(self, segmentIndex, expectedVoxelCount):
        segment = self.segmentation.GetNthSegment(segmentIndex)
        self.assertIsNotNone(segment)

        labelmap = slicer.vtkOrientedImageData()
        labelmap.SetImageToWorldMatrix(self.ijkToRas)
        segmentID = self.segmentation.GetNthSegmentID(segmentIndex)
        self.segmentationNode.GetBinaryLabelmapRepresentation(segmentID, labelmap)

        imageStat = vtk.vtkImageAccumulate()
        imageStat.SetInputData(labelmap)
        imageStat.SetComponentExtent(0, 4, 0, 0, 0, 0)
        imageStat.SetComponentOrigin(0, 0, 0)
        imageStat.SetComponentSpacing(1, 1, 1)
        imageStat.IgnoreZeroOn()
        imageStat.Update()

        self.assertEqual(imageStat.GetVoxelCount(), expectedVoxelCount)

    # ------------------------------------------------------------------------------
    def setupIslandLabelmap(self, labelmap, extent, value=1):
        labelmap.SetExtent(extent)
        labelmap.AllocateScalars(vtk.VTK_UNSIGNED_CHAR, 1)
        labelmap.GetPointData().GetScalars().Fill(value)

    # ------------------------------------------------------------------------------
    def TestSection_MarginEffects(self):
        logging.info("Running test on margin effect")

        slicer.modules.segmenteditor.widgetRepresentation().self().editor.effectByName("Margin")

        self.segmentation.RemoveAllSegments()
        segment1Id = self.segmentation.AddEmptySegment("Segment_1")
        segment1 = self.segmentation.GetSegment(segment1Id)
        segment1.SetLabelValue(1)
        self.segmentEditorNode.SetSelectedSegmentID("Segment_1")

        segment2Id = self.segmentation.AddEmptySegment("Segment_2")
        segment2 = self.segmentation.GetSegment(segment2Id)
        segment2.SetLabelValue(2)

        binaryLabelmapRepresentationName = slicer.vtkSegmentationConverter.GetBinaryLabelmapRepresentationName()
        dataTypes = [
            vtk.VTK_CHAR,
            vtk.VTK_SIGNED_CHAR,
            vtk.VTK_UNSIGNED_CHAR,
            vtk.VTK_SHORT,
            vtk.VTK_UNSIGNED_SHORT,
            vtk.VTK_INT,
            vtk.VTK_UNSIGNED_INT,
            # vtk.VTK_LONG, # On linux, VTK_LONG has the same size as VTK_LONG_LONG. This causes issues in vtkImageThreshold.
            # vtk.VTK_UNSIGNED_LONG, See https://github.com/Slicer/Slicer/issues/5427
            # vtk.VTK_FLOAT, # Since float can't represent all int, we jump straight to double.
            vtk.VTK_DOUBLE,
            # vtk.VTK_LONG_LONG, # These types are unsupported in ITK
            # vtk.VTK_UNSIGNED_LONG_LONG,
        ]
        logging.info("Testing shared labelmaps")
        for dataType in dataTypes:
            initialLabelmap = slicer.vtkOrientedImageData()
            initialLabelmap.SetImageToWorldMatrix(self.ijkToRas)
            initialLabelmap.SetExtent(0, 10, 0, 10, 0, 10)
            initialLabelmap.AllocateScalars(dataType, 1)
            initialLabelmap.GetPointData().GetScalars().Fill(0)
            segment1.AddRepresentation(binaryLabelmapRepresentationName, initialLabelmap)
            segment2.AddRepresentation(binaryLabelmapRepresentationName, initialLabelmap)

            self.runMarginEffect(segment1, segment2, dataType, self.segmentEditorNode.OverwriteAllSegments)
            self.assertEqual(self.segmentation.GetNumberOfLayers(), 1)
            self.runMarginEffect(segment1, segment2, dataType, self.segmentEditorNode.OverwriteNone)
            self.assertEqual(self.segmentation.GetNumberOfLayers(), 2)

        logging.info("Testing separate labelmaps")
        for dataType in dataTypes:
            segment1Labelmap = slicer.vtkOrientedImageData()
            segment1Labelmap.SetImageToWorldMatrix(self.ijkToRas)
            segment1Labelmap.SetExtent(0, 10, 0, 10, 0, 10)
            segment1Labelmap.AllocateScalars(dataType, 1)
            segment1Labelmap.GetPointData().GetScalars().Fill(0)
            segment1.AddRepresentation(binaryLabelmapRepresentationName, segment1Labelmap)

            segment2Labelmap = slicer.vtkOrientedImageData()
            segment2Labelmap.DeepCopy(segment1Labelmap)
            segment2.AddRepresentation(binaryLabelmapRepresentationName, segment2Labelmap)

            self.runMarginEffect(segment1, segment2, dataType, self.segmentEditorNode.OverwriteAllSegments)
            self.assertEqual(self.segmentation.GetNumberOfLayers(), 2)
            self.runMarginEffect(segment1, segment2, dataType, self.segmentEditorNode.OverwriteNone)
            self.assertEqual(self.segmentation.GetNumberOfLayers(), 2)

    # ------------------------------------------------------------------------------
    def runMarginEffect(self, segment1, segment2, dataType, overwriteMode):
        logging.info(f"Running margin effect with data type: {dataType}, and overwriteMode {overwriteMode}")
        marginEffect = slicer.modules.segmenteditor.widgetRepresentation().self().editor.effectByName("Margin")

        marginEffect.setParameter("MarginSizeMm", 50.0)

        oldOverwriteMode = self.segmentEditorNode.GetOverwriteMode()
        self.segmentEditorNode.SetOverwriteMode(overwriteMode)

        segment1Labelmap = segment1.GetRepresentation(self.binaryLabelmapReprName)
        segment1Labelmap.AllocateScalars(dataType, 1)
        segment1Labelmap.GetPointData().GetScalars().Fill(0)

        segment2Labelmap = segment2.GetRepresentation(self.binaryLabelmapReprName)
        segment2Labelmap.AllocateScalars(dataType, 1)
        segment2Labelmap.GetPointData().GetScalars().Fill(0)

        segment1Position_IJK = [5, 5, 5]
        segment1Labelmap.SetScalarComponentFromDouble(segment1Position_IJK[0], segment1Position_IJK[1], segment1Position_IJK[2], 0, segment1.GetLabelValue())
        segment2Position_IJK = [6, 5, 6]
        segment2Labelmap.SetScalarComponentFromDouble(segment2Position_IJK[0], segment2Position_IJK[1], segment2Position_IJK[2], 0, segment2.GetLabelValue())

        self.checkSegmentVoxelCount(0, 1)
        self.checkSegmentVoxelCount(1, 1)

        marginEffect.self().onApply()
        self.checkSegmentVoxelCount(0, 9)  # Margin grow
        self.checkSegmentVoxelCount(1, 1)

        marginEffect.self().onApply()
        self.checkSegmentVoxelCount(0, 37)  # Margin grow
        if overwriteMode == slicer.vtkMRMLSegmentEditorNode.OverwriteAllSegments:
            self.checkSegmentVoxelCount(1, 0)  # Overwritten
        else:
            self.checkSegmentVoxelCount(1, 1)  # Not overwritten

        marginEffect.setParameter("MarginSizeMm", -50.0)
        marginEffect.self().onApply()

        self.checkSegmentVoxelCount(0, 9)  # Margin shrink
        if overwriteMode == slicer.vtkMRMLSegmentEditorNode.OverwriteAllSegments:
            self.checkSegmentVoxelCount(1, 0)  # Overwritten
        else:
            self.checkSegmentVoxelCount(1, 1)  # Not overwritten

        self.segmentEditorNode.SetOverwriteMode(oldOverwriteMode)

    # ------------------------------------------------------------------------------
    def TestSection_MaskingSettings(self):
        self.segmentation.RemoveAllSegments()
        segment1Id = self.segmentation.AddEmptySegment("Segment_1")
        segment2Id = self.segmentation.AddEmptySegment("Segment_2")
        segment3Id = self.segmentation.AddEmptySegment("Segment_3")
        segment4Id = self.segmentation.AddEmptySegment("Segment_4")

        oldOverwriteMode = self.segmentEditorNode.GetOverwriteMode()

        # -------------------
        # Test applying threshold with no masking
        self.segmentEditorNode.SetSelectedSegmentID(segment1Id)
        self.thresholdEffect.setParameter("MinimumThreshold", "-17")
        self.thresholdEffect.setParameter("MaximumThreshold", "848")
        self.thresholdEffect.self().onApply()
        self.checkSegmentVoxelCount(0, 204)  # Segment_1
        self.checkSegmentVoxelCount(1, 0)  # Segment_2

        # -------------------
        # Add paint to segment 2. No overwrite
        paintModifierLabelmap = vtkSegmentationCore.vtkOrientedImageData()
        paintModifierLabelmap.SetImageToWorldMatrix(self.ijkToRas)
        paintModifierLabelmap.SetExtent(2, 5, 2, 5, 2, 5)
        paintModifierLabelmap.AllocateScalars(vtk.VTK_UNSIGNED_CHAR, 1)
        paintModifierLabelmap.GetPointData().GetScalars().Fill(1)

        self.segmentEditorNode.SetOverwriteMode(self.segmentEditorNode.OverwriteNone)
        self.segmentEditorNode.SetSelectedSegmentID(segment2Id)
        self.paintEffect.modifySelectedSegmentByLabelmap(paintModifierLabelmap, self.paintEffect.ModificationModeAdd)

        self.checkSegmentVoxelCount(0, 204)  # Segment_1
        self.checkSegmentVoxelCount(1, 64)  # Segment_2

        # -------------------
        # Test erasing with no masking
        eraseModifierLabelmap = vtkSegmentationCore.vtkOrientedImageData()
        eraseModifierLabelmap.SetImageToWorldMatrix(self.ijkToRas)
        eraseModifierLabelmap.SetExtent(2, 5, 2, 5, 2, 5)
        eraseModifierLabelmap.AllocateScalars(vtk.VTK_UNSIGNED_CHAR, 1)
        eraseModifierLabelmap.GetPointData().GetScalars().Fill(1)

        self.segmentEditorNode.SetSelectedSegmentID(segment1Id)
        self.eraseEffect.modifySelectedSegmentByLabelmap(eraseModifierLabelmap, self.paintEffect.ModificationModeRemove)
        self.checkSegmentVoxelCount(0, 177)  # Segment_1
        self.checkSegmentVoxelCount(1, 64)  # Segment_2

        # -------------------
        # Test erasing with masking on empty segment
        self.segmentEditorNode.SetSelectedSegmentID(segment1Id)
        self.thresholdEffect.self().onApply()  # Reset Segment_1
        self.checkSegmentVoxelCount(0, 204)  # Segment_1
        self.segmentEditorNode.SetMaskMode(slicer.vtkMRMLSegmentationNode.EditAllowedInsideSingleSegment)
        self.segmentEditorNode.SetMaskSegmentID(segment2Id)
        self.eraseEffect.modifySelectedSegmentByLabelmap(eraseModifierLabelmap, self.paintEffect.ModificationModeRemove)
        self.checkSegmentVoxelCount(0, 177)  # We expect to be able to erase the current segment regardless of masking
        self.checkSegmentVoxelCount(1, 64)  # Segment_2

        # -------------------
        # Test erasing with masking on the same segment
        self.segmentEditorNode.SetSelectedSegmentID(segment1Id)
        self.thresholdEffect.self().onApply()  # Reset Segment_1
        self.checkSegmentVoxelCount(0, 204)  # Segment_1
        self.segmentEditorNode.SetMaskSegmentID(segment1Id)
        self.eraseEffect.modifySelectedSegmentByLabelmap(eraseModifierLabelmap, self.paintEffect.ModificationModeRemove)
        self.checkSegmentVoxelCount(0, 177)  # Segment_1
        self.checkSegmentVoxelCount(1, 64)  # Segment_2

        # -------------------
        # Test erasing all segments
        self.segmentEditorNode.SetMaskMode(slicer.vtkMRMLSegmentationNode.EditAllowedEverywhere)
        self.thresholdEffect.self().onApply()  # Reset Segment_1
        self.checkSegmentVoxelCount(0, 204)  # Segment_1
        self.segmentEditorNode.SetSelectedSegmentID(segment1Id)
        self.eraseEffect.modifySelectedSegmentByLabelmap(eraseModifierLabelmap, self.paintEffect.ModificationModeRemoveAll)
        self.checkSegmentVoxelCount(0, 177)  # Segment_1
        self.checkSegmentVoxelCount(1, 0)  # Segment_2

        # -------------------
        # Test adding back segments
        self.thresholdEffect.self().onApply()  # Reset Segment_1
        self.checkSegmentVoxelCount(0, 204)  # Segment_1
        self.segmentEditorNode.SetMaskMode(slicer.vtkMRMLSegmentationNode.EditAllowedInsideSingleSegment)
        self.segmentEditorNode.SetMaskSegmentID(segment2Id)
        self.eraseEffect.modifySelectedSegmentByLabelmap(eraseModifierLabelmap, self.paintEffect.ModificationModeRemove)
        self.checkSegmentVoxelCount(0, 177)  # Segment_1
        self.checkSegmentVoxelCount(1, 27)  # Segment_2

        # -------------------
        # Test threshold effect segment mask
        self.segmentEditorNode.SetMaskSegmentID(segment2Id)  # Erase Segment_2
        self.segmentEditorNode.SetSelectedSegmentID(segment2Id)
        self.eraseEffect.modifySelectedSegmentByLabelmap(eraseModifierLabelmap, self.paintEffect.ModificationModeRemove)
        self.segmentEditorNode.SetMaskSegmentID(segment1Id)
        self.segmentEditorNode.SetSelectedSegmentID(segment2Id)
        self.thresholdEffect.self().onApply()  # Threshold Segment_2 within Segment_1
        self.checkSegmentVoxelCount(0, 177)  # Segment_1
        self.checkSegmentVoxelCount(1, 177)  # Segment_2

        # -------------------
        # Test intensity masking with segment mask
        self.segmentEditorNode.SourceVolumeIntensityMaskOn()
        self.segmentEditorNode.SetSourceVolumeIntensityMaskRange(-17, 848)
        self.thresholdEffect.setParameter("MinimumThreshold", "-99999")
        self.thresholdEffect.setParameter("MaximumThreshold", "99999")
        self.segmentEditorNode.SetSelectedSegmentID(segment3Id)
        self.thresholdEffect.self().onApply()  # Threshold Segment_3
        self.checkSegmentVoxelCount(2, 177)  # Segment_3

        # -------------------
        # Test intensity masking with islands
        self.segmentEditorNode.SetMaskMode(slicer.vtkMRMLSegmentationNode.EditAllowedEverywhere)
        self.segmentEditorNode.SourceVolumeIntensityMaskOff()
        self.segmentEditorNode.SetSelectedSegmentID(segment4Id)

        island1ModifierLabelmap = vtkSegmentationCore.vtkOrientedImageData()
        island1ModifierLabelmap.SetImageToWorldMatrix(self.ijkToRas)
        island1ModifierLabelmap.SetExtent(2, 5, 2, 5, 2, 5)
        island1ModifierLabelmap.AllocateScalars(vtk.VTK_UNSIGNED_CHAR, 1)
        island1ModifierLabelmap.GetPointData().GetScalars().Fill(1)
        self.paintEffect.modifySelectedSegmentByLabelmap(island1ModifierLabelmap, self.paintEffect.ModificationModeAdd)

        island2ModifierLabelmap = vtkSegmentationCore.vtkOrientedImageData()
        island2ModifierLabelmap.SetImageToWorldMatrix(self.ijkToRas)
        island2ModifierLabelmap.SetExtent(7, 9, 7, 9, 7, 9)
        island2ModifierLabelmap.AllocateScalars(vtk.VTK_UNSIGNED_CHAR, 1)
        island2ModifierLabelmap.GetPointData().GetScalars().Fill(1)
        self.paintEffect.modifySelectedSegmentByLabelmap(island2ModifierLabelmap, self.paintEffect.ModificationModeAdd)
        self.checkSegmentVoxelCount(3, 91)  # Segment_4

        # Test that no masking works as expected
        minimumSize = 3
        self.islandEffect.setParameter('MinimumSize', minimumSize)
        self.islandEffect.setParameter('Operation', 'KEEP_LARGEST_ISLAND')
        self.islandEffect.self().onApply()
        self.checkSegmentVoxelCount(3, 64)  # Segment_4

        # Reset Segment_4 islands
        self.paintEffect.modifySelectedSegmentByLabelmap(island1ModifierLabelmap, self.paintEffect.ModificationModeAdd)
        self.paintEffect.modifySelectedSegmentByLabelmap(island2ModifierLabelmap, self.paintEffect.ModificationModeAdd)

        # Test intensity masking
        self.segmentEditorNode.SourceVolumeIntensityMaskOn()
        self.segmentEditorNode.SetSourceVolumeIntensityMaskRange(-17, 848)
        self.islandEffect.self().onApply()
        self.checkSegmentVoxelCount(3, 87)  # Segment_4

        # Restore old overwrite setting
        self.segmentEditorNode.SetOverwriteMode(oldOverwriteMode)
        self.segmentEditorNode.SourceVolumeIntensityMaskOff()

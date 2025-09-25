import SampleData
import numpy as np
import slicer
import vtk

from slicer import vtkSlicerSegmentEditorLogic, vtkSlicerSegmentationsModuleLogic
from slicer.ScriptedLoadableModule import ScriptedLoadableModuleTest


class SegmentationLogicTest(ScriptedLoadableModuleTest):
    def setUp(self):
        # Configure the segmentation logic
        slicer.mrmlScene.Clear(0)

        self.logic = vtkSlicerSegmentEditorLogic()
        self.logic.SetMRMLScene(slicer.mrmlScene)
        self.logic.SetMRMLApplicationLogic(slicer.app.applicationLogic())
        self.logic.SetVerbose(True)

        # Initialize with a tiny patient and basic segment editor node
        self.segmentEditorNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSegmentEditorNode")
        self.segmentEditorNode.SetAndObserveSegmentationNode(
            slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSegmentationNode"),
        )
        self.tinyVolumeNode, self.tinySegmentationNode = SampleData.downloadSamples("TinyPatient")

        # Configure logic with initialized segment editor node and volume for other tests
        self.logic.SetSegmentEditorNode(self.segmentEditorNode)
        self.logic.SetSourceVolumeNode(self.tinyVolumeNode)

        assert self.segmentEditorNode.GetSourceVolumeNode() == self.tinyVolumeNode

        # Configure representation to binary label map
        assert self.logic.CanTriviallyConvertSourceRepresentationToBinaryLabelMap()
        assert self.logic.TrivialSetSourceRepresentationToBinaryLabelmap()

        # Reset default label map modifier
        assert self.logic.ResetModifierLabelmapToDefault()

    def test_with_no_segment_editor_node_is_invalid(self):
        self.logic.SetSegmentEditorNode(None)
        assert not self.logic.IsSegmentationNodeValid()

    def test_with_no_segmentation_node_is_invalid(self):
        self.segmentEditorNode.SetAndObserveSegmentationNode(None)
        assert not self.logic.IsSegmentationNodeValid()

    def test_with_segment_editor_and_segmentation_node_is_valid(self):
        assert self.logic.IsSegmentationNodeValid()

    def test_can_create_default_modifier_label_map_matching_source_volume(self):
        labelMap = self.logic.GetModifierLabelmap()
        assert labelMap
        assert labelMap.GetExtent() == self.tinyVolumeNode.GetImageData().GetExtent()

        modifierArray = vtk.util.numpy_support.vtk_to_numpy(labelMap.GetPointData().GetScalars())
        np.testing.assert_array_equal(modifierArray, np.zeros_like(modifierArray))

    def test_can_add_segments_to_segmentation_node(self):
        assert self.logic.CanAddSegments()
        status = vtkSlicerSegmentationsModuleLogic.InProgress
        self.logic.AddEmptySegment("", status)

        assert len(self.logic.GetSegmentIDs()) == 1
        segment = self.logic.GetSelectedSegment()
        assert segment
        assert vtkSlicerSegmentationsModuleLogic.GetSegmentStatus(segment) == status

    def test_added_segment_is_selected_automatically(self):
        assert not self.logic.GetCurrentSegmentID()
        self.logic.AddEmptySegment()
        segmentId = self.logic.GetCurrentSegmentID()
        assert segmentId

        self.logic.AddEmptySegment()
        assert self.logic.GetCurrentSegmentID() != segmentId

    def test_can_remove_segments_from_segmentation_node(self):
        self.logic.AddEmptySegment()
        self.logic.AddEmptySegment()

        currentID = self.logic.GetCurrentSegmentID()
        assert currentID

        assert self.logic.CanRemoveSegments()
        self.logic.RemoveSelectedSegment()

        assert len(self.logic.GetSegmentIDs()) == 1
        assert currentID not in self.logic.GetSegmentIDs()

    def test_on_remove_selects_next_if_valid(self):
        firstId = self.logic.AddEmptySegment()
        secondId = self.logic.AddEmptySegment()
        self.logic.SetCurrentSegmentID(firstId)
        assert self.logic.RemoveSelectedSegment() == secondId

    def test_on_remove_selects_previous_if_next_not_valid(self):
        firstId = self.logic.AddEmptySegment()
        secondId = self.logic.AddEmptySegment()
        self.logic.SetCurrentSegmentID(secondId)
        assert self.logic.RemoveSelectedSegment() == firstId

    def test_can_undo_redo_changes_to_segmentation_node(self):
        assert self.logic.GetMaximumNumberOfUndoStates() > 0
        self.logic.AddEmptySegment()
        self.logic.AddEmptySegment()

        assert self.logic.CanUndo()
        self.logic.Undo()
        assert len(self.logic.GetSegmentIDs()) == 1

        assert self.logic.CanRedo()
        self.logic.Redo()
        assert len(self.logic.GetSegmentIDs()) == 2

    def test_can_apply_modifier_labelmap_changes_to_segmentation(self):
        # Add one segment
        self.logic.AddEmptySegment()

        # Modify the added segment using a label map
        labelMap = self.logic.GetModifierLabelmap()
        arrayShape = tuple(reversed(labelMap.GetDimensions()))
        expArray = vtk.util.numpy_support.vtk_to_numpy(labelMap.GetPointData().GetScalars()).reshape(arrayShape)
        expArray[0, 0, 0] = 1
        expArray[-1, -1, -1] = 1

        segmentId = self.logic.GetCurrentSegmentID()
        self.logic.ModifySegmentByLabelmap(
            self.logic.GetSegmentationNode(),
            segmentId,
            labelMap,
            vtkSlicerSegmentEditorLogic.ModificationModeAdd,
            labelMap.GetExtent(),
            True,
            False,
        )

        # Assert the modifications have been taken into account
        segmentArray = slicer.util.arrayFromSegmentInternalBinaryLabelmap(self.logic.GetSegmentationNode(), segmentId)
        np.testing.assert_array_equal(segmentArray, expArray)

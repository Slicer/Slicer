from unittest.mock import MagicMock

import slicer
from slicer import (
    vtkMRMLLayerDMPipelineFactory,
    vtkMRMLLayerDMPipelineManager,
    vtkMRMLLayerDMPipelineScriptedCreator,
    vtkMRMLAbstractViewNode,
    vtkMRMLInteractionEventData,
    vtkMRMLMarkupsFiducialNode,
    vtkMRMLModelNode,
    vtkMRMLScalarVolumeNode,
)
from slicer.ScriptedLoadableModule import ScriptedLoadableModuleTest
from vtk import vtkRenderWindow, reference as ref, vtkCommand, vtkRenderer
from MockPipeline import MockPipeline


class PipelineManagerTest(ScriptedLoadableModuleTest):
    def setUp(self):
        slicer.mrmlScene.Clear(0)

        self.renderWindow = vtkRenderWindow()
        self.factory = vtkMRMLLayerDMPipelineFactory()

        self.viewNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLViewNode")
        self.pipelineManager = vtkMRMLLayerDMPipelineManager()
        self.pipelineManager.SetViewNode(self.viewNode)
        self.pipelineManager.SetFactory(self.factory)
        self.pipelineManager.SetScene(slicer.mrmlScene)
        self.pipelineManager.SetRenderWindow(self.renderWindow)
        self.defaultRenderer = vtkRenderer()
        self.renderWindow.AddRenderer(self.defaultRenderer)

        modelCreator = vtkMRMLLayerDMPipelineScriptedCreator()
        modelCreator.SetPythonCallback(self.createModelPipeline)
        self.mockModelCreate = MagicMock()
        self.factory.AddPipelineCreator(modelCreator)

        self.nextMock = None
        self.mockCreate = MagicMock(side_effect=self.createMock)
        mockCreator = vtkMRMLLayerDMPipelineScriptedCreator()
        mockCreator.SetPythonCallback(self.mockCreate)
        self.factory.AddPipelineCreator(mockCreator)

        self.mockVolumeCreate = MagicMock()

    def createMock(self, *_):
        return self.nextMock

    def createModelPipeline(self, _viewNode: vtkMRMLAbstractViewNode, node: vtkMRMLModelNode):
        # Dummy pipeline creator for model nodes
        self.mockModelCreate(_viewNode, node)
        if not isinstance(node, vtkMRMLModelNode):
            return None
        return MockPipeline()

    def createVolumePipeline(self, _viewNode: vtkMRMLAbstractViewNode, node: vtkMRMLScalarVolumeNode):
        self.mockVolumeCreate(_viewNode, node)
        if not isinstance(node, vtkMRMLScalarVolumeNode):
            return None
        return MockPipeline()

    def test_on_add_node_added_adds_associated_pipeline(self):
        self.mockModelCreate.reset_mock()
        modelNode = vtkMRMLModelNode()
        assert self.pipelineManager.AddNode(modelNode)
        self.mockModelCreate.assert_called_once_with(self.viewNode, modelNode)
        assert self.pipelineManager.GetNodePipeline(modelNode) is not None

    def test_on_remove_node_removes_associated_pipeline(self):
        modelNode = vtkMRMLModelNode()
        assert self.pipelineManager.AddNode(modelNode)
        assert self.pipelineManager.RemoveNode(modelNode)
        assert self.pipelineManager.GetNodePipeline(modelNode) is None

    def test_on_factory_updated_creates_pipelines_if_needed(self):
        volumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScalarVolumeNode")
        assert volumeNode
        assert self.pipelineManager.GetNodePipeline(volumeNode) is None

        volumeCreator = vtkMRMLLayerDMPipelineScriptedCreator()
        volumeCreator.SetPythonCallback(self.createVolumePipeline)
        self.factory.AddPipelineCreator(volumeCreator)
        assert self.pipelineManager.GetNodePipeline(volumeNode) is not None

    def test_on_scene_update_add_pipeline_as_needed(self):
        modelNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode")
        self.pipelineManager.UpdateFromScene()
        assert self.pipelineManager.GetNodePipeline(modelNode) is not None

    def test_on_scene_update_remove_pipeline_as_needed(self):
        modelNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode")
        self.pipelineManager.AddNode(modelNode)
        slicer.mrmlScene.RemoveNode(modelNode)
        self.pipelineManager.UpdateFromScene()
        assert self.pipelineManager.GetNodePipeline(modelNode) is None

    def triggerMockPipelineCreation(self, mock: MockPipeline) -> MockPipeline:
        self.nextMock = mock
        node = vtkMRMLMarkupsFiducialNode()
        assert self.pipelineManager.AddNode(node)
        assert self.pipelineManager.GetNodePipeline(node) == mock
        return mock

    def test_dispatches_process_interaction_to_higher_order_pipeline_first(self):
        m1 = self.triggerMockPipelineCreation(MockPipeline(renderOrder=1))
        m2 = self.triggerMockPipelineCreation(MockPipeline(renderOrder=10))
        m3 = self.triggerMockPipelineCreation(MockPipeline(renderOrder=100))
        m4 = self.triggerMockPipelineCreation(MockPipeline(renderOrder=0))
        m1.mockCanProcess.return_value = (True, 1)
        m2.mockCanProcess.return_value = (True, 1000)
        m3.mockCanProcess.return_value = (True, 2000)
        m4.mockCanProcess.return_value = (False, 0)

        distance = ref(0.0)
        assert self.pipelineManager.CanProcessInteractionEvent(vtkMRMLInteractionEventData(), distance)

        # Reported distance is the min of those that can process
        assert distance == 1

        # Did process is the furthest one that could process and did process
        m1.mockProcess.return_value = True
        m2.mockProcess.return_value = True
        m3.mockProcess.return_value = False
        m4.mockProcess.return_value = True

        assert self.pipelineManager.ProcessInteractionEvent(vtkMRMLInteractionEventData())
        m4.mockProcess.assert_not_called()
        m3.mockProcess.assert_called_once()
        m2.mockProcess.assert_called_once()
        m1.mockProcess.assert_not_called()

    def test_dispatches_process_to_max_state_first(self):
        # Create two pipelines with same order and disable pipeline 2 for first interaction
        m1 = self.triggerMockPipelineCreation(MockPipeline(renderOrder=1))
        m2 = self.triggerMockPipelineCreation(MockPipeline(renderOrder=1))
        m1.mockCanProcess.return_value = (True, 0)
        m2.mockCanProcess.return_value = (False, 0)
        m1.mockProcess.return_value = True
        m2.mockProcess.return_value = True

        distance = ref(0.0)
        assert self.pipelineManager.CanProcessInteractionEvent(vtkMRMLInteractionEventData(), distance)
        assert self.pipelineManager.ProcessInteractionEvent(vtkMRMLInteractionEventData())
        m1.mockProcess.assert_called_once()

        # At second interaction m1 should have focus and not m2
        # Set both to can process with m1 further from interaction but with active widget state
        m1.mockCanProcess.return_value = (True, 10000)
        m2.mockCanProcess.return_value = (True, 0)
        m1.mockGetWidgetState.return_value = 100
        assert self.pipelineManager.CanProcessInteractionEvent(vtkMRMLInteractionEventData(), distance)
        assert self.pipelineManager.ProcessInteractionEvent(vtkMRMLInteractionEventData())

        # Expect m1 to have handled the interaction regardless of m2 proximity
        assert m1.mockProcess.call_count == 2
        m2.mockProcess.assert_not_called()
        assert m1.mockCanProcess.call_count == 2
        assert m2.mockCanProcess.call_count == 2

    def test_on_lose_focus_forwards_information_of_last_with_focus(self):
        m1 = self.triggerMockPipelineCreation(MockPipeline(renderOrder=1))
        m1.mockCanProcess.return_value = (True, 0)
        m1.mockProcess.return_value = True

        distance = ref(0.0)
        assert self.pipelineManager.CanProcessInteractionEvent(vtkMRMLInteractionEventData(), distance)
        assert self.pipelineManager.ProcessInteractionEvent(vtkMRMLInteractionEventData())

        loseFocusData = vtkMRMLInteractionEventData()
        self.pipelineManager.LoseFocus(loseFocusData)
        self.pipelineManager.LoseFocus(None)
        self.pipelineManager.LoseFocus()
        m1.mockLoseFocus.assert_called_once_with(loseFocusData)

    def test_if_last_with_focus_cannot_process_loses_focus(self):
        m1 = self.triggerMockPipelineCreation(MockPipeline(renderOrder=1))
        m2 = self.triggerMockPipelineCreation(MockPipeline(renderOrder=10))
        m1.mockCanProcess.return_value = (True, 0)
        m2.mockCanProcess.return_value = (False, 0)

        distance = ref(0.0)
        assert self.pipelineManager.CanProcessInteractionEvent(vtkMRMLInteractionEventData(), distance)

        m1.mockProcess.return_value = True
        m2.mockProcess.return_value = True
        assert self.pipelineManager.ProcessInteractionEvent(vtkMRMLInteractionEventData())
        m1.mockProcess.assert_called_once()

        m1.mockProcess.reset_mock()
        m1.mockCanProcess.return_value = (False, 0)
        m2.mockCanProcess.return_value = (True, 0)

        assert self.pipelineManager.CanProcessInteractionEvent(vtkMRMLInteractionEventData(), distance)
        m1.mockLoseFocus.assert_called_once()

        assert self.pipelineManager.ProcessInteractionEvent(vtkMRMLInteractionEventData())
        m1.mockProcess.assert_not_called()
        m2.mockProcess.assert_called_once()

    def test_if_last_with_focus_did_not_process_loses_focus(self):
        m1 = self.triggerMockPipelineCreation(MockPipeline(renderOrder=1))
        m2 = self.triggerMockPipelineCreation(MockPipeline(renderOrder=10))
        m1.mockCanProcess.return_value = (True, 0)
        m2.mockCanProcess.return_value = (False, 0)

        distance = ref(0.0)
        assert self.pipelineManager.CanProcessInteractionEvent(vtkMRMLInteractionEventData(), distance)

        m1.mockProcess.return_value = True
        m2.mockProcess.return_value = True
        assert self.pipelineManager.ProcessInteractionEvent(vtkMRMLInteractionEventData())

        m1.mockProcess.reset_mock()
        m1.mockProcess.return_value = False
        m1.mockGetWidgetState.return_value = 100
        m2.mockCanProcess.return_value = (True, 0)
        assert self.pipelineManager.CanProcessInteractionEvent(vtkMRMLInteractionEventData(), distance)
        m1.mockLoseFocus.assert_not_called()

        assert self.pipelineManager.ProcessInteractionEvent(vtkMRMLInteractionEventData())
        m1.mockProcess.assert_called_once()
        m2.mockProcess.assert_called_once()
        m1.mockLoseFocus.assert_called_once()

    def test_on_pipeline_added_triggers_modified_event(self):
        mock = MagicMock()
        self.pipelineManager.AddObserver(vtkCommand.ModifiedEvent, mock)

        modelNode = vtkMRMLModelNode()
        self.pipelineManager.AddNode(modelNode)
        mock.assert_called_once()

        self.pipelineManager.AddNode(modelNode)
        mock.assert_called_once()

        self.pipelineManager.AddNode(vtkMRMLModelNode())
        assert mock.call_count == 2

    def test_on_pipeline_removed_triggers_modified_event(self):
        mock = MagicMock()

        modelNode = vtkMRMLModelNode()
        self.pipelineManager.AddNode(modelNode)

        self.pipelineManager.AddObserver(vtkCommand.ModifiedEvent, mock)
        self.pipelineManager.RemoveNode(modelNode)
        mock.assert_called_once()

        self.pipelineManager.RemoveNode(modelNode)
        mock.assert_called_once()

    def test_on_pipeline_added_triggers_renderer_added(self):
        m1 = self.triggerMockPipelineCreation(MockPipeline())
        m1.mockOnRendererAdded.assert_called_once_with(self.defaultRenderer)

    def test_on_pipeline_removed_triggers_renderer_removed(self):
        m1 = self.triggerMockPipelineCreation(MockPipeline())

        # Reset mockOnRendererRemoved (called once when setting the first renderer with value None)
        m1.mockOnRendererRemoved.reset_mock()

        self.pipelineManager.RemoveNode(m1.GetDisplayNode())
        m1.mockOnRendererRemoved.assert_called_once_with(self.defaultRenderer)

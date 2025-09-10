import slicer
from slicer import vtkMRMLLayerDMInteractionLogic, vtkMRMLInteractionEventData, vtkMRMLAbstractWidget
from slicer.ScriptedLoadableModule import ScriptedLoadableModuleTest
from vtk import reference, vtkCommand

from MockPipeline import MockPipeline


class InteractionLogicTest(ScriptedLoadableModuleTest):
    def setUp(self):
        slicer.mrmlScene.Clear(0)
        self.distance = reference(0.0)
        self.event = vtkMRMLInteractionEventData()
        self.logic = vtkMRMLLayerDMInteractionLogic()

    def test_with_no_pipelines_cannot_process(self):
        assert not self.logic.CanProcessInteractionEvent(self.event, self.distance)

    def test_with_multiple_pipelines_can_process_returns_min_distance(self):
        minDistance = 3
        for i in range(5):
            self.logic.AddPipeline(MockPipeline(canProcess=True, processDistance=i + minDistance))

        assert self.logic.CanProcessInteractionEvent(self.event, self.distance)
        assert self.distance == minDistance

    def test_with_pipeline_with_state_not_idle_returns_negative_distance(self):
        self.logic.AddPipeline(
            MockPipeline(canProcess=True, widgetState=vtkMRMLAbstractWidget.WidgetStateTranslate, processDistance=10)
        )

        assert self.logic.CanProcessInteractionEvent(self.event, self.distance)
        assert self.distance < 0

    def test_with_same_state_and_layer_processes_min_dist_first(self):
        pipelines = []
        for i in range(5):
            pipeline = MockPipeline(canProcess=True, didProcess=True, processDistance=i)
            pipelines.append(pipeline)
            self.logic.AddPipeline(pipeline)

        assert self.logic.CanProcessInteractionEvent(self.event, self.distance)
        assert self.logic.ProcessInteractionEvent(self.event)
        pipelines[0].mockProcess.assert_called_once_with(self.event)

    def test_with_same_state_processes_max_layer_first(self):
        pipelines = []
        for i in range(5):
            pipeline = MockPipeline(canProcess=True, didProcess=True, layer=i)
            pipelines.append(pipeline)
            self.logic.AddPipeline(pipeline)

        assert self.logic.CanProcessInteractionEvent(self.event, self.distance)
        assert self.logic.ProcessInteractionEvent(self.event)
        pipelines[-1].mockProcess.assert_called_once_with(self.event)

    def test_with_not_idle_state_processes_max_state_first(self):
        pipelines = []
        for i in range(5):
            pipeline = MockPipeline(canProcess=True, didProcess=True, widgetState=i)
            pipelines.append(pipeline)
            self.logic.AddPipeline(pipeline)

        assert self.logic.CanProcessInteractionEvent(self.event, self.distance)
        assert self.logic.ProcessInteractionEvent(self.event)
        pipelines[-1].mockProcess.assert_called_once_with(self.event)

    def test_on_leave_event_lose_last_focus(self):
        pipeline = MockPipeline(canProcess=True, didProcess=True)
        self.logic.AddPipeline(pipeline)

        assert self.logic.CanProcessInteractionEvent(self.event, self.distance)
        assert self.logic.ProcessInteractionEvent(self.event)

        self.event.SetType(vtkCommand.LeaveEvent)
        assert not self.logic.CanProcessInteractionEvent(self.event, self.distance)
        pipeline.mockLoseFocus.assert_called_once_with(self.event)

    def test_on_focused_cannot_process_loses_focus(self):
        pipeline = MockPipeline(canProcess=True, didProcess=True)
        self.logic.AddPipeline(pipeline)

        assert self.logic.CanProcessInteractionEvent(self.event, self.distance)
        assert self.logic.ProcessInteractionEvent(self.event)

        pipeline.mockCanProcess.return_value = False, 0
        assert not self.logic.CanProcessInteractionEvent(self.event, self.distance)

        pipeline.mockLoseFocus.assert_called_once_with(self.event)

    def test_on_new_interaction_different_from_focused_loses_focus(self):
        pipeline = MockPipeline(canProcess=True, didProcess=True)
        self.logic.AddPipeline(pipeline)

        assert self.logic.CanProcessInteractionEvent(self.event, self.distance)
        assert self.logic.ProcessInteractionEvent(self.event)

        p2 = MockPipeline(canProcess=True, didProcess=True, layer=10)
        self.logic.AddPipeline(p2)

        assert self.logic.CanProcessInteractionEvent(self.event, self.distance)
        assert self.logic.ProcessInteractionEvent(self.event)

        pipeline.mockLoseFocus.assert_called_once_with(self.event)
        p2.mockProcess.assert_called_once_with(self.event)

        assert self.logic.GetLastFocusedPipeline() == p2

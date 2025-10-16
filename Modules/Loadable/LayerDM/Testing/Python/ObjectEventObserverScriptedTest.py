import slicer
from slicer import (
    vtkMRMLMarkupsFiducialNode,
    vtkMRMLModelNode,
    vtkMRMLScene,
    vtkMRMLLayerDMObjectEventObserverScripted,
)
from slicer.ScriptedLoadableModule import ScriptedLoadableModuleTest
from unittest.mock import MagicMock, call
from vtk import vtkCommand, VTK_OBJECT


class vtkMRMLLayerDMObjectEventObserverTest(ScriptedLoadableModuleTest):
    def setUp(self):
        slicer.mrmlScene.Clear(0)

        self.mock = MagicMock(return_value=None)
        self.observer = vtkMRMLLayerDMObjectEventObserverScripted()
        self.observer.SetPythonCallback(self.mock)

    def test_can_be_attached_to_vtk_object_modified_event(self):
        modelNode = vtkMRMLModelNode()

        self.observer.UpdateObserver(None, modelNode, vtkCommand.ModifiedEvent)
        modelNode.Modified()
        self.mock.assert_called_once_with(modelNode, vtkCommand.ModifiedEvent, None)

    def test_can_be_attached_to_multiple_events(self):
        markups_node = vtkMRMLMarkupsFiducialNode()

        self.observer.UpdateObserver(None, markups_node, [vtkCommand.ModifiedEvent, markups_node.PointModifiedEvent])
        markups_node.InvokeEvent(markups_node.PointModifiedEvent)
        self.mock.assert_called_once_with(markups_node, markups_node.PointModifiedEvent, None)

        self.mock.reset_mock()
        markups_node.Modified()
        self.mock.assert_called_once_with(markups_node, vtkCommand.ModifiedEvent, None)

    def test_can_update_observer(self):
        m1 = vtkMRMLModelNode()
        m2 = vtkMRMLModelNode()

        self.observer.UpdateObserver(None, m1, vtkCommand.ModifiedEvent)
        self.observer.UpdateObserver(m1, m2, vtkCommand.ModifiedEvent)
        m1.Modified()
        m2.Modified()
        self.mock.assert_called_once_with(m2, vtkCommand.ModifiedEvent, None)

    def test_can_remove_observer(self):
        modelNode = vtkMRMLModelNode()
        self.observer.UpdateObserver(None, modelNode, vtkCommand.ModifiedEvent)
        self.observer.RemoveObserver(modelNode)
        modelNode.Modified()
        self.mock.assert_not_called()

    def test_can_cast_observed_call_data(self):
        self.observer.UpdateObserver(None, slicer.mrmlScene, vtkMRMLScene.NodeAddedEvent)

        modelNode = vtkMRMLModelNode()
        slicer.mrmlScene.AddNode(modelNode)

        self.mock.assert_called_once()
        args = self.mock.call_args[0]
        assert args[0] == slicer.mrmlScene
        assert args[1] == vtkMRMLScene.NodeAddedEvent
        assert self.observer.CastCallData(args[2], VTK_OBJECT) == modelNode

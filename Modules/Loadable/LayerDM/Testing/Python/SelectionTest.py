from unittest.mock import MagicMock

import slicer
from slicer import (
    vtkMRMLLayerDMSelectionObserver,
    vtkMRMLScene,
    vtkMRMLLayerDMObjectEventObserverScripted,
    vtkMRMLInteractionNode,
)
from slicer.ScriptedLoadableModule import ScriptedLoadableModuleTest


class SelectionTest(ScriptedLoadableModuleTest):
    def setUp(self):
        slicer.mrmlScene.Clear(0)
        self.selection_obs = vtkMRMLLayerDMSelectionObserver()
        self.selection_obs.SetScene(slicer.mrmlScene)
        self.mock = MagicMock(return_value=None)
        self._spy = vtkMRMLLayerDMObjectEventObserverScripted()
        self._spy.UpdateObserver(None, self.selection_obs)
        self._spy.SetPythonCallback(self.mock)

    def test_has_access_to_singleton_nodes(self):
        assert self.selection_obs.GetInteractionNode() is not None
        assert self.selection_obs.GetSelectionNode() is not None

    def test_with_empty_new_scene_doesnt_initialize_singleton_nodes(self):
        scene = vtkMRMLScene()
        obs = vtkMRMLLayerDMSelectionObserver()
        obs.SetScene(scene)

        assert obs.GetInteractionNode() is None
        assert obs.GetSelectionNode() is None

    def test_helps_in_placing_nodes(self):
        markups_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode")
        self.selection_obs.StartPlace(markups_node, True)

        self.mock.assert_called()
        assert self.selection_obs.GetActivePlaceNodeID() == markups_node.GetID()
        assert self.selection_obs.GetCurrentInteractionMode() == vtkMRMLInteractionNode.Place
        assert self.selection_obs.GetPlaceModePersistence()
        assert self.selection_obs.IsPlacing(markups_node)
        self.mock.reset_mock()
        self.selection_obs.StopPlace()
        self.mock.assert_called()

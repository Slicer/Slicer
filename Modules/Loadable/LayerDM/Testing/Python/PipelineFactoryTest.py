from unittest.mock import MagicMock

import slicer
from slicer import (
    vtkMRMLLayerDMPipelineFactory,
    vtkMRMLLayerDMPipelineI,
    vtkMRMLLayerDMPipelineScriptedCreator,
    vtkMRMLLayerDMPipelineCreatorI,
    vtkMRMLViewNode,
    vtkMRMLCameraNode
)
from slicer.ScriptedLoadableModule import ScriptedLoadableModuleTest
from vtk import vtkCommand


class PipelineFactoryTest(ScriptedLoadableModuleTest):
    def setUp(self):
        slicer.mrmlScene.Clear(0)
        self.factory = vtkMRMLLayerDMPipelineFactory()

    def test_is_compatible_with_scripted_dm_creator(self):
        instance = vtkMRMLLayerDMPipelineI()
        creator = vtkMRMLLayerDMPipelineScriptedCreator()
        creator.SetPythonCallback(lambda *_: instance)
        assert isinstance(creator, vtkMRMLLayerDMPipelineCreatorI)

        self.factory.AddPipelineCreator(creator)
        assert self.factory.ContainsPipelineCreator(creator)

        for _ in range(5):
            assert self.factory.CreatePipeline(None, None) == instance

    def test_provides_singleton_instance(self):
        assert vtkMRMLLayerDMPipelineFactory.GetInstance() == vtkMRMLLayerDMPipelineFactory.GetInstance()

    def test_can_remove_creator_by_ref(self):
        c1 = vtkMRMLLayerDMPipelineCreatorI()

        self.factory.AddPipelineCreator(c1)
        assert self.factory.ContainsPipelineCreator(c1)

        c2 = vtkMRMLLayerDMPipelineCreatorI()
        assert not self.factory.ContainsPipelineCreator(c2)
        self.factory.AddPipelineCreator(c2)
        assert self.factory.ContainsPipelineCreator(c2)

        self.factory.RemovePipelineCreator(c1)
        assert not self.factory.ContainsPipelineCreator(c1)

    def test_invokes_modified_event_on_add_remove_creator(self):
        creator = vtkMRMLLayerDMPipelineCreatorI()
        mock = MagicMock()
        self.factory.AddObserver(vtkCommand.ModifiedEvent, mock)

        self.factory.AddPipelineCreator(creator)
        mock.assert_called_once()
        mock.reset_mock()

        self.factory.RemovePipelineCreator(creator)
        mock.assert_called_once()
        mock.reset_mock()

        self.factory.RemovePipelineCreator(creator)
        mock.assert_not_called()

    def test_iterates_on_provided_creators_until_one_can_create(self):
        mocks = []

        for _ in range(5):
            mock = MagicMock(return_value=None)
            mocks.append(mock)
            creator = vtkMRMLLayerDMPipelineScriptedCreator()
            creator.SetPythonCallback(mock)
            self.factory.AddPipelineCreator(creator)

        viewNode = vtkMRMLViewNode()
        node = vtkMRMLCameraNode()

        assert self.factory.CreatePipeline(viewNode, node) is None
        for mock in mocks:
            mock.assert_called_once_with(viewNode, node)
            mock.reset_mock()

        instance = vtkMRMLLayerDMPipelineI()
        mocks[0].return_value = instance

        assert self.factory.CreatePipeline(viewNode, node) == instance
        for mock in mocks[1:]:
            mock.assert_not_called()

    def test_notifies_on_pipeline_creation(self):
        def on_about_to_be_created(*_):
            mock(self.factory.GetLastViewNode(), self.factory.GetLastNode(), self.factory.GetLastPipeline())

        mock = MagicMock()
        self.factory.AddObserver(self.factory.PipelineAboutToBeCreatedEvent, on_about_to_be_created)

        instance = vtkMRMLLayerDMPipelineI()
        creator = vtkMRMLLayerDMPipelineScriptedCreator()
        creator.SetPythonCallback(lambda *_: instance)

        self.factory.AddPipelineCreator(creator)

        viewNode = vtkMRMLViewNode()
        node = vtkMRMLCameraNode()

        self.factory.CreatePipeline(viewNode, node)
        mock.assert_called_once_with(viewNode, node, instance)

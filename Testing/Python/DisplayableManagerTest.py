import slicer
from LayerDMManagerLib import vtkMRMLLayerDMScriptedPipeline
from slicer import (
    vtkMRMLLayerDMPipelineFactory,
    vtkMRMLLayerDMPipelineManager,
    vtkMRMLLayerDMPipelineScriptedCreator,
    vtkMRMLLayerDisplayableManager,
    vtkMRMLModelNode,
    vtkMRMLScene,
    vtkMRMLScriptedModuleNode,
    vtkMRMLSliceViewDisplayableManagerFactory,
    vtkMRMLThreeDViewDisplayableManagerFactory,
    vtkMRMLLayerDMPipelineI,
)
from slicer.ScriptedLoadableModule import ScriptedLoadableModuleTest
from vtk import vtkRenderer, vtkActor, vtkObjectFactory
from vtkmodules.vtkCommonCore import VTK_OBJECT

from MockPipeline import MockPipeline


class DisplayableManagerTest(ScriptedLoadableModuleTest):
    def setUp(self):
        slicer.mrmlScene.Clear(0)

        # Verify the DM can be created by the vtk Object Factory
        assert vtkObjectFactory.CreateInstance(vtkMRMLLayerDisplayableManager.__name__) is not None

        # Register the DM to the default views
        vtkMRMLLayerDisplayableManager.RegisterInDefaultViews()

        # Verify registration is correct for both 2D and 3D views
        assert vtkMRMLLayerDisplayableManager.IsRegisteredInFactory(
            vtkMRMLSliceViewDisplayableManagerFactory.GetInstance()
        )
        assert vtkMRMLLayerDisplayableManager.IsRegisteredInFactory(
            vtkMRMLThreeDViewDisplayableManagerFactory.GetInstance()
        )

        self.threeDNode = slicer.mrmlScene.GetNodeByID("vtkMRMLViewNode1")
        self.node = vtkMRMLScriptedModuleNode()

        # Register a pipeline creator with static pipeline
        self.pipeline = MockPipeline()
        self.factory = vtkMRMLLayerDMPipelineFactory.GetInstance()
        self.creator = vtkMRMLLayerDMPipelineScriptedCreator()
        self.creator.SetPythonCallback(
            lambda view, node: self.pipeline if view == self.threeDNode and node == self.node else None
        )
        self.factory.AddPipelineCreator(self.creator)

    def tearDown(self):
        self.factory.RemovePipelineCreator(self.creator)

    def test_registered_pipelines_can_add_their_actors_to_views(self):
        # Configure pipeline renderer added to add a new actor
        actor = vtkActor()

        def onRendererAdded(renderer: vtkRenderer):
            if renderer:
                renderer.AddViewProp(actor)

        self.pipeline.mockOnRendererAdded.side_effect = onRendererAdded

        # Trigger pipeline creation by adding a node to the scene
        slicer.mrmlScene.AddNode(self.node)

        # Verify the pipeline's actor is present in the threed view
        renderWindow = slicer.app.layoutManager().viewWidget(self.threeDNode).viewWidget().renderWindow()
        renderer = renderWindow.GetRenderers().GetItemAsObject(0)
        assert renderer.HasViewProp(actor)

    def test_registered_pipelines_have_access_to_scene_manager_view_and_display_node(self):
        # Trigger pipeline creation by adding a new model node to the scene
        slicer.mrmlScene.AddNode(self.node)

        # Check getters point to the right instances
        assert self.pipeline.GetScene() == slicer.mrmlScene
        assert isinstance(self.pipeline.GetPipelineManager(), vtkMRMLLayerDMPipelineManager)
        assert self.pipeline.GetViewNode() == self.threeDNode
        assert self.pipeline.GetDisplayNode() == self.node

    def test_registered_pipelines_can_override_setters_to_scene_manager_view_and_display_node(self):
        # Trigger pipeline creation by adding a new model node to the scene
        slicer.mrmlScene.AddNode(self.node)

        # Check overridden setters where called
        self.pipeline.mockSetScene.assert_called_with(slicer.mrmlScene)
        self.pipeline.mockSetPipelineManager.assert_called_with(self.pipeline.GetPipelineManager())
        self.pipeline.mockSetViewNode.assert_called_with(self.threeDNode)
        self.pipeline.mockSetDisplayNode.assert_called_with(self.node)

    def test_pipelines_have_access_to_sibling_pipelines(self):
        # Register a second pipeline creator
        otherNode = vtkMRMLScriptedModuleNode()
        otherPipeline = MockPipeline()

        otherCreator = vtkMRMLLayerDMPipelineScriptedCreator()
        otherCreator.SetPythonCallback(
            lambda view, node: otherPipeline if view == self.threeDNode and node == otherNode else None
        )
        self.factory.AddPipelineCreator(otherCreator)

        # Add both nodes to the scene for pipeline creation to happen
        slicer.mrmlScene.AddNode(self.node)
        slicer.mrmlScene.AddNode(otherNode)

        # Assert the first pipeline has access to the second one
        assert self.pipeline.GetNodePipeline(otherNode) == otherPipeline

    def test_pipelines_can_add_observers_to_vtk_objects_and_events(self):
        def onSetScene(scene: vtkMRMLScene):
            assert self.pipeline.GetScene() != scene
            self.pipeline.UpdateObserver(self.pipeline.GetScene(), scene, vtkMRMLScene.NodeAddedEvent)
            vtkMRMLLayerDMPipelineI.SetScene(self.pipeline, scene)

        # Add observer on the scene when its added
        self.pipeline.mockSetScene.side_effect = onSetScene

        slicer.mrmlScene.AddNode(self.node)
        self.pipeline.mockSetScene.assert_called_once()

        modelNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode")
        self.pipeline.mockOnUpdate.assert_called_once()
        args = self.pipeline.mockOnUpdate.call_args[0]
        assert args[0] == slicer.mrmlScene
        assert args[1] == vtkMRMLScene.NodeAddedEvent
        assert self.pipeline.CastCallData(args[2], VTK_OBJECT) == modelNode

    def test_pipelines_update_display_is_called_at_init(self):
        slicer.mrmlScene.AddNode(self.node)
        self.pipeline.mockUpdatePipeline.assert_called_once()

    def test_pipelines_update_display_is_called_at_reset(self):
        slicer.mrmlScene.AddNode(self.node)
        self.pipeline.mockUpdatePipeline.reset_mock()

        self.pipeline.ResetDisplay()
        self.pipeline.mockUpdatePipeline.assert_called_once()

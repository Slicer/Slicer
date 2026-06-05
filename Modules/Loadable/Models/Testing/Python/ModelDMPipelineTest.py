import slicer
from slicer.ScriptedLoadableModule import ScriptedLoadableModuleTest
import vtk


class ModelDMPipelineTest(ScriptedLoadableModuleTest):
    def setUp(self):
        slicer.mrmlScene.Clear(0)

        # Access the 3D view LayerDM
        three_d_view_node = slicer.mrmlScene.GetNodeByID("vtkMRMLViewNode1")
        app_logic = slicer.app.applicationLogic()
        self.layer_dm = app_logic.GetViewDisplayableManagerByClassName(three_d_view_node, "vtkMRMLLayerDisplayableManager")

    def test_at_model_display_node_creation_creates_a_visible_pipeline(self):
        # Instantiate a sphere model node and create its default display nodes
        sphere = vtk.vtkSphereSource()
        sphere.SetCenter(-6, 30, 28)
        sphere.SetRadius(10)
        model_node = slicer.modules.models.logic().AddModel(sphere.GetOutputPort())
        model_node.CreateDefaultDisplayNodes()
        assert model_node.GetDisplayNode()

        # Check that a pipeline exists for the model_node
        pipeline = self.layer_dm.GetNodePipeline(model_node.GetDisplayNode())
        assert isinstance(pipeline, slicer.vtkMRMLModelDMPipeline)
        assert pipeline.GetModelNode() == model_node

        # Check that at least one of the pipeline's actors is visible
        assert any(prop.GetVisibility() for prop in pipeline.GetProps())

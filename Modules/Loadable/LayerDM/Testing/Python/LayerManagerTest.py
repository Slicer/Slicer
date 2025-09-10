import slicer
from LayerDMManagerLib import vtkMRMLLayerDMScriptedPipeline
from slicer import vtkMRMLLayerDMLayerManager
from slicer.ScriptedLoadableModule import ScriptedLoadableModuleTest
from vtk import vtkRenderWindow, vtkRenderer, vtkCamera


class Pipeline(vtkMRMLLayerDMScriptedPipeline):
    def __init__(self, renderLayer: int = 0, camera: vtkCamera = None):
        super().__init__()
        self._renderLayer = renderLayer
        self._camera = camera

    def GetRenderLayer(self) -> int:
        return self._renderLayer

    def GetCamera(self) -> vtkCamera | None:
        return self._camera


class LayerManagerTest(ScriptedLoadableModuleTest):
    def setUp(self):
        slicer.mrmlScene.Clear(0)
        self.renderWindow = vtkRenderWindow()
        self.defaultRenderer = vtkRenderer()
        self.renderWindow.AddRenderer(self.defaultRenderer)

        self.firstCamera = vtkCamera()
        self.defaultCamera = vtkCamera()
        self.defaultRenderer.SetActiveCamera(self.firstCamera)

        self.layerManager = vtkMRMLLayerDMLayerManager()
        self.layerManager.SetRenderWindow(self.renderWindow)
        self.layerManager.SetDefaultCamera(self.defaultCamera)

    def test_scripted_pipeline_can_be_instantiated(self):
        assert vtkMRMLLayerDMScriptedPipeline() is not None
        assert Pipeline() is not None

    def test_at_init_has_one_distinct_default_layer(self):
        assert self.layerManager.GetNumberOfDistinctLayers() == 1

    def test_adding_pipeline_on_the_default_layer_doesnt_create_layers(self):
        pipelines = [Pipeline() for _ in range(5)]

        for pipeline in pipelines:
            self.layerManager.AddPipeline(pipeline)

        self.assertAreExpectedLayers([pipelines], expRenderLayers=[0])

    def test_adding_pipelines_to_non_default_are_grouped_by_value(self):
        layerValues = [1, 1000, 2000]
        pipelineLists = [[Pipeline(layer) for _ in range(3)] for layer in layerValues]

        for pipelines in pipelineLists:
            for pipeline in pipelines:
                self.layerManager.AddPipeline(pipeline)

        self.assertAreExpectedLayers(pipelineLists, expRenderLayers=[1, 2, 3])

    def test_removed_pipeline_layers_are_collapsed(self):
        layerValues = [1, 1000, 2000]
        pipelineLists = [[Pipeline(layer) for _ in range(3)] for layer in layerValues]

        for pipelines in pipelineLists:
            for pipeline in pipelines:
                self.layerManager.AddPipeline(pipeline)

        for pipeline in pipelineLists[1]:
            self.layerManager.RemovePipeline(pipeline)

        pipelineLists = [pipelineLists[0], pipelineLists[-1]]
        self.assertAreExpectedLayers(pipelineLists, expRenderLayers=[1, 2])

    def test_pipelines_with_different_cameras_map_to_different_layer(self):
        cameras = [vtkCamera(), vtkCamera()]
        pipelineLists = [[Pipeline(1, camera) for _ in range(3)] for camera in cameras]

        for pipelines in pipelineLists:
            for pipeline in pipelines:
                self.layerManager.AddPipeline(pipeline)

        pipelineLists = [pipelineLists[0], pipelineLists[-1]]
        self.assertAreExpectedLayers(pipelineLists, expRenderLayers=[1, 2])

    def test_managed_renderers_are_numbered_from_layer_one_onwards_regardless_of_existing_renderers(self):
        for iUnManaged in range(4):
            renderer = vtkRenderer()
            renderer.SetLayer(iUnManaged)
            self.renderWindow.AddRenderer(renderer)

        pipelines = [Pipeline(1) for _ in range(5)]
        for pipeline in pipelines:
            self.layerManager.AddPipeline(pipeline)

        self.assertAreExpectedLayers([pipelines], expRenderLayers=[1], nUnmanagedRenderers=5, expNumberOfLayers=4)

    def test_cleans_up_render_window_when_changed(self):
        for _ in range(2):
            self.renderWindow.AddRenderer(vtkRenderer())

        pipelines = [Pipeline(1) for _ in range(5)]
        for pipeline in pipelines:
            self.layerManager.AddPipeline(pipeline)

        self.layerManager.SetRenderWindow(vtkRenderWindow())
        assert self.renderWindow.GetRenderers().GetNumberOfItems() == 3
        assert self.renderWindow.GetNumberOfLayers() == 1

    def test_renderers_are_set_to_correct_camera(self):
        customCam = vtkCamera()
        camLayers = [(1, None), (2, customCam), (4, None)]
        pipelineLists = [[Pipeline(layer, camera) for _ in range(3)] for layer, camera in camLayers]

        for pipelines in pipelineLists:
            for pipeline in pipelines:
                self.layerManager.AddPipeline(pipeline)

        renderers = list(self.renderWindow.GetRenderers())
        assert renderers[0].GetActiveCamera() == self.firstCamera
        assert renderers[1].GetActiveCamera() == self.defaultCamera
        assert renderers[2].GetActiveCamera() == customCam
        assert renderers[3].GetActiveCamera() == self.defaultCamera

        for pipeline in pipelineLists[0]:
            self.layerManager.RemovePipeline(pipeline)

        assert renderers[0].GetActiveCamera() == self.firstCamera
        assert renderers[1].GetActiveCamera() == customCam
        assert renderers[2].GetActiveCamera() == self.defaultCamera

    def test_created_renderers_are_set_to_not_interactive(self):
        pipelines = [Pipeline(1) for _ in range(5)]
        for pipeline in pipelines:
            self.layerManager.AddPipeline(pipeline)

        renderers = list(self.renderWindow.GetRenderers())
        assert not renderers[1].GetInteractive()

    def assertAreExpectedLayers(self, pipelineLists, expRenderLayers, nUnmanagedRenderers=1, expNumberOfLayers=None):
        if expNumberOfLayers is None:
            expNumberOfLayers = max(expRenderLayers) + 1

        nManaged = len([layer for layer in expRenderLayers if layer != 0])
        assert self.layerManager.GetNumberOfRenderers() == nManaged
        assert self.layerManager.GetNumberOfManagedLayers() == nManaged

        assert self.renderWindow.GetRenderers().GetNumberOfItems() == nManaged + nUnmanagedRenderers
        assert self.renderWindow.GetNumberOfLayers() == expNumberOfLayers

        actLayers = set()
        for pipelines in pipelineLists:
            # Check that all pipelines map to the same renderer
            renderers = self.getPipelinesRenderers(pipelines)
            assert len(renderers) == 1

            # Check that the renderer has the expected layer
            renderer = renderers[0]
            rendererLayer = renderer.GetLayer()
            actLayers.add(rendererLayer)

            # For layer 0 verify the renderer is the default renderer (and not the default otherwise)
            assert (rendererLayer == 0) == (renderer == self.defaultRenderer)

        assert list(actLayers) == list(expRenderLayers)

    @staticmethod
    def getPipelinesRenderers(pipelines: list[Pipeline]) -> list[vtkRenderer]:
        return list({pipeline.GetRenderer() for pipeline in pipelines})

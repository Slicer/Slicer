import slicer
from LayerDMLib import vtkMRMLLayerDMScriptedPipeline
from slicer import vtkMRMLLayerDMLayerManager
from slicer.ScriptedLoadableModule import ScriptedLoadableModuleTest
from vtk import vtkActor, vtkCamera, vtkPolyDataMapper, vtkRenderWindow, vtkRenderer, vtkSphereSource


class Pipeline(vtkMRMLLayerDMScriptedPipeline):
    """
    Simple sphere pipeline displaying a sphere with given render order / radius and center position.
    """

    def __init__(
        self, renderOrder: int = 0, camera: vtkCamera = None, radius: float = 2, center: list[float] | None = None
    ):
        super().__init__()
        center = center or [0.0] * 3
        self._renderOrder = renderOrder
        self._camera = camera

        self._sphere = vtkSphereSource()
        self._sphere.SetRadius(radius)
        self._sphere.SetCenter(*center)
        self._sphere.Update()
        self._mapper = vtkPolyDataMapper()
        self._mapper.SetInputData(self._sphere.GetOutput())
        self._actor = vtkActor()
        self._actor.SetMapper(self._mapper)

    def OnRendererAdded(self, renderer: vtkRenderer | None) -> None:
        if not renderer or renderer.HasViewProp(self._actor):
            return

        renderer.AddViewProp(self._actor)

    def OnRendererRemoved(self, renderer: vtkRenderer) -> None:
        if not renderer or not renderer.HasViewProp(self._actor):
            return

        renderer.RemoveViewProp(self._actor)

    def GetRenderOrder(self) -> int:
        return self._renderOrder

    def GetCustomCamera(self) -> vtkCamera | None:
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

    def assert_are_expected_layers(self, pipelineLists, expRenderLayers, nUnmanagedRenderers=1, expNumberOfLayers=None):
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
            renderers = self.get_pipelines_renderers(pipelines)
            assert len(renderers) == 1

            # Check that the renderer has the expected layer
            renderer = renderers[0]
            rendererLayer = renderer.GetLayer()
            actLayers.add(rendererLayer)

            # For layer 0 verify the renderer is the default renderer (and not the default otherwise)
            assert (rendererLayer == 0) == (renderer == self.defaultRenderer)

        assert list(actLayers) == list(expRenderLayers)

    @staticmethod
    def get_pipelines_renderers(pipelines: list[Pipeline]) -> list[vtkRenderer]:
        return list({pipeline.GetRenderer() for pipeline in pipelines})

    def configure_layer_manager_with_multiple_pipelines(self):
        # Create sphere pipelines with different centers and add them respectively to different renderers
        cameras = [None, None, None, vtkCamera()]
        renderOrders = [0, 1, 2, 3]
        radius = 2
        centers = [[0, 0, 0], [10, 10, 10], [-10, -10, -10], [0, 0, 0]]

        # Add pipelines to the layer manager
        pipelines = [Pipeline(order, cam, radius, center) for order, center, cam in zip(renderOrders, centers, cameras)]

        for pipeline in pipelines:
            self.layerManager.AddPipeline(pipeline)

        return pipelines

    def test_scripted_pipeline_can_be_instantiated(self):
        assert vtkMRMLLayerDMScriptedPipeline() is not None
        assert Pipeline() is not None

    def test_at_init_has_one_distinct_default_layer(self):
        assert self.layerManager.GetNumberOfDistinctLayers() == 1

    def test_adding_pipeline_with_default_order_doesnt_create_layers(self):
        pipelines = [Pipeline() for _ in range(5)]

        for pipeline in pipelines:
            self.layerManager.AddPipeline(pipeline)

        self.assert_are_expected_layers([pipelines], expRenderLayers=[0])

    def test_adding_pipelines_to_non_default_order_are_grouped_by_value(self):
        orderValues = [1, 1000, 2000]
        pipelineLists = [[Pipeline(order) for _ in range(3)] for order in orderValues]

        for pipelines in pipelineLists:
            for pipeline in pipelines:
                self.layerManager.AddPipeline(pipeline)

        self.assert_are_expected_layers(pipelineLists, expRenderLayers=[1, 2, 3])

    def test_removed_pipeline_layers_are_collapsed(self):
        orderValues = [1, 1000, 2000]
        pipelineLists = [[Pipeline(order) for _ in range(3)] for order in orderValues]

        for pipelines in pipelineLists:
            for pipeline in pipelines:
                self.layerManager.AddPipeline(pipeline)

        for pipeline in pipelineLists[1]:
            self.layerManager.RemovePipeline(pipeline)

        pipelineLists = [pipelineLists[0], pipelineLists[-1]]
        self.assert_are_expected_layers(pipelineLists, expRenderLayers=[1, 2])

    def test_pipelines_with_different_cameras_map_to_different_layer(self):
        cameras = [vtkCamera(), vtkCamera()]
        pipelineLists = [[Pipeline(1, camera) for _ in range(3)] for camera in cameras]

        for pipelines in pipelineLists:
            for pipeline in pipelines:
                self.layerManager.AddPipeline(pipeline)

        pipelineLists = [pipelineLists[0], pipelineLists[-1]]
        self.assert_are_expected_layers(pipelineLists, expRenderLayers=[1, 2])

    def test_managed_renderers_are_numbered_from_layer_one_onwards_regardless_of_existing_renderers(self):
        for iUnManaged in range(4):
            renderer = vtkRenderer()
            renderer.SetLayer(iUnManaged)
            self.renderWindow.AddRenderer(renderer)

        pipelines = [Pipeline(1) for _ in range(5)]
        for pipeline in pipelines:
            self.layerManager.AddPipeline(pipeline)

        self.assert_are_expected_layers([pipelines], expRenderLayers=[1], nUnmanagedRenderers=5, expNumberOfLayers=4)

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
        assert renderers[3].GetActiveCamera() == self.defaultCamera
        assert renderers[2].GetActiveCamera() == customCam

        for pipeline in pipelineLists[0]:
            self.layerManager.RemovePipeline(pipeline)

        assert renderers[0].GetActiveCamera() == self.firstCamera
        assert renderers[1].GetActiveCamera() == customCam
        assert renderers[2].GetActiveCamera() == self.defaultCamera

    def test_renderer_cameras_are_set_to_default_camera(self):
        customCam = vtkCamera()

        camLayers = [(1, None), (2, customCam), (4, None)]
        pipelineLists = [[Pipeline(layer, camera) for _ in range(3)] for layer, camera in camLayers]

        for pipelines in pipelineLists:
            for pipeline in pipelines:
                self.layerManager.AddPipeline(pipeline)

        # The camera sync has the responsibility of updating the default camera when the first cam is updated.
        self.firstCamera.SetDistance(1)
        customCam.SetDistance(2)
        self.defaultCamera.SetDistance(3)

        renderers = list(self.renderWindow.GetRenderers())
        assert renderers[0].GetActiveCamera().GetDistance() == 1
        assert renderers[1].GetActiveCamera().GetDistance() == 3
        assert renderers[2].GetActiveCamera().GetDistance() == 2
        assert renderers[3].GetActiveCamera().GetDistance() == 3

    def test_created_renderers_are_set_to_not_interactive(self):
        pipelines = [Pipeline(1) for _ in range(5)]
        for pipeline in pipelines:
            self.layerManager.AddPipeline(pipeline)

        renderers = list(self.renderWindow.GetRenderers())
        assert not renderers[1].GetInteractive()

    def test_clips_depending_on_renderer_roi(self):
        self.configure_layer_manager_with_multiple_pipelines()

        # Reset the clipping and expect all cameras clipping range to have been updated
        renderers = list(self.renderWindow.GetRenderers())
        prev_clipping_ranges = [renderer.GetActiveCamera().GetClippingRange() for renderer in renderers]

        self.layerManager.ResetCameraClippingRange()
        next_clipping_ranges = [renderer.GetActiveCamera().GetClippingRange() for renderer in renderers]

        for prev_clipping, next_clipping in zip(prev_clipping_ranges, next_clipping_ranges):
            assert prev_clipping != next_clipping

    def test_on_reset_camera_clipping_ranges_are_valid(self):
        self.configure_layer_manager_with_multiple_pipelines()
        self.layerManager.ResetCameraClippingRange()
        renderers = list(self.renderWindow.GetRenderers())

        for renderer in renderers:
            clippingRange = renderer.GetActiveCamera().GetClippingRange()
            assert -1000 < clippingRange[0] < 1000
            assert -1000 < clippingRange[1] < 1000

    def test_reset_clipping_range_affects_unmanaged_cameras(self):
        pipelines = self.configure_layer_manager_with_multiple_pipelines()
        custom_camera = pipelines[-1].GetCustomCamera()
        assert custom_camera is not None

        prev_clipping_range = custom_camera.GetClippingRange()
        self.layerManager.ResetCameraClippingRange()

        assert custom_camera.GetClippingRange()[0] != prev_clipping_range[0]
        assert custom_camera.GetClippingRange()[1] != prev_clipping_range[1]

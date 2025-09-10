import slicer
from slicer import vtkMRMLLayerDMCameraSynchronizer, vtkMRMLViewNode, vtkMRMLLayerDMPipelineManager, vtkMRMLSliceNode
from slicer.ScriptedLoadableModule import ScriptedLoadableModuleTest
from vtk import vtkRenderWindow, vtkRenderer, vtkCamera


class CameraSynchronizerTest(ScriptedLoadableModuleTest):
    def setUp(self):
        slicer.mrmlScene.Clear(0)
        self.renderWindow = vtkRenderWindow()
        self.renderer = vtkRenderer()
        self.renderWindow.AddRenderer(self.renderer)
        self.defaultCam = vtkCamera()
        self.cameraSync = vtkMRMLLayerDMCameraSynchronizer()
        self.cameraSync.SetRenderer(self.renderer)
        self.cameraSync.SetDefaultCamera(self.defaultCam)

    def test_with_threed_view_node_synchronize_cam_on_active(self):
        cam1 = vtkCamera()
        cam1.SetDistance(1)
        self.cameraSync.SetViewNode(vtkMRMLViewNode())

        self.renderer.SetActiveCamera(cam1)
        assert self.defaultCam.GetDistance() == 1

        cam1.SetDistance(3)
        assert self.defaultCam.GetDistance() == 3

        cam2 = vtkCamera()
        cam2.SetDistance(4)
        self.renderer.SetActiveCamera(cam2)
        assert self.defaultCam.GetDistance() == 4

    def test_with_slice_view_synchronize_on_slice_modified(self):
        preMTime = self.defaultCam.GetMTime()

        sliceNode = vtkMRMLSliceNode()
        self.cameraSync.SetViewNode(sliceNode)
        assert preMTime != self.defaultCam.GetMTime()
        preMTime = self.defaultCam.GetMTime()

        sliceNode.SetXYZOrigin([1, 2, 3])
        assert preMTime != self.defaultCam.GetMTime()

from unittest.mock import MagicMock

import slicer
from slicer import vtkMRMLLayerDMCameraSynchronizer, vtkMRMLSliceNode, vtkMRMLViewNode
from slicer.ScriptedLoadableModule import ScriptedLoadableModuleTest
from vtk import vtkCamera, vtkCommand, vtkRenderWindow, vtkRenderer


class CameraSynchronizerTest(ScriptedLoadableModuleTest):
    def setUp(self):
        slicer.mrmlScene.Clear(0)
        self.renderWindow = vtkRenderWindow()
        self.renderer = vtkRenderer()

        self.renderWindow.AddRenderer(self.renderer)

        self.firstCam = vtkCamera()
        self.defaultCam = vtkCamera()
        self.renderer.SetActiveCamera(self.firstCam)

        self.cameraSync = vtkMRMLLayerDMCameraSynchronizer()
        self.cameraSync.SetRenderer(self.renderer)
        self.cameraSync.SetDefaultCamera(self.defaultCam)

        # Create spy to check if the default camera was modified
        self.mockModified = MagicMock()
        self.cameraSync.AddObserver(vtkCommand.ModifiedEvent, self.mockModified)

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

    def test_updating_the_default_camera_triggers_camera_update_once(self):
        self.cameraSync.SetViewNode(vtkMRMLViewNode())
        self.mockModified.reset_mock()

        self.firstCam.Modified()
        self.mockModified.assert_called_once()

    def test_doesnt_trigger_modified_events_on_camera_update_when_blocked(self):
        assert not self.cameraSync.BlockModified(True)
        self.cameraSync.SetViewNode(vtkMRMLViewNode())
        self.firstCam.Modified()
        self.mockModified.assert_not_called()

        assert self.cameraSync.BlockModified(False)
        self.firstCam.Modified()
        self.mockModified.assert_called_once()

    def test_updating_slice_view_triggers_camera_update_once(self):
        sliceNode = vtkMRMLSliceNode()

        self.cameraSync.SetViewNode(sliceNode)
        self.mockModified.reset_mock()

        sliceNode.SetXYZOrigin([1, 2, 3])
        self.mockModified.assert_called_once()

    def test_at_init_updates_trigger_camera_update_once(self):
        self.cameraSync.SetViewNode(vtkMRMLViewNode())
        self.mockModified.assert_called_once()

    def test_on_first_camera_changed_default_camera_clipping_is_preserved(self):
        self.cameraSync.SetViewNode(vtkMRMLViewNode())
        self.defaultCam.SetClippingRange(1, 42)
        self.mockModified.reset_mock()

        self.firstCam.SetClippingRange(3,12)
        self.mockModified.assert_called_once()
        assert self.defaultCam.GetClippingRange()[0] == 1
        assert self.defaultCam.GetClippingRange()[1] == 42

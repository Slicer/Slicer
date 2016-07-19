import unittest
import slicer


class VolumeRenderingThreeDOnlyLayout(unittest.TestCase):
    def setUp(self):
        pass

    def tearDown(self):
        slicer.mrmlScene.Clear(0)

    def test_VolumeRenderThreeDOnlyLayout(self):
        """
        Test that the following workflow does not segfault:
        - Set 3D-only layout, reinitialize slice widgets
        - Load volume
        - Enter the volume rendering module
        """
        # Set 3D-only layout
        layoutManager = slicer.app.layoutManager()
        layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUp3DView)

        # Reinitialize MRML scene to force re-creating slice widgets
        mrmlScene = layoutManager.mrmlScene()
        layoutManager.setMRMLScene(None)
        layoutManager.setMRMLScene(mrmlScene)

        # Load MRHead volume
        from SampleData import SampleDataLogic
        SampleDataLogic().downloadMRHead()

        # Enter the volume rendering module
        slicer.util.mainWindow().moduleSelector().selectModule('VolumeRendering')

import unittest
import slicer
from slicer.ScriptedLoadableModule import *


class VolumeRenderingThreeDOnlyLayout(ScriptedLoadableModuleTest):
    """
    This is the test case for your scripted module.
    Uses ScriptedLoadableModuleTest base class, available at:
    https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
    """
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
        self.delayDisplay("Starting the test")

        # Set 3D-only layout
        layoutManager = slicer.app.layoutManager()
        layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUp3DView)

        # Reinitialize MRML scene to force re-creating slice widgets
        mrmlScene = layoutManager.mrmlScene()
        layoutManager.setMRMLScene(None)
        layoutManager.setMRMLScene(mrmlScene)

        # Load MRHead volume
        import SampleData
        SampleData.downloadSample("MRHead")

        # Enter the volume rendering module
        slicer.util.mainWindow().moduleSelector().selectModule('VolumeRendering')

        self.delayDisplay('Test passed!')

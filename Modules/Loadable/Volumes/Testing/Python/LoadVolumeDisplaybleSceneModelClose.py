import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

class VolumesLoadSceneCloseTesting(ScriptedLoadableModuleTest):

  def setUp(self):
    pass

  def test_LoadVolumeCloseScene(self):
    """
    Load a volume, go to a module that has a displayable scene model set for the tree view, then close the scene.
    Tests the case of closing a scene with a displayable node in it while a GUI is up that is showing a tree view with a displayable scene model (display nodes are set to null during scene closing and can trigger events).
    """
    self.delayDisplay("Starting the test")

    #
    # first, get some sample data
    #
    import SampleData
    SampleData.downloadSample("MRHead")

    #
    # enter the models module
    #
    mainWindow = slicer.util.mainWindow()
    mainWindow.moduleSelector().selectModule('Models')

    #
    # close the scene
    #
    slicer.mrmlScene.Clear(0)

    self.delayDisplay('Test passed')

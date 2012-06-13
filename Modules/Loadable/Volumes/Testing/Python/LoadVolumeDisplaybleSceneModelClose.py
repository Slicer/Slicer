import unittest
from  __main__ import vtk, qt, ctk, slicer


class VolumesLoadSceneCloseTesting(unittest.TestCase):
  def setUp(self):
    pass

  def test_LoadVolumeCloseScene(self):
    """
    Load a volume, go to a module that has a displayable scene model set for the tree view, then close the scene.
    Tests the case of closing a scene with a displayable node in it while a GUI is up that is showing a tree view with a displayable scene model (display nodes are set to null during scene closing and can trigger events).
    """

    #
    # first, get some sample data
    #
    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    head = sampleDataLogic.downloadMRHead()

    #
    # enter the models module
    #
    mainWindow = slicer.util.mainWindow()
    mainWindow.moduleSelector().selectModule('Models')

    #
    # close the scene
    #
    slicer.mrmlScene.Clear(0)

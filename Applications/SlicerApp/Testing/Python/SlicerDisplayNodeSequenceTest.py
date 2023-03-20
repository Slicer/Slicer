import slicer
from slicer.ScriptedLoadableModule import *


#
# SlicerDisplayNodeSequenceTest
#
class SlicerDisplayNodeSequenceTest(ScriptedLoadableModule):
    """Uses ScriptedLoadableModule base class, available at:
    https://githuindex_1_display_node.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "SlicerDisplayNodeSequenceTest"
        self.parent.categories = ["Testing.TestCases"]
        self.parent.dependencies = []
        self.parent.contributors = ["James Butler (PerkinElmer)"]
        self.parent.helpText = """
      This test has been added to check that a vtkMRMLScalarVolumeDisplayNode is appropriately copied into a sequence node.
      """
        self.parent.acknowledgementText = """
    This file was originally developed by James Butler, PerkinElmer.
    """


#
# SlicerDisplayNodeSequenceTestWidget
#
class SlicerDisplayNodeSequenceTestWidget(ScriptedLoadableModuleWidget):
    """
    """

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)


#
# SlicerDisplayNodeSequenceTestLogic
#
class SlicerDisplayNodeSequenceTestLogic(ScriptedLoadableModuleLogic):
    """
    """


class SlicerDisplayNodeSequenceTestTest(ScriptedLoadableModuleTest):
    """
    """

    def setUp(self):
        """ Do whatever is needed to reset the state - typically a scene clear will be enough.
        """
        slicer.mrmlScene.Clear(0)

    def runTest(self):
        """Run as few or as many tests as needed here.
        """
        self.setUp()
        self.test_ScalarVolumeDisplayNodeSequence()
        self.delayDisplay('Test completed.')

    def test_ScalarVolumeDisplayNodeSequence(self):
        # Load first volume and apply custom display
        import SampleData
        sampleDataLogic = SampleData.SampleDataLogic()
        mrHead = sampleDataLogic.downloadMRHead()
        mrHead.GetDisplayNode().ApplyThresholdOn()
        mrHead.GetDisplayNode().SetAutoWindowLevel(False)
        mrHead.GetDisplayNode().SetWindowLevelMinMax(10, 120)
        min, max = mrHead.GetImageData().GetScalarRange()
        mrHead.GetDisplayNode().SetThreshold(0, max)
        window_preset = 99
        level_preset = 49
        mrHead.GetDisplayNode().AddWindowLevelPreset(window_preset, level_preset)

        # Load second volume and apply custom display
        mrHead2 = sampleDataLogic.downloadMRHead()
        mrHead2.GetDisplayNode().SetAndObserveColorNodeID("vtkMRMLColorTableNodeRed")
        mrHead2.GetDisplayNode().ApplyThresholdOn()
        min, max = mrHead2.GetImageData().GetScalarRange()
        mrHead2.GetDisplayNode().SetThreshold(79, max)
        mrHead2.GetDisplayNode().ApplyThresholdOn()
        mrHead2.GetDisplayNode().SetAutoWindowLevel(False)
        mrHead2.GetDisplayNode().SetWindowLevelMinMax(20, 100)
        window_preset = 99
        level_preset = 49
        mrHead2.GetDisplayNode().AddWindowLevelPreset(window_preset, level_preset)
        mrHead2.GetDisplayNode().AddWindowLevelPreset(window_preset - 1, level_preset - 1)

        # Create a vtkMRMLScalarVolumeNode sequence
        sequence_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSequenceNode", "MySequenceNode")
        sequence_node.SetDataNodeAtValue(mrHead, "0")
        sequence_node.SetDataNodeAtValue(mrHead2, "1")

        # Create a vtkMRMLScalarVolumeDisplayNode sequence
        sequence_display_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSequenceNode", "MySequenceDisplayNode")
        sequence_display_node.SetDataNodeAtValue(mrHead.GetDisplayNode(), "0")
        sequence_display_node.SetDataNodeAtValue(mrHead2.GetDisplayNode(), "1")

        # Synchronize the two sequences and display
        browser_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSequenceBrowserNode", "MyBrowserNode")
        browser_node.AddSynchronizedSequenceNode(sequence_node)
        browser_node.AddSynchronizedSequenceNode(sequence_display_node)
        volume_proxy_node = browser_node.GetProxyNode(sequence_node)
        display_proxy_node = browser_node.GetProxyNode(sequence_display_node)
        volume_proxy_node.SetAndObserveDisplayNodeID(display_proxy_node.GetID())
        slicer.modules.sequences.toolBar().setActiveBrowserNode(browser_node)
        slicer.util.setSliceViewerLayers(background=volume_proxy_node)

        # Confirm the display properties have copied over into the Display sequence data nodes
        index_0_display_node = sequence_display_node.GetDataNodeAtValue("0")
        self.assertTrue(index_0_display_node.GetColorNodeID() == mrHead.GetDisplayNode().GetColorNodeID())
        self.assertTrue(index_0_display_node.GetWindowLevelMin() == mrHead.GetDisplayNode().GetWindowLevelMin())
        self.assertTrue(index_0_display_node.GetWindowLevelMax() == mrHead.GetDisplayNode().GetWindowLevelMax())
        self.assertTrue(index_0_display_node.GetLowerThreshold() == mrHead.GetDisplayNode().GetLowerThreshold())
        self.assertTrue(index_0_display_node.GetUpperThreshold() == mrHead.GetDisplayNode().GetUpperThreshold())
        self.assertTrue(index_0_display_node.GetNumberOfWindowLevelPresets() == mrHead.GetDisplayNode().GetNumberOfWindowLevelPresets())
        for i in range(mrHead.GetDisplayNode().GetNumberOfWindowLevelPresets()):
            self.assertTrue(index_0_display_node.GetWindowPreset(i) == mrHead.GetDisplayNode().GetWindowPreset(i))
            self.assertTrue(index_0_display_node.GetLevelPreset(i) == mrHead.GetDisplayNode().GetLevelPreset(i))

        index_1_display_node = sequence_display_node.GetDataNodeAtValue("1")
        self.assertTrue(index_1_display_node.GetColorNodeID() == mrHead2.GetDisplayNode().GetColorNodeID())
        self.assertTrue(index_1_display_node.GetWindowLevelMin() == mrHead2.GetDisplayNode().GetWindowLevelMin())
        self.assertTrue(index_1_display_node.GetWindowLevelMax() == mrHead2.GetDisplayNode().GetWindowLevelMax())
        self.assertTrue(index_1_display_node.GetLowerThreshold() == mrHead2.GetDisplayNode().GetLowerThreshold())
        self.assertTrue(index_1_display_node.GetUpperThreshold() == mrHead2.GetDisplayNode().GetUpperThreshold())
        self.assertTrue(index_1_display_node.GetNumberOfWindowLevelPresets() == mrHead2.GetDisplayNode().GetNumberOfWindowLevelPresets())
        for i in range(mrHead2.GetDisplayNode().GetNumberOfWindowLevelPresets()):
            self.assertTrue(index_1_display_node.GetWindowPreset(i) == mrHead2.GetDisplayNode().GetWindowPreset(i))
            self.assertTrue(index_1_display_node.GetLevelPreset(i) == mrHead2.GetDisplayNode().GetLevelPreset(i))

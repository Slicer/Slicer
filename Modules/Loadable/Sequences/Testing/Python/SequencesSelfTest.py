import os
import shutil

import slicer
from slicer.ScriptedLoadableModule import *


#
# SequencesSelfTest
#


class SequencesSelfTest(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "SequencesSelfTest"
        self.parent.categories = ["Testing.TestCases"]
        self.parent.dependencies = ["Sequences"]
        self.parent.contributors = ["Andras Lasso (PerkLab, Queen's)"]
        self.parent.helpText = """This is a self test for Sequences related node and widgets."""
        parent.acknowledgementText = """This file was originally developed by Andras Lasso, PerkLab, Queen's University
            and was funded in part by NIH 1R01HL153166-01 (Computer Modeling of the Tricuspid Valve in Hypoplastic Left Heart Syndrome)"""


#
# SequencesSelfTestWidget
#


class SequencesSelfTestWidget(ScriptedLoadableModuleWidget):
    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)


#
# SequencesSelfTestLogic
#


class SequencesSelfTestLogic(ScriptedLoadableModuleLogic):
    """This class should implement all the actual
    computation done by your module.  The interface
    should be such that other python code can import
    this class and make use of the functionality without
    requiring an instance of the Widget
    """

    def __init__(self):
        pass


class SequencesSelfTestTest(ScriptedLoadableModuleTest):
    """This is the test case for your scripted module."""

    def setUp(self):
        """Do whatever is needed to reset the state - typically a scene clear will be enough."""
        slicer.mrmlScene.Clear(0)

    def runTest(self):
        """Run as few or as many tests as needed here."""
        self.setUp()
        self.test_SequencesSelfTest_FullTest1()

    # ------------------------------------------------------------------------------
    def test_SequencesSelfTest_FullTest1(self):
        # Check for Sequences module
        self.assertTrue(slicer.modules.sequences)

        self.section_SetupPathsAndNames()
        self.section_ReplaySavedSequence()
        self.delayDisplay("Test passed")

    # ------------------------------------------------------------------------------
    def section_SetupPathsAndNames(self):
        # Set constants
        self.sequencesSelfTestDir = slicer.app.temporaryPath + "/SequencesSelfTest"
        print("Test directory: " + self.sequencesSelfTestDir)
        if not os.access(self.sequencesSelfTestDir, os.F_OK):
            os.mkdir(self.sequencesSelfTestDir)
        self.assertTrue(os.access(self.sequencesSelfTestDir, os.F_OK))

    # ------------------------------------------------------------------------------
    def section_ReplaySavedSequence(self):

        def clearSavedSceneFolder(savedSceneDir):
            # Clear the output savedScene folder
            for sceneFiles in ["SequencesSelfTest.mrml", "Data/CTPCardioSeq.seq.nrrd", "Data/CTPCardioSeq.nrrd", "Data"]:
                try:
                    os.remove(os.path.join(savedSceneDir, sceneFiles))
                except OSError:
                    pass
            shutil.rmtree(savedSceneDir, True)

        def checkSequenceItems(browserNode, sequenceNode):
            volumeNode = browserNode.GetProxyNode(sequenceNode)

            # Check voxel values of item number 5
            browserNode.SetSelectedItemNumber(5)
            voxelArray = slicer.util.arrayFromVolume(volumeNode)
            self.assertAlmostEqual(voxelArray.mean(), -886.678, delta=0.1)

            # Check voxel values of item number 15
            browserNode.SetSelectedItemNumber(15)
            voxelArray = slicer.util.arrayFromVolume(volumeNode)
            self.assertAlmostEqual(voxelArray.mean(), -898.4033, delta=0.1)

        self.delayDisplay("Load sequence data")

        # Get a sequence node
        import SampleData
        sequenceNode = SampleData.SampleDataLogic().downloadSample("CTPCardioSeq")

        # Find corresponding sequence browser node
        browserNode = slicer.modules.sequences.logic().GetFirstBrowserNodeForSequenceNode(sequenceNode)

        # Check sequence information
        self.assertEqual(browserNode.GetNumberOfItems(), 26)
        self.assertEqual(browserNode.GetMasterSequenceNode().GetIndexName(), "frame")
        volumeNode = browserNode.GetProxyNode(sequenceNode)
        self.assertIsNotNone(volumeNode)

        self.delayDisplay("Test browsing of sequence")
        checkSequenceItems(browserNode, sequenceNode)

        # Test saving and loading of sequence
        savedSceneDir = os.path.join(self.sequencesSelfTestDir, "savedscene")
        clearSavedSceneFolder(savedSceneDir)

        if not os.access(savedSceneDir, os.F_OK):
            os.mkdir(savedSceneDir)
        self.assertTrue(os.access(savedSceneDir, os.F_OK))

        self.delayDisplay("Test saving of sequence")
        self.assertTrue(slicer.app.applicationLogic().SaveSceneToSlicerDataBundleDirectory(savedSceneDir, None))

        self.delayDisplay("Test loading of sequence")
        slicer.mrmlScene.Clear(0)
        slicer.util.loadScene(os.path.join(savedSceneDir, "savedscene.mrml"))
        # Remove temporary folder to not pollute the file system
        clearSavedSceneFolder(savedSceneDir)

        self.delayDisplay("Test browsing of loaded sequence")
        browserNode = slicer.util.getFirstNodeByClassByName("vtkMRMLSequenceBrowserNode", "CTPCardioSeq browser")
        self.assertIsNotNone(browserNode)
        sequenceNode = slicer.util.getFirstNodeByClassByName("vtkMRMLSequenceNode", "CTPCardioSeq")
        self.assertIsNotNone(sequenceNode)
        checkSequenceItems(browserNode, sequenceNode)

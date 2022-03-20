import ctk
import qt
import vtk

import slicer
from slicer.ScriptedLoadableModule import *

#
# sceneImport2428
#


class sceneImport2428(ScriptedLoadableModule):
    """Uses ScriptedLoadableModule base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        parent.title = "Scene Import (Issue 2428)"  # make this more human readable by adding spaces
        parent.categories = ["Testing.TestCases"]
        parent.dependencies = []
        parent.contributors = ["Steve Pieper (Isomics)"]  # replace with "Firstname Lastname (Org)"
        parent.helpText = """
    This is an example of scripted loadable module bundled in an extension.
    """
        parent.acknowledgementText = """
    This file was originally developed by Steve Pieper, Isomics, Inc. and was partially funded by NIH grant 3P41RR013218-12S1.
    This is a module to support testing of <a>https://mantisarchive.slicer.org/view.php?id=2428</a>
"""  # replace with organization, grant and thanks.

#
# qsceneImport2428Widget
#


class sceneImport2428Widget(ScriptedLoadableModuleWidget):
    """Uses ScriptedLoadableModuleWidget base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)
        # Instantiate and connect widgets ...

        # Collapsible button
        dummyCollapsibleButton = ctk.ctkCollapsibleButton()
        dummyCollapsibleButton.text = "A collapsible button"
        self.layout.addWidget(dummyCollapsibleButton)

        # Layout within the dummy collapsible button
        dummyFormLayout = qt.QFormLayout(dummyCollapsibleButton)

        # HelloWorld button
        helloWorldButton = qt.QPushButton("Hello world")
        helloWorldButton.toolTip = "Print 'Hello world' in standard output."
        dummyFormLayout.addWidget(helloWorldButton)
        helloWorldButton.connect('clicked(bool)', self.onHelloWorldButtonClicked)

        # Add vertical spacer
        self.layout.addStretch(1)

        # Set local var as instance attribute
        self.helloWorldButton = helloWorldButton

    def onHelloWorldButtonClicked(self):
        print("Hello World !")


class sceneImport2428Test(ScriptedLoadableModuleTest):
    """
    This is the test case for your scripted module.
    Uses ScriptedLoadableModuleTest base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def setUp(self):
        """ Do whatever is needed to reset the state - typically a scene clear will be enough.
        """
        slicer.mrmlScene.Clear(0)

    def runTest(self):
        """Run as few or as many tests as needed here.
        """
        self.setUp()
        self.test_sceneImport24281()

    def test_sceneImport24281(self):
        """ Ideally you should have several levels of tests.  At the lowest level
        tests should exercise the functionality of the logic with different inputs
        (both valid and invalid).  At higher levels your tests should emulate the
        way the user would interact with your code and confirm that it still works
        the way you intended.
        One of the most important features of the tests is that it should alert other
        developers when their changes will have an impact on the behavior of your
        module.  For example, if a developer removes a feature that you depend on,
        your test should break so they know that the feature is needed.
        """

        self.delayDisplay("Starting the test")
        #
        # first, get some data
        #
        self.delayDisplay("Getting Data")
        import SampleData
        head = SampleData.downloadSample("MRHead")

        # Create segmentation
        segmentationNode = slicer.vtkMRMLSegmentationNode()
        slicer.mrmlScene.AddNode(segmentationNode)
        segmentationNode.CreateDefaultDisplayNodes()  # only needed for display
        segmentationNode.SetReferenceImageGeometryParameterFromVolumeNode(head)

        # Add a few segments
        segments = [
            ["Segment A", [0, 65, 32], 25, [1.0, 0.0, 0.0]],
            ["Segment B", [1, -14, 30], 30, [1.0, 1.0, 0.0]],
            ["Segment C", [0, 28, -7], 15, [0.0, 1.0, 1.0]],
            ["Segment D", [31, 33, 27], 25, [0.0, 0.0, 1.0]]]
        for [name, position, radius, color] in segments:
            seed = vtk.vtkSphereSource()
            seed.SetCenter(position)
            seed.SetRadius(radius)
            seed.Update()
            segmentationNode.AddSegmentFromClosedSurfaceRepresentation(seed.GetOutput(), name, color)

        # Export to labelmap volume
        headLabel = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLLabelMapVolumeNode')
        slicer.modules.segmentations.logic().ExportVisibleSegmentsToLabelmapNode(segmentationNode, headLabel, head)

        selectionNode = slicer.app.applicationLogic().GetSelectionNode()
        selectionNode.SetActiveVolumeID(head.GetID())
        selectionNode.SetActiveLabelVolumeID(headLabel.GetID())
        slicer.app.applicationLogic().PropagateVolumeSelection(0)

        #
        # now build:
        # create a model using the command line module
        # based on the current editor parameters
        # - make a new hierarchy node
        #

        self.delayDisplay("Building...")

        parameters = {}
        parameters["InputVolume"] = headLabel.GetID()
        # create models for all labels
        parameters["JointSmoothing"] = True
        parameters["StartLabel"] = -1
        parameters["EndLabel"] = -1
        outHierarchy = slicer.vtkMRMLModelHierarchyNode()
        outHierarchy.SetScene(slicer.mrmlScene)
        outHierarchy.SetName("sceneImport2428Hierachy")
        slicer.mrmlScene.AddNode(outHierarchy)
        parameters["ModelSceneFile"] = outHierarchy

        modelMaker = slicer.modules.modelmaker
        self.CLINode = None
        self.CLINode = slicer.cli.runSync(modelMaker, self.CLINode, parameters, delete_temporary_files=False)

        self.delayDisplay("Models built")

        success = self.verifyModels()

        success = success and (slicer.mrmlScene.GetNumberOfNodesByClass("vtkMRMLModelNode") > 3)

        self.delayDisplay("Test finished")

        if success:
            self.delayDisplay("Ahh... test passed.")
        else:
            self.delayDisplay("!$!$!#!@#!@!@$%! Test Failed!!")

        self.assertTrue(success)

    def verifyModels(self):
        """Return True if the models have unique polydata and have the
        same polydata as their display nodes have.

    # paste this in the slicer console for testing/verifying any scene
    def verifyModels():
      nn = getNodes('*ModelNode*')
      for n in nn:
        mpd = nn[n].GetPolyData()
        mdpd = nn[n].GetDisplayNode().GetInputPolyData()
        if mpd != mdpd:
          print(nn[n].GetName())
          print(mpd,mdpd)

    verifyModels()
        """

        #
        # now check that all models have the same poly data in the
        # model node as in the display node
        #
        polyDataInScene = []
        fileNamesInScene = []
        success = True
        numModels = slicer.mrmlScene.GetNumberOfNodesByClass("vtkMRMLModelNode")
        for n in range(numModels):
            modelNode = slicer.mrmlScene.GetNthNodeByClass(n, "vtkMRMLModelNode")
            polyDataInScene.append(modelNode.GetPolyData())
            for dn in range(modelNode.GetNumberOfDisplayNodes()):
                displayNode = modelNode.GetNthDisplayNode(dn)
                if modelNode.GetPolyData() != displayNode.GetInputPolyData():
                    self.delayDisplay("Model %d does not match its display node %d! (name: %s, ids: %s and %s)" % (n, dn, modelNode.GetName(), modelNode.GetID(), displayNode.GetID()))
                    success = False
            for sn in range(modelNode.GetNumberOfStorageNodes()):
                storageNode = modelNode.GetNthStorageNode(sn)
                fileName = storageNode.GetFileName()
                fileNamesInScene.append(fileName)
                if fileName in fileNamesInScene:
                    self.delayDisplay("Model %d has duplicate file name %s! (ids: %s and %s)" % (n, fileName, modelNode.GetID(), storageNode.GetID()))
                    success = False

        #
        # now check that each model has a unique polydata
        #
        for n in range(numModels):
            modelNode = slicer.mrmlScene.GetNthNodeByClass(n, "vtkMRMLModelNode")
            if polyDataInScene.count(modelNode.GetPolyData()) > 1:
                self.delayDisplay(f"Polydata for Model is duplicated! (id: {n} and {modelNode.GetID()})")
                success = False

        return success

import logging
import os
import vtk

import slicer
from slicer.ScriptedLoadableModule import *


class SlicerScriptedFileReaderWriterTest(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        parent.title = "SlicerScriptedFileReaderWriterTest"
        parent.categories = ["Testing.TestCases"]
        parent.dependencies = []
        parent.contributors = ["Andras Lasso (PerkLab, Queen's)"]
        parent.helpText = """
    This module is used to test qSlicerScriptedFileReader and qSlicerScriptedFileWriter classes.
    """
        parent.acknowledgementText = """
    This file was originally developed by Andras Lasso, PerkLab.
    """
        self.parent = parent


class SlicerScriptedFileReaderWriterTestWidget(ScriptedLoadableModuleWidget):
    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)
        # Default reload&test widgets are enough.
        # Note that reader and writer is not reloaded.


class SlicerScriptedFileReaderWriterTestFileReader:
    def __init__(self, parent):
        self.parent = parent

    def description(self):
        return "My file type"

    def fileType(self):
        return "MyFileType"

    def extensions(self):
        return ["My file type (*.mft)"]

    def canLoadFileConfidence(self, filePath):
        # Only enable this reader in testing mode
        if not slicer.app.testingEnabled():
            return 0.0

        # Check first if loadable based on file extension
        if not self.parent.supportedNameFilters(filePath):
            return 0.0

        firstLine = ""
        with open(filePath) as f:
            firstLine = f.readline()
        fileLooksValid = "magic" in firstLine
        # Default confidence is 0.5 + 0.01 * fileExtensionLength = 0.53,
        # we return a higher value if we recognize this file
        return 0.8 if fileLooksValid else 0.3

    def load(self, properties):
        try:
            filePath = properties["fileName"]

            # Get node base name from filename
            if "name" in properties.keys():
                baseName = properties["name"]
            else:
                baseName = os.path.splitext(os.path.basename(filePath))[0]
                baseName = slicer.mrmlScene.GenerateUniqueName(baseName)

            # Read file content
            with open(filePath) as myfile:
                data = myfile.readlines()

            # Check if file is valid
            firstLine = data[0].rstrip()
            if firstLine != "magic":
                raise ValueError("Cannot read file, it is expected to start with magic")

            # Uncomment the next line to display a warning message to the user.
            # self.parent.userMessages().AddMessage(vtk.vtkCommand.WarningEvent, "This is a warning message")

            # Load content into new node
            loadedNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTextNode", baseName)
            loadedNode.SetText("".join(data[1:]))

            # We always want to save this node in a separate file.
            # Without this, by default short text nodes are saved in the scene file to reduce clutter.
            loadedNode.SetForceCreateStorageNode(True)
            # Add a node attribute to designate this as a particular type of text node.
            # This allows filtering in node selectors and making the custom writer plugin chosen by default.
            loadedNode.SetAttribute("MyFileCategory", "aaa")

            # Set up a custom text storage node to support custom file extension (.mft)
            # The writer plugin could handle custom file extension, but when the scene is loaded/saved
            # without GUI (e.g., when writing to MRML Scene Bundle .mrb file) then it is important
            # that the storage node supports the custom file extension (and uses it as default extension for writing).
            loadedNode.AddDefaultStorageNode()
            storageNode = loadedNode.GetStorageNode()
            storageNode.SetSupportedReadFileExtensions(["mft"])
            storageNode.SetSupportedWriteFileExtensions(["mft"])
            storageNode.SetFileName(filePath)

        except Exception as e:
            import traceback

            traceback.print_exc()
            errorMessage = f"Failed to read file: {str(e)}"
            self.parent.userMessages().AddMessage(vtk.vtkCommand.ErrorEvent, errorMessage)
            return False

        self.parent.loadedNodes = [loadedNode.GetID()]
        return True


class SlicerScriptedFileReaderWriterTestFileWriter:
    def __init__(self, parent):
        self.parent = parent

    def description(self):
        return "My file type"

    def fileType(self):
        return "MyFileType"

    def extensions(self, obj):
        return ["My file type (.mft)"]

    def canWriteObjectConfidence(self, obj):
        # Only enable this writer in testing mode
        if not slicer.app.testingEnabled():
            return 0.0

        if not obj.IsA("vtkMRMLTextNode"):
            return 0.0

        # Select this custom reader by default by returning higher confidence than default
        isMyFileType = obj.GetAttribute("MyFileCategory") == "aaa"
        # Return larger than default confidence (0.8) if we recognize the file (from the attribute)
        # and return with a lower-than default, but still non-zero confidence value to not use this file
        # writer by default but let the user select it manually.
        return 0.8 if isMyFileType else 0.3

    def write(self, properties):
        try:
            # Get node
            node = slicer.mrmlScene.GetNodeByID(properties["nodeID"])

            # Write node content to file
            filePath = properties["fileName"]
            with open(filePath, "w") as myfile:
                myfile.write("magic\n")
                myfile.write(node.GetText())

        except Exception as e:
            import traceback

            traceback.print_exc()
            errorMessage = f"Failed to write file: {str(e)}"
            self.parent.userMessages().AddMessage(vtk.vtkCommand.ErrorEvent, errorMessage)
            return False

        self.parent.writtenNodes = [node.GetID()]
        return True


class SlicerScriptedFileReaderWriterTestTest(ScriptedLoadableModuleTest):
    def runTest(self):
        """Run as few or as many tests as needed here."""
        self.setUp()
        self.test_Writer()
        self.test_Reader()
        self.tearDown()
        self.delayDisplay("Testing complete")

    def setUp(self):
        self.tempDir = slicer.util.tempDirectory()
        logging.info("tempDir: " + self.tempDir)
        self.textInNode = "This is\nsome example test"
        self.validFilename = self.tempDir + "/tempSlicerScriptedFileReaderWriterTestValid.mft"
        self.invalidFilename = self.tempDir + "/tempSlicerScriptedFileReaderWriterTestInvalid.mft"
        slicer.mrmlScene.Clear()

    def tearDown(self):
        import shutil

        shutil.rmtree(self.tempDir, True)

    def test_WriterReader(self):
        # Writer and reader tests are put in the same function to ensure
        # that writing is done before reading (it generates input data for reading).

        self.delayDisplay("Testing node writer")
        slicer.mrmlScene.Clear()
        textNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTextNode")
        textNode.SetAttribute("MyFileCategory", "aaa")
        textNode.SetText(self.textInNode)
        self.assertTrue(slicer.util.saveNode(textNode, self.validFilename, {"fileType": "MyFileType"}))

        self.delayDisplay("Testing node reader")
        slicer.mrmlScene.Clear()
        loadedNode = slicer.util.loadNodeFromFile(self.validFilename, "MyFileType")
        self.assertIsNotNone(loadedNode)
        self.assertTrue(loadedNode.IsA("vtkMRMLTextNode"))
        self.assertEqual(loadedNode.GetText(), self.textInNode)

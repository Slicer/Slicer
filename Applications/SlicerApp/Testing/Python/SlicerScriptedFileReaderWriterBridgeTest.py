import gc
import json
import os
import weakref

import vtk

import slicer
from slicer.ScriptedLoadableModule import *


#
# SlicerScriptedFileReaderWriterBridgeTest
#


class SlicerScriptedFileReaderWriterBridgeTest(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        parent.title = "SlicerScriptedFileReaderWriterBridgeTest"
        parent.categories = ["Testing.TestCases"]
        parent.dependencies = []
        parent.contributors = ["Andras Lasso (PerkLab, Queen's)"]
        parent.helpText = """
    This module is used to test file readers and writers that are implemented in Python by subclassing
    slicer.vtkSlicerScriptedFileReader and slicer.vtkSlicerScriptedFileWriter.
    """
        parent.acknowledgementText = ""
        self.parent = parent


#
# Reader and writer. They are registered automatically, because their names are <ModuleName>FileReader and
# <ModuleName>FileWriter.
#


class Payload:
    """Object stored in an attribute of the reader, to check that attributes are kept."""


class SlicerScriptedFileReaderWriterBridgeTestFileReader(slicer.vtkSlicerScriptedFileReader):
    """Reader of .btf files: first line is "BRIDGE", the rest is the content of a text node."""

    def __init__(self):
        super().__init__()
        self.SetFileType("BridgeTestFile")
        self.SetDescription("Bridge test file")
        self.SetNameFilters(["Bridge test file (*.btf)"])
        # Attributes are kept while only C++ refers to the reader
        self.headerText = "BRIDGE"
        self.payload = Payload()
        self.calls = []

    def CanLoadFileConfidence(self, filePath):
        self.calls.append(("CanLoadFileConfidence", type(filePath).__name__))
        confidence = super().CanLoadFileConfidence(filePath)
        if confidence <= 0.0:
            return 0.0
        with open(filePath) as f:
            return 0.8 if f.readline().strip() == self.headerText else 0.1

    def GetOptionsDescription(self, description):
        self.calls.append(("GetOptionsDescription", type(description).__name__))
        description.AddBoolOption("upperCase", "Upper case", "Convert the text to upper case", False)

    def Load(self, properties):
        self.calls.append(("Load", type(properties).__name__))
        fileNames = properties.GetStringListProperty("fileNames") or [properties.GetStringProperty("fileName")]
        content = ""
        for fileName in fileNames:
            with open(fileName) as f:
                lines = f.read().split("\n", 1)
            if lines[0] != self.headerText:
                self.GetUserMessages().AddMessage(vtk.vtkCommand.ErrorEvent, f"Invalid header in {fileName}")
                return False
            content += lines[1] if len(lines) > 1 else ""
        if properties.GetBoolProperty("upperCase"):
            content = content.upper()
        if "warning" in content.lower():
            self.GetUserMessages().AddMessage(vtk.vtkCommand.WarningEvent, "Content contains a warning")
        node = self.GetScene().AddNewNodeByClass("vtkMRMLTextNode", properties.GetStringProperty("name", "BridgeText"))
        node.SetText(content)
        self.AddLoadedNodeID(node.GetID())
        return True


class SlicerScriptedFileReaderWriterBridgeTestFileWriter(slicer.vtkSlicerScriptedFileWriter):
    """Writer of text nodes into .btf files."""

    def __init__(self):
        super().__init__()
        self.SetFileType("BridgeTestFile")
        self.SetDescription("Bridge test file")
        self.SetNodeClassNames(["vtkMRMLTextNode"])
        self.calls = []

    def CanWriteObjectConfidence(self, obj):
        self.calls.append(("CanWriteObjectConfidence", type(obj).__name__))
        if not super().CanWriteObject(obj):
            return 0.0
        return 0.9 if obj.GetAttribute("BridgeTest") else 0.0

    def GetNameFiltersForObject(self, obj):
        self.calls.append(("GetNameFiltersForObject", type(obj).__name__))
        return ["Bridge test file (*.btf)"]

    def Write(self, properties):
        self.calls.append(("Write", type(properties).__name__))
        node = self.GetScene().GetNodeByID(properties.GetStringProperty("nodeID"))
        with open(properties.GetStringProperty("fileName"), "w") as f:
            f.write("BRIDGE\n" + node.GetText())
        self.AddWrittenNodeID(node.GetID())
        return True


#
# SlicerScriptedFileReaderWriterBridgeTestWidget
#


class SlicerScriptedFileReaderWriterBridgeTestWidget(ScriptedLoadableModuleWidget):
    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)


#
# SlicerScriptedFileReaderWriterBridgeTestTest
#


class SlicerScriptedFileReaderWriterBridgeTestTest(ScriptedLoadableModuleTest):
    def setUp(self):
        self.tempDir = slicer.util.tempDirectory()
        slicer.mrmlScene.Clear()

    def tearDown(self):
        import shutil

        shutil.rmtree(self.tempDir, True)

    def writeFile(self, name, content):
        filePath = os.path.join(self.tempDir, name)
        with open(filePath, "w") as f:
            f.write(content)
        return filePath

    def standaloneManager(self):
        """File IO manager that is not used by the application (so that test readers do not interfere)."""
        fileIOManager = slicer.vtkMRMLFileIOManager()
        fileIOManager.SetScene(slicer.mrmlScene)
        return fileIOManager

    def test_Registration(self):
        """Readers and writers of scripted modules are registered automatically and the manager returns the Python objects."""
        fileIOManager = slicer.app.applicationLogic().GetFileIOManager()
        reader = fileIOManager.GetReaderByDescription("Bridge test file")
        self.assertIsInstance(reader, SlicerScriptedFileReaderWriterBridgeTestFileReader)
        self.assertIsInstance(reader, slicer.vtkSlicerScriptedFileReader)
        self.assertEqual(reader.GetClassName(), "vtkSlicerScriptedFileReaderBridge")
        writers = fileIOManager.GetWritersForFileType("BridgeTestFile")
        self.assertEqual(len(writers), 1)
        self.assertIsInstance(writers[0], SlicerScriptedFileReaderWriterBridgeTestFileWriter)

        # Attributes are kept while only C++ refers to the reader
        payloadReference = weakref.ref(reader.payload)
        del reader, writers
        gc.collect()
        reader = fileIOManager.GetReaderByDescription("Bridge test file")
        self.assertEqual(reader.headerText, "BRIDGE")
        self.assertIs(reader.payload, payloadReference())

    def test_WriteAndRead(self):
        """Writing and reading using the application's file IO manager (called from C++)."""
        fileIOManager = slicer.app.applicationLogic().GetFileIOManager()
        reader = fileIOManager.GetReaderByDescription("Bridge test file")
        writer = fileIOManager.GetWritersForFileType("BridgeTestFile")[0]
        reader.calls.clear()
        writer.calls.clear()

        textNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTextNode", "Original")
        textNode.SetText("Some text\nwith a warning")
        textNode.SetAttribute("BridgeTest", "1")
        self.assertEqual(fileIOManager.GetFileWriterFileType(textNode), "BridgeTestFile")
        self.assertIn("Bridge test file (*.btf)", fileIOManager.GetFileWriterExtensions(textNode))
        filePath = os.path.join(self.tempDir, "saved.btf")
        self.assertTrue(slicer.util.saveNode(textNode, filePath, {"fileType": "BridgeTestFile"}))
        self.assertEqual(writer.GetWrittenNodeIDs(), (textNode.GetID(),))
        with open(filePath) as f:
            self.assertEqual(f.read(), "BRIDGE\nSome text\nwith a warning")

        # Methods are called with the expected argument types
        self.assertIn(("CanWriteObjectConfidence", "vtkMRMLTextNode"), writer.calls)
        self.assertIn(("GetNameFiltersForObject", "vtkMRMLTextNode"), writer.calls)
        self.assertIn(("Write", "vtkMRMLIOProperties"), writer.calls)

        # Reading
        self.assertEqual(fileIOManager.GetFileTypeForFile(filePath), "BridgeTestFile")
        userMessages = slicer.vtkMRMLMessageCollection()
        properties = slicer.vtkMRMLIOProperties()
        properties.SetStringProperty("fileName", filePath)
        properties.SetBoolProperty("upperCase", True)
        loadedNodes = vtk.vtkCollection()
        self.assertTrue(fileIOManager.LoadNodes("BridgeTestFile", properties, loadedNodes, userMessages))
        self.assertEqual(loadedNodes.GetNumberOfItems(), 1)
        self.assertEqual(loadedNodes.GetItemAsObject(0).GetText(), "SOME TEXT\nWITH A WARNING")
        self.assertEqual(len(reader.GetLoadedNodeIDs()), 1)
        self.assertEqual(userMessages.GetNumberOfMessagesOfType(vtk.vtkCommand.WarningEvent), 1)
        self.assertIn(("CanLoadFileConfidence", "str"), reader.calls)
        self.assertIn(("Load", "vtkMRMLIOProperties"), reader.calls)

        # Options
        optionsJSON = reader.GetOptionsDescriptionJSON(properties)
        self.assertIn(("GetOptionsDescription", "vtkMRMLIOOptionsDescription"), reader.calls)
        options = {option["property"]: option for option in json.loads(optionsJSON)["options"]}
        self.assertIn("upperCase", options)

    def test_DefaultsAndSuper(self):
        """Methods that are not overridden use the C++ implementation, calling super() does not recurse."""

        class OnlyLoadReader(slicer.vtkSlicerScriptedFileReader):
            def __init__(self):
                super().__init__()
                self.SetFileType("OnlyLoadFile")
                self.SetNameFilters(["Only load (*.btf)"])

            def Load(self, properties):
                return True

        fileIOManager = self.standaloneManager()
        reader = OnlyLoadReader()
        fileIOManager.RegisterReader(reader)
        filePath = self.writeFile("defaults.btf", "BRIDGE\ntext")
        # Default confidence: 0.5 + 0.01 * length of the matched extension (".btf")
        self.assertAlmostEqual(reader.CanLoadFileConfidence(filePath), 0.54)
        self.assertEqual(fileIOManager.GetReaderForFile(filePath), reader)
        self.assertTrue(reader.CanLoadFile(filePath))
        self.assertFalse(reader.CanLoadFile(filePath + ".txt"))

        # A reader that calls super() gets the C++ value (the test reader returns 0.8 for a valid header)
        testReader = SlicerScriptedFileReaderWriterBridgeTestFileReader()
        self.assertAlmostEqual(testReader.CanLoadFileConfidence(filePath), 0.8)
        fileIOManager.RegisterReader(testReader)
        self.assertEqual(fileIOManager.GetReaderForFile(filePath), testReader)

    def test_ErrorHandling(self):
        """Errors in Python methods are logged, the failing reader or writer does not claim files or nodes,
        and other readers can be still used.
        """

        class FailingReader(slicer.vtkSlicerScriptedFileReader):
            def __init__(self):
                super().__init__()
                self.SetFileType("FailingFile")
                self.SetNameFilters(["Failing (*.fail)"])

            def CanLoadFileConfidence(self, filePath):
                return "not a number"

            def Load(self, properties):
                raise RuntimeError("Load failed on purpose")

        fileIOManager = self.standaloneManager()
        failingReader = FailingReader()
        testReader = SlicerScriptedFileReaderWriterBridgeTestFileReader()
        fileIOManager.RegisterReader(failingReader)
        fileIOManager.RegisterReader(testReader)
        failingFile = self.writeFile("error.fail", "anything")
        validFile = self.writeFile("valid.btf", "BRIDGE\nvalid")

        # Bad return type: the error is logged and the reader does not claim the file
        self.assertIsNone(fileIOManager.GetReaderForFile(failingFile))
        # Exception: loading fails, the error is logged and cleared
        properties = slicer.vtkMRMLIOProperties()
        properties.SetStringProperty("fileName", failingFile)
        self.assertFalse(fileIOManager.LoadNodes("FailingFile", properties))
        # Other readers (and the same reader) can be still used
        properties.SetStringProperty("fileName", validFile)
        self.assertTrue(fileIOManager.LoadNodes("BridgeTestFile", properties))
        self.assertEqual(fileIOManager.GetReaderForFile(validFile), testReader)

        # Exceptions in methods that decide if a file or node is handled: the file or node is not claimed.
        # Unbound calls of the bridge methods call the C++ implementation, which calls the Python method.
        class RaisingReader(slicer.vtkSlicerScriptedFileReader):
            def CanLoadFile(self, filePath):
                raise RuntimeError("CanLoadFile failed on purpose")

            def CanLoadFileConfidence(self, filePath):
                raise RuntimeError("CanLoadFileConfidence failed on purpose")

        class RaisingWriter(slicer.vtkSlicerScriptedFileWriter):
            def CanWriteObject(self, obj):
                raise RuntimeError("CanWriteObject failed on purpose")

            def CanWriteObjectConfidence(self, obj):
                raise RuntimeError("CanWriteObjectConfidence failed on purpose")

        raisingReader = RaisingReader()
        raisingReader.SetNameFilters(["Bridge test (*.btf)"])
        self.assertFalse(slicer.vtkSlicerScriptedFileReaderBridge.CanLoadFile(raisingReader, validFile))
        self.assertEqual(slicer.vtkSlicerScriptedFileReaderBridge.CanLoadFileConfidence(raisingReader, validFile), 0.0)
        raisingWriter = RaisingWriter()
        textNode = slicer.vtkMRMLTextNode()
        self.assertFalse(slicer.vtkSlicerScriptedFileWriterBridge.CanWriteObject(raisingWriter, textNode))
        self.assertEqual(slicer.vtkSlicerScriptedFileWriterBridge.CanWriteObjectConfidence(raisingWriter, textNode), 0.0)

    def test_ValueConversion(self):
        """Values are converted between C++ and Python without crash and without leaving a pending Python error."""
        import numpy as np

        class ConversionReader(slicer.vtkSlicerScriptedFileReader):
            def __init__(self):
                super().__init__()
                self.SetFileType("ConversionFile")
                self.SetNameFilters(["Conversion (*.mft)"])
                self.confidence = 0.0
                self.receivedFilePaths = []

            def CanLoadFileConfidence(self, filePath):
                self.receivedFilePaths.append(filePath)
                return self.confidence

        reader = ConversionReader()
        # Unbound calls of the bridge methods call the C++ implementation, which calls the Python method
        bridge = slicer.vtkSlicerScriptedFileReaderBridge
        filePath = self.writeFile("some.mft", "")
        # Numbers of any type are accepted as confidence value
        for confidence, expectedConfidence in [(1, 1.0), (True, 1.0), (np.float64(0.9), 0.9), (0.25, 0.25)]:
            reader.confidence = confidence
            self.assertAlmostEqual(bridge.CanLoadFileConfidence(reader, filePath), expectedConfidence)
        # A string is not accepted, the reader does not claim the file
        reader.confidence = "0.9"
        self.assertEqual(bridge.CanLoadFileConfidence(reader, filePath), 0.0)
        # No Python error is left pending (the next call works normally)
        reader.confidence = 0.75
        self.assertAlmostEqual(bridge.CanLoadFileConfidence(reader, filePath), 0.75)

        # A non-ASCII file path is received as an equal str
        reader.receivedFilePaths.clear()
        nonAsciiFilePath = "árvíztűrő.mft"
        bridge.CanLoadFileConfidence(reader, nonAsciiFilePath)
        self.assertEqual(reader.receivedFilePaths, [nonAsciiFilePath])
        self.assertIsInstance(reader.receivedFilePaths[0], str)

    def test_Lifetime(self):
        """The reader is deleted when it is no longer used, and the Python object is released."""
        fileIOManager = self.standaloneManager()
        reader = SlicerScriptedFileReaderWriterBridgeTestFileReader()
        fileIOManager.RegisterReader(reader)
        readerReference = vtk.vtkWeakReference()
        readerReference.Set(reader)
        payloadReference = weakref.ref(reader.payload)
        del reader
        gc.collect()

        # The reader is kept alive by the manager, with its Python class and attributes
        filePath = self.writeFile("lifetime.btf", "BRIDGE\ntext")
        self.assertEqual(fileIOManager.GetReaderForFile(filePath).headerText, "BRIDGE")
        self.assertIsNotNone(payloadReference())

        # Unregistering deletes the reader and releases its Python attributes immediately
        # (the temporary Python object that is passed to Unregister is released after the call)
        fileIOManager.Unregister(readerReference.Get())
        self.assertIsNone(readerReference.Get())
        self.assertIsNone(payloadReference())

        # Writer
        writer = SlicerScriptedFileReaderWriterBridgeTestFileWriter()
        fileIOManager.RegisterWriter(writer)
        writerReference = vtk.vtkWeakReference()
        writerReference.Set(writer)
        del writer
        gc.collect()
        self.assertIsNotNone(writerReference.Get())
        fileIOManager.Unregister(writerReference.Get())
        self.assertIsNone(writerReference.Get())

        # Reader deleted from C++ while no Python object refers to it: VTK releases the Python attributes
        # of the deleted object the next time the Python object of a VTK object that has attributes is released
        # while the VTK object still exists.
        otherFileIOManager = self.standaloneManager()
        reader = SlicerScriptedFileReaderWriterBridgeTestFileReader()
        otherFileIOManager.RegisterReader(reader)
        readerReference.Set(reader)
        payloadReference = weakref.ref(reader.payload)
        del reader
        gc.collect()
        del otherFileIOManager  # deletes the reader
        self.assertIsNone(readerReference.Get())
        otherObject = vtk.vtkObject()
        otherObject.someAttribute = 1
        objectCollection = vtk.vtkCollection()
        objectCollection.AddItem(otherObject)
        del otherObject  # the VTK object is kept alive by the collection
        self.assertIsNone(payloadReference())

    def test_BareBridge(self):
        """The bridge classes can be used without a Python subclass: they behave like the C++ base classes."""
        fileIOManager = self.standaloneManager()
        reader = slicer.vtkSlicerScriptedFileReaderBridge()
        reader.SetFileType("BareFile")
        reader.SetNameFilters(["Bare (*.bare)"])
        fileIOManager.RegisterReader(reader)
        filePath = self.writeFile("bare.bare", "text")
        self.assertEqual(fileIOManager.GetReaderForFile(filePath), reader)
        self.assertAlmostEqual(reader.CanLoadFileConfidence(filePath), 0.55)
        properties = slicer.vtkMRMLIOProperties()
        properties.SetStringProperty("fileName", filePath)
        self.assertFalse(fileIOManager.LoadNodes("BareFile", properties))

        writer = slicer.vtkSlicerScriptedFileWriterBridge()
        writer.SetNodeClassNames(["vtkMRMLTextNode"])
        textNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTextNode")
        self.assertTrue(writer.CanWriteObject(textNode))
        self.assertAlmostEqual(writer.CanWriteObjectConfidence(textNode), 0.5)

    def test_MixedRegistration(self):
        """New-style and legacy readers of the same file type: the reader with the higher confidence is used."""

        class LegacyReader:
            def __init__(self, parent):
                self.parent = parent

            def description(self):
                return "Legacy bridge test file"

            def fileType(self):
                return "LegacyBridgeTestFile"

            def extensions(self):
                return ["Legacy bridge test file (*.btf)"]

            def canLoadFileConfidence(self, filePath):
                return 0.6 if self.parent.supportedNameFilters(filePath) else 0.0

            def load(self, properties):
                return True

        filePath = self.writeFile("mixed.btf", "BRIDGE\ntext")
        for legacyFirst in [True, False]:
            fileIOManager = self.standaloneManager()
            legacyReader = slicer.ScriptedFileIO.createScriptedFileReader(LegacyReader)
            self.assertIsInstance(legacyReader, slicer.ScriptedFileIO.LegacyScriptedFileReader)
            newReader = slicer.ScriptedFileIO.createScriptedFileReader(SlicerScriptedFileReaderWriterBridgeTestFileReader)
            self.assertIsInstance(newReader, SlicerScriptedFileReaderWriterBridgeTestFileReader)
            readers = [legacyReader, newReader] if legacyFirst else [newReader, legacyReader]
            for reader in readers:
                fileIOManager.RegisterReader(reader)
            # New-style reader returns 0.8 for a valid header, legacy reader returns 0.6
            self.assertEqual(fileIOManager.GetFileTypesForFile(filePath), ("BridgeTestFile", "LegacyBridgeTestFile"))

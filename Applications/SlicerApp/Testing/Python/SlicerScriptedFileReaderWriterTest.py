import logging
import os
import vtk

import slicer
from slicer.i18n import tr as _
from slicer.ScriptedLoadableModule import *


class SlicerScriptedFileReaderWriterTest(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        parent.title = "SlicerScriptedFileReaderWriterTest"
        parent.categories = ["Testing.TestCases"]
        parent.dependencies = []
        parent.contributors = ["Andras Lasso (PerkLab, Queen's)"]
        parent.helpText = """
    This module is used to test vtkSlicerScriptedFileReader and vtkSlicerScriptedFileWriter classes.
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
        with open(filePath, encoding="latin-1") as f:
            firstLine = f.readline()
        fileLooksValid = firstLine.startswith("magic")
        # Default confidence is 0.5 + 0.01 * fileExtensionLength = 0.53,
        # we return a higher value if we recognize this file
        return 0.8 if fileLooksValid else 0.3

    def getOptionsDescription(self, description):
        """Describe the options that the user can set in the Add data dialog.

        The description is requested again whenever the user changes an option,
        therefore default values and enabled state may depend on the file name
        and on the current value of other options.
        """
        # String option. Default value depends on the file name.
        fileName = description.GetFileName()
        defaultName = os.path.splitext(os.path.basename(fileName))[0] if fileName else ""
        description.AddStringOption("name", _("Name"), _("Name of the loaded text node."), defaultName)

        # Enumeration option (displayed as a combobox)
        description.AddEnumOption("encoding", _("Encoding"), _("Text encoding of the file."), "utf-8")
        description.AddEnumChoice("encoding", "utf-8", _("UTF-8"))
        description.AddEnumChoice("encoding", "latin-1", _("Latin-1"))

        # Boolean option (displayed as a checkbox)
        description.AddBoolOption("limitLines", _("Limit lines"), _("Only load the first lines of the file."), False)

        # Integer option, which is only enabled if another option is checked
        description.AddIntOption("maxLines", _("Maximum lines"), _("Maximum number of lines to load."), 10, 1, 10000)
        description.SetOptionEnabled("maxLines", description.GetBoolOptionValue("limitLines"))

    # Properties of the last load() call (for testing)
    lastLoadProperties = None

    def load(self, properties):
        SlicerScriptedFileReaderWriterTestFileReader.lastLoadProperties = properties
        try:
            filePath = properties["fileName"]

            # Get node base name from filename
            if "name" in properties.keys():
                baseName = properties["name"]
            else:
                baseName = os.path.splitext(os.path.basename(filePath))[0]
                baseName = slicer.mrmlScene.GenerateUniqueName(baseName)

            # Read file content. Options may not be specified (for example, when loading using
            # slicer.util.loadNodeFromFile without properties), therefore default values must be used.
            with open(filePath, encoding=properties.get("encoding", "utf-8"), newline=None) as myfile:
                data = myfile.readlines()

            # Check if file is valid. Header line is: "magic [category]"
            header = data[0].split()
            if not header or header[0] != "magic":
                raise ValueError("Cannot read file, it is expected to start with magic")
            category = header[1] if len(header) > 1 else "aaa"

            content = data[1:]
            if properties.get("limitLines", False):
                content = content[: properties.get("maxLines", 10)]

            # Uncomment the next line to display a warning message to the user.
            # self.parent.userMessages().AddMessage(vtk.vtkCommand.WarningEvent, "This is a warning message")

            # Load content into new node
            loadedNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTextNode", baseName)
            loadedNode.SetText("".join(content))

            # We always want to save this node in a separate file.
            # Without this, by default short text nodes are saved in the scene file to reduce clutter.
            loadedNode.SetForceCreateStorageNode(True)
            # Add a node attribute to designate this as a particular type of text node.
            # This allows filtering in node selectors and making the custom writer plugin chosen by default.
            loadedNode.SetAttribute("MyFileCategory", category)

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

    def getOptionsDescription(self, description):
        """Describe the options that the user can set in the Save data dialog.

        The properties of the description contain the ID of the node that is written ("nodeID"),
        therefore default values may depend on the node.
        """
        description.AddEnumOption("lineEnding", _("Line ending"), _("Line ending characters used in the file."), "LF")
        description.AddEnumChoice("lineEnding", "LF", _("LF (Linux, macOS)"))
        description.AddEnumChoice("lineEnding", "CRLF", _("CRLF (Windows)"))

        # Default value depends on the node that is written
        properties = description.GetProperties()
        node = slicer.mrmlScene.GetNodeByID(properties.GetStringProperty("nodeID")) if properties else None
        defaultCategory = (node.GetAttribute("MyFileCategory") if node else None) or "aaa"
        description.AddStringOption("category", _("Category"), _("Category that is stored in the file header."), defaultCategory)

    def write(self, properties):
        try:
            # Get node
            node = slicer.mrmlScene.GetNodeByID(properties["nodeID"])

            # Write node content to file. Options may not be specified (for example, when saving
            # using slicer.util.saveNode without properties), therefore default values must be used.
            filePath = properties["fileName"]
            lineEnding = "\r\n" if properties.get("lineEnding", "LF") == "CRLF" else "\n"
            category = properties.get("category") or node.GetAttribute("MyFileCategory") or "aaa"
            with open(filePath, "w", newline=lineEnding) as myfile:
                myfile.write(f"magic {category}\n")
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

        self.delayDisplay("Testing property value types")
        slicer.mrmlScene.Clear()
        loadedNode = slicer.util.loadNodeFromFile(self.validFilename, "MyFileType", {
            "myBool": True,
            "myInt": 5,
            "myLargeInt": 5000000000,
            "myFloat": 2.5,
            "myList": ["text", 12, 1.5, False],
            "myMap": {"path": ["a", "b"], "value": 3},
        })
        self.assertIsNotNone(loadedNode)
        receivedProperties = SlicerScriptedFileReaderWriterTestFileReader.lastLoadProperties
        self.assertIs(receivedProperties["myBool"], True)
        self.assertEqual(receivedProperties["myInt"], 5)
        self.assertEqual(receivedProperties["myLargeInt"], 5000000000)
        self.assertEqual(receivedProperties["myFloat"], 2.5)
        self.assertEqual(receivedProperties["myList"], ["text", 12, 1.5, False])
        self.assertIs(receivedProperties["myList"][3], False)
        self.assertEqual(receivedProperties["myMap"], {"path": ["a", "b"], "value": 3})

        self.delayDisplay("Testing reader options")
        fileIOManager = slicer.app.applicationLogic().GetFileIOManager()
        reader = fileIOManager.GetReaderByDescription("My file type")
        self.assertIsNotNone(reader)
        self.assertEqual(reader.GetClassName(), "vtkSlicerScriptedFileReaderBridge")
        properties = slicer.vtkMRMLIOProperties()
        properties.SetStringProperty("fileName", self.validFilename)
        optionValues = slicer.vtkMRMLIOProperties()
        reader.GetOptionValues(properties, optionValues)
        self.assertEqual(optionValues.GetStringProperty("name"), "tempSlicerScriptedFileReaderWriterTestValid")
        self.assertEqual(optionValues.GetStringProperty("encoding"), "utf-8")
        self.assertFalse(optionValues.GetBoolProperty("limitLines"))
        self.assertEqual(optionValues.GetIntProperty("maxLines"), 10)
        # Options description, as the user interface gets it
        description = slicer.vtkMRMLIOOptionsDescription()
        reader.FillOptionsDescription(properties, description)
        optionIndices = {description.GetNthOptionProperty(i): i for i in range(description.GetNumberOfOptions())}
        self.assertEqual(list(optionIndices.keys()), ["name", "encoding", "limitLines", "maxLines"])
        self.assertEqual(description.GetNthOptionType(optionIndices["name"]), "string")
        self.assertEqual(description.GetNthOptionValue(optionIndices["name"]).ToString(), "tempSlicerScriptedFileReaderWriterTestValid")
        encodingIndex = optionIndices["encoding"]
        self.assertEqual(description.GetNthOptionType(encodingIndex), "enum")
        self.assertEqual(description.GetNthOptionNumberOfChoices(encodingIndex), 2)
        self.assertEqual(description.GetNthOptionChoiceValue(encodingIndex, 1).ToString(), "latin-1")
        self.assertEqual(description.GetNthOptionChoiceLabel(encodingIndex, 1), "Latin-1")
        maxLinesIndex = optionIndices["maxLines"]
        self.assertEqual(description.GetNthOptionType(maxLinesIndex), "int")
        self.assertEqual(description.GetNthOptionValue(maxLinesIndex).ToInt(), 10)
        self.assertTrue(description.GetNthOptionHasRange(maxLinesIndex))
        self.assertEqual(description.GetNthOptionMinimum(maxLinesIndex), 1)
        self.assertEqual(description.GetNthOptionMaximum(maxLinesIndex), 10000)
        # maxLines option is only enabled if limitLines is checked
        self.assertFalse(description.GetNthOptionEnabled(maxLinesIndex))
        properties.SetBoolProperty("limitLines", True)
        reader.FillOptionsDescription(properties, description)
        self.assertEqual(description.GetNthOptionValue(optionIndices["limitLines"]).ToInt(), 1)
        self.assertTrue(description.GetNthOptionEnabled(maxLinesIndex))

        # Loading with options
        slicer.mrmlScene.Clear()
        loadedNode = slicer.util.loadNodeFromFile(self.validFilename, "MyFileType", {"name": "LimitedText", "limitLines": True, "maxLines": 1})
        self.assertEqual(loadedNode.GetName(), "LimitedText")
        self.assertEqual(loadedNode.GetText(), self.textInNode.splitlines(keepends=True)[0])

        self.delayDisplay("Testing writer options")
        writer = None
        for writerIndex in range(fileIOManager.GetNumberOfWriters()):
            if fileIOManager.GetNthWriter(writerIndex).GetDescription() == "My file type":
                writer = fileIOManager.GetNthWriter(writerIndex)
        self.assertIsNotNone(writer)
        textNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTextNode")
        textNode.SetAttribute("MyFileCategory", "bbb")
        textNode.SetText(self.textInNode)
        properties = slicer.vtkMRMLIOProperties()
        properties.SetStringProperty("nodeID", textNode.GetID())
        optionValues = slicer.vtkMRMLIOProperties()
        writer.GetOptionValues(properties, optionValues)
        self.assertEqual(optionValues.GetStringProperty("lineEnding"), "LF")
        self.assertEqual(optionValues.GetStringProperty("category"), "bbb")

        # Saving with options
        crlfFilename = self.tempDir + "/tempSlicerScriptedFileReaderWriterTestCrlf.mft"
        self.assertTrue(slicer.util.saveNode(textNode, crlfFilename, {"fileType": "MyFileType", "lineEnding": "CRLF", "category": "ccc"}))
        with open(crlfFilename, "rb") as f:
            fileContent = f.read()
        self.assertTrue(fileContent.startswith(b"magic ccc\r\n"))
        loadedNode = slicer.util.loadNodeFromFile(crlfFilename, "MyFileType")
        self.assertEqual(loadedNode.GetAttribute("MyFileCategory"), "ccc")
        self.assertEqual(loadedNode.GetText(), self.textInNode)

        self.delayDisplay("Testing that additional logic instances do not register readers")
        fileIOManager = slicer.app.applicationLogic().GetFileIOManager()
        numberOfReaders = fileIOManager.GetNumberOfReaders()
        numberOfWriters = fileIOManager.GetNumberOfWriters()
        additionalTablesLogic = slicer.vtkSlicerTablesLogic()
        additionalTablesLogic.SetMRMLApplicationLogic(slicer.app.applicationLogic())
        self.assertEqual(fileIOManager.GetNumberOfReaders(), numberOfReaders)
        self.assertEqual(fileIOManager.GetNumberOfWriters(), numberOfWriters)
        del additionalTablesLogic
        self.assertIsNotNone(fileIOManager.GetReaderByClassName("vtkSlicerTablesReader"))

    def test_OptionsWidgetOwnership(self):
        import qt

        self.delayDisplay("Testing ownership of options widgets")
        ioManager = slicer.app.ioManager()

        # Widget without parent is owned by Python: it is deleted when it is no longer referenced
        destroyed = []
        optionsWidget = ioManager.fileOptionsWidget("Volume")
        self.assertIsNotNone(optionsWidget)
        optionsWidget.connect("destroyed()", lambda: destroyed.append("noParent"))
        del optionsWidget
        slicer.app.processEvents()
        self.assertEqual(destroyed, ["noParent"])

        # Widget with parent is owned by the parent
        destroyed.clear()
        parentWidget = qt.QWidget()
        optionsWidget = ioManager.fileOptionsWidget("Volume", parentWidget)
        self.assertIsNotNone(optionsWidget)
        self.assertEqual(optionsWidget.parent(), parentWidget)
        optionsWidget.connect("destroyed()", lambda: destroyed.append("withParent"))
        del optionsWidget
        slicer.app.processEvents()
        self.assertEqual(destroyed, [])
        del parentWidget
        slicer.app.processEvents()
        self.assertEqual(destroyed, ["withParent"])

    def test_ParentLifetime(self):
        import weakref

        self.delayDisplay("Testing self.parent of scripted readers")
        with open(self.validFilename, "w") as f:
            f.write("magic\n" + self.textInNode)

        instances = []
        deleteEventLog = []

        class LifetimeTestFileReader(SlicerScriptedFileReaderWriterTestFileReader):
            def __init__(self, parent):
                super().__init__(parent)
                instances.append(weakref.ref(self))
                # Using self.parent in a DeleteEvent observer of the reader must not cause infinite recursion
                parent.AddObserver(vtk.vtkCommand.DeleteEvent, lambda caller, event: deleteEventLog.append(repr(self.parent)))

        reader = slicer.ScriptedFileIO.createScriptedFileReader(LifetimeTestFileReader)
        self.assertIsInstance(reader, slicer.ScriptedFileIO.LegacyScriptedFileReader)
        self.assertEqual(len(instances), 1)

        # self.parent behaves like the reader
        parent = instances[0]().parent
        self.assertIsInstance(parent, slicer.vtkSlicerScriptedFileReader)
        self.assertEqual(parent.GetDescription(), "My file type")
        self.assertEqual(parent.GetFileType(), "MyFileType")
        self.assertEqual(list(parent.supportedNameFilters(self.validFilename)), ["My file type (*.mft)"])
        self.assertEqual(parent.userMessages(), reader.GetUserMessages())
        self.assertTrue(hasattr(parent, "userMessages"))
        self.assertEqual(repr(parent), repr(reader))
        # it can be passed to VTK methods
        collection = vtk.vtkCollection()
        collection.AddItem(parent)
        self.assertEqual(collection.GetItemAsObject(0), reader)
        collection.RemoveAllItems()
        del parent

        # Loading reports loaded nodes via self.parent.loadedNodes
        fileIOManager = slicer.vtkMRMLFileIOManager()
        fileIOManager.RegisterReader(reader)
        properties = slicer.vtkMRMLIOProperties()
        properties.SetStringProperty("fileName", self.validFilename)
        self.assertTrue(reader.Load(properties))
        self.assertEqual(len(reader.GetLoadedNodeIDs()), 1)
        loadedNode = slicer.mrmlScene.GetNodeByID(reader.GetLoadedNodeIDs()[0])
        self.assertEqual(loadedNode.GetText(), self.textInNode)
        # The attribute is removed after loading, it is not kept in the reader
        self.assertFalse(hasattr(reader, "loadedNodes"))

        # The reader and the Python object are deleted when the reader is no longer used:
        # self.parent does not create a reference cycle.
        keptParent = instances[0]().parent
        readerReference = vtk.vtkWeakReference()
        readerReference.Set(reader)
        fileIOManager.Unregister(reader)
        self.assertIsNotNone(readerReference.Get())
        del reader
        self.assertIsNone(readerReference.Get())
        self.assertIsNone(instances[0]())
        self.assertEqual(len(deleteEventLog), 1)
        self.assertIn("deleted", deleteEventLog[0])

        # Using self.parent after the reader is deleted raises an error
        with self.assertRaises(ReferenceError):
            keptParent.userMessages()
        self.assertFalse(hasattr(keptParent, "userMessages"))
        self.assertNotIsInstance(keptParent, vtk.vtkObject)
        self.assertIn("deleted", repr(keptParent))

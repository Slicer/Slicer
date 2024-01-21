import pathlib

import qt

from slicer.ScriptedLoadableModule import *

SOMEVAR = "G"


class ModuleG_WithFileReaderAndIOOptions_WithoutWidget(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "Module G"
        self.parent.contributors = ["Jean-Christophe Fillion-Robin (Kitware)"]
        self.parent.helpText = """
    This module allows to test the scripted module import.
    """
        self.parent.acknowledgementText = """
    Developed by Jean-Christophe Fillion-Robin, Kitware Inc.,
    partially funded by NIH grant 3P41RR013218-12S1.
    """

    def somevar(self):
        return SOMEVAR


class ModuleG_WithFileReaderAndIOOptions_WithoutWidgetFileReader:

    def __init__(self, parent):
        self.parent = parent

    def description(self):
        return "My reader with IO options file type"

    def fileType(self):
        return "MyReaderWithIOOptionsFileType"

    def extensions(self):
        return ["My reader with IO options file type (*.mrwift)"]

    def canLoadFile(self, filePath):
        print(filePath)
        return pathlib.Path(filePath).suffix == ".mrwift"

    def load(self, properties):
        print(properties)
        return True


class ModuleG_WithFileReaderAndIOOptions_WithoutWidgetIOOptionsWidget:
    def __init__(self, parent):
        print("Registered ModuleG_WithFileReaderAndIOOptions_WithoutWidgetFileReaderIOOptionsWidget")
        self.parent = parent

        self.setupUi()

        self.coordinateSystemComboBox.connect("currentIndexChanged(int)", self.updateProperties)

    def setupUi(self):
        layout = qt.QHBoxLayout()
        parent.setLayout(layout)

        coordindateSystemlabel = qt.QLabel("Coordinate system:")
        layout.addWidget(coordindateSystemlabel)

        coordindateSystemlabelComboBox = qt.QComboBox()
        coordindateSystemlabelComboBox.addItems(["Default", "LPS", "RAS"])
        layout.addWidget(coordindateSystemlabelComboBox)

        self.coordindateSystemlabel = coordindateSystemlabel
        self.coordindateSystemlabelComboBox = coordindateSystemlabelComboBox

    def updateGUI(self):
        pass

    def updateProperties(self):
        # TODO: Add scripted specific API for updating "qSlicerIOOptionsPrivate::Properties".
        # A new function "qSlicerScriptedIOOptionsWidget::setProperty(const QString& properyName, const QVariant& value)"
        # could be added

        #properties = {}
        #properties["coordinateSystem"] = slicer.vtkMRMLStorageNode.GetCoordinateSystemTypeFromString(
        #    self.coordinateSystemComboBox->currentText())
        pass

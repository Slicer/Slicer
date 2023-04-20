from slicer.ScriptedLoadableModule import *

SOMEVAR = 'F'


class ModuleF_WithFileReader_WithoutWidget(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "Module F"
        self.parent.contributors = ["Jean-Christophe Fillion-Robin (Kitware)", ]
        self.parent.helpText = """
    This module allows to test the scripted module import.
    """
        self.parent.acknowledgementText = """
    Developed by Jean-Christophe Fillion-Robin, Kitware Inc.,
    partially funded by NIH grant 3P41RR013218-12S1.
    """

    def somevar(self):
        return SOMEVAR


class ModuleF_WithFileReader_WithoutWidgetFileReader:

    def __init__(self, parent):
        self.parent = parent

    def description(self):
        return 'My reader file type'

    def fileType(self):
        return 'MyReaderFileType'

    def extensions(self):
        return ['My reader file type (*.mrft)']

    def canLoadFile(self, filePath):
        print(filePath)
        return False

    def load(self, properties):
        print(properties)
        return True

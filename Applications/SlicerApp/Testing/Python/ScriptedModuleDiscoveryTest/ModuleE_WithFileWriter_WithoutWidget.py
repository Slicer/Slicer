from slicer.ScriptedLoadableModule import *

SOMEVAR = "E"


class ModuleE_WithFileWriter_WithoutWidget(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "Module E"
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


class ModuleE_WithFileWriter_WithoutWidgetFileWriter:
    def __init__(self, parent):
        self.parent = parent

    def description(self):
        return "My writer file type"

    def fileType(self):
        return "MyWriterFileType"

    def extensions(self, obj):
        print(obj)
        return ["My writer file type (*.mwft)"]

    def canWriteObject(self, obj):
        return False

    def write(self, properties):
        return True

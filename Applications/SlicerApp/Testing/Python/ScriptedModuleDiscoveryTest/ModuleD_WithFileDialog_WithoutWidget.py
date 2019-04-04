
from slicer.ScriptedLoadableModule import *

SOMEVAR = 'D'

class ModuleD_WithFileDialog_WithoutWidget(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Module A"
    self.parent.contributors = ["Jean-Christophe Fillion-Robin (Kitware)",]
    self.parent.helpText = """
    This module allows to test the scripted module import.
    """
    self.parent.acknowledgementText = """
    Developed by Jean-Christophe Fillion-Robin, Kitware Inc.,
    partially funded by NIH grant 3P41RR013218-12S1.
    """

  def somevar(self):
    return SOMEVAR


class DICOMFileDialog(object):

  def __init__(self,qSlicerFileDialog):
    self.qSlicerFileDialog = qSlicerFileDialog
    qSlicerFileDialog.fileType = 'Foo Directory'
    qSlicerFileDialog.description = 'Do something awesome with Foo'
    qSlicerFileDialog.action = slicer.qSlicerFileDialog.Read

  def execDialog(self):
    pass

  def isMimeDataAccepted(self):
    self.qSlicerFileDialog.acceptMimeData(True)

  def dropEvent(self):
    pass

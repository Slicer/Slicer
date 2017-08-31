
from slicer.ScriptedLoadableModule import *

SOMEVAR = 'B'

class ModuleB(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Module B"
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

class ModuleBWidget(ScriptedLoadableModuleWidget):
  def __init__(self, parent=None):
    ScriptedLoadableModuleWidget.__init__(self, parent)
  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

import os

import slicer.util

from slicer.ScriptedLoadableModule import *


class ModuleCleanup(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Module for cleanup test"
    self.parent.categories = ["ModuleCleanup"]  # Explicitly add a category to work arround issue #4698
    self.parent.contributors = ["Jean-Christophe Fillion-Robin (Kitware)",]
    self.parent.helpText = """
    This module allows to test that exception raised during module cleanup
    sets exit code.
    """
    self.parent.acknowledgementText = """
    Developed by Jean-Christophe Fillion-Robin, Kitware Inc.,
    partially funded by NIH grant 1R01EB021391.
    """

class ModuleCleanupWidget(ScriptedLoadableModuleWidget):
  def __init__(self, parent=None):
    ScriptedLoadableModuleWidget.__init__(self, parent)

    self.testOutputFileName = os.environ['SLICER_MODULE_CLEANUP_TEST_OUTPUT']
    if os.path.isfile(self.testOutputFileName):
      os.remove(self.testOutputFileName)

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    print("ModuleCleanupWidget setup")
  def cleanup(self):
    with open(self.testOutputFileName, "w") as fd:
      fd.write('ModuleCleanup generated this file when application exited')
    raise RuntimeError("ModuleCleanupWidget error")

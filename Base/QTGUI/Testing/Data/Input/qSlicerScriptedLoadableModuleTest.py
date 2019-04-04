

class qSlicerScriptedLoadableModuleTest(object):
  def __init__(self, parent):
    import string
    parent.title = "qSlicerScriptedLoadableModule Test"
    parent.categories = ["Testing"]
    parent.associatedNodeTypes = ["vtkMRMLModelNode", "vtkMRMLScalarVolumeNode"]
    parent.contributors = ["Jean-Christophe Fillion-Robin (Kitware)"]
    parent.helpText = """
    This module is used to test qSlicerScriptedLoadableModule and qSlicerScriptedLoadableModuleWidget classes.
    """
    parent.acknowledgementText = """
    This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc. and was partially funded by NIH grant 3P41RR013218-12S1
    """
    self.parent = parent

  def setup(self):
    self.parent.setProperty('setup_called_within_Python', True)


class qSlicerScriptedLoadableModuleTestWidget(object):
  def __init__(self, parent=None):
    self.parent = parent

  def setup(self):
    self.parent.setProperty('setup_called_within_Python', True)

  def enter(self):
    self.parent.setProperty('enter_called_within_Python', True)

  def exit(self):
    self.parent.setProperty('exit_called_within_Python', True)


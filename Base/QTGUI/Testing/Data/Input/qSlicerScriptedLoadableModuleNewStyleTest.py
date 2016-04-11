

class qSlicerScriptedLoadableModuleNewStyleTest(object):
  def __init__(self, parent):
    parent.title = "qSlicerScriptedLoadableModuleNewStyle Test"
    parent.categories = ["Testing"]
    parent.associatedNodeTypes = ["vtkMRMLModelNode", "vtkMRMLScalarVolumeNode"]
    parent.contributors = ["Jean-Christophe Fillion-Robin (Kitware); Max Smolens (Kitware)"]
    parent.helpText = """
    This module is for testing.
    """
    parent.acknowledgementText = """
    Based on qSlicerScriptedLoadableModuleTest .
    """
    self.parent = parent

  def setup(self):
    self.parent.setProperty('setup_called_within_Python', True)

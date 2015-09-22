

class qSlicerScriptedLoadableModuleNewStyleTestWidget(object):
  def __init__(self, parent=None):
    self.parent = parent

  def setup(self):
    self.parent.setProperty('setup_called_within_Python', True)

  def enter(self):
    self.parent.setProperty('enter_called_within_Python', True)

  def exit(self):
    self.parent.setProperty('exit_called_within_Python', True)

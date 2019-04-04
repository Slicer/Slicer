

class qSlicerScriptedLoadableModuleTestWidget(object):
  def __init__(self, parent=None):
    self.parent = parent

  def setup(self):
    self.parent.setProperty('setup_called_within_Python', True)

  def enter(self):
    self.parent.setProperty('enter_called_within_Python', True)

  def exit(self):
    self.parent.setProperty('exit_called_within_Python', True)

  def setEditedNode(self, node, role = '', context = ''):
    self.parent.setProperty('editedNodeName', node.GetName() if node is not None else "")
    self.parent.setProperty('editedNodeRole', role)
    self.parent.setProperty('editedNodeContext', context)
    return (node is not None)

  def nodeEditable(self, node):
    self.parent.setProperty('editableNodeName', node.GetName() if node is not None else "")
    return 0.7 if node is not None else 0.3

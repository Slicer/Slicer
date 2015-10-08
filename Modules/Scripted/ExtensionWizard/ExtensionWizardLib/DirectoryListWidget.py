import slicer, qt

#=============================================================================
#
# _ui_DirectoryListWidget
#
#=============================================================================
class _ui_DirectoryListWidget(object):
  #---------------------------------------------------------------------------
  def __init__(self, parent):
    layout = qt.QGridLayout(parent)

    self.pathList = slicer.qSlicerDirectoryListView()
    layout.addWidget(self.pathList, 0, 0, 3, 1)

    self.addPathButton = qt.QToolButton()
    self.addPathButton.icon = qt.QIcon.fromTheme("list-add")
    self.addPathButton.text = "Add"
    layout.addWidget(self.addPathButton, 0, 1)

    self.removePathButton = qt.QToolButton()
    self.removePathButton.icon = qt.QIcon.fromTheme("list-remove")
    self.removePathButton.text = "Remove"
    layout.addWidget(self.removePathButton, 1, 1)

#=============================================================================
#
# DirectoryListWidget
#
#=============================================================================
class DirectoryListWidget(qt.QWidget):
  #---------------------------------------------------------------------------
  def __init__(self, *args, **kwargs):
    qt.QWidget.__init__(self, *args, **kwargs)
    self.ui = _ui_DirectoryListWidget(self)

    self.ui.addPathButton.connect('clicked()', self.addDirectory)
    self.ui.removePathButton.connect('clicked()', self.ui.pathList,
                                     'removeSelectedDirectories()')

  #---------------------------------------------------------------------------
  def addDirectory(self):
    path = qt.QFileDialog.getExistingDirectory(self.window(), "Select folder")
    if len(path):
      self.ui.pathList.addDirectory(path)

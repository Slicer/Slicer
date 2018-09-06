import slicer, qt

#-----------------------------------------------------------------------------
def _dialogIcon(icon):
  s = slicer.app.style()
  i = s.standardIcon(icon)
  return i.pixmap(qt.QSize(64, 64))

#=============================================================================
#
# _ui_LoadModulesDialog
#
#=============================================================================
class _ui_LoadModulesDialog(object):
  #---------------------------------------------------------------------------
  def __init__(self, parent):
    vLayout = qt.QVBoxLayout(parent)
    hLayout = qt.QHBoxLayout()

    self.icon = qt.QLabel()
    self.icon.setPixmap(_dialogIcon(qt.QStyle.SP_MessageBoxQuestion))
    hLayout.addWidget(self.icon, 0)

    self.label = qt.QLabel()
    self.label.wordWrap = True
    hLayout.addWidget(self.label, 1)

    vLayout.addLayout(hLayout)

    self.moduleList = qt.QListWidget()
    self.moduleList.selectionMode = qt.QAbstractItemView.NoSelection
    vLayout.addWidget(self.moduleList)

    self.addToSearchPaths = qt.QCheckBox()
    vLayout.addWidget(self.addToSearchPaths)
    self.addToSearchPaths.checked = True

    self.enableDeveloperMode = qt.QCheckBox()
    self.enableDeveloperMode.text = "Enable developer mode"
    self.enableDeveloperMode.toolTip = "Sets the 'Developer mode' application option to enabled. Enabling developer mode is recommended while developing scripted modules, as it makes the Reload and Testing section displayed in the module user interface."
    self.enableDeveloperMode.checked = True
    vLayout.addWidget(self.enableDeveloperMode)

    self.buttonBox = qt.QDialogButtonBox()
    self.buttonBox.setStandardButtons(qt.QDialogButtonBox.Yes |
                                      qt.QDialogButtonBox.No)
    vLayout.addWidget(self.buttonBox)

#=============================================================================
#
# LoadModulesDialog
#
#=============================================================================
class LoadModulesDialog(object):
  #---------------------------------------------------------------------------
  def __init__(self, parent):
    self.dialog = qt.QDialog(parent)
    self.ui = _ui_LoadModulesDialog(self.dialog)

    self.ui.buttonBox.connect("accepted()", self.dialog, "accept()")
    self.ui.buttonBox.connect("rejected()", self.dialog, "reject()")
    self.ui.moduleList.connect("itemChanged(QListWidgetItem*)", self.validate)

  #---------------------------------------------------------------------------
  def validate(self):
    moduleCount = len(self.selectedModules)

    if moduleCount == 0:
      self.ui.buttonBox.button(qt.QDialogButtonBox.Yes).enabled = False
      self.ui.addToSearchPaths.enabled = False

      moduleCount = len(self._moduleItems)

    else:
      self.ui.buttonBox.button(qt.QDialogButtonBox.Yes).enabled = True
      self.ui.addToSearchPaths.enabled = True

    if moduleCount == 1:
      self.ui.addToSearchPaths.text = "Add selected module to search paths"
    else:
      self.ui.addToSearchPaths.text = "Add selected modules to search paths"

    # If developer mode is already enabled then don't even show the option
    developerModeAlreadyEnabled = slicer.util.settingsValue('Developer/DeveloperMode', False, converter=slicer.util.toBool)
    if developerModeAlreadyEnabled:
      self.ui.enableDeveloperMode.visible = False
      self.ui.enableDeveloperMode.checked = False

  #---------------------------------------------------------------------------
  def exec_(self):
    return self.dialog.exec_()

  #---------------------------------------------------------------------------
  def setModules(self, modules):
    self.ui.moduleList.clear()
    self._moduleItems = {}

    for module in modules:
      item = qt.QListWidgetItem(module.key)
      item.setFlags(item.flags() | qt.Qt.ItemIsUserCheckable)
      item.setCheckState(qt.Qt.Checked)
      self.ui.moduleList.addItem(item)
      self._moduleItems[item] = module

    if len(modules) > 1:
      self.ui.label.text = (
        "The following modules can be loaded. "
        "Would you like to load them now?")

    elif len(modules) == 1:
      self.ui.label.text = (
        "The following module can be loaded. "
        "Would you like to load it now?")

    else:
      raise ValueError("At least one module must be provided")

    self.validate()

  #---------------------------------------------------------------------------
  @property
  def addToSearchPaths(self):
    return self.ui.addToSearchPaths.checked

  #---------------------------------------------------------------------------
  @property
  def enableDeveloperMode(self):
    return self.ui.enableDeveloperMode.checked

  #---------------------------------------------------------------------------
  @property
  def selectedModules(self):
    result = []

    for item, module in self._moduleItems.items():
      if item.checkState():
        result.append(module)

    return result

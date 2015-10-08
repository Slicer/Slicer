import os

import qt, ctk

#=============================================================================
#
# _ui_CreateComponentDialog
#
#=============================================================================
class _ui_CreateComponentDialog(object):
  #---------------------------------------------------------------------------
  def __init__(self, parent):
    self.vLayout = qt.QVBoxLayout(parent)
    self.formLayout = qt.QFormLayout()

    self.componentName = qt.QLineEdit()
    self.formLayout.addRow("Name:", self.componentName)

    self.componentType = qt.QComboBox()
    self.formLayout.addRow("Type:", self.componentType)

    self.destination = ctk.ctkPathLineEdit()
    self.destination.filters = ctk.ctkPathLineEdit.Dirs
    self.formLayout.addRow("Destination:", self.destination)

    self.vLayout.addLayout(self.formLayout)
    self.vLayout.addStretch(1)

    self.buttonBox = qt.QDialogButtonBox()
    self.buttonBox.setStandardButtons(qt.QDialogButtonBox.Ok |
                                      qt.QDialogButtonBox.Cancel)
    self.vLayout.addWidget(self.buttonBox)

#=============================================================================
#
# CreateComponentDialog
#
#=============================================================================
class CreateComponentDialog(object):
  #---------------------------------------------------------------------------
  def __init__(self, componenttype, parent):
    self.dialog = qt.QDialog(parent)
    self.ui = _ui_CreateComponentDialog(self.dialog)

    self.ui.buttonBox.connect("accepted()", self.accept)
    self.ui.buttonBox.connect("rejected()", self.dialog, "reject()")

    self._typelc = componenttype.lower()
    self._typetc = componenttype.title()

  #---------------------------------------------------------------------------
  def accept(self):
    if not len(self.componentName):
      qt.QMessageBox.critical(self.dialog, u"Cannot create %s" % self._typelc,
                              u"%s name may not be empty." % self._typetc)
      return

    if self.showDestination:
      dest = self.destination
      if not len(dest) or not os.path.exists(dest):
        qt.QMessageBox.critical(self.dialog, u"Cannot create %s" % self._typelc,
                                u"Destination must be an existing directory.")
        return

    self.dialog.accept()

  #---------------------------------------------------------------------------
  def setTemplates(self, templates, default="default"):
    self.ui.componentType.clear()
    self.ui.componentType.addItems(templates)

    try:
      self.ui.componentType.currentIndex = templates.index(default)
    except ValueError:
      pass

  #---------------------------------------------------------------------------
  def exec_(self):
    return self.dialog.exec_()

  #---------------------------------------------------------------------------
  @property
  def showDestination(self):
    return self.ui.destination.visible

  #---------------------------------------------------------------------------
  @showDestination.setter
  def showDestination(self, value):
    field = self.ui.destination
    label = self.ui.formLayout.labelForField(field)

    label.visible = value
    field.visible = value

  #---------------------------------------------------------------------------
  @property
  def componentName(self):
    return self.ui.componentName.text

  #---------------------------------------------------------------------------
  @property
  def componentType(self):
    return self.ui.componentType.currentText

  #---------------------------------------------------------------------------
  @property
  def destination(self):
    return self.ui.destination.currentPath

import os

from __main__ import qt, ctk

#=============================================================================
#
# _ui_CreateExtensionDialog
#
#=============================================================================
class _ui_CreateExtensionDialog:
  #---------------------------------------------------------------------------
  def __init__(self, parent):
    vLayout = qt.QVBoxLayout(parent)
    formLayout = qt.QFormLayout()

    self.extensionName = qt.QLineEdit()
    formLayout.addRow("Name:", self.extensionName)

    self.extensionType = qt.QComboBox()
    formLayout.addRow("Type:", self.extensionType)

    self.destination = ctk.ctkPathLineEdit()
    self.destination.filters = ctk.ctkPathLineEdit.Dirs
    formLayout.addRow("Destination:", self.destination)

    vLayout.addLayout(formLayout)
    vLayout.addStretch(1)

    self.buttonBox = qt.QDialogButtonBox()
    self.buttonBox.setStandardButtons(qt.QDialogButtonBox.Ok |
                                      qt.QDialogButtonBox.Cancel)
    vLayout.addWidget(self.buttonBox)

#=============================================================================
#
# CreateExtensionDialog
#
#=============================================================================
class CreateExtensionDialog:
  #---------------------------------------------------------------------------
  def __init__(self, parent):
    self.dialog = qt.QDialog(parent)
    self.ui = _ui_CreateExtensionDialog(self.dialog)

    self.ui.buttonBox.connect("accepted()", self.accept)
    self.ui.buttonBox.connect("rejected()", self.dialog, "reject()")

  #---------------------------------------------------------------------------
  def accept(self):
    if not len(self.extensionName()):
      qt.QMessageBox.critical(self.dialog, "Cannot create extension",
                              "Extension name may not be empty.")
      return

    dest = self.destination()
    if not len(dest) or not os.path.exists(dest):
      qt.QMessageBox.critical(self.dialog, "Cannot create extension",
                              "Destination must be an existing directory.")
      return

    self.dialog.accept()

  #---------------------------------------------------------------------------
  def setTemplates(self, templates):
    self.ui.extensionType.clear()
    self.ui.extensionType.addItems(templates)

    try:
      self.ui.extensionType.currentIndex = templates.index("default")
    except ValueError:
      pass

  #---------------------------------------------------------------------------
  def exec_(self):
    return self.dialog.exec_()

  #---------------------------------------------------------------------------
  def extensionName(self):
    return self.ui.extensionName.text

  #---------------------------------------------------------------------------
  def extensionType(self):
    return self.ui.extensionType.currentText

  #---------------------------------------------------------------------------
  def destination(self):
    return self.ui.destination.currentPath

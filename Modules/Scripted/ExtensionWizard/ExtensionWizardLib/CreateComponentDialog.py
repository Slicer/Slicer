import os
import slicer
import qt, ctk
import re

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

    self.componentNameValidator = qt.QRegExpValidator(
        qt.QRegExp(r"^[a-zA-Z_][a-zA-Z0-9_]*$"))
    self.componentName.setValidator(self.componentNameValidator)

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
      slicer.util.errorDisplay("%s name may not be empty." % self._typetc,
                               windowTitle="Cannot create %s" % self._typelc, parent=self.dialog)
      return

    if self.showDestination:
      dest = self.destination
      if not len(dest) or not os.path.exists(dest):
        slicer.util.errorDisplay("Destination must be an existing directory.",
                                 windowTitle="Cannot create %s" % self._typelc, parent=self.dialog)
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

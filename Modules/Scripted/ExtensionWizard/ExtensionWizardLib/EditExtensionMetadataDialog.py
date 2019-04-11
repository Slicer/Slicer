import os
import re
import slicer
import qt, ctk

from .EditableTreeWidget import EditableTreeWidget

#-----------------------------------------------------------------------------
def _map_property(objfunc, name):
  return property(lambda self: getattr(objfunc(self), name),
                  lambda self, value: setattr(objfunc(self), name, value))

#=============================================================================
#
# _ui_EditExtensionMetadataDialog
#
#=============================================================================
class _ui_EditExtensionMetadataDialog(object):
  #---------------------------------------------------------------------------
  def __init__(self, parent):
    vLayout = qt.QVBoxLayout(parent)
    formLayout = qt.QFormLayout()

    self.nameEdit = qt.QLineEdit()
    formLayout.addRow("Name:", self.nameEdit)

    self.categoryEdit = qt.QLineEdit()
    formLayout.addRow("Category:", self.categoryEdit)

    self.descriptionEdit = qt.QTextEdit()
    self.descriptionEdit.acceptRichText = False
    formLayout.addRow("Description:", self.descriptionEdit)

    self.contributorsList = EditableTreeWidget()
    self.contributorsList.rootIsDecorated = False
    self.contributorsList.selectionBehavior = qt.QAbstractItemView.SelectRows
    self.contributorsList.selectionMode = qt.QAbstractItemView.ExtendedSelection
    self.contributorsList.setHeaderLabels(["Name", "Organization"])
    formLayout.addRow("Contributors:", self.contributorsList)

    vLayout.addLayout(formLayout)
    vLayout.addStretch(1)

    self.buttonBox = qt.QDialogButtonBox()
    self.buttonBox.setStandardButtons(qt.QDialogButtonBox.Ok |
                                      qt.QDialogButtonBox.Cancel)
    vLayout.addWidget(self.buttonBox)

#=============================================================================
#
# EditExtensionMetadataDialog
#
#=============================================================================
class EditExtensionMetadataDialog(object):
  project = _map_property(lambda self: self.ui.nameEdit, "text")
  category = _map_property(lambda self: self.ui.categoryEdit, "text")
  description = _map_property(lambda self: self.ui.descriptionEdit, "plainText")

  #---------------------------------------------------------------------------
  def __init__(self, parent):
    self.dialog = qt.QDialog(parent)
    self.ui = _ui_EditExtensionMetadataDialog(self.dialog)

    self.ui.buttonBox.connect("accepted()", self.accept)
    self.ui.buttonBox.connect("rejected()", self.dialog, "reject()")

  #---------------------------------------------------------------------------
  def accept(self):
    if not len(self.project):
      slicer.util.errorDisplay("Extension name may not be empty.", windowTitle="Invalid metadata", parent=self.dialog)
      return

    if not len(self.description):
      slicer.util.errorDisplay("Extension description may not be empty.",
                               windowTitle="Invalid metadata", parent=self.dialog)
      return

    self.dialog.accept()

  #---------------------------------------------------------------------------
  def exec_(self):
    return self.dialog.exec_()

  #---------------------------------------------------------------------------
  @property
  def contributors(self):
    result = []
    for row in range(self.ui.contributorsList.itemCount):
      item = self.ui.contributorsList.topLevelItem(row)
      name = item.text(0)
      organization = item.text(1)
      if len(organization):
        result.append("%s (%s)" % (name, organization))
      else:
        result.append(name)
    return ", ".join(result)

  #---------------------------------------------------------------------------
  @contributors.setter
  def contributors(self, value):
    self.ui.contributorsList.clear()
    for c in re.split("(?<=[)])\s*,", value):
      c = c.strip()
      item = qt.QTreeWidgetItem()

      try:
        n = c.index("(")
        item.setText(0, c[:n].strip())
        item.setText(1, c[n+1:-1].strip())

      except ValueError:
        qt.qWarning("%r: badly formatted contributor" % c)
        item.setText(0, c)

      self.ui.contributorsList.addItem(item)

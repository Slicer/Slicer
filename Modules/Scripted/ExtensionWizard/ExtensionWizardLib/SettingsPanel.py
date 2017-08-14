import qt, ctk

import SlicerWizard.TemplateManager

from .DirectoryListWidget import DirectoryListWidget
from .TemplatePathUtilities import *

#=============================================================================
#
# _ui_SettingsPanel
#
#=============================================================================
class _ui_SettingsPanel(object):
  #---------------------------------------------------------------------------
  def __init__(self, parent):
    self.formLayout = qt.QFormLayout(parent)

    self.builtinPath = qt.QLineEdit()
    builtinPath = builtinTemplatePath()
    if (builtinPath):
      self.builtinPath.text = builtinPath
    else:
      self.builtinPath.text = "(Unavailable)"
      self.builtinPath.enabled = False
    self.builtinPath.readOnly = True
    self.addRow("Built-in template path:", self.builtinPath)

    self.genericPaths = DirectoryListWidget()
    self.addRow("Additional template\npaths:", self.genericPaths)

    self.paths = {}

    for category in SlicerWizard.TemplateManager.categories():
      self.paths[category] = DirectoryListWidget()
      self.addRow("Additional template\npaths for %s:" % category,
                  self.paths[category])

  #---------------------------------------------------------------------------
  def addRow(self, label, widget):
    self.formLayout.addRow(label, widget)
    label = self.formLayout.labelForField(widget)
    label.alignment = self.formLayout.labelAlignment

#=============================================================================
#
# SettingsPanel
#
#=============================================================================
class SettingsPanel(ctk.ctkSettingsPanel):
  #---------------------------------------------------------------------------
  def __init__(self, *args, **kwargs):
    ctk.ctkSettingsPanel.__init__(self, *args, **kwargs)
    self.ui = _ui_SettingsPanel(self)

    self.registerProperty(
      userTemplatePathKey(), self.ui.genericPaths.ui.pathList,
      "directoryList", str(qt.SIGNAL("directoryListChanged()")),
      "Additional template paths", ctk.ctkSettingsPanel.OptionRequireRestart)

    for category in self.ui.paths.keys():
      self.registerProperty(
        userTemplatePathKey(category), self.ui.paths[category].ui.pathList,
        "directoryList", str(qt.SIGNAL("directoryListChanged()")),
        "Additional template paths for %s" % category,
        ctk.ctkSettingsPanel.OptionRequireRestart)

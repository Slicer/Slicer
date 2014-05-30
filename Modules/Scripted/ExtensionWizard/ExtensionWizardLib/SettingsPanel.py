from __main__ import qt, ctk

import SlicerWizard.TemplateManager

from .DirectoryListWidget import DirectoryListWidget

#=============================================================================
#
# _ui_SettingsPanel
#
#=============================================================================
class _ui_SettingsPanel(object):
  #---------------------------------------------------------------------------
  def __init__(self, parent):
    self.formLayout = qt.QFormLayout(parent)

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
      "ExtensionWizard/TemplatePaths", self.ui.genericPaths.ui.pathList,
      "directoryList", qt.SIGNAL("directoryListChanged()"),
      "Additional template paths", ctk.ctkSettingsPanel.OptionRequireRestart)

    for category in self.ui.paths.keys():
      self.registerProperty(
        "ExtensionWizard/TemplatePaths/%s" % category,
        self.ui.paths[category].ui.pathList,
        "directoryList", qt.SIGNAL("directoryListChanged()"),
        "Additional template paths for %s" % category,
        ctk.ctkSettingsPanel.OptionRequireRestart)

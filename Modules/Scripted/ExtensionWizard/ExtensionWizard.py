import os
import sys
import traceback

from __main__ import qt, ctk, slicer

import SlicerWizard.ExtensionProject
import SlicerWizard.TemplateManager
import SlicerWizard.Utilities

from ExtensionWizardLib import *

#=============================================================================
#
# ExtensionWizard
#
#=============================================================================
class ExtensionWizard:
  #---------------------------------------------------------------------------
  def __init__(self, parent):
    parent.title = "Extension Wizard"
    parent.categories = ["Developer Tools"]
    parent.dependencies = []
    parent.contributors = ["Matthew Woehlke (Kitware)"]
    parent.helpText = """
    This module provides tools to create and manage extensions from within Slicer.
    """
    parent.acknowledgementText = """
    This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community.
    See <a>http://www.slicer.org</a> for details."""
    self.parent = parent

#=============================================================================
#
# ExtensionWizardWidget
#
#=============================================================================
class ExtensionWizardWidget:
  #---------------------------------------------------------------------------
  def __init__(self, parent = None):
    if not parent:
      self.parent = qt.QWidget()
      self.parent.setLayout(qt.QVBoxLayout())
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parent.show()

    self.templateManager = None
    self.setupTemplates()

  #---------------------------------------------------------------------------
  def setup(self):
    # Instantiate and connect widgets ...

    #
    # Tools Area
    #
    self.toolsCollapsibleButton = ctk.ctkCollapsibleButton()
    self.toolsCollapsibleButton.text = "Extension Tools"
    self.layout.addWidget(self.toolsCollapsibleButton)

    # Layout within the dummy collapsible button
    self.createExtensionButton = qt.QPushButton("Create Extension")
    self.createExtensionButton.connect('clicked(bool)', self.createExtension)

    self.selectExtensionButton = qt.QPushButton("Select Extension")
    self.selectExtensionButton.connect('clicked(bool)', self.selectExtension)

    toolsFormLayout = qt.QVBoxLayout(self.toolsCollapsibleButton)
    toolsFormLayout.addWidget(self.createExtensionButton)
    toolsFormLayout.addWidget(self.selectExtensionButton)

    #
    # Editor Area
    #
    self.editorCollapsibleButton = ctk.ctkCollapsibleButton()
    self.editorCollapsibleButton.text = "Extension Editor"
    self.editorCollapsibleButton.enabled = False
    self.editorCollapsibleButton.collapsed = True
    self.layout.addWidget(self.editorCollapsibleButton)

    # Add vertical spacer
    self.layout.addStretch(1)

  #---------------------------------------------------------------------------
  def cleanup(self):
    pass

  #---------------------------------------------------------------------------
  def setupTemplates(self):
    self.templateManager = SlicerWizard.TemplateManager()

    s = qt.QSettings()
    s.beginGroup("ExtensionWizard/TemplatePaths")

    for k in s.allKeys():
      path = s.value(k)
      if "/" in k:
        c = k.split("/")[0]
        try:
          self.templateManager.addCategoryPath(c, path)
        except:
          mp = (c, path)
          qt.qWarning("failed to add template path %r for category %r" % mp)
          qt.qWarning(traceback.format_exc())

      else:
        try:
          self.templateManager.addPath(path)
        except:
          qt.qWarning("failed to add template path %r" % path)
          qt.qWarning(traceback.format_exc())

  #---------------------------------------------------------------------------
  def createExtension(self):
    dlg = CreateExtensionDialog(self.parent.window())
    dlg.setTemplates(self.templateManager.templates("extensions"))

    while dlg.exec_() == qt.QDialog.Accepted:
      try:
        path = self.templateManager.copyTemplate(
                dlg.destination(), "extensions", dlg.extensionType(),
                dlg.extensionName())

      except:
        md = qt.QMessageBox(self.parent.window())
        md.icon = qt.QMessageBox.Critical
        md.text = "An error occurred while trying to create the extension."
        md.detailedText = traceback.format_exc()
        md.standardButtons = qt.QMessageBox.Retry | qt.QMessageBox.Close
        if md.exec_() != qt.QMessageBox.Retry:
          return

        continue

      self.selectExtension(path)
      return

  #---------------------------------------------------------------------------
  def selectExtension(self, path=None):
    # TODO
    pass

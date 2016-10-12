import os
import string
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

#
# DMRIInstall
#

class DMRIInstall(ScriptedLoadableModule):
  """
  """

  helpText = """
Please use the Extension Manager to install the "SlicerDMRI" extension for diffusion-related tools including:

    Diffusion Tensor Estimation
    Tractography Display
    Tractography Seeding
    Fiber Tract Measurement
"""

  def __init__(self, parent):

    # Hide this module if SlicerDMRI is already installed
    model = slicer.app.extensionsManagerModel()
    if model.isExtensionInstalled("SlicerDMRI"):
      return

    ScriptedLoadableModule.__init__(self, parent)

    self.parent.categories = ["Diffusion"]
    self.parent.title = "Install Slicer Diffusion Tools"
    self.parent.dependencies = []
    self.parent.contributors = ["Isaiah Norton"]
    self.parent.helpText = DMRIInstall.helpText
    self.parent.helpText += self.getDefaultModuleDocumentationLink()
    self.parent.acknowledgementText = """
SlicerDMRI supported by NIH NCI ITCR U01CA199459 (Open Source Diffusion MRI Technology For Brain Cancer Research), and made possible by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community.
"""

class DMRIInstallWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    self.textBox = qt.QLabel()
    self.textBox.setWordWrap(1)
    self.textBox.setText(DMRIInstall.helpText)
    self.parent.layout().addWidget(self.textBox)

    #
    # Apply Button
    #
    self.applyButton = qt.QPushButton("Open Extension Manager")
    self.applyButton.toolTip = """Install the "SlicerDMRI" extension from the Diffusion category."""
    self.applyButton.enabled = True
    self.applyButton.connect('clicked()', self.onApply)
    self.parent.layout().addWidget(self.applyButton)

    self.parent.layout().addStretch(1)

  def onApply(self):
    slicer.app.openExtensionsManagerDialog()

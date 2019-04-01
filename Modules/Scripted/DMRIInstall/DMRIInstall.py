import os
import string
import textwrap
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

  helpText = textwrap.dedent(
  """
  The SlicerDMRI extension provides diffusion-related tools including:

  <ul>
    <li> Diffusion Tensor Estimation</li>
    <li>Tractography Display</li>
    <li>Tractography Seeding</li>
    <li>Fiber Tract Measurement</li>
  </ul>
  <br>
  <br>
  For more information, please visit:
  <br>
  <br>
  &nbsp;&nbsp; <a href="http://dmri.slicer.org">http://dmri.slicer.org</a>
  <br>
  <br>
  Questions are welcome on the Slicer forum:
  <br>
  <br>
  &nbsp;&nbsp; <a href="https://discourse.slicer.org">https://discourse.slicer.org</a><br><br>
  """)

  errorText = textwrap.dedent(
  """
  <h5 style="color:red">The SlicerDMRI extension is currently unavailable.</h5><br>
  Please try a manual installation via the Extension Manager,
  and contact the Slicer forum at:<br><br>

  &nbsp;&nbsp;<a href="https://discourse.slicer.org">https://discourse.slicer.org</a><br><br>

  With the following information:<br>
  Slicer version: {builddate}<br>
  Slicer revision: {revision}<br>
  Platform: {platform}
  """).format(builddate=slicer.app.applicationVersion,
              revision = slicer.app.repositoryRevision,
              platform = slicer.app.platform)

  def __init__(self, parent):

    # Hide this module if SlicerDMRI is already installed
    model = slicer.app.extensionsManagerModel()
    if model.isExtensionInstalled("SlicerDMRI"):
      parent.hidden = True

    ScriptedLoadableModule.__init__(self, parent)

    self.parent.categories = ["Diffusion"]
    self.parent.title = "Install Slicer Diffusion Tools (SlicerDMRI)"
    self.parent.dependencies = []
    self.parent.contributors = ["Isaiah Norton (BWH), Lauren O'Donnell (BWH)"]
    self.parent.helpText = DMRIInstall.helpText
    self.parent.helpText += self.getDefaultModuleDocumentationLink()
    self.parent.acknowledgementText = textwrap.dedent(
    """
    SlicerDMRI supported by NIH NCI ITCR U01CA199459 (Open Source Diffusion MRI
    Technology For Brain Cancer Research), and made possible by NA-MIC, NAC,
    BIRN, NCIGT, and the Slicer Community.
    """)

class DMRIInstallWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    self.textBox = ctk.ctkFittedTextBrowser()
    self.textBox.setOpenExternalLinks(True) # Open links in default browser
    self.textBox.setHtml(DMRIInstall.helpText)
    self.parent.layout().addWidget(self.textBox)

    #
    # Apply Button
    #
    self.applyButton = qt.QPushButton("Install SlicerDMRI")
    self.applyButton.toolTip = 'Installs the "SlicerDMRI" extension from the Diffusion category.'
    self.applyButton.icon = qt.QIcon(":/Icons/ExtensionDefaultIcon.png")
    self.applyButton.enabled = True
    self.applyButton.connect('clicked()', self.onApply)
    self.parent.layout().addWidget(self.applyButton)

    self.parent.layout().addStretch(1)


  def onError(self):
    self.applyButton.enabled = False
    self.textBox.setHtml(DMRIInstall.errorText)
    return


  def onApply(self):
    emm = slicer.app.extensionsManagerModel()

    if emm.isExtensionInstalled("SlicerDMRI"):
      self.textBox.setHtml("<h4>SlicerDMRI is already installed.<h4>")
      self.applyButton.enabled = False
      return

    md = emm.retrieveExtensionMetadataByName("SlicerDMRI")

    if not md or 'extension_id' not in md:
      return self.onError()

    if emm.downloadAndInstallExtension(md['extension_id']):
      slicer.app.confirmRestart("Restart to complete SlicerDMRI installation?")
    else:
      self.onError()

from __future__ import print_function
import os, copy
import qt
import vtk
import logging

import slicer
from slicer.util import VTKObservationMixin

from slicer.util import settingsValue, toBool
import DICOMLib


class DICOMSendDialog(qt.QDialog):
  """Implement the Qt dialog for doing a DICOM Send (storage SCU)
  """

  def __init__(self, files, parent="mainWindow"):
    super(DICOMSendDialog, self).__init__(slicer.util.mainWindow() if parent == "mainWindow" else parent)
    self.setWindowTitle('Send DICOM Study')
    self.setWindowModality(1)
    self.setLayout(qt.QVBoxLayout())
    self.files = files
    self.cancelRequested = False
    self.sendingIsInProgress = False
    self.setMinimumWidth(200)
    self.open()

  def open(self):
    self.studyLabel = qt.QLabel('Send %d items to destination' % len(self.files))
    self.layout().addWidget(self.studyLabel)

    # Send Parameters
    self.dicomFrame = qt.QFrame(self)
    self.dicomFormLayout = qt.QFormLayout()
    self.dicomFrame.setLayout(self.dicomFormLayout)

    self.settings = qt.QSettings()

    self.protocolSelectorCombobox = qt.QComboBox()
    self.protocolSelectorCombobox.addItems(["DIMSE","DICOMweb"])
    self.protocolSelectorCombobox.setCurrentText(self.settings.value('DICOM/Send/Protocol', 'DIMSE'))
    self.dicomFormLayout.addRow("Protocol: ", self.protocolSelectorCombobox)

    self.serverAddressLineEdit = qt.QLineEdit()
    self.serverAddressLineEdit.setToolTip("Address includes hostname and port number in standard URL format (hostname:port).")
    self.serverAddressLineEdit.text = self.settings.value('DICOM/Send/URL', '')
    self.dicomFormLayout.addRow("Destination Address: ", self.serverAddressLineEdit)

    self.layout().addWidget(self.dicomFrame)

    # button box
    self.bbox = qt.QDialogButtonBox(self)
    self.bbox.addButton(self.bbox.Ok)
    self.bbox.addButton(self.bbox.Cancel)
    self.bbox.accepted.connect(self.onOk)
    self.bbox.rejected.connect(self.onCancel)
    self.layout().addWidget(self.bbox)

    self.progressBar = qt.QProgressBar(self.parent().window())
    self.progressBar.hide()
    self.dicomFormLayout.addRow(self.progressBar)

    qt.QDialog.open(self)

  def onOk(self):
    self.sendingIsInProgress = True
    address = self.serverAddressLineEdit.text
    protocol = self.protocolSelectorCombobox.currentText
    self.settings.setValue('DICOM/Send/URL', address)
    self.settings.setValue('DICOM/Send/Protocol', protocol)
    self.progressBar.value = 0
    self.progressBar.maximum = len(self.files)+1
    self.progressBar.show()
    self.cancelRequested = False
    okButton = self.bbox.button(self.bbox.Ok)

    try:
      #qt.QApplication.setOverrideCursor(qt.Qt.WaitCursor)
      okButton.enabled = False
      DICOMLib.DICOMSender(self.files, address, protocol, progressCallback=self.onProgress)
      logging.debug("DICOM sending of %s files succeeded" % len(self.files))
      self.close()
    except Exception as result:
      import traceback
      slicer.util.errorDisplay("DICOM sending failed: %s" % str(result), parent=self.parent().window(), detailedText=traceback.format_exc())

    #qt.QApplication.restoreOverrideCursor()
    okButton.enabled = True
    self.sendingIsInProgress = False

  def onCancel(self):
    if self.sendingIsInProgress:
      self.cancelRequested = True
    else:
      self.close()

  def onProgress(self, message):
    self.progressBar.value += 1
    # message can be long, do not display it, but still log it (might be useful for troubleshooting)
    logging.debug("DICOM send: " + message)
    slicer.app.processEvents()
    return not self.cancelRequested

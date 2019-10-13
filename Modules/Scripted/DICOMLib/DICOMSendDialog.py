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
    self.settings = qt.QSettings()
    self.sendAddress = self.settings.value('DICOM.sendAddress')
    self.sendPort = self.settings.value('DICOM.sendPort')

    self.open()

  def open(self):
    self.studyLabel = qt.QLabel('Send %d items to destination' % len(self.files))
    self.layout().addWidget(self.studyLabel)

    # Send Parameters
    self.dicomFrame = qt.QFrame(self)
    self.dicomFormLayout = qt.QFormLayout()
    self.dicomFrame.setLayout(self.dicomFormLayout)
    self.dicomEntries = {}
    self.dicomParameters = {
      "Destination Address": self.sendAddress,
      "Destination Port": self.sendPort
    }
    for label in self.dicomParameters.keys():
      self.dicomEntries[label] = qt.QLineEdit()
      self.dicomEntries[label].text = self.dicomParameters[label]
      self.dicomFormLayout.addRow(label + ": ", self.dicomEntries[label])
    self.layout().addWidget(self.dicomFrame)

    # button box
    bbox = qt.QDialogButtonBox(self)
    bbox.addButton(bbox.Ok)
    bbox.addButton(bbox.Cancel)
    bbox.accepted.connect(self.onOk)
    bbox.rejected.connect(self.onCancel)
    self.layout().addWidget(bbox)

    qt.QDialog.open(self)

  def onOk(self):
    address = self.dicomEntries['Destination Address'].text
    port = self.dicomEntries['Destination Port'].text
    self.settings.setValue('DICOM.sendAddress', address)
    self.settings.setValue('DICOM.sendPort', port)
    self.progress = slicer.util.createProgressDialog(value=0, maximum=len(self.files))
    self.progressValue = 0

    try:
      DICOMLib.DICOMSender(self.files, address, port, progressCallback=self.onProgress)
    except Exception as result:
      slicer.util.warningDisplay('Could not send data: %s' % result, windowTitle='DICOM Send', parent=self)
    self.progress.close()
    self.progress = None
    self.progressValue = None
    self.close()

  def onCancel(self):
    self.close()

  def onProgress(self, message):
    self.progress.show()
    self.progress.activateWindow()
    self.centerProgress()
    self.progressValue += 1
    self.progress.setValue(self.progressValue)
    self.progress.setLabelText(message)
    slicer.app.processEvents()

  def centerProgress(self):
    mainWindow = slicer.util.mainWindow()
    screenMainPos = mainWindow.pos
    x = screenMainPos.x() + int((mainWindow.width - self.progress.width)/2)
    y = screenMainPos.y() + int((mainWindow.height - self.progress.height)/2)
    self.progress.move(x,y)

import os
import glob
from __main__ import qt
from __main__ import vtk
from __main__ import ctk
from __main__ import slicer

import DICOMLib

#########################################################
#
# 
comment = """

DICOMWidgets are helper classes to build an interface
to manage DICOM data in the context of slicer.

This code is slicer-specific and relies on the slicer python module
for elements like slicer.dicomDatabase and slicer.mrmlScene

# TODO : 
"""
#
#########################################################

class DICOMDetailsPopup(object):
  """Implement the Qt window showing details and possible
  operations to perform on the selected dicom tree item.
  This is a helper used in the DICOMWidget class.
  """

  def __init__(self,dicomApp):
    self.dicomApp = dicomApp
    self.create()

  def create(self):
    """
    main window is a frame with widgets from the app
    widget repacked into it along with slicer-specific 
    extra widgets
    """

    # find internals of widget for reference and repacking
    self.tree = slicer.util.findChildren(self.dicomApp, 'TreeView')[0]
    self.userFrame = slicer.util.findChildren(self.dicomApp, 'UserFrame')[0]
    self.thumbs = slicer.util.findChildren(self.dicomApp, 'ThumbnailsWidget')[0]
    self.widthSlider = slicer.util.findChildren(self.dicomApp, 'ThumbnailWidthSlider')[0]
    self.preview = slicer.util.findChildren(self.dicomApp, 'PreviewFrame')[0]

    self.window = ctk.ctkPopupWidget(self.dicomApp)
    self.window.orientation = 1
    self.window.horizontalDirection = 0
    self.window.alignment = 0x82
    self.window.setWindowTitle('DICOM Details')

    self.layout = qt.QGridLayout()
    self.window.setLayout(self.layout)

    #
    # preview related column
    #
    self.previewLayout = qt.QVBoxLayout()
    self.layout.addLayout(self.previewLayout,0,0)

    self.previewLayout.addWidget(self.thumbs)
    self.previewLayout.addWidget(self.widthSlider)
    self.previewLayout.addWidget(self.preview)

    #
    # action related column (interacting with slicer)
    #
    self.actionLayout = qt.QVBoxLayout()
    self.layout.addLayout(self.actionLayout,0,1)
    self.actionLayout.addWidget(self.userFrame)
    
    self.loadButton = qt.QPushButton('Load Selection to Slicer')
    self.loadButton.enabled = False 
    self.actionLayout.addWidget(self.loadButton)
    self.loadButton.connect('clicked()', self.loadCheckedVolumes)

    self.volumeTable = DICOMVolumeTable(self.userFrame)
    self.actionLayout.addWidget(self.volumeTable.widget)

    self.actionLayout.addStretch(1)

    #
    # header related column (more details about the selected file)
    #
    self.headerLayout = qt.QVBoxLayout()
    self.layout.addLayout(self.headerLayout,0,2)
    self.header = DICOMHeaderWidget(self.window)
    self.headerLayout.addWidget(self.header.widget)
    self.headerLayout.addStretch(1)

  def open(self):
    self.window.show()

  def offerVolumes(self,uid,role):
    """Get all the volume options at the currently selected level
    and present them in the volume table"""
    fileLists = []
    volumes = []
    if role == "Series":
      fileLists.append(slicer.dicomDatabase.filesForSeries(uid))
    if role == "Study":
      series = slicer.dicomDatabase.seriesForStudy(uid)
      for serie in series:
        fileLists.append(slicer.dicomDatabase.filesForSeries(serie))
    if role == "Patient":
      studies = slicer.dicomDatabase.studiesForPatient(uid)
      for study in studies:
        series = slicer.dicomDatabase.seriesForStudy(study)
        for serie in series:
          fileLists.append(slicer.dicomDatabase.filesForSeries(serie))

    for files in fileLists:
      volumeFiles = DICOMLib.DICOMVolumeOrganizer().organizeFiles(files)
      if volumeFiles:
        for volume in volumeFiles:
          volumes.append(volume)

    volumes = sorted(volumes, key=lambda x: not x.selected)
    self.volumeTable.setVolumes(volumes)
    self.loadButton.enabled = volumes != []

    # TODO: update based on selection
    self.header.setHeader(volumes[0].files[0])

  def loadCheckedVolumes(self):
    volumes = self.volumeTable.checkedVolumes()

    self.progress = qt.QProgressDialog(slicer.util.mainWindow())
    self.progress.minimumDuration = 0
    self.progress.show()
    self.progress.setValue(0)
    self.progress.setMaximum(len(volumes))
    step = 0
    for volume in volumes:
      self.progress.labelText = '\nLoading %s' % volume.name
      self.progress.setValue(step)
      slicer.app.processEvents()
      loader = DICOMLib.DICOMLoader(volume.files,volume.name)
      if not loader.volumeNode:
        qt.QMessageBox.warning(slicer.util.mainWindow(), 'Load', 'Could not load volume for: %s' % name)
        print('Tried to load volume as %s using: ' % name, files)
      step += 1
      self.progress.setValue(step)
    self.progress.close()
    self.progress = None


class DICOMVolumeTable(object):
  """Implement the Qt code for a table of
  selectable slicer volumes to be made from 
  the given dicom files
  """

  def __init__(self,parent):
    self.widget = qt.QTableWidget(parent)
    self.sizePolicy = qt.QSizePolicy(qt.QSizePolicy.Ignored,qt.QSizePolicy.Ignored)
    self.sizePolicy.setVerticalStretch(1)
    self.widget.setSizePolicy( self.sizePolicy )
    self.widget.setMinimumHeight(500)
    self.widget.setMinimumWidth(350)
    self.items = []
    self.setVolumes([])

  def setVolumes(self,volumes):
    """Load the table widget with a list
    of volume options (of class DICOMVolume)
    """
    self.widget.clearContents()
    self.widget.setColumnCount(2)
    self.widget.setHorizontalHeaderLabels(['Volumes to Load','Warnings'])
    self.widget.setColumnWidth(0,150)
    self.widget.setColumnWidth(1,200)
    self.widget.setRowCount(len(volumes))
    row = 0
    for volume in volumes:
      # name and check state
      item = qt.QTableWidgetItem(volume.name)
      item.setCheckState(volume.selected * 2)
      self.items.append(item)
      self.widget.setItem(row,0,item)
      item.setToolTip(volume.name)
      # warning
      if volume.warning:
        warnItem = qt.QTableWidgetItem(volume.warning)
        self.items.append(warnItem)
        self.widget.setItem(row,1,warnItem)
        item.setToolTip(item.toolTip() + "\n" + volume.warning)
        warnItem.setToolTip(item.toolTip())
      row += 1
    self.widget.setVerticalHeaderLabels(row * [""])
    self.volumes = volumes

  def checkedVolumes(self):
    returnVolumes = []
    for row in xrange(self.widget.rowCount):
      item = self.widget.item(row,0)
      if item.checkState():
        returnVolumes.append(self.volumes[row])
    return(returnVolumes)

class DICOMHeaderWidget(object):
  """Implement the Qt code for a table of
  DICOM header values
  """

  def __init__(self,parent):
    self.widget = qt.QTableWidget(parent)
    self.sizePolicy = qt.QSizePolicy(7,7)
    self.sizePolicy.setVerticalStretch(1)
    self.widget.setSizePolicy( self.sizePolicy )
    self.widget.setMinimumHeight(500)
    self.widget.setMinimumWidth(300)
    self.items = []
    self.setHeader(None)

  def setHeader(self,file):
    """Load the table widget with header values for the file
    """
    self.widget.clearContents()
    self.widget.setColumnCount(2)
    self.widget.setHorizontalHeaderLabels(['Tag','Value'])
    self.widget.setColumnWidth(0,100)
    self.widget.setColumnWidth(1,200)

    if not file:
      return

    slicer.dicomDatabase.loadFileHeader(file)
    keys = slicer.dicomDatabase.headerKeys()
    self.widget.setRowCount(len(keys))
    row = 0
    for key in keys:
      item = qt.QTableWidgetItem(key)
      self.widget.setItem(row,0,item)
      self.items.append(item)
      dump = slicer.dicomDatabase.headerValue(key)
      try:
        value = dump[dump.index('[')+1:dump.index(']')]
      except ValueError:
        value = "Unknown"
      item = qt.QTableWidgetItem(value)
      self.widget.setItem(row,1,item)
      self.items.append(item)
      row += 1


class DICOMExportDialog(object):
  """Implement the Qt dialog for selecting slicer data to be exported
  to be part of a DICOM study (e.g. a slicer volume as a new dicom series).
  """

  def __init__(self,studyUID,onExportFinished=None):
    self.studyUID = studyUID
    self.onExportFinished = onExportFinished

  def open(self):

    # main dialog
    self.dialog = qt.QDialog(slicer.util.mainWindow())
    self.dialog.setWindowTitle('Export to DICOM Study')
    self.dialog.setWindowModality(1)
    layout = qt.QVBoxLayout()
    self.dialog.setLayout(layout)

    self.studyLabel = qt.QLabel('Attach Data to Study: %s' % self.studyUID)
    layout.addWidget(self.studyLabel)

    # scene or volume option
    self.selectFrame = qt.QFrame(self.dialog)
    layout.addWidget(self.selectFrame)
    self.selectLayout = qt.QGridLayout()
    self.selectFrame.setLayout(self.selectLayout)
    self.exportScene = qt.QRadioButton("Export Entire Scene", self.selectFrame)
    #TODO re-enable export scene when slicer data bundle reading works
    self.exportScene.enabled = False
    self.exportScene.setToolTip( "Create a Slicer Data Bundle in a DICOM Private Creator\n(Only compatible with Slicer)" )
    self.exportVolume = qt.QRadioButton("Export Selected Volume", self.selectFrame)
    self.exportVolume.setToolTip( "Create a compatible DICOM series of slice images" )
    self.exportVolume.checked = True
    self.selectLayout.addWidget(self.exportScene, 0, 0)
    self.selectLayout.addWidget(self.exportVolume, 1, 0)
    self.exportScene.connect('toggled(bool)', self.onExportRadio)
    self.exportVolume.connect('toggled(bool)', self.onExportRadio)

    # select volume
    self.volumeSelector = slicer.qMRMLNodeComboBox(self.dialog)
    self.volumeSelector.nodeTypes = ( "vtkMRMLScalarVolumeNode", "" )
    self.volumeSelector.selectNodeUponCreation = False
    self.volumeSelector.addEnabled = False
    self.volumeSelector.noneEnabled = False
    self.volumeSelector.removeEnabled = False
    self.volumeSelector.showHidden = False
    self.volumeSelector.showChildNodeTypes = False
    self.volumeSelector.setMRMLScene( slicer.mrmlScene )
    self.volumeSelector.setToolTip( "Pick the label map to edit" )
    self.selectLayout.addWidget( self.volumeSelector, 1, 1 )

    # DICOM Parameters
    self.dicomFrame = qt.QFrame(self.dialog)
    self.dicomFormLayout = qt.QFormLayout()
    self.dicomFrame.setLayout(self.dicomFormLayout)
    self.dicomEntries = {}
    exporter = DICOMLib.DICOMExporter(self.studyUID)
    self.dicomParameters = exporter.parametersFromStudy()
    self.dicomParameters['Series Description'] = '3D Slicer Export'
    for label in self.dicomParameters.keys():
      self.dicomEntries[label] = qt.QLineEdit()
      self.dicomEntries[label].text = self.dicomParameters[label]
      self.dicomFormLayout.addRow(label+": ", self.dicomEntries[label])
    layout.addWidget(self.dicomFrame)

    # button box
    bbox = qt.QDialogButtonBox(self.dialog)
    bbox.addButton(bbox.Ok)
    bbox.addButton(bbox.Cancel)
    bbox.connect('accepted()', self.onOk)
    bbox.connect('rejected()', self.onCancel)
    layout.addWidget(bbox)

    self.dialog.open()

  def onExportRadio(self,toggled):
    self.volumeSelector.enabled = self.exportVolume.checked

  def onOk(self):
    """Run the export process for either the scene or the selected volume"""
    if self.exportScene.checked:
      volumeNode = None
    else:
      volumeNode = self.volumeSelector.currentNode()
    if volumeNode or self.exportScene.checked:
      parameters = {}
      for label in self.dicomParameters.keys():
        parameters[label] = self.dicomEntries[label].text
      try:
        exporter = DICOMLib.DICOMExporter(self.studyUID,volumeNode,parameters)
        exporter.export()
      except Exception as result:
        import traceback
        qt.QMessageBox.warning(self.dialog, 'DICOM Export', 'Could not export data: %s\n\n%s' % (result, traceback.format_exception(*sys.exc_info())))
    if self.onExportFinished:
      self.onExportFinished()
    self.dialog.close()

  def onCancel(self):
    if self.onExportFinished:
      self.onExportFinished()
    self.dialog.close()

class DICOMSendDialog(object):
  """Implement the Qt dialog for doing a DICOM Send (storage SCU)
  """

  def __init__(self,files):
    self.files = files
    settings = qt.QSettings()
    self.sendAddress = settings.value('DICOM.sendAddress')
    self.sendPort = settings.value('DICOM.sendPort')
    self.open

  def open(self):
    # main dialog
    self.dialog = qt.QDialog(slicer.util.mainWindow())
    self.dialog.setWindowTitle('Send DICOM Study')
    self.dialog.setWindowModality(1)
    layout = qt.QVBoxLayout()
    self.dialog.setLayout(layout)

    self.studyLabel = qt.QLabel('Send %d items to destination' % len(self.files))
    layout.addWidget(self.studyLabel)

    # Send Parameters
    self.dicomFrame = qt.QFrame(self.dialog)
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
      self.dicomFormLayout.addRow(label+": ", self.dicomEntries[label])
    layout.addWidget(self.dicomFrame)

    # button box
    bbox = qt.QDialogButtonBox(self.dialog)
    bbox.addButton(bbox.Ok)
    bbox.addButton(bbox.Cancel)
    bbox.connect('accepted()', self.onOk)
    bbox.connect('rejected()', self.onCancel)
    layout.addWidget(bbox)

    self.dialog.open()

  def onOk(self):
    address = self.dicomEntries['Destination Address'].text
    port = self.dicomEntries['Destination Port'].text
    settings = qt.QSettings()
    settings.setValue('DICOM.sendAddress', address)
    settings.setValue('DICOM.sendPort', port)
    self.progress = qt.QProgressDialog(slicer.util.mainWindow())
    self.progress.minimumDuration = 0
    self.progress.setMaximum(len(self.files))
    self.progressValue = 0
    try:
      DICOMLib.DICOMSender(self.files, address, port, progressCallback = self.onProgress)
    except Exception as result:
      qt.QMessageBox.warning(self.dialog, 'DICOM Send', 'Could not send data: %s' % result)
    self.progress.close()
    self.dialog.close()

  def onProgress(self,message):
    self.progress.show()
    self.progressValue += 1
    self.progress.setValue(self.progressValue)
    self.progress.setLabelText(message)

  def onCancel(self):
    self.dialog.close()


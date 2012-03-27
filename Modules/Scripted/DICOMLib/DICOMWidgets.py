import os, glob, sys
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

  def __init__(self,dicomApp,setBrowserPersistence=None):
    self.dicomApp = dicomApp
    self.setBrowserPersistence = setBrowserPersistence
    self.create()
    self.popupPositioned = False
    self.pluginInstances = {}

  def create(self,widgetType='window',showHeader=False,showPreview=False):
    """
    main window is a frame with widgets from the app
    widget repacked into it along with slicer-specific 
    extra widgets
    """

    # find internals of widget for reference and repacking
    self.toolBar = slicer.util.findChildren(self.dicomApp, 'ToolBar')[0]
    self.databaseNameLabel = slicer.util.findChildren(self.dicomApp, 'DatabaseNameLabel')[0]
    self.databaseDirectoryButton = slicer.util.findChildren(self.dicomApp, 'DirectoryButton')[0]
    self.tree = slicer.util.findChildren(self.dicomApp, 'TreeView')[0]
    self.userFrame = slicer.util.findChildren(self.dicomApp, 'UserFrame')[0]
    self.thumbs = slicer.util.findChildren(self.dicomApp, 'ThumbnailsWidget')[0]
    self.widthSlider = slicer.util.findChildren(self.dicomApp, 'ThumbnailWidthSlider')[0]
    self.preview = slicer.util.findChildren(self.dicomApp, 'PreviewFrame')[0]

    self.widgetType = widgetType
    if widgetType == 'dialog':
      self.window = qt.QDialog(self.dicomApp)
    elif widgetType == 'window':
      self.window = qt.QWidget()
    elif widgetType == 'popup':
      self.window = ctk.ctkPopupWidget(self.dicomApp)
      self.window.orientation = 1
      self.window.horizontalDirection = 0
      self.window.alignment = 0x82
    elif widgetType == 'dock':
      self.dock = qt.QDockWidget(slicer.util.mainWindow())
      self.dock.setFeatures( qt.QDockWidget.DockWidgetFloatable |
                                qt.QDockWidget.DockWidgetMovable | 
                                qt.QDockWidget.DockWidgetClosable )
      slicer.util.mainWindow().addDockWidget(0x15, self.dock)
      self.window = qt.QFrame()
      self.dock.setWidget(self.window)
    else:
      raise "Unknown widget type - should be dialog, window, dock or popup"

    self.window.setWindowTitle('DICOM Details')

    self.layout = qt.QGridLayout()
    self.window.setLayout(self.layout)

    # overall layout - tree on top, preview and selection below
    toolRow = 0
    treeRow = 1
    selectionRow = 2

    # tool row at top, with commands and database
    self.toolLayout = qt.QHBoxLayout()
    self.layout.addLayout(self.toolLayout,toolRow,0,1,2)
    self.toolLayout.addWidget(self.toolBar)
    self.toolLayout.addWidget(self.databaseNameLabel)
    self.toolLayout.addWidget(self.databaseDirectoryButton)

    # tree goes next, spread across 1 row, 2 columns
    self.layout.addWidget(self.tree,treeRow,0,1,2)

    #
    # preview related column
    #
    self.previewLayout = qt.QVBoxLayout()
    self.layout.addLayout(self.previewLayout,selectionRow,0)

    self.previewLayout.addWidget(self.thumbs)
    self.previewLayout.addWidget(self.widthSlider)
    if showPreview:
      self.previewLayout.addWidget(self.preview)
    else:
      self.preview.hide()

    #
    # action related column (interacting with slicer)
    #
    self.actionLayout = qt.QVBoxLayout()
    self.layout.addLayout(self.actionLayout,selectionRow,1)
    self.actionLayout.addWidget(self.userFrame)
    
    tableWidth = 350 if showHeader else 700
    self.loadableTable = DICOMLoadableTable(self.userFrame,width=tableWidth)
    self.actionLayout.addWidget(self.loadableTable.widget)

    #
    # button row for action column
    #
    self.actionButtonLayout = qt.QHBoxLayout()
    self.actionLayout.addLayout(self.actionButtonLayout)

    self.loadButton = qt.QPushButton('Load Selection to Slicer')
    self.loadButton.enabled = False 
    self.actionButtonLayout.addWidget(self.loadButton)
    self.loadButton.connect('clicked()', self.loadCheckedLoadables)

    self.closeButton = qt.QPushButton('Close')
    self.actionButtonLayout.addWidget(self.closeButton)
    self.closeButton.connect('clicked()', self.close)

    if self.setBrowserPersistence:
      self.browserPersistentButton = qt.QCheckBox('Make DICOM Browser Persistent')
      self.browserPersistentButton.toolTip = 'When enabled, DICOM Broswer remains open and usable after leaving DICOM module'
      self.actionLayout.addWidget(self.browserPersistentButton)
      self.browserPersistentButton.connect('stateChanged(int)', self.setBrowserPersistence)


    #
    # header related column (more details about the selected file)
    #
    if showHeader:
      self.headerLayout = qt.QVBoxLayout()
      self.layout.addLayout(self.headerLayout,selectionRow,2)
      self.header = DICOMHeaderWidget(self.window)
      self.headerLayout.addWidget(self.header.widget)

  def open(self):
    self.window.show()
    if not self.popupPositioned:
      if False:
        appWidth = self.dicomApp.geometry.width()
        screenAppPos = self.dicomApp.mapToGlobal(self.dicomApp.pos)
        x = screenAppPos.x() + appWidth
        y = screenAppPos.y()
      else:
        mainWindow = slicer.util.mainWindow()
        #screenMainPos = mainWindow.mapToGlobal(mainWindow.pos)
        screenMainPos = mainWindow.pos
        x = screenMainPos.x() + 100
        y = screenMainPos.y() + 100
      self.window.move(qt.QPoint(x,y))
      self.popupPositioned = True
    self.window.raise_()

  def close(self):
    self.window.hide()

  def setModality(self,modality):
    if self.widgetType == 'dialog':
      self.window.setModal(modality)

  def offerLoadables(self,uid,role):
    """Get all the loadable options at the currently selected level
    and present them in the loadable table"""
    fileLists = []
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
          fileList = slicer.dicomDatabase.filesForSeries(serie)
          fileLists.append(fileList)


    allFileCount = missingFileCount = 0
    for fileList in fileLists:
        for filePath in fileList:
          allFileCount += 1
          if not os.path.exists(filePath):
            missingFileCount += 1

    if missingFileCount > 0:
      qt.QMessageBox.warning(self.window,
          "DICOM", "Warning: %d of %d files listed in the database for this %s cannot be found on disk." % (missingFileCount, allFileCount, role))

    if missingFileCount == allFileCount:
      return

    self.progress = qt.QProgressDialog(self.window)
    self.progress.modal = True
    self.progress.minimumDuration = 0
    self.progress.show()
    self.progress.setValue(0)
    self.progress.setMaximum(len(slicer.modules.dicomPlugins))
    step = 0

    loadEnabled = False
    self.loadablesByPlugin = {}
    for pluginClass in slicer.modules.dicomPlugins:
      if not self.pluginInstances.has_key(pluginClass):
        self.pluginInstances[pluginClass] = slicer.modules.dicomPlugins[pluginClass]()
      plugin = self.pluginInstances[pluginClass]
      if self.progress.wasCanceled:
        break
      self.progress.labelText = '\nChecking %s' % pluginClass
      self.progress.setValue(step)
      slicer.app.processEvents()
      self.loadablesByPlugin[plugin] = plugin.examine(fileLists)
      loadEnabled = loadEnabled or self.loadablesByPlugin[plugin] != []
      step +=1
    self.loadButton.enabled = loadEnabled
    self.loadableTable.setLoadables(self.loadablesByPlugin)
    self.progress.close()
    self.progress = None

  def loadCheckedLoadables(self):
    """Invoke the load method on each plugin for the DICOMLoadable
    instances that are selected"""
    self.loadableTable.updateCheckstate()
    loadableCount = 0
    for plugin in self.loadablesByPlugin:
      for loadable in self.loadablesByPlugin[plugin]:
        loadableCount += 1
    self.progress = qt.QProgressDialog(self.window)
    self.progress.minimumDuration = 0
    self.progress.show()
    self.progress.setValue(0)
    self.progress.setMaximum(loadableCount)
    step = 0
    for plugin in self.loadablesByPlugin:
      for loadable in self.loadablesByPlugin[plugin]:
        if self.progress.wasCanceled:
          break
        self.progress.labelText = '\nLoading %s' % loadable.name
        self.progress.setValue(step)
        slicer.app.processEvents()
        if loadable.selected:
          if not plugin.load(loadable):
            qt.QMessageBox.warning(slicer.util.mainWindow(), 
                'Load', 'Could not load: %s as a %s' % (loadable.name,plugin.loadType))
        step += 1
        self.progress.setValue(step)
    self.progress.close()
    self.progress = None
    self.close()

class DICOMLoadableTable(object):
  """Implement the Qt code for a table of
  selectable slicer data to be made from 
  the given dicom files
  """

  def __init__(self,parent, width=350,height=300):
    self.widget = qt.QTableWidget(parent)
    self.widget.setMinimumHeight(height)
    self.widget.setMinimumWidth(width)
    self.width = width
    self.height = height
    self.items = []
    self.loadables = {}
    self.setLoadables([])

  def setLoadables(self,loadablesByPlugin):
    """Load the table widget with a list
    of volume options (of class DICOMVolume)
    """
    loadableCount = 0
    for plugin in loadablesByPlugin:
      for loadable in loadablesByPlugin[plugin]:
        loadableCount += 1
    self.widget.clearContents()
    self.widget.setColumnCount(2)
    self.widget.setHorizontalHeaderLabels(['DICOM Data','Warnings'])
    self.widget.setColumnWidth(0,int(self.width * 0.4))
    self.widget.setColumnWidth(1,int(self.width * 0.6))
    self.widget.setRowCount(loadableCount)
    row = 0
    for plugin in loadablesByPlugin:
      for selectState in (True,False):
        for loadable in loadablesByPlugin[plugin]:
          if loadable.selected == selectState:
            # name and check state
            self.loadables[row] = loadable
            item = qt.QTableWidgetItem(loadable.name)
            item.setCheckState(loadable.selected * 2)
            self.items.append(item)
            self.widget.setItem(row,0,item)
            item.setToolTip(loadable.tooltip)
            # warning
            if loadable.warning:
              warnItem = qt.QTableWidgetItem(loadable.warning)
              self.items.append(warnItem)
              self.widget.setItem(row,1,warnItem)
              item.setToolTip(item.toolTip() + "\n" + loadable.warning)
              warnItem.setToolTip(item.toolTip())
            row += 1
    self.widget.setVerticalHeaderLabels(row * [""])

  def updateCheckstate(self):
    for row in xrange(self.widget.rowCount):
      item = self.widget.item(row,0)
      self.loadables[row].selected = item.checkState()

class DICOMHeaderWidget(object):
  """Implement the Qt code for a table of
  DICOM header values
  """
  # TODO: move this to ctk and use data dictionary for 
  # tag names

  def __init__(self,parent):
    self.widget = qt.QTableWidget(parent,width=350,height=300)
    self.widget.setMinimumHeight(height)
    self.widget.setMinimumWidth(width)
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

class DICOMStudyBrowser(object):
  """Create a dialog for looking at studies and series

    TODO: this is an experiment to implement some of the ideas from:
     https://www.assembla.com/spaces/sparkit/wiki/20120125_Slicer_DICOM_browser_meeting 
    Still a work in progress
 """
  def __init__(self):
    settings = qt.QSettings()
    directory = settings.value('DatabaseDirectory')
    self.db = qt.QSqlDatabase.addDatabase("QSQLITE")
    self.db.setDatabaseName('%s/ctkDICOM.sql' % directory)
    self.db.open()

    self.dialog = qt.QDialog()
    self.dialog.setWindowTitle('Study Browser')
    self.dialog.setLayout(qt.QVBoxLayout())
    self.studyTable = DICOMStudyTable(self.dialog,self.db)
    self.dialog.layout().addWidget(self.studyTable.view)
    self.seriesTable = DICOMSeriesTable(self.dialog,self.db)
    self.dialog.layout().addWidget(self.seriesTable.view)

    self.studyTable.view.connect('clicked(QModelIndex)',self.onStudyClicked)

    self.dialog.show()

  def onStudyClicked(self,index):
    uid = index.sibling(index.row(),6).data()
    self.seriesTable.setStudyInstanceUID(uid)


class DICOMStudyTable(object):
  """Implement the Qt table for a list of studies by patient
  """

  def __init__(self,parent,db):
    self.view = qt.QTableView(parent)
    self.model = qt.QSqlQueryModel()
    self.statement = """ SELECT 
                            Patients.PatientsName, Patients.PatientID, Patients.PatientsBirthDate,
                            Studies.StudyDate, Studies.StudyDescription, Studies.ModalitiesInStudy,
                            Studies.StudyInstanceUID
                         FROM 
                            Patients,Studies 
                          WHERE 
                            Patients.UID=Studies.PatientsUID
                         ORDER BY 
                            Patients.PatientsName
                         ; """
    self.query = qt.QSqlQuery(db)
    self.query.prepare(self.statement)
    self.query.exec_()
    self.model.setQuery(self.query)

    self.view.setModel(self.model)
    self.view.sortingEnabled = False
    self.view.setSelectionBehavior(self.view.SelectRows)
    self.view.setSelectionMode(self.view.SingleSelection)
    self.view.setColumnWidth(0, 250)
    self.view.setColumnWidth(1, 100)
    self.view.setColumnWidth(2, 150)
    self.view.setColumnWidth(3, 100)
    self.view.setColumnWidth(4, 180)
    self.view.setColumnWidth(5, 180)
    self.view.setColumnWidth(6, 180)
    self.view.verticalHeader().visible = False


class DICOMSeriesTable(object):
  """Implement the Qt table for a list of series for a given study
  """

  def __init__(self,parent,db):
    self.view = qt.QTableView(parent)
    self.model = qt.QSqlQueryModel()

    self.statementFormat = """SELECT
                            Series.SeriesNumber, Series.SeriesDescription, 
                            Series.SeriesDate, Series.SeriesTime, Series.SeriesInstanceUID
                         FROM 
                            Series
                          WHERE 
                            Series.StudyInstanceUID='{StudyInstanceUID}'
                         ORDER BY 
                            Series.SeriesNumber
                         ; """
    self.query = qt.QSqlQuery(db)
    self.query.prepare(self.statementFormat.format(StudyInstanceUID='Nothing'))
    self.query.exec_()
    self.model.setQuery(self.query)

    self.view.setModel(self.model)
    self.view.sortingEnabled = False
    self.view.setSelectionBehavior(self.view.SelectRows)
    self.view.setSelectionMode(self.view.SingleSelection)
    self.view.setColumnWidth(0, 250)
    self.view.setColumnWidth(1, 100)
    self.view.setColumnWidth(2, 150)
    self.view.setColumnWidth(3, 100)
    self.view.setColumnWidth(4, 180)
    self.view.setColumnWidth(5, 180)
    self.view.setColumnWidth(6, 180)
    self.view.verticalHeader().visible = False

  def setStudyInstanceUID(self,uid):
    statement = self.statementFormat.format(StudyInstanceUID=uid)
    self.query.prepare(statement)
    self.query.exec_()

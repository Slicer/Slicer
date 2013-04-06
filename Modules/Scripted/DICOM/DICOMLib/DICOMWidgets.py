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
    self.popupGeometry = qt.QRect()
    settings = qt.QSettings()
    if settings.contains('DICOM/detailsPopup.geometry'):
      self.popupGeometry = settings.value('DICOM/detailsPopup.geometry')
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

    self.uncheckAllButton = qt.QPushButton('Uncheck All')
    self.actionButtonLayout.addWidget(self.uncheckAllButton)
    self.uncheckAllButton.connect('clicked()', self.uncheckAllLoadables)

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
    if not self.window.isVisible():
      self.window.show()
      if self.popupGeometry.isValid():
        self.window.setGeometry(self.popupGeometry)
        self.popupPositioned = True
    if not self.popupPositioned:
      mainWindow = slicer.util.mainWindow()
      screenMainPos = mainWindow.pos
      x = screenMainPos.x() + 100
      y = screenMainPos.y() + 100
      self.window.move(qt.QPoint(x,y))
      self.popupPositioned = True
    self.window.raise_()

  def close(self):
    self.onPopupGeometryChanged()
    self.window.hide()

  def onPopupGeometryChanged(self):
    settings = qt.QSettings()
    self.popupGeometry = self.window.geometry
    settings.setValue('DICOM/detailsPopup.geometry', self.window.geometry)

  def setModality(self,modality):
    if self.widgetType == 'dialog':
      self.window.setModal(modality)

  def organizeLoadables(self):
    """Review the selected state and confidence of the loadables
    across plugins so that the options the user is most likely
    to want are listed at the top of the table and are selected
    by default.  Only offer one pre-selected loadable per series
    unless both plugins mark it as selected and they have equal
    confidence."""

    # first, get all loadables corresponding to a series
    seriesUIDTag = "0020,000E"
    loadablesBySeries = {}
    for plugin in self.loadablesByPlugin:
      for loadable in self.loadablesByPlugin[plugin]:
        seriesUID = slicer.dicomDatabase.fileValue(loadable.files[0],seriesUIDTag)
        if not loadablesBySeries.has_key(seriesUID):
          loadablesBySeries[seriesUID] = [loadable]
        else:
          loadablesBySeries[seriesUID].append(loadable)

    # now for each series, find the highest confidence selected loadables
    # and set all others to be unselected
    for series in loadablesBySeries:
      highestConfidenceValue = -1
      for loadable in loadablesBySeries[series]:
        if loadable.confidence > highestConfidenceValue:
          highestConfidenceValue = loadable.confidence
      for loadable in loadablesBySeries[series]:
        if loadable.confidence < highestConfidenceValue:
          loadable.selected = False

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
      slicer.app.processEvents()
      self.progress.setValue(step)
      slicer.app.processEvents()
      try:
        self.loadablesByPlugin[plugin] = plugin.examine(fileLists)
        loadEnabled = loadEnabled or self.loadablesByPlugin[plugin] != []
      except Exception,e:
        import traceback
        traceback.print_exc()
        qt.QMessageBox.warning(self.window,
            "DICOM", "Warning: Plugin failed: %s\n\nSee python console for error message." % pluginClass)
        print("DICOM Plugin failed: %s", str(e))

      step +=1
    self.loadButton.enabled = loadEnabled
    self.organizeLoadables()
    self.loadableTable.setLoadables(self.loadablesByPlugin)
    self.progress.close()
    self.progress = None

  def uncheckAllLoadables(self):
    self.loadableTable.uncheckAll()

  def loadCheckedLoadables(self):
    """Invoke the load method on each plugin for the DICOMLoadable
    instances that are selected"""
    self.loadableTable.updateCheckstate()
    loadableCount = 0
    for plugin in self.loadablesByPlugin:
      for loadable in self.loadablesByPlugin[plugin]:
        if loadable.selected:
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
        slicer.app.processEvents()
        self.progress.setValue(step)
        slicer.app.processEvents()
        if loadable.selected:
          self.progress.labelText = '\nLoading %s' % loadable.name
          slicer.app.processEvents()
          if not plugin.load(loadable):
            qt.QMessageBox.warning(slicer.util.mainWindow(), 
                'Load', 'Could not load: %s as a %s' % (loadable.name,plugin.loadType))
          step += 1
          self.progress.setValue(step)
          slicer.app.processEvents()
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

  def addLoadableRow(self,loadable,row,reader):
    """Add a row to the loadable table
    """
    # set checked state to unckecked if there is a loadable with the 
    # same file list in the table already
    if len(self.loadables) > 0:
      for addedRow in self.loadables.keys():
        if len(self.loadables[addedRow].files) == 1 and len(loadable.files) == 1:
          # needed because of the tuple-sequence comparison does not work, 
          # and sometimes tuples are created by some reason
          if self.loadables[addedRow].files[0] == loadable.files[0]:
            loadable.selected = False
            break
        elif self.loadables[addedRow].files == loadable.files:
          loadable.selected = False
          break
    # name and check state
    self.loadables[row] = loadable
    item = qt.QTableWidgetItem(loadable.name)
    item.setCheckState(loadable.selected * 2)
    self.items.append(item)
    self.widget.setItem(row,0,item)
    item.setToolTip(loadable.tooltip)
    # reader
    if reader:
      readerItem = qt.QTableWidgetItem(reader)
      self.items.append(readerItem)
      self.widget.setItem(row,1,readerItem)
      readerItem.setToolTip(item.toolTip())
    # warning
    if loadable.warning:
      warnItem = qt.QTableWidgetItem(loadable.warning)
      self.items.append(warnItem)
      self.widget.setItem(row,2,warnItem)
      item.setToolTip(item.toolTip() + "\n" + loadable.warning)
      warnItem.setToolTip(item.toolTip())

  def setLoadables(self,loadablesByPlugin):
    """Load the table widget with a list
    of volume options (of class DICOMVolume)
    """
    loadableCount = 0
    for plugin in loadablesByPlugin:
      for loadable in loadablesByPlugin[plugin]:
        loadableCount += 1
    self.widget.clearContents()
    self.widget.setColumnCount(3)
    self.widget.setHorizontalHeaderLabels(['DICOM Data','Reader','Warnings'])
    self.widget.setColumnWidth(0,int(self.width * 0.4))
    self.widget.setColumnWidth(1,int(self.width * 0.2))
    self.widget.setColumnWidth(2,int(self.width * 0.4))
    self.widget.setRowCount(loadableCount)
    self.loadables = {}
    row = 0

    for selectState in (True,False):
      for plugin in loadablesByPlugin:
        for loadable in loadablesByPlugin[plugin]:
          if loadable.selected == selectState:
            self.addLoadableRow(loadable,row,plugin.loadType)
            row += 1

    self.widget.setVerticalHeaderLabels(row * [""])

  def uncheckAll(self):
    for row in xrange(self.widget.rowCount):
      item = self.widget.item(row,0)
      item.setCheckState(False)

  def updateCheckstate(self):
    for row in xrange(self.widget.rowCount):
      item = self.widget.item(row,0)
      self.loadables[row].selected = (item.checkState() != 0)

class DICOMHeaderWidget(object):
  """Implement the Qt code for a table of
  DICOM header values
  """
  # TODO: move this to ctk and use data dictionary for 
  # tag names

  def __init__(self,parent):
    self.widget = qt.QTableWidget(parent,width=350,height=300)
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

class DICOMRecentActivityWidget(object):
  """Display the recent activity of the slicer DICOM database
  """

  def __init__(self,parent,dicomDatabase=None,detailsPopup=None):
    if dicomDatabase:
      self.dicomDatabase = dicomDatabase
    else:
      self.dicomDatabase = slicer.dicomDatabase
    self.detailsPopup = detailsPopup
    self.recentSeries = []
    self.widget = qt.QWidget(parent)
    self.widget.name = 'recentActivityWidget'
    self.layout = qt.QVBoxLayout()
    self.widget.setLayout(self.layout)

    self.statusLabel = qt.QLabel(self.widget)
    self.layout.addWidget(self.statusLabel)
    self.statusLabel.text = 'No inserts in the past hour'

    self.scrollArea = qt.QScrollArea()
    self.layout.addWidget(self.scrollArea)
    self.listWidget = qt.QListWidget()
    self.listWidget.name = 'recentActivityListWidget'
    self.scrollArea.setWidget(self.listWidget)
    self.scrollArea.setWidgetResizable(True)
    self.listWidget.setProperty('SH_ItemView_ActivateItemOnSingleClick', 1)
    self.listWidget.connect('activated(QModelIndex)', self.onActivated)

    self.refreshButton = qt.QPushButton(self.widget)
    self.layout.addWidget(self.refreshButton)
    self.refreshButton.text = 'Refresh'
    self.refreshButton.connect('clicked()', self.update)

    self.tags = {}
    self.tags['seriesDescription'] = "0008,103e"
    self.tags['patientName'] = "0010,0010"

  class seriesWithTime(object):
    """helper class to track series and time..."""
    def __init__(self,series,elapsedSinceInsert,insertDateTime,text):
      self.series = series
      self.elapsedSinceInsert = elapsedSinceInsert
      self.insertDateTime = insertDateTime
      self.text = text

  def compareSeriesTimes(self,a,b):
    if a.elapsedSinceInsert > b.elapsedSinceInsert:
      return 1
    else:
      return -1

  def recentSeriesList(self):
    """Return a list of series sorted by insert time
    (counting backwards from today)
    Assume that first insert time of series is valid 
    for entire series (should be close enough for this purpose)
    """
    recentSeries = []
    now = qt.QDateTime.currentDateTime()
    for patient in self.dicomDatabase.patients():
      for study in self.dicomDatabase.studiesForPatient(patient):
        for series in self.dicomDatabase.seriesForStudy(study):
          files = self.dicomDatabase.filesForSeries(series)
          if len(files) > 0:
            instance = self.dicomDatabase.instanceForFile(files[0])
            seriesTime = self.dicomDatabase.insertDateTimeForInstance(instance)
            try:
              patientName = self.dicomDatabase.instanceValue(instance,self.tags['patientName'])
            except RuntimeError:
              # this indicates that the particular instance is no longer
              # accessible to the dicom database, so we should ignore it here
              continue
            seriesDescription = self.dicomDatabase.instanceValue(instance,self.tags['seriesDescription'])
            elapsed = seriesTime.secsTo(now) 
            secondsPerHour = 60 * 60
            secondsPerDay = secondsPerHour * 24
            timeNote = None
            if elapsed < secondsPerDay:
              timeNote = 'Today'
            elif elapsed < 7 * secondsPerDay:
              timeNote = 'Past Week'
            elif elapsed < 30 * 7 * secondsPerDay:
              timeNote = 'Past Month'
            if timeNote:
              text = "%s: %s for %s" % (timeNote, seriesDescription, patientName)
              recentSeries.append( self.seriesWithTime(series, elapsed, seriesTime, text) )
    recentSeries.sort(self.compareSeriesTimes)
    return recentSeries


  def update(self):
    """Load the table widget with header values for the file
    """
    self.listWidget.clear()
    secondsPerHour = 60 * 60
    insertsPastHour = 0
    self.recentSeries = self.recentSeriesList()
    for series in self.recentSeries:
      self.listWidget.addItem(series.text)
      if series.elapsedSinceInsert < secondsPerHour:
        insertsPastHour += 1
    self.statusLabel.text = '%d series added to database in the past hour' % insertsPastHour
    if len(self.recentSeries) > 0:
      statusMessage = "Most recent DICOM Database addition: %s" % self.recentSeries[0].insertDateTime.toString()
      slicer.util.showStatusMessage(statusMessage, 10000)

  def onActivated(self,modelIndex):
    print('selected row %d' % modelIndex.row())
    print(self.recentSeries[modelIndex.row()].text)
    series = self.recentSeries[modelIndex.row()]
    if self.detailsPopup:
      self.detailsPopup.open()
      self.detailsPopup.offerLoadables(series.series,"Series")

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

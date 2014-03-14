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
  operations to perform on the selected dicom list item.
  This is a helper used in the DICOMWidget class.
  """

  def __init__(self,dicomBrowser,setBrowserPersistence=None):
    self.dicomBrowser = dicomBrowser
    self.setBrowserPersistence = setBrowserPersistence
    self.popupGeometry = qt.QRect()
    settings = qt.QSettings()

    self.advancedViewCheckState = False

    self.horizontalOrientation = 0x1
    self.verticalOrientation = 0x2
    self.tableOrientation = self.verticalOrientation

    if settings.contains('DICOM/detailsPopup.geometry'):
      self.popupGeometry = settings.value('DICOM/detailsPopup.geometry')
    if settings.contains('DICOM/advancedViewToggled'):
      self.advancedViewCheckState = True

    if settings.contains('DICOM/tableOrientation'):
      self.tableOrientation = int(settings.value('DICOM/tableOrientation'))

    self.create()
    self.popupPositioned = False
    self.pluginInstances = {}
    self.fileLists = []

  def create(self,widgetType='window',showHeader=False,showPreview=False):
    """
    main window is a frame with widgets from the app
    widget repacked into it along with slicer-specific
    extra widgets
    """

    # find internals of widget for reference and repacking
    self.toolBar = slicer.util.findChildren(self.dicomBrowser, 'ToolBar')[0]
    self.databaseNameLabel = slicer.util.findChildren(self.dicomBrowser, 'DatabaseNameLabel')[0]
    self.databaseDirectoryButton = slicer.util.findChildren(self.dicomBrowser, 'DirectoryButton')[0]
    #self.tables = self.dicomBrowser.tableManager
    self.tables = slicer.util.findChildren(self.dicomBrowser, 'dicomTableManager')[0]

    #self.userFrame = slicer.util.findChildren(self.dicomBrowser, 'UserFrame')[0]
    self.userFrame = qt.QWidget()
    #self.thumbs = slicer.util.findChildren(self.dicomBrowser, 'ThumbnailsWidget')[0]
    #self.widthSlider = slicer.util.findChildren(self.dicomBrowser, 'ThumbnailWidthSlider')[0]
    self.preview = qt.QWidget()

    self.widgetType = widgetType
    if widgetType == 'dialog':
      self.window = qt.QDialog(self.dicomBrowser)
    elif widgetType == 'window':
      self.window = qt.QWidget()
    elif widgetType == 'popup':
      self.window = ctk.ctkPopupWidget(self.dicomBrowser)
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

    self.window.setWindowTitle('DICOM Browser')

    self.layout = qt.QVBoxLayout()
    self.window.setLayout(self.layout)

    # tool row at top, with commands and database
    self.toolFrame = qt.QWidget()
    self.toolLayout = qt.QHBoxLayout(self.toolFrame)
    self.layout.addWidget(self.toolFrame)
    self.toolLayout.addWidget(self.toolBar)
    self.toolLayout.addWidget(self.databaseNameLabel)
    self.toolLayout.addWidget(self.databaseDirectoryButton)

    # tables goes next, spread across 1 row, 2 columns
    self.tables.tableOrientation = self.tableOrientation
    self.tables.dynamicTableLayout = False
    self.tablesExpandableWidget = ctk.ctkExpandableWidget()
    self.layout.addWidget(self.tablesExpandableWidget)
    self.tablesLayout = qt.QVBoxLayout(self.tablesExpandableWidget)

    self.tablesLayout.addWidget(self.tables)

    #
    # preview related column
    #
    self.previewLayout = qt.QVBoxLayout()
    #self.layout.addLayout(self.previewLayout,selectionRow,0)

    #self.previewLayout.addWidget(self.thumbs)
    #self.previewLayout.addWidget(self.widthSlider)
    if showPreview:
      self.previewLayout.addWidget(self.preview)
    else:
      self.preview.hide()

    #
    # action related column (interacting with slicer)
    #
    self.loadableTableFrame = qt.QWidget()
    self.loadableTableLayout = qt.QFormLayout(self.loadableTableFrame)
    self.layout.addWidget(self.loadableTableFrame)

    self.loadableTableLayout.addWidget(self.userFrame)
    self.userFrame.hide()

    tableWidth = 350 if showHeader else 700
    self.loadableTable = DICOMLoadableTable(self.userFrame,width=tableWidth)
    #self.loadableTableLayout.addWidget(self.loadableTable.widget)
    #self.loadableTable.widget.hide()

    #
    # button row for action column
    #
    self.actionButtonsFrame = qt.QWidget()
    self.layout.addWidget(self.actionButtonsFrame)
    self.layout.addStretch(1)
    self.actionButtonLayout = qt.QHBoxLayout()
    self.actionButtonsFrame.setLayout(self.actionButtonLayout)

    self.loadButton = qt.QPushButton('Load')
    self.loadButton.enabled = True
    self.loadButton.toolTip = 'Load Selection to Slicer'
    self.actionButtonLayout.addWidget(self.loadButton)
    self.loadButton.connect('clicked()', self.loadCheckedLoadables)

    self.headerPopup = DICOMLib.DICOMHeaderPopup()

    self.viewMetadataButton = qt.QPushButton('Metadata')
    self.viewMetadataButton.toolTip = 'Display Metadata of the Selected Series'
    self.viewMetadataButton.enabled = True
    self.actionButtonLayout.addWidget(self.viewMetadataButton)
    self.viewMetadataButton.connect('clicked()', self.onViewHeaderButton)
    self.viewMetadataButton.connect('clicked()', self.headerPopup.open)
    self.actionButtonLayout.addStretch(1)

    self.examineButton = qt.QPushButton('Examine')
    self.actionButtonLayout.addWidget(self.examineButton)
    self.examineButton.enabled = False
    self.examineButton.connect('clicked()', self.examineForLoading)

    self.uncheckAllButton = qt.QPushButton('Uncheck All')
    self.actionButtonLayout.addWidget(self.uncheckAllButton)
    self.uncheckAllButton.connect('clicked()', self.uncheckAllLoadables)
    self.actionButtonLayout.addStretch(1)

    self.closeButton = qt.QPushButton('Close')
    #self.actionButtonLayout.addWidget(self.closeButton)
    self.closeButton.connect('clicked()', self.close)

    self.advancedViewButton = qt.QCheckBox('Advanced')
    self.actionButtonLayout.addWidget(self.advancedViewButton)
    self.advancedViewButton.enabled = True
    self.advancedViewButton.checked = self.advancedViewCheckState
    self.advancedViewButton.connect('stateChanged(int)', self.onAdvanedViewButton)

    self.horizontalViewCheckBox = qt.QCheckBox('Horizontal Tables')
    self.toolLayout.addWidget(self.horizontalViewCheckBox)
    self.horizontalViewCheckBox.enabled = True
    if self.tableOrientation == 1:
      self.horizontalViewCheckBox.checked = True
    self.horizontalViewCheckBox.connect('stateChanged(int)', self.onHorizontalViewCheckBox)

    if self.setBrowserPersistence:
      self.browserPersistentButton = qt.QCheckBox('Browser Persistent')
      self.browserPersistentButton.toolTip = 'When enabled, DICOM Broswer remains open and usable after leaving DICOM module'
      self.actionButtonLayout.addWidget(self.browserPersistentButton)
      self.browserPersistentButton.connect('stateChanged(int)', self.setBrowserPersistence)


    if self.advancedViewCheckState == True:
      self.loadableTableFrame.show()
    else:
      self.examineButton.hide()
      self.uncheckAllButton.hide()
      self.loadableTableFrame.hide()
    #
    # header related column (more details about the selected file)
    #
    if showHeader:
      self.headerLayout = qt.QVBoxLayout()
      self.layout.addLayout(self.headerLayout,selectionRow,2)
      self.header = DICOMHeaderWidget(self.window)
      self.headerLayout.addWidget(self.header.widget)

    #
    # Plugin selection widget
    #
    self.pluginSelector = DICOMPluginSelector(self.window)
    self.loadableTableLayout.addRow(self.pluginSelector.widget,self.loadableTable.widget)

  def onAdvanedViewButton (self,state):
    settings = qt.QSettings()
    self.advancedViewCheckState = self.advancedViewButton.checked

    # if advanced mode is not toggled
    if state == 0:
      settings.remove('DICOM/advancedViewToggled')
      self.loadableTableFrame.hide()
      self.examineButton.hide()
      self.uncheckAllButton.hide()
      self.loadButton.enabled = True
      for pluginClass in slicer.modules.dicomPlugins:
        if pluginClass == 'DICOMScalarVolumePlugin':
          self.pluginSelector.checkBoxByPlugin[pluginClass].checked = True
        else:
          self.pluginSelector.checkBoxByPlugin[pluginClass].checked = False

    # if advanced mode is toggled
    if state == 2:
      settings.setValue('DICOM/advancedViewToggled', self.advancedViewCheckState )
      self.loadableTableFrame.show()
      for pluginClass in slicer.modules.dicomPlugins:
        self.pluginSelector.checkBoxByPlugin[pluginClass].checked = True
      self.examineButton.show()
      self.uncheckAllButton.show()
      self.loadButton.enabled = False

  def onHorizontalViewCheckBox(self, direction):

    settings = qt.QSettings()

    # Horizontal checkbox is unchecked
    if direction == 0:
      self.tables.tableOrientation = self.verticalOrientation

    # Horizontal checkbox is checked
    elif direction == 2:
      self.tables.tableOrientation = self.horizontalOrientation

    self.tableOrientation = self.tables.tableOrientation
    settings.setValue('DICOM/tableOrientation', self.tableOrientation)

  def onViewHeaderButton(self):
    self.headerPopup.setFileLists(self.fileLists)

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

  def offerLoadables(self,uidArgument,role):
    """Get all the loadable options at the currently selected level
    and present them in the loadable table"""
    self.loadableTable.setLoadables([])
    if self.advancedViewButton.checkState() == 2:
      self.loadButton.enabled = False
    self.fileLists = []
    if role == "Series":
      self.fileLists.append(slicer.dicomDatabase.filesForSeries(uidArgument))
    if role == "SeriesUIDList":
      for uid in uidArgument:
        uid = uid.replace("'","")
        self.fileLists.append(slicer.dicomDatabase.filesForSeries(uid))
    if role == "Study":
      series = slicer.dicomDatabase.seriesForStudy(uidArgument)
      for serie in series:
        self.fileLists.append(slicer.dicomDatabase.filesForSeries(serie))
    if role == "Patient":
      studies = slicer.dicomDatabase.studiesForPatient(uidArgument)
      for study in studies:
        series = slicer.dicomDatabase.seriesForStudy(study)
        for serie in series:
          fileList = slicer.dicomDatabase.filesForSeries(serie)
          self.fileLists.append(fileList)
    self.examineButton.enabled = len(self.fileLists) != 0

  def uncheckAllLoadables(self):
    self.loadableTable.uncheckAll()

  def examineForLoading(self):
    """For selected plugins, give user the option
    of what to load"""
    allFileCount = missingFileCount = 0
    for fileList in self.fileLists:
        for filePath in fileList:
          allFileCount += 1
          if not os.path.exists(filePath):
            missingFileCount += 1

    if missingFileCount > 0:
      qt.QMessageBox.warning(self.window,
          "DICOM", "Warning: %d of %d selected files listed in the database cannot be found on disk." % (missingFileCount, allFileCount))

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
    plugins = self.pluginSelector.selectedPlugins()
    for pluginClass in plugins:
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
        self.loadablesByPlugin[plugin] = plugin.examine(self.fileLists)
        loadEnabled = loadEnabled or self.loadablesByPlugin[plugin] != []
      except Exception,e:
        import traceback
        traceback.print_exc()
        qt.QMessageBox.warning(self.window,
            "DICOM", "Warning: Plugin failed: %s\n\nSee python console for error message." % pluginClass)
        print("DICOM Plugin failed: %s", str(e))
      step +=1

    self.loadButton.enabled = loadEnabled
    #self.viewMetadataButton.enabled = loadEnabled
    self.organizeLoadables()
    self.loadableTable.setLoadables(self.loadablesByPlugin)
    self.progress.close()
    self.progress = None

  def loadCheckedLoadables(self):
    """Invoke the load method on each plugin for the DICOMLoadable
    instances that are selected"""
    if self.advancedViewButton.checkState() == 0:
      self.examineForLoading()

    self.loadableTable.updateSelectedFromCheckstate()
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

class DICOMPluginSelector(object):
  """Implement the Qt code for a table of
  selectable DICOM Plugins that determine
  which mappings from DICOM to slicer datatypes
  will be considered.
  """

  def __init__(self,parent, width=50,height=100):
    self.widget = qt.QWidget(parent)
    self.widget.setMinimumHeight(height)
    self.widget.setMinimumWidth(width)
    self.width = width
    self.height = height
    self.layout = qt.QVBoxLayout()
    self.widget.setLayout(self.layout)
    self.checkBoxByPlugin = {}
    for pluginClass in slicer.modules.dicomPlugins:
      self.checkBoxByPlugin[pluginClass] = qt.QCheckBox(pluginClass)
      if pluginClass == 'DICOMScalarVolumePlugin':
        self.checkBoxByPlugin[pluginClass].checked = True
      else:
        self.checkBoxByPlugin[pluginClass].checked = False
      self.layout.addWidget(self.checkBoxByPlugin[pluginClass])

  def selectedPlugins(self):
    """Return a list of selected plugins"""
    selectedPlugins = []
    for pluginClass in slicer.modules.dicomPlugins:
      if self.checkBoxByPlugin[pluginClass].checked:
        selectedPlugins.append(pluginClass)
    return selectedPlugins


class DICOMLoadableTable(object):
  """Implement the Qt code for a table of
  selectable slicer data to be made from
  the given dicom files
  """

  def __init__(self,parent, width=350,height=200):
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
    # name and check state
    qt_ItemIsEditable = 2 # not in PythonQt
    self.loadables[row] = loadable
    item = qt.QTableWidgetItem(loadable.name)
    item.setCheckState(loadable.selected * 2)
    self.items.append(item)
    self.widget.setItem(row,0,item)
    item.setToolTip(loadable.tooltip)
    # reader
    if reader:
      readerItem = qt.QTableWidgetItem(reader)
      readerItem.setFlags(readerItem.flags() ^ qt_ItemIsEditable)
      self.items.append(readerItem)
      self.widget.setItem(row,1,readerItem)
      readerItem.setToolTip(item.toolTip())
    # warning
    if loadable.warning:
      warning = loadable.warning
    else:
      warning = ''
    warnItem = qt.QTableWidgetItem(loadable.warning)
    warnItem.setFlags(warnItem.flags() ^ qt_ItemIsEditable)
    self.items.append(warnItem)
    self.widget.setItem(row,2,warnItem)
    item.setToolTip(item.toolTip() + "\n" + warning)
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

    for plugin in loadablesByPlugin:
      for thisLoadableId in xrange(len(loadablesByPlugin[plugin])):
        for prevLoadableId in xrange(0,thisLoadableId):
          thisLoadable = loadablesByPlugin[plugin][thisLoadableId]
          prevLoadable = loadablesByPlugin[plugin][prevLoadableId]
          if len(thisLoadable.files) == 1 and len(prevLoadable.files) == 1:
            # needed because of the tuple-sequence comparison does not work,
            # and sometimes tuples are created by some reason
            if thisLoadable.files[0] == prevLoadable.files[0]:
              thisLoadable.selected = False
              break
          elif thisLoadable.files == prevLoadable.files:
            thisLoadable.selected = False
            break

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

  def updateSelectedFromCheckstate(self):
    for row in xrange(self.widget.rowCount):
      item = self.widget.item(row,0)
      self.loadables[row].selected = (item.checkState() != 0)
      # updating the names
      self.loadables[row].name = item.text()


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

class DICOMHeaderPopup(object):

  def __init__(self):

    self.popupGeometry = qt.QRect()
    settings = qt.QSettings()
    if settings.contains('DICOM/headerPopup.geometry'):
      self.popupGeometry = settings.value('DICOM/headerPopup.geometry')
    self.popupPositioned = False
    self.window = ctk.ctkDICOMObjectListWidget()
    self.window.setWindowTitle('DICOM File Metadata')

    self.layout = qt.QGridLayout()
    self.window.setLayout(self.layout)

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

  def setFileLists(self,fileLists):
    filePaths = []
    for fileList in fileLists:
      for filePath in fileList:
        filePaths.append(filePath)
    self.window.setFileList(filePaths)

  def close(self):
    self.onPopupGeometryChanged()
    self.window.hide()

  def onPopupGeometryChanged(self):
    settings = qt.QSettings()
    self.popupGeometry = self.window.geometry
    settings.setValue('DICOM/headerPopup.geometry', self.window.geometry)

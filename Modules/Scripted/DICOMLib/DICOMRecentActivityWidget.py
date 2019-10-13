import os
import vtk, qt, ctk, slicer
import logging
from functools import cmp_to_key

class DICOMRecentActivityWidget(qt.QWidget):
  """Display the recent activity of the slicer DICOM database
     Example:
       slicer.util.selectModule('DICOM')
       import DICOMLib
       w = DICOMLib.DICOMRecentActivityWidget(None, slicer.dicomDatabase, slicer.modules.DICOMInstance.browserWidget)
       w.update()
       w.show()
  """

  def __init__(self, parent, dicomDatabase=None, browserWidget=None):
    """If browserWidget is specified (e.g., set to slicer.modules.DICOMInstance.browserWidget)
    then clicking on an item selects the series in that browserWidget.
    """
    super(DICOMRecentActivityWidget, self).__init__(parent)
    if dicomDatabase:
      self.dicomDatabase = dicomDatabase
    else:
      self.dicomDatabase = slicer.dicomDatabase
    self.browserWidget = browserWidget
    self.recentSeries = []
    self.name = 'recentActivityWidget'
    self.setLayout(qt.QVBoxLayout())

    self.statusLabel = qt.QLabel()
    self.layout().addWidget(self.statusLabel)
    self.statusLabel.text = ''

    self.scrollArea = qt.QScrollArea()
    self.layout().addWidget(self.scrollArea)
    self.listWidget = qt.QListWidget()
    self.listWidget.name = 'recentActivityListWidget'
    self.scrollArea.setWidget(self.listWidget)
    self.scrollArea.setWidgetResizable(True)
    self.listWidget.setProperty('SH_ItemView_ActivateItemOnSingleClick', 1)
    self.listWidget.connect('activated(QModelIndex)', self.onActivated)

    self.refreshButton = qt.QPushButton()
    self.layout().addWidget(self.refreshButton)
    self.refreshButton.text = 'Refresh'
    self.refreshButton.connect('clicked()', self.update)

    self.tags = {}
    self.tags['seriesDescription'] = "0008,103e"
    self.tags['patientName'] = "0010,0010"

  class seriesWithTime(object):
    """helper class to track series and time..."""

    def __init__(self, series, elapsedSinceInsert, insertDateTime, text):
      self.series = series
      self.elapsedSinceInsert = elapsedSinceInsert
      self.insertDateTime = insertDateTime
      self.text = text

  @staticmethod
  def compareSeriesTimes(a, b):
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
              patientName = self.dicomDatabase.instanceValue(instance, self.tags['patientName'])
            except RuntimeError:
              # this indicates that the particular instance is no longer
              # accessible to the dicom database, so we should ignore it here
              continue
            seriesDescription = self.dicomDatabase.instanceValue(instance, self.tags['seriesDescription'])
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
              recentSeries.append(self.seriesWithTime(series, elapsed, seriesTime, text))
    recentSeries.sort(key=cmp_to_key(self.compareSeriesTimes))
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

  def onActivated(self, modelIndex):
    logging.debug('Recent activity widget selected row: %d (%s)' % (modelIndex.row(), self.recentSeries[modelIndex.row()].text))
    if not self.browserWidget:
      return
    # Select series in the series table
    series = self.recentSeries[modelIndex.row()]
    seriesUID = series.series
    seriesTableView = self.browserWidget.dicomBrowser.dicomTableManager().seriesTable().tableView()
    foundModelIndex = seriesTableView.model().match(seriesTableView.model().index(0,0), qt.Qt.ItemDataRole(), seriesUID, 1)
    if foundModelIndex:
        row = foundModelIndex[0].row()
        seriesTableView.selectRow(row)

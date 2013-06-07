import os
import glob
from __main__ import qt
from __main__ import vtk
from __main__ import ctk
from __main__ import slicer

import DICOMLib

#
# DICOM
#
# This code includes the GUI for the slicer module.  It is supported
# by the DICOMLib python code which implements most of the logic
# for data exchange and running servers.
#

class DICOM:

  @staticmethod
  def setDatabasePrecacheTags(dicomApp=None):
    """query each plugin for tags that should be cached on import
       and set them for the dicom app widget and slicer"""
    if not slicer.dicomDatabase:
      return
    tagsToPrecache = list(slicer.dicomDatabase.tagsToPrecache)
    for pluginClass in slicer.modules.dicomPlugins:
      plugin = slicer.modules.dicomPlugins[pluginClass]()
      tagsToPrecache += plugin.tags.values()
    tagsToPrecache = list(set(tagsToPrecache))  # remove duplicates
    tagsToPrecache.sort()
    slicer.dicomDatabase.tagsToPrecache = tagsToPrecache
    if dicomApp:
      dicomApp.tagsToPrecache = tagsToPrecache

  def __init__(self, parent):
    import string
    parent.title = "DICOM"
    parent.categories = ["", "Informatics"] # top level module
    parent.contributors = ["Steve Pieper (Isomics)"]
    parent.helpText = string.Template("""
The DICOM module integrates DICOM classes from CTK (based on DCMTK).  See <a href=\"$a/Documentation/$b.$c/Modules/DICOM\">the documentaiton</a> for more information.
""").substitute({ 'a':parent.slicerWikiUrl, 'b':slicer.app.majorVersion, 'c':slicer.app.minorVersion })
    parent.acknowledgementText = """
This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a href=http://www.slicer.org>http://www.slicer.org</a> for details.  Module implemented by Steve Pieper.  Based on work from CommonTK (http://www.commontk.org).
    """
    parent.icon = qt.QIcon(':Icons/Medium/SlicerLoadDICOM.png')
    self.parent = parent

    if slicer.mrmlScene.GetTagByClassName( "vtkMRMLScriptedModuleNode" ) != 'ScriptedModule':
      slicer.mrmlScene.RegisterNodeClass(vtkMRMLScriptedModuleNode())

    # initialize the dicom infrastructure
    slicer.dicomDatabase = None
    settings = qt.QSettings()
    # the dicom database is a global object for slicer
    if settings.contains('DatabaseDirectory'):
      databaseDirectory = settings.value('DatabaseDirectory')
      if databaseDirectory:
        slicer.dicomDatabase = ctk.ctkDICOMDatabase()
        slicer.dicomDatabase.openDatabase(databaseDirectory + "/ctkDICOM.sql", "SLICER")
        if not slicer.dicomDatabase.isOpen:
          # can't open the database, so prompt the user later if they enter module
          slicer.dicomDatabase = None
        else:
          # the dicom listener is also global, but only started on app start if
          # the user so chooses
          if settings.contains('DICOM/RunListenerAtStart'):
            if bool(settings.value('DICOM/RunListenerAtStart')):
              if not hasattr(slicer, 'dicomListener'):
                try:
                  slicer.dicomListener = DICOMLib.DICOMListener(slicer.dicomDatabase)
                  slicer.dicomListener.start()
                except (UserWarning,OSError) as message:
                  # TODO: how to put this into the error log?
                  print ('Problem trying to start DICOMListener:\n %s' % message)
        if slicer.dicomDatabase:
          slicer.app.setDICOMDatabase(slicer.dicomDatabase)

    # Trigger the menu to be added when application has started up
    if not slicer.app.commandOptions().noMainWindow :
      qt.QTimer.singleShot(0, self.addMenu)
    # set the dicom pre-cache tags once all plugin classes have been initialized
    qt.QTimer.singleShot(0, DICOM.setDatabasePrecacheTags)

  def addMenu(self):
    """Add an action to the File menu that will go into
    the DICOM module by selecting the module.  Note that 
    once the module is constructed (below in setup) another
    connection is made that will also cause the instance-created
    DICOM browser to be raised by this menu action"""
    a = self.parent.action()
    menuFile = slicer.util.lookupTopLevelWidget('menuFile')
    if menuFile:
      for action in menuFile.actions():
        if action.text == 'Save':
          menuFile.insertAction(action,a)

  def __del__(self):
    if hasattr(slicer, 'dicomListener'):
      print('trying to stop listener')
      slicer.dicomListener.stop()


#
# DICOM file dialog
#
class DICOMFileDialog:
  """This specially named class is detected by the scripted loadable
  module and is the target for optional drag and drop operations.
  See: Base/QTGUI/qSlicerScriptedFileDialog.h
  and commit http://svn.slicer.org/Slicer4/trunk@21951 and issue #3081
  """

  def __init__(self,qSlicerFileDialog):
    self.qSlicerFileDialog = qSlicerFileDialog
    qSlicerFileDialog.fileType = 'DICOM Directory'
    qSlicerFileDialog.description = 'Load directory into DICOM database'
    qSlicerFileDialog.action = slicer.qSlicerFileDialog.Read
    self.directoriesToAdd = []

  def execDialog(self):
    """Not used"""
    print('execDialog called on %s' % self)

  def isMimeDataAccepted(self):
    """Checks the dropped data and returns true if it is one or
    more directories"""
    self.directoriesToAdd = []
    mimeData = self.qSlicerFileDialog.mimeData()
    if mimeData.hasFormat('text/uri-list'):
      urls = mimeData.urls()
      for url in urls:
        localPath = url.toLocalFile() # convert QUrl to local path
        pathInfo = qt.QFileInfo()
        pathInfo.setFile(localPath) # information about the path
        if pathInfo.isDir(): # if it is a directory we add the files to the dialog
          self.directoriesToAdd.append(localPath)
    self.qSlicerFileDialog.acceptMimeData(len(self.directoriesToAdd) != 0)

  def dropEvent(self):
    mainWindow = slicer.util.mainWindow()
    mainWindow.moduleSelector().selectModule('DICOM')
    dicomWidget = slicer.modules.DICOMWidget
    for directory in self.directoriesToAdd:
      dicomWidget.detailsPopup.dicomApp.onImportDirectory(directory)
    self.directoriesToAdd = []


#
# DICOM widget
#

class DICOMWidget:
  """
  Slicer module that creates the Qt GUI for interacting with DICOM
  """

  def __init__(self, parent=None):
    self.testingServer = None
    self.dicomApp = None

    # hide the search box
    self.hideSearch = True

    # options for browser
    self.browserPersistent = False

    # TODO: are these wrapped so we can avoid magic numbers?
    self.dicomModelUIDRole = 32
    self.dicomModelTypeRole = self.dicomModelUIDRole + 1
    self.dicomModelTypes = ('Root', 'Patient', 'Study', 'Series', 'Image')

    # state management for compressing events
    # - each time an update is requested, start the singleShot timer
    # - if the update is requested before the timeout, the call to timer.start() resets it
    # - the actual update only happens when the the full time elapses since the last request
    self.resumeModelTimer = qt.QTimer()
    self.resumeModelTimer.singleShot = True
    self.resumeModelTimer.interval = 500
    self.resumeModelTimer.connect('timeout()', self.onResumeModelRequestTimeout)
    self.updateRecentActivityTimer = qt.QTimer()
    self.updateRecentActivityTimer.singleShot = True
    self.updateRecentActivityTimer.interval = 500
    self.updateRecentActivityTimer.connect('timeout()', self.onUpateRecentActivityRequestTimeout)

    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
      self.layout = self.parent.layout()
      self.setup()
      self.parent.show()
    else:
      self.parent = parent
      self.layout = parent.layout()

    globals()['d'] = self

  def enter(self):
    self.detailsPopup.open()

  def exit(self):
    if not self.browserPersistent:
      self.detailsPopup.close()

  def updateGUIFromMRML(self, caller, event):
    pass

  # sets up the widget
  def setup(self):

    #
    # servers
    #

    # testing server - not exposed (used for development)
    self.localFrame = ctk.ctkCollapsibleButton(self.parent)
    self.localFrame.setLayout(qt.QVBoxLayout())
    self.localFrame.setText("Servers")
    self.layout.addWidget(self.localFrame)
    self.localFrame.collapsed = False

    self.toggleServer = qt.QPushButton("Start Testing Server")
    self.localFrame.layout().addWidget(self.toggleServer)
    self.toggleServer.connect('clicked()', self.onToggleServer)

    self.verboseServer = qt.QCheckBox("Verbose")
    self.localFrame.layout().addWidget(self.verboseServer)

    # advanced options - not exposed to end users
    # developers can uncomment these lines to access testing server
    self.toggleServer.hide()
    self.verboseServer.hide()

    # Listener

    settings = qt.QSettings()
    self.toggleListener = qt.QPushButton()
    if hasattr(slicer, 'dicomListener'):
      self.toggleListener.text = "Stop Listener"
      slicer.dicomListener.process.connect('stateChanged(int)',self.onListenerStateChanged)
    else:
      self.toggleListener.text = "Start Listener"
    self.localFrame.layout().addWidget(self.toggleListener)
    self.toggleListener.connect('clicked()', self.onToggleListener)

    self.runListenerAtStart = qt.QCheckBox("Start Listener when Slicer Starts")
    self.localFrame.layout().addWidget(self.runListenerAtStart)
    if settings.contains('DICOM/RunListenerAtStart'):
      self.runListenerAtStart.checked = bool(settings.value('DICOM/RunListenerAtStart'))
    self.runListenerAtStart.connect('clicked()', self.onRunListenerAtStart)

    # the Database frame (home of the ctkDICOM widget)
    self.dicomFrame = ctk.ctkCollapsibleButton(self.parent)
    self.dicomFrame.setLayout(qt.QVBoxLayout())
    self.dicomFrame.setText("DICOM Database and Networking")
    self.layout.addWidget(self.dicomFrame)

    # initialize the dicomDatabase
    #   - pick a default and let the user know
    if not slicer.dicomDatabase:
      self.promptForDatabaseDirectory()

    #
    # create and configure the app widget - this involves
    # reaching inside and manipulating the widget hierarchy
    # - TODO: this configurability should be exposed more natively
    #   in the CTK code to avoid the findChildren calls
    #
    self.dicomApp = ctk.ctkDICOMAppWidget()
    DICOM.setDatabasePrecacheTags(self.dicomApp)
    if self.hideSearch:
      # hide the search options - doesn't work yet and doesn't fit
      # well into the frame
      slicer.util.findChildren(self.dicomApp, 'SearchOption')[0].hide()

    self.detailsPopup = DICOMLib.DICOMDetailsPopup(self.dicomApp,setBrowserPersistence=self.setBrowserPersistence)

    self.tree = self.detailsPopup.tree

    self.showBrowser = qt.QPushButton('Show DICOM Browser')
    self.dicomFrame.layout().addWidget(self.showBrowser)
    self.showBrowser.connect('clicked()', self.detailsPopup.open)

    # connect to the main window's dicom button
    mw = slicer.util.mainWindow()
    try:
      action = slicer.util.findChildren(mw,name='actionLoadDICOM')[0]
      action.connect('triggered()',self.detailsPopup.open)
    except IndexError:
      print('Could not connect to the main window DICOM button')

    # connect to our menu file entry so it raises the browser
    menuFile = slicer.util.lookupTopLevelWidget('menuFile')
    if menuFile:
      for action in menuFile.actions():
        if action.text == 'DICOM':
          action.connect('triggered()',self.detailsPopup.open)

    # make the tree view a bit bigger
    self.tree.setMinimumHeight(250)

    if hasattr(slicer, 'dicomListener'):
      slicer.dicomListener.fileToBeAddedCallback = self.onListenerToAddFile
      slicer.dicomListener.fileAddedCallback = self.onListenerAddedFile

    self.contextMenu = qt.QMenu(self.tree)
    self.exportAction = qt.QAction("Export to Study", self.contextMenu)
    self.contextMenu.addAction(self.exportAction)
    self.exportAction.enabled = False
    self.deleteAction = qt.QAction("Delete", self.contextMenu)
    self.contextMenu.addAction(self.deleteAction)
    self.contextMenu.connect('triggered(QAction*)', self.onContextMenuTriggered)

    slicer.dicomDatabase.connect('databaseChanged()', self.onDatabaseChanged)
    self.dicomApp.connect('databaseDirectoryChanged(QString)', self.onDatabaseDirectoryChanged)
    selectionModel = self.tree.selectionModel()
    # TODO: can't use this because QList<QModelIndex> is not visible in PythonQt
    #selectionModel.connect('selectionChanged(QItemSelection, QItemSelection)', self.onTreeSelectionChanged)
    self.tree.connect('clicked(QModelIndex)', self.onTreeClicked)
    self.tree.setContextMenuPolicy(3)
    self.tree.connect('customContextMenuRequested(QPoint)', self.onTreeContextMenuRequested)

    # enable to the Send button of the app widget and take it over
    # for our purposes - TODO: fix this to enable it at the ctkDICOM level
    self.sendButton = slicer.util.findChildren(self.dicomApp, text='Send')[0]
    self.sendButton.enabled = False
    self.sendButton.connect('triggered()', self.onSendClicked)

    # the recent activity frame
    self.activityFrame = ctk.ctkCollapsibleButton(self.parent)
    self.activityFrame.setLayout(qt.QVBoxLayout())
    self.activityFrame.setText("Recent DICOM Activity")
    self.layout.addWidget(self.activityFrame)

    self.recentActivity = DICOMLib.DICOMRecentActivityWidget(self.activityFrame,detailsPopup=self.detailsPopup)
    self.activityFrame.layout().addWidget(self.recentActivity.widget)
    self.requestUpdateRecentActivity()


    # Add spacer to layout
    self.layout.addStretch(1)

  def onDatabaseChanged(self):
    """Use this because to update the view in response to things
    like database inserts.  Ideally the model would do this
    directly based on signals from the SQLite database, but
    that is not currently available.
    https://bugreports.qt-project.org/browse/QTBUG-10775
    """
    self.dicomApp.suspendModel()
    self.requestResumeModel()
    self.requestUpdateRecentActivity()

  def requestUpdateRecentActivity(self):
    """This method serves to compress the requests for updating
    the recent activity widget since it is time consuming and there can be
    many of them coming in a rapid sequence when the
    database is active"""
    self.updateRecentActivityTimer.start()

  def onUpateRecentActivityRequestTimeout(self):
    self.recentActivity.update()

  def requestResumeModel(self):
    """This method serves to compress the requests for resuming
    the dicom model since it is time consuming and there can be
    many of them coming in a rapid sequence when the
    database is active"""
    self.resumeModelTimer.start()

  def onResumeModelRequestTimeout(self):
    self.dicomApp.resumeModel()

  def onDatabaseDirectoryChanged(self,databaseDirectory):
    if not hasattr(slicer, 'dicomDatabase') or not slicer.dicomDatabase:
      slicer.dicomDatabase = ctk.ctkDICOMDatabase()
    DICOM.setDatabasePrecacheTags(self.dicomApp)
    databaseFilepath = databaseDirectory + "/ctkDICOM.sql"
    messages = ""
    if not os.path.exists(databaseDirectory):
      try:
        os.mkdir(databaseDirectory)
      except OSError:
        messages += "Directory does not exist and cannot be created. "
    else:
      if not os.access(databaseDirectory, os.W_OK):
        messages += "Directory not writable. "
      if not os.access(databaseDirectory, os.R_OK):
        messages += "Directory not readable. "
    if messages != "":
      self.messageBox('The database file path "%s" cannot be used.  %s\nPlease pick a different database directory using the LocalDatabase button in the DICOM Browser.' % (databaseFilepath,messages))
    else:
      slicer.dicomDatabase.openDatabase(databaseDirectory + "/ctkDICOM.sql", "SLICER")
      if not slicer.dicomDatabase.isOpen:
        self.messageBox('The database file path "%s" cannot be opened.\nPlease pick a different database directory using the LocalDatabase button in the DICOM Browser.' % databaseFilepath)
        self.dicomDatabase = None
      else:
        if self.dicomApp:
          if self.dicomApp.databaseDirectory != databaseDirectory:
            self.dicomApp.databaseDirectory = databaseDirectory
        else:
          settings = qt.QSettings()
          settings.setValue('DatabaseDirectory', databaseDirectory)
          settings.sync()
    if slicer.dicomDatabase:
      slicer.app.setDICOMDatabase(slicer.dicomDatabase)

  def promptForDatabaseDirectory(self):
    """Ask the user to pick a database directory.
    But, if the application is in testing mode, just pick
    a temp directory
    """
    commandOptions = slicer.app.commandOptions()
    if commandOptions.testingEnabled:
      databaseDirectory = slicer.app.temporaryPath + '/tempDICOMDatbase'
      qt.QDir().mkpath(databaseDirectory)
      self.onDatabaseDirectoryChanged(databaseDirectory)
    else:
      settings = qt.QSettings()
      databaseDirectory = settings.value('DatabaseDirectory')
      if databaseDirectory:
        self.onDatabaseDirectoryChanged(databaseDirectory)
      else:
        # pick the user's Documents by default
        documentsLocation = qt.QDesktopServices.DocumentsLocation
        documents = qt.QDesktopServices.storageLocation(documentsLocation)
        databaseDirectory = documents + "/SlicerDICOMDatabase"
        message = "DICOM Database will be stored in\n\n"
        message += databaseDirectory
        message += "\n\nUse the Local Database button in the DICOM Browser "
        message += "to pick a different location."
        qt.QMessageBox.information(slicer.util.mainWindow(),
                        'DICOM', message, qt.QMessageBox.Ok)
        if not os.path.exists(databaseDirectory):
          os.mkdir(databaseDirectory)
        self.onDatabaseDirectoryChanged(databaseDirectory)

  def onTreeClicked(self,index):
    self.model = index.model()
    self.tree.setExpanded(index, not self.tree.expanded(index))
    self.selection = index.sibling(index.row(), 0)
    typeRole = self.selection.data(self.dicomModelTypeRole)
    if typeRole > 0:
      self.sendButton.enabled = True
    else:
      self.sendButton.enabled = False
    if typeRole:
      self.exportAction.enabled = self.dicomModelTypes[typeRole] == "Study"
    else:
      self.exportAction.enabled = False
    self.detailsPopup.open()
    uid = self.selection.data(self.dicomModelUIDRole)
    role = self.dicomModelTypes[self.selection.data(self.dicomModelTypeRole)]
    self.detailsPopup.offerLoadables(uid, role)

  def onTreeContextMenuRequested(self,pos):
    index = self.tree.indexAt(pos)
    self.selection = index.sibling(index.row(), 0)
    self.contextMenu.popup(self.tree.mapToGlobal(pos))

  def onContextMenuTriggered(self,action):
    if action == self.deleteAction:
      typeRole = self.selection.data(self.dicomModelTypeRole)
      role = self.dicomModelTypes[typeRole]
      uid = self.selection.data(self.dicomModelUIDRole)
      if self.okayCancel('This will remove references from the database\n(Files will not be deleted)\n\nDelete %s?' % role):
        # TODO: add delete option to ctkDICOMDatabase
        self.dicomApp.suspendModel()
        if role == "Patient":
          removeWorked = slicer.dicomDatabase.removePatient(uid)
        elif role == "Study":
          removeWorked = slicer.dicomDatabase.removeStudy(uid)
        elif role == "Series":
          removeWorked = slicer.dicomDatabase.removeSeries(uid)
        if not removeWorked:
          self.messageBox(self,"Could not remove %s" % role,title='DICOM')
        self.dicomApp.resumeModel()
    elif action == self.exportAction:
      self.onExportClicked()

  def onExportClicked(self):
    """Associate a slicer volume as a series in the selected dicom study"""
    uid = self.selection.data(self.dicomModelUIDRole)
    exportDialog = DICOMLib.DICOMExportDialog(uid,onExportFinished=self.onExportFinished)
    self.dicomApp.suspendModel()
    exportDialog.open()

  def onExportFinished(self):
    self.requestResumeModel()

  def onSendClicked(self):
    """Perform a dicom store of slicer data to a peer"""
    # TODO: this should migrate to ctk for a more complete implementation
    # - just the basics for now
    uid = self.selection.data(self.dicomModelUIDRole)
    role = self.dicomModelTypes[self.selection.data(self.dicomModelTypeRole)]
    studies = []
    if role == "Patient":
      studies = slicer.dicomDatabase.studiesForPatient(uid)
    if role == "Study":
      studies = [uid]
    series = []
    if role == "Series":
      series = [uid]
    else:
      for study in studies:
        series += slicer.dicomDatabase.seriesForStudy(study)
    files = []
    for serie in series:
      files += slicer.dicomDatabase.filesForSeries(serie)
    sendDialog = DICOMLib.DICOMSendDialog(files)
    sendDialog.open()

  def setBrowserPersistence(self,onOff):
    self.detailsPopup.setModality(not onOff)
    self.browserPersistent = onOff

  def onToggleListener(self):
    if hasattr(slicer, 'dicomListener'):
      slicer.dicomListener.stop()
      del slicer.dicomListener
      self.toggleListener.text = "Start Listener"
    else:
      try:
        slicer.dicomListener = DICOMLib.DICOMListener(database=slicer.dicomDatabase)
        slicer.dicomListener.start()
        self.onListenerStateChanged(slicer.dicomListener.process.state())
        slicer.dicomListener.process.connect('stateChanged(QProcess::ProcessState)',self.onListenerStateChanged)
        slicer.dicomListener.fileToBeAddedCallback = self.onListenerToAddFile
        slicer.dicomListener.fileAddedCallback = self.onListenerAddedFile
        self.toggleListener.text = "Stop Listener"
      except UserWarning as message:
        self.messageBox(self,"Could not start listener:\n %s" % message,title='DICOM')

  def onListenerStateChanged(self,newState):
    """ Called when the indexer process state changes
    so we can provide feedback to the user
    """
    if newState == 0:
      slicer.util.showStatusMessage("DICOM Listener not running")
    if newState == 1:
      slicer.util.showStatusMessage("DICOM Listener starting")
    if newState == 2:
      slicer.util.showStatusMessage("DICOM Listener running")

  def onListenerToAddFile(self):
    """ Called when the indexer is about to add a file to the database.
    Works around issue where ctkDICOMModel has open queries that keep the
    database locked.
    """
    self.dicomApp.suspendModel()

  def onListenerAddedFile(self):
    """Called after the listener has added a file.
    Restore and refresh the app model
    """
    newFile = slicer.dicomListener.lastFileAdded
    if newFile:
      slicer.util.showStatusMessage("Loaded: %s" % newFile, 1000)
    self.requestResumeModel()

  def onToggleServer(self):
    if self.testingServer and self.testingServer.qrRunning():
      self.testingServer.stop()
      self.toggleServer.text = "Start Testing Server"
    else:
      #
      # create&configure the testingServer if needed, start the server, and populate it
      #
      if not self.testingServer:
        # find the helper executables (only works on build trees
        # with standard naming conventions)
        self.exeDir = slicer.app.slicerHome
        if slicer.app.intDir:
          self.exeDir = self.exeDir + '/' + slicer.app.intDir
        self.exeDir = self.exeDir + '/../CTK-build/DCMTK-build'

        # TODO: deal with Debug/RelWithDebInfo on windows

        # set up temp dir
        tmpDir = slicer.app.userSettings().value('Modules/TemporaryDirectory')
        if not os.path.exists(tmpDir):
          os.mkdir(tmpDir)
        self.tmpDir = tmpDir + '/DICOM'
        if not os.path.exists(self.tmpDir):
          os.mkdir(self.tmpDir)
        self.testingServer = DICOMLib.DICOMTestingQRServer(exeDir=self.exeDir,tmpDir=self.tmpDir)

      # look for the sample data to load (only works on build trees
      # with standard naming conventions)
      self.dataDir =  slicer.app.slicerHome + '/../../Slicer4/Testing/Data/Input/CTHeadAxialDicom'
      files = glob.glob(self.dataDir+'/*.dcm')

      # now start the server
      self.testingServer.start(verbose=self.verboseServer.checked,initialFiles=files)
      self.toggleServer.text = "Stop Testing Server"

  def onRunListenerAtStart(self):
    settings = qt.QSettings()
    settings.setValue('DICOM/RunListenerAtStart', self.runListenerAtStart.checked)

  def messageBox(self,text,title='DICOM'):
    self.mb = qt.QMessageBox(slicer.util.mainWindow())
    self.mb.setWindowTitle(title)
    self.mb.setText(text)
    self.mb.setWindowModality(1)
    self.mb.exec_()
    return

  def question(self,text,title='DICOM'):
    return qt.QMessageBox.question(slicer.util.mainWindow(), title, text, 0x14000) == 0x4000

  def okayCancel(self,text,title='DICOM'):
    return qt.QMessageBox.question(slicer.util.mainWindow(), title, text, 0x400400) == 0x400

def DICOMTest():
  w = slicer.modules.dicom.widgetRepresentation()
  w.onToggleServer()
  queryButton = slicer.util.findChildren(w.dicomApp, text='Query')[0]
  queryButton.click()


  print("DICOMTest Passed!")
  return True

def DICOMDemo():
  pass

if __name__ == "__main__":
  import sys
  if '--test' in sys.argv:
    if DICOMTest():
      exit(0)
    exit(1)
  if '--demo' in sys.argv:
    DICOMDemo()
    exit()
  # TODO - 'exit()' returns so this code gets run
  # even if the argument matches one of the cases above
  #print ("usage: DICOM.py [--test | --demo]")

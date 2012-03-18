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
  def __init__(self, parent):
    import string
    parent.title = "DICOM"
    parent.categories = ["", "Informatics"] # top level module
    parent.contributors = ["Steve Pieper (Isomics)"]
    parent.helpText = string.Template("""
The DICOM module integrates DICOM classes from CTK (based on DCMTK).  See <a href=\"$a/Documentation/$b.$c/Modules/DICOM\">$a/Documentation/$b.$c/Modules/DICOM</a> for more information.
""").substitute({ 'a':parent.slicerWikiUrl, 'b':slicer.app.majorVersion, 'c':slicer.app.minorVersion })
    parent.acknowledgementText = """
This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.  Module implemented by Steve Pieper.  Based on work from CommonTK (http://www.commontk.org).
    """
    parent.icon = qt.QIcon(':Icons/Medium/SlicerLoadDICOM.png')
    self.parent = parent

    if slicer.mrmlScene.GetTagByClassName( "vtkMRMLScriptedModuleNode" ) != 'ScriptedModule':
      slicer.mrmlScene.RegisterNodeClass(vtkMRMLScriptedModuleNode())

    # initialize the dicom infrastructure
    settings = qt.QSettings()
    # the dicom database is a global object for slicer
    if settings.contains('DatabaseDirectory'):
      databaseDirectory = settings.value('DatabaseDirectory')
      if databaseDirectory: 
        slicer.dicomDatabase = ctk.ctkDICOMDatabase()
        slicer.dicomDatabase.openDatabase(databaseDirectory + "/ctkDICOM.sql", "SLICER")
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
    else:
      slicer.dicomDatabase = None
            
  def __del__(self):
    if hasattr(slicer, 'dicomListener'):
      print('trying to stop listener')
      slicer.dicomListener.stop()


#
# DICOM widget
#

class DICOMWidget:
  """
  Slicer module that creates the Qt GUI for interacting with DICOM
  """

  def __init__(self, parent=None):
    self.testingServer = None
    
    # let the popup window manage data loading
    self.useDetailsPopup = True
    # hide the search box 
    self.hideSearch = True

    # options for browser
    self.browserPersistent = False

    # TODO: are these wrapped so we can avoid magic numbers?
    self.dicomModelUIDRole = 32
    self.dicomModelTypeRole = self.dicomModelUIDRole + 1
    self.dicomModelTypes = ('Root', 'Patient', 'Study', 'Series', 'Image')


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

    #
    # create and configure the app widget - this involves
    # reaching inside and manipulating the widget hierarchy
    # - TODO: this configurability should be exposed more natively
    #   in the CTK code to avoid the findChildren calls
    #
    self.dicomApp = ctk.ctkDICOMAppWidget()
    self.dicomFrame.layout().addWidget(self.dicomApp)
    if self.hideSearch:
      # hide the search options - doesn't work yet and doesn't fit 
      # well into the frame
      slicer.util.findChildren(self.dicomApp, 'SearchOption')[0].hide()

    if self.useDetailsPopup:
      self.detailsPopup = DICOMLib.DICOMDetailsPopup(self.dicomApp,setBrowserPersistence=self.setBrowserPersistence)
      # TODO: move all functions to popup
      # for now, create dummy buttons just so callbacks work
      self.exportButton = qt.QPushButton('Export Slicer Data to Study...')
      self.loadButton = qt.QPushButton('Load to Slicer')
      self.previewLabel = qt.QLabel()

      self.tree = self.detailsPopup.tree
      
      self.showBrowser = qt.QPushButton('Show DICOM Browser')
      self.dicomFrame.layout().addWidget(self.showBrowser)
      self.showBrowser.connect('clicked()', self.detailsPopup.open)

      self.dicomFrame.layout().addStretch(1)

    else:
      userFrame = slicer.util.findChildren(self.dicomApp, 'UserFrame')[0]
      userFrame.setLayout(qt.QVBoxLayout())
      self.previewLabel = qt.QLabel(userFrame)
      userFrame.layout().addWidget(self.previewLabel)
      self.loadButton = qt.QPushButton('Load to Slicer')
      self.loadButton.enabled = False 
      userFrame.layout().addWidget(self.loadButton)
      self.loadButton.connect('clicked()', self.onLoadButton)
      self.exportButton = qt.QPushButton('Export Slicer Data to Study...')
      self.exportButton.enabled = False 
      userFrame.layout().addWidget(self.exportButton)
      self.exportButton.connect('clicked()', self.onExportClicked)
      self.tree = slicer.util.findChildren(self.dicomApp, 'TreeView')[0]

    # make the tree view a bit bigger
    self.tree.setMinimumHeight(250)

    if not slicer.dicomDatabase:
      self.promptForDatabaseDirectory()
    else:
      self.onDatabaseDirectoryChanged(self.dicomApp.databaseDirectory)
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
    self.sendButton.connect('clicked()', self.onSendClicked)

    # Add spacer to layout
    self.layout.addStretch(1)

  def onDatabaseDirectoryChanged(self,databaseDirectory):
    if not hasattr(slicer, 'dicomDatabase') or not slicer.dicomDatabase:
      slicer.dicomDatabase = ctk.ctkDICOMDatabase()
    databaseFilepath = databaseDirectory + "/ctkDICOM.sql"
    if not (os.access(databaseDirectory, os.W_OK) and os.access(databaseDirectory, os.R_OK)):
      self.messageBox('The database file path "%s" cannot be opened.' % databaseFilepath)
      return
    slicer.dicomDatabase.openDatabase(databaseDirectory + "/ctkDICOM.sql", "SLICER")
    if not slicer.dicomDatabase.isOpen:
      self.messageBox('The database file path "%s" cannot be opened.' % databaseFilepath)
    if self.dicomApp.databaseDirectory != databaseDirectory:
      self.dicomApp.databaseDirectory = databaseDirectory

  def promptForDatabaseDirectory(self):
    fileDialog = ctk.ctkFileDialog(slicer.util.mainWindow())
    fileDialog.setWindowModality(1)
    fileDialog.setWindowTitle("Select DICOM Database Directory")
    fileDialog.setFileMode(2) # prompt for directory
    fileDialog.connect('fileSelected(QString)', self.onDatabaseDirectoryChanged)
    label = qt.QLabel("<p><p>The Slicer DICOM module stores a local database with an index to all datasets that are <br>pushed to slicer, retrieved from remote dicom servers, or imported.<p>Please select a location for this database where you can store the amounts of data you require.<p>Be sure you have write access to the selected directory.", fileDialog)
    fileDialog.setBottomWidget(label)
    fileDialog.open()

  def onTreeClicked(self,index):
    self.model = index.model()
    self.tree.setExpanded(index, not self.tree.expanded(index))
    self.selection = index.sibling(index.row(), 0)
    typeRole = self.selection.data(self.dicomModelTypeRole)
    if typeRole > 0:
      self.loadButton.text = 'Load Selected %s to Slicer' % self.dicomModelTypes[typeRole]
      self.loadButton.enabled = True
      self.sendButton.enabled = True
    else:
      self.loadButton.text = 'Load to Slicer'
      self.loadButton.enabled = False 
      self.sendButton.enabled = False
    if typeRole:
      self.exportAction.enabled = self.dicomModelTypes[typeRole] == "Study"
    else:
      self.exportAction.enabled = False
    self.previewLabel.text = "Selection: " + self.dicomModelTypes[typeRole]
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


  def onLoadButton(self):
    self.progress = qt.QProgressDialog(slicer.util.mainWindow())
    self.progress.minimumDuration = 0
    self.progress.show()
    self.progress.setValue(0)
    self.progress.setMaximum(100)
    uid = self.selection.data(self.dicomModelUIDRole)
    role = self.dicomModelTypes[self.selection.data(self.dicomModelTypeRole)]
    toLoad = {}
    if role == "Patient":
      self.progress.show()
      self.loadPatient(uid)
    elif role == "Study":
      self.progress.show()
      self.loadStudy(uid)
    elif role == "Series":
      self.loadSeries(uid)
    elif role == "Image":
      pass
    self.progress.close()
    self.progress = None

  def onExportClicked(self):
    """Associate a slicer volume as a series in the selected dicom study"""
    uid = self.selection.data(self.dicomModelUIDRole)
    exportDialog = DICOMLib.DICOMExportDialog(uid,onExportFinished=self.onExportFinished)
    self.dicomApp.suspendModel()
    exportDialog.open()

  def onExportFinished(self):
    self.dicomApp.resumeModel()
    self.dicomApp.resetModel()

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

  def loadPatient(self,patientUID):
    studies = slicer.dicomDatabase.studiesForPatient(patientUID)
    s = 1
    self.progress.setLabelText("Loading Studies")
    self.progress.setValue(1)
    slicer.app.processEvents()
    for study in studies:
      self.progress.setLabelText("Loading Study %d of %d" % (s, len(studies)))
      slicer.app.processEvents()
      s += 1
      self.loadStudy(study)
      if self.progress.wasCanceled:
        break

  def loadStudy(self,studyUID):
    series = slicer.dicomDatabase.seriesForStudy(studyUID)
    s = 1
    origText = self.progress.labelText
    for serie in series:
      self.progress.setLabelText(origText + "\nLoading Series %d of %d" % (s, len(series)))
      slicer.app.processEvents()
      s += 1
      self.progress.setValue(100.*s/len(series))
      self.loadSeries(serie)
      if self.progress.wasCanceled:
        break

  def loadSeries(self,seriesUID):
    files = slicer.dicomDatabase.filesForSeries(seriesUID)
    slicer.dicomDatabase.loadFileHeader(files[0])
    seriesDescription = "0008,103e"
    d = slicer.dicomDatabase.headerValue(seriesDescription)
    try:
      name = d[d.index('[')+1:d.index(']')]
    except ValueError:
      name = "Unknown"
    self.progress.labelText += '\nLoading %s' % name
    slicer.app.processEvents()
    self.loadFiles(slicer.dicomDatabase.filesForSeries(seriesUID), name)

  def loadFiles(self, files, name):
    loader = DICOMLib.DICOMLoader(files,name)
    if not loader.volumeNode:
      qt.QMessageBox.warning(slicer.util.mainWindow(), 'Load', 'Could not load volume for: %s' % name)
      print('Tried to load volume as %s using: ' % name, files)

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
    self.dicomApp.resumeModel()
    self.dicomApp.resetModel()

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
        tmpDir = slicer.app.settings().value('Modules/TemporaryDirectory')
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
    self.mb.open()
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

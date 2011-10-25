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
    parent.title = "DICOM"
    parent.category = "" # top level module
    parent.contributor = "Steve Pieper"
    parent.helpText = """
The DICOM module is a place to experiment a bit with dicom classes from CTK (based on DCMTK).  It is a 'tent' because it is meant to be suitable for explorers, but may not be robust enough for civilized people.

Warning: all data directories are temporary and data may be gone next time you look!
    """
    parent.acknowledgementText = """
This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.  Module implemented by Steve Pieper.  Based on work from CommonTK (http://www.commontk.org).
    """
    self.parent = parent

    if slicer.mrmlScene.GetTagByClassName( "vtkMRMLScriptedModuleNode" ) != 'ScriptedModule':
      slicer.mrmlScene.RegisterNodeClass(vtkMRMLScriptedModuleNode())

    # initialize the dicom infrastructure
    settings = qt.QSettings()
    # the dicom database is a global object for slicer
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
            except UserWarning as message:
              # TODO: how to put this into the error log?
              print ('Problem trying to start DICOMListener:\n %s' % message)
            slicer.dicomListener.start()
            
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
    pass

  def exit(self):
    pass

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

    self.dicomApp = ctk.ctkDICOMAppWidget()
    self.dicomFrame.layout().addWidget(self.dicomApp)
    # hide the search options - doesn't work yet and doesn't fit 
    # well into the frame
    self.findChildren(self.dicomApp, 'SearchOption')[0].hide()
    # make the tree a little smaller to fit in slicer
    self.tree = self.findChildren(self.dicomApp, 'TreeView')[0]
    g = self.tree.geometry
    g.setHeight(150)
    if self.dicomApp.databaseDirectory:
      self.onDatabaseDirectoryChanged(self.dicomApp.databaseDirectory)
    else:
      self.promptForDatabaseDirectory()
    if hasattr(slicer, 'dicomListener'):
      slicer.dicomListener.fileAddedCallback = self.onListenerAddedFile

    self.contextMenu = qt.QMenu(self.tree)
    self.deleteAction = qt.QAction("Delete", self.contextMenu)
    self.contextMenu.addAction(self.deleteAction)
    self.contextMenu.connect('triggered(QAction*)', self.onContextMenuTriggered)

    self.dicomApp.connect('databaseDirectoryChanged(QString)', self.onDatabaseDirectoryChanged)
    self.tree.connect('clicked(const QModelIndex&)', self.onTreeClicked)
    self.tree.setContextMenuPolicy(3)
    self.tree.connect('customContextMenuRequested(QPoint)', self.onTreeContextMenuRequested)

    userFrame = self.findChildren(self.dicomApp, 'UserFrame')[0]
    userFrame.setLayout(qt.QVBoxLayout())
    self.treeLabel = qt.QLabel('Selection: None')
    userFrame.layout().addWidget(self.treeLabel)
    self.loadButton = qt.QPushButton('Load to Slicer')
    self.loadButton.enabled = False 
    userFrame.layout().addWidget(self.loadButton)
    self.loadButton.connect('clicked()', self.onLoadButton)
    self.exportButton = qt.QPushButton('Export Slicer Data to Study...')
    self.exportButton.enabled = False 
    userFrame.layout().addWidget(self.exportButton)
    self.exportButton.connect('clicked()', self.onExportClicked)

    # enable to the Send button of the app widget and take it over
    # for our purposes - TODO: fix this to enable it at the ctkDICOM level
    self.sendButton = slicer.util.findChildren(self.dicomApp, text='Send')[0]
    self.sendButton.enabled = False
    self.sendButton.connect('clicked()', self.onSendClicked)

    # Add spacer to layout
    self.layout.addStretch(1)

  def onDatabaseDirectoryChanged(self,databaseDirectory):
    if not hasattr(slicer, 'dicomDatabase'):
      slicer.dicomDatabase = ctk.ctkDICOMDatabase()
    databaseFilepath = databaseDirectory + "/ctkDICOM.sql"
    if not (os.access(databaseDirectory, os.W_OK) and os.access(databaseDirectory, os.R_OK)):
      self.messageBox('The database file path "%s" cannot be opened.' % databaseFilepath)
      return
    slicer.dicomDatabase.openDatabase(databaseDirectory + "/ctkDICOM.sql", "SLICER")
    if not slicer.dicomDatabase.isOpen:
      self.messageBox('The database file path "%s" cannot be opened.' % databaseFilepath)

  def promptForDatabaseDirectory(self):
    fileDialog = qt.QFileDialog(slicer.util.mainWindow())
    fileDialog.setWindowModality(1)
    fileDialog.setWindowTitle("Select DICOM Database Directory")
    fileDialog.setFileMode(2) # prompt for directory
    fileDialog.connect('fileSelected(QString)', self.onDatabaseDirectoryChanged)
    label = qt.QLabel("<p>The Slicer DICOM module stores a local database with an index to all datasets that are pushed to slicer, retrieved from remote dicom servers, or imported.<p>Please select a location for this database where you can store the amounts of data you require.")
    layout = fileDialog.layout()
    layout.addWidget(label, layout.rowCount(), 1, -1, -1)
    fileDialog.open()

  def onTreeClicked(self,index):
    self.model = index.model()
    self.selection = index.sibling(index.row(), 0)
    typeRole = self.selection.data(self.dicomModelTypeRole)
    self.treeLabel.text = 'Selection: %s' % self.dicomModelTypes[typeRole]
    if typeRole > 0:
      self.loadButton.text = 'Load Selected %s to Slicer' % self.dicomModelTypes[typeRole]
      self.loadButton.enabled = True
      self.sendButton.enabled = True
    else:
      self.loadButton.text = 'Load to Slicer'
      self.loadButton.enabled = False 
      self.sendButton.enabled = False
    self.exportButton.enabled = self.dicomModelTypes[typeRole] == "Study"

  def onTreeContextMenuRequested(self,pos):
    index = self.tree.indexAt(pos)
    self.selection = index.sibling(index.row(), 0)
    self.contextMenu.popup(self.tree.mapToGlobal(pos))

  def onContextMenuTriggered(self,action):
    if action == self.deleteAction:
      typeRole = self.selection.data(self.dicomModelTypeRole)
      role = self.dicomModelTypes[typeRole]
      uid = self.selection.data(self.dicomModelUIDRole)
      if self.okayCancel('Delete to %s: "%s"?' % (role, uid)):
        deleteFiles = False
        if self.question('Delete files in addition to database entries?'):
          # TODO: add delete option to ctkDICOMDatabase
          self.messageBox("Delete not yet implemented")

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
    exportDialog = DICOMExportDialog(uid,onExportFinished=self.onExportFinished)
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
    sendDialog = DICOMSendDialog(files)
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
    self.loadFiles(slicer.dicomDatabase.filesForSeries(seriesUID), name)

  def loadFiles(self, files, name):
    DICOMLib.DICOMLoader(files,name)

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

  def findChildren(self,widget,name):
    """ return a list of child widgets that match the passed name """
    # TODO: figure out why the native QWidget.findChildren method
    # does not seem to work from PythonQt
    children = []
    parents = [widget]
    while parents != []:
      p = parents.pop()
      parents += p.children()
      if name == "" or p.name == name:
        children.append(p)
    return children

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

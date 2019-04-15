import glob
import logging
import os
import qt
import vtk
import ctk
import slicer

from slicer.util import settingsValue, toBool
from slicer.ScriptedLoadableModule import *
import DICOMLib

#
# DICOM
#
# This code includes the GUI for the slicer module.  It is supported
# by the DICOMLib python code which implements most of the logic
# for data exchange and running servers.
#

class DICOM(ScriptedLoadableModule):


  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)

    import string
    self.parent.title = "DICOM"
    self.parent.categories = ["", "Informatics"] # top level module
    self.parent.contributors = ["Steve Pieper (Isomics)"]
    self.parent.helpText = """
The DICOM module integrates DICOM classes from CTK (based on DCMTK).
"""
    self.parent.helpText += self.getDefaultModuleDocumentationLink()
    self.parent.acknowledgementText = """
This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a href=http://www.slicer.org>http://www.slicer.org</a> for details.  Module implemented by Steve Pieper.  Based on work from CommonTK (http://www.commontk.org).
"""
    self.parent.icon = qt.QIcon(':Icons/Medium/SlicerLoadDICOM.png')
    self.parent.dependencies = ["SubjectHierarchy"]


    # Tasks to execute after the application has started up
    slicer.app.connect("startupCompleted()", self.performPostModuleDiscoveryTasks)

  def setup(self):
    pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
    pluginHandlerSingleton.registerPlugin(slicer.qSlicerSubjectHierarchyDICOMPlugin())

  def performPostModuleDiscoveryTasks(self):
    """Since dicom plugins are discovered while the application
    is initialized, they may be found after the DICOM module
    itself if initialized.  This method is tied to a singleShot
    that will be called once the event loop is read to start.
    """

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
          self.startListener()
        if slicer.dicomDatabase:
          slicer.app.setDICOMDatabase(slicer.dicomDatabase)
    # set the dicom pre-cache tags once all plugin classes have been initialized
    DICOMLib.setDatabasePrecacheTags()

    if not slicer.app.commandOptions().noMainWindow:
      # add to the main app file menu
      self.addMenu()
      # add the settings options
      self.settingsPanel = DICOMSettingsPanel()
      slicer.app.settingsDialog().addPanel("DICOM", self.settingsPanel)

  def startListener(self):
    # the dicom listener is also global, but only started on app start if
    # the user so chooses
    settings = qt.QSettings()
    if settings.contains('DICOM/RunListenerAtStart'):
      if settings.value('DICOM/RunListenerAtStart') == 'true':
        if not hasattr(slicer, 'dicomListener'):
          try:
            slicer.dicomListener = DICOMLib.DICOMListener(slicer.dicomDatabase)
            slicer.dicomListener.start()
          except (UserWarning,OSError) as message:
            logging.error('Problem trying to start DICOMListener:\n %s' % message)

  def addMenu(self):
    """Add an action to the File menu that will go into
    the DICOM module by selecting the module.  Note that
    once the module is constructed (below in setup) another
    connection is made that will also cause the instance-created
    DICOM browser to be raised by this menu action"""
    a = self.parent.action()
    fileMenu = slicer.util.lookupTopLevelWidget('FileMenu')
    if fileMenu:
      for action in fileMenu.actions():
        if action.text == 'Save':
          fileMenu.insertAction(action,a)

  def __del__(self):
    if hasattr(slicer, 'dicomListener'):
      logging.debug('trying to stop listener')
      slicer.dicomListener.stop()

class _ui_DICOMSettingsPanel(object):
  def __init__(self, parent):
    vBoxLayout = qt.QVBoxLayout(parent)
    # Add generic settings
    genericGroupBox = ctk.ctkCollapsibleGroupBox()
    genericGroupBox.title = "Generic DICOM settings"
    genericGroupBoxFormLayout = qt.QFormLayout(genericGroupBox)

    loadReferencesComboBox = ctk.ctkComboBox()
    loadReferencesComboBox.toolTip = "Determines whether referenced DICOM series are " \
      "offered when loading DICOM, or the automatic behavior if interaction is disabled. " \
      "Interactive selection of referenced series is the default selection"
    loadReferencesComboBox.addItem("Ask user", qt.QMessageBox.InvalidRole)
    loadReferencesComboBox.addItem("Always", qt.QMessageBox.Yes)
    loadReferencesComboBox.addItem("Never", qt.QMessageBox.No)
    loadReferencesComboBox.currentIndex = 0
    genericGroupBoxFormLayout.addRow("Load referenced series:", loadReferencesComboBox)
    parent.registerProperty(
      "DICOM/automaticallyLoadReferences", loadReferencesComboBox,
      "currentUserDataAsString", str(qt.SIGNAL("currentIndexChanged(int)")))

    schemaUpdateComboBox = ctk.ctkComboBox()
    schemaUpdateComboBox.toolTip = "What do do when the supported schema version is " \
      "different from that of the loaded database"
    schemaUpdateComboBox.addItem("Always update", "AlwaysUpdate")
    schemaUpdateComboBox.addItem("Never update", "NeverUpdate")
    schemaUpdateComboBox.addItem("Ask user", "AskUser")
    schemaUpdateComboBox.currentIndex = 2 # Make 'AskUser' the default as opposed to the CTK default 'AlwaysUpdate'
    genericGroupBoxFormLayout.addRow("Schema update behavior:", schemaUpdateComboBox)
    parent.registerProperty(
      "DICOM/SchemaUpdateOption", schemaUpdateComboBox,
      "currentUserDataAsString", str(qt.SIGNAL("currentIndexChanged(int)")))

    vBoxLayout.addWidget(genericGroupBox)

    # Add settings panel for the plugins
    plugins = slicer.modules.dicomPlugins
    for pluginName in plugins.keys():
      if hasattr(plugins[pluginName], 'settingsPanelEntry'):
        pluginGroupBox = ctk.ctkCollapsibleGroupBox()
        pluginGroupBox.title = pluginName
        vBoxLayout.addWidget(pluginGroupBox)
        plugins[pluginName].settingsPanelEntry(parent, pluginGroupBox)
    vBoxLayout.addStretch(1)


class DICOMSettingsPanel(ctk.ctkSettingsPanel):
  def __init__(self, *args, **kwargs):
    ctk.ctkSettingsPanel.__init__(self, *args, **kwargs)
    self.ui = _ui_DICOMSettingsPanel(self)


# XXX Slicer 4.5 - Remove this. Here only for backward compatibility.
DICOM.setDatabasePrecacheTags = DICOMLib.setDatabasePrecacheTags

#
# Class for avoiding python error that is caused by the method DICOM::setup
# http://www.na-mic.org/Bug/view.php?id=3871
#
class DICOMFileWriter(object):
  def __init__(self, parent):
    pass


#
# DICOM file dialog
#
class DICOMFileDialog(object):
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
    logging.debug('execDialog called on %s' % self)

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

  def validDirectories(self, directoriesToAdd):
    """Return True if the directory names are acceptable for input.
    If path contains non-ASCII characters then they are rejected because
    DICOM module cannot reliable read files form folders that contain
    special characters in the name.
    """
    for directoryName in directoriesToAdd:
      if isinstance(directoryName, str):
        try:
          directoryName.encode('ascii')
        except UnicodeEncodeError:
          # encoding as ascii failed, therefore it was not an ascii string
          return False
      else:
        try:
          directoryName.decode('ascii')
        except UnicodeDecodeError:
          # decoding to ascii failed, therefore it was not an ascii string
          return False
    return True

  def dropEvent(self):
    if not self.validDirectories(self.directoriesToAdd):
      if not slicer.util.confirmYesNoDisplay("Import from folders with special (non-ASCII) characters in the name is not supported."
          " It is recommended to move files into a different folder and retry. Try to import from current location anyway?"):
        self.directoriesToAdd = []
        return

    mainWindow = slicer.util.mainWindow()
    mainWindow.moduleSelector().selectModule('DICOM')
    dicomWidget = slicer.modules.DICOMWidget

    dicomWidget.detailsPopup.dicomBrowser.importDirectories(self.directoriesToAdd)
    self.directoriesToAdd = []

#
# DICOM widget
#

class DICOMWidget(object):
  """
  Slicer module that creates the Qt GUI for interacting with DICOM
  """

  detailWidgetClasses = [DICOMLib.DICOMDetailsWindow, DICOMLib.DICOMDetailsDialog, DICOMLib.DICOMDetailsDock]

  @staticmethod
  def getSavedDICOMDetailsWidgetType(default="window"):
    widgetType = settingsValue('DICOM/BrowserWidgetType', default)
    widgetClass = DICOMWidget.getDetailsWidgetClassForType(widgetType)
    if not widgetClass:
      qt.QSettings().setValue('DICOM/BrowserWidgetType', widgetType)
      widgetClass = DICOMWidget.getDetailsWidgetClassForType(default)
    return widgetClass

  @staticmethod
  def setDICOMDetailsWidgetType(widgetType):
    if not widgetType in DICOMWidget.getAvailableWidgetTypes():
      raise ValueError("Widget type '%s' for DICOMDetails does not exist" % widgetType)
    else:
      qt.QSettings().setValue('DICOM/BrowserWidgetType', widgetType)

  @staticmethod
  def getAvailableWidgetTypes():
    return [c.widgetType for c in DICOMWidget.detailWidgetClasses]

  @staticmethod
  def getDetailsWidgetClassForType(widgetType):
    try:
      return DICOMWidget.detailWidgetClasses[DICOMWidget.getAvailableWidgetTypes().index(widgetType)]
    except (KeyError, ValueError):
      return None

  def __init__(self, parent=None):
    self.testingServer = None

    # state management for compressing events
    # - each time an update is requested, start the singleShot timer
    # - if the update is requested before the timeout, the call to timer.start() resets it
    # - the actual update only happens when the the full time elapses since the last request
    self.updateRecentActivityTimer = qt.QTimer()
    self.updateRecentActivityTimer.setSingleShot(True)
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
    self.onOpenDetailsPopup()

  def exit(self):
    if not self.detailsPopup.browserPersistent:
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
    self.toggleListener.checkable = True
    if hasattr(slicer, 'dicomListener'):
      self.toggleListener.text = "Stop Listener"
      slicer.dicomListener.process.connect('stateChanged(QProcess::ProcessState)',self.onListenerStateChanged)
    else:
      self.toggleListener.text = "Start Listener"
    self.localFrame.layout().addWidget(self.toggleListener)
    self.toggleListener.connect('clicked()', self.onToggleListener)

    self.runListenerAtStart = qt.QCheckBox("Start Listener when Slicer Starts")
    self.localFrame.layout().addWidget(self.runListenerAtStart)
    self.runListenerAtStart.checked = settingsValue('DICOM/RunListenerAtStart', False, converter=toBool)
    self.runListenerAtStart.connect('clicked()', self.onRunListenerAtStart)

    # the Database frame (home of the ctkDICOM widget)
    self.dicomFrame = ctk.ctkCollapsibleButton(self.parent)
    self.dicomFrame.setLayout(qt.QVBoxLayout())
    self.dicomFrame.setText("DICOM Database and Networking")
    self.layout.addWidget(self.dicomFrame)

    self.detailsPopup = self.getSavedDICOMDetailsWidgetType()()

    # XXX Slicer 4.5 - Remove these. Here only for backward compatibility.
    self.dicomBrowser = self.detailsPopup.dicomBrowser
    self.tables = self.detailsPopup.tables

    # connect to the 'Show DICOM Browser' button
    self.showBrowserButton = qt.QPushButton('Show DICOM Browser')
    self.dicomFrame.layout().addWidget(self.showBrowserButton)
    self.showBrowserButton.connect('clicked()', self.onOpenDetailsPopup)

    # connect to the main window's dicom button
    mw = slicer.util.mainWindow()
    if mw:
      try:
        action = slicer.util.findChildren(mw,name='LoadDICOMAction')[0]
        action.connect('triggered()',self.onOpenDetailsPopup)
      except IndexError:
        logging.error('Could not connect to the main window DICOM button')

    if hasattr(slicer, 'dicomListener'):
      slicer.dicomListener.fileToBeAddedCallback = self.onListenerToAddFile
      slicer.dicomListener.fileAddedCallback = self.onListenerAddedFile

    slicer.dicomDatabase.connect('databaseChanged()', self.onDatabaseChanged)

    # the recent activity frame
    self.activityFrame = ctk.ctkCollapsibleButton(self.parent)
    self.activityFrame.setLayout(qt.QVBoxLayout())
    self.activityFrame.setText("Recent DICOM Activity")
    self.layout.addWidget(self.activityFrame)

    self.recentActivity = DICOMLib.DICOMRecentActivityWidget(self.activityFrame,detailsPopup=self.detailsPopup)
    self.activityFrame.layout().addWidget(self.recentActivity)
    self.requestUpdateRecentActivity()


    # Add spacer to layout
    self.layout.addStretch(1)

  def onOpenDetailsPopup(self):
    if not isinstance(self.detailsPopup, self.getSavedDICOMDetailsWidgetType()):
      self.detailsPopup = self.getSavedDICOMDetailsWidgetType()()
    self.detailsPopup.open()

  def onDatabaseChanged(self):
    """Use this because to update the view in response to things
    like database inserts.  Ideally the model would do this
    directly based on signals from the SQLite database, but
    that is not currently available.
    https://bugreports.qt-project.org/browse/QTBUG-10775
    """
    self.requestUpdateRecentActivity()

  def requestUpdateRecentActivity(self):
    """This method serves to compress the requests for updating
    the recent activity widget since it is time consuming and there can be
    many of them coming in a rapid sequence when the
    database is active"""
    self.updateRecentActivityTimer.start()

  def onUpateRecentActivityRequestTimeout(self):
    self.recentActivity.update()

  def onToggleListener(self):
    self.toggleListener.checked = False
    if hasattr(slicer, 'dicomListener'):
      slicer.dicomListener.stop()
      del slicer.dicomListener
    else:
      try:
        dicomListener = DICOMLib.DICOMListener(database=slicer.dicomDatabase)
        dicomListener.start()
        if dicomListener.process:
          self.onListenerStateChanged(dicomListener.process.state())
          dicomListener.process.connect('stateChanged(QProcess::ProcessState)',self.onListenerStateChanged)
          dicomListener.fileToBeAddedCallback = self.onListenerToAddFile
          dicomListener.fileAddedCallback = self.onListenerAddedFile
          slicer.dicomListener = dicomListener
      except UserWarning as message:
        slicer.util.warningDisplay("Could not start listener:\n %s" % message, windowTitle="DICOM")

  def onListenerStateChanged(self,newState):
    """ Called when the indexer process state changes
    so we can provide feedback to the user
    """
    if newState == 0:
      slicer.util.showStatusMessage("DICOM Listener not running")
      self.toggleListener.text = "Start Listener"
      self.toggleListener.checked = False
      del slicer.dicomListener
    if newState == 1:
      slicer.util.showStatusMessage("DICOM Listener starting")
    if newState == 2:
      slicer.util.showStatusMessage("DICOM Listener running")
      self.toggleListener.text = "Stop Listener"
      self.toggleListener.checked = True

  def onListenerToAddFile(self):
    """ Called when the indexer is about to add a file to the database.
    Works around issue where ctkDICOMModel has open queries that keep the
    database locked.
    """

  def onListenerAddedFile(self):
    """Called after the listener has added a file.
    Restore and refresh the app model
    """
    newFile = slicer.dicomListener.lastFileAdded
    if newFile:
      slicer.util.showStatusMessage("Loaded: %s" % newFile, 1000)

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
      #self.toggleServer.text = "Stop Testing Server"

  def onRunListenerAtStart(self):
    settings = qt.QSettings()
    settings.setValue('DICOM/RunListenerAtStart', self.runListenerAtStart.checked)

  def onDatabaseDirectoryChanged(self,databaseDirectory):
    # XXX Slicer 4.5 - Remove this function. Was here only for backward compatibility.
    self.detailsPopup.onDatabaseDirectoryChanged(databaseDirectory)

  def messageBox(self,text,title='DICOM'):
    # XXX Slicer 4.5 - Remove this function. Was here only for backward compatibility.
    #                  Instead, slicer.util.warningDisplay() should be used.
    slicer.util.warningDisplay(text, windowTitle=title)

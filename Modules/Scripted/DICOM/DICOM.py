import glob
import logging
import os

import ctk
import qt

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

        self.parent.title = "DICOM"
        self.parent.categories = ["", "Informatics"]  # top level module
        self.parent.contributors = ["Steve Pieper (Isomics)", "Andras Lasso (PerkLab)"]
        self.parent.helpText = """
This module allows importing, loading, and exporting DICOM files, and sending receiving data using DICOM networking.
"""
        self.parent.helpText += self.getDefaultModuleDocumentationLink()
        self.parent.acknowledgementText = """
This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community.
"""
        self.parent.icon = qt.QIcon(':Icons/Medium/SlicerLoadDICOM.png')
        self.parent.dependencies = ["SubjectHierarchy"]

        self.viewWidget = None  # Widget used in the layout manager (contains just label and browser widget)
        self.browserWidget = None  # SlicerDICOMBrowser instance (ctkDICOMBrowser with additional section for loading the selected items)
        self.browserSettingsWidget = None
        self.currentViewArrangement = 0
        # This variable is set to true if we temporarily
        # hide the data probe (and so we need to restore its visibility).
        self.dataProbeHasBeenTemporarilyHidden = False

        self.postModuleDiscoveryTasksPerformed = False

    def setup(self):
        # Tasks to execute after the application has started up
        slicer.app.connect("startupCompleted()", self.performPostModuleDiscoveryTasks)
        slicer.app.connect("urlReceived(QString)", self.onURLReceived)

        pluginHandlerSingleton = slicer.qSlicerSubjectHierarchyPluginHandler.instance()
        pluginHandlerSingleton.registerPlugin(slicer.qSlicerSubjectHierarchyDICOMPlugin())

        self.viewFactory = slicer.qSlicerSingletonViewFactory()
        self.viewFactory.setTagName("dicombrowser")
        if slicer.app.layoutManager() is not None:
            slicer.app.layoutManager().registerViewFactory(self.viewFactory)

    def performPostModuleDiscoveryTasks(self):
        """Since dicom plugins are discovered while the application
        is initialized, they may be found after the DICOM module
        itself if initialized.  This method is tied to a singleShot
        that will be called once the event loop is ready to start.
        """

        if self.postModuleDiscoveryTasksPerformed:
            return
        self.postModuleDiscoveryTasksPerformed = True

        if slicer.mrmlScene.GetTagByClassName("vtkMRMLScriptedModuleNode") != 'ScriptedModule':
            slicer.mrmlScene.RegisterNodeClass(vtkMRMLScriptedModuleNode())

        self.initializeDICOMDatabase()

        settings = qt.QSettings()
        if settings.contains('DICOM/RunListenerAtStart') and not slicer.app.commandOptions().testingEnabled:
            if settings.value('DICOM/RunListenerAtStart') == 'true':
                self.startListener()

        if not slicer.app.commandOptions().noMainWindow:
            slicer.util.mainWindow().findChild(qt.QAction, "LoadDICOMAction").setVisible(True)
            # add to the main app file menu
            self.addMenu()
            # add the settings options
            self.settingsPanel = DICOMSettingsPanel()
            slicer.app.settingsDialog().addPanel("DICOM", self.settingsPanel)

            layoutManager = slicer.app.layoutManager()
            layoutManager.layoutChanged.connect(self.onLayoutChanged)
            layout = (
                "<layout type=\"horizontal\">"
                " <item>"
                "  <dicombrowser></dicombrowser>"
                " </item>"
                "</layout>"
            )
            layoutNode = slicer.app.layoutManager().layoutLogic().GetLayoutNode()
            layoutNode.AddLayoutDescription(
                slicer.vtkMRMLLayoutNode.SlicerLayoutDicomBrowserView, layout)
            self.currentViewArrangement = layoutNode.GetViewArrangement()
            self.previousViewArrangement = layoutNode.GetViewArrangement()

            slicer.app.moduleManager().connect(
                'moduleAboutToBeUnloaded(QString)', self._onModuleAboutToBeUnloaded)

    def onURLReceived(self, urlString):
        """Process DICOM view requests.

        Required query path: viewer/
        Query parameters:
          - studyUID: study instance UID (required)
          - dicomweb_endpoint: DICOMweb server address (required)
          - access_token: token for accessing the server (optional)
          - bulk_retrieve: set to 0 for slower retrieve but better compatibility
            with simple servers (optional, valid values are 0 and 1, default 1)

        Example 1 (server supports bulk retrieve):

        slicer://viewer/?studyUID=2.16.840.1.113669.632.20.121711.10000158860
          &access_token=k0zR6WAPpNbVguQ8gGUHp6
          &dicomweb_endpoint=http%3A%2F%2Fdemo.kheops.online%2Fapi
          &dicomweb_uri_endpoint=%20http%3A%2F%2Fdemo.kheops.online%2Fapi%2Fwado

        Example 2 (server does not support bulk retrieve):

        slicer://viewer/?studyUID=1.2.826.0.1.3680043.8.498.77209180964150541470378654317482622226&dicomweb_endpoint=http%3A%2F%2F130.15.7.119:2016%2Fdicom&bulk_retrieve=0

        """

        url = qt.QUrl(urlString)
        if (url.authority().lower() != "viewer"):
            logging.debug("DICOM module ignores non-viewer URL: " + urlString)
            return
        query = qt.QUrlQuery(url)
        queryMap = {}
        for key, value in query.queryItems(qt.QUrl.FullyDecoded):
            queryMap[key] = qt.QUrl.fromPercentEncoding(value)

        if "dicomweb_endpoint" not in queryMap:
            logging.debug("DICOM module ignores URL without dicomweb_endpoint query parameter: " + urlString)
            return
        if "studyUID" not in queryMap:
            logging.debug("DICOM module ignores URL without studyUID query parameter: " + urlString)
            return

        bulkRetrieve = True
        if "bulk_retrieve" in queryMap:
            bulkRetrieve = int(queryMap["bulk_retrieve"]) > 0

        logging.info("DICOM module received URL: " + urlString)

        accessToken = None
        if "access_token" in queryMap:
            accessToken = queryMap["access_token"]

        slicer.util.selectModule("DICOM")
        slicer.app.processEvents()
        from DICOMLib import DICOMUtils
        importedSeriesInstanceUIDs = DICOMUtils.importFromDICOMWeb(
            dicomWebEndpoint=queryMap["dicomweb_endpoint"],
            studyInstanceUID=queryMap["studyUID"],
            accessToken=accessToken,
            bulkRetrieve=bulkRetrieve)

        # Select newly loaded items to make it easier to load them
        self.browserWidget.dicomBrowser.setSelectedItems(ctk.ctkDICOMModel.SeriesType, importedSeriesInstanceUIDs)

    def initializeDICOMDatabase(self):
        #  Create alias for convenience
        slicer.dicomDatabase = slicer.app.dicomDatabase()

        # Set the dicom pre-cache tags once all plugin classes have been initialized.
        # Pre-caching tags is very important for fast DICOM loading because tags that are
        # not pre-cached during DICOM import in bulk, will be cached during Examine step one-by-one
        # (which takes magnitudes more time).
        tagsToPrecache = list(slicer.dicomDatabase.tagsToPrecache)
        for pluginClass in slicer.modules.dicomPlugins:
            plugin = slicer.modules.dicomPlugins[pluginClass]()
            tagsToPrecache += list(plugin.tags.values())
        tagsToPrecache = sorted(set(tagsToPrecache))  # remove duplicates
        slicer.dicomDatabase.tagsToPrecache = tagsToPrecache

        # Try to initialize the database using the location stored in settings
        if slicer.app.commandOptions().testingEnabled:
            # For automatic tests (use a separate DICOM database for testing)
            slicer.dicomDatabaseDirectorySettingsKey = 'DatabaseDirectoryTest_' + ctk.ctkDICOMDatabase().schemaVersion()
            databaseDirectory = os.path.join(slicer.app.temporaryPath,
                                             'temp' + slicer.app.applicationName + 'DICOMDatabase_' + ctk.ctkDICOMDatabase().schemaVersion())
        else:
            # For production
            slicer.dicomDatabaseDirectorySettingsKey = 'DatabaseDirectory_' + ctk.ctkDICOMDatabase().schemaVersion()
            settings = qt.QSettings()
            databaseDirectory = settings.value(slicer.dicomDatabaseDirectorySettingsKey)
            if not databaseDirectory:
                documentsLocation = qt.QStandardPaths.DocumentsLocation
                documents = qt.QStandardPaths.writableLocation(documentsLocation)
                databaseDirectory = os.path.join(documents, slicer.app.applicationName + "DICOMDatabase")
                settings.setValue(slicer.dicomDatabaseDirectorySettingsKey, databaseDirectory)

        # Attempt to open the database. If it fails then user will have to configure it using DICOM module.
        databaseFileName = databaseDirectory + "/ctkDICOM.sql"
        slicer.dicomDatabase.openDatabase(databaseFileName)
        if slicer.dicomDatabase.isOpen:
            # There is an existing database at the current location
            if slicer.dicomDatabase.schemaVersionLoaded() != slicer.dicomDatabase.schemaVersion():
                # Schema does not match, do not use it
                slicer.dicomDatabase.closeDatabase()

    def startListener(self):

        if not slicer.dicomDatabase.isOpen:
            logging.error("Failed to start DICOM listener. DICOM database is not open.")
            return False

        if not hasattr(slicer, 'dicomListener'):
            dicomListener = DICOMLib.DICOMListener(slicer.dicomDatabase)

        try:
            dicomListener.start()
        except (UserWarning, OSError) as message:
            logging.error('Problem trying to start DICOM listener:\n %s' % message)
            return False
        if not dicomListener.process:
            logging.error("Failed to start DICOM listener. Process start failed.")
            return False
        slicer.dicomListener = dicomListener
        logging.info("DICOM C-Store SCP service started at port " + str(slicer.dicomListener.port))

    def stopListener(self):
        if hasattr(slicer, 'dicomListener'):
            logging.info("DICOM C-Store SCP service stopping")
            slicer.dicomListener.stop()
            del slicer.dicomListener

    def addMenu(self):
        """Add an action to the File menu that will go into
        the DICOM module by selecting the module.  Note that
        once the module is constructed (below in setup) another
        connection is made that will also cause the instance-created
        DICOM browser to be raised by this menu action"""
        a = self.parent.action()
        a.setText(a.tr("Add DICOM Data"))
        fileMenu = slicer.util.lookupTopLevelWidget('FileMenu')
        if fileMenu:
            for child in fileMenu.children():
                if child.objectName == "RecentlyLoadedMenu":
                    fileMenu.insertAction(child.menuAction(), a)  # insert action before RecentlyLoadedMenu

    def setBrowserWidgetInDICOMLayout(self, browserWidget):
        """Set DICOM browser widget in the custom view layout"""
        if self.browserWidget == browserWidget:
            return

        if self.browserWidget is not None:
            self.browserWidget.closed.disconnect(self.onBrowserWidgetClosed)

        oldBrowserWidget = self.browserWidget
        self.browserWidget = browserWidget
        self.browserWidget.setAutoFillBackground(True)
        if slicer.util.mainWindow():
            # For some reason, we cannot disconnect this event connection if
            # main window, and not disconnecting would cause crash on application shutdown,
            # so we only connect when main window is present.
            self.browserWidget.closed.connect(self.onBrowserWidgetClosed)

        if self.viewWidget is None:
            self.viewWidget = qt.QWidget()
            self.viewWidget.setAutoFillBackground(True)
            self.viewFactory.setWidget(self.viewWidget)
            layout = qt.QVBoxLayout()
            self.viewWidget.setLayout(layout)

            label = qt.QLabel("DICOM database")
            label.setSizePolicy(qt.QSizePolicy.Expanding, qt.QSizePolicy.Fixed)
            layout.addWidget(label)
            font = qt.QFont()
            font.setBold(True)
            font.setPointSize(12)
            label.setFont(font)

        if oldBrowserWidget is not None:
            self.viewWidget.layout().removeWidget(oldBrowserWidget)

        if self.browserWidget:
            self.viewWidget.layout().addWidget(self.browserWidget)

    def onLayoutChanged(self, viewArrangement):
        if viewArrangement == self.currentViewArrangement:
            return

        if (self.currentViewArrangement != slicer.vtkMRMLLayoutNode.SlicerLayoutNone and
                self.currentViewArrangement != slicer.vtkMRMLLayoutNode.SlicerLayoutDicomBrowserView):
            self.previousViewArrangement = self.currentViewArrangement
        self.currentViewArrangement = viewArrangement

        if self.browserWidget is None:
            return
        mw = slicer.util.mainWindow()
        dataProbe = mw.findChild("QWidget", "DataProbeCollapsibleWidget") if mw else None
        if self.currentViewArrangement == slicer.vtkMRMLLayoutNode.SlicerLayoutDicomBrowserView:
            # View has been changed to the DICOM browser view
            self.browserWidget.show()
            # If we are in DICOM module, hide the Data Probe to have more space for the module
            try:
                inDicomModule = slicer.modules.dicom.widgetRepresentation().isEntered
            except AttributeError:
                # Slicer is shutting down
                inDicomModule = False
            if inDicomModule and dataProbe and dataProbe.isVisible():
                dataProbe.setVisible(False)
                self.dataProbeHasBeenTemporarilyHidden = True
        else:
            # View has been changed from the DICOM browser view
            if self.dataProbeHasBeenTemporarilyHidden:
                # DataProbe was temporarily hidden, restore its visibility now
                dataProbe.setVisible(True)
                self.dataProbeHasBeenTemporarilyHidden = False

    def onBrowserWidgetClosed(self):
        if (self.currentViewArrangement != slicer.vtkMRMLLayoutNode.SlicerLayoutDicomBrowserView and
                self.currentViewArrangement != slicer.vtkMRMLLayoutNode.SlicerLayoutNone):
            # current layout is a valid layout that is not the DICOM browser view, so nothing to do
            return

        layoutId = self.previousViewArrangement

        # Use a default layout if this layout is not valid
        if (layoutId == slicer.vtkMRMLLayoutNode.SlicerLayoutNone
                or layoutId == slicer.vtkMRMLLayoutNode.SlicerLayoutDicomBrowserView):
            layoutId = qt.QSettings().value("MainWindow/layout", slicer.vtkMRMLLayoutNode.SlicerLayoutInitialView)

        slicer.app.layoutManager().setLayout(layoutId)

    def _onModuleAboutToBeUnloaded(self, moduleName):
        # Application is shutting down. Stop the listener.
        if moduleName == "DICOM":
            self.stopListener()
            slicer.app.moduleManager().disconnect(
                'moduleAboutToBeUnloaded(QString)', self._onModuleAboutToBeUnloaded)


class _ui_DICOMSettingsPanel:
    def __init__(self, parent):
        vBoxLayout = qt.QVBoxLayout(parent)
        # Add generic settings
        genericGroupBox = ctk.ctkCollapsibleGroupBox()
        genericGroupBox.title = "Generic DICOM settings"
        genericGroupBoxFormLayout = qt.QFormLayout(genericGroupBox)

        directoryButton = ctk.ctkDirectoryButton()
        genericGroupBoxFormLayout.addRow("Database location:", directoryButton)
        parent.registerProperty(slicer.dicomDatabaseDirectorySettingsKey, directoryButton,
                                "directory", str(qt.SIGNAL("directoryChanged(QString)")),
                                "DICOM general settings", ctk.ctkSettingsPanel.OptionRequireRestart)
        # Restart is forced because no mechanism is implemented that would reopen the DICOM database after
        # folder location is changed. It is easier to restart the application than implementing an update
        # mechanism.

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

        detailedLoggingCheckBox = qt.QCheckBox()
        detailedLoggingCheckBox.toolTip = ("Log more details during DICOM operations."
                                           " Useful for investigating DICOM loading issues but may impact performance.")
        genericGroupBoxFormLayout.addRow("Detailed logging:", detailedLoggingCheckBox)
        detailedLoggingMapper = ctk.ctkBooleanMapper(detailedLoggingCheckBox, "checked", str(qt.SIGNAL("toggled(bool)")))
        parent.registerProperty(
            "DICOM/detailedLogging", detailedLoggingMapper,
            "valueAsInt", str(qt.SIGNAL("valueAsIntChanged(int)")))

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


#
# DICOM file dialog
#
class DICOMFileDialog:
    """This specially named class is detected by the scripted loadable
    module and is the target for optional drag and drop operations.
    See: Base/QTGUI/qSlicerScriptedFileDialog.h
    and commit http://svn.slicer.org/Slicer4/trunk@21951 and issue #3081
    """

    def __init__(self, qSlicerFileDialog):
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
        self.directoriesToAdd, _ = DICOMFileDialog.pathsFromMimeData(self.qSlicerFileDialog.mimeData())
        self.qSlicerFileDialog.acceptMimeData(len(self.directoriesToAdd) != 0)

    @staticmethod
    def pathsFromMimeData(mimeData):
        directoriesToAdd = []
        filesToAdd = []
        if mimeData.hasFormat('text/uri-list'):
            urls = mimeData.urls()
            for url in urls:
                localPath = url.toLocalFile()  # convert QUrl to local path
                pathInfo = qt.QFileInfo()
                pathInfo.setFile(localPath)  # information about the path
                if pathInfo.isDir():  # if it is a directory we add the files to the dialog
                    directoriesToAdd.append(localPath)
                else:
                    filesToAdd.append(localPath)
        return directoriesToAdd, filesToAdd

    @staticmethod
    def isAscii(s):
        """Return True if string only contains ASCII characters.
        """
        if isinstance(s, str):
            try:
                s.encode('ascii')
            except UnicodeEncodeError:
                # encoding as ascii failed, therefore it was not an ascii string
                return False
        else:
            try:
                s.decode('ascii')
            except UnicodeDecodeError:
                # decoding to ascii failed, therefore it was not an ascii string
                return False
        return True

    @staticmethod
    def validDirectories(directoriesToAdd):
        """Return True if the directory names are acceptable for input.
        If path contains non-ASCII characters then they are rejected because
        DICOM module cannot reliable read files form folders that contain
        special characters in the name.
        """
        if slicer.app.isCodePageUtf8():
            return True

        for directoryName in directoriesToAdd:
            for root, dirs, files in os.walk(directoryName):
                if not DICOMFileDialog.isAscii(root):
                    return False
                for name in files:
                    if not DICOMFileDialog.isAscii(name):
                        return False
                for name in dirs:
                    if not DICOMFileDialog.isAscii(name):
                        return False

        return True

    @staticmethod
    def createDefaultDatabase():
        """If DICOM database is invalid then try to create a default one. If fails then show an error message.
        This method should only be used when user initiates DICOM import on the GUI, because the error message is
        shown in a popup, which would block execution of auomated processing scripts.
        Returns True if a valid DICOM database is available (has been created successfully or it was already available).
        """
        if slicer.dicomDatabase and slicer.dicomDatabase.isOpen:
            # Valid DICOM database already exists
            return True

        # Try to create a database with default settings
        if slicer.modules.DICOMInstance.browserWidget is None:
            slicer.util.selectModule('DICOM')
        slicer.modules.DICOMInstance.browserWidget.dicomBrowser.createNewDatabaseDirectory()
        if slicer.dicomDatabase and slicer.dicomDatabase.isOpen:
            # DICOM database created successfully
            return True

        # Failed to create database
        # Make sure the browser is visible then display error message
        slicer.util.selectModule('DICOM')
        slicer.modules.dicom.widgetRepresentation().self().onOpenBrowserWidget()
        slicer.util.warningDisplay("Could not create a DICOM database with default settings. Please create a new database or"
                                   " update the existing incompatible database using options shown in DICOM browser.")
        return False

    def dropEvent(self):
        if not DICOMFileDialog.createDefaultDatabase():
            return

        if not DICOMFileDialog.validDirectories(self.directoriesToAdd):
            if not slicer.util.confirmYesNoDisplay("Import of files that have special (non-ASCII) characters in their names is not supported."
                                                   " It is recommended to move files into a different folder and retry. Try to import from current location anyway?"):
                self.directoriesToAdd = []
                return

        slicer.util.selectModule('DICOM')
        slicer.modules.DICOMInstance.browserWidget.dicomBrowser.importDirectories(self.directoriesToAdd)
        self.directoriesToAdd = []


class DICOMLoadingByDragAndDropEventFilter(qt.QWidget):
    """This event filter is used for overriding drag-and-drop behavior while
    the DICOM module is active. To simplify DICOM import, while DICOM module is active
    then files or folders that are drag-and-dropped to the application window
    are always interpreted as DICOM data.
    """

    def eventFilter(self, object, event):
        """
        Custom event filter for Slicer Main Window.

        Inputs: Object (QObject), Event (QEvent)
        """
        if event.type() == qt.QEvent.DragEnter:
            self.dragEnterEvent(event)
            return True
        if event.type() == qt.QEvent.Drop:
            self.dropEvent(event)
            return True
        return False

    def dragEnterEvent(self, event):
        """
        Actions to do when a drag enter event occurs in the Main Window.

        Read up on https://doc.qt.io/qt-5.12/dnd.html#dropping
        Input: Event (QEvent)
        """
        self.directoriesToAdd, self.filesToAdd = DICOMFileDialog.pathsFromMimeData(event.mimeData())
        if self.directoriesToAdd or self.filesToAdd:
            event.acceptProposedAction()  # allows drop event to proceed
        else:
            event.ignore()

    def dropEvent(self, event):
        if not DICOMFileDialog.createDefaultDatabase():
            return

        if not DICOMFileDialog.validDirectories(self.directoriesToAdd) or not DICOMFileDialog.validDirectories(self.filesToAdd):
            if not slicer.util.confirmYesNoDisplay("Import from folders with special (non-ASCII) characters in the name is not supported."
                                                   " It is recommended to move files into a different folder and retry. Try to import from current location anyway?"):
                self.directoriesToAdd = []
                return

        slicer.modules.DICOMInstance.browserWidget.dicomBrowser.importDirectories(self.directoriesToAdd)
        slicer.modules.DICOMInstance.browserWidget.dicomBrowser.importFiles(self.filesToAdd)


#
# DICOM widget
#

class DICOMWidget(ScriptedLoadableModuleWidget):
    """
    Slicer module that creates the Qt GUI for interacting with DICOM
    """

    # sets up the widget
    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)

        # If DICOM module is the startup module then this widget will be shown
        # before startup completes, therefore we need to ensure here that
        # module discovery happens before proceeding.
        slicer.modules.DICOMInstance.performPostModuleDiscoveryTasks()

        # This module is often used in developer mode, therefore
        # collapse reload & test section by default.
        if hasattr(self, "reloadCollapsibleButton"):
            self.reloadCollapsibleButton.collapsed = True

        self.dragAndDropEventFilter = DICOMLoadingByDragAndDropEventFilter()

        globals()['d'] = self

        self.testingServer = None
        self.browserWidget = None

        # Load widget from .ui file (created by Qt Designer)
        uiWidget = slicer.util.loadUI(self.resourcePath('UI/DICOM.ui'))
        self.layout.addWidget(uiWidget)
        self.ui = slicer.util.childWidgetVariables(uiWidget)

        self.browserWidget = DICOMLib.SlicerDICOMBrowser()
        self.browserWidget.objectName = 'SlicerDICOMBrowser'

        slicer.modules.DICOMInstance.setBrowserWidgetInDICOMLayout(self.browserWidget)

        layoutManager = slicer.app.layoutManager()
        if layoutManager is not None:
            layoutManager.layoutChanged.connect(self.onLayoutChanged)
            viewArrangement = slicer.app.layoutManager().layoutLogic().GetLayoutNode().GetViewArrangement()
            self.ui.showBrowserButton.checked = (viewArrangement == slicer.vtkMRMLLayoutNode.SlicerLayoutDicomBrowserView)

        # connect 'Import DICOM files' and 'Show DICOM Browser' button
        self.ui.showBrowserButton.connect('clicked()', self.toggleBrowserWidget)
        self.ui.importButton.connect('clicked()', self.importFolder)

        # Add import options menu to import button

        importButtonMenu = qt.QMenu("Import options", self.ui.importButton)
        importButtonMenu.toolTipsVisible = True
        self.ui.importButton.setMenu(importButtonMenu)
        importButtonMenu.connect('aboutToShow()', self.aboutToShowImportOptionsMenu)

        self.copyOnImportAction = qt.QAction("Copy imported files to DICOM database", importButtonMenu)
        self.copyOnImportAction.setToolTip("If enabled, all imported files are copied into the DICOM database."
                                           " This is useful when importing from removable drives.")
        self.copyOnImportAction.setCheckable(True)
        importButtonMenu.addAction(self.copyOnImportAction)
        self.copyOnImportAction.connect('toggled(bool)', self.copyOnImportToggled)

        #
        # DICOM networking
        #

        self.ui.networkingFrame.collapsed = True
        self.ui.queryServerButton.connect('clicked()', self.browserWidget.dicomBrowser, "openQueryDialog()")

        self.ui.toggleListener.connect('toggled(bool)', self.onToggleListener)

        settings = qt.QSettings()
        self.ui.runListenerAtStart.checked = settingsValue('DICOM/RunListenerAtStart', False, converter=toBool)
        self.ui.runListenerAtStart.connect('toggled(bool)', self.onRunListenerAtStart)

        # Testing server - not exposed (used for development)

        self.toggleServer = qt.QPushButton("Start Testing Server")
        self.ui.networkingFrame.layout().addWidget(self.toggleServer)
        self.toggleServer.connect('clicked()', self.onToggleServer)

        self.verboseServer = qt.QCheckBox("Verbose")
        self.ui.networkingFrame.layout().addWidget(self.verboseServer)

        # advanced options - not exposed to end users
        # developers can uncomment these lines to access testing server
        self.toggleServer.hide()
        self.verboseServer.hide()

        #
        # Browser settings
        #

        self.ui.browserSettingsFrame.collapsed = True

        self.updateDatabaseDirectoryFromBrowser(self.browserWidget.dicomBrowser.databaseDirectory)
        # Synchronize database selection between browser and this widget
        self.ui.directoryButton.directoryChanged.connect(self.updateDatabaseDirectoryFromWidget)
        self.ui.directoryButton.sizePolicy = qt.QSizePolicy(qt.QSizePolicy.Ignored, qt.QSizePolicy.Fixed)
        self.browserWidget.dicomBrowser.databaseDirectoryChanged.connect(self.updateDatabaseDirectoryFromBrowser)

        self.ui.browserAutoHideCheckBox.checked = not settingsValue('DICOM/BrowserPersistent', False, converter=toBool)
        self.ui.browserAutoHideCheckBox.stateChanged.connect(self.onBrowserAutoHideStateChanged)

        self.ui.repairDatabaseButton.connect('clicked()', self.browserWidget.dicomBrowser, "onRepairAction()")
        self.ui.clearDatabaseButton.connect('clicked()', self.onClearDatabase)

        # connect to the main window's dicom button
        mw = slicer.util.mainWindow()
        if mw:
            try:
                action = slicer.util.findChildren(mw, name='LoadDICOMAction')[0]
                action.connect('triggered()', self.onOpenBrowserWidget)
            except IndexError:
                logging.error('Could not connect to the main window DICOM button')

        self.databaseRefreshRequestTimer = qt.QTimer()
        self.databaseRefreshRequestTimer.setSingleShot(True)
        # If not receiving new file for 2 seconds then a database update is triggered.
        self.databaseRefreshRequestTimer.setInterval(2000)
        self.databaseRefreshRequestTimer.connect('timeout()', self.requestDatabaseRefresh)

        #
        # DICOM Plugins selection widget
        #
        self.ui.dicomPluginsFrame.collapsed = True
        self.pluginSelector = DICOMLib.DICOMPluginSelector(self.ui.dicomPluginsFrame)
        self.ui.dicomPluginsFrame.layout().addWidget(self.pluginSelector)
        self.checkBoxByPlugins = []

        for pluginClass in slicer.modules.dicomPlugins:
            self.checkBox = self.pluginSelector.checkBoxByPlugin[pluginClass]
            self.checkBox.connect('stateChanged(int)', self.onPluginStateChanged)
            self.checkBoxByPlugins.append(self.checkBox)

    def onPluginStateChanged(self, state):
        settings = qt.QSettings()
        settings.beginWriteArray('DICOM/disabledPlugins')

        for key in settings.allKeys():
            settings.remove(key)

        plugins = self.pluginSelector.selectedPlugins()
        arrayIndex = 0
        for pluginClass in slicer.modules.dicomPlugins:
            if pluginClass not in plugins:
                settings.setArrayIndex(arrayIndex)
                settings.setValue(pluginClass, 'disabled')
                arrayIndex += 1

        settings.endArray()

    def enter(self):
        self.onOpenBrowserWidget()
        self.addListenerObservers()
        self.onListenerStateChanged()
        # While DICOM module is active, drag-and-drop always performs DICOM import
        mw = slicer.util.mainWindow()
        if mw:
            mw.installEventFilter(self.dragAndDropEventFilter)

    def exit(self):
        mw = slicer.util.mainWindow()
        if mw:
            mw.removeEventFilter(self.dragAndDropEventFilter)
        self.removeListenerObservers()
        self.browserWidget.close()

    def addListenerObservers(self):
        if not hasattr(slicer, 'dicomListener'):
            return
        if slicer.dicomListener.process is not None:
            slicer.dicomListener.process.connect('stateChanged(QProcess::ProcessState)', self.onListenerStateChanged)
        slicer.dicomListener.fileToBeAddedCallback = self.onListenerToAddFile
        slicer.dicomListener.fileAddedCallback = self.onListenerAddedFile

    def removeListenerObservers(self):
        if not hasattr(slicer, 'dicomListener'):
            return
        if slicer.dicomListener.process is not None:
            slicer.dicomListener.process.disconnect('stateChanged(QProcess::ProcessState)', self.onListenerStateChanged)
        slicer.dicomListener.fileToBeAddedCallback = None
        slicer.dicomListener.fileAddedCallback = None

    def updateGUIFromMRML(self, caller, event):
        pass

    def onLayoutChanged(self, viewArrangement):
        self.ui.showBrowserButton.checked = (viewArrangement == slicer.vtkMRMLLayoutNode.SlicerLayoutDicomBrowserView)

    def onCurrentItemChanged(self, id):
        plugin = slicer.qSlicerSubjectHierarchyPluginHandler.instance().getOwnerPluginForSubjectHierarchyItem(id)
        if not plugin:
            self.subjectHierarchyCurrentVisibility = False
            return
        self.subjectHierarchyCurrentVisibility = plugin.getDisplayVisibility(id)

    def onCurrentItemModified(self, id):
        oldSubjectHierarchyCurrentVisibility = self.subjectHierarchyCurrentVisibility

        plugin = slicer.qSlicerSubjectHierarchyPluginHandler.instance().getOwnerPluginForSubjectHierarchyItem(id)
        if not plugin:
            self.subjectHierarchyCurrentVisibility = False
        else:
            self.subjectHierarchyCurrentVisibility = plugin.getDisplayVisibility(id)

        if self.browserWidget is None:
            return

        if (oldSubjectHierarchyCurrentVisibility != self.subjectHierarchyCurrentVisibility and
                self.subjectHierarchyCurrentVisibility):
            self.browserWidget.close()

    def toggleBrowserWidget(self):
        if self.ui.showBrowserButton.checked:
            self.onOpenBrowserWidget()
        else:
            if self.browserWidget:
                self.browserWidget.close()

    def aboutToShowImportOptionsMenu(self):
        self.copyOnImportAction.checked = (self.browserWidget.dicomBrowser.ImportDirectoryMode == ctk.ctkDICOMBrowser.ImportDirectoryCopy)

    def copyOnImportToggled(self, copyOnImport):
        if self.copyOnImportAction.checked:
            self.browserWidget.dicomBrowser.ImportDirectoryMode = ctk.ctkDICOMBrowser.ImportDirectoryCopy
        else:
            self.browserWidget.dicomBrowser.ImportDirectoryMode = ctk.ctkDICOMBrowser.ImportDirectoryAddLink

    def importFolder(self):
        if not DICOMFileDialog.createDefaultDatabase():
            return
        self.browserWidget.dicomBrowser.openImportDialog()

    def onOpenBrowserWidget(self):
        slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutDicomBrowserView)

    def onToggleListener(self, toggled):
        if hasattr(slicer, 'dicomListener'):
            self.removeListenerObservers()
            slicer.modules.DICOMInstance.stopListener()
        if toggled:
            slicer.modules.DICOMInstance.startListener()
            self.addListenerObservers()
        self.onListenerStateChanged()

    def onListenerStateChanged(self, newState=None):
        """ Called when the indexer process state changes
        so we can provide feedback to the user
        """
        if hasattr(slicer, 'dicomListener') and slicer.dicomListener.process is not None:
            newState = slicer.dicomListener.process.state()
        else:
            newState = 0

        if newState == 0:
            self.ui.listenerStateLabel.text = "not started"
            wasBlocked = self.ui.toggleListener.blockSignals(True)
            self.ui.toggleListener.checked = False
            self.ui.toggleListener.blockSignals(wasBlocked)
            if hasattr(slicer.modules, 'DICOMInstance'):  # custom applications may not have the standard DICOM module
                slicer.modules.DICOMInstance.stopListener()
        if newState == 1:
            self.ui.listenerStateLabel.text = "starting"
        if newState == 2:
            port = str(slicer.dicomListener.port) if hasattr(slicer, 'dicomListener') else "unknown"
            self.ui.listenerStateLabel.text = "running at port " + port
            self.ui.toggleListener.checked = True

    def onListenerToAddFile(self):
        """ Called when the indexer is about to add a file to the database.
        Works around issue where ctkDICOMModel has open queries that keep the
        database locked.
        """
        pass

    def onListenerAddedFile(self):
        """Called after the listener has added a file.
        Restore and refresh the app model
        """
        newFile = slicer.dicomListener.lastFileAdded
        if newFile:
            slicer.util.showStatusMessage("Received DICOM file: %s" % newFile, 1000)
        self.databaseRefreshRequestTimer.start()

    def requestDatabaseRefresh(self):
        logging.debug("Database refresh is requested")
        if slicer.dicomDatabase.isOpen:
            slicer.dicomDatabase.databaseChanged()

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
                tmpDir = slicer.app.temporaryPath
                if not os.path.exists(tmpDir):
                    os.mkdir(tmpDir)
                self.tmpDir = tmpDir + '/DICOM'
                if not os.path.exists(self.tmpDir):
                    os.mkdir(self.tmpDir)
                self.testingServer = DICOMLib.DICOMTestingQRServer(exeDir=self.exeDir, tmpDir=self.tmpDir)

            # look for the sample data to load (only works on build trees
            # with standard naming conventions)
            self.dataDir = slicer.app.slicerHome + '/../../Slicer4/Testing/Data/Input/CTHeadAxialDicom'
            files = glob.glob(self.dataDir + '/*.dcm')

            # now start the server
            self.testingServer.start(verbose=self.verboseServer.checked, initialFiles=files)
            # self.toggleServer.text = "Stop Testing Server"

    def onRunListenerAtStart(self, toggled):
        settings = qt.QSettings()
        settings.setValue('DICOM/RunListenerAtStart', toggled)

    def updateDatabaseDirectoryFromWidget(self, databaseDirectory):
        self.browserWidget.dicomBrowser.databaseDirectory = databaseDirectory

    def updateDatabaseDirectoryFromBrowser(self, databaseDirectory):
        wasBlocked = self.ui.directoryButton.blockSignals(True)
        self.ui.directoryButton.directory = databaseDirectory
        self.ui.directoryButton.blockSignals(wasBlocked)

    def onBrowserAutoHideStateChanged(self, autoHideState):
        if self.browserWidget:
            self.browserWidget.setBrowserPersistence(autoHideState != qt.Qt.Checked)

    def onClearDatabase(self):
        patientIds = slicer.dicomDatabase.patients()
        if len(patientIds) == 0:
            slicer.util.infoDisplay("DICOM database is already empty.")
        elif not slicer.util.confirmYesNoDisplay(
            'Are you sure you want to delete all data and files copied into the database (%d patients)?' % len(patientIds),
                windowTitle='Clear entire DICOM database'):
            return
        slicer.app.setOverrideCursor(qt.Qt.WaitCursor)
        DICOMLib.clearDatabase(slicer.dicomDatabase)
        slicer.app.restoreOverrideCursor()

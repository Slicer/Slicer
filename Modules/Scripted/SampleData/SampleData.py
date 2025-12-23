import logging
import os
import textwrap

import ctk
import qt
import vtk

import slicer
from slicer.i18n import tr as _
from slicer.i18n import translate
from slicer.ScriptedLoadableModule import *
from slicer.util import computeChecksum, extractAlgoAndDigest, TESTING_DATA_URL


#
# SampleData methods

def downloadFromURL(uris=None, fileNames=None, nodeNames=None, checksums=None, loadFiles=None,
                    customDownloader=None, loadFileTypes=None, loadFileProperties={}):
    """Download and optionally load data into the application.

    :param uris: Download URL(s).
    :param fileNames: File name(s) that will be downloaded (and loaded).
    :param nodeNames: Node name(s) in the scene.
    :param checksums: Checksum(s) formatted as ``<algo>:<digest>`` to verify the downloaded file(s). For example, ``SHA256:cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93``.
    :param loadFiles: Boolean indicating if file(s) should be loaded. By default, the function decides.
    :param customDownloader: Custom function for downloading.
    :param loadFileTypes: file format name(s) (if not specified then the default file reader will be used).
    :param loadFileProperties: custom properties passed to the IO plugin.

    If the given ``fileNames`` are not found in the application cache directory, they
    are downloaded using the associated URIs.
    See ``slicer.mrmlScene.GetCacheManager().GetRemoteCacheDirectory()``

    If not explicitly provided or if set to ``None``, the ``loadFileTypes`` are
    guessed based on the corresponding filename extensions.

    If a given fileName has the ``.mrb`` or ``.mrml`` extension, it will **not** be loaded
    by default. To ensure the file is loaded, ``loadFiles`` must be set.

    The ``loadFileProperties`` are common for all files. If different properties
    need to be associated with files of different types, downloadFromURL must
    be called for each.
    """
    return SampleDataLogic().downloadFromURL(
        uris, fileNames, nodeNames, checksums, loadFiles, customDownloader, loadFileTypes, loadFileProperties)


def downloadSample(sampleName):
    """For a given sample name this will search the available sources
    and load it if it is available.  Returns the first loaded node.
    """
    return SampleDataLogic().downloadSamples(sampleName)[0]


def downloadSamples(sampleName):
    """For a given sample name this will search the available sources
    and load it if it is available.  Returns the loaded nodes.
    """
    return SampleDataLogic().downloadSamples(sampleName)


#
# SampleData
#


class SampleData(ScriptedLoadableModule):
    """Uses ScriptedLoadableModule base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = _("Sample Data")
        self.parent.categories = [translate("qSlicerAbstractCoreModule", "Informatics")]
        self.parent.dependencies = []
        self.parent.contributors = ["Steve Pieper (Isomics), Benjamin Long (Kitware), Jean-Christophe Fillion-Robin (Kitware)"]
        self.parent.helpText = _("""
This module provides data sets that can be used for testing 3D Slicer.
""")
        self.parent.helpText += self.getDefaultModuleDocumentationLink()
        self.parent.acknowledgementText = _("""
<p>This work was was funded in part by Cancer Care Ontario
and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)</p>

<p>MRHead, CBCT-MR Head, and CT-MR Brain data sets were donated to 3D Slicer project by the persons visible in the images, to be used without any restrictions.</p>

<p>CTLiver dataset comes from <a href="http://medicaldecathlon.com/">Medical Decathlon project</a> (imagesTr/liver_100.nii.gz in Task03_Liver collection)
with a permissive copyright-license (<a href="https://creativecommons.org/licenses/by-sa/4.0/">CC-BY-SA 4.0</a>), allowing for data to be shared, distributed and improved upon.</p>

<p>CTA abdomen (Panoramix) dataset comes from <a href="https://www.osirix-viewer.com/resources/dicom-image-library/">Osirix DICOM image library</a>
and is exclusively available for research and teaching. You are not authorized to redistribute or sell it, or
use it for commercial purposes.</p>
""")

        if slicer.mrmlScene.GetTagByClassName("vtkMRMLScriptedModuleNode") != "ScriptedModule":
            slicer.mrmlScene.RegisterNodeClass(vtkMRMLScriptedModuleNode())

        # Trigger the menu to be added when application has started up
        if not slicer.app.commandOptions().noMainWindow:
            slicer.app.connect("startupCompleted()", self.addMenu)

        # allow other modules to register sample data sources by appending
        # instances or subclasses SampleDataSource objects on this list
        try:
            slicer.modules.sampleDataSources
        except AttributeError:
            slicer.modules.sampleDataSources = {}

    def addMenu(self):
        a = qt.QAction(_("Download Sample Data"), slicer.util.mainWindow())
        a.setToolTip(_("Go to the SampleData module to download data from the network"))
        a.connect("triggered()", self.select)

        fileMenu = slicer.util.lookupTopLevelWidget("FileMenu")
        if fileMenu:
            for action in fileMenu.actions():
                if action.objectName == "FileSaveSceneAction":
                    fileMenu.insertAction(action, a)
                    fileMenu.insertSeparator(action)

    def select(self):
        m = slicer.util.mainWindow()
        m.moduleSelector().selectModule("SampleData")


#
# SampleDataSource
#
class SampleDataSource:
    """Describe a set of sample data associated with one or multiple URIs and filenames.

    Example::

      import SampleData
      from slicer.util import TESTING_DATA_URL
      dataSource = SampleData.SampleDataSource(
        nodeNames='fixed',
        fileNames='fixed.nrrd',
        uris=TESTING_DATA_URL + 'SHA256/b757f9c61c1b939f104e5d7861130bb28d90f33267a012eb8bb763a435f29d37')
      loadedNode = SampleData.SampleDataLogic().downloadFromSource(dataSource)[0]
    """

    def __init__(self, sampleName=None, sampleDescription=None, uris=None, fileNames=None, nodeNames=None,
                 checksums=None, loadFiles=None,
                 customDownloader=None, thumbnailFileName=None,
                 loadFileTypes=None, loadFileProperties=None,
                 loadFileType=None):
        """
        :param sampleName: Name identifying the data set.
        :param sampleDescription: Displayed name of data set in SampleData module GUI. (default is ``sampleName``)
        :param thumbnailFileName: Displayed thumbnail of data set in SampleData module GUI,
        :param uris: Download URL(s).
        :param fileNames: File name(s) that will be downloaded (and loaded).
        :param nodeNames: Node name(s) in the scene.
        :param checksums: Checksum(s) formatted as ``<algo>:<digest>`` to verify the downloaded file(s). For example, ``SHA256:cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93``.
        :param loadFiles: Boolean indicating if file(s) should be loaded.
        :param customDownloader: Custom function for downloading.
        :param loadFileTypes: file format name(s) (if not specified then the default file reader will be used).
        :param loadFileProperties: custom properties passed to the IO plugin.
        :param loadFileType: deprecated, use ``loadFileTypes`` instead.
        """

        # For backward compatibility (allow using "loadFileType" instead of "loadFileTypes")
        if (loadFileType is not None) and (loadFileTypes is not None):
            raise ValueError("loadFileType and loadFileTypes cannot be specified at the same time")
        if (loadFileType is not None) and (loadFileTypes is None):
            loadFileTypes = loadFileType

        self.sampleName = sampleName
        if sampleDescription is None:
            sampleDescription = sampleName
        self.sampleDescription = sampleDescription
        if isinstance(uris, list) or isinstance(uris, tuple):
            if isinstance(loadFileTypes, str) or loadFileTypes is None:
                loadFileTypes = [loadFileTypes] * len(uris)
            if nodeNames is None:
                nodeNames = [None] * len(uris)
            if loadFiles is None:
                loadFiles = [None] * len(uris)
            if checksums is None:
                checksums = [None] * len(uris)
        elif isinstance(uris, str):
            uris = [uris]
            fileNames = [fileNames]
            nodeNames = [nodeNames]
            loadFiles = [loadFiles]
            loadFileTypes = [loadFileTypes]
            checksums = [checksums]

        if loadFileProperties is None:
            loadFileProperties = {}

        self.uris = uris
        self.fileNames = fileNames
        self.nodeNames = nodeNames
        self.loadFiles = loadFiles
        self.customDownloader = customDownloader
        self.thumbnailFileName = thumbnailFileName
        self.loadFileTypes = loadFileTypes
        self.loadFileProperties = loadFileProperties
        self.checksums = checksums
        if not len(uris) == len(fileNames) == len(nodeNames) == len(loadFiles) == len(loadFileTypes) == len(checksums):
            raise ValueError(
                f"All fields of sample data source must have the same length\n"
                f"  uris                 : {uris}\n"
                f"  len(uris)            : {len(uris)}\n"
                f"  len(fileNames)       : {len(fileNames)}\n"
                f"  len(nodeNames)       : {len(nodeNames)}\n"
                f"  len(loadFiles)       : {len(loadFiles)}\n"
                f"  len(loadFileTypes)   : {len(loadFileTypes)}\n"
                f"  len(checksums)       : {len(checksums)}\n",
            )

    def __eq__(self, other):
        return str(self) == str(other)

    def __str__(self):
        output = [
            "sampleName        : %s" % self.sampleName,
            "sampleDescription : %s" % self.sampleDescription,
            "thumbnailFileName : %s" % self.thumbnailFileName,
            "loadFileProperties: %s" % self.loadFileProperties,
            "customDownloader  : %s" % self.customDownloader,
            "",
        ]
        for fileName, uri, nodeName, loadFile, fileType, checksum in zip(
            self.fileNames, self.uris, self.nodeNames, self.loadFiles, self.loadFileTypes, self.checksums, strict=True):

            output.extend([
                " fileName     : %s" % fileName,
                " uri          : %s" % uri,
                " checksum     : %s" % checksum,
                " nodeName     : %s" % nodeName,
                " loadFile     : %s" % loadFile,
                " loadFileType : %s" % fileType,
                "",
            ])
        return "\n".join(output)


#
# SampleData widget
#

class TextEditFinishEventFilter(qt.QObject):

    def __init__(self, *args, **kwargs):
        qt.QObject.__init__(self, *args, **kwargs)
        self.textEditFinishedCallback = None

    def eventFilter(self, object, event):
        if self.textEditFinishedCallback is None:
            return
        if (event.type() == qt.QEvent.KeyPress and event.key() in (qt.Qt.Key_Return, qt.Qt.Key_Enter)
                and (event.modifiers() & qt.Qt.ControlModifier)):
            self.textEditFinishedCallback()
            return True  # stop further handling
        return False

class SampleDataWidget(ScriptedLoadableModuleWidget):
    """Uses ScriptedLoadableModuleWidget base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)

        # This module is often used in developer mode, therefore
        # collapse reload & test section by default.
        if hasattr(self, "reloadCollapsibleButton"):
            self.reloadCollapsibleButton.collapsed = True

        self.logic = SampleDataLogic(self.logMessage)

        self.categoryLayout = qt.QVBoxLayout()
        self.categoryLayout.setContentsMargins(0, 0, 0, 0)
        self.layout.addLayout(self.categoryLayout)

        SampleDataWidget.setCategoriesFromSampleDataSources(self.categoryLayout, slicer.modules.sampleDataSources, self.logic)
        if self.developerMode is False:
            self.setCategoryVisible(self.logic.developmentCategoryName, False)

        customFrame = ctk.ctkCollapsibleGroupBox()
        self.categoryLayout.addWidget(customFrame)
        customFrame.title = _("Load data from URL")
        customFrameLayout = qt.QVBoxLayout()
        customFrame.setLayout(customFrameLayout)
        self.customSampleLabel = qt.QLabel(_("Download URLs:"))
        customFrameLayout.addWidget(self.customSampleLabel)
        self.customSampleUrlEdit = qt.QTextEdit()
        self.customSampleUrlEdit.toolTip = _("Enter one or more URLs (one per line) to download and load the corresponding data sets. "
                                            "Press Ctrl+Enter or click 'Load' button to start loading.")

        # Install event filter to catch Ctrl+Enter keypress to start loading
        self.customSampleUrlEditFinishedEventFilter = TextEditFinishEventFilter()
        self.customSampleUrlEditFinishedEventFilter.textEditFinishedCallback = self.onCustomDataDownload
        self.customSampleUrlEdit.installEventFilter(self.customSampleUrlEditFinishedEventFilter)

        customFrameLayout.addWidget(self.customSampleUrlEdit)
        self.customDownloadButton = qt.QPushButton(_("Load"))
        self.customDownloadButton.toolTip = _("Download the dataset from the given URL and import it into the scene")
        self.customDownloadButton.default = True
        customFrameLayout.addWidget(self.customDownloadButton)
        self.showCustomDataFolderButton = qt.QPushButton(_("Show folder"))
        self.showCustomDataFolderButton.toolTip = _("Show folder where custom data sets are downloaded ({path}).").format(path=slicer.app.cachePath)
        customFrameLayout.addWidget(self.showCustomDataFolderButton)
        customFrame.collapsed = True
        self.customDownloadButton.connect("clicked()", self.onCustomDataDownload)
        self.showCustomDataFolderButton.connect("clicked()", self.onShowCustomDataFolder)

        self.log = qt.QTextEdit()
        self.log.readOnly = True
        self.layout.addWidget(self.log)

        # Add spacer to layout
        self.layout.addStretch(1)

    def cleanup(self):
        SampleDataWidget.setCategoriesFromSampleDataSources(self.categoryLayout, {}, self.logic)

    def onCustomDataDownload(self):
        uris = self.customSampleUrlEdit.toPlainText().strip()
        if not uris:
            return
        uriList = [u.strip() for u in uris.splitlines() if u.strip()]
        totalCount = 0
        errorCount = 0
        for uri in uriList:
            self.logMessage(f"Downloading data from {uri} ...", logging.INFO)
            if not uri:
                continue
            try:
                totalCount += 1
                # Get filename from URL
                import urllib
                parsedUrl = urllib.parse.urlparse(uri)
                basename, ext = os.path.splitext(os.path.basename(parsedUrl.path))
                filename = basename + ext
                # Download
                result = self.logic.downloadFromURL(uris=uri, fileNames=filename, forceDownload=True)
                if not result:
                    errorCount += 1
            except Exception as e:
                errorCount += 1
                self.logMessage(f"Data download from {uri} failed: {str(e)}", logging.ERROR)
                import traceback
                traceback.print_exc()

        if totalCount > 1:
            self.logMessage("", logging.INFO)
            if errorCount == 0:
                self.logMessage(_("All {totalCount} data sets were loaded successfully.").format(totalCount=totalCount), logging.INFO)
            else:
                self.logMessage(_("Failed to load {errorCount} out of {totalCount} data sets.").format(errorCount=errorCount, totalCount=totalCount), logging.ERROR)

    def onShowCustomDataFolder(self):
        qt.QDesktopServices.openUrl(qt.QUrl("file:///" + slicer.app.cachePath, qt.QUrl.TolerantMode))

    @staticmethod
    def removeCategories(categoryLayout):
        """Remove all categories from the given category layout."""
        while categoryLayout.count() > 0:
            frame = categoryLayout.itemAt(0).widget()
            frame.visible = False
            categoryLayout.removeWidget(frame)
            frame.setParent(0)
            del frame

    @staticmethod
    def setCategoriesFromSampleDataSources(categoryLayout, dataSources, logic):
        """Update categoryLayout adding buttons for downloading dataSources.

        Download buttons are organized in collapsible GroupBox with one GroupBox
        per category.
        """
        iconPath = os.path.join(os.path.dirname(__file__).replace("\\", "/"), "Resources", "Icons")
        mainWindow = slicer.util.mainWindow()
        if mainWindow:
            # Set thumbnail size from default icon size. This results in toolbutton size that makes
            # two columns of buttons fit into the size of the Welcome module's minimum width
            # on screens with a various resolution and scaling (see qt.QDesktopWidget().size,
            # desktop.devicePixelRatioF(), qt.QDesktopWidget().physicalDpiX())
            iconSize = qt.QSize(int(mainWindow.iconSize.width() * 6), int(mainWindow.iconSize.height() * 4))
        else:
            # There is no main window in the automated tests
            screens = slicer.app.screens()
            primaryScreen = screens[0] if screens else None
            if primaryScreen:
                mainScreenSize = primaryScreen.availableGeometry.size()
                iconSize = qt.QSize(
                    int(mainScreenSize.width() / 15),
                    int(mainScreenSize.height() / 10),
                )
            else:
                # Absolute fallback (should never happen, but keeps tests robust)
                iconSize = qt.QSize(128, 128)

        categories = sorted(dataSources.keys())

        # Ensure "builtIn" category is always first
        if logic.builtInCategoryName in categories:
            categories.remove(logic.builtInCategoryName)
            categories.insert(0, logic.builtInCategoryName)

        # Clear category layout
        SampleDataWidget.removeCategories(categoryLayout)

        # Populate category layout
        for category in categories:
            frame = ctk.ctkCollapsibleGroupBox(categoryLayout.parentWidget())
            categoryLayout.addWidget(frame)
            frame.title = category
            frame.name = "%sCollapsibleGroupBox" % category
            layout = ctk.ctkFlowLayout()
            layout.preferredExpandingDirections = qt.Qt.Vertical
            frame.setLayout(layout)
            for source in dataSources[category]:
                name = source.sampleDescription
                if not name:
                    name = source.nodeNames[0]

                b = qt.QToolButton()
                b.setText(name)

                # Set thumbnail
                if source.thumbnailFileName:
                    # Thumbnail provided
                    thumbnailImage = source.thumbnailFileName
                else:
                    # Look for thumbnail image with the name of any node name with .png extension
                    thumbnailImage = None
                    for nodeName in source.nodeNames:
                        if not nodeName:
                            continue
                        thumbnailImageAttempt = os.path.join(iconPath, nodeName + ".png")
                        if os.path.exists(thumbnailImageAttempt):
                            thumbnailImage = thumbnailImageAttempt
                            break
                if thumbnailImage and os.path.exists(thumbnailImage):
                    b.setIcon(qt.QIcon(thumbnailImage))

                b.setIconSize(iconSize)
                b.setToolButtonStyle(qt.Qt.ToolButtonTextUnderIcon)
                qSize = qt.QSizePolicy()
                qSize.setHorizontalPolicy(qt.QSizePolicy.Expanding)
                b.setSizePolicy(qSize)

                b.name = "%sPushButton" % name
                layout.addWidget(b)
                if source.customDownloader:
                    b.connect("clicked()", lambda s=source: s.customDownloader(s))
                else:
                    b.connect("clicked()", lambda s=source: logic.downloadFromSource(s))

    def logMessage(self, message, logLevel=logging.DEBUG):
        # Format based on log level
        if logLevel >= logging.ERROR:
            message = '<b><font color="red">' + message + "</font></b>"
        elif logLevel >= logging.WARNING:
            message = '<b><font color="orange">' + message + "</font></b>"

        # Show message in status bar
        doc = qt.QTextDocument()
        doc.setHtml(message)
        slicer.util.showStatusMessage(doc.toPlainText(), 3000)
        logging.log(logLevel, doc.toPlainText())

        # Show message in log window at the bottom of the module widget
        self.log.insertHtml(message)
        self.log.insertPlainText("\n")
        self.log.ensureCursorVisible()
        self.log.repaint()

        slicer.app.processEvents(qt.QEventLoop.ExcludeUserInputEvents)

    def isCategoryVisible(self, category):
        """Check the visibility of a SampleData category given its name.

        Returns False if the category is not visible or if it does not exist,
        otherwise returns True.
        """
        if not SampleDataLogic.sampleDataSourcesByCategory(category):
            return False
        return slicer.util.findChild(self.parent, "%sCollapsibleGroupBox" % category).isVisible()

    def setCategoryVisible(self, category, visible):
        """Update visibility of a SampleData category given its name.

        The function is a no-op if the category does not exist.
        """
        if not SampleDataLogic.sampleDataSourcesByCategory(category):
            return
        slicer.util.findChild(self.parent, "%sCollapsibleGroupBox" % category).setVisible(visible)


#
# SampleData logic
#


class SampleDataLogic:
    """Manage the slicer.modules.sampleDataSources dictionary.
    The dictionary keys are categories of sample data sources.
    The BuiltIn category is managed here.  Modules or extensions can
    register their own sample data by creating instances of the
    SampleDataSource class.  These instances should be stored in a
    list that is assigned to a category following the model
    used in registerBuiltInSampleDataSources below.

    Checksums are expected to be formatted as a string of the form
    ``<algo>:<digest>``. For example, ``SHA256:cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93``.
    """

    @staticmethod
    def registerCustomSampleDataSource(category="Custom",
                                       sampleName=None, uris=None, fileNames=None, nodeNames=None,
                                       customDownloader=None, thumbnailFileName=None,
                                       loadFileTypes=None, loadFiles=None, loadFileProperties={},
                                       checksums=None,
                                       loadFileType=None):
        """Adds custom data sets to SampleData.
        :param category: Section title of data set in SampleData module GUI.
        :param sampleName: Displayed name of data set in SampleData module GUI.
        :param thumbnailFileName: Displayed thumbnail of data set in SampleData module GUI,
        :param uris: Download URL(s).
        :param fileNames: File name(s) that will be loaded.
        :param nodeNames: Node name(s) in the scene.
        :param customDownloader: Custom function for downloading.
        :param loadFileTypes: file format name(s) (if not specified then the default file reader will be used).
        :param loadFiles: Boolean indicating if file(s) should be loaded. By default, the function decides.
        :param loadFileProperties: custom properties passed to the IO plugin.
        :param checksums: Checksum(s) formatted as ``<algo>:<digest>`` to verify the downloaded file(s). For example, ``SHA256:cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93``.
        :param loadFileType: deprecated, use ``loadFileTypes`` instead.
        """

        # For backward compatibility (allow using "loadFileType" instead of "loadFileTypes")
        if (loadFileType is not None) and (loadFileTypes is not None):
            raise ValueError("loadFileType and loadFileTypes cannot be specified at the same time")
        if (loadFileType is not None) and (loadFileTypes is None):
            loadFileTypes = loadFileType

        try:
            slicer.modules.sampleDataSources
        except AttributeError:
            slicer.modules.sampleDataSources = {}

        if category not in slicer.modules.sampleDataSources:
            slicer.modules.sampleDataSources[category] = []

        dataSource = SampleDataSource(
            sampleName=sampleName,
            uris=uris,
            fileNames=fileNames,
            nodeNames=nodeNames,
            thumbnailFileName=thumbnailFileName,
            loadFileTypes=loadFileTypes,
            loadFiles=loadFiles,
            loadFileProperties=loadFileProperties,
            checksums=checksums,
            customDownloader=customDownloader,
        )

        if SampleDataLogic.isSampleDataSourceRegistered(category, dataSource):
            return

        slicer.modules.sampleDataSources[category].append(dataSource)

    @staticmethod
    def sampleDataSourcesByCategory(category=None):
        """Return the registered SampleDataSources for with the given category.

        If no category is specified, returns all registered SampleDataSources.
        """
        try:
            slicer.modules.sampleDataSources
        except AttributeError:
            slicer.modules.sampleDataSources = {}

        if category is None:
            return slicer.modules.sampleDataSources
        else:
            return slicer.modules.sampleDataSources.get(category, [])

    @staticmethod
    def isSampleDataSourceRegistered(category, sampleDataSource):
        """Returns True if the sampleDataSource is registered with the category."""
        try:
            slicer.modules.sampleDataSources
        except AttributeError:
            slicer.modules.sampleDataSources = {}

        if not isinstance(sampleDataSource, SampleDataSource):
            raise TypeError(f"unsupported sampleDataSource type '{type(sampleDataSource)}': '{str(SampleDataSource)}' is expected")

        return sampleDataSource in slicer.modules.sampleDataSources.get(category, [])

    @staticmethod
    def registerUrlRewriteRule(name, rewriteFunction):
        """Adds a URL rewrite rule for custom data sets in SampleData.
        :param name: Name of the rewrite rule.
        :param rewriteFunction: Function that implements the rewrite logic.
        """
        if not callable(rewriteFunction):
            raise TypeError(f"Expected 'rewriteFunction' to be callable, got {type(rewriteFunction)}")

        try:
            slicer.modules.sampleDataUrlRewriteRules
        except AttributeError:
            slicer.modules.sampleDataUrlRewriteRules = {}

        slicer.modules.sampleDataUrlRewriteRules[name] = rewriteFunction

    def __init__(self, logMessage=None):
        if logMessage:
            self.logMessage = logMessage
        self.builtInCategoryName = _("General")
        self.developmentCategoryName = _("Development")
        self.registerBuiltInSampleDataSources()
        self.registerDevelopmentSampleDataSources()
        if slicer.app.testingEnabled():
            self.registerTestingDataSources()
        self.registerBuiltInUrlRewriteRules()
        self.downloadPercent = 0

    def registerBuiltInSampleDataSources(self):
        """Fills in the pre-define sample data sources"""

        #   Arguments:
        #     sampleName=None, sampleDescription=None,
        #     uris=None,
        #     fileNames=None, nodeNames=None,
        #     checksums=None,
        #     loadFiles=None, customDownloader=None, thumbnailFileName=None, loadFileTypes=None, loadFileProperties=None
        sourceArguments = (
            ("MRHead", None, TESTING_DATA_URL + "SHA256/cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93",
             "MR-head.nrrd", "MRHead", "SHA256:cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93"),
            ("CTChest", None, TESTING_DATA_URL + "SHA256/4507b664690840abb6cb9af2d919377ffc4ef75b167cb6fd0f747befdb12e38e",
             "CT-chest.nrrd", "CTChest", "SHA256:4507b664690840abb6cb9af2d919377ffc4ef75b167cb6fd0f747befdb12e38e"),
            ("CTACardio", None, TESTING_DATA_URL + "SHA256/3b0d4eb1a7d8ebb0c5a89cc0504640f76a030b4e869e33ff34c564c3d3b88ad2",
             "CTA-cardio.nrrd", "CTACardio", "SHA256:3b0d4eb1a7d8ebb0c5a89cc0504640f76a030b4e869e33ff34c564c3d3b88ad2"),
            ("DTIBrain", None, TESTING_DATA_URL + "SHA256/5c78d00c86ae8d968caa7a49b870ef8e1c04525b1abc53845751d8bce1f0b91a",
             "DTI-Brain.nrrd", "DTIBrain", "SHA256:5c78d00c86ae8d968caa7a49b870ef8e1c04525b1abc53845751d8bce1f0b91a"),
            ("MRBrainTumor1", None, TESTING_DATA_URL + "SHA256/998cb522173839c78657f4bc0ea907cea09fd04e44601f17c82ea27927937b95",
             "RegLib_C01_1.nrrd", "MRBrainTumor1", "SHA256:998cb522173839c78657f4bc0ea907cea09fd04e44601f17c82ea27927937b95"),
            ("MRBrainTumor2", None, TESTING_DATA_URL + "SHA256/1a64f3f422eb3d1c9b093d1a18da354b13bcf307907c66317e2463ee530b7a97",
             "RegLib_C01_2.nrrd", "MRBrainTumor2", "SHA256:1a64f3f422eb3d1c9b093d1a18da354b13bcf307907c66317e2463ee530b7a97"),
            ("BaselineVolume", None, TESTING_DATA_URL + "SHA256/dff28a7711d20b6e16d5416535f6010eb99fd0c8468aaa39be4e39da78e93ec2",
             "BaselineVolume.nrrd", "BaselineVolume", "SHA256:dff28a7711d20b6e16d5416535f6010eb99fd0c8468aaa39be4e39da78e93ec2"),
            ("DTIVolume", None,
             (TESTING_DATA_URL + "SHA256/d785837276758ddd9d21d76a3694e7fd866505a05bc305793517774c117cb38d",
              TESTING_DATA_URL + "SHA256/67564aa42c7e2eec5c3fd68afb5a910e9eab837b61da780933716a3b922e50fe" ),
             ("DTIVolume.raw.gz", "DTIVolume.nhdr"), (None, "DTIVolume"),
             ("SHA256:d785837276758ddd9d21d76a3694e7fd866505a05bc305793517774c117cb38d",
              "SHA256:67564aa42c7e2eec5c3fd68afb5a910e9eab837b61da780933716a3b922e50fe")),
            ("DWIVolume", None,
             (TESTING_DATA_URL + "SHA256/cf03fd53583dc05120d3314d0a82bdf5946799b1f72f2a7f08963f3fd24ca692",
              TESTING_DATA_URL + "SHA256/7666d83bc205382e418444ea60ab7df6dba6a0bd684933df8809da6b476b0fed"),
             ("dwi.raw.gz", "dwi.nhdr"), (None, "dwi"),
             ("SHA256:cf03fd53583dc05120d3314d0a82bdf5946799b1f72f2a7f08963f3fd24ca692",
              "SHA256:7666d83bc205382e418444ea60ab7df6dba6a0bd684933df8809da6b476b0fed")),
            ("CTAAbdomenPanoramix", "CTA abdomen\n(Panoramix)", TESTING_DATA_URL + "SHA256/146af87511520c500a3706b7b2bfb545f40d5d04dd180be3a7a2c6940e447433",
             "Panoramix-cropped.nrrd", "Panoramix-cropped", "SHA256:146af87511520c500a3706b7b2bfb545f40d5d04dd180be3a7a2c6940e447433"),
            ("CBCTDentalSurgery", None,
             (TESTING_DATA_URL + "SHA256/7bfa16945629c319a439f414cfb7edddd2a97ba97753e12eede3b56a0eb09968",
              TESTING_DATA_URL + "SHA256/4cdc3dc35519bb57daeef4e5df89c00849750e778809e94971d3876f95cc7bbd"),
             ("PreDentalSurgery.gipl.gz", "PostDentalSurgery.gipl.gz"), ("PreDentalSurgery", "PostDentalSurgery"),
             ("SHA256:7bfa16945629c319a439f414cfb7edddd2a97ba97753e12eede3b56a0eb09968",
              "SHA256:4cdc3dc35519bb57daeef4e5df89c00849750e778809e94971d3876f95cc7bbd")),
            ("MRUSProstate", "MR-US Prostate",
             (TESTING_DATA_URL + "SHA256/4843cdc9ea5d7bcce61650d1492ce01035727c892019339dca726380496896aa",
              TESTING_DATA_URL + "SHA256/34decf58b1e6794069acbe947b460252262fe95b6858c5e320aeab03bc82ebb2"),
             ("Case10-MR.nrrd", "case10_US_resampled.nrrd"), ("MRProstate", "USProstate"),
             ("SHA256:4843cdc9ea5d7bcce61650d1492ce01035727c892019339dca726380496896aa",
              "SHA256:34decf58b1e6794069acbe947b460252262fe95b6858c5e320aeab03bc82ebb2")),
            ("CTMRBrain", "CT-MR Brain",
             (TESTING_DATA_URL + "SHA256/6a5b6caccb76576a863beb095e3bfb910c50ca78f4c9bf043aa42f976cfa53d1",
              TESTING_DATA_URL + "SHA256/2da3f655ed20356ee8cdf32aa0f8f9420385de4b6e407d28e67f9974d7ce1593",
              TESTING_DATA_URL + "SHA256/fa1fe5910a69182f2b03c0150d8151ac6c75df986449fb5a6c5ae67141e0f5e7"),
             ("CT-brain.nrrd", "MR-brain-T1.nrrd", "MR-brain-T2.nrrd"),
             ("CTBrain", "MRBrainT1", "MRBrainT2"),
             ("SHA256:6a5b6caccb76576a863beb095e3bfb910c50ca78f4c9bf043aa42f976cfa53d1",
              "SHA256:2da3f655ed20356ee8cdf32aa0f8f9420385de4b6e407d28e67f9974d7ce1593",
              "SHA256:fa1fe5910a69182f2b03c0150d8151ac6c75df986449fb5a6c5ae67141e0f5e7")),
            ("CBCTMRHead", "CBCT-MR Head",
             (TESTING_DATA_URL + "SHA256/4ce7aa75278b5a7b757ed0c8d7a6b3caccfc3e2973b020532456dbc8f3def7db",
              TESTING_DATA_URL + "SHA256/b5e9f8afac58d6eb0e0d63d059616c25a98e0beb80f3108410b15260a6817842"),
             ("DZ-CBCT.nrrd", "DZ-MR.nrrd"),
             ("DZ-CBCT", "DZ-MR"),
             ("SHA256:4ce7aa75278b5a7b757ed0c8d7a6b3caccfc3e2973b020532456dbc8f3def7db",
              "SHA256:b5e9f8afac58d6eb0e0d63d059616c25a98e0beb80f3108410b15260a6817842")),
            ("CTLiver", None, TESTING_DATA_URL + "SHA256/e16eae0ae6fefa858c5c11e58f0f1bb81834d81b7102e021571056324ef6f37e",
             "CTLiver.nrrd", "CTLiver", "SHA256:e16eae0ae6fefa858c5c11e58f0f1bb81834d81b7102e021571056324ef6f37e"),
            ("CTPCardioSeq", "CTP Cardio Sequence",
             "https://github.com/Slicer/SlicerDataStore/releases/download/SHA256/7fbb6ad0aed9c00820d66e143c2f037568025ed63db0a8db05ae7f26affeb1c2",
             "CTP-cardio.seq.nrrd", "CTPCardioSeq",
             "SHA256:7fbb6ad0aed9c00820d66e143c2f037568025ed63db0a8db05ae7f26affeb1c2",
             None, None, None, "SequenceFile"),
            ("CTCardioSeq", "CT Cardio Sequence",
             "https://github.com/Slicer/SlicerDataStore/releases/download/SHA256/d1a1119969acead6c39c7c3ec69223fa2957edc561bc5bf384a203e2284dbc93",
             "CT-cardio.seq.nrrd", "CTCardioSeq",
             "SHA256:d1a1119969acead6c39c7c3ec69223fa2957edc561bc5bf384a203e2284dbc93",
             None, None, None, "SequenceFile"),
        )

        if self.builtInCategoryName not in slicer.modules.sampleDataSources:
            slicer.modules.sampleDataSources[self.builtInCategoryName] = []
        for sourceArgument in sourceArguments:
            dataSource = SampleDataSource(*sourceArgument)
            if SampleDataLogic.isSampleDataSourceRegistered(self.builtInCategoryName, dataSource):
                continue
            slicer.modules.sampleDataSources[self.builtInCategoryName].append(dataSource)

    def registerDevelopmentSampleDataSources(self):
        """Fills in the sample data sources displayed only if developer mode is enabled."""
        iconPath = os.path.join(os.path.dirname(__file__).replace("\\", "/"), "Resources", "Icons")
        self.registerCustomSampleDataSource(
            category=self.developmentCategoryName, sampleName="TinyPatient",
            uris=[TESTING_DATA_URL + "SHA256/c0743772587e2dd4c97d4e894f5486f7a9a202049c8575e032114c0a5c935c3b",
                  TESTING_DATA_URL + "SHA256/3243b62bde36b1db1cdbfe204785bd4bc1fbb772558d5f8cac964cda8385d470"],
            fileNames=["TinyPatient_CT.nrrd", "TinyPatient_Structures.seg.nrrd"],
            nodeNames=["TinyPatient_CT", "TinyPatient_Segments"],
            thumbnailFileName=os.path.join(iconPath, "TinyPatient.png"),
            loadFileTypes=["VolumeFile", "SegmentationFile"],
            checksums=["SHA256:c0743772587e2dd4c97d4e894f5486f7a9a202049c8575e032114c0a5c935c3b", "SHA256:3243b62bde36b1db1cdbfe204785bd4bc1fbb772558d5f8cac964cda8385d470"],
        )

    def registerTestingDataSources(self):
        """Register sample data sources used by SampleData self-test to test module functionalities."""
        self.registerCustomSampleDataSource(**SampleDataTest.CustomDownloaderDataSource)

    def registerBuiltInUrlRewriteRules(self):
        """Register built-in URL rewrite rules."""

        def githubFileWebpageUrlRewriteRule(url):
            """Rewrite GitHub file webpage URL to download URL.
            Example:
              https://github.com/SlicerMorph/terms-and-colors/blob/main/JaimiGrayTetrapodSkulls.csv
            is automatically converted to
              https://raw.githubusercontent.com/SlicerMorph/terms-and-colors/refs/heads/main/JaimiGrayTetrapodSkulls.csv
            """
            import urllib.parse as urlparse
            parsedUrl = urlparse.urlparse(url)
            if parsedUrl.scheme != "https" or parsedUrl.netloc != "github.com":
                return url
            pathParts = parsedUrl.path.split("/")
            if len(pathParts) <= 4:
                return url
            organization = pathParts[1]
            repository = pathParts[2]
            rootFolder = pathParts[3]
            filePath = "/".join(pathParts[4:])
            if rootFolder != "blob":
                return url
            parsedUrl = parsedUrl._replace(netloc="raw.githubusercontent.com")
            parsedUrl = parsedUrl._replace(path="/".join([organization, repository, "refs", "heads"]) + "/" + filePath)
            return parsedUrl.geturl()

        def dropboxDownloadUrlRewriteRule(url):
            """Rewrite Dropbox shared link to direct download link.
            Example:
              https://www.dropbox.com/s/abcd1234efgh5678/myfile.nrrd
            is automatically converted to
              https://dl.dropbox.com/s/abcd1234efgh5678/myfile.nrrd
            """
            import urllib.parse as urlparse
            parsedUrl = urlparse.urlparse(url)
            if parsedUrl.scheme != "https" or parsedUrl.netloc != "www.dropbox.com":
                return url
            parsedUrl = parsedUrl._replace(netloc="dl.dropbox.com")
            return parsedUrl.geturl()

        self.registerUrlRewriteRule("GitHubFileWebpage", githubFileWebpageUrlRewriteRule)
        self.registerUrlRewriteRule("DropboxDownloadLink", dropboxDownloadUrlRewriteRule)

    def downloadFileIntoCache(self, uri, name, checksum=None, forceDownload=False):
        """Given a uri and a filename, download the data into
        a file of the given name in the scene's cache.
        If checksum is provided then it is used to decide if the file has to be downloaded again.
        If checksum is not provided then the file is not downloaded again unless forceDownload is True.
        """
        destFolderPath = slicer.mrmlScene.GetCacheManager().GetRemoteCacheDirectory()

        if not os.access(destFolderPath, os.W_OK):
            try:
                os.makedirs(destFolderPath, exist_ok=True)
            except:
                self.logMessage(_("Failed to create cache folder {path}").format(path=destFolderPath), logging.ERROR)
            if not os.access(destFolderPath, os.W_OK):
                self.logMessage(_("Cache folder {path} is not writable").format(path=destFolderPath), logging.ERROR)
        return self.downloadFile(uri, destFolderPath, name, checksum, forceDownload=forceDownload)

    def downloadSourceIntoCache(self, source):
        """Download all files for the given source and return a
        list of file paths for the results
        """
        filePaths = []
        for uri, fileName, checksum in zip(source.uris, source.fileNames, source.checksums, strict=False):
            filePaths.append(self.downloadFileIntoCache(uri, fileName, checksum))
        return filePaths

    def downloadFromSource(self, source, maximumAttemptsCount=3, forceDownload=False):
        """Given an instance of SampleDataSource, downloads the associated data and
        load them into Slicer if it applies.

        The function always returns a list.

        Based on the fileType(s), nodeName(s) and loadFile(s) associated with
        the source, different values may be appended to the returned list:

          - if nodeName is specified, appends loaded nodes but if ``loadFile`` is False appends downloaded filepath
          - if fileType is ``SceneFile``, appends downloaded filepath
          - if fileType is ``ZipFile``, appends directory of extracted archive but if ``loadFile`` is False appends downloaded filepath

        If no ``nodeNames`` and no ``fileTypes`` are specified or if ``loadFiles`` are all False,
        returns the list of all downloaded filepaths.

        If ``forceDownload`` is True then existing files are downloaded again (if no hash is provided).
        """

        # Input may contain urls without associated node names, which correspond to additional data files
        # (e.g., .raw file for a .nhdr header file). Therefore we collect nodes and file paths separately
        # and we only return file paths if no node names have been provided.
        resultNodes = []
        resultFilePaths = []

        # If some node names are defined and some are left empty then we assume that it is intentional
        # (e.g., you only want to load the image.nhdr file and not the image.raw file).
        # In this case default node names are not generated.
        generateDefaultNodeNames = all(n is None for n in source.nodeNames)

        for uri, fileName, nodeName, checksum, loadFile, loadFileType in zip(
            source.uris, source.fileNames, source.nodeNames, source.checksums, source.loadFiles, source.loadFileTypes, strict=False):

            if nodeName is None or fileName is None:
                # Determine file basename and extension from URL or path
                import urllib
                import uuid
                if fileName is not None:
                    basename, ext = os.path.splitext(os.path.basename(fileName))
                else:
                    p = urllib.parse.urlparse(uri)
                    basename, ext = os.path.splitext(os.path.basename(p.path))

                # Generate default node name (we only need this if we want to load the file into the scene and no node name is provided)
                if (nodeName is None) and (loadFile is not False) and generateDefaultNodeNames:
                    nodeName = basename

                # Generate default file name (we always need this, even for just downloading)
                if fileName is None:
                    # Generate a unique filename to avoid overwriting existing file with the same name
                    fileName = f"{nodeName if nodeName else basename}-{uuid.uuid4().hex}{ext}"

            current_source = SampleDataSource(
                uris=uri,
                fileNames=fileName,
                nodeNames=nodeName,
                checksums=checksum,
                loadFiles=loadFile,
                loadFileTypes=loadFileType,
                loadFileProperties=source.loadFileProperties)

            for attemptsCount in range(maximumAttemptsCount):

                # Download
                try:
                    filePath = self.downloadFileIntoCache(uri, fileName, checksum, forceDownload=forceDownload)
                except ValueError:
                    self.logMessage(_("Download failed (attempt {current} of {total})...").format(
                        current=attemptsCount + 1, total=maximumAttemptsCount), logging.ERROR)
                    continue

                # Special behavior (how `loadFileType` is used and what is returned in `resultNodes` ) is implemented
                # for scene and zip file loading, for preserving backward compatible behavior.
                # - ZipFile: If `loadFile` is explicitly set to `False` then the zip file is just downloaded. Otherwise, the zip file is extracted.
                #   By default `loadFile` is set to `None`, so by default the zip file is extracted.
                #   Nodes are not loaded from the .zip file in either case. To load a scene from a .zip file, `loadFileType` has to be set explicitly to `SceneFile`.
                #   Path is returned in `resultNodes`.
                # - SceneFile: If `loadFile` is not explicitly set or it is set to `False` then the scene is just downloaded (not loaded).
                #   Path is returned in `resultNodes`.
                if (loadFileType is None) and (nodeName is None):
                    ext = os.path.splitext(fileName.lower())[1]
                    if ext in [".mrml", ".mrb"]:
                        loadFileType = "SceneFile"
                    elif ext in [".zip"]:
                        loadFileType = "ZipFile"

                if loadFileType == "ZipFile":
                    if loadFile is False:
                        resultNodes.append(filePath)
                        break
                    outputDir = slicer.mrmlScene.GetCacheManager().GetRemoteCacheDirectory() + "/" + os.path.splitext(os.path.basename(filePath))[0]
                    qt.QDir().mkpath(outputDir)
                    if slicer.util.extractArchive(filePath, outputDir):
                        # Success
                        resultNodes.append(outputDir)
                        break
                elif loadFileType == "SceneFile":
                    if not loadFile:
                        resultNodes.append(filePath)
                        break
                    if self.loadScene(filePath, source.loadFileProperties.copy()):
                        # Success
                        resultNodes.append(filePath)
                        break
                elif nodeName:
                    if loadFile is False:
                        resultNodes.append(filePath)
                        break
                    loadedNode = self.loadNode(filePath, nodeName, loadFileType, source.loadFileProperties.copy())
                    if loadedNode:
                        # Success
                        resultNodes.append(loadedNode)
                        break
                else:
                    # no need to load node
                    resultFilePaths.append(filePath)
                    break

                # Failed. Clean up downloaded file (it might have been a partial download)
                file = qt.QFile(filePath)
                if file.exists() and not file.remove():
                    self.logMessage(_("Load failed (attempt {current} of {total}). Unable to delete and try again loading {path}").format(
                        current=attemptsCount + 1, total=maximumAttemptsCount, path=filePath), logging.ERROR)
                    resultNodes.append(loadedNode)
                    break
                self.logMessage(_("Load failed (attempt {current} of {total})...").format(
                    current=attemptsCount + 1, total=maximumAttemptsCount), logging.ERROR)

        if resultNodes:
            return resultNodes
        else:
            return resultFilePaths

    def sourceForSampleName(self, sampleName):
        """For a given sample name this will search the available sources.
        Returns SampleDataSource instance.
        """
        for category in slicer.modules.sampleDataSources.keys():
            for source in slicer.modules.sampleDataSources[category]:
                if sampleName == source.sampleName:
                    return source
        return None

    def categoryForSource(self, a_source):
        """For a given SampleDataSource return the associated category name."""
        for category in slicer.modules.sampleDataSources.keys():
            for source in slicer.modules.sampleDataSources[category]:
                if a_source == source:
                    return category
        return None

    def downloadFromURL(self, uris=None, fileNames=None, nodeNames=None, checksums=None, loadFiles=None,
                        customDownloader=None, loadFileTypes=None, loadFileProperties={}, forceDownload=False):
        """Download and optionally load data into the application.

        :param uris: Download URL(s).
        :param fileNames: File name(s) that will be downloaded (and loaded).
        :param nodeNames: Node name(s) in the scene.
        :param checksums: Checksum(s) formatted as ``<algo>:<digest>`` to verify the downloaded file(s). For example, ``SHA256:cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93``.
        :param loadFiles: Boolean indicating if file(s) should be loaded. By default, the function decides.
        :param customDownloader: Custom function for downloading.
        :param loadFileTypes: file format name(s) (if not specified then the default file reader will be used).
        :param loadFileProperties: custom properties passed to the IO plugin.

        If the given ``fileNames`` are not found in the application cache directory, they
        are downloaded using the associated URIs.
        See ``slicer.mrmlScene.GetCacheManager().GetRemoteCacheDirectory()``

        If not explicitly provided or if set to ``None``, the ``loadFileTypes`` are
        guessed based on the corresponding filename extensions and file content.

        Special behavior for scene and archive files:
        - If a ``fileName`` has the ``.mrb`` or ``.mrml`` extension, it will **not** be loaded
          by default. To ensure the file is loaded, ``loadFiles`` must be set.
        - If a ``fileName`` has the ``.zip`` extension and ``loadFiles`` is set to ``True`` (or left at default value)
          then the archive is extracted and the folder that contains the files is returned.
          If ``loadFiles`` is set to ``False`` then the archive is not extracted and the filepath of the downloaded
          zip file is returned.

        The ``loadFileProperties`` are common for all files. If different properties
        need to be associated with files of different types, downloadFromURL must
        be called for each.

        If ``forceDownload`` is True then existing files are downloaded again (if no hash is provided).
        """
        source = SampleDataSource(
            uris=uris, fileNames=fileNames, nodeNames=nodeNames, loadFiles=loadFiles,
            loadFileTypes=loadFileTypes, loadFileProperties=loadFileProperties, checksums=checksums,
            customDownloader=customDownloader)
        return self.downloadFromSource(source, forceDownload=forceDownload)

    def downloadSample(self, sampleName):
        """For a given sample name this will search the available sources
        and load it if it is available.  Returns the first loaded node.
        """
        return self.downloadSamples(sampleName)[0]

    def downloadSamples(self, sampleName):
        """For a given sample name this will search the available sources
        and load it if it is available.  Returns the loaded nodes.
        """
        source = self.sourceForSampleName(sampleName)
        nodes = []
        if source:
            nodes = self.downloadFromSource(source)
        return nodes

    def logMessage(self, message, logLevel=logging.DEBUG):
        logging.log(logLevel, message)

    """Utility methods for backwards compatibility"""

    def downloadMRHead(self):
        return self.downloadSample("MRHead")

    def downloadCTChest(self):
        return self.downloadSample("CTChest")

    def downloadCTACardio(self):
        return self.downloadSample("CTACardio")

    def downloadDTIBrain(self):
        return self.downloadSample("DTIBrain")

    def downloadMRBrainTumor1(self):
        return self.downloadSample("MRBrainTumor1")

    def downloadMRBrainTumor2(self):
        return self.downloadSample("MRBrainTumor2")

    def downloadWhiteMatterExplorationBaselineVolume(self):
        return self.downloadSample("BaselineVolume")

    def downloadWhiteMatterExplorationDTIVolume(self):
        return self.downloadSample("DTIVolume")

    def downloadDiffusionMRIDWIVolume(self):
        return self.downloadSample("DWIVolume")

    def downloadAbdominalCTVolume(self):
        return self.downloadSample("CTAAbdomenPanoramix")

    def downloadDentalSurgery(self):
        # returns list since that's what earlier method did
        return self.downloadSamples("CBCTDentalSurgery")

    def downloadMRUSPostate(self):
        # returns list since that's what earlier method did
        return self.downloadSamples("MRUSProstate")

    def humanFormatSize(self, size):
        """from https://stackoverflow.com/questions/1094841/reusable-library-to-get-human-readable-version-of-file-size"""
        for x in ["bytes", "KB", "MB", "GB"]:
            if size < 1024.0 and size > -1024.0:
                return f"{size:3.1f} {x}"
            size /= 1024.0
        return "{:3.1f} {}".format(size, "TB")

    def reportHook(self, blocksSoFar, blockSize, totalSize):
        # we clamp to 100% because the blockSize might be larger than the file itself
        percent = min(int((100.0 * blocksSoFar * blockSize) / totalSize), 100)
        if percent == 100 or (percent - self.downloadPercent >= 10):
            # we clamp to totalSize when blockSize is larger than totalSize
            humanSizeSoFar = self.humanFormatSize(min(blocksSoFar * blockSize, totalSize))
            humanSizeTotal = self.humanFormatSize(totalSize)
            self.logMessage("<i>" + _("Downloaded {sizeCompleted} ({percentCompleted}% of {sizeTotal})...").format(
                            sizeCompleted=humanSizeSoFar, percentCompleted=percent, sizeTotal=humanSizeTotal) + "</i>")
            self.downloadPercent = percent

    @staticmethod
    def rewriteUrl(url):
        """Apply all registered URL rewrite rules to the given URL."""

        try:
            slicer.modules.sampleDataUrlRewriteRules
        except AttributeError:
            # No rules are registered
            return url

        for rule in slicer.modules.sampleDataUrlRewriteRules.values():
            url = rule(url)
        return url

    def downloadFile(self, uri, destFolderPath, name, checksum=None, forceDownload=False):
        """
        :param uri: Download URL. It is rewritten using registered URL rewrite rules before download.
        :param destFolderPath: Folder to download the file into.
        :param name: File name that will be downloaded.
        :param checksum: Checksum formatted as ``<algo>:<digest>`` to verify the downloaded file. For example, ``SHA256:cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93``.
        :param forceDownload: If True then the file is always downloaded even if it already exists in the destination folder.
        """

        uri = self.rewriteUrl(uri)

        self.downloadPercent = 0
        filePath = destFolderPath + "/" + name
        (algo, digest) = extractAlgoAndDigest(checksum)
        if forceDownload or not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
            import urllib.request, urllib.parse, urllib.error

            self.logMessage(_("Requesting download {name} from {uri} ...").format(name=name, uri=uri))
            try:
                urllib.request.urlretrieve(uri, filePath, self.reportHook)
                self.logMessage(_("Download finished"))
            except OSError as e:
                self.logMessage("\t" + _("Download failed: {errorMessage}").format(errorMessage=e), logging.ERROR)
                raise ValueError(_("Failed to download {uri} to {filePath}").format(uri=uri, filePath=filePath))

            if algo is not None:
                self.logMessage(_("Verifying checksum"))
                current_digest = computeChecksum(algo, filePath)
                if current_digest != digest:
                    self.logMessage(
                        _("Checksum verification failed. Computed checksum {currentChecksum} different from expected checksum {expectedChecksum}").format(
                            currentChecksum=current_digest, expectedChecksum=digest))
                    qt.QFile(filePath).remove()
                else:
                    self.downloadPercent = 100
                    self.logMessage(_("Checksum OK"))
        else:
            if algo is not None:
                self.logMessage(_("Verifying checksum"))
                current_digest = computeChecksum(algo, filePath)
                if current_digest != digest:
                    self.logMessage(_("File already exists in cache but checksum is different - re-downloading it."))
                    qt.QFile(filePath).remove()
                    return self.downloadFile(uri, destFolderPath, name, checksum)
                else:
                    self.downloadPercent = 100
                    self.logMessage(_("File already exists and checksum is OK - reusing it."))
            else:
                self.downloadPercent = 100
                self.logMessage(_("File already exists in cache - reusing it."))
        return filePath

    def loadScene(self, uri, fileProperties={}):
        """Returns True is scene loading was successful, False if failed."""
        loadedNode = self.loadNode(uri, None, "SceneFile", fileProperties)
        success = loadedNode is not None
        return success

    def loadNode(self, uri, name, fileType=None, fileProperties={}):
        """Returns the first loaded node (or the scene if the reader did not provide a specific node) on success.
        Returns None if failed.
        """
        self.logMessage("<b>" + _("Requesting load {name} from {uri} ...").format(name=name, uri=uri) + "</b>")

        fileProperties["fileName"] = uri
        if name:
            fileProperties["name"] = name
        if not fileType:
            fileType = slicer.app.coreIOManager().fileType(fileProperties["fileName"])
        firstLoadedNode = None
        loadedNodes = vtk.vtkCollection()
        success = slicer.app.coreIOManager().loadNodes(fileType, fileProperties, loadedNodes)
        if not success:
            if loadedNodes.GetNumberOfItems() < 1:
                self.logMessage("\t" + _("Load failed!"), logging.ERROR)
                return None
            else:
                # Loading did not fail, because some nodes were loaded, proceed with a warning
                self.logMessage(_("Error was reported while loading {count} nodes from {path}").format(
                                count=loadedNodes.GetNumberOfItems(), path=uri), logging.WARNING)

        self.logMessage("<b>" + _("Load finished") + "</b><p></p>")

        # since nodes were read from a temp directory remove the storage nodes
        for i in range(loadedNodes.GetNumberOfItems()):
            loadedNode = loadedNodes.GetItemAsObject(i)
            if not loadedNode.IsA("vtkMRMLStorableNode"):
                continue
            storageNode = loadedNode.GetStorageNode()
            if not storageNode:
                continue
            slicer.mrmlScene.RemoveNode(storageNode)
            loadedNode.SetAndObserveStorageNodeID(None)

        firstLoadedNode = loadedNodes.GetItemAsObject(0)
        if firstLoadedNode:
            return firstLoadedNode
        else:
            # If a reader does not report loading of any specific node (it may happen for example with a scene reader)
            # then return the scene to distinguish from a load error.
            return slicer.mrmlScene


class SampleDataTest(ScriptedLoadableModuleTest):
    """
    This is the test case for your scripted module.
    Uses ScriptedLoadableModuleTest base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    customDownloads = []

    def setUp(self):
        slicer.mrmlScene.Clear(0)
        SampleDataTest.customDownloads = []

    def runTest(self):
        for test in [
            self.test_downloadFromSource_downloadFiles,
            self.test_downloadFromSource_downloadZipFile,
            self.test_downloadFromSource_loadMRBFile,
            self.test_downloadFromSource_loadMRMLFile,
            self.test_downloadFromSource_downloadMRBFile,
            self.test_downloadFromSource_downloadMRMLFile,
            self.test_downloadFromSource_loadNode,
            self.test_downloadFromSource_loadNodeFromMultipleFiles,
            self.test_downloadFromSource_loadNodes,
            self.test_downloadFromSource_loadNodesWithLoadFileFalse,
            self.test_sampleDataSourcesByCategory,
            self.test_categoryVisibility,
            self.test_setCategoriesFromSampleDataSources,
            self.test_isSampleDataSourceRegistered,
            self.test_defaultFileType,
            self.test_customDownloader,
            self.test_categoryForSource,
        ]:
            self.setUp()
            test()

    @staticmethod
    def path2uri(path):
        """Gets a URI from a local file path.
        Typically it prefixes the received path by file:// or file:///.
        """
        import urllib.parse, urllib.request, urllib.parse, urllib.error

        return urllib.parse.urljoin("file:", urllib.request.pathname2url(path))

    def test_downloadFromSource_downloadFiles(self):
        """Specifying URIs and fileNames without nodeNames is expected to download the files
        without loading into Slicer.
        """
        logic = SampleDataLogic()

        sceneMTime = slicer.mrmlScene.GetMTime()
        filePaths = logic.downloadFromSource(SampleDataSource(
            uris=TESTING_DATA_URL + "SHA256/cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93",
            fileNames="MR-head.nrrd", loadFiles=False))
        self.assertEqual(len(filePaths), 1)
        self.assertTrue(os.path.exists(filePaths[0]))
        self.assertTrue(os.path.isfile(filePaths[0]))
        self.assertEqual(sceneMTime, slicer.mrmlScene.GetMTime())

        sceneMTime = slicer.mrmlScene.GetMTime()
        filePaths = logic.downloadFromSource(SampleDataSource(
            uris=[TESTING_DATA_URL + "SHA256/cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93",
                  TESTING_DATA_URL + "SHA256/4507b664690840abb6cb9af2d919377ffc4ef75b167cb6fd0f747befdb12e38e"],
            fileNames=["MR-head.nrrd", "CT-chest.nrrd"],
            loadFiles=[False, False]))
        self.assertEqual(len(filePaths), 2)
        self.assertTrue(os.path.exists(filePaths[0]))
        self.assertTrue(os.path.isfile(filePaths[0]))
        self.assertTrue(os.path.exists(filePaths[1]))
        self.assertTrue(os.path.isfile(filePaths[1]))
        self.assertEqual(sceneMTime, slicer.mrmlScene.GetMTime())

    def test_downloadFromSource_downloadZipFile(self):
        logic = SampleDataLogic()
        sceneMTime = slicer.mrmlScene.GetMTime()
        filePaths = logic.downloadFromSource(SampleDataSource(
            uris=TESTING_DATA_URL + "SHA256/b902f635ef2059cd3b4ba854c000b388e4a9e817a651f28be05c22511a317ec7",
            fileNames="TinyPatient_Seg.zip",
            loadFileTypes="ZipFile"))
        self.assertEqual(len(filePaths), 1)
        self.assertTrue(os.path.exists(filePaths[0]))
        self.assertTrue(os.path.isdir(filePaths[0]))
        self.assertEqual(sceneMTime, slicer.mrmlScene.GetMTime())

    def test_downloadFromSource_loadMRBFile(self):
        logic = SampleDataLogic()
        sceneMTime = slicer.mrmlScene.GetMTime()
        nodes = logic.downloadFromSource(SampleDataSource(
            uris=TESTING_DATA_URL + "SHA256/5a1c78c3347f77970b1a29e718bfa10e5376214692d55a7320af94b9d8d592b8",
            loadFiles=True, fileNames="slicer4minute.mrb"))
        self.assertEqual(len(nodes), 1)
        self.assertIsInstance(nodes[0], slicer.vtkMRMLCameraNode)
        self.assertTrue(sceneMTime < slicer.mrmlScene.GetMTime())

    def test_downloadFromSource_loadMRMLFile(self):
        logic = SampleDataLogic()
        tempFile = qt.QTemporaryFile(slicer.app.temporaryPath + "/SampleDataTest-loadSceneFile-XXXXXX.mrml")
        tempFile.open()
        tempFile.write(textwrap.dedent("""
      <?xml version="1.0" encoding="UTF-8"?>
      <MRML  version="Slicer4.4.0" userTags="">
      </MRML>
      """).strip())
        tempFile.close()
        sceneMTime = slicer.mrmlScene.GetMTime()
        nodes = logic.downloadFromSource(SampleDataSource(
            uris=self.path2uri(tempFile.fileName()), loadFiles=True, fileNames="scene.mrml"))
        self.assertEqual(len(nodes), 1)
        self.assertIsInstance(nodes[0], slicer.vtkMRMLScene)
        self.assertTrue(sceneMTime < slicer.mrmlScene.GetMTime())

    def test_downloadFromSource_downloadMRBFile(self):
        logic = SampleDataLogic()
        sceneMTime = slicer.mrmlScene.GetMTime()
        filePaths = logic.downloadFromSource(SampleDataSource(
            uris=TESTING_DATA_URL + "SHA256/5a1c78c3347f77970b1a29e718bfa10e5376214692d55a7320af94b9d8d592b8",
            fileNames="slicer4minute.mrb",
            loadFileTypes="SceneFile"))
        self.assertEqual(len(filePaths), 1)
        self.assertTrue(os.path.exists(filePaths[0]))
        self.assertTrue(os.path.isfile(filePaths[0]))
        self.assertEqual(sceneMTime, slicer.mrmlScene.GetMTime())

    def test_downloadFromSource_downloadMRMLFile(self):
        logic = SampleDataLogic()
        tempFile = qt.QTemporaryFile(slicer.app.temporaryPath + "/SampleDataTest-loadSceneFile-XXXXXX.mrml")
        tempFile.open()
        tempFile.write(textwrap.dedent("""
      <?xml version="1.0" encoding="UTF-8"?>
      <MRML  version="Slicer4.4.0" userTags="">
      </MRML>
      """).strip())
        tempFile.close()
        sceneMTime = slicer.mrmlScene.GetMTime()
        filePaths = logic.downloadFromSource(SampleDataSource(
            uris=self.path2uri(tempFile.fileName()), fileNames="scene.mrml", loadFileTypes="SceneFile"))
        self.assertEqual(len(filePaths), 1)
        self.assertTrue(os.path.exists(filePaths[0]))
        self.assertTrue(os.path.isfile(filePaths[0]))
        self.assertEqual(sceneMTime, slicer.mrmlScene.GetMTime())

    def test_downloadFromSource_loadNode(self):
        logic = SampleDataLogic()
        nodes = logic.downloadFromSource(SampleDataSource(
            uris=TESTING_DATA_URL + "MD5/39b01631b7b38232a220007230624c8e",
            fileNames="MR-head.nrrd", nodeNames="MRHead"))
        self.assertEqual(len(nodes), 1)
        self.assertEqual(nodes[0], slicer.mrmlScene.GetFirstNodeByName("MRHead"))

    def test_downloadFromSource_loadNodeWithoutNodeName(self):
        logic = SampleDataLogic()
        nodes = logic.downloadFromSource(SampleDataSource(
            uris=TESTING_DATA_URL + "MD5/39b01631b7b38232a220007230624c8e",
            fileNames="MR-head.nrrd"))
        self.assertEqual(len(nodes), 1)
        self.assertEqual(nodes[0], slicer.mrmlScene.GetFirstNodeByName("MR-head"))

    def test_downloadFromSource_loadNodeFromMultipleFiles(self):
        logic = SampleDataLogic()
        nodes = logic.downloadFromSource(SampleDataSource(
            uris=[TESTING_DATA_URL + "SHA256/bbfd8dd1914e9d7af09df3f6e254374064f6993a37552cc587581623a520a11f",
                  TESTING_DATA_URL + "SHA256/33825585b01a506e532934581a7bddd9de9e7b898e24adfed5454ffc6dfe48ea"],
            fileNames=["MRHeadResampled.raw.gz", "MRHeadResampled.nhdr"],
            nodeNames=[None, "MRHeadResampled"],
            loadFiles=[False, True]))
        self.assertEqual(len(nodes), 1)
        self.assertEqual(nodes[0], slicer.mrmlScene.GetFirstNodeByName("MRHeadResampled"))

    def test_downloadFromSource_loadNodesWithLoadFileFalse(self):
        logic = SampleDataLogic()
        nodes = logic.downloadFromSource(SampleDataSource(
            uris=[TESTING_DATA_URL + "SHA256/cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93",
                  TESTING_DATA_URL + "SHA256/4507b664690840abb6cb9af2d919377ffc4ef75b167cb6fd0f747befdb12e38e"],
            fileNames=["MR-head.nrrd", "CT-chest.nrrd"],
            nodeNames=["MRHead", "CTChest"],
            loadFiles=[False, True]))
        self.assertEqual(len(nodes), 2)
        self.assertTrue(os.path.exists(nodes[0]))
        self.assertTrue(os.path.isfile(nodes[0]))
        self.assertEqual(nodes[1], slicer.mrmlScene.GetFirstNodeByName("CTChest"))

    def test_downloadFromSource_loadNodes(self):
        logic = SampleDataLogic()
        nodes = logic.downloadFromSource(SampleDataSource(
            uris=[TESTING_DATA_URL + "SHA256/cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93",
                  TESTING_DATA_URL + "SHA256/4507b664690840abb6cb9af2d919377ffc4ef75b167cb6fd0f747befdb12e38e"],
            fileNames=["MR-head.nrrd", "CT-chest.nrrd"],
            nodeNames=["MRHead", "CTChest"]))
        self.assertEqual(len(nodes), 2)
        self.assertEqual(nodes[0], slicer.mrmlScene.GetFirstNodeByName("MRHead"))
        self.assertEqual(nodes[1], slicer.mrmlScene.GetFirstNodeByName("CTChest"))

    def test_downloadFromSource_loadNodesWithoutNodeNames(self):
        logic = SampleDataLogic()
        nodes = logic.downloadFromSource(SampleDataSource(
            uris=[TESTING_DATA_URL + "SHA256/cc211f0dfd9a05ca3841ce1141b292898b2dd2d3f08286affadf823a7e58df93",
                  TESTING_DATA_URL + "SHA256/4507b664690840abb6cb9af2d919377ffc4ef75b167cb6fd0f747befdb12e38e"],
            fileNames=["MR-head.nrrd", "CT-chest.nrrd"]))
        self.assertEqual(len(nodes), 2)
        self.assertEqual(nodes[0], slicer.mrmlScene.GetFirstNodeByName("MR-head"))
        self.assertEqual(nodes[1], slicer.mrmlScene.GetFirstNodeByName("CT-chest"))

    def test_sampleDataSourcesByCategory(self):
        self.assertTrue(len(SampleDataLogic.sampleDataSourcesByCategory()) > 0)
        self.assertTrue(len(SampleDataLogic.sampleDataSourcesByCategory("General")) > 0)
        self.assertTrue(len(SampleDataLogic.sampleDataSourcesByCategory("Not_A_Registered_Category")) == 0)

    def test_categoryVisibility(self):
        slicer.util.selectModule("SampleData")
        widget = slicer.modules.SampleDataWidget
        widget.setCategoryVisible("General", False)
        self.assertFalse(widget.isCategoryVisible("General"))
        widget.setCategoryVisible("General", True)
        self.assertTrue(widget.isCategoryVisible("General"))

    def test_setCategoriesFromSampleDataSources(self):
        slicer.util.selectModule("SampleData")
        widget = slicer.modules.SampleDataWidget
        self.assertGreater(widget.categoryLayout.count(), 0)

        SampleDataWidget.removeCategories(widget.categoryLayout)
        self.assertEqual(widget.categoryLayout.count(), 0)

        SampleDataWidget.setCategoriesFromSampleDataSources(widget.categoryLayout, slicer.modules.sampleDataSources, widget.logic)
        self.assertGreater(widget.categoryLayout.count(), 0)

    def test_isSampleDataSourceRegistered(self):
        if not slicer.app.testingEnabled():
            return
        sourceArguments = {
            "sampleName": "isSampleDataSourceRegistered",
            "uris": "https://slicer.org",
            "fileNames": "volume.nrrd",
            "loadFileTypes": "VolumeFile",
        }
        self.assertFalse(SampleDataLogic.isSampleDataSourceRegistered("Testing", SampleDataSource(**sourceArguments)))
        SampleDataLogic.registerCustomSampleDataSource(**sourceArguments, category="Testing")
        self.assertTrue(SampleDataLogic.isSampleDataSourceRegistered("Testing", SampleDataSource(**sourceArguments)))
        self.assertFalse(SampleDataLogic.isSampleDataSourceRegistered("Other", SampleDataSource(**sourceArguments)))

    def test_defaultFileType(self):
        """Test that file type is guessed correctly when not specified."""

        logic = SampleDataLogic()

        nodes = logic.downloadFromSource(SampleDataSource(
            uris=[TESTING_DATA_URL + "MD5/958737f8621437b18e4d8ab16eb65ad7"],
            fileNames=["brainMesh.vtk"],
            nodeNames=["brainMesh"]))
        self.assertEqual(len(nodes), 1)
        self.assertEqual(nodes[0].GetClassName(), "vtkMRMLModelNode")

        nodes = logic.downloadFromSource(SampleDataSource(
            uris=[TESTING_DATA_URL + "SHA256/3243b62bde36b1db1cdbfe204785bd4bc1fbb772558d5f8cac964cda8385d470"],
            fileNames=["TinyPatient_Structures.seg.nrrd"],
            nodeNames=["TinyPatient_Segments"]))
        self.assertEqual(len(nodes), 1)
        self.assertEqual(nodes[0].GetClassName(), "vtkMRMLSegmentationNode")

        nodes = logic.downloadFromSource(SampleDataSource(
            uris=[TESTING_DATA_URL + "SHA256/72fbc8c8e0e4fc7c3f628d833e4a6fb7adbe15b0bac2f8669f296e052414578c"],
            fileNames=["FastNonrigidBSplineregistrationTransform.tfm"],
            nodeNames=["FastNonrigidBSplineregistrationTransform"]))
        self.assertEqual(len(nodes), 1)
        self.assertEqual(nodes[0].GetClassName(), "vtkMRMLBSplineTransformNode")

    class CustomDownloader:
        def __call__(self, source):
            SampleDataTest.customDownloads.append(source)

    CustomDownloaderDataSource = {
        "category": "Testing",
        "sampleName": "customDownloader",
        "uris": "http://down.load/test",
        "fileNames": "cust.om",
        "customDownloader": CustomDownloader(),
    }

    def test_customDownloader(self):
        if not slicer.app.testingEnabled():
            return
        slicer.util.selectModule("SampleData")
        widget = slicer.modules.SampleDataWidget
        button = slicer.util.findChild(widget.parent, "customDownloaderPushButton")

        self.assertEqual(self.customDownloads, [])

        button.click()

        self.assertEqual(len(self.customDownloads), 1)
        self.assertEqual(self.customDownloads[0].sampleName, "customDownloader")

    def test_categoryForSource(self):
        logic = SampleDataLogic()
        source = slicer.modules.sampleDataSources[logic.builtInCategoryName][0]
        self.assertEqual(logic.categoryForSource(source), logic.builtInCategoryName)

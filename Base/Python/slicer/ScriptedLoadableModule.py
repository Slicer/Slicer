import importlib
import logging
import os
import unittest

import ctk
import qt
import vtk

import slicer
from slicer.i18n import tr as _


__all__ = ["ScriptedLoadableModule", "ScriptedLoadableModuleWidget", "ScriptedLoadableModuleLogic", "ScriptedLoadableModuleTest"]


class ScriptedLoadableModule:
    def __init__(self, parent):
        super().__init__()
        self.parent = parent
        self.moduleName = self.__class__.__name__

        parent.title = ""
        parent.categories = []
        parent.dependencies = []
        parent.contributors = ["Andras Lasso (PerkLab, Queen's University), Steve Pieper (Isomics)"]
        parent.helpText = _("""
This module was created from a template and the help section has not yet been updated.
""")

        parent.acknowledgementText = _("""
This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See {link} for details.
This work is partially supported by PAR-07-249: R01CA131718 NA-MIC Virtual Colonoscopy.
""").format(link="<a href=\"https://www.slicer.org\">https://www.slicer.org</a>")

        # Set module icon from possible icons, taking into account theme and extension
        # Search directories are not loaded yet, so we do this manually
        # We are keeping serach path loading in widget setup so that it plays nice
        # with module reload
        moduleDir = os.path.dirname(self.parent.path)
        for iconExtension in [".svg", ".png"]:
            iconPathTheme = os.path.join(moduleDir, "Resources/Icons", slicer.util.getCurrentSlicerTheme() ,self.moduleName + iconExtension)
            if os.path.isfile(iconPathTheme):
                parent.icon = qt.QIcon(iconPathTheme)
                break
            iconPathNoTheme = os.path.join(moduleDir, "Resources/Icons", self.moduleName + iconExtension)
            if os.path.isfile(iconPathNoTheme):
                parent.icon = qt.QIcon(iconPathNoTheme)
                break

        # Add this test to the SelfTest module's list for discovery when the module
        # is created.  Since this module may be discovered before SelfTests itself,
        # create the list if it doesn't already exist.
        try:
            slicer.selfTests
        except AttributeError:
            slicer.selfTests = {}
        slicer.selfTests[self.moduleName] = self.runTest

    def resourcePath(self, filename):
        """Return the absolute path of the module ``Resources`` directory."""
        scriptedModulesPath = os.path.dirname(self.parent.path)
        return os.path.join(scriptedModulesPath, "Resources", filename)

    def getDefaultModuleDocumentationLink(self, docPage=None):
        """Return string that can be inserted into the application help text that contains
        link to the module's documentation in current Slicer version's documentation.
        The text is "For more information see the online documentation."
        If docPage is not specified then the link points to URL returned by :func:`slicer.app.moduleDocumentationUrl`.
        """
        if docPage:
            url = slicer.app.documentationBaseUrl + docPage
        else:
            url = slicer.app.moduleDocumentationUrl(self.moduleName)
        linkText = ("<p>" + _("For more information, see the {url_start}online documentation{url_end}.")
                    .format(url_start=f'<a href="{url}">', url_end="</a>") + "</p>")
        return linkText

    def runTest(self, msec=100, **kwargs):
        """:param msec: delay to associate with :func:`ScriptedLoadableModuleTest.delayDisplay()`."""
        # Name of the test case class is expected to be <ModuleName>Test
        module = importlib.import_module(self.__module__)
        className = self.moduleName + "Test"
        try:
            TestCaseClass = getattr(module, className)
        except AttributeError:
            # Treat missing test case class as a failure; provide useful error message
            raise AssertionError(f"Test case class not found: {self.__module__}.{className} ")

        testCase = TestCaseClass()
        testCase.messageDelay = msec
        testCase.runTest(**kwargs)


class ScriptedLoadableModuleWidget:
    def __init__(self, parent=None):
        """If parent widget is not specified: a top-level widget is created automatically;
        the application has to delete this widget (by calling widget.parent.deleteLater() to avoid memory leaks.
        """
        super().__init__()
        # Get module name by stripping 'Widget' from the class name
        self.moduleName = self.__class__.__name__
        if self.moduleName.endswith("Widget"):
            self.moduleName = self.moduleName[:-6]
        self.developerMode = slicer.util.settingsValue("Developer/DeveloperMode", False, converter=slicer.util.toBool)
        if not parent:
            self.parent = slicer.qMRMLWidget()
            self.parent.setLayout(qt.QVBoxLayout())
            self.parent.setMRMLScene(slicer.mrmlScene)
        else:
            self.parent = parent
        self.layout = self.parent.layout()
        if not parent:
            self.setup()
            self.parent.show()
        slicer.app.moduleManager().connect(
            "moduleAboutToBeUnloaded(QString)", self._onModuleAboutToBeUnloaded)


    def iconPath(self, name):
        return f"{self.moduleName}:{name}"


    def resourcePath(self, filename):
        """Return the absolute path of the module ``Resources`` directory."""
        scriptedModulesPath = os.path.dirname(slicer.util.modulePath(self.moduleName))
        return os.path.join(scriptedModulesPath, "Resources", filename)

    def cleanup(self):
        """Override this function to implement module widget specific cleanup.

        It is invoked when the signal `qSlicerModuleManager::moduleAboutToBeUnloaded(QString)`
        corresponding to the current module is emitted and just before a module is
        effectively unloaded.
        """
        pass

    def _onModuleAboutToBeUnloaded(self, moduleName):
        """This slot calls `cleanup()` if the module about to be unloaded is the
        current one.
        """
        if moduleName == self.moduleName:
            self.cleanup()

            if self.developerMode and hasattr(self, "reloadTestMenuButton"):
                settings = qt.QSettings()
                settings.setValue(
                    f"{self.moduleName}/PreferredReloadAndTestAction",
                    self.reloadTestMenuButton.defaultAction().objectName)

            slicer.app.moduleManager().disconnect(
                "moduleAboutToBeUnloaded(QString)", self._onModuleAboutToBeUnloaded)

    def setupDeveloperSection(self):
        if not self.developerMode:
            return

        def createHLayout(elements):
            rowLayout = qt.QHBoxLayout()
            for element in elements:
                rowLayout.addWidget(element)
            return rowLayout

        #
        # Reload and Test area
        # Used during development, but hidden when delivering
        # developer mode is turned off.

        self.reloadCollapsibleButton = ctk.ctkCollapsibleButton()
        self.reloadCollapsibleButton.text = "Reload && Test"
        self.layout.addWidget(self.reloadCollapsibleButton)
        reloadFormLayout = qt.QFormLayout(self.reloadCollapsibleButton)

        # reload button
        self.reloadButton = qt.QPushButton("Reload")
        self.reloadButton.toolTip = "Reload this module."
        self.reloadButton.name = "ScriptedLoadableModuleTemplate Reload"
        self.reloadButton.connect("clicked()", self.onReload)

        # Toolbutton with either Test or Reload and Test
        self.reloadTestMenu = qt.QMenu(self)
        self.testAction = qt.QAction("Test", self.reloadTestMenu)
        self.testAction.setStatusTip("Test the current module")
        self.testAction.setObjectName("TestAction")
        self.testAction.triggered.connect(self.onTest)
        self.reloadTestMenu.addAction(self.testAction)

        self.reloadTestAction = qt.QAction("Reload and Test", self.reloadTestMenu)
        self.reloadTestAction.setStatusTip("Reload and test the current module")
        self.reloadTestAction.setObjectName("ReloadTestAction")
        self.reloadTestAction.triggered.connect(self.onReloadAndTest)
        self.reloadTestMenu.addAction(self.reloadTestAction)

        self.reloadTestMenuButton = qt.QToolButton()
        toolbuttonSizePolicy = qt.QSizePolicy(qt.QSizePolicy.Preferred, qt.QSizePolicy.Preferred)
        self.reloadTestMenuButton.setSizePolicy(toolbuttonSizePolicy)
        self.reloadTestMenuButton.toolTip = "Test or Reload & Test"
        self.reloadTestMenuButton.name = "ScriptedLoadableModuleTemplate Testing"
        self.reloadTestMenuButton.setMenu(self.reloadTestMenu)

        # Restore Last used action for this module
        defaultAction = self.reloadTestAction
        actions = [self.reloadTestAction, self.testAction]
        defaultActionName = qt.QSettings().value(f"{self.moduleName}/PreferredReloadAndTestAction", defaultAction.objectName)
        for action in actions:
            if action.objectName == defaultActionName:
                defaultAction = action
                break

        self.reloadTestMenuButton.setDefaultAction(defaultAction)

        # edit python source code
        self.editSourceButton = qt.QPushButton("Edit")
        self.editSourceButton.toolTip = ("Edit the module's source code. Shift+Click to copy the source code path to the clipboard."
                                         " The default editor can be changed in the Python section of the Application Settings.")
        self.editSourceButton.connect("clicked()", self.onEditSource)

        self.editModuleUiButton = None
        moduleUiFileName = self.resourcePath("UI/%s.ui" % self.moduleName)

        import os.path

        if os.path.isfile(moduleUiFileName):
            # Module UI file exists
            self.editModuleUiButton = qt.QPushButton("Edit UI")
            self.editModuleUiButton.toolTip = "Edit the module's .ui file."
            self.editModuleUiButton.connect("clicked()", lambda filename=moduleUiFileName: slicer.util.startQtDesigner(moduleUiFileName))

        # restart Slicer button
        # (use this during development, but remove it when delivering
        #  your module to users)
        self.restartButton = qt.QPushButton("Restart Slicer")
        self.restartButton.toolTip = "Restart Slicer"
        self.restartButton.name = "ScriptedLoadableModuleTemplate Restart"
        self.restartButton.connect("clicked()", slicer.app.restart)

        if self.editModuleUiButton:
            # There are many buttons, distribute them in two rows
            reloadFormLayout.addRow(createHLayout([self.reloadButton, self.reloadTestMenuButton, self.restartButton]))
            reloadFormLayout.addRow(createHLayout([self.editSourceButton, self.editModuleUiButton]))
        else:
            reloadFormLayout.addRow(createHLayout([self.reloadButton, self.reloadTestMenuButton, self.editSourceButton, self.restartButton]))

    def setup(self):
        # Instantiate and connect default widgets ...
        self.setupDeveloperSection()

        # Set up the icon paths
        qt.QDir.setSearchPaths(self.moduleName,[self.resourcePath(f"Icons/{slicer.util.getCurrentSlicerTheme()}"), self.resourcePath("Icons")])


    def onReload(self):
        """Reload scripted module widget representation."""

        # Print a clearly visible separator to make it easier
        # to distinguish new error messages (during/after reload)
        # from old ones.
        print("\n" * 2)
        print("-" * 30)
        print("Reloading module: " + self.moduleName)
        print("-" * 30)
        print("\n" * 2)

        slicer.util.reloadScriptedModule(self.moduleName)

    def onTest(self, **kwargs):
        """Tests scripted module widget, can be used when reload and test doesn't work, calls
        :func:`ScriptedLoadableModuleTest.runTest()` passing ``kwargs``.
        """
        if hasattr(self, "reloadTestMenuButton"):
            self.reloadTestMenuButton.setDefaultAction(self.testAction)
        with slicer.util.tryWithErrorDisplay("Test failed."):
            test = slicer.selfTests[self.moduleName]
            test(msec=int(slicer.app.userSettings().value("Developer/SelfTestDisplayMessageDelay")), **kwargs)

    def onReloadAndTest(self, **kwargs):
        """Reload scripted module widget representation and call :func:`ScriptedLoadableModuleTest.runTest()`
        passing ``kwargs``.
        """
        if hasattr(self, "reloadTestMenuButton"):
            self.reloadTestMenuButton.setDefaultAction(self.reloadTestAction)
        with slicer.util.tryWithErrorDisplay("Reload and Test failed."):
            self.onReload()
            test = slicer.selfTests[self.moduleName]
            test(msec=int(slicer.app.userSettings().value("Developer/SelfTestDisplayMessageDelay")), **kwargs)

    def onEditSource(self):
        filePath = slicer.util.modulePath(self.moduleName)

        # On Shift+Click copy the module path instead of opening it in the editor
        if slicer.app.keyboardModifiers() == qt.Qt.ShiftModifier:
            absFilePath = os.path.abspath(filePath)
            slicer.app.clipboard().setText(absFilePath)
            slicer.util.delayDisplay(f"Module file path '{absFilePath}' is copied to clipboard")
            return

        editor = slicer.app.settings().value("Python/Editor")
        if editor:
            # User specified a custom editor for .py files
            import subprocess

            try:
                # Use the startup environment to avoid Python environment issues with text editors implemented in Python
                subprocess.Popen([editor, filePath], env=slicer.util.startupEnvironment())
            except:
                slicer.util.errorDisplay(f"Failed to open file:\n\n{filePath}\n\nusing editor: {editor}")
        else:
            # Rely on the default file association for opening the .py file
            qt.QDesktopServices.openUrl(qt.QUrl("file:///" + filePath, qt.QUrl.TolerantMode))


class ScriptedLoadableModuleLogic:
    def __init__(self, parent=None):
        super().__init__()
        # Get module name by stripping 'Logic' from the class name
        self.moduleName = self.__class__.__name__
        if self.moduleName.endswith("Logic"):
            self.moduleName = self.moduleName[:-5]

        # If parameter node is singleton then only one parameter node
        # is allowed in a scene.
        # Derived classes can set self.isSingletonParameterNode = False
        # to allow having multiple parameter nodes in the scene.
        self.isSingletonParameterNode = True

    def getParameterNode(self):
        """
        Return the first available parameter node for this module
        If no parameter nodes are available for this module then a new one is created.
        """
        if self.isSingletonParameterNode:
            parameterNode = slicer.mrmlScene.GetSingletonNode(self.moduleName, "vtkMRMLScriptedModuleNode")
            if parameterNode:
                # After close scene, ModuleName attribute may be removed, restore it now
                if parameterNode.GetAttribute("ModuleName") != self.moduleName:
                    parameterNode.SetAttribute("ModuleName", self.moduleName)
                return parameterNode
        else:
            numberOfScriptedModuleNodes = slicer.mrmlScene.GetNumberOfNodesByClass("vtkMRMLScriptedModuleNode")
            for nodeIndex in range(numberOfScriptedModuleNodes):
                parameterNode = slicer.mrmlScene.GetNthNodeByClass(nodeIndex, "vtkMRMLScriptedModuleNode")
                if parameterNode.GetAttribute("ModuleName") == self.moduleName:
                    return parameterNode
        # no parameter node was found for this module, therefore we add a new one now
        parameterNode = slicer.mrmlScene.AddNode(self.createParameterNode())
        return parameterNode

    def getAllParameterNodes(self):
        """
        Return a list of all parameter nodes for this module
        Multiple parameter nodes are useful for storing multiple parameter sets in a single scene.
        """
        foundParameterNodes = []
        numberOfScriptedModuleNodes = slicer.mrmlScene.GetNumberOfNodesByClass("vtkMRMLScriptedModuleNode")
        for nodeIndex in range(numberOfScriptedModuleNodes):
            parameterNode = slicer.mrmlScene.GetNthNodeByClass(nodeIndex, "vtkMRMLScriptedModuleNode")
            if parameterNode.GetAttribute("ModuleName") == self.moduleName:
                foundParameterNodes.append(parameterNode)
        return foundParameterNodes

    def createParameterNode(self):
        """
        Create a new parameter node
        The node is of vtkMRMLScriptedModuleNode class. Module name is added as an attribute to allow filtering
        in node selector widgets (attribute name: ModuleName, attribute value: the module's name).
        This method can be overridden in derived classes to create a default parameter node with all
        parameter values set to their default.
        """
        if slicer.mrmlScene is None:
            return

        node = slicer.mrmlScene.CreateNodeByClass("vtkMRMLScriptedModuleNode")
        node.UnRegister(None)  # object is owned by the Python variable now
        if self.isSingletonParameterNode:
            node.SetSingletonTag(self.moduleName)
        # Add module name in an attribute to allow filtering in node selector widgets
        # Note that SetModuleName is not used anymore as it would be redundant with the ModuleName attribute.
        node.SetAttribute("ModuleName", self.moduleName)
        node.SetName(slicer.mrmlScene.GenerateUniqueName(self.moduleName))
        return node


class ScriptedLoadableModuleTest(unittest.TestCase):
    """
    Base class for module tester class.
    Setting messageDelay to something small, like 50ms allows
    faster development time.
    """

    def __init__(self, *args, **kwargs):
        self.moduleTestNames = []
        super().__init__(*args, **kwargs)

        # See https://github.com/Slicer/Slicer/pull/6243#issuecomment-1061800718 for more information.
        # Do not pass *args, **kwargs since there is no base class after `unittest.TestCase`. This is only relevant to
        # mixins of derived classes, which should each have the same signature as `object()`.
        super(unittest.TestCase, self).__init__()

        # takeScreenshot default parameters
        self.enableScreenshots = False
        self.screenshotScaleFactor = 1.0
        if not self.moduleTestNames:
            self.moduleTestNames = [method for method in dir(self) if "test" in method and callable(getattr(self, method))]

    def delayDisplay(self, message, requestedDelay=None, msec=None):
        """
        Display messages to the user/tester during testing.

        By default, the delay is 50ms.

        The function accepts the keyword arguments ``requestedDelay`` or ``msec``. If both
        are specified, the value associated with ``msec`` is used.

        This method can be temporarily overridden to allow tests running
        with longer or shorter message display time.

        Displaying a dialog and waiting does two things:
        1) it lets the event loop catch up to the state of the test so
        that rendering and widget updates have all taken place before
        the test continues and
        2) it shows the user/developer/tester the state of the test
        so that we'll know when it breaks.

        Note:
        Information that might be useful (but not important enough to show
        to the user) can be logged using logging.info() function
        (printed to console and application log) or logging.debug()
        function (printed to application log only).
        Error messages should be logged by logging.error() function
        and displayed to user by slicer.util.errorDisplay function.
        """
        if hasattr(self, "messageDelay"):
            msec = self.messageDelay
        if msec is None:
            msec = requestedDelay
        if msec is None:
            msec = 100

        slicer.util.delayDisplay(message, msec)

    def takeScreenshot(self, name, description, type=-1):
        """Take a screenshot of the selected viewport and store as and
        annotation snapshot node. Convenience method for automated testing.

        If self.enableScreenshots is False then only a message is displayed but screenshot
        is not stored. Screenshots are scaled by self.screenshotScaleFactor.

        :param name: snapshot node name
        :param description: description of the node
        :param type: which viewport to capture. If not specified then captures the entire window.
            Valid values: slicer.qMRMLScreenShotDialog.FullLayout,
            slicer.qMRMLScreenShotDialog.ThreeD, slicer.qMRMLScreenShotDialog.Red,
            slicer.qMRMLScreenShotDialog.Yellow, slicer.qMRMLScreenShotDialog.Green.
        """

        # show the message even if not taking a screen shot
        self.delayDisplay(description)

        if not self.enableScreenshots:
            return

        lm = slicer.app.layoutManager()
        # switch on the type to get the requested window
        widget = 0
        if type == slicer.qMRMLScreenShotDialog.FullLayout:
            # full layout
            widget = lm.viewport()
        elif type == slicer.qMRMLScreenShotDialog.ThreeD:
            # just the 3D window
            widget = lm.threeDWidget(0).threeDView()
        elif type == slicer.qMRMLScreenShotDialog.Red:
            # red slice window
            widget = lm.sliceWidget("Red")
        elif type == slicer.qMRMLScreenShotDialog.Yellow:
            # yellow slice window
            widget = lm.sliceWidget("Yellow")
        elif type == slicer.qMRMLScreenShotDialog.Green:
            # green slice window
            widget = lm.sliceWidget("Green")
        else:
            # default to using the full window
            widget = slicer.util.mainWindow()
            # reset the type so that the node is set correctly
            type = slicer.qMRMLScreenShotDialog.FullLayout

        # grab and convert to vtk image data
        qimage = ctk.ctkWidgetsUtils.grabWidget(widget)
        imageData = vtk.vtkImageData()
        slicer.qMRMLUtils().qImageToVtkImageData(qimage, imageData)

        annotationLogic = slicer.modules.annotations.logic()
        annotationLogic.CreateSnapShot(name, description, type, self.screenshotScaleFactor, imageData)

    def setUp(self):
        """
        Method called to prepare the test fixture. This is called immediately before calling the test method; other
        than AssertionError or SkipTest, any exception raised by this method will be considered an error rather than
        a test failure. The default implementation does nothing.
        """
        return

    def tearDown(self):
        """
        Method called immediately after the test method has been called and the result recorded. This is called
        even if the test method raised an exception, so the implementation in subclasses may need to be
        particularly careful about checking internal state. Any exception, other than AssertionError or
        SkipTest, raised by this method will be considered an additional error rather than a test failure
        (thus increasing the total number of reported errors). This method will only be called if the setUp()
        succeeds, regardless of the outcome of the test method. The default implementation does nothing.
        """
        return

    def runTest(self):
        """Run a default selection of tests here."""
        if len(self.moduleTestNames) == 0:
            logging.warning("No test is defined in " + self.__class__.__name__)
            return
        self.setUpClass()
        for test_name in self.moduleTestNames:
            self.setUp()
            logging.info(f"Running {test_name}")
            getattr(self, test_name)()
            self.tearDown()
        self.tearDownClass()

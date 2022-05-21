import logging

import ctk
import qt

import slicer
from slicer.ScriptedLoadableModule import *


#
# PluggableMarkupsSelfTest
#
class PluggableMarkupsSelfTest(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "PluggableMarkupsSelfTest"
        self.parent.categories = ["Testing.TestCases"]
        self.parent.dependencies = []
        self.parent.contributors = ["Rafael Palomar (OUS)"]
        self.parent.helpText = """
    This is a test case for the markups pluggable architecture.
    It unregisters the markups provided by the Markups module and
    registers them again.
    """
        self.parent.acknowledgementText = """
    This file was originally developed by Rafael Palomar (OUS) and was funded by
    the Research Council of Norway (grant nr. 311393).
    """


#
# PluggableMarkupsSelfTestWidget
#
class PluggableMarkupsSelfTestWidget(ScriptedLoadableModuleWidget):

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)

        # Instantiate and connect widgets ...

        #
        # Parameters Area
        #
        parametersCollapsibleButton = ctk.ctkCollapsibleButton()
        parametersCollapsibleButton.text = "Parameters"
        self.layout.addWidget(parametersCollapsibleButton)

        # Layout within the dummy collapsible button
        parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)

        # Apply Button
        #
        self.applyButton = qt.QPushButton("Apply")
        self.applyButton.toolTip = "Run the test."
        self.applyButton.enabled = True
        parametersFormLayout.addRow(self.applyButton)

        # connections
        self.applyButton.connect('clicked(bool)', self.onApplyButton)

        # Add vertical spacer
        self.layout.addStretch(1)

    def cleanup(self):
        pass

    def onApplyButton(self):
        logging.debug("Execute logic.run() method")
        logic = PluggableMarkupsSelfTestLogic()
        logic.run()


class PluggableMarkupsSelfTestLogic(ScriptedLoadableModuleLogic):

    def __init__(self):
        ScriptedLoadableModuleLogic.__init__(self)

    def setUp(self):
        #
        # Step 1: Register all available markups nodes
        #

        markupsWidget = slicer.modules.markups.widgetRepresentation()
        if markupsWidget is None:
            raise Exception("Couldn't get the Markups module widget")

        markupsLogic = slicer.modules.markups.logic()
        if markupsLogic is None:
            raise Exception("Couldn't get the Markups module logic")

        markupsNodes = self.markupsNodes()

        # Check Markups module standard nodes are registered
        for markupNode in markupsNodes:
            markupsLogic.RegisterMarkupsNode(markupNode, markupsNodes[markupNode])

        #
        # Step 2: Register all available additional options widgets
        #
        additionalOptionsWidgetsFactory = slicer.qMRMLMarkupsOptionsWidgetsFactory().instance()
        for additionalOptionsWidget in self.additionalOptionsWidgets():
            additionalOptionsWidgetsFactory.registerOptionsWidget(additionalOptionsWidget)

    def __checkPushButtonExists(self, widget, name):
        pushButtonObjectName = "Create%sPushButton" % name
        # slicer.util.delayDisplay("Checking whether '%s' exists" % pushButtonObjectName)
        if widget.findChild(qt.QPushButton, pushButtonObjectName):
            return True
        return False

    def __checkWidgetExists(self, widget, name):
        # slicer.util.delayDisplay("Checking whether '%s' exists" % name)
        if widget.findChild(qt.QWidget, name):
            return True
        return False

    def __checkWidgetVisibility(self, widget, name):
        # slicer.util.delayDisplay("Checking whether '%s' is visible" % pushButtonObjectName)
        w = widget.findChild(qt.QWidget, name)
        return w.isVisible()

    def markupsNodes(self):
        return {
            slicer.vtkMRMLMarkupsAngleNode(): slicer.vtkSlicerAngleWidget(),
            slicer.vtkMRMLMarkupsClosedCurveNode(): slicer.vtkSlicerCurveWidget(),
            slicer.vtkMRMLMarkupsCurveNode(): slicer.vtkSlicerCurveWidget(),
            slicer.vtkMRMLMarkupsFiducialNode(): slicer.vtkSlicerPointsWidget(),
            slicer.vtkMRMLMarkupsLineNode(): slicer.vtkSlicerLineWidget(),
            slicer.vtkMRMLMarkupsPlaneNode(): slicer.vtkSlicerPlaneWidget(),
            slicer.vtkMRMLMarkupsROINode(): slicer.vtkSlicerROIWidget(),
            slicer.vtkMRMLMarkupsTestLineNode(): slicer.vtkSlicerTestLineWidget()
        }

    def additionalOptionsWidgets(self):
        return [
            slicer.qMRMLMarkupsCurveSettingsWidget(),
            slicer.qMRMLMarkupsAngleMeasurementsWidget(),
            slicer.qMRMLMarkupsPlaneWidget(),
            slicer.qMRMLMarkupsROIWidget(),
            slicer.qMRMLMarkupsTestLineWidget()
        ]

    def test_unregister_existing_markups(self):
        """
        This unregisters existing registered markups
        """

        markupsWidget = slicer.modules.markups.widgetRepresentation()
        if markupsWidget is None:
            raise Exception("Couldn't get the Markups module widget")

        # Check Markups module standard nodes are registered
        for markupNode in self.markupsNodes():
            if self.__checkPushButtonExists(markupsWidget, markupNode.GetMarkupType()) is None:
                raise Exception("Create PushButton for %s is not present" % markupNode.GetMarkupType())

        markupsLogic = slicer.modules.markups.logic()
        if markupsLogic is None:
            raise Exception("Couldn't get the Markups module logic")

        # Unregister Markups and check the buttons are gone
        for markupNode in self.markupsNodes():
            slicer.util.delayDisplay("Unregistering %s" % markupNode.GetMarkupType())
            markupsLogic.UnregisterMarkupsNode(markupNode)
            if self.__checkPushButtonExists(markupsWidget, markupNode.GetMarkupType()):
                raise Exception("Create PushButton for %s is present after unregistration" % markupNode.GetMarkupType())

    def test_register_markups(self):
        """
        This registers all known markups
        """
        markupsWidget = slicer.modules.markups.widgetRepresentation()
        if markupsWidget is None:
            raise Exception("Couldn't get the Markups module widget")

        markupsLogic = slicer.modules.markups.logic()
        if markupsLogic is None:
            raise Exception("Couldn't get the Markups module logic")

        markupsNodes = self.markupsNodes()

        # Check Markups module standard nodes are registered
        for markupNode in markupsNodes:
            slicer.util.delayDisplay("Registering %s" % markupNode.GetMarkupType())
            markupsLogic.RegisterMarkupsNode(markupNode, markupsNodes[markupNode])
            if self.__checkPushButtonExists(markupsWidget, markupNode.GetMarkupType()) is None:
                raise Exception("Create PushButton for %s is not present" % markupNode.GetMarkupType())

    def test_unregister_additional_options_widgets(self):
        """
        This unregisters all the additional options widgets
        """
        markupsWidget = slicer.modules.markups.widgetRepresentation()
        if markupsWidget is None:
            raise Exception("Couldn't get the Markups module widget")

        additionalOptionsWidgetsFactory = slicer.qMRMLMarkupsOptionsWidgetsFactory().instance()
        for additionalOptionsWidget in self.additionalOptionsWidgets():

            # Check the widget exists
            if not self.__checkWidgetExists(markupsWidget, additionalOptionsWidget.objectName):
                raise Exception("%s does not exist" % additionalOptionsWidget.objectName)

            # NOTE: since the widget will get destroyed, we take note of the name for the checking step
            objectName = additionalOptionsWidget.objectName

            # Unregister widget
            additionalOptionsWidgetsFactory.unregisterOptionsWidget(additionalOptionsWidget.className)

            # Check the widget does not exist
            if self.__checkWidgetExists(markupsWidget, objectName):
                raise Exception("%s does still exist" % objectName)

    def test_register_additional_options_widgets(self):
        """
        This reigisters additional options widgets
        """

        additionalOptionsWidgetsFactory = slicer.qMRMLMarkupsOptionsWidgetsFactory().instance()

        markupsWidget = slicer.modules.markups.widgetRepresentation()
        if markupsWidget is None:
            raise Exception("Couldn't get the Markups module widget")

        for additionalOptionsWidget in self.additionalOptionsWidgets():
            name = additionalOptionsWidget.objectName
            slicer.util.delayDisplay("Registering %s" % additionalOptionsWidget.objectName)
            additionalOptionsWidgetsFactory.registerOptionsWidget(additionalOptionsWidget)

            # Check the widget exists
            if not self.__checkWidgetExists(markupsWidget, name):
                raise Exception("%s does not exist" % additionalOptionsWidget.objectName)

    def run(self):
        """
        Run the tests
        """
        slicer.util.delayDisplay('Running integration tests for Pluggable Markups')

        self.test_unregister_existing_markups()
        self.test_register_markups()
        # self.test_unregister_additional_options_widgets()
        self.test_register_additional_options_widgets()

        logging.info('Process completed')


class PluggableMarkupsSelfTestTest(ScriptedLoadableModuleTest):
    """
    This is the test case
    """

    def setUp(self):
        logic = PluggableMarkupsSelfTestLogic()
        logic.setUp()

    def runTest(self):
        self.setUp()
        self.test_PluggableMarkupsSelfTest1()

    def test_PluggableMarkupsSelfTest1(self):

        self.delayDisplay("Starting the Pluggable Markups Test")

        # Open the markups module
        slicer.util.mainWindow().moduleSelector().selectModule('Markups')
        self.delayDisplay('In Markups module')

        logic = PluggableMarkupsSelfTestLogic()
        logic.run()

        self.delayDisplay('Test passed!')

import ctk
import qt

import slicer
from slicer.ScriptedLoadableModule import *

#
# ColorLegendSelfTest
#


class ColorLegendSelfTest(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = "ColorLegendSelfTest"
        self.parent.categories = ["Testing.TestCases"]
        self.parent.dependencies = []
        self.parent.contributors = ["Kevin Wang (PMH), Nicole Aucoin (BWH), Mikhail Polkovnikov (IHEP)"]
        self.parent.helpText = """
    This is a test case for the new vtkSlicerScalarBarActor class.
    It iterates through all the color nodes and sets them active in the
    Colors module while the color legend widget is displayed.
    """
        self.parent.acknowledgementText = """
    This file was originally developed by Kevin Wang, PMH and was funded by CCO and OCAIRO.
"""  # replace with organization, grant and thanks.

#
# ColorLegendSelfTestWidget
#


class ColorLegendSelfTestWidget(ScriptedLoadableModuleWidget):

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
        self.applyButton.toolTip = "Run the algorithm."
        self.applyButton.enabled = True
        parametersFormLayout.addRow(self.applyButton)

        # connections
        self.applyButton.connect('clicked(bool)', self.onApplyButton)

        # Add vertical spacer
        self.layout.addStretch(1)

    def cleanup(self):
        pass

    def onApplyButton(self):
        test = ColorLegendSelfTestTest()
        print("Run the test algorithm")
        test.test_ColorLegendSelfTest1()


class ColorLegendSelfTestTest(ScriptedLoadableModuleTest):
    """
    This is the test case for your scripted module.
    """

    def setUp(self):
        """ Do whatever is needed to reset the state - typically a scene clear will be enough.
        """
        slicer.mrmlScene.Clear(0)
        # Timeout delay
        self.delayMs = 700

    def runTest(self):
        """Run as few or as many tests as needed here.
        """
        self.setUp()
        self.test_ColorLegendSelfTest1()

    def test_ColorLegendSelfTest1(self):

        self.delayDisplay("Starting test_ColorLegendSelfTest1")

        self.delayDisplay('Load CTChest sample volume')
        import SampleData
        sampleDataLogic = SampleData.SampleDataLogic()
        ctVolumeNode = sampleDataLogic.downloadCTChest()
        self.assertIsNotNone(ctVolumeNode)

        self.delayDisplay('Switch to Colors module')
        m = slicer.util.mainWindow()
        m.moduleSelector().selectModule('Colors')

        # Get widgets for testing via GUI
        colorWidget = slicer.modules.colors.widgetRepresentation()
        activeColorNodeSelector = slicer.util.findChildren(colorWidget, 'ColorTableComboBox')[0]
        self.assertIsNotNone(activeColorNodeSelector)
        activeDisplayableNodeSelector = slicer.util.findChildren(colorWidget, 'DisplayableNodeComboBox')[0]
        self.assertIsNotNone(activeDisplayableNodeSelector)
        createColorLegendButton = slicer.util.findChildren(colorWidget, 'CreateColorLegendButton')[0]
        self.assertIsNotNone(createColorLegendButton)
        useCurrentColorsButton = slicer.util.findChildren(colorWidget, 'UseCurrentColorsButton')[0]
        self.assertIsNotNone(useCurrentColorsButton)
        colorLegendDisplayNodeWidget = slicer.util.findChildren(colorWidget, 'ColorLegendDisplayNodeWidget')[0]
        self.assertIsNotNone(colorLegendDisplayNodeWidget)
        colorLegendVisibilityCheckBox = slicer.util.findChildren(colorLegendDisplayNodeWidget, 'ColorLegendVisibilityCheckBox')[0]
        self.assertIsNotNone(colorLegendVisibilityCheckBox)

        self.delayDisplay('Show color legend on all views and slices', self.delayMs)
        activeDisplayableNodeSelector.setCurrentNode(ctVolumeNode)
        createColorLegendButton.click()
        self.assertTrue(colorLegendVisibilityCheckBox.checked)

        self.delayDisplay('Iterate over the color nodes and set each one active', self.delayMs)
        shortDelayMs = 5
        # There are many color nodes, we don't test each to make the test complete faster
        testedColorNodeIndices = list(range(0, 60, 3))
        for ind, n in enumerate(testedColorNodeIndices):
            colorNode = slicer.mrmlScene.GetNthNodeByClass(n, 'vtkMRMLColorNode')
            self.delayDisplay(f"Setting color node {colorNode.GetName()} ({ind}/{len(testedColorNodeIndices)}) for the displayable node", shortDelayMs)
            activeColorNodeSelector.setCurrentNodeID(colorNode.GetID())
            # use the delay display here to ensure a render
            useCurrentColorsButton.click()

        self.delayDisplay('Test color legend visibility', self.delayMs)
        colorLegend = slicer.modules.colors.logic().GetColorLegendDisplayNode(ctVolumeNode)
        self.assertIsNotNone(colorLegend)

        self.delayDisplay('Exercise color legend updates via MRML', self.delayMs)
        # signal to displayable manager to show a created color legend
        colorLegend.SetMaxNumberOfColors(256)
        colorLegend.SetVisibility(True)

        self.delayDisplay('Show color legend in Red slice and 3D views only', self.delayMs)
        sliceNodeRed = slicer.app.layoutManager().sliceWidget('Red').mrmlSliceNode()
        self.assertIsNotNone(sliceNodeRed)
        threeDViewNode = slicer.app.layoutManager().threeDWidget(0).mrmlViewNode()
        self.assertIsNotNone(threeDViewNode)
        colorLegend.SetViewNodeIDs([sliceNodeRed.GetID(), threeDViewNode.GetID()])

        self.delayDisplay('Show color legend in the 3D view only', self.delayMs)
        colorLegend.SetViewNodeIDs([threeDViewNode.GetID()])
        self.delayDisplay('Test color legend on 3D view finished!', self.delayMs)

        # Test showing color legend only in a single slice node
        sliceNameColor = {
            'Red': [1., 0., 0.],
            'Green': [0., 1., 0.],
            'Yellow': [1., 1., 0.]
        }
        for sliceName, titleColor in sliceNameColor.items():
            self.delayDisplay('Test color legend on the ' + sliceName + ' slice view', self.delayMs)
            sliceNode = slicer.app.layoutManager().sliceWidget(sliceName).mrmlSliceNode()
            colorLegend.SetViewNodeIDs([sliceNode.GetID()])
            colorLegend.SetTitleText(sliceName)
            colorLegend.GetTitleTextProperty().SetColor(titleColor)
            self.delayDisplay('Test color legend on the ' + sliceName + ' slice view finished!', self.delayMs * 2)

        colorLegend.SetVisibility(False)

        self.delayDisplay('Test passed!')

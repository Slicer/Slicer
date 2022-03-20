import time

import ctk
import qt
import vtk

import slicer
from slicer.ScriptedLoadableModule import *


#
# AddManyMarkupsFiducialTest
#

class AddManyMarkupsFiducialTest(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        parent.title = "AddManyMarkupsFiducialTest"
        parent.categories = ["Testing.TestCases"]
        parent.dependencies = []
        parent.contributors = ["Nicole Aucoin (BWH)"]
        parent.helpText = """
    This is a test case that adds many Markup Fiducials to the scene and times the operation.
    """
        parent.acknowledgementText = """
    This file was originally developed by Nicole Aucoin, BWH and was partially funded by NIH grant 3P41RR013218-12S1.
"""  # replace with organization, grant and thanks.


#
# qAddManyMarkupsFiducialTestWidget
#

class AddManyMarkupsFiducialTestWidget(ScriptedLoadableModuleWidget):
    """Uses ScriptedLoadableModuleWidget base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

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

        #
        # node type to add
        #
        self.nodeTypeComboBox = qt.QComboBox()
        self.nodeTypeComboBox.addItem("vtkMRMLMarkupsFiducialNode")
        self.nodeTypeComboBox.addItem("vtkMRMLMarkupsLineNode")
        self.nodeTypeComboBox.addItem("vtkMRMLMarkupsAngleNode")
        self.nodeTypeComboBox.addItem("vtkMRMLMarkupsCurveNode")
        self.nodeTypeComboBox.addItem("vtkMRMLMarkupsClosedCurveNode")
        self.nodeTypeComboBox.addItem("vtkMRMLMarkupsROINode")
        parametersFormLayout.addRow("Node type: ", self.nodeTypeComboBox)

        #
        # number of nodes to add
        #
        self.numberOfNodesSliderWidget = ctk.ctkSliderWidget()
        self.numberOfNodesSliderWidget.singleStep = 1.0
        self.numberOfNodesSliderWidget.decimals = 0
        self.numberOfNodesSliderWidget.minimum = 0.0
        self.numberOfNodesSliderWidget.maximum = 1000.0
        self.numberOfNodesSliderWidget.value = 1.0
        self.numberOfNodesSliderWidget.toolTip = "Set the number of nodes to add."
        parametersFormLayout.addRow("Number of nodes: ", self.numberOfNodesSliderWidget)

        #
        # number of fiducials to add
        #
        self.numberOfControlPointsSliderWidget = ctk.ctkSliderWidget()
        self.numberOfControlPointsSliderWidget.singleStep = 1.0
        self.numberOfControlPointsSliderWidget.decimals = 0
        self.numberOfControlPointsSliderWidget.minimum = 0.0
        self.numberOfControlPointsSliderWidget.maximum = 10000.0
        self.numberOfControlPointsSliderWidget.value = 500.0
        self.numberOfControlPointsSliderWidget.toolTip = "Set the number of control points to add per node."
        parametersFormLayout.addRow("Number of control points: ", self.numberOfControlPointsSliderWidget)

        #
        # check box to trigger fewer modify events, adding all the new points
        # is wrapped inside of a StartModify/EndModify block
        #
        self.fewerModifyFlagCheckBox = qt.QCheckBox()
        self.fewerModifyFlagCheckBox.checked = 0
        self.fewerModifyFlagCheckBox.toolTip = 'If checked, wrap adding points inside of a StartModify - EndModify block'
        parametersFormLayout.addRow("Fewer modify events: ", self.fewerModifyFlagCheckBox)

        #
        # markups locked
        #
        self.lockedFlagCheckBox = qt.QCheckBox()
        self.lockedFlagCheckBox.checked = 0
        self.lockedFlagCheckBox.toolTip = 'If checked, markups will be locked for editing'
        parametersFormLayout.addRow("Locked nodes: ", self.lockedFlagCheckBox)

        #
        # markups labels hidden
        #
        self.labelsHiddenFlagCheckBox = qt.QCheckBox()
        self.labelsHiddenFlagCheckBox.checked = 0
        self.labelsHiddenFlagCheckBox.toolTip = 'If checked, markups labels will be forced to be hidden, regardless of default markups properties'
        parametersFormLayout.addRow("Labels hidden: ", self.labelsHiddenFlagCheckBox)

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
        logic = AddManyMarkupsFiducialTestLogic()
        nodeType = self.nodeTypeComboBox.currentText
        numberOfNodes = int(self.numberOfNodesSliderWidget.value)
        numberOfControlPoints = int(self.numberOfControlPointsSliderWidget.value)
        fewerModifyFlag = self.fewerModifyFlagCheckBox.checked
        labelsHiddenFlag = self.labelsHiddenFlagCheckBox.checked
        locked = self.lockedFlagCheckBox.checked
        print(f"Run the logic method to add {numberOfNodes} nodes with {numberOfControlPoints} control points each")
        logic.run(nodeType, numberOfNodes, numberOfControlPoints, 0, fewerModifyFlag, locked, labelsHiddenFlag)


#
# AddManyMarkupsFiducialTestLogic
#

class AddManyMarkupsFiducialTestLogic(ScriptedLoadableModuleLogic):

    def run(self, nodeType, numberOfNodes=10, numberOfControlPoints=10, rOffset=0, usefewerModifyCalls=False, locked=False, labelsHidden=False):
        """
        Run the actual algorithm
        """
        print(f'Running test to add {numberOfNodes} nodes markups with {numberOfControlPoints} control points')
        print('Index\tTime to add fid\tDelta between adds')
        print("%(index)04s\t" % {'index': "i"}, "t\tdt'")
        r = rOffset
        a = 0
        s = 0
        t1 = 0
        t2 = 0
        t3 = 0
        t4 = 0
        timeToAddThisFid = 0
        timeToAddLastFid = 0

        testStartTime = time.process_time()

        import random

        if usefewerModifyCalls:
            print("Pause render")
            slicer.app.pauseRender()

        for nodeIndex in range(numberOfNodes):

            markupsNode = slicer.mrmlScene.AddNewNodeByClass(nodeType)
            markupsNode.CreateDefaultDisplayNodes()
            if locked:
                markupsNode.SetLocked(True)

            if labelsHidden:
                markupsNode.GetDisplayNode().SetPropertiesLabelVisibility(False)
                markupsNode.GetDisplayNode().SetPointLabelsVisibility(False)

            if usefewerModifyCalls:
                print("Start modify")
                mod = markupsNode.StartModify()

            for controlPointIndex in range(numberOfControlPoints):
                #    print "controlPointIndex = ", controlPointIndex, "/", numberOfControlPoints, ", r = ", r, ", a = ", a, ", s = ", s
                t1 = time.process_time()
                markupsNode.AddControlPoint(vtk.vtkVector3d(r, a, s))
                t2 = time.process_time()
                timeToAddThisFid = t2 - t1
                dt = timeToAddThisFid - timeToAddLastFid
                # print '%(index)04d\t' % {'index': controlPointIndex}, timeToAddThisFid, "\t", dt
                r = float(controlPointIndex) / numberOfControlPoints * 100.0 - 50.0 + random.uniform(-20.0, 20.0)
                a = float(controlPointIndex) / numberOfControlPoints * 100.0 - 50.0 + random.uniform(-20.0, 20.0)
                s = random.uniform(-20.0, 20.0)
                timeToAddLastFid = timeToAddThisFid

            if usefewerModifyCalls:
                markupsNode.EndModify(mod)

        if usefewerModifyCalls:
            print("Resume render")
            slicer.app.resumeRender()

        testEndTime = time.process_time()
        testTime = testEndTime - testStartTime
        print("Total time to add ", numberOfControlPoints, " = ", testTime)

        return True


class AddManyMarkupsFiducialTestTest(ScriptedLoadableModuleTest):
    """
    This is the test case for your scripted module.
    Uses ScriptedLoadableModuleTest base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def setUp(self):
        """ Do whatever is needed to reset the state - typically a scene clear will be enough.
        """
        slicer.mrmlScene.Clear(0)

    def runTest(self):
        """Run as few or as many tests as needed here.
        """
        self.setUp()
        self.test_AddManyMarkupsFiducialTest1()

    def test_AddManyMarkupsFiducialTest1(self):

        self.delayDisplay("Starting the add many Markups fiducials test")

        # start in the welcome module
        m = slicer.util.mainWindow()
        m.moduleSelector().selectModule('Welcome')

        logic = AddManyMarkupsFiducialTestLogic()
        logic.run('vtkMRMLMarkupsFiducialNode', numberOfNodes=1, numberOfControlPoints=100, rOffset=0)

        self.delayDisplay("Now running it while the Markups Module is open")
        m.moduleSelector().selectModule('Markups')
        logic.run('vtkMRMLMarkupsFiducialNode', numberOfNodes=1, numberOfControlPoints=100, rOffset=100)

        self.delayDisplay('Test passed!')

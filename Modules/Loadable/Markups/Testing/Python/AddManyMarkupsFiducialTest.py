from __future__ import print_function
import os
import time
import unittest
import vtk, qt, ctk, slicer
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
""" # replace with organization, grant and thanks.

#
# qAddManyMarkupsFiducialTestWidget
#

class AddManyMarkupsFiducialTestWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
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
    # number of fiducials to add
    #
    self.numToAddSliderWidget = ctk.ctkSliderWidget()
    self.numToAddSliderWidget.singleStep = 1.0
    self.numToAddSliderWidget.minimum = 0.0
    self.numToAddSliderWidget.maximum = 10000.0
    self.numToAddSliderWidget.value = 500.0
    self.numToAddSliderWidget.toolTip = "Set the number of fiducials to add."
    parametersFormLayout.addRow("Number of Fiducials to Add", self.numToAddSliderWidget)

    #
    # check box to trigger fewer modify events, adding all the new points
    # is wrapped inside of a StartModify/EndModify block
    #
    self.fewerModifyFlagCheckBox = qt.QCheckBox()
    self.fewerModifyFlagCheckBox.checked = 0
    self.fewerModifyFlagCheckBox.toolTip = 'If checked, wrap adding points inside of a StartModify - EndModify block'
    parametersFormLayout.addRow("Fewer Modify Events", self.fewerModifyFlagCheckBox)

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
    sliderValue = int(self.numToAddSliderWidget.value)
    fewerModifyFlag = self.fewerModifyFlagCheckBox.checked
    print("Run the logic method to add %s fids" % sliderValue)
    logic.run(sliderValue,0,fewerModifyFlag)


#
# AddManyMarkupsFiducialTestLogic
#

class AddManyMarkupsFiducialTestLogic(ScriptedLoadableModuleLogic):

  def run(self,numToAdd=100,rOffset=0,usefewerModifyCalls=0):
    """
    Run the actual algorithm
    """
    print('Running test to add %s fidicuals' % (numToAdd,))
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

    testStartTime = time.clock()
    fidNode = slicer.vtkMRMLMarkupsFiducialNode()
    slicer.mrmlScene.AddNode(fidNode)
    fidNode.CreateDefaultDisplayNodes()

    if usefewerModifyCalls == 1:
      print("Start modify")
      mod = fidNode.StartModify()

    import random

    # iterate over the number of fiducials to add
    for i in range(numToAdd):
      #    print "i = ", i, "/", numToAdd, ", r = ", r, ", a = ", a, ", s = ", s
      t1 = time.clock()
      fidNode.AddFiducial(r,a,s)
      t2 = time.clock()
      timeToAddThisFid = t2 - t1
      dt = timeToAddThisFid - timeToAddLastFid
      #print '%(index)04d\t' % {'index': i}, timeToAddThisFid, "\t", dt
      r = float(i)/numToAdd * 100.0 - 50.0 + random.uniform(-20.0, 20.0)
      a = float(i)/numToAdd * 100.0 - 50.0 + random.uniform(-20.0, 20.0)
      s = random.uniform(-20.0, 20.0)
      timeToAddLastFid = timeToAddThisFid

    if usefewerModifyCalls == 1:
      fidNode.EndModify(mod)

    testEndTime = time.clock()
    testTime = testEndTime - testStartTime
    print("Total time to add ",numToAdd," = ", testTime)

    return True


class AddManyMarkupsFiducialTestTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
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
    logic.run(100)

    self.delayDisplay("Now running it while the Markups Module is open")
    m.moduleSelector().selectModule('Markups')
    logic.run(100,100)

    self.delayDisplay('Test passed!')

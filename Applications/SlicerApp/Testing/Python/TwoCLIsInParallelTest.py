from __future__ import print_function
import os
import unittest
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import time

#
# TwoCLIsInParallelTest
#

class TwoCLIsInParallelTest(ScriptedLoadableModule):
  def __init__(self, parent):
    parent.title = "TwoCLIsInParallelTest" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = ["CLI4Test"]
    parent.contributors = ["Johan Andruejol (Kitware)"]
    parent.helpText = """
    This is a self test that tests running two CLIs in parallel through python
    """
    parent.acknowledgementText = """""" # replace with organization, grant and thanks.
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['TwoCLIsInParallelTest'] = self.runTest

  def runTest(self):
    tester = TwoCLIsInParallelTestTest()
    tester.runTest()

#
# TwoCLIsInParallelTestWidget
#

class TwoCLIsInParallelTestWidget(ScriptedLoadableModuleWidget):

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

#
# TwoCLIsInParallelTestLogic
#

class TwoCLIsInParallelTestLogic(ScriptedLoadableModuleLogic):
  def __init__(self):
    self.Observations = []
    self.StatusModifiedEvent = slicer.vtkMRMLCommandLineModuleNode().StatusModifiedEvent

    self.parameters = {}
    self.success = False

  def runModule1(self):
    cliModule = slicer.modules.cli4test
    cliNode = slicer.cli.createNode(cliModule)
    cliNode.SetName("CLIModule1")
    self.addObserver(cliNode, self.StatusModifiedEvent, self.onModule1Modified)
    cliNode = slicer.cli.run(cliModule, cliNode, self.parameters, False)

  def onModule1Modified(self, cliNode, event):
    print("--",cliNode.GetStatusString(),":", cliNode.GetName())
    if not cliNode.IsBusy():
      self.removeObservers(cliNode, self.StatusModifiedEvent, self.onModule1Modified)

  def runModule2(self):
    cliModule = slicer.modules.cli4test
    cliNode = slicer.cli.createNode(cliModule)
    cliNode.SetName("CLIModule2")
    cliNode = slicer.cli.run(cliModule, cliNode, self.parameters, True)
    self.success = cliNode.GetStatusString() == 'Completed'

  def addObserver(self, object, event, method, group = 'none'):
    if self.hasObserver(object, event, method):
      print(object.GetName(),'already has observer')
      return
    tag = object.AddObserver(event, method)
    self.Observations.append([object, event, method, group, tag])

  def hasObserver(self, object, event, method):
    for o, e, m, g, t in self.Observations:
      if o == object and e == event and m == method:
        return True
    return False

  def removeObservers(self, object, event, method):
    for o, e, m, g, t in self.Observations:
      if object == o and event == e and method == m:
        o.RemoveObserver(t)
        self.Observations.remove([o, e, m, g, t])

#
# TwoCLIsInParallelTestLogic
#

class TwoCLIsInParallelTestTest(ScriptedLoadableModuleTest):

  def setUp(self):
    """ Reset the state for testing.
    """
    pass

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_TwoCLIsInParallelTest()

  def test_TwoCLIsInParallelTest(self):
    self.delayDisplay('Running two CLIs in a row Test')

    tempFile = qt.QTemporaryFile("TwoCLIsInParallelTest-outputFile-XXXXXX");
    self.assertTrue(tempFile.open())

    logic = TwoCLIsInParallelTestLogic()
    logic.parameters = {}
    logic.parameters["InputValue1"] = 1
    logic.parameters["InputValue2"] = 2
    logic.parameters["OperationType"] = 'Addition'
    logic.parameters["OutputFile"] = tempFile.fileName()

    logic.runModule1()
    self.delayDisplay('... Waiting to start module 2 ...')
    logic.runModule2()
    self.assertTrue(logic.success)

    self.delayDisplay('Two CLIs in parallel test passed !')


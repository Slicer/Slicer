from __future__ import print_function
import os
import unittest
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import time

#
# TwoCLIsInARowTest
#

class TwoCLIsInARowTest(ScriptedLoadableModule):
  def __init__(self, parent):
    parent.title = "TwoCLIsInARowTest" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = ["CLI4Test"]
    parent.contributors = ["Alexis Girault (Kitware), Johan Andruejol (Kitware)"]
    parent.helpText = """
    This is a self test that tests the piping of two CLIs through python
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
    slicer.selfTests['TwoCLIsInARowTest'] = self.runTest

  def runTest(self):
    tester = TwoCLIsInARowTestTest()
    tester.runTest()

#
# TwoCLIsInARowTestWidget
#

class TwoCLIsInARowTestWidget(ScriptedLoadableModuleWidget):

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

#
# TwoCLIsInARowTestLogic
#

class TwoCLIsInARowTestLogic(ScriptedLoadableModuleLogic):
  def __init__(self):
    self.Observations = []
    self.StatusModifiedEvent = slicer.vtkMRMLCommandLineModuleNode().StatusModifiedEvent

    self.parameters = {}
    self.success = False

  def runTest(self):
    self.runModule1()

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
      if cliNode.GetStatusString() == 'Completed':
        self.runModule2()

  def runModule2(self):
    cliModule = slicer.modules.cli4test
    cliNode = slicer.cli.createNode(cliModule)
    cliNode.SetName("CLIModule2")
    self.addObserver(cliNode, self.StatusModifiedEvent, self.onModule2Modified)
    cliNode = slicer.cli.run(cliModule, cliNode, self.parameters, False)

  def onModule2Modified(self, cliNode, event):
    print("--",cliNode.GetStatusString(),":", cliNode.GetName())
    if not cliNode.IsBusy():
      self.removeObservers(cliNode, self.StatusModifiedEvent, self.onModule2Modified)
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
# TwoCLIsInARowTestLogic
#

class TwoCLIsInARowTestTest(ScriptedLoadableModuleTest):

  def setUp(self):
    """ Reset the state for testing.
    """
    pass

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_TwoCLIsInARowTest()

  def test_TwoCLIsInARowTest(self):
    self.delayDisplay('Running two CLIs in a row Test')

    tempFile = qt.QTemporaryFile("TwoCLIsInARowTest-outputFile-XXXXXX");
    self.assertTrue(tempFile.open())

    logic = TwoCLIsInARowTestLogic()
    logic.parameters = {}
    logic.parameters["InputValue1"] = 1
    logic.parameters["InputValue2"] = 2
    logic.parameters["OperationType"] = 'Addition'
    logic.parameters["OutputFile"] = tempFile.fileName()

    logic.runTest()
    while not logic.success:
      self.delayDisplay('Waiting for module 2 to complete...')
    self.assertTrue(logic.success)

    self.delayDisplay('Two CLIs in a row test passed !')

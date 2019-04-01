from __future__ import print_function
import os
import unittest
from __main__ import vtk, qt, ctk, slicer
from slicer.util import VTKObservationMixin
from slicer.ScriptedLoadableModule import *
import time

#
# CLIEventTest
#

class CLIEventTest(ScriptedLoadableModule):
  def __init__(self, parent):
    parent.title = "CLIEventTest" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = ["CLI4Test"]
    parent.contributors = ["Johan Andruejol (Kitware)"]
    parent.helpText = """
    This is a self test that tests that CLI send all the event properly.
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
    slicer.selfTests['CLIEventTest'] = self.runTest

  def runTest(self):
    tester = CLIEventTestTest()
    tester.runTest()

#
# CLIEventTestWidget
#

class CLIEventTestWidget(ScriptedLoadableModuleWidget):

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

#
# CLIEventTestLogic
#

class CLIEventTestLogic(VTKObservationMixin):
  def __init__(self):
    VTKObservationMixin.__init__(self)

    cli = slicer.vtkMRMLCommandLineModuleNode()
    self.StatusModifiedEvent = cli.StatusModifiedEvent
    self.StatusEvents = []

    self.ExecutionFinished = False
    self.StatusEventCallback = None

  def runCLI(self, cliModule, cliNode, parameters, wait_for_completion):
    self.addObserver(cliNode, self.StatusModifiedEvent, self.onCLIModified)
    cliNode = slicer.cli.run(cliModule, cliNode, parameters, wait_for_completion)

  def onCLIModified(self, cliNode, event):
    print ("-- " + cliNode.GetStatusString() + ":" + cliNode.GetName())
    self.StatusEvents.append(cliNode.GetStatus())

    if not cliNode.IsBusy():
      self.removeObserver(cliNode, self.StatusModifiedEvent, self.onCLIModified)
      self.ExecutionFinished = True

    if self.StatusEventCallback:
      self.StatusEventCallback(cliNode)

#
# CLIEventTestLogic
#

class CLIEventTestTest(ScriptedLoadableModuleTest):

  def setUp(self):
    """ Reset the state for testing.
    """

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_CLIStatusEventTestSynchronous()
    self.test_CLIStatusEventTestAsynchronous()
    self.test_CLIStatusEventTestCancel()
    self.test_CLIStatusEventOnErrorTestSynchronous()
    self.test_CLIStatusEventOnErrorTestAsynchronous()
    self.test_SubjectHierarchyReference()

  # Testing a the status event on a normal execution
  def test_CLIStatusEventTestSynchronous(self):
    self._testCLIStatusEventTest(True)

  def test_CLIStatusEventTestAsynchronous(self):
    self._testCLIStatusEventTest(False)

  def _testCLIStatusEventTest(self, wait_for_completion):
    self.delayDisplay('Testing status events for a normal execution of a CLI')

    tempFile = qt.QTemporaryFile("CLIEventTest-outputFile-XXXXXX");
    self.assertTrue(tempFile.open())

    logic = CLIEventTestLogic()
    parameters = {}
    parameters["InputValue1"] = 1
    parameters["InputValue2"] = 2
    parameters["OperationType"] = 'Addition'
    parameters["OutputFile"] = tempFile.fileName()

    cliModule = slicer.modules.cli4test
    cli = slicer.cli.createNode(cliModule)
    self.assertEqual(cli.GetStatus(), cli.Idle)

    logic.runCLI(cliModule, cli, parameters, wait_for_completion)

    while not logic.ExecutionFinished:
      self.delayDisplay('Waiting for module to complete...')

    cli = slicer.vtkMRMLCommandLineModuleNode()
    expectedEvents = []
    if not wait_for_completion:
      expectedEvents.append(cli.Scheduled)
    expectedEvents.append(cli.Completed)

    # Ignore cli.Running event (it may or may not be fired)
    if cli.Running in logic.StatusEvents:
      logic.StatusEvents.remove(cli.Running)

    self.assertEqual(logic.StatusEvents, expectedEvents)
    self.delayDisplay('Testing normal execution Passed')

  # Testing the status event on a bad execution
  def test_CLIStatusEventOnErrorTestSynchronous(self):
    self._testCLIStatusEventOnErrorTest(True)

  def test_CLIStatusEventOnErrorTestAsynchronous(self):
    self._testCLIStatusEventOnErrorTest(False)

  def _testCLIStatusEventOnErrorTest(self, wait_for_completion):
    self.delayDisplay('Testing status events for a bad execution of a CLI')

    tempFile = qt.QTemporaryFile("CLIEventTest-outputFile-XXXXXX");
    self.assertTrue(tempFile.open())

    logic = CLIEventTestLogic()
    parameters = {}
    parameters["InputValue1"] = 1
    parameters["InputValue2"] = 2
    parameters["OperationType"] = 'Fail'
    parameters["OutputFile"] = tempFile.fileName()

    cliModule = slicer.modules.cli4test
    cli = slicer.cli.createNode(cliModule)
    self.assertEqual(cli.GetStatus(), cli.Idle)

    logic.runCLI(cliModule, cli, parameters, wait_for_completion)

    while not logic.ExecutionFinished:
      self.delayDisplay('Waiting for module to complete...')

    cli = slicer.vtkMRMLCommandLineModuleNode()
    expectedEvents = []
    if not wait_for_completion:
      expectedEvents.append(cli.Scheduled)
    expectedEvents.append(cli.CompletedWithErrors)

    # Ignore cli.Running event (it may or may not be fired)
    if cli.Running in logic.StatusEvents:
      logic.StatusEvents.remove(cli.Running)

    self.assertEqual(logic.StatusEvents, expectedEvents)
    self.delayDisplay('Testing bad execution Passed')

  # Testing a the status event when canceling
  def test_CLIStatusEventTestCancel(self):
    self.delayDisplay('Testing status events when cancelling the execution of a CLI')

    tempFile = qt.QTemporaryFile("CLIEventTest-outputFile-XXXXXX");
    self.assertTrue(tempFile.open())

    logic = CLIEventTestLogic()
    parameters = {}
    parameters["InputValue1"] = 1
    parameters["InputValue2"] = 2
    parameters["OperationType"] = 'Addition'
    parameters["OutputFile"] = tempFile.fileName()

    cliModule = slicer.modules.cli4test
    cli = slicer.cli.createNode(cliModule)
    self.assertEqual(cli.GetStatus(), cli.Idle)

    logic.runCLI(cliModule, cli, parameters, False)
    cli.Cancel()

    while not logic.ExecutionFinished:
      self.delayDisplay('Waiting for module to complete...')

    expectedEvents = [
      cli.Scheduled,
      cli.Cancelling,
      cli.Cancelled,
      ]

    # Ignore cli.Running event (it may or may not be fired)
    if cli.Running in logic.StatusEvents:
      logic.StatusEvents.remove(cli.Running)

    self.assertEqual(logic.StatusEvents, expectedEvents)
    self.delayDisplay('Testing cancelled execution Passed')

  def test_SubjectHierarchyReference(self):
    self.delayDisplay('Test that output node moved to referenced node location in subject hierarchy')

    self.delayDisplay('Load input volume')
    import SampleData
    inputVolume = SampleData.downloadSample("MRHead")

    self.delayDisplay('Create subject hierarchy of input volume')
    shNode = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
    self.patientItemID = shNode.CreateSubjectItem(shNode.GetSceneItemID(), "John Doe")
    self.studyItemID = shNode.CreateStudyItem(self.patientItemID, "Some study")
    self.folderItemID = shNode.CreateFolderItem(self.studyItemID, "Some group")
    shNode.SetItemParent(shNode.GetItemByDataNode(inputVolume), self.folderItemID)

    outputVolume = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScalarVolumeNode")
    # New node is expected to be created in the subject hierarchy root
    self.assertEqual(shNode.GetItemParent(shNode.GetItemByDataNode(outputVolume)), shNode.GetSceneItemID())

    self.delayDisplay('Run CLI module')
    cliParams = {'InputVolume': inputVolume.GetID(), 'OutputVolume': outputVolume.GetID(), 'ThresholdValue' : 100, 'ThresholdType' : 'Above'}
    cliNode = slicer.cli.run(slicer.modules.thresholdscalarvolume, None, cliParams, wait_for_completion=True)

    # After CLI execution is completed, output volume must be in the same folder as the referenced node
    self.assertEqual(shNode.GetItemParent(shNode.GetItemByDataNode(outputVolume)), shNode.GetItemParent(shNode.GetItemByDataNode(inputVolume)))

    # After pending events are processed, the output volume must be in the same subject hierarchy folder
    slicer.app.processEvents()
    self.assertEqual(shNode.GetItemParent(shNode.GetItemByDataNode(outputVolume)), shNode.GetItemParent(shNode.GetItemByDataNode(inputVolume)))

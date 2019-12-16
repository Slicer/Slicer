import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *

#
# MarkupsWidgetsSelfTest
#

class MarkupsWidgetsSelfTest(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "MarkupsWidgetsSelfTest"
    self.parent.categories = ["Testing.TestCases"]
    self.parent.dependencies = ["Markups"]
    self.parent.contributors = ["Andras Lasso (PerkLab, Queen's)"]
    self.parent.helpText = """This is a self test for Markups widgets."""
    self.parent.acknowledgementText = """This file was originally developed by Andras Lasso, PerkLab, Queen's University and was supported through the Applied Cancer Research Unit program of Cancer Care Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care"""

#
# MarkupsWidgetsSelfTestWidget
#

class MarkupsWidgetsSelfTestWidget(ScriptedLoadableModuleWidget):
  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

#
# MarkupsWidgetsSelfTestLogic
#

class MarkupsWidgetsSelfTestLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass


class MarkupsWidgetsSelfTestTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

    self.delayMs = 700

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_MarkupsWidgetsSelfTest_FullTest1()

  # ------------------------------------------------------------------------------
  def test_MarkupsWidgetsSelfTest_FullTest1(self):
    # Check for Tables module
    self.assertTrue( slicer.modules.tables )

    self.section_SetupPathsAndNames()
    self.section_CreateMarkups()
    self.section_SimpleMarkupsWidget()
    self.section_MarkupsPlaceWidget()
    self.delayDisplay("Test passed",self.delayMs)

  # ------------------------------------------------------------------------------
  def section_SetupPathsAndNames(self):
    # Set constants
    self.sampleMarkupsNodeName1 = 'SampleMarkups1'
    self.sampleMarkupsNodeName2 = 'SampleMarkups2'

  # ------------------------------------------------------------------------------
  def section_CreateMarkups(self):
    self.delayDisplay("Create markup nodes",self.delayMs)

    self.markupsLogic = slicer.modules.markups.logic()

    # Create sample markups node
    self.markupsNode1 = slicer.mrmlScene.GetNodeByID(self.markupsLogic.AddNewFiducialNode())
    self.markupsNode1.SetName(self.sampleMarkupsNodeName1)

    self.markupsNode2 = slicer.mrmlScene.GetNodeByID(self.markupsLogic.AddNewFiducialNode())
    self.markupsNode2.SetName(self.sampleMarkupsNodeName2)

  # ------------------------------------------------------------------------------
  def section_SimpleMarkupsWidget(self):
    self.delayDisplay("Test SimpleMarkupsWidget",self.delayMs)

    simpleMarkupsWidget = slicer.qSlicerSimpleMarkupsWidget()
    nodeSelector = slicer.util.findChildren(simpleMarkupsWidget,"MarkupsNodeComboBox")[0]
    self.assertIsNone(simpleMarkupsWidget.interactionNode())
    simpleMarkupsWidget.setMRMLScene(slicer.mrmlScene)
    simpleMarkupsWidget.show()

    placeWidget = simpleMarkupsWidget.markupsPlaceWidget()
    self.assertIsNotNone(placeWidget)

    simpleMarkupsWidget.setCurrentNode(None)
    simpleMarkupsWidget.enterPlaceModeOnNodeChange = False
    placeWidget.placeModeEnabled = False
    nodeSelector.setCurrentNode(self.markupsNode1)
    self.assertFalse(placeWidget.placeModeEnabled)

    simpleMarkupsWidget.enterPlaceModeOnNodeChange = True
    nodeSelector.setCurrentNode(self.markupsNode2)
    self.assertTrue(placeWidget.placeModeEnabled)

    simpleMarkupsWidget.jumpToSliceEnabled = True
    self.assertTrue(simpleMarkupsWidget.jumpToSliceEnabled)
    simpleMarkupsWidget.jumpToSliceEnabled = False
    self.assertFalse(simpleMarkupsWidget.jumpToSliceEnabled)

    simpleMarkupsWidget.nodeSelectorVisible = False
    self.assertFalse(simpleMarkupsWidget.nodeSelectorVisible)
    simpleMarkupsWidget.nodeSelectorVisible = True
    self.assertTrue(simpleMarkupsWidget.nodeSelectorVisible)

    simpleMarkupsWidget.optionsVisible = False
    self.assertFalse(simpleMarkupsWidget.optionsVisible)
    simpleMarkupsWidget.optionsVisible = True
    self.assertTrue(simpleMarkupsWidget.optionsVisible)

    defaultColor = qt.QColor(0,255,0)
    simpleMarkupsWidget.defaultNodeColor = defaultColor
    self.assertEqual(simpleMarkupsWidget.defaultNodeColor, defaultColor)

    self.markupsNode3 = nodeSelector.addNode()
    displayNode3 = self.markupsNode3.GetDisplayNode()
    color3 = displayNode3.GetColor()
    self.assertEqual(color3[0]*255, defaultColor.red())
    self.assertEqual(color3[1]*255, defaultColor.green())
    self.assertEqual(color3[2]*255, defaultColor.blue())

    numberOfFiducialsAdded = 5
    for i in range(numberOfFiducialsAdded):
      self.markupsLogic.AddFiducial()

    tableWidget = simpleMarkupsWidget.tableWidget()
    self.assertEqual(tableWidget.rowCount, numberOfFiducialsAdded)

    self.assertEqual(simpleMarkupsWidget.interactionNode(), slicer.app.applicationLogic().GetInteractionNode())
    otherInteractionNode = slicer.vtkMRMLInteractionNode()
    otherInteractionNode.SetSingletonOff()
    slicer.mrmlScene.AddNode(otherInteractionNode)
    simpleMarkupsWidget.setInteractionNode(otherInteractionNode)
    self.assertEqual(simpleMarkupsWidget.interactionNode(), otherInteractionNode)

  # ------------------------------------------------------------------------------
  def section_MarkupsPlaceWidget(self):
    self.delayDisplay("Test MarkupsPlaceWidget",self.delayMs)

    placeWidget = slicer.qSlicerMarkupsPlaceWidget()
    self.assertIsNone(placeWidget.interactionNode())
    placeWidget.setMRMLScene(slicer.mrmlScene)
    placeWidget.setCurrentNode(self.markupsNode1)
    placeWidget.show()

    placeWidget.buttonsVisible = False
    self.assertFalse(placeWidget.buttonsVisible)
    placeWidget.buttonsVisible = True
    self.assertTrue(placeWidget.buttonsVisible)

    placeWidget.deleteAllMarkupsOptionVisible = False
    self.assertFalse(placeWidget.deleteAllMarkupsOptionVisible)
    placeWidget.deleteAllMarkupsOptionVisible = True
    self.assertTrue(placeWidget.deleteAllMarkupsOptionVisible)

    placeWidget.deleteAllMarkupsOptionVisible = False
    self.assertFalse(placeWidget.deleteAllMarkupsOptionVisible)
    placeWidget.deleteAllMarkupsOptionVisible = True
    self.assertTrue(placeWidget.deleteAllMarkupsOptionVisible)

    placeWidget.placeMultipleMarkups = slicer.qSlicerMarkupsPlaceWidget.ForcePlaceSingleMarkup
    placeWidget.placeModeEnabled = True
    self.assertFalse(placeWidget.placeModePersistency)

    placeWidget.placeMultipleMarkups = slicer.qSlicerMarkupsPlaceWidget.ForcePlaceMultipleMarkups
    placeWidget.placeModeEnabled = False
    placeWidget.placeModeEnabled = True
    self.assertTrue(placeWidget.placeModePersistency)

    self.assertEqual(placeWidget.interactionNode(), slicer.app.applicationLogic().GetInteractionNode())
    otherInteractionNode = slicer.vtkMRMLInteractionNode()
    otherInteractionNode.SetSingletonOff()
    slicer.mrmlScene.AddNode(otherInteractionNode)
    placeWidget.setInteractionNode(otherInteractionNode)
    self.assertEqual(placeWidget.interactionNode(), otherInteractionNode)

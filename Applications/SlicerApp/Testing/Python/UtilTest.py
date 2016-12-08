import os
import unittest
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import time

#
# UtilTest
#

class UtilTest(ScriptedLoadableModule):
  def __init__(self, parent):
    parent.title = "UtilTest" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Johan Andruejol (Kitware)"]
    parent.helpText = """
    This is a self test that tests the methods of slicer.util
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
    slicer.selfTests['UtilTest'] = self.runTest

  def runTest(self):
    tester = UtilTestTest()
    tester.runTest()

#
# UtilTestWidget
#

class UtilTestWidget(ScriptedLoadableModuleWidget):
  def __init__(self):
    ScriptedLoadableModuleWidget.__init__(self)
    self.Widget = None

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    moduleName = 'UtilTest'
    scriptedModulesPath = os.path.dirname(slicer.util.modulePath(moduleName))
    path = os.path.join(scriptedModulesPath, 'Resources', 'UI', moduleName + '.ui')

    self.Widget = slicer.util.loadUI(path)
    self.layout.addWidget(self.Widget)

#
# UtilTestLogic
#

class UtilTestLogic(ScriptedLoadableModuleLogic):
  def __init__(self):
    ScriptedLoadableModuleLogic.__init__(self)

#
# UtilTestLogic
#

class UtilTestTest(ScriptedLoadableModuleTest):

  def setUp(self):
    """ Reset the state for testing.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_setSliceViewerLayers()
    self.test_loadUI()
    self.test_findChild()

  def test_setSliceViewerLayers(self):
    self.delayDisplay('Testing slicer.util.setSliceViewerLayers')

    layoutManager = slicer.app.layoutManager()
    layoutManager.layout = slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpRedSliceView

    redSliceCompositeNode = slicer.mrmlScene.GetNodeByID('vtkMRMLSliceCompositeNodeRed')
    self.assertIsNotNone(redSliceCompositeNode)
    self.assertIsNone(redSliceCompositeNode.GetBackgroundVolumeID())
    self.assertIsNone(redSliceCompositeNode.GetForegroundVolumeID())
    self.assertIsNone(redSliceCompositeNode.GetLabelVolumeID())

    backgroundNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLScalarVolumeNode())
    backgroundNode.SetName('Background')
    foregroundNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLScalarVolumeNode())
    foregroundNode.SetName('Foreground')
    labelmapNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLScalarVolumeNode())
    labelmapNode.SetName('Labelmap')

    # Try with nothing
    slicer.util.setSliceViewerLayers()
    self.assertIsNone(redSliceCompositeNode.GetBackgroundVolumeID())
    self.assertIsNone(redSliceCompositeNode.GetForegroundVolumeID())
    self.assertIsNone(redSliceCompositeNode.GetLabelVolumeID())

    # Try with nodes
    slicer.util.setSliceViewerLayers(
      background = backgroundNode,
      foreground = foregroundNode,
      label = labelmapNode,
      foregroundOpacity = 0.5,
      labelOpacity = 0.1
      )
    self.assertEqual(redSliceCompositeNode.GetBackgroundVolumeID(), backgroundNode.GetID())
    self.assertEqual(redSliceCompositeNode.GetForegroundVolumeID(), foregroundNode.GetID())
    self.assertEqual(redSliceCompositeNode.GetLabelVolumeID(), labelmapNode.GetID())
    self.assertEqual(redSliceCompositeNode.GetForegroundOpacity(), 0.5)
    self.assertEqual(redSliceCompositeNode.GetLabelOpacity(), 0.1)

    # Try to reset
    otherBackgroundNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLScalarVolumeNode())
    otherBackgroundNode.SetName('OtherBackground')
    otherForegroundNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLScalarVolumeNode())
    otherForegroundNode.SetName('OtherForeground')
    otherLabelmapNode = slicer.mrmlScene.AddNode(slicer.vtkMRMLScalarVolumeNode())
    otherLabelmapNode.SetName('OtherLabelmap')

    # Try with node id's
    slicer.util.setSliceViewerLayers(
      background = otherBackgroundNode.GetID(),
      foreground = otherForegroundNode.GetID(),
      label = otherLabelmapNode.GetID(),
      foregroundOpacity = 0.0,
      labelOpacity = 1.0
      )
    self.assertEqual(redSliceCompositeNode.GetBackgroundVolumeID(), otherBackgroundNode.GetID())
    self.assertEqual(redSliceCompositeNode.GetForegroundVolumeID(), otherForegroundNode.GetID())
    self.assertEqual(redSliceCompositeNode.GetLabelVolumeID(), otherLabelmapNode.GetID())
    self.assertEqual(redSliceCompositeNode.GetForegroundOpacity(), 0.0)
    self.assertEqual(redSliceCompositeNode.GetLabelOpacity(), 1.0)

    self.delayDisplay('Testing slicer.util.setSliceViewerLayers passed !')

  def test_loadUI(self):
    # Try to load a UI that does not exist and catch exception
    caughtException = False
    try:
      slicer.util.loadUI('does/not/exists.ui')
    except RuntimeError:
      caughtException = True
    self.assertTrue(caughtException)

    # Correct path
    utilWidget = UtilTestWidget()
    caughtException = False
    try:
      utilWidget.setup()
    except RuntimeError:
      caughtException = True
    self.assertFalse(caughtException)

  def test_findChild(self):
    utilWidget = UtilTestWidget()

    # Try with nothing (widget isn't setup)
    caughtException = False
    try:
      slicer.util.findChild(utilWidget.Widget, 'UtilTest_Label')
    except RuntimeError:
      caughtException = True
    self.assertTrue(caughtException)

    utilWidget.setup()

    # Try to get a widget that exists
    label = slicer.util.findChild(utilWidget.Widget, 'UtilTest_Label')
    self.assertIsNotNone(label, qt.QLabel)
    self.assertEqual(label.text, 'My custom UI')

    # Try to get a widget that does not exists
    caughtException = False
    try:
      slicer.util.findChild(utilWidget.Widget, 'Unexistant_Label')
    except RuntimeError:
      caughtException = True
    self.assertTrue(caughtException)

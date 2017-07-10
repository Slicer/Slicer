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
  def __init__(self, parent = None):
    self.Widget = None
    ScriptedLoadableModuleWidget.__init__(self, parent)

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
    self.test_arrayFromVolume()
    self.test_updateVolumeFromArray()
    self.test_array()

  def test_setSliceViewerLayers(self):
    self.delayDisplay('Testing slicer.util.setSliceViewerLayers')

    layoutManager = slicer.app.layoutManager()
    layoutManager.layout = slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpRedSliceView

    redSliceCompositeNode = slicer.mrmlScene.GetNodeByID('vtkMRMLSliceCompositeNodeRed')
    self.assertIsNotNone(redSliceCompositeNode)
    self.assertIsNone(redSliceCompositeNode.GetBackgroundVolumeID())
    self.assertIsNone(redSliceCompositeNode.GetForegroundVolumeID())
    self.assertIsNone(redSliceCompositeNode.GetLabelVolumeID())

    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()

    backgroundNode = sampleDataLogic.downloadMRHead()
    backgroundNode.SetName('Background')
    foregroundNode = sampleDataLogic.downloadMRHead()
    foregroundNode.SetName('Foreground')

    volumesLogic = slicer.modules.volumes.logic()
    labelmapNode = volumesLogic.CreateAndAddLabelVolume( slicer.mrmlScene, backgroundNode, 'Labelmap' )

    thresholder = vtk.vtkImageThreshold()
    thresholder.SetInputData(backgroundNode.GetImageData())
    thresholder.ThresholdByLower(80)
    thresholder.Update()
    labelmapNode.SetAndObserveImageData(thresholder.GetOutput())

    # Try with nothing
    slicer.util.setSliceViewerLayers(background = None, foreground = None, label = None)
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
    otherBackgroundNode = sampleDataLogic.downloadMRHead()
    otherBackgroundNode.SetName('OtherBackground')
    otherForegroundNode = sampleDataLogic.downloadMRHead()
    otherForegroundNode.SetName('OtherForeground')
    otherLabelmapNode = volumesLogic.CreateAndAddLabelVolume( slicer.mrmlScene, backgroundNode, 'OtherLabelmap' )

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

    self.delayDisplay('Testing slicer.util.setSliceViewerLayers passed')

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

    # Try to get a widget that exists
    label = slicer.util.findChild(utilWidget.parent, 'UtilTest_Label')
    self.assertIsNotNone(label, qt.QLabel)
    self.assertEqual(label.text, 'My custom UI')

  def test_findChild(self):
    # Create a top-level widget (parent is not specified)
    utilWidget = UtilTestWidget()

    # Try to get a widget that exists
    label = slicer.util.findChild(utilWidget.Widget, 'UtilTest_Label')
    self.assertIsNotNone(label, qt.QLabel)
    self.assertEqual(label.text, 'My custom UI')

    # Try to get a widget that does not exists
    caughtException = False
    try:
      slicer.util.findChild(utilWidget.Widget, 'Nonexisting_Label')
    except RuntimeError:
      caughtException = True
    self.assertTrue(caughtException)

  def test_arrayFromVolume(self):
    # Test if retrieving voxels as a numpy array works

    self.delayDisplay('Download sample data')
    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    volumeNode = sampleDataLogic.downloadMRHead()

    self.delayDisplay('Test voxel value read')
    voxelPos = [120,135,89]
    voxelValueVtk = volumeNode.GetImageData().GetScalarComponentAsDouble(voxelPos[0], voxelPos[1], voxelPos[2], 0)
    narray = slicer.util.arrayFromVolume(volumeNode)
    voxelValueNumpy = narray[voxelPos[2], voxelPos[1], voxelPos[0]]
    self.assertEqual(voxelValueVtk, voxelValueNumpy)

    self.delayDisplay('Test voxel value write')
    voxelValueNumpy =  155
    narray[voxelPos[2], voxelPos[1], voxelPos[0]] = voxelValueNumpy
    voxelValueVtk = volumeNode.GetImageData().GetScalarComponentAsDouble(voxelPos[0], voxelPos[1], voxelPos[2], 0)
    self.assertEqual(voxelValueVtk, voxelValueNumpy)

    self.delayDisplay('Testing slicer.util.test_arrayFromVolume passed')

  def test_updateVolumeFromArray(self):
    # Test if updating voxels from a numpy array works

    self.delayDisplay('Download sample data')
    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    volumeNode = sampleDataLogic.downloadMRHead()

    import numpy as np
    import math

    def some_func(x, y, z):
      return 0.1*x*x + 0.03*y*y + 0.05*z*z

    f = np.fromfunction(some_func,(30,20,15))

    slicer.util.updateVolumeFromArray(volumeNode, f)

    self.delayDisplay('Test voxel value update')
    voxelPos = [11, 12, 4]
    voxelValueNumpy = some_func(voxelPos[2], voxelPos[1], voxelPos[0])
    voxelValueVtk = volumeNode.GetImageData().GetScalarComponentAsDouble(voxelPos[0], voxelPos[1], voxelPos[2], 0)
    self.assertEqual(voxelValueVtk, voxelValueNumpy)

    self.delayDisplay('Testing slicer.util.test_updateVolumeFromArray passed')

  def test_array(self):
    # Test if convenience function of getting numpy array from various nodes works

    self.delayDisplay('Test array with scalar image')
    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    volumeNode = sampleDataLogic.downloadMRHead()
    voxelPos = [120,135,89]
    voxelValueVtk = volumeNode.GetImageData().GetScalarComponentAsDouble(voxelPos[0], voxelPos[1], voxelPos[2], 0)
    narray = slicer.util.arrayFromVolume(volumeNode)
    voxelValueNumpy = narray[voxelPos[2], voxelPos[1], voxelPos[0]]
    self.assertEqual(voxelValueVtk, voxelValueNumpy)

    self.delayDisplay('Test array with tensor image')
    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    tensorVolumeNode = sampleDataLogic.downloadDTIBrain()
    narray = slicer.util.array(tensorVolumeNode.GetName())
    self.assertEqual(narray.shape, (85, 144, 144, 3, 3))

    self.delayDisplay('Test array with model points')
    sphere = vtk.vtkSphereSource()
    modelNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLModelNode')
    modelNode.SetPolyDataConnection(sphere.GetOutputPort())
    narray = slicer.util.array(modelNode.GetName())
    self.assertEqual(narray.shape, (50, 3))

    self.delayDisplay('Testing slicer.util.test_array passed')

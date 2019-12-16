import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging
import time

#
# UtilTest
#

class UtilTest(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "UtilTest" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Johan Andruejol (Kitware)"]
    parent.helpText = """
    This is a self test that tests the methods of slicer.util
    """
    parent.acknowledgementText = """""" # replace with organization, grant and thanks.

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
    self.test_updateTableFromArray()
    self.test_arrayFromModelPoints()
    self.test_arrayFromVTKMatrix()
    self.test_arrayFromTransformMatrix()
    self.test_arrayFromMarkupsControlPoints()
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

    backgroundNode = SampleData.downloadSample("MRHead")
    backgroundNode.SetName('Background')
    foregroundNode = SampleData.downloadSample("MRHead")
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
    otherBackgroundNode = SampleData.downloadSample("MRHead")
    otherBackgroundNode.SetName('OtherBackground')
    otherForegroundNode = SampleData.downloadSample("MRHead")
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

    # Parent window is created automatically, delete it now to prevent memory leaks
    utilWidget.parent.deleteLater()

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
      slicer.util.findChild(utilWidget.Widget, 'Unexistant_Label')
    except RuntimeError:
      caughtException = True
    self.assertTrue(caughtException)

    # Parent window is created automatically, delete it now to prevent memory leaks
    utilWidget.parent.deleteLater()

  def test_arrayFromVolume(self):
    # Test if retrieving voxels as a numpy array works

    self.delayDisplay('Download sample data')
    import SampleData
    volumeNode = SampleData.downloadSample("MRHead")

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
    volumeNode = SampleData.downloadSample("MRHead")

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

  def test_updateTableFromArray(self):
    # Test if updating table values from a numpy array works
    import numpy as np

    self.delayDisplay('Test simple 2D array update')
    a=np.array([[1,2,3,4],[5,6,7,8],[9,10,11,12]])
    tableNode1 = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTableNode")
    slicer.util.updateTableFromArray(tableNode1, a)
    self.assertEqual(tableNode1.GetNumberOfColumns(), 4)
    self.assertEqual(tableNode1.GetNumberOfRows(), 3)

    self.delayDisplay('Download sample data')
    import SampleData
    volumeNode = SampleData.downloadSample("MRHead")

    self.delayDisplay('Compute histogram')
    histogram = np.histogram(slicer.util.arrayFromVolume(volumeNode))

    self.delayDisplay('Test table update')
    tableNode2 = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTableNode")
    slicer.util.updateTableFromArray(tableNode2, histogram)
    self.assertEqual(tableNode2.GetNumberOfColumns(), 2)
    self.assertEqual(tableNode2.GetNumberOfRows(), 11)

    self.delayDisplay('Testing slicer.util.test_updateTableFromArray passed')

  def test_arrayFromModelPoints(self):
    # Test if retrieving point coordinates as a numpy array works

    self.delayDisplay('Create a model containing a sphere')
    sphere = vtk.vtkSphereSource()
    sphere.SetRadius(30.0)
    sphere.Update()
    modelNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLModelNode')
    modelNode.SetAndObservePolyData(sphere.GetOutput())
    modelNode.CreateDefaultDisplayNodes()
    a = slicer.util.arrayFromModelPoints(modelNode)

    self.delayDisplay('Change Y scaling')
    a[:,2] = a[:,2] * 2.5
    modelNode.GetPolyData().Modified()

    self.delayDisplay('Testing slicer.util.test_arrayFromModelPoints passed')

  def test_arrayFromVTKMatrix(self):
    # Test arrayFromVTKMatrix,  vtkMatrixFromArray, and updateVTKMatrixFromArray
    import numpy as np

    self.delayDisplay('Test arrayFromVTKMatrix, vtkMatrixFromArray, and updateVTKMatrixFromArray')

    # Test 4x4 matrix
    a = np.array([[1.5,0.5,0,4],[0,2.0,0,11],[0,0,2.5,6],[0,0,0,1]])
    vmatrix = slicer.util.vtkMatrixFromArray(a)
    self.assertTrue(isinstance(vmatrix,vtk.vtkMatrix4x4))
    self.assertEqual(vmatrix.GetElement(0,1), 0.5)
    self.assertEqual(vmatrix.GetElement(1,3), 11)

    narray = slicer.util.arrayFromVTKMatrix(vmatrix)
    np.testing.assert_array_equal(a, narray)

    vmatrixExisting = vtk.vtkMatrix4x4()
    slicer.util.updateVTKMatrixFromArray(vmatrixExisting, a)
    narray = slicer.util.arrayFromVTKMatrix(vmatrixExisting)
    np.testing.assert_array_equal(a, narray)

    # Test 3x3 matrix
    a = np.array([[1.5,0,0],[0,2.0,0],[0,1.2,2.5]])
    vmatrix = slicer.util.vtkMatrixFromArray(a)
    self.assertTrue(isinstance(vmatrix,vtk.vtkMatrix3x3))
    self.assertEqual(vmatrix.GetElement(0,0), 1.5)
    self.assertEqual(vmatrix.GetElement(2,1), 1.2)

    narray = slicer.util.arrayFromVTKMatrix(vmatrix)
    np.testing.assert_array_equal(a, narray)

    vmatrixExisting = vtk.vtkMatrix3x3()
    slicer.util.updateVTKMatrixFromArray(vmatrixExisting, a)
    narray = slicer.util.arrayFromVTKMatrix(vmatrixExisting)
    np.testing.assert_array_equal(a, narray)

    # Test invalid matrix size
    caughtException = False
    try:
      vmatrix = slicer.util.vtkMatrixFromArray(np.zeros([3,4]))
    except RuntimeError:
      caughtException = True
    self.assertTrue(caughtException)

  def test_arrayFromTransformMatrix(self):
    # Test arrayFromTransformMatrix and updateTransformMatrixFromArray
    import numpy as np

    self.delayDisplay('Test arrayFromTransformMatrix')

    transformNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLTransformNode')
    transformMatrix = vtk.vtkMatrix4x4()
    transformMatrix.SetElement(0,0, 5.0)
    transformMatrix.SetElement(0,1, 3.0)
    transformMatrix.SetElement(1,1, 2.3)
    transformMatrix.SetElement(2,2, 1.3)
    transformMatrix.SetElement(0,3, 11.0)
    transformMatrix.SetElement(1,3, 22.0)
    transformMatrix.SetElement(2,3, 44.0)
    transformNode.SetMatrixTransformToParent(transformMatrix)

    narray = slicer.util.arrayFromTransformMatrix(transformNode)
    self.assertEqual(narray.shape, (4, 4))
    self.assertEqual(narray[0,0], 5.0)
    self.assertEqual(narray[0,1], 3.0)
    self.assertEqual(narray[0,3], 11.0)
    self.assertEqual(narray[2,3], 44.0)

    self.delayDisplay('Test arrayFromTransformMatrix with toWorld=True')

    parentTransformNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLTransformNode')
    parentTransformMatrix = vtk.vtkMatrix4x4()
    parentTransformMatrix.SetElement(1,1, 0.3)
    parentTransformMatrix.SetElement(0,3, 30.0)
    parentTransformMatrix.SetElement(1,3, 20.0)
    parentTransformMatrix.SetElement(2,3, 10.0)
    parentTransformNode.SetMatrixTransformToParent(parentTransformMatrix)
    narrayParent = slicer.util.arrayFromTransformMatrix(parentTransformNode)

    transformNode.SetAndObserveTransformNodeID(parentTransformNode.GetID())

    narrayToWorld = slicer.util.arrayFromTransformMatrix(transformNode, toWorld=True)
    narrayToWorldExpected = np.dot(narrayParent, narray)
    np.testing.assert_array_equal(narrayToWorld, narrayToWorldExpected)

    self.delayDisplay('Test updateTransformMatrixFromArray')
    narrayUpdated = np.array([[1.5,0.5,0,4],[0,2.0,0,11],[0,0,2.5,6],[0,0,0,1]])
    slicer.util.updateTransformMatrixFromArray(transformNode, narrayUpdated)
    transformMatrixUpdated = vtk.vtkMatrix4x4()
    transformNode.GetMatrixTransformToParent(transformMatrixUpdated)
    for r in range(4):
      for c in range(4):
        self.assertEqual(narrayUpdated[r,c], transformMatrixUpdated.GetElement(r,c))

    self.delayDisplay('Test updateTransformMatrixFromArray with toWorld=True')
    narrayUpdated = np.array([[2.5,1.5,0,2],[0,2.0,0,15],[0,1,3.5,6],[0,0,0,1]])
    slicer.util.updateTransformMatrixFromArray(transformNode, narrayUpdated, toWorld=True)
    transformMatrixUpdated = vtk.vtkMatrix4x4()
    transformNode.GetMatrixTransformToWorld(transformMatrixUpdated)
    for r in range(4):
      for c in range(4):
        self.assertEqual(narrayUpdated[r,c], transformMatrixUpdated.GetElement(r,c))

  def test_arrayFromMarkupsControlPoints(self):
    # Test if retrieving markups control coordinates as a numpy array works
    import numpy as np

    self.delayDisplay('Test arrayFromMarkupsControlPoints')

    markupsNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode')
    markupsNode.AddControlPoint(vtk.vtkVector3d(10,20,30))
    markupsNode.AddControlPoint(vtk.vtkVector3d(21,21,31))
    markupsNode.AddControlPoint(vtk.vtkVector3d(32,33,44))
    markupsNode.AddControlPoint(vtk.vtkVector3d(45,45,55))
    markupsNode.AddControlPoint(vtk.vtkVector3d(51,41,59))

    translation = [10.0, 30.0, 20.0]
    transformNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLTransformNode')
    transformMatrix = vtk.vtkMatrix4x4()
    transformMatrix.SetElement(0,3, translation[0])
    transformMatrix.SetElement(1,3, translation[1])
    transformMatrix.SetElement(2,3, translation[2])
    transformNode.SetMatrixTransformToParent(transformMatrix)
    markupsNode.SetAndObserveTransformNodeID(transformNode.GetID())

    narray = slicer.util.arrayFromMarkupsControlPoints(markupsNode)
    self.assertEqual(narray.shape, (5, 3))
    self.assertEqual(narray[0,0], 10)
    self.assertEqual(narray[1,2], 31)
    self.assertEqual(narray[4,2], 59)

    self.delayDisplay('Test arrayFromMarkupsControlPoints with world=True')

    narray = slicer.util.arrayFromMarkupsControlPoints(markupsNode, world=True)
    self.assertEqual(narray.shape, (5, 3))
    self.assertEqual(narray[0,0], 10+translation[0])
    self.assertEqual(narray[1,2], 31+translation[2])
    self.assertEqual(narray[4,2], 59+translation[2])

    self.delayDisplay('Test updateMarkupsControlPointsFromArray')

    narray = np.array([[2,3,4],[6,7,8]])
    slicer.util.updateMarkupsControlPointsFromArray(markupsNode, narray)
    self.assertEqual(markupsNode.GetNumberOfControlPoints(), 2)
    position = [0]*3
    markupsNode.GetNthControlPointPosition(1,position)
    np.testing.assert_array_equal(position,narray[1,:])

    self.delayDisplay('Test updateMarkupsControlPointsFromArray with world=True')

    narray = np.array([[2,3,4],[6,7,8]])
    slicer.util.updateMarkupsControlPointsFromArray(markupsNode, narray, world=True)
    self.assertEqual(markupsNode.GetNumberOfControlPoints(), 2)
    markupsNode.GetNthControlPointPositionWorld(1,position)
    np.testing.assert_array_equal(position,narray[1,:])

  def test_array(self):
    # Test if convenience function of getting numpy array from various nodes works

    self.delayDisplay('Test array with scalar image')
    import SampleData
    volumeNode = SampleData.downloadSample("MRHead")
    voxelPos = [120,135,89]
    voxelValueVtk = volumeNode.GetImageData().GetScalarComponentAsDouble(voxelPos[0], voxelPos[1], voxelPos[2], 0)
    narray = slicer.util.arrayFromVolume(volumeNode)
    voxelValueNumpy = narray[voxelPos[2], voxelPos[1], voxelPos[0]]
    self.assertEqual(voxelValueVtk, voxelValueNumpy)

    # self.delayDisplay('Test array with tensor image')
    # tensorVolumeNode = SampleData.downloadSample('DTIBrain')
    # narray = slicer.util.array(tensorVolumeNode.GetName())
    # self.assertEqual(narray.shape, (85, 144, 144, 3, 3))

    self.delayDisplay('Test array with model points')
    sphere = vtk.vtkSphereSource()
    modelNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLModelNode')
    modelNode.SetPolyDataConnection(sphere.GetOutputPort())
    narray = slicer.util.array(modelNode.GetName())
    self.assertEqual(narray.shape, (50, 3))

    self.delayDisplay('Test array with markups fiducials')
    markupsNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsFiducialNode')
    markupsNode.AddControlPoint(vtk.vtkVector3d(10,20,30))
    markupsNode.AddControlPoint(vtk.vtkVector3d(21,21,31))
    markupsNode.AddControlPoint(vtk.vtkVector3d(32,33,44))
    markupsNode.AddControlPoint(vtk.vtkVector3d(45,45,55))
    markupsNode.AddControlPoint(vtk.vtkVector3d(51,41,59))
    narray = slicer.util.array(markupsNode.GetName())
    self.assertEqual(narray.shape, (5, 3))

    self.delayDisplay('Test array with transforms')
    transformNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLTransformNode')
    transformMatrix = vtk.vtkMatrix4x4()
    transformMatrix.SetElement(0,0, 2.0)
    transformMatrix.SetElement(0,3, 11.0)
    transformMatrix.SetElement(1,3, 22.0)
    transformMatrix.SetElement(2,3, 44.0)
    transformNode.SetMatrixTransformToParent(transformMatrix)
    narray = slicer.util.array(transformNode.GetName())
    self.assertEqual(narray.shape, (4, 4))

    self.delayDisplay('Testing slicer.util.test_array passed')

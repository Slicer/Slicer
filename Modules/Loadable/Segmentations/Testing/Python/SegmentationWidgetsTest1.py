import os
import unittest
import vtk, qt, ctk, slicer
import logging
from slicer.ScriptedLoadableModule import *

class SegmentationWidgetsTest1(ScriptedLoadableModuleTest):

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_SegmentationWidgetsTest1()

  #------------------------------------------------------------------------------
  def test_SegmentationWidgetsTest1(self):
    # Check for modules
    self.assertIsNotNone( slicer.modules.segmentations )

    self.TestSection_00_SetupPathsAndNames()
    self.TestSection_01_GenerateInputData()
    self.TestSection_02_qMRMLSegmentsTableView()
    self.TestSection_03_qMRMLSegmentationGeometryWidget()
    self.TestSection_04_qMRMLSegmentEditorWidget()

    logging.info('Test finished')

  #------------------------------------------------------------------------------
  def TestSection_00_SetupPathsAndNames(self):
    logging.info('Test section 0: SetupPathsAndNames')
    self.inputSegmentationNode = None

  #------------------------------------------------------------------------------
  def TestSection_01_GenerateInputData(self):
    logging.info('Test section 1: GenerateInputData')
    self.inputSegmentationNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLSegmentationNode')

    # Create new segments
    import random
    for segmentName in ['first', 'second', 'third']:
      sphereSegment = slicer.vtkSegment()
      sphereSegment.SetName(segmentName)
      sphereSegment.SetColor(random.uniform(0.0,1.0), random.uniform(0.0,1.0), random.uniform(0.0,1.0))

      sphere = vtk.vtkSphereSource()
      sphere.SetCenter(random.uniform(0,100),random.uniform(0,100),random.uniform(0,100))
      sphere.SetRadius(random.uniform(20,30))
      sphere.Update()
      spherePolyData = sphere.GetOutput()
      sphereSegment.AddRepresentation(
        slicer.vtkSegmentationConverter.GetSegmentationClosedSurfaceRepresentationName(),
        spherePolyData)

      self.inputSegmentationNode.GetSegmentation().AddSegment(sphereSegment)

    self.assertEqual(self.inputSegmentationNode.GetSegmentation().GetNumberOfSegments(), 3)

    self.inputSegmentationNode.CreateDefaultDisplayNodes()
    displayNode = self.inputSegmentationNode.GetDisplayNode()
    self.assertIsNotNone(displayNode)

  #------------------------------------------------------------------------------
  def TestSection_02_qMRMLSegmentsTableView(self):
    logging.info('Test section 2: qMRMLSegmentsTableView')

    displayNode = self.inputSegmentationNode.GetDisplayNode()
    self.assertIsNotNone(displayNode)

    segmentsTableView = slicer.qMRMLSegmentsTableView()
    segmentsTableView.setMRMLScene(slicer.mrmlScene)
    segmentsTableView.setSegmentationNode(self.inputSegmentationNode)
    self.assertEqual(len(segmentsTableView.displayedSegmentIDs()), 3)
    segmentsTableView.show()
    slicer.app.processEvents()
    slicer.util.delayDisplay("All shown")

    segmentsTableView.setHideSegments(['second'])
    self.assertEqual(len(segmentsTableView.displayedSegmentIDs()), 2)
    slicer.app.processEvents()
    slicer.util.delayDisplay("Hidden 'second'")

    segmentsTableView.setHideSegments([])
    segmentsTableView.filterBarVisible = True
    segmentsTableView.textFilter = "third"
    self.assertEqual(len(segmentsTableView.displayedSegmentIDs()), 1)
    slicer.app.processEvents()
    slicer.util.delayDisplay("All but 'third' filtered")

    segmentsTableView.textFilter = ""
    firstSegment = self.inputSegmentationNode.GetSegmentation().GetSegment("first")
    logic = slicer.modules.segmentations.logic()
    logic.SetSegmentStatus(firstSegment, logic.InProgress)
    sortFilterProxyModel = segmentsTableView.sortFilterProxyModel()
    sortFilterProxyModel.setShowStatus(logic.NotStarted, True)
    self.assertEqual(len(segmentsTableView.displayedSegmentIDs()), 2)
    slicer.app.processEvents()
    slicer.util.delayDisplay("'NotStarted' shown")


    segmentsTableView.setSelectedSegmentIDs(["third"])
    segmentsTableView.setHideSegments(['second'])
    segmentsTableView.showOnlySelectedSegments()
    self.assertEqual(displayNode.GetSegmentVisibility("first"), False)
    self.assertEqual(displayNode.GetSegmentVisibility("second"), True)
    self.assertEqual(displayNode.GetSegmentVisibility("third"), True)
    slicer.app.processEvents()
    slicer.util.delayDisplay("Show only selected segments")

    displayNode.SetSegmentVisibility("first", True)
    displayNode.SetSegmentVisibility("second", True)
    displayNode.SetSegmentVisibility("third", True)
    segmentsTableView.filterBarVisible = False

    # Reset the filtering parameters in the segmentation node to avoid interference with other tests that
    # use this segmentation node
    self.inputSegmentationNode.SetSegmentListFilterEnabled(False)
    self.inputSegmentationNode.SetSegmentListFilterOptions("")

  #------------------------------------------------------------------------------
  def compareOutputGeometry(self, orientedImageData, spacing, origin, directions):
    if orientedImageData is None:
      logging.error('Invalid input oriented image data')
      return False
    if (not isinstance(spacing, list) and not isinstance(spacing, tuple)) \
        or (not isinstance(origin, list) and not isinstance(origin, tuple)) \
        or not isinstance(directions, list):
      logging.error('Invalid baseline object types - need lists')
      return False
    if len(spacing) != 3 or len(origin) != 3 or len(directions) != 3 \
        or len(directions[0]) != 3 or len(directions[1]) != 3 or len(directions[2]) != 3:
      logging.error('Baseline lists need to contain 3 elements each, the directions 3 lists of 3')
      return False
    import numpy
    tolerance = 0.0001
    actualSpacing = orientedImageData.GetSpacing()
    actualOrigin = orientedImageData.GetOrigin()
    actualDirections = [[0]*3,[0]*3,[0]*3]
    orientedImageData.GetDirections(actualDirections)
    for i in [0,1,2]:
      if not numpy.isclose(spacing[i], actualSpacing[i], tolerance):
        logging.warning('Spacing discrepancy: ' + str(spacing) + ' != ' + str(actualSpacing))
        return False
      if not numpy.isclose(origin[i], actualOrigin[i], tolerance):
        logging.warning('Origin discrepancy: ' + str(origin) + ' != ' + str(actualOrigin))
        return False
      for j in [0,1,2]:
        if not numpy.isclose(directions[i][j], actualDirections[i][j], tolerance):
          logging.warning('Directions discrepancy: ' + str(directions) + ' != ' + str(actualDirections))
          return False
    return True

  #------------------------------------------------------------------------------
  def getForegroundVoxelCount(self, imageData):
    if imageData is None:
      logging.error('Invalid input image data')
      return False
    imageAccumulate = vtk.vtkImageAccumulate()
    imageAccumulate.SetInputData(imageData)
    imageAccumulate.SetIgnoreZero(1)
    imageAccumulate.Update()
    return imageAccumulate.GetVoxelCount()

  #------------------------------------------------------------------------------
  def TestSection_03_qMRMLSegmentationGeometryWidget(self):
    logging.info('Test section 2: qMRMLSegmentationGeometryWidget')

    binaryLabelmapReprName = slicer.vtkSegmentationConverter.GetBinaryLabelmapRepresentationName()
    closedSurfaceReprName = slicer.vtkSegmentationConverter.GetClosedSurfaceRepresentationName()

    # Use MRHead and Tinypatient for testing
    import SampleData
    mrVolumeNode = SampleData.downloadSample("MRHead")
    [tinyVolumeNode, tinySegmentationNode] = SampleData.downloadSamples('TinyPatient')

    # Convert MRHead to oriented image data
    import vtkSlicerSegmentationsModuleLogicPython as vtkSlicerSegmentationsModuleLogic
    mrOrientedImageData = vtkSlicerSegmentationsModuleLogic.vtkSlicerSegmentationsModuleLogic.CreateOrientedImageDataFromVolumeNode(mrVolumeNode)
    mrOrientedImageData.UnRegister(None)

    # Create segmentation node with binary labelmap master and one segment with MRHead geometry
    segmentationNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLSegmentationNode')
    segmentationNode.GetSegmentation().SetMasterRepresentationName(binaryLabelmapReprName)
    geometryStr = slicer.vtkSegmentationConverter.SerializeImageGeometry(mrOrientedImageData)
    segmentationNode.GetSegmentation().SetConversionParameter(
      slicer.vtkSegmentationConverter.GetReferenceImageGeometryParameterName(), geometryStr)

    threshold = vtk.vtkImageThreshold()
    threshold.SetInputData(mrOrientedImageData)
    threshold.ThresholdByUpper(16.0)
    threshold.SetInValue(1)
    threshold.SetOutValue(0)
    threshold.SetOutputScalarType(vtk.VTK_UNSIGNED_CHAR)
    threshold.Update()
    segmentOrientedImageData = slicer.vtkOrientedImageData()
    segmentOrientedImageData.DeepCopy(threshold.GetOutput())
    mrImageToWorldMatrix = vtk.vtkMatrix4x4()
    mrOrientedImageData.GetImageToWorldMatrix(mrImageToWorldMatrix)
    segmentOrientedImageData.SetImageToWorldMatrix(mrImageToWorldMatrix)
    segment = slicer.vtkSegment()
    segment.SetName('Brain')
    segment.SetColor(0.0,0.0,1.0)
    segment.AddRepresentation(binaryLabelmapReprName, segmentOrientedImageData)
    segmentationNode.GetSegmentation().AddSegment(segment)

    # Create geometry widget
    geometryWidget = slicer.qMRMLSegmentationGeometryWidget()
    geometryWidget.setSegmentationNode(segmentationNode)
    geometryWidget.editEnabled = True
    geometryImageData = slicer.vtkOrientedImageData() # To contain the output later

    # Volume source with no transforms
    geometryWidget.setSourceNode(tinyVolumeNode)
    geometryWidget.geometryImageData(geometryImageData)
    self.assertTrue(self.compareOutputGeometry(geometryImageData,
        (49,49,23), (248.8439, 248.2890, -123.75),
        [[-1.0, 0.0, 0.0], [0.0, -1.0, 0.0], [0.0, 0.0, 1.0]]))
    slicer.vtkOrientedImageDataResample.ResampleOrientedImageToReferenceOrientedImage(
      segmentOrientedImageData, geometryImageData, geometryImageData, False, True)
    self.assertEqual(self.getForegroundVoxelCount(geometryImageData), 92)

    # Transformed volume source
    translationTransformMatrix = vtk.vtkMatrix4x4()
    translationTransformMatrix.SetElement(0,3,24.5)
    translationTransformMatrix.SetElement(1,3,24.5)
    translationTransformMatrix.SetElement(2,3,11.5)
    translationTransformNode = slicer.vtkMRMLLinearTransformNode()
    translationTransformNode.SetName('TestTranslation')
    slicer.mrmlScene.AddNode(translationTransformNode)
    translationTransformNode.SetMatrixTransformToParent(translationTransformMatrix)

    tinyVolumeNode.SetAndObserveTransformNodeID(translationTransformNode.GetID())
    geometryWidget.geometryImageData(geometryImageData)
    self.assertTrue(self.compareOutputGeometry(geometryImageData,
        (49,49,23), (273.3439, 272.7890, -112.25),
        [[-1.0, 0.0, 0.0], [0.0, -1.0, 0.0], [0.0, 0.0, 1.0]]))
    slicer.vtkOrientedImageDataResample.ResampleOrientedImageToReferenceOrientedImage(
      segmentOrientedImageData, geometryImageData, geometryImageData, False, True)
    self.assertEqual(self.getForegroundVoxelCount(geometryImageData), 94)

    # Volume source with isotropic spacing
    tinyVolumeNode.SetAndObserveTransformNodeID(None)
    geometryWidget.setIsotropicSpacing(True)
    geometryWidget.geometryImageData(geometryImageData)
    self.assertTrue(self.compareOutputGeometry(geometryImageData,
        (23,23,23), (248.8439, 248.2890, -123.75),
        [[-1.0, 0.0, 0.0], [0.0, -1.0, 0.0], [0.0, 0.0, 1.0]]))
    slicer.vtkOrientedImageDataResample.ResampleOrientedImageToReferenceOrientedImage(
      segmentOrientedImageData, geometryImageData, geometryImageData, False, True)
    self.assertEqual(self.getForegroundVoxelCount(geometryImageData), 414)

    # Volume source with oversampling
    geometryWidget.setIsotropicSpacing(False)
    geometryWidget.setOversamplingFactor(2.0)
    geometryWidget.geometryImageData(geometryImageData)
    self.assertTrue(self.compareOutputGeometry(geometryImageData,
        (24.5, 24.5, 11.5), (261.0939, 260.5390, -129.5),
        [[-1.0, 0.0, 0.0], [0.0, -1.0, 0.0], [0.0, 0.0, 1.0]]))
    slicer.vtkOrientedImageDataResample.ResampleOrientedImageToReferenceOrientedImage(
      segmentOrientedImageData, geometryImageData, geometryImageData, False, True)
    self.assertEqual(self.getForegroundVoxelCount(geometryImageData), 751)
    slicer.util.delayDisplay('Volume source cases - OK')

    # Segmentation source with binary labelmap master
    geometryWidget.setOversamplingFactor(1.0)
    geometryWidget.setSourceNode(tinySegmentationNode)
    geometryWidget.geometryImageData(geometryImageData)
    self.assertTrue(self.compareOutputGeometry(geometryImageData,
        (49,49,23), (248.8439, 248.2890, -123.75),
        [[-1.0, 0.0, 0.0], [0.0, -1.0, 0.0], [0.0, 0.0, 1.0]]))
    slicer.vtkOrientedImageDataResample.ResampleOrientedImageToReferenceOrientedImage(
      segmentOrientedImageData, geometryImageData, geometryImageData, False, True)
    self.assertEqual(self.getForegroundVoxelCount(geometryImageData), 92)

    # Segmentation source with closed surface master
    tinySegmentationNode.GetSegmentation().SetConversionParameter('Smoothing factor', '0.0')
    self.assertTrue(tinySegmentationNode.GetSegmentation().CreateRepresentation(closedSurfaceReprName))
    tinySegmentationNode.GetSegmentation().SetMasterRepresentationName(closedSurfaceReprName)
    tinySegmentationNode.Modified() # Trigger re-calculation of geometry (only generic Modified event is observed)
    geometryWidget.geometryImageData(geometryImageData)
    self.assertTrue(self.compareOutputGeometry(geometryImageData,
        (1,1,1), (-86.645, 133.929, 116.786),  # current origin of the segmentation is kept
        [[0.0, 0.0, 1.0], [-1.0, 0.0, 0.0], [0.0, -1.0, 0.0]]))
    slicer.vtkOrientedImageDataResample.ResampleOrientedImageToReferenceOrientedImage(
      segmentOrientedImageData, geometryImageData, geometryImageData, False, True)
    self.assertEqual(self.getForegroundVoxelCount(geometryImageData), 5223040)
    slicer.util.delayDisplay('Segmentation source cases - OK')

    # Model source with no transform
    shNode = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
    outputFolderId = shNode.CreateFolderItem(shNode.GetSceneItemID(), 'ModelsFolder')
    success = vtkSlicerSegmentationsModuleLogic.vtkSlicerSegmentationsModuleLogic.ExportVisibleSegmentsToModels(
      tinySegmentationNode, outputFolderId )
    self.assertTrue(success)
    modelNode = slicer.util.getNode('Body_Contour')
    geometryWidget.setSourceNode(modelNode)
    geometryWidget.geometryImageData(geometryImageData)
    self.assertTrue(self.compareOutputGeometry(geometryImageData,
        (1,1,1), (-86.645, 133.929, 116.786),  # current origin of the segmentation is kept
        [[0.0, 0.0, 1.0], [-1.0, 0.0, 0.0], [0.0, -1.0, 0.0]]))
    slicer.vtkOrientedImageDataResample.ResampleOrientedImageToReferenceOrientedImage(
      segmentOrientedImageData, geometryImageData, geometryImageData, False, True)
    self.assertEqual(self.getForegroundVoxelCount(geometryImageData), 5223040)

    # Transformed model source
    rotationTransform = vtk.vtkTransform()
    rotationTransform.RotateX(45)
    rotationTransformMatrix = vtk.vtkMatrix4x4()
    rotationTransform.GetMatrix(rotationTransformMatrix)
    rotationTransformNode = slicer.vtkMRMLLinearTransformNode()
    rotationTransformNode.SetName('TestRotation')
    slicer.mrmlScene.AddNode(rotationTransformNode)
    rotationTransformNode.SetMatrixTransformToParent(rotationTransformMatrix)

    modelNode.SetAndObserveTransformNodeID(rotationTransformNode.GetID())
    modelNode.Modified()
    geometryWidget.geometryImageData(geometryImageData)
    self.assertTrue(self.compareOutputGeometry(geometryImageData,
        (1,1,1), (-86.645, 177.282, -12.122),
        [[0.0, 0.0, 1.0], [-0.7071, -0.7071, 0.0], [0.7071, -0.7071, 0.0]]))
    slicer.vtkOrientedImageDataResample.ResampleOrientedImageToReferenceOrientedImage(
      segmentOrientedImageData, geometryImageData, geometryImageData, False, True)
    self.assertEqual(self.getForegroundVoxelCount(geometryImageData), 5229164)

    # ROI source
    roiNode = slicer.vtkMRMLAnnotationROINode()
    roiNode.SetName('SourceROI')
    slicer.mrmlScene.AddNode(roiNode)
    roiNode.UnRegister(None)
    xyz = [0]*3
    center = [0]*3
    slicer.vtkMRMLSliceLogic.GetVolumeRASBox(tinyVolumeNode, xyz, center)
    radius = [x/2.0 for x in xyz]
    roiNode.SetXYZ(center)
    roiNode.SetRadiusXYZ(radius)
    geometryWidget.setSourceNode(roiNode)
    geometryWidget.geometryImageData(geometryImageData)
    self.assertTrue(self.compareOutputGeometry(geometryImageData,
        (1,1,1), (0.0, 0.0, 0.0),
        [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]]))
    slicer.vtkOrientedImageDataResample.ResampleOrientedImageToReferenceOrientedImage(
      segmentOrientedImageData, geometryImageData, geometryImageData, False, True)
    self.assertEqual(self.getForegroundVoxelCount(geometryImageData), 5224232)
    slicer.util.delayDisplay('Model and ROI source cases - OK')

    slicer.util.delayDisplay('Segmentation geometry widget test passed')

  #------------------------------------------------------------------------------
  def TestSection_04_qMRMLSegmentEditorWidget(self):
    logging.info('Test section 4: qMRMLSegmentEditorWidget')

    self.segmentEditorNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLSegmentEditorNode')
    self.assertIsNotNone(self.segmentEditorNode)

    self.inputSegmentationNode.SetSegmentListFilterEnabled(False)
    self.inputSegmentationNode.SetSegmentListFilterOptions("")

    displayNode = self.inputSegmentationNode.GetDisplayNode()
    self.assertIsNotNone(displayNode)

    segmentEditorWidget = slicer.qMRMLSegmentEditorWidget()
    segmentEditorWidget.setMRMLSegmentEditorNode(self.segmentEditorNode)
    segmentEditorWidget.setMRMLScene(slicer.mrmlScene)
    segmentEditorWidget.setSegmentationNode(self.inputSegmentationNode)
    segmentEditorWidget.installKeyboardShortcuts(segmentEditorWidget)
    segmentEditorWidget.setFocus(qt.Qt.OtherFocusReason)
    segmentEditorWidget.show()

    self.segmentEditorNode.SetSelectedSegmentID('first')
    self.assertEqual(self.segmentEditorNode.GetSelectedSegmentID(), 'first')
    slicer.app.processEvents()
    slicer.util.delayDisplay("First selected")

    segmentEditorWidget.selectNextSegment()
    self.assertEqual(self.segmentEditorNode.GetSelectedSegmentID(), 'second')
    slicer.app.processEvents()
    slicer.util.delayDisplay("Next segment")

    segmentEditorWidget.selectPreviousSegment()
    self.assertEqual(self.segmentEditorNode.GetSelectedSegmentID(), 'first')
    slicer.app.processEvents()
    slicer.util.delayDisplay("Previous segment")

    displayNode.SetSegmentVisibility('second', False)
    segmentEditorWidget.selectNextSegment()
    self.assertEqual(self.segmentEditorNode.GetSelectedSegmentID(), 'third')
    slicer.app.processEvents()
    slicer.util.delayDisplay("Next segment (with second segment hidden)")

    # Trying to go out of bounds past first segment
    segmentEditorWidget.selectPreviousSegment() #First
    self.assertEqual(self.segmentEditorNode.GetSelectedSegmentID(), 'first')
    segmentEditorWidget.selectPreviousSegment() #First
    self.assertEqual(self.segmentEditorNode.GetSelectedSegmentID(), 'first')
    segmentEditorWidget.selectPreviousSegment() #First
    self.assertEqual(self.segmentEditorNode.GetSelectedSegmentID(), 'first')
    slicer.app.processEvents()
    slicer.util.delayDisplay("Multiple previous segment")

    # Wrap around
    self.segmentEditorNode.SetSelectedSegmentID('third')
    segmentEditorWidget.selectNextSegment()
    self.assertEqual(self.segmentEditorNode.GetSelectedSegmentID(), 'first')
    slicer.util.delayDisplay("Wrap around segments")

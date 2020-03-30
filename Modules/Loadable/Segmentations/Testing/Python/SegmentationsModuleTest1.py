import os
import unittest
import vtk, qt, ctk, slicer
import logging
from slicer.ScriptedLoadableModule import *
from slicer.util import TESTING_DATA_URL
import vtkSegmentationCore


class SegmentationsModuleTest1(unittest.TestCase):

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_SegmentationsModuleTest1()

  #------------------------------------------------------------------------------
  def test_SegmentationsModuleTest1(self):
    # Check for modules
    self.assertIsNotNone( slicer.modules.segmentations )

    self.TestSection_SetupPathsAndNames()
    self.TestSection_RetrieveInputData()
    self.TestSection_LoadInputData()
    self.TestSection_AddRemoveSegment()
    self.TestSection_MergeLabelmapWithDifferentGeometries()
    self.TestSection_ImportExportSegment()
    self.TestSection_ImportExportSegment2()
    self.TestSection_SubjectHierarchy()

    logging.info('Test finished')

  #------------------------------------------------------------------------------
  def TestSection_SetupPathsAndNames(self):
    # Set up paths used for this test
    self.segmentationsModuleTestDir = slicer.app.temporaryPath + '/SegmentationsModuleTest'
    if not os.access(self.segmentationsModuleTestDir, os.F_OK):
      os.mkdir(self.segmentationsModuleTestDir)

    self.dataDir = self.segmentationsModuleTestDir + '/TinyPatient_Seg'
    if not os.access(self.dataDir, os.F_OK):
      os.mkdir(self.dataDir)
    self.dataSegDir = self.dataDir + '/TinyPatient_Structures.seg'

    self.dataZipFilePath = self.segmentationsModuleTestDir + '/TinyPatient_Seg.zip'

    # Define variables
    self.expectedNumOfFilesInDataDir = 4
    self.expectedNumOfFilesInDataSegDir = 2
    self.inputSegmentationNode = None
    self.bodySegmentName = 'Body_Contour'
    self.tumorSegmentName = 'Tumor_Contour'
    self.secondSegmentationNode = None
    self.sphereSegment = None
    self.sphereSegmentName = 'Sphere'
    self.closedSurfaceReprName = vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationClosedSurfaceRepresentationName()
    self.binaryLabelmapReprName = vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationBinaryLabelmapRepresentationName()

  #------------------------------------------------------------------------------
  def TestSection_RetrieveInputData(self):
    try:
      slicer.util.downloadAndExtractArchive(
        TESTING_DATA_URL + 'SHA256/b902f635ef2059cd3b4ba854c000b388e4a9e817a651f28be05c22511a317ec7',
        self.dataZipFilePath, self.segmentationsModuleTestDir,
        checksum='SHA256:b902f635ef2059cd3b4ba854c000b388e4a9e817a651f28be05c22511a317ec7')

      numOfFilesInDataDirTest = len([name for name in os.listdir(self.dataDir) if os.path.isfile(self.dataDir + '/' + name)])
      self.assertEqual( numOfFilesInDataDirTest, self.expectedNumOfFilesInDataDir )
      self.assertTrue( os.access(self.dataSegDir, os.F_OK) )
      numOfFilesInDataSegDirTest = len([name for name in os.listdir(self.dataSegDir) if os.path.isfile(self.dataSegDir + '/' + name)])
      self.assertEqual( numOfFilesInDataSegDirTest, self.expectedNumOfFilesInDataSegDir )

    except Exception as e:
      import traceback
      traceback.print_exc()
      logging.error('Test caused exception!\n' + str(e))

  #------------------------------------------------------------------------------
  def TestSection_LoadInputData(self):
    # Load into Slicer
    slicer.util.loadVolume(self.dataDir + '/TinyPatient_CT.nrrd')
    slicer.util.loadNodeFromFile(self.dataDir + '/TinyPatient_Structures.seg.vtm', "SegmentationFile", {})

    # Change master representation to closed surface (so that conversion is possible when adding segment)
    self.inputSegmentationNode = slicer.util.getNode('vtkMRMLSegmentationNode1')
    self.inputSegmentationNode.GetSegmentation().SetMasterRepresentationName(self.closedSurfaceReprName)

  #------------------------------------------------------------------------------
  def TestSection_AddRemoveSegment(self):
    # Add/remove segment from segmentation (check display properties, color table, etc.)
    logging.info('Test section: Add/remove segment')

    # Get baseline values
    displayNode = self.inputSegmentationNode.GetDisplayNode()
    self.assertIsNotNone(displayNode)
    # If segments are not found then the returned color is the pre-defined invalid color
    bodyColor = self.inputSegmentationNode.GetSegmentation().GetSegment(self.bodySegmentName).GetColor()
    logging.info("bodyColor: {0}".format(bodyColor))
    self.assertEqual(int(bodyColor[0]*100), 33)
    self.assertEqual(int(bodyColor[1]*100), 66)
    self.assertEqual(int(bodyColor[2]*100), 0)
    tumorColor = self.inputSegmentationNode.GetSegmentation().GetSegment(self.tumorSegmentName).GetColor()
    logging.info("tumorColor: {0}".format(tumorColor))
    self.assertEqual(int(tumorColor[0]*100), 100)
    self.assertEqual(int(tumorColor[1]*100), 0)
    self.assertEqual(int(tumorColor[2]*100), 0)

    # Create new segment
    sphere = vtk.vtkSphereSource()
    sphere.SetCenter(0,50,0)
    sphere.SetRadius(80)
    sphere.Update()
    spherePolyData = vtk.vtkPolyData()
    spherePolyData.DeepCopy(sphere.GetOutput())

    self.sphereSegment = vtkSegmentationCore.vtkSegment()
    self.sphereSegment.SetName(self.sphereSegmentName)
    self.sphereSegment.SetColor(0.0,0.0,1.0)
    self.sphereSegment.AddRepresentation(self.closedSurfaceReprName, spherePolyData)

    # Add segment to segmentation
    self.inputSegmentationNode.GetSegmentation().AddSegment(self.sphereSegment)
    self.assertEqual(self.inputSegmentationNode.GetSegmentation().GetNumberOfSegments(), 3)

    # Check merged labelmap
    mergedLabelmap = vtkSegmentationCore.vtkOrientedImageData()
    self.inputSegmentationNode.GetSegmentation().CreateRepresentation(self.binaryLabelmapReprName)
    self.inputSegmentationNode.GenerateMergedLabelmapForAllSegments(mergedLabelmap, 0)
    imageStat = vtk.vtkImageAccumulate()
    imageStat.SetInputData(mergedLabelmap)
    imageStat.SetComponentExtent(0,4,0,0,0,0)
    imageStat.SetComponentOrigin(0,0,0)
    imageStat.SetComponentSpacing(1,1,1)
    imageStat.Update()
    imageStatResult = imageStat.GetOutput()
    for i in range(4):
      logging.info("Volume {0}: {1}".format(i, imageStatResult.GetScalarComponentAsDouble(i,0,0,0)))
    self.assertEqual(imageStat.GetVoxelCount(), 1000)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(0,0,0,0), 786)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(1,0,0,0), 170)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(2,0,0,0), 4)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(3,0,0,0), 40)

    # Check if segment reorder is taken into account in merged labelmap generation
    # Change segment order
    sphereSegmentId = self.inputSegmentationNode.GetSegmentation().GetSegmentIdBySegment(self.sphereSegment)
    self.inputSegmentationNode.GetSegmentation().SetSegmentIndex(sphereSegmentId, 1)
    # Re-generate merged labelmap
    self.inputSegmentationNode.GenerateMergedLabelmapForAllSegments(mergedLabelmap, 0)
    imageStat.SetInputData(mergedLabelmap)
    imageStat.Update()
    imageStatResult = imageStat.GetOutput()
    for i in range(4):
      logging.info("Volume {0}: {1}".format(i, imageStatResult.GetScalarComponentAsDouble(i,0,0,0)))
    self.assertEqual(imageStat.GetVoxelCount(), 1000)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(0,0,0,0), 786)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(1,0,0,0), 170)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(2,0,0,0), 39)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(3,0,0,0), 5)

    # Remove segment from segmentation
    self.inputSegmentationNode.GetSegmentation().RemoveSegment(self.sphereSegmentName)
    self.assertEqual(self.inputSegmentationNode.GetSegmentation().GetNumberOfSegments(), 2)

  #------------------------------------------------------------------------------
  def TestSection_MergeLabelmapWithDifferentGeometries(self):
    # Merge labelmap when segments containing labelmaps with different geometries (both same directions, different directions)
    logging.info('Test section: Merge labelmap with different geometries')

    self.assertIsNotNone(self.sphereSegment)
    self.sphereSegment.RemoveRepresentation(self.binaryLabelmapReprName)
    self.assertIsNone(self.sphereSegment.GetRepresentation(self.binaryLabelmapReprName))

    # Create new segmentation with sphere segment
    self.secondSegmentationNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLSegmentationNode', 'Second')
    self.secondSegmentationNode.GetSegmentation().SetMasterRepresentationName(self.binaryLabelmapReprName)

    self.secondSegmentationNode.GetSegmentation().AddSegment(self.sphereSegment)

    # Check automatically converted labelmap. It is supposed to have the default geometry
    # (which is different than the one in the input segmentation)
    sphereLabelmap = self.sphereSegment.GetRepresentation(self.binaryLabelmapReprName)
    self.assertIsNotNone(sphereLabelmap)
    sphereLabelmapSpacing = sphereLabelmap.GetSpacing()
    self.assertAlmostEqual(sphereLabelmapSpacing[0], 0.629257364931788, 8)
    self.assertAlmostEqual(sphereLabelmapSpacing[1], 0.629257364931788, 8)
    self.assertAlmostEqual(sphereLabelmapSpacing[2], 0.629257364931788, 8)

    # Create binary labelmap in segmentation that will create the merged labelmap from
    # different geometries so that labelmap is not removed from sphere segment when adding
    self.inputSegmentationNode.GetSegmentation().CreateRepresentation(self.binaryLabelmapReprName)

    # Copy segment to input segmentation
    self.inputSegmentationNode.GetSegmentation().CopySegmentFromSegmentation(self.secondSegmentationNode.GetSegmentation(), self.sphereSegmentName)
    self.assertEqual(self.inputSegmentationNode.GetSegmentation().GetNumberOfSegments(), 3)

    # Check merged labelmap
    # Reference geometry has the tiny patient spacing, and it is oversampled to have similar
    # voxel size as the sphere labelmap with the uniform 0.629mm spacing
    mergedLabelmap = vtkSegmentationCore.vtkOrientedImageData()
    self.inputSegmentationNode.GenerateMergedLabelmapForAllSegments(mergedLabelmap, 0)
    mergedLabelmapSpacing = mergedLabelmap.GetSpacing()
    self.assertAlmostEqual(mergedLabelmapSpacing[0], 0.80327868852459, 8)
    self.assertAlmostEqual(mergedLabelmapSpacing[1], 0.80327868852459, 8)
    self.assertAlmostEqual(mergedLabelmapSpacing[2], 0.377049180327869, 8)

    imageStat = vtk.vtkImageAccumulate()
    imageStat.SetInputData(mergedLabelmap)
    imageStat.SetComponentExtent(0,5,0,0,0,0)
    imageStat.SetComponentOrigin(0,0,0)
    imageStat.SetComponentSpacing(1,1,1)
    imageStat.Update()
    imageStatResult = imageStat.GetOutput()
    for i in range(5):
      logging.info("Volume {0}: {1}".format(i, imageStatResult.GetScalarComponentAsDouble(i,0,0,0)))
    self.assertEqual(imageStat.GetVoxelCount(), 226981000)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(0,0,0,0), 178838889)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(1,0,0,0), 39705288)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(2,0,0,0), 890883)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(3,0,0,0), 7545940)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(4,0,0,0), 0)  # Built from color table and color four is removed in previous test section

  #------------------------------------------------------------------------------
  def TestSection_ImportExportSegment(self):
    # Import/export, both one label and all labels
    logging.info('Test section: Import/export segment')

    # Export single segment to model node
    bodyModelNode = slicer.vtkMRMLModelNode()
    bodyModelNode.SetName('BodyModel')
    slicer.mrmlScene.AddNode(bodyModelNode)

    bodySegment = self.inputSegmentationNode.GetSegmentation().GetSegment(self.bodySegmentName)
    result = slicer.vtkSlicerSegmentationsModuleLogic.ExportSegmentToRepresentationNode(bodySegment, bodyModelNode)
    self.assertTrue(result)
    self.assertIsNotNone(bodyModelNode.GetPolyData())
    #TODO: Number of points increased to 1677 due to end-capping, need to investigate!
    #self.assertEqual(bodyModelNode.GetPolyData().GetNumberOfPoints(), 302)
    #TODO: On Linux and Windows it is 588, on Mac it is 580. Need to investigate
    # self.assertEqual(bodyModelNode.GetPolyData().GetNumberOfCells(), 588)
    #self.assertTrue(bodyModelNode.GetPolyData().GetNumberOfCells() == 588 or bodyModelNode.GetPolyData().GetNumberOfCells() == 580)

    # Export single segment to volume node
    bodyLabelmapNode = slicer.vtkMRMLLabelMapVolumeNode()
    bodyLabelmapNode.SetName('BodyLabelmap')
    slicer.mrmlScene.AddNode(bodyLabelmapNode)
    result = slicer.vtkSlicerSegmentationsModuleLogic.ExportSegmentToRepresentationNode(bodySegment, bodyLabelmapNode)
    self.assertTrue(result)
    bodyImageData = bodyLabelmapNode.GetImageData()
    self.assertIsNotNone(bodyImageData)
    imageStat = vtk.vtkImageAccumulate()
    imageStat.SetInputData(bodyImageData)
    imageStat.Update()
    self.assertEqual(imageStat.GetVoxelCount(), 792)
    self.assertEqual(imageStat.GetMin()[0], 0)
    self.assertEqual(imageStat.GetMax()[0], 1)

    # Export multiple segments to volume node
    allSegmentsLabelmapNode = slicer.vtkMRMLLabelMapVolumeNode()
    allSegmentsLabelmapNode.SetName('AllSegmentsLabelmap')
    slicer.mrmlScene.AddNode(allSegmentsLabelmapNode)
    result = slicer.vtkSlicerSegmentationsModuleLogic.ExportAllSegmentsToLabelmapNode(self.inputSegmentationNode, allSegmentsLabelmapNode)
    self.assertTrue(result)
    allSegmentsImageData = allSegmentsLabelmapNode.GetImageData()
    self.assertIsNotNone(allSegmentsImageData)
    imageStat = vtk.vtkImageAccumulate()
    imageStat.SetInputData(allSegmentsImageData)
    imageStat.SetComponentExtent(0,5,0,0,0,0)
    imageStat.SetComponentOrigin(0,0,0)
    imageStat.SetComponentSpacing(1,1,1)
    imageStat.Update()
    imageStatResult = imageStat.GetOutput()
    for i in range(4):
      logging.info("Volume {0}: {1}".format(i, imageStatResult.GetScalarComponentAsDouble(i,0,0,0)))
    self.assertEqual(imageStat.GetVoxelCount(), 127109360)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(0,0,0,0), 78967249)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(1,0,0,0), 39705288)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(2,0,0,0), 890883)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(3,0,0,0), 7545940)
    # Import model to segment
    modelImportSegmentationNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLSegmentationNode', 'ModelImport')
    modelImportSegmentationNode.GetSegmentation().SetMasterRepresentationName(self.closedSurfaceReprName)
    modelSegment = slicer.vtkSlicerSegmentationsModuleLogic.CreateSegmentFromModelNode(bodyModelNode)
    modelSegment.UnRegister(None) # Need to release ownership
    self.assertIsNotNone(modelSegment)
    self.assertIsNotNone(modelSegment.GetRepresentation(self.closedSurfaceReprName))

    # Import multi-label labelmap to segmentation
    multiLabelImportSegmentationNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLSegmentationNode', 'MultiLabelImport')
    multiLabelImportSegmentationNode.GetSegmentation().SetMasterRepresentationName(self.binaryLabelmapReprName)
    result = slicer.vtkSlicerSegmentationsModuleLogic.ImportLabelmapToSegmentationNode(allSegmentsLabelmapNode, multiLabelImportSegmentationNode)
    self.assertTrue(result)
    self.assertEqual(multiLabelImportSegmentationNode.GetSegmentation().GetNumberOfSegments(), 3)

    # Import labelmap into single segment
    singleLabelImportSegmentationNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLSegmentationNode', 'SingleLabelImport')
    singleLabelImportSegmentationNode.GetSegmentation().SetMasterRepresentationName(self.binaryLabelmapReprName)
    # Should not import multi-label labelmap to segment
    nullSegment = slicer.vtkSlicerSegmentationsModuleLogic.CreateSegmentFromLabelmapVolumeNode(allSegmentsLabelmapNode)
    self.assertIsNone(nullSegment)
    logging.info('(This error message is a result of testing an impossible scenario, it is supposed to appear)')
    # Make labelmap single-label and import again
    threshold = vtk.vtkImageThreshold()
    threshold.SetInValue(0)
    threshold.SetOutValue(1)
    threshold.ReplaceInOn()
    threshold.ThresholdByLower(0)
    threshold.SetOutputScalarType(vtk.VTK_UNSIGNED_CHAR)
    if vtk.VTK_MAJOR_VERSION <= 5:
      threshold.SetInput(allSegmentsLabelmapNode.GetImageData())
    else:
      threshold.SetInputData(allSegmentsLabelmapNode.GetImageData())
    threshold.Update()
    allSegmentsLabelmapNode.GetImageData().ShallowCopy(threshold.GetOutput())
    labelSegment = slicer.vtkSlicerSegmentationsModuleLogic.CreateSegmentFromLabelmapVolumeNode(allSegmentsLabelmapNode)
    labelSegment.UnRegister(None) # Need to release ownership
    self.assertIsNotNone(labelSegment)
    self.assertIsNotNone(labelSegment.GetRepresentation(self.binaryLabelmapReprName))

    # Import/export with transforms
    logging.info('Test subsection: Import/export with transforms')

    # Create transform node that will be used to transform the tested nodes
    bodyModelTransformNode = slicer.vtkMRMLLinearTransformNode()
    slicer.mrmlScene.AddNode(bodyModelTransformNode)
    bodyModelTransform = vtk.vtkTransform()
    bodyModelTransform.Translate(1000.0, 0.0, 0.0)
    bodyModelTransformNode.ApplyTransformMatrix(bodyModelTransform.GetMatrix())

    # Set transform as parent to input segmentation node
    self.inputSegmentationNode.SetAndObserveTransformNodeID(bodyModelTransformNode.GetID())

    # Export single segment to model node from transformed segmentation
    bodyModelNodeTransformed = slicer.vtkMRMLModelNode()
    bodyModelNodeTransformed.SetName('BodyModelTransformed')
    slicer.mrmlScene.AddNode(bodyModelNodeTransformed)
    bodySegment = self.inputSegmentationNode.GetSegmentation().GetSegment(self.bodySegmentName)
    result = slicer.vtkSlicerSegmentationsModuleLogic.ExportSegmentToRepresentationNode(bodySegment, bodyModelNodeTransformed)
    self.assertTrue(result)
    self.assertIsNotNone(bodyModelNodeTransformed.GetParentTransformNode())

    # Export single segment to volume node from transformed segmentation
    bodyLabelmapNodeTransformed = slicer.vtkMRMLLabelMapVolumeNode()
    bodyLabelmapNodeTransformed.SetName('BodyLabelmapTransformed')
    slicer.mrmlScene.AddNode(bodyLabelmapNodeTransformed)
    result = slicer.vtkSlicerSegmentationsModuleLogic.ExportSegmentToRepresentationNode(bodySegment, bodyLabelmapNodeTransformed)
    self.assertTrue(result)
    self.assertIsNotNone(bodyLabelmapNodeTransformed.GetParentTransformNode())

    # Create transform node that will be used to transform the tested nodes
    modelTransformedImportSegmentationTransformNode = slicer.vtkMRMLLinearTransformNode()
    slicer.mrmlScene.AddNode(modelTransformedImportSegmentationTransformNode)
    modelTransformedImportSegmentationTransform = vtk.vtkTransform()
    modelTransformedImportSegmentationTransform.Translate(-500.0, 0.0, 0.0)
    modelTransformedImportSegmentationTransformNode.ApplyTransformMatrix(modelTransformedImportSegmentationTransform.GetMatrix())

    # Import transformed model to segment in transformed segmentation
    modelTransformedImportSegmentationNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLSegmentationNode', 'ModelImportTransformed')
    modelTransformedImportSegmentationNode.GetSegmentation().SetMasterRepresentationName(self.closedSurfaceReprName)
    modelTransformedImportSegmentationNode.SetAndObserveTransformNodeID(modelTransformedImportSegmentationTransformNode.GetID())
    modelSegmentTranformed = slicer.vtkSlicerSegmentationsModuleLogic.CreateSegmentFromModelNode(bodyModelNodeTransformed, modelTransformedImportSegmentationNode)
    modelSegmentTranformed.UnRegister(None) # Need to release ownership
    self.assertIsNotNone(modelSegmentTranformed)
    modelSegmentTransformedPolyData = modelSegmentTranformed.GetRepresentation(self.closedSurfaceReprName)
    self.assertIsNotNone(modelSegmentTransformedPolyData)
    self.assertEqual(int(modelSegmentTransformedPolyData.GetBounds()[0]), 1332)
    self.assertEqual(int(modelSegmentTransformedPolyData.GetBounds()[1]), 1675)

    # Clean up temporary nodes
    slicer.mrmlScene.RemoveNode(bodyModelNode)
    slicer.mrmlScene.RemoveNode(bodyLabelmapNode)
    slicer.mrmlScene.RemoveNode(allSegmentsLabelmapNode)
    slicer.mrmlScene.RemoveNode(modelImportSegmentationNode)
    slicer.mrmlScene.RemoveNode(multiLabelImportSegmentationNode)
    slicer.mrmlScene.RemoveNode(singleLabelImportSegmentationNode)
    slicer.mrmlScene.RemoveNode(bodyModelTransformNode)
    slicer.mrmlScene.RemoveNode(bodyModelNodeTransformed)
    slicer.mrmlScene.RemoveNode(bodyLabelmapNodeTransformed)
    slicer.mrmlScene.RemoveNode(modelTransformedImportSegmentationNode)

  def TestSection_ImportExportSegment2(self):
    # Testing sequential add of individual segments to a segmentation through ImportLabelmapToSegmentationNode
    logging.info('Test section: Import/export segment 2')

    # Export body segment to volume node
    bodySegment = self.inputSegmentationNode.GetSegmentation().GetSegment(self.bodySegmentName)
    bodyLabelmapNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLLabelMapVolumeNode', 'BodyLabelmap')
    result = slicer.vtkSlicerSegmentationsModuleLogic.ExportSegmentToRepresentationNode(bodySegment, bodyLabelmapNode)
    self.assertTrue(result)
    bodyImageData = bodyLabelmapNode.GetImageData()
    self.assertIsNotNone(bodyImageData)
    imageStat = vtk.vtkImageAccumulate()
    imageStat.SetInputData(bodyImageData)
    imageStat.Update()
    self.assertEqual(imageStat.GetVoxelCount(), 792)
    self.assertEqual(imageStat.GetMin()[0], 0)
    self.assertEqual(imageStat.GetMax()[0], 1)

    # Export tumor segment to volume node
    tumorSegment = self.inputSegmentationNode.GetSegmentation().GetSegment(self.tumorSegmentName)
    tumorLabelmapNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLLabelMapVolumeNode', 'TumorLabelmap')
    result = slicer.vtkSlicerSegmentationsModuleLogic.ExportSegmentToRepresentationNode(tumorSegment, tumorLabelmapNode)
    self.assertTrue(result)
    tumorImageData = tumorLabelmapNode.GetImageData()
    self.assertIsNotNone(tumorImageData)
    imageStat = vtk.vtkImageAccumulate()
    imageStat.SetInputData(tumorImageData)
    imageStat.Update()
    self.assertEqual(imageStat.GetVoxelCount(), 12)
    self.assertEqual(imageStat.GetMin()[0], 0)
    self.assertEqual(imageStat.GetMax()[0], 1)

    # Import single-label labelmap to segmentation
    singleLabelImportSegmentationNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLSegmentationNode', 'SingleLabelImport')
    singleLabelImportSegmentationNode.GetSegmentation().SetMasterRepresentationName(self.binaryLabelmapReprName)

    bodySegmentID = singleLabelImportSegmentationNode.GetSegmentation().AddEmptySegment('BodyLabelmap')
    bodySegmentIDArray = vtk.vtkStringArray()
    bodySegmentIDArray.SetNumberOfValues(1)
    bodySegmentIDArray.SetValue(0, bodySegmentID)
    result = slicer.vtkSlicerSegmentationsModuleLogic.ImportLabelmapToSegmentationNode(bodyLabelmapNode, singleLabelImportSegmentationNode, bodySegmentIDArray)

    self.assertTrue(result)
    self.assertEqual(singleLabelImportSegmentationNode.GetSegmentation().GetNumberOfSegments(), 1)

    tumorSegmentID = singleLabelImportSegmentationNode.GetSegmentation().AddEmptySegment('TumorLabelmap')
    tumorSegmentIDArray = vtk.vtkStringArray()
    tumorSegmentIDArray.SetNumberOfValues(1)
    tumorSegmentIDArray.SetValue(0, tumorSegmentID)
    result = slicer.vtkSlicerSegmentationsModuleLogic.ImportLabelmapToSegmentationNode(tumorLabelmapNode, singleLabelImportSegmentationNode, tumorSegmentIDArray)
    self.assertTrue(result)
    self.assertEqual(singleLabelImportSegmentationNode.GetSegmentation().GetNumberOfSegments(), 2)

    bodyLabelmap = slicer.vtkOrientedImageData()
    singleLabelImportSegmentationNode.GetBinaryLabelmapRepresentation(bodySegmentID, bodyLabelmap)
    imageStat = vtk.vtkImageAccumulate()
    imageStat.SetInputData(bodyLabelmap)
    imageStat.Update()
    self.assertEqual(imageStat.GetVoxelCount(), 792)
    self.assertEqual(imageStat.GetMin()[0], 0)
    self.assertEqual(imageStat.GetMax()[0], 1)

    tumorLabelmap = slicer.vtkOrientedImageData()
    singleLabelImportSegmentationNode.GetBinaryLabelmapRepresentation(tumorSegmentID, tumorLabelmap)
    self.assertIsNotNone(tumorLabelmap)
    imageStat = vtk.vtkImageAccumulate()
    imageStat.SetInputData(tumorLabelmap)
    imageStat.Update()
    self.assertEqual(imageStat.GetVoxelCount(), 12)
    self.assertEqual(imageStat.GetMin()[0], 0)
    self.assertEqual(imageStat.GetMax()[0], 1)

    # Clean up temporary nodes
    slicer.mrmlScene.RemoveNode(bodyLabelmapNode)
    slicer.mrmlScene.RemoveNode(tumorLabelmapNode)
    slicer.mrmlScene.RemoveNode(singleLabelImportSegmentationNode)

  #------------------------------------------------------------------------------
  def TestSection_SubjectHierarchy(self):
    # Subject hierarchy plugin: item creation, removal, renaming
    logging.info('Test section: Subject hierarchy')

    shNode = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
    self.assertIsNotNone( shNode )

    # Check if subject hierarchy items have been created
    segmentationShItemID = shNode.GetItemByDataNode(self.inputSegmentationNode)
    self.assertIsNotNone( segmentationShItemID )

    bodyItemID = shNode.GetItemChildWithName(segmentationShItemID, self.bodySegmentName)
    self.assertIsNotNone( bodyItemID )
    tumorItemID  = shNode.GetItemChildWithName(segmentationShItemID, self.tumorSegmentName)
    self.assertIsNotNone( tumorItemID )
    sphereItemID  = shNode.GetItemChildWithName(segmentationShItemID, self.sphereSegmentName)
    self.assertIsNotNone( sphereItemID )

    # Rename segment
    bodySegment = self.inputSegmentationNode.GetSegmentation().GetSegment(self.bodySegmentName)
    bodySegment.SetName('Body')
    qt.QApplication.processEvents()
    self.assertEqual( shNode.GetItemName(bodyItemID), 'Body')

    tumorSegment = self.inputSegmentationNode.GetSegmentation().GetSegment(self.tumorSegmentName)
    shNode.SetItemName(tumorItemID, 'Tumor')
    qt.QApplication.processEvents()
    self.assertEqual( tumorSegment.GetName(), 'Tumor')

    # Remove segment
    self.inputSegmentationNode.GetSegmentation().RemoveSegment(bodySegment)
    qt.QApplication.processEvents()
    logging.info('(The error messages below are results of testing invalidity of objects, they are supposed to appear)')
    self.assertEqual( shNode.GetItemChildWithName(segmentationShItemID, 'Body'), 0)
    self.assertEqual( self.inputSegmentationNode.GetSegmentation().GetNumberOfSegments(), 2)

    shNode.RemoveItem(tumorItemID)
    qt.QApplication.processEvents()
    self.assertEqual( self.inputSegmentationNode.GetSegmentation().GetNumberOfSegments(), 1 )

    # Remove segmentation
    slicer.mrmlScene.RemoveNode(self.inputSegmentationNode)
    self.assertEqual( shNode.GetItemName(segmentationShItemID), '')
    self.assertEqual( shNode.GetItemName(sphereItemID), '')

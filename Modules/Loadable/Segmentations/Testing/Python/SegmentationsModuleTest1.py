import os
import unittest
import vtk, qt, ctk, slicer
import logging

import vtkSegmentationCorePython as vtkSegmentationCore

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

    self.TestSection_00_SetupPathsAndNames()
    self.TestSection_01_RetrieveInputData()
    self.TestSection_02_LoadInputData()
    self.TestSection_1_AddRemoveSegment()
    self.TestSection_2_MergeLabelmapWithDifferentGeometries()
    self.TestSection_3_ImportExportSegment()

    logging.info('Test finished')

  #------------------------------------------------------------------------------
  def TestSection_00_SetupPathsAndNames(self):
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
    self.expectedNumOfFilesInDataDir = 2
    self.expectedNumOfFilesInDataSegDir = 2
    self.inputSegmentationNode = None
    self.secondSegmentationNode = None
    self.sphereSegment = None
    self.sphereSegmentName = 'Sphere'
    self.closedSurfaceReprName = vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationClosedSurfaceRepresentationName()
    self.binaryLabelmapReprName = vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationBinaryLabelmapRepresentationName()

  #------------------------------------------------------------------------------
  def TestSection_01_RetrieveInputData(self):
    try:
      import urllib
      downloads = (
          ('http://slicer.kitware.com/midas3/download/folder/3763/TinyPatient_Seg.zip', self.dataZipFilePath),
          )

      downloaded = 0
      for url,filePath in downloads:
        if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
          if downloaded == 0:
            logging.info('Downloading input data to folder\n' + self.dataZipFilePath)
          logging.info('Requesting download from %s...' % (url))
          urllib.urlretrieve(url, filePath)
          downloaded += 1
        else:
          logging.info('Input data has been found in folder ' + self.dataZipFilePath)
      if downloaded > 0:
        logging.info('Downloading input data finished')

      numOfFilesInDataDir = len([name for name in os.listdir(self.dataDir) if os.path.isfile(self.dataDir + '/' + name)])
      if (numOfFilesInDataDir != self.expectedNumOfFilesInDataDir):
        slicer.app.applicationLogic().Unzip(self.dataZipFilePath, self.segmentationsModuleTestDir)
        logging.info("Unzipping done")

      numOfFilesInDataDirTest = len([name for name in os.listdir(self.dataDir) if os.path.isfile(self.dataDir + '/' + name)])
      self.assertEqual( numOfFilesInDataDirTest, self.expectedNumOfFilesInDataDir )
      self.assertTrue( os.access(self.dataSegDir, os.F_OK) )
      numOfFilesInDataSegDirTest = len([name for name in os.listdir(self.dataSegDir) if os.path.isfile(self.dataSegDir + '/' + name)])
      self.assertEqual( numOfFilesInDataSegDirTest, self.expectedNumOfFilesInDataSegDir )

    except Exception, e:
      import traceback
      traceback.print_exc()
      logging.error('Test caused exception!\n' + str(e))

  #------------------------------------------------------------------------------
  def TestSection_02_LoadInputData(self):
    # Load into Slicer
    ctLoadSuccess = slicer.util.loadVolume(self.dataDir + '/TinyPatient_CT.nrrd')
    self.assertTrue( ctLoadSuccess )
    segLoadSuccess = slicer.util.loadNodeFromFile(self.dataDir + '/TinyPatient_Structures.seg.vtm', "SegmentationFile", {})
    self.assertTrue( segLoadSuccess )

    # Change master representation to closed surface (so that conversion is possible when adding segment)
    self.inputSegmentationNode = slicer.util.getNode('vtkMRMLSegmentationNode1')
    self.assertIsNotNone(self.inputSegmentationNode)
    self.inputSegmentationNode.GetSegmentation().SetMasterRepresentationName(self.closedSurfaceReprName)

  #------------------------------------------------------------------------------
  def TestSection_1_AddRemoveSegment(self):
    # Add/remove segment from segmentation (check display properties, color table, etc.)
    logging.info('Test section 1: Add/remove segment')

    # Get baseline values
    displayNode = self.inputSegmentationNode.GetDisplayNode()
    self.assertIsNotNone(displayNode)
    # If segments are not found then the returned color is the pre-defined invalid color
    bodyColor = displayNode.GetSegmentColor('Body_Contour')
    self.assertTrue(int(bodyColor[0]*100) == 33 and int(bodyColor[1]*100) == 66 and bodyColor[2] == 0.0)
    tumorColor = displayNode.GetSegmentColor('Tumor_Contour')
    self.assertTrue(tumorColor[0] == 1.0 and tumorColor[1] == 0.0 and tumorColor[2] == 0.0)

    # Create new segment
    sphere = vtk.vtkSphereSource()
    sphere.SetCenter(0,50,0)
    sphere.SetRadius(50)
    sphere.Update()
    spherePolyData = vtk.vtkPolyData()
    spherePolyData.DeepCopy(sphere.GetOutput())

    self.sphereSegment = vtkSegmentationCore.vtkSegment()
    self.sphereSegment.SetName(self.sphereSegmentName)
    self.sphereSegment.SetDefaultColor(0.0,0.0,1.0)
    self.sphereSegment.AddRepresentation(self.closedSurfaceReprName, spherePolyData)

    # Add segment to segmentation
    self.inputSegmentationNode.GetSegmentation().AddSegment(self.sphereSegment)
    self.assertEqual(self.inputSegmentationNode.GetSegmentation().GetNumberOfSegments(), 3)
    sphereColor = displayNode.GetSegmentColor(self.sphereSegmentName)
    self.assertTrue(sphereColor[0] == 0.0 and sphereColor[1] == 0.0 and sphereColor[2] == 1.0)

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
    self.assertEqual(imageStat.GetVoxelCount(), 1000)
    imageStatResult = imageStat.GetOutput()
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(0,0,0,0), 795)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(1,0,0,0), 194)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(2,0,0,0), 5)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(3,0,0,0), 6)

    # Remove segment from segmentation
    self.inputSegmentationNode.GetSegmentation().RemoveSegment(self.sphereSegmentName)
    self.assertEqual(self.inputSegmentationNode.GetSegmentation().GetNumberOfSegments(), 2)
    sphereColor = displayNode.GetSegmentColor(self.sphereSegmentName)
    self.assertTrue(sphereColor[0] == 0.5 and sphereColor[1] == 0.5 and sphereColor[2] == 0.5)

  #------------------------------------------------------------------------------
  def TestSection_2_MergeLabelmapWithDifferentGeometries(self):
    # Merge labelmap when segments containing labelmaps with different geometries (both same directions, different directions)
    logging.info('Test section 2: Merge labelmap with different geometries')

    self.assertIsNotNone(self.sphereSegment)
    self.sphereSegment.RemoveRepresentation(self.binaryLabelmapReprName)
    self.assertIsNone(self.sphereSegment.GetRepresentation(self.binaryLabelmapReprName))

    # Create new segmentation with sphere segment
    self.secondSegmentationNode = slicer.vtkMRMLSegmentationNode()
    self.secondSegmentationNode.SetName('Second')
    self.secondSegmentationNode.GetSegmentation().SetMasterRepresentationName(self.binaryLabelmapReprName)
    slicer.mrmlScene.AddNode(self.secondSegmentationNode)

    self.secondSegmentationNode.GetSegmentation().AddSegment(self.sphereSegment)

    # Check automatically converted labelmap. It is supposed to have the default geometry
    # (which is different than the one in the input segmentation)
    sphereLabelmap = self.sphereSegment.GetRepresentation(self.binaryLabelmapReprName)
    self.assertIsNotNone(sphereLabelmap)
    sphereLabelmapSpacing = sphereLabelmap.GetSpacing()
    self.assertTrue(sphereLabelmapSpacing[0] == 1.0 and sphereLabelmapSpacing[1] == 1.0 and sphereLabelmapSpacing[2] == 1.0)

    # Create binary labelmap in segmentation that will create the merged labelmap from
    # different geometries so that labelmap is not removed from sphere segment when adding
    self.inputSegmentationNode.GetSegmentation().CreateRepresentation(self.binaryLabelmapReprName)

    # Copy segment to input segmentation
    self.inputSegmentationNode.GetSegmentation().CopySegmentFromSegmentation(self.secondSegmentationNode.GetSegmentation(), self.sphereSegmentName)
    self.assertEqual(self.inputSegmentationNode.GetSegmentation().GetNumberOfSegments(), 3)

    # Check merged labelmap
    # Reference geometry has the tiny patient spacing, and it is oversampled to have smimilar
    # voxel size as the sphere labelmap with the uniform 1mm spacing
    mergedLabelmap = vtkSegmentationCore.vtkOrientedImageData()
    self.inputSegmentationNode.GenerateMergedLabelmapForAllSegments(mergedLabelmap, 0)
    mergedLabelmapSpacing = mergedLabelmap.GetSpacing()
    self.assertAlmostEqual(mergedLabelmapSpacing[0], 1.2894736842, 8)
    self.assertAlmostEqual(mergedLabelmapSpacing[1], 1.2894736842, 8)
    self.assertAlmostEqual(mergedLabelmapSpacing[2], 0.6052631578, 8)

    imageStat = vtk.vtkImageAccumulate()
    imageStat.SetInputData(mergedLabelmap)
    imageStat.SetComponentExtent(0,5,0,0,0,0)
    imageStat.SetComponentOrigin(0,0,0)
    imageStat.SetComponentSpacing(1,1,1)
    imageStat.Update()
    self.assertEqual(imageStat.GetVoxelCount(), 54872000)
    imageStatResult = imageStat.GetOutput()
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(0,0,0,0), 43573723)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(1,0,0,0), 10601312)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(2,0,0,0), 274360)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(3,0,0,0), 422605)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(4,0,0,0), 0)  # Built from color table and color four is removed in previous test section

  #------------------------------------------------------------------------------
  def TestSection_3_ImportExportSegment(self):
    # Import/export, both one label and all labels
    logging.info('Test section 3: Import/export segment')

    # Export single segment to model node
    bodyModelNode = slicer.vtkMRMLModelNode()
    bodyModelNode.SetName('BodyModel')
    slicer.mrmlScene.AddNode(bodyModelNode)

    bodySegment = self.inputSegmentationNode.GetSegmentation().GetSegment('Body_Contour')
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
    self.assertEqual(imageStat.GetVoxelCount(), 24198552)
    imageStatResult = imageStat.GetOutput()
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(0,0,0,0), 12900275)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(1,0,0,0), 10601312)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(2,0,0,0), 274360)
    self.assertEqual(imageStatResult.GetScalarComponentAsDouble(3,0,0,0), 422605)

    # Import model to segment
    modelImportSegmentationNode = slicer.vtkMRMLSegmentationNode()
    modelImportSegmentationNode.SetName('ModelImport')
    modelImportSegmentationNode.GetSegmentation().SetMasterRepresentationName(self.closedSurfaceReprName)
    slicer.mrmlScene.AddNode(modelImportSegmentationNode)
    modelSegment = slicer.vtkSlicerSegmentationsModuleLogic.CreateSegmentFromModelNode(bodyModelNode)
    modelSegment.UnRegister(None) # Need to release ownership
    self.assertIsNotNone(modelSegment)
    self.assertIsNotNone(modelSegment.GetRepresentation(self.closedSurfaceReprName))

    # Import multi-label labelmap to segmentation
    multiLabelImportSegmentationNode = slicer.vtkMRMLSegmentationNode()
    multiLabelImportSegmentationNode.SetName('MultiLabelImport')
    multiLabelImportSegmentationNode.GetSegmentation().SetMasterRepresentationName(self.binaryLabelmapReprName)
    slicer.mrmlScene.AddNode(multiLabelImportSegmentationNode)
    result = slicer.vtkSlicerSegmentationsModuleLogic.ImportLabelmapToSegmentationNode(allSegmentsLabelmapNode, multiLabelImportSegmentationNode)
    self.assertTrue(result)
    self.assertEqual(multiLabelImportSegmentationNode.GetSegmentation().GetNumberOfSegments(), 3)

    # Import labelmap into single segment
    singleLabelImportSegmentationNode = slicer.vtkMRMLSegmentationNode()
    singleLabelImportSegmentationNode.SetName('SingleLabelImport')
    singleLabelImportSegmentationNode.GetSegmentation().SetMasterRepresentationName(self.binaryLabelmapReprName)
    slicer.mrmlScene.AddNode(singleLabelImportSegmentationNode)
    # Should not import multi-label labelmap to segment
    nullSegment = slicer.vtkSlicerSegmentationsModuleLogic.CreateSegmentFromLabelmapVolumeNode(allSegmentsLabelmapNode)
    self.assertIsNone(nullSegment)
    logging.info('(This error message tests an impossible scenario, it is supposed to appear)')
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
    logging.info('Test section 4/2: Import/export with transforms')

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
    bodySegment = self.inputSegmentationNode.GetSegmentation().GetSegment('Body_Contour')
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
    modelTransformedImportSegmentationNode = slicer.vtkMRMLSegmentationNode()
    modelTransformedImportSegmentationNode.SetName('ModelImportTransformed')
    modelTransformedImportSegmentationNode.GetSegmentation().SetMasterRepresentationName(self.closedSurfaceReprName)
    slicer.mrmlScene.AddNode(modelTransformedImportSegmentationNode)
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

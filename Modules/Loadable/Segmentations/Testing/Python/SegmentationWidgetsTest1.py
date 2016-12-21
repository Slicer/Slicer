import os
import unittest
import vtk, qt, ctk, slicer
import logging

import vtkSegmentationCorePython as vtkSegmentationCore

class SegmentationWidgetsTest1(unittest.TestCase):
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
    self.TestSection_1_qMRMLSegmentsTableView()

    logging.info('Test finished')

  #------------------------------------------------------------------------------
  def TestSection_00_SetupPathsAndNames(self):

    self.inputSegmentationNode = None

  #------------------------------------------------------------------------------
  def TestSection_01_GenerateInputData(self):

    self.inputSegmentationNode = slicer.vtkMRMLSegmentationNode()
    slicer.mrmlScene.AddNode(self.inputSegmentationNode)

    # Create new segments
    import random
    for segmentName in ['first', 'second', 'third']:

      sphereSegment = vtkSegmentationCore.vtkSegment()
      sphereSegment.SetName(segmentName)
      sphereSegment.SetColor(random.uniform(0.0,1.0), random.uniform(0.0,1.0), random.uniform(0.0,1.0))

      sphere = vtk.vtkSphereSource()
      sphere.SetCenter(random.uniform(0,100),random.uniform(0,100),random.uniform(0,100))
      sphere.SetRadius(random.uniform(20,30))
      sphere.Update()
      spherePolyData = sphere.GetOutput()
      sphereSegment.AddRepresentation(
        vtkSegmentationCore.vtkSegmentationConverter.GetSegmentationClosedSurfaceRepresentationName(),
        spherePolyData)

      self.inputSegmentationNode.GetSegmentation().AddSegment(sphereSegment)

    self.assertEqual(self.inputSegmentationNode.GetSegmentation().GetNumberOfSegments(), 3)

  #------------------------------------------------------------------------------
  def TestSection_1_qMRMLSegmentsTableView(self):
    logging.info('Test section 1: qMRMLSegmentsTableView')

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

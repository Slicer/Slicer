/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLChangeTrackerNode.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

int vtkMRMLChangeTrackerNodeTest(int , char * [] )
{
  vtkSmartPointer< vtkMRMLChangeTrackerNode > node1 = vtkSmartPointer< vtkMRMLChangeTrackerNode >::New();

  std::cout << "ExerciseBasicMethods" << std::endl;
  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  std::cout << "ExerciseBasicMRMLMethods" << std::endl;
  EXERCISE_BASIC_MRML_METHODS(vtkMRMLChangeTrackerNode, node1);

  TEST_SET_GET_STRING(node1, Scan1_Ref);
  TEST_SET_GET_STRING(node1, WorkingDir);

//  TEST_SET_GET_INT_RANGE(node1, ROIMin, 0, 1);
//  TEST_SET_GET_INT_RANGE(node1, ROIMax, 0, 1);
  node1->SetROIMin(0, 100);
  if (node1->GetROIMin(0) != 100)
    {
    std::cerr << "Error getting ROI min at 0, expected 100, got " << node1->GetROIMax(0) << std::endl;
    return EXIT_FAILURE;
    }
  node1->SetROIMax(0, 99);
  if (node1->GetROIMax(0) != 99)
    {
    std::cerr << "Error getting ROI max at 0, expected 99, got " << node1->GetROIMax(0) << std::endl;
    return EXIT_FAILURE;
    }
  TEST_SET_GET_DOUBLE_RANGE(node1, SuperSampled_Spacing, 0.0, 10.0);
  TEST_SET_GET_DOUBLE_RANGE(node1, SuperSampled_VoxelVolume, 0.0, 10.0);
  TEST_SET_GET_DOUBLE_RANGE(node1, SuperSampled_RatioNewOldSpacing, 0.0, 10.0);
  TEST_SET_GET_DOUBLE_RANGE(node1, Scan1_VoxelVolume, 0.0, 10.0);

  TEST_SET_GET_STRING(node1, Scan1_SuperSampleRef);

  TEST_SET_GET_DOUBLE_RANGE(node1, SegmentThresholdMin, 0.0, 10.0);
  TEST_SET_GET_DOUBLE_RANGE(node1, SegmentThresholdMax, 0.0, 10.0);
  TEST_SET_GET_STRING(node1, Scan1_SegmentRef);

  TEST_SET_GET_STRING(node1, Scan2_Ref);
  TEST_SET_GET_STRING(node1, Scan2_GlobalRef);
  TEST_SET_GET_STRING(node1, Scan2_LocalRef);
  TEST_SET_GET_STRING(node1, Scan2_SuperSampleRef);
  TEST_SET_GET_STRING(node1, Scan2_NormedRef);
  TEST_SET_GET_STRING(node1, Scan1_ThreshRef);
  TEST_SET_GET_STRING(node1, Scan2_ThreshRef);
  TEST_SET_GET_STRING(node1, Grid_Ref);

  TEST_SET_GET_INT_RANGE(node1, Analysis_Intensity_Flag, 0, 1);
  TEST_SET_GET_DOUBLE_RANGE(node1, Analysis_Intensity_Sensitivity, 0.0, 10.0);
  TEST_SET_GET_STRING(node1, Analysis_Intensity_Ref);
  TEST_SET_GET_INT_RANGE(node1, Analysis_Deformable_Flag, 0, 1);
  TEST_SET_GET_DOUBLE_RANGE(node1, Analysis_Deformable_JacobianGrowth, 0.0, 10.0);
  TEST_SET_GET_DOUBLE_RANGE(node1, Analysis_Deformable_SegmentationGrowth, 0.0, 10.0);
  TEST_SET_GET_STRING(node1, Analysis_Deformable_Ref);
  TEST_SET_GET_BOOLEAN(node1, UseITK);
  TEST_SET_GET_STRING(node1, Scan2_RegisteredRef);
  TEST_SET_GET_INT_RANGE(node1, RegistrationChoice, 1, 3);
  TEST_SET_GET_BOOLEAN(node1, ROIRegistration);
  TEST_SET_GET_STRING(node1, Scan2_TransformRef);
  TEST_SET_GET_INT_RANGE(node1, ResampleChoice, 1, 3);
  TEST_SET_GET_DOUBLE_RANGE(node1, ResampleConst, 0.0, 10.0);
  TEST_SET_GET_STRING(node1, Scan1_InputSegmentRef);
  TEST_SET_GET_STRING(node1, Scan1_InputSegmentSuperSampleRef);
  TEST_SET_GET_STRING(node1, Scan2_InputSegmentRef);
  TEST_SET_GET_STRING(node1, Scan2_InputSegmentSuperSampleRef);
  TEST_SET_GET_STRING(node1, ROI_Ref);
  
  return EXIT_SUCCESS;
}

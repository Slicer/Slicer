/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRMLLogic includes
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceLayerLogic.h"

// MRML includes
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>

// VTK includes
#include <vtkImageBlend.h>
#include <vtkNew.h>

#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLSliceLogicTest1(int , char * [] )
{
  vtkNew<vtkMRMLSliceLogic> node1;
  EXERCISE_BASIC_OBJECT_METHODS( node1.GetPointer() );

  vtkNew<vtkMRMLScene> scene;
  node1->SetName("Green");
  node1->SetMRMLScene(scene.GetPointer());

  vtkNew<vtkMRMLSliceNode> SliceNode;
  TEST_SET_GET_VALUE(node1, SliceNode, SliceNode.GetPointer());

  vtkNew<vtkMRMLSliceLayerLogic> LabelLayer;
  TEST_SET_GET_VALUE(node1, LabelLayer, LabelLayer.GetPointer());

  vtkNew<vtkMRMLSliceCompositeNode> SliceCompositeNode;
  TEST_SET_GET_VALUE(node1, SliceCompositeNode, SliceCompositeNode.GetPointer());

  vtkNew<vtkMRMLSliceLayerLogic> ForegroundLayer;
  TEST_SET_GET_VALUE(node1, ForegroundLayer, ForegroundLayer.GetPointer());

  vtkNew<vtkMRMLSliceLayerLogic> BackgroundLayer;
  TEST_SET_GET_VALUE(node1, BackgroundLayer, BackgroundLayer.GetPointer());

  // TODO: need to fix the test.
  // The problem here is that the current node of the logic is wrong
  // it hasn't been added to the mrml scene. So when modified,
  // the logic realizes it and create a new node (loosing the props).
  //TEST_SET_GET_VALUE(node1, SliceOffset, 1);

  node1->DeleteSliceModel();
  node1->CreateSliceModel();
  TEST_GET_OBJECT(node1, SliceModelNode);
  TEST_GET_OBJECT(node1, SliceModelDisplayNode);
  TEST_GET_OBJECT(node1, SliceModelTransformNode);
  TEST_GET_OBJECT(node1, Blend);

  node1->Print(std::cout);
  return EXIT_SUCCESS;
}


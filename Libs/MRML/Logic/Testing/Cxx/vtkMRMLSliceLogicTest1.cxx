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
  vtkNew<vtkMRMLSliceLogic> logic;
  EXERCISE_BASIC_OBJECT_METHODS(logic.GetPointer());

  vtkNew<vtkMRMLScene> scene;

  // Add default slice orientation presets
  vtkMRMLSliceNode::AddDefaultSliceOrientationPresets(scene.GetPointer());

  logic->SetName("Green");
  logic->SetMRMLScene(scene.GetPointer());

  vtkNew<vtkMRMLSliceNode> SliceNode;
  TEST_SET_GET_VALUE(logic, SliceNode, SliceNode.GetPointer());

  vtkNew<vtkMRMLSliceLayerLogic> LabelLayer;
  TEST_SET_GET_VALUE(logic, LabelLayer, LabelLayer.GetPointer());

  vtkNew<vtkMRMLSliceCompositeNode> SliceCompositeNode;
  TEST_SET_GET_VALUE(logic, SliceCompositeNode, SliceCompositeNode.GetPointer());

  vtkNew<vtkMRMLSliceLayerLogic> ForegroundLayer;
  TEST_SET_GET_VALUE(logic, ForegroundLayer, ForegroundLayer.GetPointer());

  vtkNew<vtkMRMLSliceLayerLogic> BackgroundLayer;
  TEST_SET_GET_VALUE(logic, BackgroundLayer, BackgroundLayer.GetPointer());

  // TODO: need to fix the test.
  // The problem here is that the current node of the logic is wrong
  // it hasn't been added to the mrml scene. So when modified,
  // the logic realizes it and create a new node (loosing the props).
  //TEST_SET_GET_VALUE(logic, SliceOffset, 1);

  logic->DeleteSliceModel();
  logic->CreateSliceModel();
  TEST_GET_OBJECT(logic, SliceModelNode);
  TEST_GET_OBJECT(logic, SliceModelDisplayNode);
  TEST_GET_OBJECT(logic, SliceModelTransformNode);
  TEST_GET_OBJECT(logic, Blend);

  logic->Print(std::cout);
  return EXIT_SUCCESS;
}


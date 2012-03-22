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
#include <vtkMRMLSliceCompositeNode.h>

// VTK includes
#include <vtkImageBlend.h>


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLSliceLogicTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLSliceLogic > node1 = vtkSmartPointer< vtkMRMLSliceLogic >::New();
  EXERCISE_BASIC_OBJECT_METHODS( node1 );

#define TEST_SET_GET_OBJECT(object,prefix,variable) \
  vtkSmartPointer<prefix> __##variable = vtkSmartPointer<prefix>::New();\
  object->Set##variable(__##variable); \
  object->Get##variable()->Print(std::cout)

#define TEST_GET_OBJECT(object,variable) \
  object->Get##variable()->Print(std::cout)

#define TEST_SET_GET_VALUE(object,variable,value) \
  object->Set##variable(value); \
  if (object->Get##variable() != value) \
    {   \
    std::cerr << "Error getting " << #variable << std::endl; \
    std::cerr << "Expected " << value << std::endl; \
    std::cerr << "but got  " << object->Get##variable()<< std::endl; \
    return EXIT_FAILURE; \
    }
    
  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  node1->SetName("Green");
  node1->SetMRMLScene(scene);
  TEST_SET_GET_OBJECT(node1, vtkMRMLSliceNode, SliceNode);
  TEST_SET_GET_OBJECT(node1, vtkMRMLSliceLayerLogic, LabelLayer);
  TEST_SET_GET_OBJECT(node1, vtkMRMLSliceCompositeNode, SliceCompositeNode);
  TEST_SET_GET_OBJECT(node1, vtkMRMLSliceLayerLogic, ForegroundLayer);
  TEST_SET_GET_OBJECT(node1, vtkMRMLSliceLayerLogic, BackgroundLayer);
  TEST_SET_GET_VALUE(node1, ForegroundOpacity, .5);
  TEST_SET_GET_VALUE(node1, LabelOpacity, .5);
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


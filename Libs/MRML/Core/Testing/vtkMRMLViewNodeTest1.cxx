/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLViewNode.h"

int vtkMRMLViewNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLViewNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  // Test Set/GetInteractionNode without scene
  {
    vtkNew<vtkMRMLViewNode> viewNode;
    vtkNew<vtkMRMLInteractionNode> interactionNode;
    CHECK_NULL(viewNode->GetInteractionNode());
    viewNode->SetInteractionNode(interactionNode.GetPointer());
    CHECK_NULL(viewNode->GetInteractionNode());
  }

  // Test Set/GetInteractionNode with scene
  {
    vtkNew<vtkMRMLScene> scene;
    vtkNew<vtkMRMLViewNode> viewNode;
    vtkNew<vtkMRMLInteractionNode> interactionNode; // interaction node is a singleton by default
    scene->AddNode(viewNode.GetPointer());
    scene->AddNode(interactionNode.GetPointer());
    CHECK_POINTER(viewNode->GetInteractionNode(), interactionNode.GetPointer());
    CHECK_POINTER(viewNode->GetInteractionNode(), scene->GetNodeByID("vtkMRMLInteractionNodeSingleton"));

    vtkNew<vtkMRMLInteractionNode> otherInteractionNode;
    otherInteractionNode->SetSingletonOff();
    scene->AddNode(otherInteractionNode.GetPointer());
    CHECK_POINTER(viewNode->GetInteractionNode(), scene->GetNodeByID("vtkMRMLInteractionNodeSingleton"));
    viewNode->SetInteractionNode(otherInteractionNode.GetPointer());
    CHECK_POINTER(viewNode->GetInteractionNode(), otherInteractionNode.GetPointer());
    CHECK_POINTER_DIFFERENT(otherInteractionNode.GetPointer(), scene->GetNodeByID("vtkMRMLInteractionNodeSingleton"));
  }

  return EXIT_SUCCESS;
}

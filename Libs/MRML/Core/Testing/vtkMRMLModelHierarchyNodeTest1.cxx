/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>

int vtkMRMLModelHierarchyNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLModelHierarchyNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  TEST_SET_GET_STRING(node1.GetPointer(), ModelNodeID);
  //TEST_SET_GET_STRING(node1, DisplayNodeID);

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLModelDisplayNode> dnode;
  scene->AddNode(node1.GetPointer());
  scene->AddNode(dnode.GetPointer());
  node1->SetAndObserveDisplayNodeID(dnode->GetID());
  TEST_SET_GET_BOOLEAN(node1.GetPointer(), Expanded);

  vtkNew<vtkMRMLModelNode> mnode;
  scene->AddNode(mnode.GetPointer());
  node1->SetModelNodeID(mnode->GetID());

  vtkMRMLModelNode * mnode2 = node1->GetModelNode();
  if (mnode2 != mnode.GetPointer())
    {
    std::cerr << "ERROR setting/getting model node" << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkCollection> col;
  node1->GetChildrenModelNodes(col.GetPointer());
  int numChildren =  col->GetNumberOfItems();
  if (numChildren != 1)
    {
    std::cerr << "Expected 1 child, got " << numChildren << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

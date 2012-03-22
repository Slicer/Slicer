/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

#include "vtkMRMLCoreTestingMacros.h"

// VTK includes
#include <vtkCollection.h>

int vtkMRMLModelHierarchyNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLModelHierarchyNode > node1 = vtkSmartPointer< vtkMRMLModelHierarchyNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLModelHierarchyNode, node1);

  TEST_SET_GET_STRING(node1, ModelNodeID);
  //TEST_SET_GET_STRING(node1, DisplayNodeID);
  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtkSmartPointer<vtkMRMLModelDisplayNode> dnode = vtkSmartPointer<vtkMRMLModelDisplayNode>::New();
  scene->AddNode(node1);
  scene->AddNode(dnode);
  node1->SetAndObserveDisplayNodeID(dnode->GetID());
  TEST_SET_GET_BOOLEAN(node1, Expanded);
  
  vtkSmartPointer<vtkMRMLModelNode> mnode = vtkSmartPointer<vtkMRMLModelNode>::New();
  scene->AddNode(mnode);
  node1->SetModelNodeID(mnode->GetID());
  vtkSmartPointer<vtkMRMLModelNode> mnode2 = vtkSmartPointer<vtkMRMLModelNode>::New();
  mnode2 = node1->GetModelNode();
  if (mnode2 != mnode)
    {
    std::cerr << "ERROR setting/getting model node" << std::endl;
    return EXIT_FAILURE;
    }

  vtkSmartPointer<vtkMRMLModelHierarchyNode> hnode1 = node1->GetUnExpandedParentNode();
  std::cout << "Unexpanded parent node = " << (hnode1 == NULL ? "NULL" : hnode1->GetID()) << std::endl;
  vtkSmartPointer<vtkCollection> col =  vtkSmartPointer<vtkCollection>::New();
  node1->GetChildrenModelNodes(col);
  int numChildren =  col->GetNumberOfItems();
  std::cout << "Number of children model nodes = " << numChildren << std::endl;
  if (numChildren != 1)
    {
    std::cerr << "Expected 1 child, got " << numChildren << std::endl;
    return EXIT_FAILURE;
    }
  
  return EXIT_SUCCESS;
}

/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLDisplayableHierarchyNode.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

#include "vtkCollection.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLScene.h"

int vtkMRMLDisplayableHierarchyNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLDisplayableHierarchyNode > node1 = vtkSmartPointer< vtkMRMLDisplayableHierarchyNode >::New();
  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLDisplayableHierarchyNode, node1);

  TEST_SET_GET_STRING(node1, DisplayableNodeID);

  node1->SetDisplayableNodeIDReference("testingDisplayableNodeID");

  node1->SetAndObserveDisplayNodeID("testingDisplayNodeID");
  std::cout << "DisplayNodeID = " << ( node1->GetDisplayNodeID() == NULL ? "NULL" : node1->GetDisplayNodeID()) << std::endl;
  
  vtkSmartPointer<vtkMRMLDisplayableNode> pnode = node1->GetDisplayableNode();
  std::cout << "GetDisplayableNode returned " << (pnode == NULL ? "null" : "not null") << std::endl;

  vtkSmartPointer<vtkMRMLDisplayNode> dnode1 = node1->GetDisplayNode();
  std::cout << "GetDisplayNode returned " << (dnode1 == NULL ? "null" : "not null") << std::endl;

  TEST_SET_GET_BOOLEAN(node1, Expanded);

  vtkSmartPointer<vtkMRMLDisplayableHierarchyNode> pnode1 = node1->GetUnExpandedParentNode();
  std::cout << "GetUnexpandedParentNode = " << (pnode1 == NULL ? "NULL" : "not null") << std::endl;

  vtkSmartPointer<vtkMRMLDisplayableHierarchyNode> pnode2 = node1->GetTopParentNode();
  std::cout << "GetTopParentNode  = " << (pnode2 == NULL ? "NULL" : "not null") << std::endl;

  vtkSmartPointer<vtkCollection> col =  vtkSmartPointer<vtkCollection>::New();
  node1->GetChildrenDisplayableNodes(col);
  std::cout << "Number of children displayble nodes = " << col->GetNumberOfItems() << std::endl;

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtkSmartPointer<vtkMRMLDisplayNode> dnode2 = vtkSmartPointer<vtkMRMLDisplayNode>::New();
  std::cout << "Adding node 1\n";
  scene->AddNode(node1);
  std::cout << "Adding display node 2\n";
  scene->AddNode(dnode2);
  if (dnode2 && dnode2->GetID())
    {
    node1->SetAndObserveDisplayNodeID(dnode2->GetID());
    }
  else
    {
    std::cout << "Dispaly node2 is null or its id is null, not observing it\n";
    }
  
  vtkSmartPointer<vtkMRMLDisplayableHierarchyNode> hnode3 = node1->GetDisplayableHierarchyNode(scene, "myid");
   std::cout << "Displayable hierarchy node = " << (hnode3 == NULL ? "NULL" : hnode3->GetID()) << std::endl;

  return EXIT_SUCCESS;
}

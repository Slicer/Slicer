/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLHierarchyNode.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"


class vtkMRMLHierarchyNodeTestHelper1 : public vtkMRMLHierarchyNode
{
public:
  // Provide a concrete New.
  static vtkMRMLHierarchyNodeTestHelper1 *New(){return new vtkMRMLHierarchyNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLHierarchyNodeTestHelper1,vtkMRMLHierarchyNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLHierarchyNodeTestHelper1;
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLHierarchyNodeTestHelper1";
    }

  virtual bool CanApplyNonLinearTransforms() { return false; }
};

int vtkMRMLHierarchyNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLHierarchyNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLHierarchyNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLHierarchyNodeTestHelper1, node1);

  TEST_SET_GET_STRING(node1, ParentNodeID);

  vtkSmartPointer<vtkMRMLHierarchyNode> pnode = node1->GetParentNode();
  std::cout << "GetParentNode returned " << (pnode == NULL ? "null" : "not null") << std::endl;

  node1->SetParentNodeIDReference("testingID");
  pnode = node1->GetParentNode();
  std::cout << "GetParentNode returned " << (pnode == NULL ? "null" : "not null") << std::endl;
  
  return EXIT_SUCCESS;
}

/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLHierarchyNode.h"


#include "vtkMRMLCoreTestingMacros.h"


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

//  vtkSmartPointer< vtkMRMLHierarchyNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLHierarchyNodeTestHelper1 >::New();

  vtkSmartPointer< vtkMRMLHierarchyNode > node1 = vtkSmartPointer< vtkMRMLHierarchyNode >::New();
  std::cout << "node1 is " << (node1 == NULL ? "null" : "not null") << std::endl;
  if (node1 == NULL)
    {
    return EXIT_FAILURE;
    }
  
  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLHierarchyNode, node1);

  TEST_SET_GET_STRING(node1, ParentNodeID);

  vtkSmartPointer<vtkMRMLHierarchyNode> pnode = node1->GetParentNode();
  std::cout << "GetParentNode returned " << (pnode == NULL ? "null" : "not null") << std::endl;

  node1->SetParentNodeID("testingID");
  pnode = node1->GetParentNode();
  std::cout << "GetParentNode returned " << (pnode == NULL ? "null" : "not null") << std::endl;

  TEST_SET_GET_DOUBLE_RANGE(node1, SortingValue, 0.0, 10.0);

  TEST_SET_GET_BOOLEAN(node1, AllowMultipleChildren);

  return EXIT_SUCCESS;
}

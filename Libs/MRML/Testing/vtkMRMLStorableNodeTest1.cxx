/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLStorableNode.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

class vtkMRMLStorableNodeTestHelper1 : public vtkMRMLStorableNode
{
public:
  // Provide a concrete New.
  static vtkMRMLStorableNodeTestHelper1 *New(){return new vtkMRMLStorableNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLStorableNodeTestHelper1,vtkMRMLStorableNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLStorableNodeTestHelper1;
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLStorableNodeTestHelper1";
    }

  virtual bool CanApplyNonLinearTransforms() { return false; }
  virtual void ApplyTransform(vtkAbstractTransform* vtkNotUsed(transform)) { return; }
  using vtkMRMLTransformableNode::ApplyTransform;
};
 
int vtkMRMLStorableNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLStorableNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLStorableNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  vtkMRMLNode * newNode = node1->CreateNodeInstance();

  if( newNode == NULL )
    {
    std::cerr << "Error in CreateNodeInstance()" << std::endl;
    return EXIT_FAILURE;
    }

  newNode->Delete();

  return EXIT_SUCCESS;
}

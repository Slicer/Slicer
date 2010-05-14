/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLColorTableNode.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

class vtkMRMLColorTableNodeTestHelper1 : public vtkMRMLColorTableNode
{
public:
  // Provide a concrete New.
  static vtkMRMLColorTableNodeTestHelper1 *New(){return new vtkMRMLColorTableNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLColorTableNodeTestHelper1,vtkMRMLColorTableNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLColorTableNodeTestHelper1;
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLColorTableNodeTestHelper1";
    }

  virtual bool CanApplyNonLinearTransforms() { return false; }
  virtual void ApplyTransform(vtkAbstractTransform* vtkNotUsed(transform)) { return; }
  using vtkMRMLColorNode::ApplyTransform; 

  virtual int ReadData(vtkMRMLNode *vtkNotUsed(refNode)) { return 0; }
  virtual int WriteData(vtkMRMLNode *vtkNotUsed(refNode)) { return 0; }
};
 
int vtkMRMLColorTableNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLColorTableNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLColorTableNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS(vtkMRMLColorTableNodeTestHelper1, node1);
  
  return EXIT_SUCCESS;
}

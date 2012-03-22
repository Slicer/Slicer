/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLTransformableNode.h"


#include "vtkMRMLCoreTestingMacros.h"

class vtkMRMLTransformableNodeTestHelper1 : public vtkMRMLTransformableNode
{
public:
  // Provide a concrete New.
  static vtkMRMLTransformableNodeTestHelper1 *New(){return new vtkMRMLTransformableNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLTransformableNodeTestHelper1,vtkMRMLTransformableNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLTransformableNodeTestHelper1;
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLTransformableNodeTestHelper1";
    }
};
 
int vtkMRMLTransformableNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLTransformableNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLTransformableNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS(vtkMRMLTransformableNodeTestHelper1, node1);
  
  return EXIT_SUCCESS;
}

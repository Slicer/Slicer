/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/



#include "vtkMRMLCoreTestingMacros.h"

class vtkMRMLNodeTestHelper1 : public vtkMRMLNode
{
public:
  // Provide a concrete New.
  static vtkMRMLNodeTestHelper1 *New(){return new vtkMRMLNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLNodeTestHelper1,vtkMRMLNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLNodeTestHelper1;
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLNodeTestHelper1";
    }
};
 
int vtkMRMLNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLNodeTestHelper1, node1);

  return EXIT_SUCCESS;
}

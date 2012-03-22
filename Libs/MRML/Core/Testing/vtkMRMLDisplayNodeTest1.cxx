/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLDisplayNode.h"

#include "vtkMRMLCoreTestingMacros.h"

//----------------------------------------------------------------------------
class vtkMRMLDisplayNodeTestHelper1 : public vtkMRMLDisplayNode
{
public:
  // Provide a concrete New.
  static vtkMRMLDisplayNodeTestHelper1 *New(){return new vtkMRMLDisplayNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLDisplayNodeTestHelper1,vtkMRMLDisplayNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLDisplayNodeTestHelper1;
    }

  const char * GetTypeAsString()
    {
    return "vtkMRMLDisplayNodeTestHelper1";
    }

  int ReadFile()
    {
    std::cout << "vtkMRMLDisplayNodeTestHelper1 pretending to read a file " << std::endl;
    return EXIT_SUCCESS;
    }

  virtual const char* GetNodeTagName() 
    {
    return "Testing is good";
    }
};

//----------------------------------------------------------------------------
int vtkMRMLDisplayNodeTest1(int , char * [])
{
  vtkSmartPointer< vtkMRMLDisplayNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLDisplayNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  // EXERCISE_BASIC_MRML_METHODS(vtkMRMLDisplayNodeTestHelper1, node1);

  EXERCISE_BASIC_DISPLAY_MRML_METHODS(vtkMRMLDisplayNodeTestHelper1, node1);

  return EXIT_SUCCESS;
}

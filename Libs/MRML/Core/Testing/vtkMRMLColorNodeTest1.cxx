/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLColorNode.h"


#include "vtkMRMLCoreTestingMacros.h"

class vtkMRMLColorNodeTestHelper1 : public vtkMRMLColorNode
{
public:
  // Provide a concrete New.
  static vtkMRMLColorNodeTestHelper1 *New(){return new vtkMRMLColorNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLColorNodeTestHelper1,vtkMRMLColorNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLColorNodeTestHelper1;
    }

  const char * GetTypeAsString()
    {
    return "vtkMRMLColorNodeTestHelper1";
    }

  int ReadFile()
    {
    std::cout << "vtkMRMLColorNodeTestHelper1 pretending to read a file " << std::endl;
    return EXIT_SUCCESS;
    }
};
 
int vtkMRMLColorNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLColorNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLColorNodeTestHelper1 >::New();

  node1->DebugOn();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS( vtkMRMLColorNodeTestHelper1, node1);
  
  return EXIT_SUCCESS;
}

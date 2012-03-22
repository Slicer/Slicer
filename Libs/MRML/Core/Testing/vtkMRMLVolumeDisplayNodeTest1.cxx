/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLVolumeDisplayNode.h"


#include "vtkMRMLCoreTestingMacros.h"

class vtkMRMLVolumeDisplayNodeTestHelper1 : public vtkMRMLVolumeDisplayNode
{
public:
  // Provide a concrete New.
  static vtkMRMLVolumeDisplayNodeTestHelper1 *New(){return new vtkMRMLVolumeDisplayNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLVolumeDisplayNodeTestHelper1,vtkMRMLVolumeDisplayNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLVolumeDisplayNodeTestHelper1;
    }

  const char * GetTypeAsString()
    {
    return "vtkMRMLVolumeDisplayNodeTestHelper1";
    }

  int ReadFile()
    {
    std::cout << "vtkMRMLVolumeDisplayNodeTestHelper1 pretending to read a file " << std::endl;
    return EXIT_SUCCESS;
    }

  virtual const char* GetNodeTagName() 
    {
    return "Testing is good";
    }
};
 
int vtkMRMLVolumeDisplayNodeTest1(int , char * [] )
{

  vtkSmartPointer< vtkMRMLVolumeDisplayNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLVolumeDisplayNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_DISPLAY_MRML_METHODS( vtkMRMLVolumeDisplayNodeTestHelper1, node1);
  
  return EXIT_SUCCESS;
}

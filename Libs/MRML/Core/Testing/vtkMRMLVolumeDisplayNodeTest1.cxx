/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLVolumeDisplayNode.h"

// VTK includes
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
class vtkMRMLVolumeDisplayNodeTestHelper1 : public vtkMRMLVolumeDisplayNode
{
public:
  // Provide a concrete New.
  static vtkMRMLVolumeDisplayNodeTestHelper1 *New();

  vtkTypeMacro(vtkMRMLVolumeDisplayNodeTestHelper1,vtkMRMLVolumeDisplayNode);

  vtkMRMLNode* CreateNodeInstance() override
    {
    return vtkMRMLVolumeDisplayNodeTestHelper1::New();
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

  const char* GetNodeTagName() override
    {
    return "Testing is good";
    }
};
vtkStandardNewMacro(vtkMRMLVolumeDisplayNodeTestHelper1);

//---------------------------------------------------------------------------
int vtkMRMLVolumeDisplayNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLVolumeDisplayNodeTestHelper1> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  return EXIT_SUCCESS;
}

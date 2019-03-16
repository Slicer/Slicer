/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"

// VTK includes
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
class vtkMRMLDiffusionTensorDisplayPropertiesNodeTestHelper1 : public vtkMRMLDiffusionTensorDisplayPropertiesNode
{
public:
  // Provide a concrete New.
  static vtkMRMLDiffusionTensorDisplayPropertiesNodeTestHelper1 *New();

  vtkTypeMacro(vtkMRMLDiffusionTensorDisplayPropertiesNodeTestHelper1,vtkMRMLDiffusionTensorDisplayPropertiesNode);

  vtkMRMLNode* CreateNodeInstance() override
    {
    return vtkMRMLDiffusionTensorDisplayPropertiesNodeTestHelper1::New();
    }
  const char* GetNodeTagName() override
    {
    return "vtkMRMLDiffusionTensorDisplayPropertiesNodeTestHelper1";
    }

  virtual int ReadData(vtkMRMLNode *vtkNotUsed(refNode)) { return 0; }
  virtual int WriteData(vtkMRMLNode *vtkNotUsed(refNode)) { return 0; }
};
vtkStandardNewMacro(vtkMRMLDiffusionTensorDisplayPropertiesNodeTestHelper1);

//---------------------------------------------------------------------------
int vtkMRMLDiffusionTensorDisplayPropertiesNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLDiffusionTensorDisplayPropertiesNodeTestHelper1> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  return EXIT_SUCCESS;
}

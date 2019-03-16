/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLStorableNode.h"

// VTK includes
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
class vtkMRMLStorableNodeTestHelper1 : public vtkMRMLStorableNode
{
public:
  // Provide a concrete New.
  static vtkMRMLStorableNodeTestHelper1 *New();

  vtkTypeMacro(vtkMRMLStorableNodeTestHelper1,vtkMRMLStorableNode);

  vtkMRMLNode* CreateNodeInstance() override
    {
    return vtkMRMLStorableNodeTestHelper1::New();
    }
  const char* GetNodeTagName() override
    {
    return "vtkMRMLStorableNodeTestHelper1";
    }

  // for testing purposes, return a valid storage node,
  // vtkMRMLStorageNode::New returns nullptr
  vtkMRMLStorageNode* CreateDefaultStorageNode() override { return vtkMRMLModelStorageNode::New(); }
};
vtkStandardNewMacro(vtkMRMLStorableNodeTestHelper1);

//---------------------------------------------------------------------------
int vtkMRMLStorableNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLStorableNodeTestHelper1> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  return EXIT_SUCCESS;
}

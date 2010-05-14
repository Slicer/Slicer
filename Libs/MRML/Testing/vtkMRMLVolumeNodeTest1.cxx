/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLVolumeNode.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

// return a concrete storage node, vtkMRMLStorageNode::New returns null
#include "vtkMRMLVolumeArchetypeStorageNode.h"

class vtkMRMLVolumeNodeTestHelper1 : public vtkMRMLVolumeNode
{
public:
  // Provide a concrete New.
  static vtkMRMLVolumeNodeTestHelper1 *New(){return new vtkMRMLVolumeNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLVolumeNodeTestHelper1,vtkMRMLVolumeNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLVolumeNodeTestHelper1;
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLVolumeNodeTestHelper1";
    }

  virtual bool CanApplyNonLinearTransforms() { return false; }
  virtual void ApplyTransform(vtkAbstractTransform* vtkNotUsed(transform)) { return; }
  using vtkMRMLVolumeNode::ApplyTransform;

  virtual vtkMRMLStorageNode* CreateDefaultStorageNode() { return vtkMRMLVolumeArchetypeStorageNode::New(); }
};
 
int vtkMRMLVolumeNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLVolumeNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLVolumeNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_DISPLAYABLE_MRML_METHODS(vtkMRMLVolumeNodeTestHelper1, node1);

  return EXIT_SUCCESS;
}

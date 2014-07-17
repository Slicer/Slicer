/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformableNode.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
class vtkMRMLTransformableNodeTestHelper1 : public vtkMRMLTransformableNode
{
public:
  // Provide a concrete New.
  static vtkMRMLTransformableNodeTestHelper1 *New();

  vtkTypeMacro(vtkMRMLTransformableNodeTestHelper1,vtkMRMLTransformableNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return vtkMRMLTransformableNodeTestHelper1::New();
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLTransformableNodeTestHelper1";
    }
};
vtkStandardNewMacro(vtkMRMLTransformableNodeTestHelper1);

//---------------------------------------------------------------------------
bool TestSetAndObserveTransformNodeID();

//---------------------------------------------------------------------------
int vtkMRMLTransformableNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLTransformableNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLTransformableNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS(vtkMRMLTransformableNodeTestHelper1, node1);

  bool res = true;
  res = TestSetAndObserveTransformNodeID() && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//---------------------------------------------------------------------------
bool TestSetAndObserveTransformNodeID()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLTransformableNodeTestHelper1> transformable;
  scene->AddNode(transformable.GetPointer());

  vtkNew<vtkMRMLLinearTransformNode> transform;
  scene->AddNode(transform.GetPointer());
  vtkNew<vtkMatrix4x4> matrix;
  matrix->SetElement(0,3, 1.);
  transform->SetMatrixTransformToParent(matrix.GetPointer());

  transformable->SetAndObserveTransformNodeID(transform->GetID());
  if (transformable->GetParentTransformNode() != transform.GetPointer())
    {
    std::cout << __LINE__ << "SetAndObserveTransformNodeID failed"
              << std::endl;
    return false;
    }
  double point[4] = {0., 0., 0., 1.};
  double res[4] = {-1., -1., -1., -1.};
  transformable->TransformPointToWorld(point, res);
  if (res[0] != 1. || res[1] != 0. || res[2] != 0. || res[3] != 1. )
    {
    std::cout << __LINE__ << "TransformPointToWorld failed"
              << std::endl;
    return false;
    }
  return true;
}

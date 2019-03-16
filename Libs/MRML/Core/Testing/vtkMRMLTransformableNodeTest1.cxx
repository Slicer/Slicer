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

  vtkMRMLNode* CreateNodeInstance() override
    {
    return vtkMRMLTransformableNodeTestHelper1::New();
    }
  const char* GetNodeTagName() override
    {
    return "vtkMRMLTransformableNodeTestHelper1";
    }
};
vtkStandardNewMacro(vtkMRMLTransformableNodeTestHelper1);

//---------------------------------------------------------------------------
int TestSetAndObserveTransformNodeID();

//---------------------------------------------------------------------------
int vtkMRMLTransformableNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLTransformableNodeTestHelper1> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  CHECK_EXIT_SUCCESS(TestSetAndObserveTransformNodeID());
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestSetAndObserveTransformNodeID()
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
  CHECK_POINTER(transformable->GetParentTransformNode(), transform.GetPointer());
  double point[3] = {0., 0., 0.};
  double res[3] = {-1., -1., -1.};
  transformable->TransformPointToWorld(point, res);
  if (res[0] != 1. || res[1] != 0. || res[2] != 0.)
    {
    std::cout << __LINE__ << "TransformPointToWorld failed"
              << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

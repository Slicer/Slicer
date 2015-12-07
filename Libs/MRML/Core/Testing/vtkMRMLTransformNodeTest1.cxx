/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLBSplineTransformNode.h"
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"

#include <vtkGeneralTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkTransform.h>

// TODO: Move this fuzzy matrix comparison with configurable tolerance to vtkAddon
bool Matrix4x4AreEqual(vtkMatrix4x4 *m1, vtkMatrix4x4 *m2)
{
  const double tol = 1e-3;
  for (int i = 0; i < 4; i++)
    {
    for (int j = 0; j < 4; j++)
      {
      if ( fabs(m1->GetElement(i, j) - m2->GetElement(i, j)) > tol )
        {
        return false;
        }
      }
    }
    return true;
}

vtkMatrix4x4* CreateTransformMatrix(double translateX, double translateY, double translateZ, double rotateX, double rotateY, double rotateZ)
{
  vtkNew<vtkTransform> tr;
  tr->Translate(translateX, translateY, translateZ);
  tr->RotateX(rotateX);
  tr->RotateY(rotateY);
  tr->RotateZ(rotateZ);
  vtkMatrix4x4* matrix = vtkMatrix4x4::New();
  tr->GetMatrix(matrix);
  return matrix;
}

int vtkMRMLTransformNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLTransformNode> node1;
  EXERCISE_BASIC_OBJECT_METHODS(node1.GetPointer());
  EXERCISE_BASIC_TRANSFORM_MRML_METHODS(vtkMRMLTransformNode, node1.GetPointer());

  /// Test matrix get/set functions

  // Create transform nodes
  vtkNew<vtkMRMLTransformNode> bTransform; // == b_to_w
  vtkNew<vtkMRMLTransformNode> cTransform; // == c_to_b
  vtkNew<vtkMRMLTransformNode> dTransform; // == d_to_c
  vtkNew<vtkMRMLTransformNode> eTransform; // == e_to_d
  vtkNew<vtkMRMLTransformNode> qTransform; // == q_to_b
  vtkNew<vtkMRMLTransformNode> rTransform; // == r_to_q
  vtkSmartPointer<vtkMatrix4x4> w_from_b_mx = vtkSmartPointer<vtkMatrix4x4>::Take(CreateTransformMatrix( 34,  23, -12,  44,  12,  78));
  vtkSmartPointer<vtkMatrix4x4> b_from_c_mx = vtkSmartPointer<vtkMatrix4x4>::Take(CreateTransformMatrix(-34,  11,  12, -22, 128,  18));
  vtkSmartPointer<vtkMatrix4x4> c_from_d_mx = vtkSmartPointer<vtkMatrix4x4>::Take(CreateTransformMatrix( 14, -23,  44,  11, -71,  38));
  vtkSmartPointer<vtkMatrix4x4> d_from_e_mx = vtkSmartPointer<vtkMatrix4x4>::Take(CreateTransformMatrix( 73,  81,  35,  22,  11, -98));
  vtkSmartPointer<vtkMatrix4x4> b_from_q_mx = vtkSmartPointer<vtkMatrix4x4>::Take(CreateTransformMatrix( 13, -71, 335, -42,  91, -28));
  vtkSmartPointer<vtkMatrix4x4> q_from_r_mx = vtkSmartPointer<vtkMatrix4x4>::Take(CreateTransformMatrix( 53, -11,  65, -12,  21,   8));
  bTransform->SetMatrixTransformToParent(w_from_b_mx.GetPointer());
  cTransform->SetMatrixTransformToParent(b_from_c_mx.GetPointer());
  dTransform->SetMatrixTransformToParent(c_from_d_mx.GetPointer());
  eTransform->SetMatrixTransformToParent(d_from_e_mx.GetPointer());
  qTransform->SetMatrixTransformToParent(b_from_q_mx.GetPointer());
  rTransform->SetMatrixTransformToParent(q_from_r_mx.GetPointer());

  // Create transfor hierarchy in the scene
  //
  // WORLD -> w coordinate system
  //  |-- bTransform
  //         |-- cTransform
  //         |      |-- dTransform
  //         |             |-- eTransform
  //         |-- qTransform
  //                |-- rTransform
  //
  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(bTransform.GetPointer());
  scene->AddNode(cTransform.GetPointer());
  scene->AddNode(dTransform.GetPointer());
  scene->AddNode(eTransform.GetPointer());
  scene->AddNode(qTransform.GetPointer());
  scene->AddNode(rTransform.GetPointer());
  cTransform->SetAndObserveTransformNodeID(bTransform->GetID());
  dTransform->SetAndObserveTransformNodeID(cTransform->GetID());
  eTransform->SetAndObserveTransformNodeID(dTransform->GetID());
  qTransform->SetAndObserveTransformNodeID(bTransform->GetID());
  rTransform->SetAndObserveTransformNodeID(qTransform->GetID());

  // Pre-compute transform matrices that will be used for testing
  vtkNew<vtkMatrix4x4> c_from_e_mx;
  vtkMatrix4x4::Multiply4x4(c_from_d_mx.GetPointer(), d_from_e_mx.GetPointer(), c_from_e_mx.GetPointer());
  vtkNew<vtkMatrix4x4> b_from_e_mx;
  vtkMatrix4x4::Multiply4x4(b_from_c_mx.GetPointer(), c_from_e_mx.GetPointer(), b_from_e_mx.GetPointer());
  vtkNew<vtkMatrix4x4> w_from_e_mx;
  vtkMatrix4x4::Multiply4x4(w_from_b_mx.GetPointer(), b_from_e_mx.GetPointer(), w_from_e_mx.GetPointer());
  vtkNew<vtkMatrix4x4> e_from_c_mx;
  vtkMatrix4x4::Invert(c_from_e_mx.GetPointer(), e_from_c_mx.GetPointer());
  vtkNew<vtkMatrix4x4> c_from_b_mx;
  vtkNew<vtkMatrix4x4> b_from_r_mx;
  vtkMatrix4x4::Multiply4x4(b_from_q_mx.GetPointer(), q_from_r_mx.GetPointer(), b_from_r_mx.GetPointer());
  vtkMatrix4x4::Invert(b_from_c_mx.GetPointer(), c_from_b_mx.GetPointer());
  vtkNew<vtkMatrix4x4> c_from_r_mx;
  vtkMatrix4x4::Multiply4x4(c_from_b_mx.GetPointer(), b_from_r_mx.GetPointer(), c_from_r_mx.GetPointer());

  // Test GetMatrixTransformToNode computations
  vtkNew<vtkMatrix4x4> test_mx;

  // GetMatrixTransformToNode: target node is parent
  eTransform->GetMatrixTransformToNode(cTransform.GetPointer(), test_mx.GetPointer());
  if (!Matrix4x4AreEqual(c_from_e_mx.GetPointer(), test_mx.GetPointer()))
    {
    std::cerr << __LINE__ << " vtkMRMLTransformNodeTest1 failed" << std::endl;
    return EXIT_FAILURE;
    }
  // GetMatrixTransformToNode: target node is child
  cTransform->GetMatrixTransformToNode(eTransform.GetPointer(), test_mx.GetPointer());
  if (!Matrix4x4AreEqual(e_from_c_mx.GetPointer(), test_mx.GetPointer()))
    {
    std::cerr << __LINE__ << " vtkMRMLTransformNodeTest1 failed" << std::endl;
    return EXIT_FAILURE;
    }
  // GetMatrixTransformToNode: target node is world
  eTransform->GetMatrixTransformToNode(NULL, test_mx.GetPointer());
  if (!Matrix4x4AreEqual(w_from_e_mx.GetPointer(), test_mx.GetPointer()))
    {
    std::cerr << __LINE__ << " vtkMRMLTransformNodeTest1 failed" << std::endl;
    return EXIT_FAILURE;
    }
  // GetMatrixTransformToWorld
  eTransform->GetMatrixTransformToWorld(test_mx.GetPointer());
  if (!Matrix4x4AreEqual(w_from_e_mx.GetPointer(), test_mx.GetPointer()))
    {
    std::cerr << __LINE__ << " vtkMRMLTransformNodeTest1 failed" << std::endl;
    return EXIT_FAILURE;
    }

  // GetMatrixTransformToNode: target node is in different branch
  rTransform->GetMatrixTransformToNode(cTransform.GetPointer(), test_mx.GetPointer());
  if (!Matrix4x4AreEqual(c_from_r_mx.GetPointer(), test_mx.GetPointer()))
    {
    std::cerr << __LINE__ << " vtkMRMLTransformNodeTest1 failed" << std::endl;
    return EXIT_FAILURE;
    }
  // GetMatrixTransformToNode: target node is the same as the source
  eTransform->GetMatrixTransformToNode(eTransform.GetPointer(), test_mx.GetPointer());
  vtkNew<vtkMatrix4x4> identity;
  if (!Matrix4x4AreEqual(identity.GetPointer(), test_mx.GetPointer()))
    {
    std::cerr << __LINE__ << " vtkMRMLTransformNodeTest1 failed" << std::endl;
    return EXIT_FAILURE;
    }

  // Test when there is a nonlinear transform above the common parent of two transform nodes.
  // Transform to world is nonlinear but the relative transform is linear.
  vtkNew<vtkMRMLBSplineTransformNode> nonlinearTransform;
  scene->AddNode(nonlinearTransform.GetPointer());
  bTransform->SetAndObserveTransformNodeID(nonlinearTransform->GetID());
  //
  // WORLD -> w coordinate system
  //  |-- nonlinearTransform
  //       |-- bTransform
  //              |-- cTransform
  //              |      |-- dTransform
  //              |             |-- eTransform
  //              |-- qTransform
  //                     |-- rTransform
  //
  rTransform->GetMatrixTransformToNode(cTransform.GetPointer(), test_mx.GetPointer());
  if (!Matrix4x4AreEqual(c_from_r_mx.GetPointer(), test_mx.GetPointer()))
    {
    std::cerr << __LINE__ << " vtkMRMLTransformNodeTest1 failed" << std::endl;
    return EXIT_FAILURE;
    }
  if (rTransform->GetFirstCommonParent(dTransform.GetPointer()) != bTransform.GetPointer())
    {
    std::cerr << __LINE__ << " vtkMRMLTransformNodeTest1 failed" << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "vtkMRMLTransformNodeTest1 successfully completed" << std::endl;

  return EXIT_SUCCESS;
}

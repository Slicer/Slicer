/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>

// vtkAddon includes
#include <vtkAddonMathUtilities.h>

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLTransformStorageNode.h"
#include "vtkMRMLScene.h"

int TestSaveAndRead(std::string filename, vtkMRMLScene* scene, vtkMatrix4x4* matrix, double centerOfTransformation[3])
{
  std::cout << std::endl << "|||||||||||||" << std::endl << filename << std::endl << std::endl;

  vtkMRMLTransformNode* writeTransformNode =
    vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  writeTransformNode->SetMatrixTransformToParent(matrix);
  writeTransformNode->SetCenterOfTransformation(centerOfTransformation);

  writeTransformNode->AddDefaultStorageNode();
  vtkMRMLTransformStorageNode* writeStorageNode =
    vtkMRMLTransformStorageNode::SafeDownCast(writeTransformNode->GetStorageNode());
  if (!writeStorageNode)
  {
    std::cerr << "Error: Storage node is not created." << std::endl;
    return EXIT_FAILURE;
  }
  writeStorageNode->SetFileName(filename.c_str());
  CHECK_INT(writeStorageNode->WriteData(writeTransformNode), 1);

  vtkMRMLTransformNode* readTransformNode =
    vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  readTransformNode->AddDefaultStorageNode();
  vtkMRMLTransformStorageNode* readStorageNode =
    vtkMRMLTransformStorageNode::SafeDownCast(readTransformNode->GetStorageNode());
  readStorageNode->SetFileName(filename.c_str());
  CHECK_INT(readStorageNode->ReadData(readTransformNode), 1);

  vtkNew<vtkMatrix4x4> writeMatrix;
  writeTransformNode->GetMatrixTransformToParent(writeMatrix);
  vtkNew<vtkMatrix4x4> readMatrix;
  readTransformNode->GetMatrixTransformToParent(readMatrix);

  std::cout << "Expected matrix: " << std::endl;
  writeMatrix->PrintSelf(std::cout, vtkIndent(0));
  std::cout << std::endl;

  std::cout << "Actual matrix: " << std::endl;
  readMatrix->PrintSelf(std::cout, vtkIndent(0));
  std::cout << std::endl;

  if (!vtkAddonMathUtilities::MatrixAreEqual(writeMatrix, readMatrix))
  {
    std::cerr << "Error: Read matrix is not equal to the written matrix." << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Center of transformation: " << std::endl;
  double* expectedCenterOfTransformation = writeTransformNode->GetCenterOfTransformation();
  std::cout << "\tExpected: " << std::endl;
  std::cout << "\t" << expectedCenterOfTransformation[0] << "\t" << expectedCenterOfTransformation[1] << "\t"
            << expectedCenterOfTransformation[2] << std::endl;
  double* actualCenterOfTransformation = readTransformNode->GetCenterOfTransformation();
  std::cout << "\tActual: " << std::endl;
  std::cout << "\t" << actualCenterOfTransformation[0] << "\t" << actualCenterOfTransformation[1] << "\t"
            << actualCenterOfTransformation[2] << std::endl;

  double tolerance = 1e-6;
  for (int i = 0; i < 3; ++i)
  {
    double expected = expectedCenterOfTransformation[i];
    double actual = actualCenterOfTransformation[i];
    CHECK_DOUBLE_TOLERANCE(expected, actual, tolerance);
  }
  std::cout << std::endl << filename << " successfully saved and read." << std::endl;

  return EXIT_SUCCESS;
}

int TestCenterOfTransformationReadWrite(vtkMRMLScene* scene)
{
  int fileIndex = 1;

  vtkNew<vtkTransform> transform;
  double centerOfTransformation[3] = { 0.0, 0.0, 0.0 };

  // Default center of transformation, identity transform
  {
    transform->Identity();
    CHECK_EXIT_SUCCESS(TestSaveAndRead(
      std::to_string(fileIndex++) + "_Default_I.h5", scene, transform->GetMatrix(), centerOfTransformation));
  }

  // Default center of transformation, translation
  {
    transform->Identity();
    transform->Translate(4.0, 5.0, 6.0);
    CHECK_EXIT_SUCCESS(TestSaveAndRead(
      std::to_string(fileIndex++) + "_Default_T.h5", scene, transform->GetMatrix(), centerOfTransformation));
  }

  // Default center of transformation, rotation
  {
    transform->Identity();
    transform->RotateX(30.0);
    transform->RotateY(45.0);
    transform->RotateZ(60.0);
    CHECK_EXIT_SUCCESS(TestSaveAndRead(
      std::to_string(fileIndex++) + "_Default_R.h5", scene, transform->GetMatrix(), centerOfTransformation));
  }

  // Default center of transformation, trnslation, rotation
  {
    transform->Identity();
    transform->Translate(4.0, 5.0, 6.0);
    transform->RotateX(30.0);
    transform->RotateY(45.0);
    transform->RotateZ(60.0);
    CHECK_EXIT_SUCCESS(TestSaveAndRead(
      std::to_string(fileIndex++) + "_Default_TR.h5", scene, transform->GetMatrix(), centerOfTransformation));
  }

  // Update center of transformation
  centerOfTransformation[0] = 10.5;
  centerOfTransformation[1] = 21.5;
  centerOfTransformation[2] = 32.5;

  // Non-default center of transformation, identity transform
  {
    transform->Identity();
    CHECK_EXIT_SUCCESS(TestSaveAndRead(
      std::to_string(fileIndex++) + "_NonDefault_I.h5", scene, transform->GetMatrix(), centerOfTransformation));
  }

  // Non-default center of transformation, translation
  {
    transform->Identity();
    transform->Translate(4.0, 5.0, 6.0);
    CHECK_EXIT_SUCCESS(TestSaveAndRead(
      std::to_string(fileIndex++) + "_NonDefault_T.h5", scene, transform->GetMatrix(), centerOfTransformation));
  }

  // Non-default center of transformation, rotation
  {
    transform->Identity();
    transform->RotateX(30.0);
    transform->RotateY(45.0);
    transform->RotateZ(60.0);
    CHECK_EXIT_SUCCESS(TestSaveAndRead(
      std::to_string(fileIndex++) + "_NonDefault_R.h5", scene, transform->GetMatrix(), centerOfTransformation));
  }

  // Non-default center of transformation, translation, rotation
  {
    transform->Identity();
    transform->Translate(4.0, 5.0, 6.0);
    transform->RotateX(30.0);
    transform->RotateY(45.0);
    transform->RotateZ(60.0);
    CHECK_EXIT_SUCCESS(TestSaveAndRead(
      std::to_string(fileIndex++) + "_NonDefault_TR.h5", scene, transform->GetMatrix(), centerOfTransformation));
  }

  return EXIT_SUCCESS;
}

int vtkMRMLTransformStorageNodeTest1(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " /path/to/temp" << std::endl;
    return EXIT_FAILURE;
  }

  vtkNew<vtkMRMLTransformStorageNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1);

  vtkNew<vtkMRMLScene> scene;
  scene->SetRootDirectory(argv[1]);

  CHECK_EXIT_SUCCESS(TestCenterOfTransformationReadWrite(scene));

  return EXIT_SUCCESS;
}

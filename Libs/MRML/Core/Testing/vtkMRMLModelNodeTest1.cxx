/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkCellArray.h>
#include <vtkDataSetAttributes.h>
#include <vtkFloatArray.h>
#include <vtkGeneralTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>
#include <vtkUnstructuredGrid.h>

// STD includes
#include <cmath>
#include <iostream>

//---------------------------------------------------------------------------
int ExerciseBasicMethods();
int TestActiveScalars();
int TestGetSetMesh();
int TestIsOrientationReversingTransform();
int TestApplyTransformReflection();
int TestAutoReverseOrientation();

//---------------------------------------------------------------------------
int vtkMRMLModelNodeTest1(int, char*[])
{
  CHECK_EXIT_SUCCESS(ExerciseBasicMethods());
  CHECK_EXIT_SUCCESS(TestActiveScalars());
  CHECK_EXIT_SUCCESS(TestGetSetMesh());
  CHECK_EXIT_SUCCESS(TestIsOrientationReversingTransform());
  CHECK_EXIT_SUCCESS(TestApplyTransformReflection());
  CHECK_EXIT_SUCCESS(TestAutoReverseOrientation());
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int ExerciseBasicMethods()
{
  vtkNew<vtkMRMLModelNode> node1;
  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestActiveScalars()
{
  vtkNew<vtkMRMLModelNode> node1;

  vtkNew<vtkSphereSource> source;
  node1->SetPolyDataConnection(source->GetOutputPort());

  vtkNew<vtkIntArray> testingArray;
  testingArray->SetName("testingArray");
  node1->AddPointScalars(testingArray.GetPointer());

  vtkNew<vtkIntArray> testingArray2;
  testingArray2->SetName("testingArray2");
  node1->AddCellScalars(testingArray2.GetPointer());

  int attribute = vtkDataSetAttributes::SCALARS;
  node1->SetActivePointScalars("testingArray", attribute);
  node1->SetActiveCellScalars("testingArray2", attribute);

  const char* name = node1->GetActivePointScalarName(vtkDataSetAttributes::SCALARS);
  std::cout << "Active point scalars name = " << (name == nullptr ? "null" : name) << std::endl;
  name = node1->GetActiveCellScalarName(vtkDataSetAttributes::SCALARS);
  std::cout << "Active cell scalars name = " << (name == nullptr ? "null" : name) << std::endl;
  node1->RemoveScalars("testingArray");

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestGetSetMesh()
{
  vtkNew<vtkUnstructuredGrid> ug;
  vtkNew<vtkPolyData> poly;

  vtkNew<vtkMRMLModelNode> node1;
  CHECK_NULL(node1->GetMesh());
  CHECK_NULL(node1->GetMeshConnection());
  CHECK_NULL(node1->GetPolyData());
  CHECK_NULL(node1->GetPolyDataConnection());
  CHECK_NULL(node1->GetUnstructuredGrid());
  CHECK_NULL(node1->GetUnstructuredGridConnection());

  // backward compatible but deprecated
  node1->SetAndObservePolyData(poly.GetPointer());
  CHECK_INT(node1->GetMeshType(), vtkMRMLModelNode::PolyDataMeshType)
  CHECK_NOT_NULL(node1->GetMesh());
  CHECK_NOT_NULL(node1->GetMeshConnection());
  CHECK_NOT_NULL(node1->GetPolyData());
  CHECK_NOT_NULL(node1->GetPolyDataConnection());
  CHECK_NULL(node1->GetUnstructuredGrid());
  CHECK_NULL(node1->GetUnstructuredGridConnection());
  CHECK_POINTER(node1->GetMeshConnection(), node1->GetPolyDataConnection());
  CHECK_POINTER(node1->GetPolyData(), poly.GetPointer());
  CHECK_POINTER(vtkPolyData::SafeDownCast(node1->GetMesh()), poly.GetPointer());

  // set unstructured grid
  node1->SetAndObserveMesh(ug.GetPointer());
  CHECK_INT(node1->GetMeshType(), vtkMRMLModelNode::UnstructuredGridMeshType)
  CHECK_NOT_NULL(node1->GetMesh());
  CHECK_NOT_NULL(node1->GetMeshConnection());
  CHECK_NULL(node1->GetPolyData());
  CHECK_NULL(node1->GetPolyDataConnection());
  CHECK_NOT_NULL(node1->GetUnstructuredGrid());
  CHECK_NOT_NULL(node1->GetUnstructuredGridConnection());
  CHECK_POINTER(node1->GetMeshConnection(), node1->GetUnstructuredGridConnection());
  CHECK_POINTER(node1->GetUnstructuredGrid(), ug.GetPointer());
  CHECK_POINTER(vtkUnstructuredGrid::SafeDownCast(node1->GetMesh()), ug.GetPointer());

  // set poly data
  node1->SetAndObserveMesh(poly.GetPointer());
  CHECK_INT(node1->GetMeshType(), vtkMRMLModelNode::PolyDataMeshType)
  CHECK_NOT_NULL(node1->GetMesh());
  CHECK_NOT_NULL(node1->GetMeshConnection());
  CHECK_NOT_NULL(node1->GetPolyData());
  CHECK_NOT_NULL(node1->GetPolyDataConnection());
  CHECK_NULL(node1->GetUnstructuredGrid());
  CHECK_NULL(node1->GetUnstructuredGridConnection());
  CHECK_POINTER(node1->GetMeshConnection(), node1->GetPolyDataConnection());
  CHECK_POINTER(node1->GetPolyData(), poly.GetPointer());
  CHECK_POINTER(vtkPolyData::SafeDownCast(node1->GetMesh()), poly.GetPointer());

  // deep-copy polydata
  vtkNew<vtkMRMLModelNode> node2;
  node2->SetAndObserveMesh(ug.GetPointer());
  node2->CopyContent(node1.GetPointer(), true);
  CHECK_INT(node2->GetMeshType(), vtkMRMLModelNode::PolyDataMeshType)
  CHECK_NOT_NULL(node2->GetMesh());
  CHECK_NOT_NULL(node2->GetMeshConnection());
  CHECK_NOT_NULL(node2->GetPolyData());
  CHECK_NOT_NULL(node2->GetPolyDataConnection());
  CHECK_NULL(node2->GetUnstructuredGrid());
  CHECK_NULL(node2->GetUnstructuredGridConnection());
  CHECK_INT(node2->GetMesh()->GetNumberOfPoints(), node2->GetPolyData()->GetNumberOfPoints());
  CHECK_INT(node2->GetMesh()->GetNumberOfCells(), node2->GetPolyData()->GetNumberOfCells());
  CHECK_INT(node2->GetPolyData()->GetNumberOfPoints(), poly->GetNumberOfPoints());
  CHECK_INT(node2->GetPolyData()->GetNumberOfCells(), poly->GetNumberOfCells());
  CHECK_INT(vtkPolyData::SafeDownCast(node2->GetMesh())->GetNumberOfPoints(), poly->GetNumberOfPoints());
  CHECK_INT(vtkPolyData::SafeDownCast(node2->GetMesh())->GetNumberOfCells(), poly->GetNumberOfCells());

  // shallow-copy polydata
  node2->CopyContent(node1.GetPointer(), false);
  CHECK_POINTER(node2->GetMeshConnection(), node2->GetPolyDataConnection());
  CHECK_POINTER(node2->GetPolyData(), poly.GetPointer());
  CHECK_POINTER(vtkPolyData::SafeDownCast(node2->GetMesh()), poly.GetPointer());

  // unset mesh
  node2->SetAndObserveMesh(nullptr);
  CHECK_NULL(node2->GetMesh());
  CHECK_NULL(node2->GetMeshConnection());
  CHECK_NULL(node2->GetPolyData());
  CHECK_NULL(node2->GetPolyDataConnection());
  CHECK_NULL(node2->GetUnstructuredGrid());
  CHECK_NULL(node2->GetUnstructuredGridConnection());

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestIsOrientationReversingTransform()
{
  // Null transform
  CHECK_BOOL(vtkMRMLTransformableNode::IsOrientationReversingTransform(nullptr), false);

  // Identity transform (det = 1)
  vtkNew<vtkTransform> identity;
  CHECK_BOOL(vtkMRMLTransformableNode::IsOrientationReversingTransform(identity.GetPointer()), false);

  // Pure rotation (det = 1)
  vtkNew<vtkTransform> rotation;
  rotation->RotateZ(45.0);
  CHECK_BOOL(vtkMRMLTransformableNode::IsOrientationReversingTransform(rotation.GetPointer()), false);

  // Pure translation (det = 1)
  vtkNew<vtkTransform> translation;
  translation->Translate(10.0, 20.0, 30.0);
  CHECK_BOOL(vtkMRMLTransformableNode::IsOrientationReversingTransform(translation.GetPointer()), false);

  // Uniform scaling (det > 0)
  vtkNew<vtkTransform> uniformScale;
  uniformScale->Scale(2.0, 2.0, 2.0);
  CHECK_BOOL(vtkMRMLTransformableNode::IsOrientationReversingTransform(uniformScale.GetPointer()), false);

  // Single-axis reflection (det < 0)
  vtkNew<vtkTransform> reflectX;
  reflectX->Scale(-1.0, 1.0, 1.0);
  CHECK_BOOL(vtkMRMLTransformableNode::IsOrientationReversingTransform(reflectX.GetPointer()), true);

  vtkNew<vtkTransform> reflectY;
  reflectY->Scale(1.0, -1.0, 1.0);
  CHECK_BOOL(vtkMRMLTransformableNode::IsOrientationReversingTransform(reflectY.GetPointer()), true);

  vtkNew<vtkTransform> reflectZ;
  reflectZ->Scale(1.0, 1.0, -1.0);
  CHECK_BOOL(vtkMRMLTransformableNode::IsOrientationReversingTransform(reflectZ.GetPointer()), true);

  // Double reflection (det > 0, two negations cancel)
  vtkNew<vtkTransform> doubleReflect;
  doubleReflect->Scale(-1.0, -1.0, 1.0);
  CHECK_BOOL(vtkMRMLTransformableNode::IsOrientationReversingTransform(doubleReflect.GetPointer()), false);

  // Triple reflection (det < 0)
  vtkNew<vtkTransform> tripleReflect;
  tripleReflect->Scale(-1.0, -1.0, -1.0);
  CHECK_BOOL(vtkMRMLTransformableNode::IsOrientationReversingTransform(tripleReflect.GetPointer()), true);

  // Reflection combined with rotation (det < 0)
  vtkNew<vtkTransform> reflectAndRotate;
  reflectAndRotate->Scale(-1.0, 1.0, 1.0);
  reflectAndRotate->RotateZ(30.0);
  CHECK_BOOL(vtkMRMLTransformableNode::IsOrientationReversingTransform(reflectAndRotate.GetPointer()), true);

  // Composite transform (vtkGeneralTransform) with reflection
  vtkNew<vtkGeneralTransform> compositeReflect;
  vtkNew<vtkTransform> innerRotation;
  innerRotation->RotateX(45.0);
  compositeReflect->Concatenate(innerRotation.GetPointer());
  vtkNew<vtkTransform> innerReflection;
  innerReflection->Scale(-1.0, 1.0, 1.0);
  compositeReflect->Concatenate(innerReflection.GetPointer());
  CHECK_BOOL(vtkMRMLTransformableNode::IsOrientationReversingTransform(compositeReflect.GetPointer()), true);

  // Composite transform without reflection
  vtkNew<vtkGeneralTransform> compositeNoReflect;
  vtkNew<vtkTransform> innerTranslation;
  innerTranslation->Translate(5.0, 0.0, 0.0);
  compositeNoReflect->Concatenate(innerTranslation.GetPointer());
  vtkNew<vtkTransform> innerRotation2;
  innerRotation2->RotateY(90.0);
  compositeNoReflect->Concatenate(innerRotation2.GetPointer());
  CHECK_BOOL(vtkMRMLTransformableNode::IsOrientationReversingTransform(compositeNoReflect.GetPointer()), false);

  // Composite with two reflections (cancel out)
  vtkNew<vtkGeneralTransform> compositeDoubleReflect;
  vtkNew<vtkTransform> reflection1;
  reflection1->Scale(-1.0, 1.0, 1.0);
  compositeDoubleReflect->Concatenate(reflection1.GetPointer());
  vtkNew<vtkTransform> reflection2;
  reflection2->Scale(1.0, -1.0, 1.0);
  compositeDoubleReflect->Concatenate(reflection2.GetPointer());
  CHECK_BOOL(vtkMRMLTransformableNode::IsOrientationReversingTransform(compositeDoubleReflect.GetPointer()), false);

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestApplyTransformReflection()
{
  // Create a simple triangle mesh with explicit point normals.
  // Triangle: (0,0,0), (1,0,0), (0,1,0) with normal (0,0,1)
  vtkNew<vtkPoints> points;
  points->InsertNextPoint(0.0, 0.0, 0.0);
  points->InsertNextPoint(1.0, 0.0, 0.0);
  points->InsertNextPoint(0.0, 1.0, 0.0);

  vtkNew<vtkCellArray> triangles;
  vtkIdType tri[3] = { 0, 1, 2 };
  triangles->InsertNextCell(3, tri);

  vtkNew<vtkFloatArray> normals;
  normals->SetNumberOfComponents(3);
  normals->SetName("Normals");
  normals->InsertNextTuple3(0.0, 0.0, 1.0);
  normals->InsertNextTuple3(0.0, 0.0, 1.0);
  normals->InsertNextTuple3(0.0, 0.0, 1.0);

  vtkNew<vtkPolyData> poly;
  poly->SetPoints(points.GetPointer());
  poly->SetPolys(triangles.GetPointer());
  poly->GetPointData()->SetNormals(normals.GetPointer());

  // Record original cell winding
  vtkIdType npts;
  const vtkIdType* pts;
  poly->GetPolys()->GetCellAtId(0, npts, pts);
  CHECK_INT(static_cast<int>(npts), 3);
  vtkIdType origWinding[3] = { pts[0], pts[1], pts[2] };

  // Apply X-axis reflection to a model node
  vtkNew<vtkMRMLModelNode> modelNode;
  modelNode->SetAndObserveMesh(poly.GetPointer());

  vtkNew<vtkTransform> reflectX;
  reflectX->Scale(-1.0, 1.0, 1.0);
  modelNode->ApplyTransform(reflectX.GetPointer());

  // Verify positions are reflected (x negated)
  vtkPolyData* result = modelNode->GetPolyData();
  CHECK_NOT_NULL(result);
  CHECK_INT(static_cast<int>(result->GetNumberOfPoints()), 3);

  double pt[3];
  result->GetPoint(0, pt);
  CHECK_BOOL(std::fabs(pt[0] - 0.0) < 1e-6, true);
  result->GetPoint(1, pt);
  CHECK_BOOL(std::fabs(pt[0] - (-1.0)) < 1e-6, true);

  // Verify normals are transformed by vtkTransformFilter via inverse-transpose.
  // For Scale(-1,1,1): inverse is Scale(-1,1,1), transpose is the same,
  // so inverse-transpose is Scale(-1,1,1). Applied to normal (0,0,1) gives (0,0,1).
  vtkFloatArray* resultNormals = vtkFloatArray::SafeDownCast(result->GetPointData()->GetNormals());
  CHECK_NOT_NULL(resultNormals);
  double n[3];
  resultNormals->GetTuple(0, n);
  CHECK_BOOL(std::fabs(n[2] - 1.0) < 1e-6, true);

  // Verify cell winding is reversed (ReverseCellsOn was applied)
  result->GetPolys()->GetCellAtId(0, npts, pts);
  CHECK_INT(static_cast<int>(npts), 3);
  // After ReverseCellsOn, winding should be reversed
  CHECK_BOOL(pts[0] == origWinding[2] && pts[1] == origWinding[1] && pts[2] == origWinding[0], true);

  // Apply a non-reflecting transform (rotation) and verify winding is NOT reversed
  vtkNew<vtkMRMLModelNode> modelNode2;
  vtkNew<vtkPolyData> poly2;
  poly2->DeepCopy(poly.GetPointer());
  vtkNew<vtkCellArray> triangles2;
  triangles2->InsertNextCell(3, tri);
  poly2->SetPolys(triangles2.GetPointer());
  modelNode2->SetAndObserveMesh(poly2.GetPointer());

  vtkNew<vtkTransform> rotateOnly;
  rotateOnly->RotateZ(90.0);
  modelNode2->ApplyTransform(rotateOnly.GetPointer());

  vtkPolyData* result2 = modelNode2->GetPolyData();
  CHECK_NOT_NULL(result2);
  result2->GetPolys()->GetCellAtId(0, npts, pts);
  CHECK_INT(static_cast<int>(npts), 3);
  // Winding should be preserved for non-reflecting transform
  CHECK_BOOL(pts[0] == origWinding[0] && pts[1] == origWinding[1] && pts[2] == origWinding[2], true);

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestAutoReverseOrientation()
{
  // Test default value
  vtkNew<vtkMRMLModelNode> modelNode;
  CHECK_BOOL(modelNode->GetAutoReverseOrientation(), true);

  // Test Set/Get/On/Off
  modelNode->SetAutoReverseOrientation(false);
  CHECK_BOOL(modelNode->GetAutoReverseOrientation(), false);
  modelNode->AutoReverseOrientationOn();
  CHECK_BOOL(modelNode->GetAutoReverseOrientation(), true);
  modelNode->AutoReverseOrientationOff();
  CHECK_BOOL(modelNode->GetAutoReverseOrientation(), false);

  // Test that AutoReverseOrientation=false prevents cell winding reversal
  // during ApplyTransform with a reflection
  vtkNew<vtkPoints> points;
  points->InsertNextPoint(0.0, 0.0, 0.0);
  points->InsertNextPoint(1.0, 0.0, 0.0);
  points->InsertNextPoint(0.0, 1.0, 0.0);

  vtkNew<vtkCellArray> triangles;
  vtkIdType tri[3] = { 0, 1, 2 };
  triangles->InsertNextCell(3, tri);

  vtkNew<vtkPolyData> poly;
  poly->SetPoints(points.GetPointer());
  poly->SetPolys(triangles.GetPointer());

  // Record original winding
  vtkIdType npts;
  const vtkIdType* pts;
  poly->GetPolys()->GetCellAtId(0, npts, pts);
  vtkIdType origWinding[3] = { pts[0], pts[1], pts[2] };

  vtkNew<vtkMRMLModelNode> modelNodeOff;
  modelNodeOff->SetAutoReverseOrientation(false);
  modelNodeOff->SetAndObserveMesh(poly.GetPointer());

  vtkNew<vtkTransform> reflectX;
  reflectX->Scale(-1.0, 1.0, 1.0);
  modelNodeOff->ApplyTransform(reflectX.GetPointer());

  // With AutoReverseOrientation=false, winding should NOT be reversed
  vtkPolyData* result = modelNodeOff->GetPolyData();
  CHECK_NOT_NULL(result);
  result->GetPolys()->GetCellAtId(0, npts, pts);
  CHECK_INT(static_cast<int>(npts), 3);
  CHECK_BOOL(pts[0] == origWinding[0] && pts[1] == origWinding[1] && pts[2] == origWinding[2], true);

  // Test CopyContent preserves AutoReverseOrientation
  vtkNew<vtkMRMLModelNode> sourceNode;
  sourceNode->SetAutoReverseOrientation(false);
  vtkNew<vtkMRMLModelNode> targetNode;
  CHECK_BOOL(targetNode->GetAutoReverseOrientation(), true);
  targetNode->CopyContent(sourceNode.GetPointer());
  CHECK_BOOL(targetNode->GetAutoReverseOrientation(), false);

  // Test XML serialization round-trip
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLModelNode> nodeToSerialize;
  nodeToSerialize->SetAutoReverseOrientation(false);
  scene->AddNode(nodeToSerialize.GetPointer());

  // Write scene to string
  scene->SetSaveToXMLString(1);
  scene->Commit();
  std::string sceneXML = scene->GetSceneXMLString();

  // Verify the XML contains the attribute
  CHECK_BOOL(sceneXML.find("autoReverseOrientation=\"false\"") != std::string::npos, true);

  // Read scene from string
  vtkNew<vtkMRMLScene> scene2;
  scene2->SetLoadFromXMLString(1);
  scene2->SetSceneXMLString(sceneXML);
  scene2->Import();

  // Find the model node in the imported scene
  vtkMRMLModelNode* importedNode = vtkMRMLModelNode::SafeDownCast(scene2->GetFirstNodeByClass("vtkMRMLModelNode"));
  CHECK_NOT_NULL(importedNode);
  CHECK_BOOL(importedNode->GetAutoReverseOrientation(), false);

  // Also verify default value (true) is not written to XML
  vtkNew<vtkMRMLScene> scene3;
  vtkNew<vtkMRMLModelNode> defaultNode;
  scene3->AddNode(defaultNode.GetPointer());
  scene3->SetSaveToXMLString(1);
  scene3->Commit();
  std::string sceneXML3 = scene3->GetSceneXMLString();
  CHECK_BOOL(sceneXML3.find("autoReverseOrientation") == std::string::npos, true);

  return EXIT_SUCCESS;
}

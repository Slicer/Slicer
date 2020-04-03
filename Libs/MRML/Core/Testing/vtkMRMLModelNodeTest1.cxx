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

// VTK includes
#include <vtkDataSetAttributes.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkUnstructuredGrid.h>

//---------------------------------------------------------------------------
int ExerciseBasicMethods();
int TestActiveScalars();
int TestGetSetMesh();

//---------------------------------------------------------------------------
int vtkMRMLModelNodeTest1(int , char * [] )
{
  CHECK_EXIT_SUCCESS(ExerciseBasicMethods());
  CHECK_EXIT_SUCCESS(TestActiveScalars());
  CHECK_EXIT_SUCCESS(TestGetSetMesh());
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

  const char *name = node1->GetActivePointScalarName(vtkDataSetAttributes::SCALARS);
  std::cout << "Active point scalars name = " << (name  == nullptr ? "null" : name) << std::endl;
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

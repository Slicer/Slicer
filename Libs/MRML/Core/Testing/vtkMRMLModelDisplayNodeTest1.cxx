/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkGeometryFilter.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkShortArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVoxel.h>

// STD includes
#include <iostream>

namespace
{
const char* VOXEL_ARRAY_NAME = "ids";
}
//---------------------------------------------------------------------------
int TestSetMesh(bool observeMeshBeforeObserveDisplay, bool observeDisplayBeforeAddToScene, bool meshTypeIsPolyData);

int TestScalarRange(vtkPointSet* mesh);
int TestThreshold(vtkPointSet* mesh);
int TestMergeCoincidentPoints();
void CreateVoxelMeshes(vtkUnstructuredGrid* ug, vtkPolyData* poly);

//---------------------------------------------------------------------------
int vtkMRMLModelDisplayNodeTest1(int, char*[])
{
  vtkSmartPointer<vtkMRMLModelDisplayNode> node1 = vtkSmartPointer<vtkMRMLModelDisplayNode>::New();
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  vtkNew<vtkUnstructuredGrid> ug;
  vtkNew<vtkPolyData> poly;
  CreateVoxelMeshes(ug.GetPointer(), poly.GetPointer());

  CHECK_EXIT_SUCCESS(TestScalarRange(ug.GetPointer()));
  CHECK_EXIT_SUCCESS(TestScalarRange(poly.GetPointer()));

  CHECK_EXIT_SUCCESS(TestThreshold(ug.GetPointer()));
  CHECK_EXIT_SUCCESS(TestThreshold(poly.GetPointer()));

  CHECK_EXIT_SUCCESS(TestMergeCoincidentPoints());

  CHECK_EXIT_SUCCESS(TestSetMesh(true, true, true));
  CHECK_EXIT_SUCCESS(TestSetMesh(true, true, false));
  CHECK_EXIT_SUCCESS(TestSetMesh(true, false, true));
  CHECK_EXIT_SUCCESS(TestSetMesh(true, false, false));
  CHECK_EXIT_SUCCESS(TestSetMesh(false, true, true));
  CHECK_EXIT_SUCCESS(TestSetMesh(false, true, false));
  CHECK_EXIT_SUCCESS(TestSetMesh(false, false, true));
  CHECK_EXIT_SUCCESS(TestSetMesh(false, false, false));

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestScalarRange(vtkPointSet* mesh)
{
  double dataRange[2];
  mesh->GetPointData()->GetArray(VOXEL_ARRAY_NAME)->GetRange(dataRange);

  vtkNew<vtkMRMLModelNode> mNode;
  mNode->SetAndObserveMesh(mesh);

  vtkNew<vtkMRMLModelDisplayNode> mdNode;
  mdNode->SetInputMeshConnection(mNode->GetMeshConnection());

  mdNode->SetActiveScalarName(VOXEL_ARRAY_NAME);
  CHECK_STRING(mdNode->GetActiveScalarName(), VOXEL_ARRAY_NAME);

  double mdRange[2];

  // default should be data range
  mdNode->GetScalarRange(mdRange);
  CHECK_DOUBLE(mdRange[0], dataRange[0]);
  CHECK_DOUBLE(mdRange[1], dataRange[1]);

  // test manual change
  mdNode->SetScalarRangeFlag(vtkMRMLModelDisplayNode::UseManualScalarRange);
  mdNode->SetScalarRange(-1.0, 1.0);
  mdNode->GetScalarRange(mdRange);
  CHECK_DOUBLE(mdRange[0], -1.0);
  CHECK_DOUBLE(mdRange[1], 1.0);

  // test back to data range
  mdNode->SetScalarRangeFlag(vtkMRMLModelDisplayNode::UseDataScalarRange);
  mdNode->GetScalarRange(mdRange);
  CHECK_DOUBLE(mdRange[0], dataRange[0]);
  CHECK_DOUBLE(mdRange[1], dataRange[1]);

  // test data type range
  mdNode->SetScalarRangeFlag(vtkMRMLModelDisplayNode::UseDataTypeScalarRange);
  mdNode->GetScalarRange(mdRange);
  CHECK_DOUBLE(mdRange[0], VTK_SHORT_MIN);
  CHECK_DOUBLE(mdRange[1], VTK_SHORT_MAX);

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestThreshold(vtkPointSet* mesh)
{
  vtkNew<vtkMRMLModelNode> mNode;
  mNode->SetAndObserveMesh(mesh);

  vtkNew<vtkMRMLModelDisplayNode> mdNode;
  mdNode->SetInputMeshConnection(mNode->GetMeshConnection());

  mdNode->SetActiveScalarName(VOXEL_ARRAY_NAME);
  mdNode->ScalarVisibilityOn(); // needs visibility on to threshold
  mdNode->ThresholdEnabledOn();

  double tRange[2];
  double mdRange[2];
  mdNode->GetScalarRange(mdRange);

  // Warning: Since there is only one cell (once voxel)
  // once a point is outside of the threshold range, the
  // whole cell is removed.
  mdNode->SetThresholdRange(-2, 12);
  mdNode->GetOutputMesh()->GetScalarRange(tRange);
  CHECK_DOUBLE(tRange[0], mdRange[0]);
  CHECK_DOUBLE(tRange[1], mdRange[1]);
  CHECK_DOUBLE(mdNode->GetOutputMesh()->GetNumberOfCells(), mesh->GetNumberOfCells());

  mdNode->SetThresholdRange(2, 9);
  mdNode->GetOutputMesh()->GetScalarRange(tRange);
  CHECK_BOOL(tRange[0] > mdRange[0], true);
  CHECK_BOOL(mdNode->GetOutputMesh()->GetNumberOfCells() < mesh->GetNumberOfCells(), true);

  mdNode->SetThresholdRange(-2, 5);
  mdNode->GetOutputMesh()->GetScalarRange(tRange);
  CHECK_BOOL(tRange[1] < mdRange[1], true);
  CHECK_BOOL(mdNode->GetOutputMesh()->GetNumberOfCells() < mesh->GetNumberOfCells(), true);

  mdNode->SetThresholdRange(3, 4);
  mdNode->GetOutputMesh()->GetScalarRange(tRange);
  CHECK_BOOL(tRange[0] > mdRange[0], true);
  CHECK_BOOL(tRange[1] < mdRange[1], true);
  CHECK_BOOL(mdNode->GetOutputMesh()->GetNumberOfCells() < mesh->GetNumberOfCells(), true);

  mdNode->ThresholdEnabledOff();
  CHECK_DOUBLE(mdNode->GetOutputMesh()->GetNumberOfCells(), mesh->GetNumberOfCells());

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
namespace
{
int GetNumberOfSurfaceCells(vtkPointSet* mesh)
{
  vtkNew<vtkGeometryFilter> surfaceFilter;
  surfaceFilter->SetInputData(mesh);
  surfaceFilter->Update();
  return surfaceFilter->GetOutput()->GetNumberOfCells();
}
} // namespace

//---------------------------------------------------------------------------
int TestMergeCoincidentPoints()
{
  // Two tetrahedra that have a common face, but the points of that face are
  // stored twice, so the cells do not share any points.
  vtkNew<vtkPoints> points;
  points->InsertNextPoint(0.0, 0.0, 0.0);
  points->InsertNextPoint(1.0, 0.0, 0.0);
  points->InsertNextPoint(0.0, 1.0, 0.0);
  points->InsertNextPoint(0.0, 0.0, 1.0);
  points->InsertNextPoint(1.0, 1.0, 1.0);
  // duplicates of points 1, 2, 3
  points->InsertNextPoint(1.0, 0.0, 0.0);
  points->InsertNextPoint(0.0, 1.0, 0.0);
  points->InsertNextPoint(0.0, 0.0, 1.0);
  vtkNew<vtkUnstructuredGrid> ug;
  ug->SetPoints(points);
  vtkIdType tetra1[4] = { 0, 1, 2, 3 };
  vtkIdType tetra2[4] = { 4, 5, 6, 7 };
  ug->InsertNextCell(VTK_TETRA, 4, tetra1);
  ug->InsertNextCell(VTK_TETRA, 4, tetra2);
  // Without merging the points, all the 8 faces are on the surface
  CHECK_INT(GetNumberOfSurfaceCells(ug), 8);

  vtkNew<vtkMRMLScene> scene;
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelDisplayNode"));
  modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  modelNode->SetAndObserveMesh(ug);

  // Points are merged by default, so the common face is not on the surface
  CHECK_BOOL(displayNode->GetMergeCoincidentPoints(), true);
  CHECK_INT(displayNode->GetOutputMesh()->GetNumberOfCells(), 2);
  CHECK_INT(GetNumberOfSurfaceCells(displayNode->GetOutputMesh()), 6);

  // Thresholding is done before merging
  vtkNew<vtkShortArray> scalars;
  scalars->SetName(VOXEL_ARRAY_NAME);
  for (vtkIdType pointIndex = 0; pointIndex < points->GetNumberOfPoints(); ++pointIndex)
  {
    scalars->InsertNextValue(1);
  }
  ug->GetPointData()->AddArray(scalars);
  ug->Modified();
  displayNode->SetActiveScalarName(VOXEL_ARRAY_NAME);
  displayNode->ScalarVisibilityOn();
  displayNode->SetThresholdRange(0, 2);
  displayNode->ThresholdEnabledOn();
  CHECK_INT(GetNumberOfSurfaceCells(displayNode->GetOutputMesh()), 6);
  displayNode->ThresholdEnabledOff();

  // Without merging, the internal faces appear
  displayNode->SetMergeCoincidentPoints(false);
  CHECK_INT(GetNumberOfSurfaceCells(displayNode->GetOutputMesh()), 8);
  displayNode->MergeCoincidentPointsOn();
  CHECK_INT(GetNumberOfSurfaceCells(displayNode->GetOutputMesh()), 6);

  // Input mesh is not changed
  CHECK_INT(ug->GetNumberOfPoints(), 8);
  CHECK_POINTER(displayNode->GetInputMesh(), ug.GetPointer());

  // Option is saved in the scene and copied
  displayNode->SetMergeCoincidentPoints(false);
  scene->SetSaveToXMLString(1);
  scene->Commit();
  std::string xmlScene = scene->GetSceneXMLString();
  vtkNew<vtkMRMLScene> reloadedScene;
  reloadedScene->SetLoadFromXMLString(1);
  reloadedScene->SetSceneXMLString(xmlScene);
  reloadedScene->Import();
  vtkMRMLModelDisplayNode* reloadedDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(reloadedScene->GetFirstNodeByClass("vtkMRMLModelDisplayNode"));
  CHECK_NOT_NULL(reloadedDisplayNode);
  CHECK_BOOL(reloadedDisplayNode->GetMergeCoincidentPoints(), false);
  vtkNew<vtkMRMLModelDisplayNode> copiedDisplayNode;
  copiedDisplayNode->CopyContent(displayNode);
  CHECK_BOOL(copiedDisplayNode->GetMergeCoincidentPoints(), false);

  // Polydata meshes are not affected
  vtkNew<vtkPolyData> poly;
  vtkNew<vtkUnstructuredGrid> voxelUg;
  CreateVoxelMeshes(voxelUg, poly);
  modelNode->SetAndObserveMesh(poly);
  displayNode->MergeCoincidentPointsOn();
  CHECK_BOOL(vtkPolyData::SafeDownCast(displayNode->GetOutputMesh()) != nullptr, true);
  CHECK_INT(displayNode->GetOutputMesh()->GetNumberOfPoints(), poly->GetNumberOfPoints());

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestSetMesh(bool observeMeshBeforeObserveDisplay, bool observeDisplayBeforeAddToScene, bool meshTypeIsPolyData)
{
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLModelNode> model;
  scene->AddNode(model.GetPointer());

  vtkPointSet* mesh;
  if (meshTypeIsPolyData)
  {
    mesh = vtkPolyData::New();
  }
  else
  {
    mesh = vtkUnstructuredGrid::New();
  }

  if (observeMeshBeforeObserveDisplay)
  {
    model->SetAndObserveMesh(mesh);
  }

  vtkNew<vtkMRMLModelDisplayNode> display;
  if (!observeDisplayBeforeAddToScene)
  {
    scene->AddNode(display.GetPointer());
  }

  model->SetAndObserveDisplayNodeID("vtkMRMLModelDisplayNode1");
  if (!observeMeshBeforeObserveDisplay)
  {
    model->SetAndObserveMesh(mesh);
  }
  if (observeDisplayBeforeAddToScene)
  {
    scene->AddNode(display.GetPointer());
    model->UpdateScene(scene.GetPointer());
  }

  vtkPointSet* displayInputMesh = display->GetInputMesh();
  if ((meshTypeIsPolyData && displayInputMesh != display->GetInputPolyData()) ||          //
      (!meshTypeIsPolyData && displayInputMesh != display->GetInputUnstructuredGrid()) || //
      displayInputMesh != model->GetMesh() ||                                             //
      displayInputMesh != mesh)
  {
    std::cerr << __LINE__ << ": vtkMRMLModelNode::SetAndobserveMesh failed when " //
              << (meshTypeIsPolyData ? "surface" : "volumetric")                  //
              << " mesh is set "                                                  //
              << (observeMeshBeforeObserveDisplay ? "before" : "after")           //
              << " the display node is observed and when the display node is "    //
              << "added in the scene "                                            //
              << (observeDisplayBeforeAddToScene ? "before" : "after")            //
              << " the observation:\n"                                            //
              << "Mesh: " << mesh << ", "                                         //
              << "Model: " << model->GetMesh() << ", "                            //
              << "Display: " << display->GetInputMesh() << std::endl;
    mesh->Delete();
    return EXIT_FAILURE;
  }
  mesh->Delete();
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
void CreateVoxelMeshes(vtkUnstructuredGrid* ug, vtkPolyData* poly)
{
  vtkNew<vtkPoints> points;
  points->InsertNextPoint(0, 0, 0);
  points->InsertNextPoint(1, 0, 0);
  points->InsertNextPoint(0, 1, 0);
  points->InsertNextPoint(1, 1, 0);
  points->InsertNextPoint(0, 0, 1);
  points->InsertNextPoint(1, 0, 1);
  points->InsertNextPoint(0, 1, 1);
  points->InsertNextPoint(1, 1, 1);

  vtkNew<vtkVoxel> voxel;
  voxel->GetPointIds()->SetId(0, 0);
  voxel->GetPointIds()->SetId(1, 1);
  voxel->GetPointIds()->SetId(2, 2);
  voxel->GetPointIds()->SetId(3, 3);
  voxel->GetPointIds()->SetId(4, 4);
  voxel->GetPointIds()->SetId(5, 5);
  voxel->GetPointIds()->SetId(6, 6);
  voxel->GetPointIds()->SetId(7, 7);

  vtkNew<vtkShortArray> array;
  array->SetName(VOXEL_ARRAY_NAME);
  array->InsertNextValue(0);
  array->InsertNextValue(1);
  array->InsertNextValue(2);
  array->InsertNextValue(3);
  array->InsertNextValue(4);
  array->InsertNextValue(5);
  array->InsertNextValue(6);
  array->InsertNextValue(7);

  ug->SetPoints(points.GetPointer());
  ug->InsertNextCell(voxel->GetCellType(), voxel->GetPointIds());
  ug->GetPointData()->AddArray(array.GetPointer());

  vtkNew<vtkGeometryFilter> filter;
  filter->SetInputData(ug);
  filter->Update();
  poly->DeepCopy(filter->GetOutput());
}

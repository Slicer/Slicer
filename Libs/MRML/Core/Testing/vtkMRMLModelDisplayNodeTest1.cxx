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
#include <vtkNew.h>
#include <vtkPolyData.h>

//---------------------------------------------------------------------------
int TestSetPolyData(bool observePolyDataBeforeObserveDisplay,
                     bool observeDisplayBeforeAddToScene);

//---------------------------------------------------------------------------
int vtkMRMLModelDisplayNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLModelDisplayNode > node1 = vtkSmartPointer< vtkMRMLModelDisplayNode >::New();
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  CHECK_EXIT_SUCCESS(TestSetPolyData(true, true));
  CHECK_EXIT_SUCCESS(TestSetPolyData(true, false));
  CHECK_EXIT_SUCCESS(TestSetPolyData(false, true));
  CHECK_EXIT_SUCCESS(TestSetPolyData(false, false));

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestSetPolyData(bool observePolyDataBeforeObserveDisplay,
                     bool observeDisplayBeforeAddToScene)
{
  std::cout << __LINE__ << "TestSetPolyData" << std::endl;
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLModelNode> model;
  scene->AddNode(model.GetPointer());

  vtkNew<vtkPolyData> polyData;
  if (observePolyDataBeforeObserveDisplay)
    {
    model->SetAndObservePolyData(polyData.GetPointer());
    }

  vtkNew<vtkMRMLModelDisplayNode> display;
  if (!observeDisplayBeforeAddToScene)
    {
    scene->AddNode(display.GetPointer());
    }

  model->SetAndObserveDisplayNodeID("vtkMRMLModelDisplayNode1");
  if (!observePolyDataBeforeObserveDisplay)
    {
    model->SetAndObservePolyData(polyData.GetPointer());
    }
  if (observeDisplayBeforeAddToScene)
    {
    scene->AddNode(display.GetPointer());
    model->UpdateScene(scene.GetPointer());
    }
  if (display->GetInputPolyData() != model->GetPolyData())
    {
    std::cerr << __LINE__ << ": vtkMRMLModelNode::SetAndObservePolyData "
              << "failed when polydata is set "
              << (observePolyDataBeforeObserveDisplay ? "before" : "after")
              << " the display node is observed and when the display node is "
              << "added in the scene "
              << (observeDisplayBeforeAddToScene ? "before" : "after")
              << " the observation:\n"
              << "PolyData: " << polyData.GetPointer() << ", "
              << "Model: " << model->GetPolyData() << ", "
              << "Display: " << display->GetInputPolyData() << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

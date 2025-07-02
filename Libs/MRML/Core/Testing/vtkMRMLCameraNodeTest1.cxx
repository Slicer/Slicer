/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLCoreTestingMacros.h"

namespace
{

//---------------------------------------------------------------------------
int ExerciseBasicMethods();
int TestGetSetLayoutName();

} // namespace

//---------------------------------------------------------------------------
int vtkMRMLCameraNodeTest1(int, char*[])
{
  CHECK_EXIT_SUCCESS(ExerciseBasicMethods());
  CHECK_EXIT_SUCCESS(TestGetSetLayoutName());
  return EXIT_SUCCESS;
}

namespace
{

//---------------------------------------------------------------------------
int ExerciseBasicMethods()
{
  vtkNew<vtkMRMLCameraNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestGetSetLayoutName()
{
  vtkNew<vtkMRMLCameraNode> node1;

  vtkNew<vtkMRMLCoreTestingUtilities::vtkMRMLNodeCallback> callback;
  node1->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  CHECK_NULL(node1->GetLayoutName());

  node1->SetLayoutName(nullptr);
  CHECK_INT(callback->GetTotalNumberOfEvents(), 0);

  node1->SetLayoutName("1");
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLCameraNode::LayoutNameModifiedEvent), 1);
  CHECK_INT(callback->GetNumberOfModified(), 1);
  CHECK_INT(callback->GetTotalNumberOfEvents(), 2);

  callback->ResetNumberOfEvents();
  node1->SetLayoutName("1");
  CHECK_INT(callback->GetTotalNumberOfEvents(), 0);

  callback->ResetNumberOfEvents();
  node1->SetLayoutName("2");
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLCameraNode::LayoutNameModifiedEvent), 1);
  CHECK_INT(callback->GetNumberOfModified(), 1);
  CHECK_INT(callback->GetTotalNumberOfEvents(), 2);

  callback->ResetNumberOfEvents();
  node1->SetLayoutName(nullptr);
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLCameraNode::LayoutNameModifiedEvent), 1);
  CHECK_INT(callback->GetNumberOfModified(), 1);
  CHECK_INT(callback->GetTotalNumberOfEvents(), 2);

  return EXIT_SUCCESS;
}

} // namespace

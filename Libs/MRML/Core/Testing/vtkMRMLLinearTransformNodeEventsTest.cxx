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

// VTK includes
#include <vtkGeneralTransform.h>
#include <vtkNew.h>

//---------------------------------------------------------------------------
int vtkMRMLLinearTransformNodeEventsTest(int , char * [] )
{
  vtkNew< vtkMRMLLinearTransformNode > linearTransformNode;

  vtkNew<vtkMRMLNodeCallback> callback;

  linearTransformNode->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  // Test vtkMRMLLinearTransformNode::SetAndObserveMatrixTransformToParent()
  vtkNew<vtkMatrix4x4> matrix;
  linearTransformNode->SetAndObserveMatrixTransformToParent(matrix.GetPointer());

  if (linearTransformNode->GetMatrixTransformToParent() != matrix.GetPointer())
    {
    return EXIT_FAILURE;
    }
  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfModified() != 1 ||
      callback->GetNumberOfEvents(vtkMRMLTransformNode::TransformModifiedEvent) != 1)
    {
    std::cerr << "vtkMRMLLinearTransformNode::SetAndObserveMatrixTransformToParent failed (1)."
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: " << callback->GetNumberOfModified() << " "
              << "Number of TransformModifiedEvent: "
              << callback->GetNumberOfEvents(vtkMRMLTransformNode::TransformModifiedEvent)
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->ResetNumberOfEvents();

  // Set the same matrix:
  linearTransformNode->SetAndObserveMatrixTransformToParent(matrix.GetPointer());
  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfModified() != 0 ||
      callback->GetNumberOfEvents(vtkMRMLTransformNode::TransformModifiedEvent) != 0)
    {
    std::cerr << "vtkMRMLLinearTransformNode::SetAndObserveMatrixTransformToParent failed (2)."
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: " << callback->GetNumberOfModified() << " "
              << "Number of TransformModifiedEvent: "
              << callback->GetNumberOfEvents(vtkMRMLTransformNode::TransformModifiedEvent)
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->ResetNumberOfEvents();

  // Update matrix
  matrix->Modified();
  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfModified() != 0 ||
      callback->GetNumberOfEvents(vtkMRMLTransformNode::TransformModifiedEvent) != 1)
    {
    std::cerr << "vtkMatrix4x4::Modified failed."
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: " << callback->GetNumberOfModified() << " "
              << "Number of TransformModifiedEvent: "
              << callback->GetNumberOfEvents(vtkMRMLTransformNode::TransformModifiedEvent)
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->ResetNumberOfEvents();

  return EXIT_SUCCESS;
}

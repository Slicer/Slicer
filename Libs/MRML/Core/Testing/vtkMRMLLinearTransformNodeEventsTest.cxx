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

  vtkNew<vtkMRMLCoreTestingUtilities::vtkMRMLNodeCallback> callback;

  linearTransformNode->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  // Test vtkMRMLLinearTransformNode::SetAndObserveMatrixTransformToParent()
  vtkNew<vtkMatrix4x4> matrix;
  linearTransformNode->SetMatrixTransformToParent(matrix.GetPointer());

  vtkNew<vtkMatrix4x4> matrixRetrieved;
  if (!linearTransformNode->GetMatrixTransformToParent(matrixRetrieved.GetPointer()))
    {
    std::cerr << "GetMatrixTransformToParent() failed" << std::endl;
    return EXIT_FAILURE;
    }
  if ( fabs(matrixRetrieved->Element[0][0]-matrix->Element[0][0])>0.001
    || fabs(matrixRetrieved->Element[0][1]-matrix->Element[0][1])>0.001
    || fabs(matrixRetrieved->Element[0][2]-matrix->Element[0][2])>0.001
    || fabs(matrixRetrieved->Element[0][3]-matrix->Element[0][3])>0.001
    || fabs(matrixRetrieved->Element[1][0]-matrix->Element[1][0])>0.001
    || fabs(matrixRetrieved->Element[1][1]-matrix->Element[1][1])>0.001
    || fabs(matrixRetrieved->Element[1][2]-matrix->Element[1][2])>0.001
    || fabs(matrixRetrieved->Element[1][3]-matrix->Element[1][3])>0.001
    || fabs(matrixRetrieved->Element[2][0]-matrix->Element[2][0])>0.001
    || fabs(matrixRetrieved->Element[2][1]-matrix->Element[2][1])>0.001
    || fabs(matrixRetrieved->Element[2][2]-matrix->Element[2][2])>0.001
    || fabs(matrixRetrieved->Element[2][3]-matrix->Element[2][3])>0.001 )
    {
    std::cerr << "GetMatrixTransformToParent() returned incorrect result" << std::endl;
    return EXIT_FAILURE;
    }
  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfModified() != 0 ||
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
  linearTransformNode->SetMatrixTransformToParent(matrix.GetPointer());
  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfModified() != 0 ||
      callback->GetNumberOfEvents(vtkMRMLTransformNode::TransformModifiedEvent) != 1)
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

  // Update matrix, as the matrix is not observed we expect that the output will not be updated
  double originalElement03=matrix->Element[0][3];
  matrix->Element[0][3]=originalElement03+1234.3456;
  matrix->Modified();
  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfModified() != 0 ||
      callback->GetNumberOfEvents(vtkMRMLTransformNode::TransformModifiedEvent) != 0)
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

  if (!linearTransformNode->GetMatrixTransformToParent(matrixRetrieved.GetPointer()))
    {
    std::cerr << "GetMatrixTransformToParent() failed" << std::endl;
    return EXIT_FAILURE;
    }
  if ( fabs(matrixRetrieved->Element[0][3]-originalElement03)>0.001 )
    {
    std::cerr << "GetMatrixTransformToParent() changed while expected it to remain the same" << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

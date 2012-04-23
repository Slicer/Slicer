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
class vtkMRMLLinearTransformNodeCallback : public vtkMRMLNodeCallback
{
public:
  static vtkMRMLLinearTransformNodeCallback *New() {return new vtkMRMLLinearTransformNodeCallback;};

  virtual void Execute(vtkObject* caller, unsigned long eid, void *callData);
  virtual void ResetNumberOfEventsVariables();

  int GetNumberOfTransformModified();

protected:
  vtkMRMLLinearTransformNodeCallback();
  int NumberOfTransformModified;
};

//---------------------------------------------------------------------------
vtkMRMLLinearTransformNodeCallback::vtkMRMLLinearTransformNodeCallback()
{
  this->ResetNumberOfEventsVariables();
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformNodeCallback::ResetNumberOfEventsVariables()
{
  this->vtkMRMLNodeCallback::ResetNumberOfEventsVariables();
  this->NumberOfTransformModified = 0;
}

//---------------------------------------------------------------------------
int vtkMRMLLinearTransformNodeCallback::GetNumberOfTransformModified()
{
  return this->NumberOfTransformModified;
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformNodeCallback::Execute(vtkObject *vtkcaller,
  unsigned long eid, void *calldata)
{
  this->vtkMRMLNodeCallback::Execute(vtkcaller, eid, calldata);
  // Let's return if an error already occured
  if (this->ErrorString.size() > 0)
    {
    return;
    }

  if (eid == vtkMRMLTransformNode::TransformModifiedEvent)
    {
    ++this->NumberOfTransformModified;
    }
}

//---------------------------------------------------------------------------
int vtkMRMLLinearTransformNodeEventsTest(int , char * [] )
{
  vtkNew< vtkMRMLLinearTransformNode > linearTransformNode;

  vtkNew<vtkMRMLLinearTransformNodeCallback> callback;

  linearTransformNode->AddObserver(vtkCommand::ModifiedEvent, callback.GetPointer());
  linearTransformNode->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent, callback.GetPointer());

  // Test vtkMRMLLinearTransformNode::SetAndObserveMatrixTransformToParent()
  vtkNew<vtkMatrix4x4> matrix;
  linearTransformNode->SetAndObserveMatrixTransformToParent(matrix.GetPointer());

  if (linearTransformNode->GetMatrixTransformToParent() != matrix.GetPointer())
    {
    return EXIT_FAILURE;
    }
  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfModified() != 1 ||
      callback->GetNumberOfTransformModified() != 1)
    {
    std::cerr << "vtkMRMLLinearTransformNode::SetAndObserveMatrixTransformToParent failed (1)."
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: " << callback->GetNumberOfModified() << " "
              << "Number of TransformModifiedEvent: " << callback->GetNumberOfTransformModified()
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->ResetNumberOfEventsVariables();

  // Set the same matrix:
  linearTransformNode->SetAndObserveMatrixTransformToParent(matrix.GetPointer());
  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfModified() != 0 ||
      callback->GetNumberOfTransformModified() != 0)
    {
    std::cerr << "vtkMRMLLinearTransformNode::SetAndObserveMatrixTransformToParent failed (2)."
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: " << callback->GetNumberOfModified() << " "
              << "Number of TransformModifiedEvent: " << callback->GetNumberOfTransformModified()
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->ResetNumberOfEventsVariables();

  // Update matrix
  matrix->Modified();
  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfModified() != 0 ||
      callback->GetNumberOfTransformModified() != 1)
    {
    std::cerr << "vtkMatrix4x4::Modified failed."
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: " << callback->GetNumberOfModified() << " "
              << "Number of TransformModifiedEvent: " << callback->GetNumberOfTransformModified()
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->ResetNumberOfEventsVariables();

  return EXIT_SUCCESS;
}

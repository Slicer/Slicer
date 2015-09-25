/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLModelDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
// MRML includes
#include "vtkMRMLModelDisplayNode.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkAssignAttribute.h>
#include <vtkCommand.h>
#include <vtkObjectFactory.h>
#include <vtkPassThrough.h>
#include <vtkPolyData.h>
#include <vtkVersion.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLModelDisplayNode);

//-----------------------------------------------------------------------------
vtkMRMLModelDisplayNode::vtkMRMLModelDisplayNode()
{
  this->PassThrough = vtkPassThrough::New();
  this->AssignAttribute = vtkAssignAttribute::New();

  // the default behavior for models is to use the scalar range of the data
  // to reset the display scalar range, so use the Data flag
  this->SetScalarRangeFlag(vtkMRMLDisplayNode::UseDataScalarRange);

  // Be careful, virtualization doesn't work in constructors
  this->UpdatePolyDataPipeline();
}

//-----------------------------------------------------------------------------
vtkMRMLModelDisplayNode::~vtkMRMLModelDisplayNode()
{
  this->PassThrough->Delete();
  this->AssignAttribute->Delete();
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::ProcessMRMLEvents(vtkObject *caller,
                                                unsigned long event,
                                                void *callData )
{
  this->Superclass::ProcessMRMLEvents(caller, event, callData);

  if (event == vtkCommand::ModifiedEvent)
    {
    this->UpdatePolyDataPipeline();
    }
}

//---------------------------------------------------------------------------
#if (VTK_MAJOR_VERSION <= 5)
void vtkMRMLModelDisplayNode::SetInputPolyData(vtkPolyData* polyData)
{
  if (this->GetInputPolyData() == polyData)
    {
    return;
    }
  this->SetInputToPolyDataPipeline(polyData);
  this->Modified();
}
#else
void vtkMRMLModelDisplayNode
::SetInputPolyDataConnection(vtkAlgorithmOutput* polyDataConnection)
{
  if (this->GetInputPolyDataConnection() == polyDataConnection)
    {
    return;
    }
  this->SetInputToPolyDataPipeline(polyDataConnection);
  this->Modified();
}
#endif

//---------------------------------------------------------------------------
#if (VTK_MAJOR_VERSION <= 5)
void vtkMRMLModelDisplayNode::SetInputToPolyDataPipeline(vtkPolyData* polyData)
{
  this->PassThrough->SetInput(polyData);
  this->AssignAttribute->SetInput(polyData);
}
#else
void vtkMRMLModelDisplayNode
::SetInputToPolyDataPipeline(vtkAlgorithmOutput* polyDataConnection)
{
  this->PassThrough->SetInputConnection(polyDataConnection);
  this->AssignAttribute->SetInputConnection(polyDataConnection);
}
#endif

//---------------------------------------------------------------------------
vtkPolyData* vtkMRMLModelDisplayNode::GetInputPolyData()
{
  return vtkPolyData::SafeDownCast(this->AssignAttribute->GetInput());
}

#if (VTK_MAJOR_VERSION > 5)
//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLModelDisplayNode::GetInputPolyDataConnection()
{
  return this->AssignAttribute->GetNumberOfInputConnections(0) ?
    this->AssignAttribute->GetInputConnection(0,0) : 0;
}
#endif

//---------------------------------------------------------------------------
vtkPolyData* vtkMRMLModelDisplayNode::GetOutputPolyData()
{
  if (!this->GetOutputPolyDataConnection())
    {
    return 0;
    }
  if (!this->GetInputPolyData())
    {
    return 0;
    }
  return vtkPolyData::SafeDownCast(
    this->GetOutputPolyDataConnection()->GetProducer()->GetOutputDataObject(
      this->GetOutputPolyDataConnection()->GetIndex()));
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLModelDisplayNode::GetOutputPolyDataConnection()
{
  if (this->GetActiveScalarName())
    {
    return this->AssignAttribute->GetOutputPort();
    }
  else
    {
    return this->PassThrough->GetOutputPort();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::SetActiveScalarName(const char *scalarName)
{
  if (scalarName && this->ActiveScalarName && !strcmp(scalarName, this->ActiveScalarName))
    {
    return;
    }
  if (scalarName == 0 && this->ActiveScalarName == 0)
    {
    return;
    }
  int wasModifying = this->StartModify();
  this->Superclass::SetActiveScalarName(scalarName);
  this->UpdatePolyDataPipeline();
  this->EndModify(wasModifying);
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::SetActiveAttributeLocation(int location)
{
  if (location == this->ActiveAttributeLocation)
    {
    return;
    }
  int wasModifying = this->StartModify();
  this->Superclass::SetActiveAttributeLocation(location);
  this->UpdatePolyDataPipeline();
  this->EndModify(wasModifying);
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::UpdatePolyDataPipeline()
{
  this->AssignAttribute->Assign(
    this->GetActiveScalarName(),
    this->GetActiveScalarName() ? vtkDataSetAttributes::SCALARS : -1,
    this->GetActiveAttributeLocation());
  if (this->GetOutputPolyData())
    {
#if (VTK_MAJOR_VERSION <= 5)
    this->GetOutputPolyData()->Update();
#else
    this->GetOutputPolyDataConnection()->GetProducer()->Update();
#endif
    if (this->GetAutoScalarRange())
      {
      vtkDebugMacro("UpdatePolyDataPipeline: Auto flag is on, resetting scalar range!");
      this->SetScalarRange(this->GetOutputPolyData()->GetScalarRange());
      }
    }
}

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
#include "vtkMRMLColorNode.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkAssignAttribute.h>
#include <vtkCellData.h>
#include <vtkCommand.h>
#include <vtkLookupTable.h>
#include <vtkObjectFactory.h>
#include <vtkPassThrough.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkPolyData.h>
#include <vtkThreshold.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVersion.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLModelDisplayNode);

//-----------------------------------------------------------------------------
vtkMRMLModelDisplayNode::vtkMRMLModelDisplayNode()
{
  this->PassThrough = vtkPassThrough::New();
  this->AssignAttribute = vtkAssignAttribute::New();
  this->ThresholdFilter = vtkThreshold::New();
  this->ThresholdEnabled = false;

  // the default behavior for models is to use the scalar range of the data
  // to reset the display scalar range, so use the Data flag
  this->SetScalarRangeFlag(vtkMRMLDisplayNode::UseDataScalarRange);
}

//-----------------------------------------------------------------------------
vtkMRMLModelDisplayNode::~vtkMRMLModelDisplayNode()
{
  this->PassThrough->Delete();
  this->AssignAttribute->Delete();
  this->ThresholdFilter->Delete();
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::ProcessMRMLEvents(vtkObject *caller,
                                                unsigned long event,
                                                void *callData )
{
  this->Superclass::ProcessMRMLEvents(caller, event, callData);

  if (event == vtkCommand::ModifiedEvent)
    {
    this->UpdateScalarRange();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayNode
::SetInputMeshConnection(vtkAlgorithmOutput* meshConnection)
{
  if (this->GetInputMeshConnection() == meshConnection)
    {
    return;
    }
  this->SetInputToMeshPipeline(meshConnection);
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayNode
::SetInputPolyDataConnection(vtkAlgorithmOutput* polyDataConnection)
{
  // Wrapping `SetInputMeshConnection` for backward compatibility
  this->SetInputMeshConnection(polyDataConnection);
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayNode
::SetInputToMeshPipeline(vtkAlgorithmOutput* meshConnection)
{
  this->PassThrough->SetInputConnection(meshConnection);
  this->AssignAttribute->SetInputConnection(meshConnection);
  this->ThresholdFilter->SetInputConnection(this->AssignAttribute->GetOutputPort());

  this->Modified();
}

//---------------------------------------------------------------------------
vtkPointSet* vtkMRMLModelDisplayNode::GetInputMesh()
{
  return vtkPointSet::SafeDownCast(this->AssignAttribute->GetInput());
}

//---------------------------------------------------------------------------
vtkPolyData* vtkMRMLModelDisplayNode::GetInputPolyData()
{
  return vtkPolyData::SafeDownCast(this->GetInputMesh());
}

//---------------------------------------------------------------------------
vtkUnstructuredGrid* vtkMRMLModelDisplayNode::GetInputUnstructuredGrid()
{
  return vtkUnstructuredGrid::SafeDownCast(this->GetInputMesh());
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLModelDisplayNode::GetInputMeshConnection()
{
  return this->AssignAttribute->GetNumberOfInputConnections(0) ?
    this->AssignAttribute->GetInputConnection(0,0) : 0;
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLModelDisplayNode::GetInputPolyDataConnection()
{
  // Wrapping `GetInputMeshConnection` for backward compatibility
  return this->GetInputMeshConnection();
}

//---------------------------------------------------------------------------
vtkPointSet* vtkMRMLModelDisplayNode::GetOutputMesh()
{
  if (!this->GetInputMeshConnection())
    {
    return NULL;
    }

  vtkAlgorithmOutput* outputConnection = this->GetOutputMeshConnection();
  vtkAlgorithm* producer =  outputConnection? outputConnection->GetProducer() : 0;
  producer->Update();
  return vtkPointSet::SafeDownCast(
    producer ? producer->GetOutputDataObject(outputConnection->GetIndex()) : 0);
}

//---------------------------------------------------------------------------
vtkPolyData* vtkMRMLModelDisplayNode::GetOutputPolyData()
{
  return vtkPolyData::SafeDownCast(this->GetOutputMesh());
}

//---------------------------------------------------------------------------
vtkUnstructuredGrid* vtkMRMLModelDisplayNode::GetOutputUnstructuredGrid()
{
  return vtkUnstructuredGrid::SafeDownCast(this->GetOutputMesh());
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLModelDisplayNode::GetOutputMeshConnection()
{
  if (this->GetActiveScalarName() &&
      this->GetScalarVisibility() && // do not threshold if scalars hidden
      this->ThresholdEnabled)
    {
    return this->ThresholdFilter->GetOutputPort();
    }
  else if (this->GetActiveScalarName())
    {
    return this->AssignAttribute->GetOutputPort();
    }
  else
    {
    return this->PassThrough->GetOutputPort();
    }
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLModelDisplayNode::GetOutputPolyDataConnection()
{
  vtkWarningMacro("vtkMRMLModelDisplayNode::GetOutputPolyDataConnection is "
                  << "deprecated. Favor GetOutputMeshConnection().");
  return this->GetOutputMeshConnection();
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::SetThresholdRange(double min, double max)
{
  vtkMTimeType mtime = this->ThresholdFilter->GetMTime();
  this->ThresholdFilter->ThresholdBetween(min, max);
  if (this->ThresholdFilter->GetMTime() > mtime)
    {
    this->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::SetThresholdRange(double range[2])
{
  this->SetThresholdRange(range[0], range[1]);
}

//---------------------------------------------------------------------------
double vtkMRMLModelDisplayNode::GetThresholdMin()
{
  return this->ThresholdFilter->GetLowerThreshold();
}

//---------------------------------------------------------------------------
double vtkMRMLModelDisplayNode::GetThresholdMax()
{
  return this->ThresholdFilter->GetUpperThreshold();
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::GetThresholdRange(double range[2])
{
  range[0] = this->GetThresholdMin();
  range[1] = this->GetThresholdMax();
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
  this->UpdateAssignedAttribute();
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
  this->UpdateAssignedAttribute();
  this->EndModify(wasModifying);
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::SetScalarRangeFlag(int flag)
{
  if (flag == this->ScalarRangeFlag)
    {
    return;
    }
  int wasModifying = this->StartModify();
  this->Superclass::SetScalarRangeFlag(flag);
  this->UpdateScalarRange();
  this->EndModify(wasModifying);
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::UpdateAssignedAttribute()
{
  this->AssignAttribute->Assign(
    this->GetActiveScalarName(),
    this->GetActiveScalarName() ? vtkDataSetAttributes::SCALARS : -1,
    this->GetActiveAttributeLocation());

  if (this->GetScalarRangeFlag() != vtkMRMLDisplayNode::UseManualScalarRange)
    {
    this->UpdateScalarRange();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::UpdateScalarRange()
{
  if (!this->GetInputMesh())
    {
    return;
    }

  int flag = this->GetScalarRangeFlag();
  if (flag == vtkMRMLDisplayNode::UseDataScalarRange)
    {
    if (this->GetActiveScalarName())
      {
      vtkDataArray *dataArray = this->GetActiveScalarArray();
      if (dataArray)
        {
        this->SetScalarRange(dataArray->GetRange());
        }
      else
        {
        vtkErrorMacro("Can not use data scalar range: the output of the "
                      << "AssignAttribute filter is not a valid vtkPointSet.");
        }
      }
    }
  else if (flag == vtkMRMLDisplayNode::UseColorNodeScalarRange)
    {
    if (!this->GetColorNode())
      {
      vtkWarningMacro("Can not use color node scalar range since model "
                      << "display node does not have a color node.");
      }
    else if (vtkLookupTable* lut = this->GetColorNode()->GetLookupTable())
      {
      this->SetScalarRange(lut->GetRange());
      }
    else
      {
      vtkWarningMacro("Can not use color node scalar range since model "
                      << "display node color node does not have a lookup table.");
      }
    }
  else if (flag == vtkMRMLDisplayNode::UseDataTypeScalarRange)
    {
    vtkDataArray *dataArray = this->GetActiveScalarArray();
    if (dataArray)
      {
      this->SetScalarRange(dataArray->GetDataTypeMin(), dataArray->GetDataTypeMax());
      }
    else
      {
      vtkWarningMacro("Can not use data type scalar range since the model display node's"
                      << "mesh does not have an active scalar array.");
      }
    }
}

//-----------------------------------------------------------
vtkDataArray* vtkMRMLModelDisplayNode::GetActiveScalarArray()
{
  if (this->GetActiveScalarName() == NULL || strcmp(this->GetActiveScalarName(),"") == 0)
    {
    return NULL;
    }

  // Use output of AssignAttribute instead of this->GetOutputMesh()
  // since the data scalar range should not be retrieved from a
  // thresholded mesh even when thresholding is enabled.
  // Need to call Update() since we need to use GetOutput() on the
  // AssignAttribuet filter to retrieve its output mesh scalar range.
  this->AssignAttribute->Update();
  vtkPointSet* mesh = vtkPointSet::SafeDownCast(this->AssignAttribute->GetOutput());
  if (mesh == NULL)
    {
    return NULL;
    }

  vtkDataSetAttributes* attributes = NULL;
  switch (this->GetActiveAttributeLocation())
    {
    case vtkAssignAttribute::POINT_DATA:
      attributes = mesh->GetPointData();
      break;
    case vtkAssignAttribute::CELL_DATA:
      attributes = mesh->GetCellData();
      break;
    default:
      vtkWarningMacro("vtkMRMLModelDisplayNode::GetActiveScalarArray failed: unsupported attribute location: "
        << this->GetActiveAttributeLocation());
      break;
    }
  if (attributes == NULL)
    {
    return NULL;
    }
  vtkDataArray *dataArray = attributes->GetArray(this->GetActiveScalarName());
  return dataArray;
}

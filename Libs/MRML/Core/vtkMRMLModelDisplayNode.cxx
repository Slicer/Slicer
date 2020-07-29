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
#include "vtkMRMLModelNode.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkAssignAttribute.h>
#include <vtkCellData.h>
#include <vtkCommand.h>
#include <vtkGeometryFilter.h>
#include <vtkIntArray.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
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

static const char* SliceDistanceEncodedProjectionColorNodeReferenceRole = "distanceEncodedProjectionColor";

//-----------------------------------------------------------------------------
vtkMRMLModelDisplayNode::vtkMRMLModelDisplayNode()
{
  this->PassThrough = vtkPassThrough::New();
  this->AssignAttribute = vtkAssignAttribute::New();
  this->ThresholdFilter = vtkThreshold::New();
  this->ThresholdFilter->ThresholdBetween(0.0, -1.0); // indicates uninitialized
  this->ThresholdRangeTemp[0] = 0.0;
  this->ThresholdRangeTemp[1] = -1.0;
  this->ConvertToPolyDataFilter = vtkGeometryFilter::New();
  this->ThresholdEnabled = false;
  this->SliceDisplayMode = SliceDisplayIntersection;
  this->BackfaceCulling = 0;

  // Backface color has slightly different hue and less saturated compared to frontface
  this->BackfaceColorHSVOffset[0] = -0.05;
  this->BackfaceColorHSVOffset[1] = -0.1;
  this->BackfaceColorHSVOffset[2] = 0.0;

  // the default behavior for models is to use the scalar range of the data
  // to reset the display scalar range, so use the Data flag
  this->SetScalarRangeFlag(vtkMRMLDisplayNode::UseDataScalarRange);

  this->ThresholdFilter->SetInputConnection(this->AssignAttribute->GetOutputPort());
  this->ConvertToPolyDataFilter->SetInputConnection(this->ThresholdFilter->GetOutputPort());

  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  this->AddNodeReferenceRole(SliceDistanceEncodedProjectionColorNodeReferenceRole, nullptr, events.GetPointer());
}

//-----------------------------------------------------------------------------
vtkMRMLModelDisplayNode::~vtkMRMLModelDisplayNode()
{
  this->PassThrough->Delete();
  this->AssignAttribute->Delete();
  this->ThresholdFilter->Delete();
  this->ConvertToPolyDataFilter->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(SliceDisplayMode);
  vtkMRMLPrintBooleanMacro(ThresholdEnabled);
  vtkMRMLPrintVectorMacro(ThresholdRange, double, 2);
  vtkMRMLPrintVectorMacro(BackfaceColorHSVOffset, double, 3);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLEnumMacro(sliceDisplayMode, SliceDisplayMode);
  vtkMRMLWriteXMLBooleanMacro(thresholdEnabled, ThresholdEnabled);
  vtkMRMLWriteXMLVectorMacro(thresholdRange, ThresholdRange, double, 2);
  vtkMRMLWriteXMLVectorMacro(backfaceColorHSVOffset, BackfaceColorHSVOffset, double, 3);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  this->Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLEnumMacro(sliceDisplayMode, SliceDisplayMode);
  vtkMRMLReadXMLBooleanMacro(thresholdEnabled, ThresholdEnabled);
  vtkMRMLReadXMLVectorMacro(thresholdRange, ThresholdRange, double, 2);
  vtkMRMLReadXMLVectorMacro(backfaceColorHSVOffset, BackfaceColorHSVOffset, double, 3);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLModelDisplayNode* node = vtkMRMLModelDisplayNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyEnumMacro(SliceDisplayMode);
  vtkMRMLCopyBooleanMacro(ThresholdEnabled);
  vtkMRMLCopyVectorMacro(ThresholdRange, double, 2);
  vtkMRMLCopyVectorMacro(BackfaceColorHSVOffset, double, 3);
  vtkMRMLCopyEndMacro();
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::ProcessMRMLEvents(vtkObject *caller,
                                                unsigned long event,
                                                void *callData )
{
  this->Superclass::ProcessMRMLEvents(caller, event, callData);

  vtkMRMLColorNode* cnode = vtkMRMLColorNode::SafeDownCast(caller);
  if (cnode != nullptr && cnode == this->GetDistanceEncodedProjectionColorNode()
    && event == vtkCommand::ModifiedEvent)
    {
    // Slice distance encoded projection color node changed
    this->InvokeEvent(vtkCommand::ModifiedEvent, nullptr);
    }
  else if (event == vtkCommand::ModifiedEvent)
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
    this->AssignAttribute->GetInputConnection(0,0) : nullptr;
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
    return nullptr;
    }

  vtkAlgorithmOutput* outputConnection = this->GetOutputMeshConnection();
  if (!outputConnection)
    {
    return nullptr;
    }

  vtkAlgorithm* producer = outputConnection->GetProducer();
  if (!producer)
    {
    return nullptr;
    }

  producer->Update();
  return vtkPointSet::SafeDownCast(
           producer->GetOutputDataObject(outputConnection->GetIndex()));
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
    vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(this->GetDisplayableNode());
    if (modelNode && modelNode->GetMeshType() == vtkMRMLModelNode::PolyDataMeshType)
      {
      // Threshold filter generates unstructured grid output. If input is a polydata mesh
      // then the pipeline expects polydata as output mesh, therefore we need to use
      // ConvertToPolyDataFilter output.
      return this->ConvertToPolyDataFilter->GetOutputPort();
      }
    else
      {
      return this->ThresholdFilter->GetOutputPort();
      }
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
void vtkMRMLModelDisplayNode::SetThresholdEnabled(bool enabled)
{
  if (this->ThresholdEnabled == enabled)
    {
    return;
    }

  int wasModified = this->StartModify();

  this->ThresholdEnabled = enabled;
  this->Modified();

  // initialize threshold range if it has not been initialized yet
  if (enabled && this->GetThresholdMin() > this->GetThresholdMax())
    {
    double dataRange[2] = { 0.0, -1.0 };
    vtkDataArray *dataArray = this->GetActiveScalarArray();
    if (dataArray)
      {
      dataArray->GetRange(dataRange);
      }
    if (dataRange[0] <= dataRange[1])
      {
      this->SetThresholdRange(dataRange);
      }
    }

  this->EndModify(wasModified);
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
double* vtkMRMLModelDisplayNode::GetThresholdRange()
{
  this->GetThresholdRange(this->ThresholdRangeTemp);
  return this->ThresholdRangeTemp;
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::SetActiveScalarName(const char *scalarName)
{
  if (scalarName && this->ActiveScalarName && !strcmp(scalarName, this->ActiveScalarName))
    {
    return;
    }
  if (scalarName == nullptr && this->ActiveScalarName == nullptr)
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
void vtkMRMLModelDisplayNode::SetActiveScalar(const char *scalarName, int location)
{
  if (location == this->ActiveAttributeLocation
    && ((scalarName && this->ActiveScalarName && !strcmp(scalarName, this->ActiveScalarName))
        || (scalarName == nullptr && this->ActiveScalarName == nullptr)))
    {
    // no change
    return;
    }
  int wasModifying = this->StartModify();
  this->Superclass::SetActiveScalarName(scalarName);
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
    vtkDataSetAttributes::SCALARS,
    this->GetActiveAttributeLocation() >= 0 ? this->GetActiveAttributeLocation() : vtkAssignAttribute::POINT_DATA);

  if (this->GetActiveAttributeLocation() == vtkAssignAttribute::POINT_DATA)
    {
    this->ThresholdFilter->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::SCALARS);
    }
  else if (this->GetActiveAttributeLocation() == vtkAssignAttribute::CELL_DATA)
    {
    this->ThresholdFilter->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, vtkDataSetAttributes::SCALARS);
    }

  this->UpdateScalarRange();
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::UpdateScalarRange()
{
  if (!this->GetInputMesh())
    {
    return;
    }

  if (this->GetScalarRangeFlag() == vtkMRMLDisplayNode::UseManualScalarRange)
    {
    return;
    }

  double newScalarRange[2] = { 0.0, -1.0 };
  int flag = this->GetScalarRangeFlag();
  if (flag == vtkMRMLDisplayNode::UseDataScalarRange)
    {
    vtkDataArray *dataArray = this->GetActiveScalarArray();
    if (dataArray)
      {
      dataArray->GetRange(newScalarRange);
      }
    }
  else if (flag == vtkMRMLDisplayNode::UseColorNodeScalarRange)
    {
    if (this->GetColorNode())
      {
      vtkLookupTable* lut = this->GetColorNode()->GetLookupTable();
      if (lut)
        {
        double* lutRange = lut->GetRange();
        newScalarRange[0] = lutRange[0];
        newScalarRange[1] = lutRange[1];
        }
      else
        {
        vtkWarningMacro("Can not use color node scalar range since model "
                        << "display node color node does not have a lookup table.");
        }
      }
    else
      {
      vtkWarningMacro("Can not use color node scalar range since model "
                      << "display node does not have a color node.");
      }
    }
  else if (flag == vtkMRMLDisplayNode::UseDataTypeScalarRange)
    {
    vtkDataArray *dataArray = this->GetActiveScalarArray();
    if (dataArray)
      {
      newScalarRange[0] = dataArray->GetDataTypeMin();
      newScalarRange[1] = dataArray->GetDataTypeMax();
      }
    else
      {
      vtkWarningMacro("Can not use data type scalar range since the model display node's"
                      << "mesh does not have an active scalar array.");
      }
    }

  this->SetScalarRange(newScalarRange);
}

//-----------------------------------------------------------
vtkDataArray* vtkMRMLModelDisplayNode::GetActiveScalarArray()
{
  if (this->GetActiveScalarName() == nullptr || strcmp(this->GetActiveScalarName(),"") == 0)
    {
    return nullptr;
    }
  if (!this->GetInputMesh())
    {
    return nullptr;
    }

  // Use output of AssignAttribute instead of this->GetOutputMesh()
  // since the data scalar range should not be retrieved from a
  // thresholded mesh even when thresholding is enabled.
  // Need to call Update() since we need to use GetOutput() on the
  // AssignAttribuet filter to retrieve its output mesh scalar range.
  this->AssignAttribute->Update();
  vtkPointSet* mesh = vtkPointSet::SafeDownCast(this->AssignAttribute->GetOutput());
  if (mesh == nullptr)
    {
    return nullptr;
    }

  vtkDataSetAttributes* attributes = nullptr;
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
  if (attributes == nullptr)
    {
    return nullptr;
    }
  vtkDataArray *dataArray = attributes->GetArray(this->GetActiveScalarName());
  return dataArray;
}

//-----------------------------------------------------------
void vtkMRMLModelDisplayNode::SetSliceDisplayModeToIntersection()
{
  this->SetSliceDisplayMode(SliceDisplayIntersection);
};

//-----------------------------------------------------------
void vtkMRMLModelDisplayNode::SetSliceDisplayModeToProjection()
{
  this->SetSliceDisplayMode(SliceDisplayProjection);
};

//-----------------------------------------------------------
void vtkMRMLModelDisplayNode::SetSliceDisplayModeToDistanceEncodedProjection()
{
  this->SetSliceDisplayMode(SliceDisplayDistanceEncodedProjection);
};

//-----------------------------------------------------------
const char* vtkMRMLModelDisplayNode::GetSliceDisplayModeAsString(int id)
{
  switch (id)
    {
    case SliceDisplayIntersection: return "intersection";
    case SliceDisplayProjection: return "projection";
    case SliceDisplayDistanceEncodedProjection: return "distanceEncodedProjection";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLModelDisplayNode::GetSliceDisplayModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i<SliceDisplayMode_Last; i++)
    {
    if (strcmp(name, GetSliceDisplayModeAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
void vtkMRMLModelDisplayNode::SetAndObserveDistanceEncodedProjectionColorNodeID(const char *colorNodeID)
{
  this->SetAndObserveNodeReferenceID(SliceDistanceEncodedProjectionColorNodeReferenceRole, colorNodeID);
}

//-----------------------------------------------------------
const char* vtkMRMLModelDisplayNode::GetDistanceEncodedProjectionColorNodeID()
{
  return this->GetNodeReferenceID(SliceDistanceEncodedProjectionColorNodeReferenceRole);
}

//-----------------------------------------------------------
vtkMRMLColorNode* vtkMRMLModelDisplayNode::GetDistanceEncodedProjectionColorNode()
{
  return vtkMRMLColorNode::SafeDownCast(this->GetNodeReference(SliceDistanceEncodedProjectionColorNodeReferenceRole));
}

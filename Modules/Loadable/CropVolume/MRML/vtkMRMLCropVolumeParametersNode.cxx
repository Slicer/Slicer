/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

// VTK includes
#include <vtkCommand.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// MRML includes
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLTransformNode.h"

// CropModuleMRML includes
#include "vtkMRMLCropVolumeParametersNode.h"

// AnnotationModuleMRML includes
#include "vtkMRMLAnnotationROINode.h"

// STD includes

static const char* InputVolumeNodeReferenceRole = "inputVolume";
static const char* InputVolumeNodeReferenceMRMLAttributeName = "inputVolumeNodeID";
static const char* OutputVolumeNodeReferenceRole = "outputVolume";
static const char* OutputVolumeNodeReferenceMRMLAttributeName = "outputVolumeNodeID";
static const char* ROINodeReferenceRole = "roi";
static const char* ROINodeReferenceMRMLAttributeName = "ROINodeID";
static const char* ROIAlignmentTransformNodeReferenceRole = "roiAlignmentTransform";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLCropVolumeParametersNode);

//----------------------------------------------------------------------------
vtkMRMLCropVolumeParametersNode::vtkMRMLCropVolumeParametersNode()
{
  this->HideFromEditors = 1;

  vtkNew<vtkIntArray> inputVolumeEvents;
  inputVolumeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  inputVolumeEvents->InsertNextValue(vtkMRMLVolumeNode::ImageDataModifiedEvent);
  this->AddNodeReferenceRole(InputVolumeNodeReferenceRole,
    InputVolumeNodeReferenceMRMLAttributeName,
    inputVolumeEvents.GetPointer());

  vtkNew<vtkIntArray> roiEvents;
  roiEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  this->AddNodeReferenceRole(ROINodeReferenceRole,
    ROINodeReferenceMRMLAttributeName,
    roiEvents.GetPointer());

  this->AddNodeReferenceRole(OutputVolumeNodeReferenceRole,
    OutputVolumeNodeReferenceMRMLAttributeName);

  this->VoxelBased = false;
  this->InterpolationMode = vtkMRMLCropVolumeParametersNode::InterpolationLinear;
  this->IsotropicResampling = false;
  this->SpacingScalingConst = 1.;
  this->FillValue = 0.;
}

//----------------------------------------------------------------------------
vtkMRMLCropVolumeParametersNode::~vtkMRMLCropVolumeParametersNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::ReadXMLAttributes(const char** atts)
{
  // Read all MRML node attributes from two arrays of names and values
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLBooleanMacro(voxelBased, VoxelBased);
  vtkMRMLReadXMLIntMacro(interpolationMode, InterpolationMode);
  vtkMRMLReadXMLBooleanMacro(isotropicResampling, IsotropicResampling);
  vtkMRMLReadXMLFloatMacro(spaceScalingConst, SpacingScalingConst);
  vtkMRMLReadXMLFloatMacro(fillValue, FillValue);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLBooleanMacro(voxelBased, VoxelBased);
  vtkMRMLWriteXMLIntMacro(interpolationMode, InterpolationMode);
  vtkMRMLWriteXMLBooleanMacro(isotropicResampling, IsotropicResampling);
  vtkMRMLWriteXMLFloatMacro(spaceScalingConst, SpacingScalingConst);
  vtkMRMLWriteXMLFloatMacro(fillValue, FillValue);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyBooleanMacro(VoxelBased);
  vtkMRMLCopyIntMacro(InterpolationMode);
  vtkMRMLCopyBooleanMacro(IsotropicResampling);
  vtkMRMLCopyFloatMacro(SpacingScalingConst);
  vtkMRMLCopyFloatMacro(FillValue);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintBooleanMacro(VoxelBased);
  vtkMRMLPrintIntMacro(InterpolationMode);
  vtkMRMLPrintBooleanMacro(IsotropicResampling);
  vtkMRMLPrintFloatMacro(SpacingScalingConst);
  vtkMRMLPrintFloatMacro(FillValue);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::SetInputVolumeNodeID(const char *nodeID)
{
  this->SetNodeReferenceID(InputVolumeNodeReferenceRole, nodeID);
}

//----------------------------------------------------------------------------
const char * vtkMRMLCropVolumeParametersNode::GetInputVolumeNodeID()
{
  return this->GetNodeReferenceID(InputVolumeNodeReferenceRole);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLCropVolumeParametersNode::GetInputVolumeNode()
{
  return vtkMRMLVolumeNode::SafeDownCast(this->GetNodeReference(InputVolumeNodeReferenceRole));
}

//----------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::SetOutputVolumeNodeID(const char *nodeID)
{
  this->SetNodeReferenceID(OutputVolumeNodeReferenceRole, nodeID);
}

//----------------------------------------------------------------------------
const char * vtkMRMLCropVolumeParametersNode::GetOutputVolumeNodeID()
{
  return this->GetNodeReferenceID(OutputVolumeNodeReferenceRole);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLCropVolumeParametersNode::GetOutputVolumeNode()
{
  return vtkMRMLVolumeNode::SafeDownCast(this->GetNodeReference(OutputVolumeNodeReferenceRole));
}

//----------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::SetROINodeID(const char *nodeID)
{
  this->SetNodeReferenceID(ROINodeReferenceRole, nodeID);
}

//----------------------------------------------------------------------------
const char * vtkMRMLCropVolumeParametersNode::GetROINodeID()
{
  return this->GetNodeReferenceID(ROINodeReferenceRole);
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationROINode* vtkMRMLCropVolumeParametersNode::GetROINode()
{
  return vtkMRMLAnnotationROINode::SafeDownCast(this->GetNodeReference(ROINodeReferenceRole));
}

//----------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::SetROIAlignmentTransformNodeID(const char *nodeID)
{
  this->SetNodeReferenceID(ROIAlignmentTransformNodeReferenceRole, nodeID);
}

//----------------------------------------------------------------------------
const char * vtkMRMLCropVolumeParametersNode::GetROIAlignmentTransformNodeID()
{
  return this->GetNodeReferenceID(ROIAlignmentTransformNodeReferenceRole);
}

//----------------------------------------------------------------------------
vtkMRMLTransformNode* vtkMRMLCropVolumeParametersNode::GetROIAlignmentTransformNode()
{
  return vtkMRMLTransformNode::SafeDownCast(this->GetNodeReference(ROIAlignmentTransformNodeReferenceRole));
}

//----------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::DeleteROIAlignmentTransformNode()
{
  vtkMRMLTransformNode* transformNode = this->GetROIAlignmentTransformNode();
  if (transformNode)
    {
    this->SetROIAlignmentTransformNodeID(nullptr);
    if (this->GetScene())
      {
      this->GetScene()->RemoveNode(transformNode);
      }
    }
}

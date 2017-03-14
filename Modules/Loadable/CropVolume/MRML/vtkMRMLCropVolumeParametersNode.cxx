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
}

//----------------------------------------------------------------------------
vtkMRMLCropVolumeParametersNode::~vtkMRMLCropVolumeParametersNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
  {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName,"VoxelBased"))
      {
      if (!strcmp(attValue, "true"))
        {
        this->VoxelBased = true;
        }
      else
        {
        this->VoxelBased = false;
        }
      }
    else if (!strcmp(attName,"interpolationMode"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->InterpolationMode;
      }
    else if (!strcmp(attName, "isotropicResampling"))
      {
      if (!strcmp(attValue, "true"))
        {
        this->IsotropicResampling = true;
        }
      else
        {
        this->IsotropicResampling = false;
        }
      }
    else if (!strcmp(attName, "spaceScalingConst"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->SpacingScalingConst;
      }
  }
}

//----------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " voxelBased=\"" << (this->VoxelBased ? "true" : "false") << "\"";
  of << indent << " interpolationMode=\"" << this->InterpolationMode << "\"";
  of << indent << " isotropicResampling=\"" << (this->IsotropicResampling ? "true" : "false") << "\"";
  of << indent << " spaceScalingConst=\"" << this->SpacingScalingConst << "\"";
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLCropVolumeParametersNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();
  
  Superclass::Copy(anode);
  vtkMRMLCropVolumeParametersNode *node = vtkMRMLCropVolumeParametersNode::SafeDownCast(anode);

  this->SetVoxelBased(node->GetVoxelBased());
  this->SetInterpolationMode(node->GetInterpolationMode());
  this->SetIsotropicResampling(node->GetIsotropicResampling());
  this->SetSpacingScalingConst(node->GetSpacingScalingConst());

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLCropVolumeParametersNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << "VoxelBased: " << (this->VoxelBased ? "true" : "false") << "\n";
  os << "InterpolationMode: " << this->InterpolationMode << "\n";
  os << "IsotropicResampling: " << (this->IsotropicResampling ? "true" : "false") << "\n";
  os << "SpacingScalingConst: " << this->SpacingScalingConst << "\n";
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
    this->SetROIAlignmentTransformNodeID(NULL);
    if (this->GetScene())
      {
      this->GetScene()->RemoveNode(transformNode);
      }
    }
}

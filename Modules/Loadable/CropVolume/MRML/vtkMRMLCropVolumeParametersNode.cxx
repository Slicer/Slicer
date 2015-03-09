/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLCropVolumeParametersNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// VTK includes
#include <vtkCommand.h>
#include <vtkObjectFactory.h>

// MRML includes
#include "vtkMRMLVolumeNode.h"

// CropModuleMRML includes
#include "vtkMRMLCropVolumeParametersNode.h"

// AnnotationModuleMRML includes
#include "vtkMRMLAnnotationROINode.h"

// STD includes

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLCropVolumeParametersNode);

//----------------------------------------------------------------------------
vtkMRMLCropVolumeParametersNode::vtkMRMLCropVolumeParametersNode()
{
  this->HideFromEditors = 1;

  this->InputVolumeNodeID = NULL;
  this->OutputVolumeNodeID = NULL;
  this->ROINodeID = NULL;

  this->ROIVisibility = true;
  this->VoxelBased = false;
  this->InterpolationMode = 2;
  this->IsotropicResampling = false;
  this->SpacingScalingConst = 1.;
}

//----------------------------------------------------------------------------
vtkMRMLCropVolumeParametersNode::~vtkMRMLCropVolumeParametersNode()
{
  if (this->InputVolumeNodeID)
    {
    delete [] this->InputVolumeNodeID;
    this->InputVolumeNodeID = NULL;
    }

  if (this->OutputVolumeNodeID)
    {
    delete [] this->OutputVolumeNodeID;
    this->OutputVolumeNodeID = NULL;
    }

  if (this->ROINodeID)
    {
    delete [] this->ROINodeID;
    this->ROINodeID = NULL;
    }
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
    if (!strcmp(attName, "inputVolumeNodeID"))
      {
      this->SetInputVolumeNodeID(attValue);
      }
    else if (!strcmp(attName, "outputVolumeNodeID"))
      {
      this->SetOutputVolumeNodeID(attValue);
      }
    else if (!strcmp(attName, "ROINodeID"))
      {
      this->SetROINodeID(attValue);
      }
    else if (!strcmp(attName, "ROIVisibility"))
      {
      if (!strcmp(attValue, "true") || !strcmp(attValue, "1"))
        {
        this->ROIVisibility = true;
        }
      else
        {
        this->ROIVisibility = false;
        }
      }
    else if (!strcmp(attName,"VoxelBased"))
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

  if (this->InputVolumeNodeID != NULL)
    {
    of << indent << " inputVolumeNodeID=\"" << this->InputVolumeNodeID << "\"";
    }
  if (this->OutputVolumeNodeID != NULL)
    {
    of << indent << " outputVolumeNodeID=\"" << this->OutputVolumeNodeID << "\"";
    }
  if (this->ROINodeID != NULL)
    {
    of << indent << " ROINodeID=\"" << this->ROINodeID << "\"";
    }

  of << indent << " ROIVisibility=\"" << (this->ROIVisibility ? "true" : "false") << "\"";
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

  this->SetInputVolumeNodeID(node->GetInputVolumeNodeID());
  this->SetOutputVolumeNodeID(node->GetOutputVolumeNodeID());
  this->SetROINodeID(node->GetROINodeID());


  this->SetROIVisibility(node->GetROIVisibility());
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

  os << "InputVolumeNodeID: " << ( (this->InputVolumeNodeID) ? this->InputVolumeNodeID : "None" ) << "\n";
  os << "OutputVolumeNodeID: " << ( (this->OutputVolumeNodeID) ? this->OutputVolumeNodeID : "None" ) << "\n";
  os << "ROINodeID: " << ( (this->ROINodeID) ? this->ROINodeID : "None" ) << "\n";

  os << "ROIVisibility: " << (this->ROIVisibility ? "true" : "false") << "\n";
  os << "VoxelBased: " << (this->VoxelBased ? "true" : "false") << "\n";
  os << "InterpolationMode: " << this->InterpolationMode << "\n";
  os << "IsotropicResampling: " << (this->IsotropicResampling ? "true" : "false") << "\n";
  os << "SpacingScalingConst: " << this->SpacingScalingConst << "\n";
}

// End

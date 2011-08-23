/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLDiffusionWeightedVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"

// VTK includes
#include <vtkImageAppendComponents.h>
#include <vtkImageData.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkImageThreshold.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLDiffusionWeightedVolumeDisplayNode* vtkMRMLDiffusionWeightedVolumeDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDiffusionWeightedVolumeDisplayNode");
  if(ret)
    {
    return (vtkMRMLDiffusionWeightedVolumeDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLDiffusionWeightedVolumeDisplayNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLDiffusionWeightedVolumeDisplayNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDiffusionWeightedVolumeDisplayNode");
  if(ret)
    {
    return (vtkMRMLDiffusionWeightedVolumeDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLDiffusionWeightedVolumeDisplayNode;
}

//----------------------------------------------------------------------------
vtkMRMLDiffusionWeightedVolumeDisplayNode::vtkMRMLDiffusionWeightedVolumeDisplayNode()
{
  // Strings
  this->DiffusionComponent = 0;
  this->ExtractComponent = vtkImageExtractComponents::New();
  this->Threshold->SetInput( this->ExtractComponent->GetOutput());
  this->MapToWindowLevelColors->SetInput( this->ExtractComponent->GetOutput());

}

//----------------------------------------------------------------------------
vtkMRMLDiffusionWeightedVolumeDisplayNode::~vtkMRMLDiffusionWeightedVolumeDisplayNode()
{
  this->ExtractComponent->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  std::stringstream ss;
  ss << this->DiffusionComponent;
  of << indent << " diffusionComponent=\"" << ss.str() << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "diffusionComponent")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->DiffusionComponent;
      }
    }  
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLDiffusionWeightedVolumeDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLDiffusionWeightedVolumeDisplayNode *node = (vtkMRMLDiffusionWeightedVolumeDisplayNode *) anode;

  this->SetDiffusionComponent(node->DiffusionComponent);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

  os << indent << "Diffusion Component:   " << this->DiffusionComponent << "\n";

}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeDisplayNode
::SetInputToImageDataPipeline(vtkImageData *imageData)
{
  this->ExtractComponent->SetInput(imageData);
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLDiffusionWeightedVolumeDisplayNode::GetInputImageData()
{
  return vtkImageData::SafeDownCast(this->ExtractComponent->GetInput());
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLDiffusionWeightedVolumeDisplayNode::GetOutputImageData()
{
  return this->AppendComponents->GetOutput();
}

//---------------------------------------------------------------------------
vtkImageData* vtkMRMLDiffusionWeightedVolumeDisplayNode::GetScalarImageData()
{
  return vtkImageData::SafeDownCast(this->ExtractComponent->GetOutput());
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionWeightedVolumeDisplayNode::UpdateImageDataPipeline()
{
  this->ExtractComponent->SetComponents(this->GetDiffusionComponent());
  this->Superclass::UpdateImageDataPipeline();
}

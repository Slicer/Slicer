/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLMultiVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLMultiVolumeDisplayNode.h"

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
vtkMRMLNodeNewMacro(vtkMRMLMultiVolumeDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLMultiVolumeDisplayNode::vtkMRMLMultiVolumeDisplayNode()
{
  // Strings
  this->FrameComponent = 0;
  this->ExtractComponent = vtkImageExtractComponents::New();
  this->Threshold->SetInput( this->ExtractComponent->GetOutput());
  this->MapToWindowLevelColors->SetInput( this->ExtractComponent->GetOutput());

}

//----------------------------------------------------------------------------
vtkMRMLMultiVolumeDisplayNode::~vtkMRMLMultiVolumeDisplayNode()
{
  this->ExtractComponent->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  std::stringstream ss;
  ss << this->FrameComponent;
  of << indent << " FrameComponent=\"" << ss.str() << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "FrameComponent")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->FrameComponent;
      }
    }  
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLMultiVolumeDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLMultiVolumeDisplayNode *node = (vtkMRMLMultiVolumeDisplayNode *) anode;

  this->SetFrameComponent(node->FrameComponent);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

  os << indent << "Frame Component:   " << this->FrameComponent << "\n";

}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeDisplayNode
::SetInputToImageDataPipeline(vtkImageData *imageData)
{
  this->ExtractComponent->SetInput(imageData);
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLMultiVolumeDisplayNode::GetInputImageData()
{
  return vtkImageData::SafeDownCast(this->ExtractComponent->GetInput());
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLMultiVolumeDisplayNode::GetOutputImageData()
{
  return this->AppendComponents->GetOutput();
}

//---------------------------------------------------------------------------
vtkImageData* vtkMRMLMultiVolumeDisplayNode::GetScalarImageData()
{
  return vtkImageData::SafeDownCast(this->ExtractComponent->GetOutput());
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeDisplayNode::UpdateImageDataPipeline()
{
  this->ExtractComponent->SetComponents(this->GetFrameComponent());
  this->Superclass::UpdateImageDataPipeline();
}

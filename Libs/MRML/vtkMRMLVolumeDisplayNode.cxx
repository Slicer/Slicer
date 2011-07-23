/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/



#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLVolumeNode.h"

// Initialize static member that controls resampling -- 
// old comment: "This offset will be changed to 0.5 from 0.0 per 2/8/2002 Slicer 
// development meeting, to move ijk coordinates to voxel centers."


//------------------------------------------------------------------------------
vtkMRMLVolumeDisplayNode* vtkMRMLVolumeDisplayNode::New()
{
  return NULL;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLVolumeDisplayNode::CreateNodeInstance()
{
  return NULL;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeDisplayNode::vtkMRMLVolumeDisplayNode()
{
  // try setting a default greyscale color map
  //this->SetDefaultColorMap(0);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeDisplayNode::~vtkMRMLVolumeDisplayNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLVolumeDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  //vtkMRMLVolumeDisplayNode *node = (vtkMRMLVolumeDisplayNode *) anode;

}

//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);
}

//-----------------------------------------------------------
void vtkMRMLVolumeDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);
}

//-----------------------------------------------------------
void vtkMRMLVolumeDisplayNode::UpdateReferences()
{
   Superclass::UpdateReferences();
}

//---------------------------------------------------------------------------
vtkImageData* vtkMRMLVolumeDisplayNode::GetImageData()
{
  if (!this->GetInputImageData())
    {
    return 0;
    }
  this->UpdateImageDataPipeline();
  return this->GetOutputImageData();
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode
::SetInputImageData(vtkImageData *vtkNotUsed(imageData))
{
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLVolumeDisplayNode::GetInputImageData()
{
  return NULL;
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLVolumeDisplayNode::GetOutputImageData()
{
  return NULL;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::UpdateImageDataPipeline()
{
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}


//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::SetDefaultColorMap()
{
  this->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLVolumeDisplayNode::GetVolumeNode()
{
  return vtkMRMLVolumeNode::SafeDownCast(this->GetDisplayableNode());
}

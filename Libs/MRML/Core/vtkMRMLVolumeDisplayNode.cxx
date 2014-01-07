/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLVolumeNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkImageData.h>

// Initialize static member that controls resampling -- 
// old comment: "This offset will be changed to 0.5 from 0.0 per 2/8/2002 Slicer 
// development meeting, to move ijk coordinates to voxel centers."

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

//---------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::ProcessMRMLEvents(vtkObject *caller,
                                                 unsigned long event,
                                                 void *callData)
{
  this->Superclass::ProcessMRMLEvents(caller, event, callData);
  if (event ==  vtkCommand::ModifiedEvent)
    {
    this->UpdateImageDataPipeline();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//-----------------------------------------------------------
void vtkMRMLVolumeDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
  this->Superclass::UpdateScene(scene);
}

//-----------------------------------------------------------
void vtkMRMLVolumeDisplayNode::GetBounds(double bounds[])
{
  this->Superclass::GetBounds(bounds);
  if (this->GetOutputImageData())
    {
    this->GetOutputImageData()->UpdateInformation();
    int wholeExtent[] = {0,0,0,0,0,0};
    this->GetOutputImageData()->GetWholeExtent(wholeExtent);
    if (wholeExtent[0] <= wholeExtent[1])
      {
//      int extentStart[3];
//      extentStart[0] = wholeExtent[0];
//      extentStart[1] = wholeExtent[2];
//      extentStart[2] = wholeExtent[4];
//      int extentEnd[3];
//      extentEnd[0] = wholeExtent[1]-1;
//      extentEnd[1] = wholeExtent[3]-1;
//      extentEnd[2] = wholeExtent[5]-1;
//      double startPoint[3];
//      this->GetOutputImageData()->GetPoint(
//            this->GetOutputImageData()->ComputePointId(extentStart),
//            startPoint);
//      double endPoint[3];
//      this->GetOutputImageData()->GetPoint(
//            this->GetOutputImageData()->ComputePointId(extentEnd),
//            endPoint);
      double origin[3];
      this->GetOutputImageData()->GetOrigin(origin);
      double spacing[3];
      this->GetOutputImageData()->GetSpacing(spacing);
      double startPoint[3];
      double endPoint[3];
      startPoint[0] = origin[0] + spacing[0]*wholeExtent[0];
      startPoint[1] = origin[1] + spacing[1]*wholeExtent[2];
      startPoint[2] = origin[2] + spacing[2]*wholeExtent[4];
      endPoint[0] = origin[0] + spacing[0]*wholeExtent[1];
      endPoint[1] = origin[1] + spacing[1]*wholeExtent[3];
      endPoint[2] = origin[2] + spacing[2]*wholeExtent[5];
      bounds[0] = std::min(startPoint[0], endPoint[0]);
      bounds[1] = std::max(startPoint[0], endPoint[0]);
      bounds[2] = std::min(startPoint[1], endPoint[1]);
      bounds[3] = std::max(startPoint[1], endPoint[1]);
      bounds[4] = std::min(startPoint[2], endPoint[2]);
      bounds[5] = std::max(startPoint[2], endPoint[2]);
      }
    }
}

//-----------------------------------------------------------
void vtkMRMLVolumeDisplayNode::UpdateReferences()
{
  this->Superclass::UpdateReferences();
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
::SetInputImageData(vtkImageData *imageData)
{
  this->SetInputToImageDataPipeline(imageData);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::SetInputToImageDataPipeline(vtkImageData *vtkNotUsed(imageData))
{
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLVolumeDisplayNode::GetInputImageData()
{
  return NULL;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::SetBackgroundImageData(vtkImageData* vtkNotUsed(imageData))
{
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLVolumeDisplayNode::GetBackgroundImageData()
{
  return 0;
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

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLVolumeDisplayNode::GetUpToDateImageData()
{
  vtkImageData* imageData = this->GetImageData();
  if (!imageData)
    {
    return NULL;
    }
  imageData->Update();
  return imageData;
}

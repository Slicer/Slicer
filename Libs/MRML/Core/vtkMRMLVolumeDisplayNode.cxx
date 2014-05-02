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
#include <vtkAlgorithmOutput.h>
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
void vtkMRMLVolumeDisplayNode::UpdateReferences()
{
  this->Superclass::UpdateReferences();
}

//---------------------------------------------------------------------------
#if (VTK_MAJOR_VERSION <= 5)
vtkImageData* vtkMRMLVolumeDisplayNode::GetImageData()
{
  if (!this->GetInputImageData())
    {
    return 0;
    }
  this->UpdateImageDataPipeline();
  return this->GetOutputImageData();
}
#else
vtkAlgorithmOutput* vtkMRMLVolumeDisplayNode::GetImageDataPort()
{
  if (!this->GetInputImageData())
    {
    return 0;
    }
  this->UpdateImageDataPipeline();
  return this->GetOutputImageDataPort();
}
#endif

//----------------------------------------------------------------------------
#if (VTK_MAJOR_VERSION <= 5)
void vtkMRMLVolumeDisplayNode
::SetInputImageData(vtkImageData *imageData)
{
  this->SetInputToImageDataPipeline(imageData);
  this->Modified();
}
#else
void vtkMRMLVolumeDisplayNode
::SetInputImageDataPort(vtkAlgorithmOutput *imageDataPort)
{
  this->SetInputToImageDataPipeline(imageDataPort);
  this->Modified();
}
#endif

//----------------------------------------------------------------------------
#if (VTK_MAJOR_VERSION <= 5)
void vtkMRMLVolumeDisplayNode::SetInputToImageDataPipeline(vtkImageData *vtkNotUsed(imageData))
{
}
#else
void vtkMRMLVolumeDisplayNode::SetInputToImageDataPipeline(vtkAlgorithmOutput *vtkNotUsed(imageDataPort))
{
}
#endif

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLVolumeDisplayNode::GetInputImageData()
{
  return NULL;
}

//----------------------------------------------------------------------------
#if (VTK_MAJOR_VERSION <= 5)
void vtkMRMLVolumeDisplayNode::SetBackgroundImageData(vtkImageData* vtkNotUsed(imageData))
{
}
#else
void vtkMRMLVolumeDisplayNode::SetBackgroundImageDataPort(vtkAlgorithmOutput* vtkNotUsed(imageDataPort))
{
}
#endif

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLVolumeDisplayNode::GetBackgroundImageData()
{
  return 0;
}

//----------------------------------------------------------------------------
#if (VTK_MAJOR_VERSION <= 5)
vtkImageData* vtkMRMLVolumeDisplayNode::GetOutputImageData()
{
  return NULL;
}
#else
vtkAlgorithmOutput* vtkMRMLVolumeDisplayNode::GetOutputImageDataPort()
{
  return NULL;
}
#endif

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
#if (VTK_MAJOR_VERSION <= 5)
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
#endif

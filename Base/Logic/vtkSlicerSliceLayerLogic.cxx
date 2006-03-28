/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerSliceLayerLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkSlicerSliceLayerLogic.h"

vtkCxxRevisionMacro(vtkSlicerSliceLayerLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerSliceLayerLogic);

//----------------------------------------------------------------------------
vtkSlicerSliceLayerLogic::vtkSlicerSliceLayerLogic()
{
  this->VolumeNode = NULL;
  this->SliceNode = NULL;

  this->Reslice = vtkImageReslice::New();
  this->MapToRGBA = vtkImageMapToRGBA::New();
  this->MapToWindowLevelColors = vtkImageMapToWindowLevelColors::New();

  this->Reslice->SetInput( this->MapToRGBA->GetOutput() );
  this->MapToRGBA->SetInput( this->MapToWindowLevelColors->GetOutput() );
}

//----------------------------------------------------------------------------
vtkSlicerSliceLayerLogic::~vtkSlicerSliceLayerLogic()
{
}


//----------------------------------------------------------------------------
void vtkSlicerSliceLayerLogic::SetSliceNode(vtkMRMLSliceNode *SliceNode)
{
  if (this->SliceNode)
    {
    this->SliceNode->Delete();
    }
  this->SliceNode = SliceNode;
  this->SliceNode->Register(this);
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLayerLogic::SetVolumeNode(vtkMRMLScalarVolumeNode *VolumeNode)
{
  if (this->VolumeNode)
    {
    this->VolumeNode->Delete();
    }
  this->VolumeNode = VolumeNode;
  this->VolumeNode->Register(this);

  if (this->VolumeNode)
    {
    this->Reslice->SetInput( this->VolumeNode->GetImageData() ); 
    }
    else
    {
    this->Reslice->SetInput( NULL ); 
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLayerLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "SlicerSliceLayerLogic:             " << this->GetClassName() << "\n";

  os << indent << "VolumeNode: " <<
    (this->VolumeNode ? this->VolumeNode->GetName() : "(none)") << "\n";
  os << indent << "SliceNode: " <<
    (this->SliceNode ? this->SliceNode->GetName() : "(none)") << "\n";
  // TODO: fix printing of vtk objects
  os << indent << "Reslice: " <<
    (this->Reslice ? "this->Reslice" : "(none)") << "\n";
  os << indent << "MapToRGBA: " <<
    (this->MapToRGBA ? "this->MapToRGBA" : "(none)") << "\n";
  os << indent << "MapToWindowLevelColors: " <<
    (this->MapToWindowLevelColors ? "this->MapToWindowLevelColors" : "(none)") << "\n";

}


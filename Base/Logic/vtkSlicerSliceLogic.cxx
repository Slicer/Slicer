/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerSliceLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkSlicerSliceLogic.h"

vtkCxxRevisionMacro(vtkSlicerSliceLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerSliceLogic);

//----------------------------------------------------------------------------
vtkSlicerSliceLogic::vtkSlicerSliceLogic()
{
  this->BackgroundLayer = NULL;
  this->ForegroundLayer = NULL;
  this->SliceNode = NULL;
  this->ForegroundOpacity = 0.5;

  this->Blend = vtkImageBlend::New();
}

//----------------------------------------------------------------------------
vtkSlicerSliceLogic::~vtkSlicerSliceLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::SetSliceNode(vtkMRMLSliceNode *SliceNode)
{
  if (this->SliceNode)
    {
    this->SliceNode->Delete();
    }
  if (this->BackgroundLayer)
    {
    this->BackgroundLayer->SetSliceNode(SliceNode);
    }

  if (this->ForegroundLayer)
    {
    this->ForegroundLayer->SetSliceNode(SliceNode);
    }

  this->SliceNode = SliceNode;
  this->SliceNode->Register(this);
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::SetBackgroundLayer(vtkSlicerSliceLayerLogic *BackgroundLayer)
{
  if (this->BackgroundLayer)
    {
    this->BackgroundLayer->Delete();
    }
  this->BackgroundLayer = BackgroundLayer;
  this->BackgroundLayer->Register(this);
  this->BackgroundLayer->SetSliceNode(SliceNode);
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::SetForegroundLayer(vtkSlicerSliceLayerLogic *ForegroundLayer)
{
  if (this->ForegroundLayer)
    {
    this->ForegroundLayer->Delete();
    }
  this->ForegroundLayer = ForegroundLayer;
  this->ForegroundLayer->Register(this);
  this->ForegroundLayer->SetSliceNode(SliceNode);
}


//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::SetForegroundOpacity(double ForegroundOpacity)
{
  this->ForegroundOpacity = ForegroundOpacity;
  this->Blend->SetOpacity(1, this->ForegroundOpacity);
}


//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "SlicerSliceLogic:             " << this->GetClassName() << "\n";

  os << indent << "SliceNode: " <<
    (this->SliceNode ? this->SliceNode->GetName() : "(none)") << "\n";
  // TODO: fix printing of vtk objects
  os << indent << "BackgroundLayer: " <<
    (this->BackgroundLayer ? "this->BackgroundLayer" : "(none)") << "\n";
  os << indent << "ForegroundLayer: " <<
    (this->ForegroundLayer ? "this->ForegroundLayer" : "(none)") << "\n";
  os << indent << "Blend: " <<
    (this->Blend ? "this->Blend" : "(none)") << "\n";
  os << indent << "ForegroundOpacity: " << this->ForegroundOpacity << "\n";

}


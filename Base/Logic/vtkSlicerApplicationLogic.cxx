/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkSlicerApplicationLogic.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLModelNode.h"

vtkCxxRevisionMacro(vtkSlicerApplicationLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerApplicationLogic);


//----------------------------------------------------------------------------
vtkSlicerApplicationLogic::vtkSlicerApplicationLogic()
{
    this->MRMLScene = vtkMRMLScene::New();

    this->Views = NULL;
    this->Slices = NULL;
    this->Modules = NULL;
    this->ActiveSlice = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerApplicationLogic::~vtkSlicerApplicationLogic()
{
  if (this->Views)
    {
    this->Views->Delete();
    this->Views = NULL;
    }
  if (this->Slices)
    {
    this->Slices->Delete();
    this->Slices = NULL;
    }
  if (this->Modules)
    {
    this->Modules->Delete();
    this->Modules = NULL;
    }
  this->SetActiveSlice(NULL);
  if (this->MRMLScene)
    {
    this->MRMLScene->Delete();
    this->MRMLScene = NULL;
    }

  // TODO - unregister/delete ivars
}

//----------------------------------------------------------------------------
// Create a new Slice with it's associated class instances
vtkSlicerSliceLogic *vtkSlicerApplicationLogic::CreateSlice ()
{
    // Create the logic instances
    vtkSlicerSliceLogic *sliceLogic = vtkSlicerSliceLogic::New();
    vtkSlicerSliceLayerLogic *bg = vtkSlicerSliceLayerLogic::New();
    vtkSlicerSliceLayerLogic *fg = vtkSlicerSliceLayerLogic::New();

    // Create the mrml nodes to store state
    vtkMRMLSliceNode *sliceNode = vtkMRMLSliceNode::New();
    this->MRMLScene->AddNode(sliceNode);

    // Configure the logic
    sliceLogic->SetBackgroundLayer(bg);
    sliceLogic->SetForegroundLayer(fg);
    sliceLogic->SetSliceNode(sliceNode);

    // Update internal state
    this->Slices->AddItem(sliceLogic);
    this->SetActiveSlice(sliceLogic);

    // Since they were New(), they should be Deleted(). If it crashes
    // then something is not ref-counted properly and should be fixed
    // (otherwise you are just leaking)

    sliceLogic->Delete();
    bg->Delete();
    fg->Delete();
    sliceNode->Delete();

    return (sliceLogic);
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "SlicerApplicationLogic:             " << this->GetClassName() << "\n"; 
} 


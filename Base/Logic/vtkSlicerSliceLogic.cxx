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
#include "vtkCallbackCommand.h"

#include "vtkSlicerSliceLogic.h"


vtkCxxRevisionMacro(vtkSlicerSliceLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerSliceLogic);

//----------------------------------------------------------------------------
vtkSlicerSliceLogic::vtkSlicerSliceLogic()
{
  this->BackgroundLayer = NULL;
  this->ForegroundLayer = NULL;
  this->SliceNode = NULL;
  this->SliceCompositeNode = NULL;
  this->ForegroundOpacity = 0.5;

  this->Blend = vtkImageBlend::New();
  this->SetForegroundOpacity(this->ForegroundOpacity);
}

//----------------------------------------------------------------------------
vtkSlicerSliceLogic::~vtkSlicerSliceLogic()
{
  this->Blend->Delete();

  if (this->MRMLCallbackCommand)
    {
    this->MRMLCallbackCommand->Delete();
    this->MRMLCallbackCommand = NULL;
    }

  if (this->LogicCallbackCommand)
    {
    this->LogicCallbackCommand->Delete();
    this->LogicCallbackCommand = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::ProcessMRMLEvents()
{
  cerr << "updating slice logic from a mrml event" << endl ;
  this->UpdatePipeline();
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::ProcessLogicEvents()
{
    cerr << "updating  slice logic from a logic event" << endl ;
  // This is called when a slice layer is modified, so pass it on
  // to anyone interested in changes to this sub-pipeline
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::SetSliceNode(vtkMRMLSliceNode *SliceNode)
{
    // Don't directly observe the slice node -- the layers will observe it and
    // will notify us when things have changed.
    // This class takes care of passing the one slice node to each of the layers
    // so that users of this class only need to set the node one place.
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

  this->Modified();

}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::SetSliceCompositeNode(vtkMRMLSliceCompositeNode *SliceCompositeNode)
{
    // Observe the composite node, since this holds the parameters for
    // this pipeline
  if (this->SliceCompositeNode)
    {
    this->SliceCompositeNode->RemoveObserver( this->MRMLCallbackCommand );
    this->SliceCompositeNode->Delete();
    }

  this->SliceCompositeNode = SliceCompositeNode;

  if (this->SliceCompositeNode)
    {
    this->SliceCompositeNode->Register(this);
    this->SliceCompositeNode->AddObserver( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }

  this->UpdatePipeline();

}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::SetBackgroundLayer(vtkSlicerSliceLayerLogic *BackgroundLayer)
{
  if (this->BackgroundLayer)
    {
    this->BackgroundLayer->RemoveObserver( this->LogicCallbackCommand );
    this->BackgroundLayer->Delete();
    }
  this->BackgroundLayer = BackgroundLayer;

  if (this->BackgroundLayer)
    {
    this->BackgroundLayer->Register(this);
    this->BackgroundLayer->AddObserver( vtkCommand::ModifiedEvent, this->LogicCallbackCommand );
    this->BackgroundLayer->SetSliceNode(SliceNode);
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::SetForegroundLayer(vtkSlicerSliceLayerLogic *ForegroundLayer)
{
  if (this->ForegroundLayer)
    {
    this->ForegroundLayer->RemoveObserver( this->LogicCallbackCommand );
    this->ForegroundLayer->Delete();
    }
  this->ForegroundLayer = ForegroundLayer;

  if (this->ForegroundLayer)
    {
    this->ForegroundLayer->Register(this);
    this->ForegroundLayer->AddObserver( vtkCommand::ModifiedEvent, this->LogicCallbackCommand );
    this->ForegroundLayer->SetSliceNode(SliceNode);
    }

  this->Modified();
}


//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::SetForegroundOpacity(double ForegroundOpacity)
{
  this->ForegroundOpacity = ForegroundOpacity;

  if ( this->Blend->GetOpacity(1) != this->ForegroundOpacity )
    {
    this->Blend->SetOpacity(1, this->ForegroundOpacity);
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::UpdatePipeline()
{
  if ( this->SliceCompositeNode )
    {
    // get the background and foreground image data from the layers
    // so we can use them as input to the image blend
    // TODO: change logic to use a volume node superclass rather than
    // a scalar volume node once the superclass is sorted out

    const char *id;
    
    // Background
    id = this->SliceCompositeNode->GetBackgroundVolumeID();
    vtkMRMLScalarVolumeNode *bgnode = NULL;
    if (id)
      {
      bgnode = vtkMRMLScalarVolumeNode::SafeDownCast (this->MRMLScene->GetNodeByID(id));
      }
    
    if (this->BackgroundLayer)
      {
      this->BackgroundLayer->SetVolumeNode (bgnode);
      }

    // Foreground
    id = this->SliceCompositeNode->GetForegroundVolumeID();
    vtkMRMLScalarVolumeNode *fgnode = NULL;
    if (id)
      {
      fgnode = vtkMRMLScalarVolumeNode::SafeDownCast (this->MRMLScene->GetNodeByID(id));
      }
    
    if (this->ForegroundLayer)
      {
      this->ForegroundLayer->SetVolumeNode (fgnode);
      }

    // Now update the image blend with the background and foreground
    this->Blend->RemoveAllInputs ( );
    if ( this->BackgroundLayer )
      {
      this->Blend->AddInput( this->BackgroundLayer->GetImageData() );
      }
    if ( this->ForegroundLayer )
      {
      this->Blend->AddInput( this->ForegroundLayer->GetImageData() );
      }

    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "SlicerSliceLogic:             " << this->GetClassName() << "\n";

  os << indent << "SliceNode: " <<
    (this->SliceNode ? this->SliceNode->GetName() : "(none)") << "\n";
  os << indent << "SliceCompositeNode: " <<
    (this->SliceCompositeNode ? this->SliceCompositeNode->GetName() : "(none)") << "\n";
  // TODO: fix printing of vtk objects
  os << indent << "BackgroundLayer: " <<
    (this->BackgroundLayer ? "not null" : "(none)") << "\n";
  os << indent << "ForegroundLayer: " <<
    (this->ForegroundLayer ? "not null" : "(none)") << "\n";
  os << indent << "Blend: " <<
    (this->Blend ? "not null" : "(none)") << "\n";
  os << indent << "ForegroundOpacity: " << this->ForegroundOpacity << "\n";

}


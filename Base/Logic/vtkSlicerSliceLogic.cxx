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
#include "vtkPlaneSource.h"
#include "vtkPoints.h"

#include "vtkMRMLModelDisplayNode.h"

#include "vtkSlicerSliceLogic.h"

#include <sstream>

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
  this->SliceModelNode = NULL;
  this->Name = NULL;
  this->SliceModelNodeID = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerSliceLogic::~vtkSlicerSliceLogic()
{
  this->SetSliceNode(NULL);

  if ( this->Blend ) 
    {
    this->Blend->Delete();
    this->Blend = NULL;
    }

  this->SetBackgroundLayer (NULL);
  this->SetForegroundLayer (NULL);

  if ( this->SliceCompositeNode ) 
    {
    this->SetAndObserveMRML( vtkObjectPointer(&this->SliceCompositeNode), NULL );
    }
  if (this->SliceModelNode != NULL)
    {
    this->SliceModelNode->Delete();
    }
  this->SetName(NULL);

  this->SetSliceModelNodeID(NULL);

}


//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::ProcessMRMLEvents()
{

  //
  // if you don't have a node yet, look in the scene to see if 
  // one exists for you to use.  If not, create one and add it to the scene
  //
  if ( this->SliceNode == NULL )
    {
    vtkMRMLSliceNode *node = vtkMRMLSliceNode::New();
    this->MRMLScene->AddNode(node);
    this->SetSliceNode (node);
    node->Delete();
    }

  //
  // if you don't have a node yet, look in the scene to see if 
  // one exists for you to use.  If not, create one and add it to the scene
  //
  if ( this->SliceCompositeNode == NULL )
    {
    vtkMRMLSliceCompositeNode *node = vtkMRMLSliceCompositeNode::New();
    this->MRMLScene->AddNode(node);
    node->SetBackgroundVolumeID("None");
    node->SetForegroundVolumeID("None");
    this->SetSliceCompositeNode (node);
    node->Delete();
    }

  //
  // check that our referenced nodes exist, and if not set to None
  //
  if ( this->MRMLScene->GetNodeByID( this->SliceCompositeNode->GetForegroundVolumeID() ) == NULL )
    {
    this->SliceCompositeNode->SetForegroundVolumeID("None");
    }

  if ( this->MRMLScene->GetNodeByID( this->SliceCompositeNode->GetBackgroundVolumeID() ) == NULL )
    {
    this->SliceCompositeNode->SetBackgroundVolumeID("None");
    }
  
  if ( this->GetSliceModelNodeID() == NULL || this->GetSliceModelNode() == NULL || this->MRMLScene->GetNodeByID( this->GetSliceModelNodeID() ) == NULL )
    {
    this->CreateSliceModel();
    }
  this->UpdatePipeline();
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::ProcessLogicEvents()
{
  //
  // if we don't have layers yet, create them 
  //
  if ( this->BackgroundLayer == NULL )
    {
    vtkSlicerSliceLayerLogic *layer = vtkSlicerSliceLayerLogic::New();
    this->SetBackgroundLayer (layer);
    layer->Delete();
    }
  if ( this->ForegroundLayer == NULL )
    {
    vtkSlicerSliceLayerLogic *layer = vtkSlicerSliceLayerLogic::New();
    this->SetForegroundLayer (layer);
    layer->Delete();
    }

  // Update slice plane geometry
  if (this->SliceNode != NULL &&  this->GetSliceModelNodeID() != NULL && this->GetSliceModelNode() != NULL && this->MRMLScene->GetNodeByID( this->GetSliceModelNodeID() ) != NULL )
    {
    vtkPoints *points = this->SliceModelNode->GetPolyData()->GetPoints();
    unsigned int *dims = this->SliceNode->GetDimensions();
    vtkMatrix4x4 *xyToRAS = this->SliceNode->GetXYToRAS();

    double inPt[4]={0,0,0,1};
    double outPt[4];
    double *outPt3 = outPt;

    xyToRAS->MultiplyPoint(inPt, outPt);
    points->SetPoint(0, outPt3);

    inPt[0] = dims[0];
    xyToRAS->MultiplyPoint(inPt, outPt);
    points->SetPoint(1, outPt3);

    inPt[0] = 0;
    inPt[1] = dims[1];
    xyToRAS->MultiplyPoint(inPt, outPt);
    points->SetPoint(2, outPt3);

    inPt[0] = dims[0];
    inPt[1] = dims[1];
    xyToRAS->MultiplyPoint(inPt, outPt);
    points->SetPoint(3, outPt3);

    this->SliceModelNode->GetPolyData()->Modified();
    }

  // This is called when a slice layer is modified, so pass it on
  // to anyone interested in changes to this sub-pipeline
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::SetSliceNode(vtkMRMLSliceNode *sliceNode)
{
    // Don't directly observe the slice node -- the layers will observe it and
    // will notify us when things have changed.
    // This class takes care of passing the one slice node to each of the layers
    // so that users of this class only need to set the node one place.
  this->SetMRML( vtkObjectPointer(&this->SliceNode), sliceNode );

  if (this->BackgroundLayer)
    {
    this->BackgroundLayer->SetSliceNode(sliceNode);
    }
  if (this->ForegroundLayer)
    {
    this->ForegroundLayer->SetSliceNode(sliceNode);
    }

  this->Modified();

}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::SetSliceCompositeNode(vtkMRMLSliceCompositeNode *sliceCompositeNode)
{
    // Observe the composite node, since this holds the parameters for
    // this pipeline
  this->SetAndObserveMRML( vtkObjectPointer(&this->SliceCompositeNode), sliceCompositeNode );
  this->UpdatePipeline();

}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::SetBackgroundLayer(vtkSlicerSliceLayerLogic *BackgroundLayer)
{
  if (this->BackgroundLayer)
    {
    this->BackgroundLayer->RemoveObserver( this->LogicCallbackCommand );
    this->BackgroundLayer->SetAndObserveMRMLScene( NULL );
    this->BackgroundLayer->Delete();
    }
  this->BackgroundLayer = BackgroundLayer;

  if (this->BackgroundLayer)
    {
    this->BackgroundLayer->Register(this);
    this->BackgroundLayer->SetAndObserveMRMLScene( this->MRMLScene );
    this->BackgroundLayer->SetSliceNode(SliceNode);
    this->BackgroundLayer->AddObserver( vtkCommand::ModifiedEvent, this->LogicCallbackCommand );
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::SetForegroundLayer(vtkSlicerSliceLayerLogic *ForegroundLayer)
{
  if (this->ForegroundLayer)
    {
    this->ForegroundLayer->RemoveObserver( this->LogicCallbackCommand );
    this->ForegroundLayer->SetAndObserveMRMLScene( NULL );
    this->ForegroundLayer->Delete();
    }
  this->ForegroundLayer = ForegroundLayer;

  if (this->ForegroundLayer)
    {
    this->ForegroundLayer->Register(this);
    this->ForegroundLayer->SetAndObserveMRMLScene( this->MRMLScene );
    this->ForegroundLayer->SetSliceNode(SliceNode);
    this->ForegroundLayer->AddObserver( vtkCommand::ModifiedEvent, this->LogicCallbackCommand );
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
    // a scalar volume node once the superclass is sorted out for vector/tensor Volumes

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
      this->Blend->SetOpacity( 1, this->SliceCompositeNode->GetOpacity() );
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

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::CreateSliceModel()
{
  this->SliceModelNode = vtkMRMLModelNode::New();
  this->SliceModelNode->SetScene(this->GetMRMLScene());
  // create plane slice
  vtkPlaneSource *plane = vtkPlaneSource::New();
  plane->GetOutput()->Update();
  this->SliceModelNode->SetAndObservePolyData(plane->GetOutput());

  // create display node and set texture
  vtkMRMLModelDisplayNode *displayNode = vtkMRMLModelDisplayNode::New();
  displayNode->SetScene(this->GetMRMLScene());
  displayNode->SetVisibility(0);
  displayNode->SetOpacity(1);
  displayNode->SetColor(1,1,1);
  displayNode->SetAmbient(1);
  displayNode->SetDiffuse(0);
  displayNode->SetAndObserveTextureImageData(this->GetImageData());

  std::stringstream ss;
  char name[256];
  ss << this->Name << " Volume Slice";
  ss.getline(name,256);
  this->SliceModelNode->SetName(name);
  this->MRMLScene->AddNode(this->SliceModelNode);
  this->SetSliceModelNodeID(this->SliceModelNode->GetID());
  this->MRMLScene->AddNode(displayNode);
  this->SliceModelNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  plane->Delete();
  this->SliceModelNode->Delete();
  displayNode->Delete();
}


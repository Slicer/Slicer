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
  this->LabelLayer = NULL;
  this->SliceNode = NULL;
  this->SliceCompositeNode = NULL;
  this->ForegroundOpacity = 0.5; // Start by blending fg/bg
  this->LabelOpacity = 1.0;
  this->Blend = vtkImageBlend::New();
  this->SetForegroundOpacity(this->ForegroundOpacity);
  this->SetLabelOpacity(this->LabelOpacity);
  this->SliceModelNode = NULL;
  this->Name = NULL;
  this->PlaneSource = NULL;
  this->SliceModelDisplayNode = NULL;
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
  this->SetLabelLayer (NULL);

  if ( this->SliceCompositeNode ) 
    {
    this->SetAndObserveMRML( vtkObjectPointer(&this->SliceCompositeNode), NULL );
    }
  if (this->SliceModelNode != NULL)
    {
    this->SliceModelNode->Delete();
    }
  if (this->PlaneSource != NULL)
    {
    this->PlaneSource->Delete();
    }
  if (this->SliceModelDisplayNode != NULL)
    {
    this->SliceModelDisplayNode->Delete();
    }

  this->SetName(NULL);

}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::UpdateSliceNode()
{
  // find SliceNode in the scene
  vtkMRMLSliceNode *node= NULL;
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLSliceNode");
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLSliceNode::SafeDownCast (
          this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLSliceNode"));
    if (!strcmp(node->GetLayoutName(), this->GetName()))
      {
      break;
      }
    node = NULL;
    }

  if ( this->SliceNode != NULL && node != NULL && strcmp(this->SliceNode->GetID(), node->GetID()) != 0 )
    {
    // local SliceNode is out of sync with the scene
    this->SetSliceNode (NULL);
    }

  if ( this->SliceNode == NULL )
    {
    if ( node == NULL )
      {
      node = vtkMRMLSliceNode::New();
      node->SetLayoutName(this->GetName());
      this->SetSliceNode (node);
      node->Delete();
      }
    else
      {
      this->SetSliceNode (node);
      }
    }


  if ( this->MRMLScene->GetNodeByID(this->SliceNode->GetID()) == NULL)
    {
    // local node not in the scene
    node = this->SliceNode;
    node->Register(this);
    this->SetSliceNode (NULL);
    this->MRMLScene->AddNodeNoNotify(node);
    this->SetSliceNode (node);
    node->UnRegister(this);
    }
}

//----------------------------------------------------------------------------

void vtkSlicerSliceLogic::UpdateSliceCompositeNode()
{
  // find SliceCompositeNode in the scene
  vtkMRMLSliceCompositeNode *node= NULL;
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLSliceCompositeNode::SafeDownCast (
          this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLSliceCompositeNode"));
    if (!strcmp(node->GetLayoutName(), this->GetName()))
      {
      break;
      }
    node = NULL;
    }

  if ( this->SliceCompositeNode != NULL && node != NULL && 
        strcmp(this->SliceCompositeNode->GetID(), node->GetID()) != 0 )
    {
    // local SliceCompositeNode is out of sync with the scene
    this->SetSliceCompositeNode (NULL);
    }

  if ( this->SliceCompositeNode == NULL )
    {
    if ( node == NULL )
      {
      node = vtkMRMLSliceCompositeNode::New();
      node->SetLayoutName(this->GetName());
      this->SetSliceCompositeNode (node);
      node->Delete();
      }
    else
      {
      this->SetSliceCompositeNode (node);
      }
    }

  if ( this->MRMLScene->GetNodeByID(this->SliceCompositeNode->GetID()) == NULL)
    {
    // local node not in the scene
    node = this->SliceCompositeNode;
    node->Register(this);
    this->SetSliceCompositeNode (NULL);
    this->MRMLScene->AddNodeNoNotify(node);
    this->SetSliceCompositeNode (node);
    node->UnRegister(this);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::ProcessMRMLEvents(vtkObject * caller, 
                                            unsigned long event, 
                                            void * callData )
{
 //
  // if you don't have a node yet, look in the scene to see if 
  // one exists for you to use.  If not, create one and add it to the scene
  //  
  if (event == vtkMRMLScene::SceneCloseEvent) 
    {
    this->SetSliceCompositeNode (NULL);
    this->SetSliceNode (NULL);
    return;
    }

  this->CreateSliceModel();

  this->UpdateSliceNode();
 
  this->UpdateSliceCompositeNode();

  //
  // check that our referenced nodes exist, and if not set to None
  //
  if ( this->MRMLScene->GetNodeByID( this->SliceCompositeNode->GetForegroundVolumeID() ) == NULL )
    {
    this->SliceCompositeNode->SetForegroundVolumeID(NULL);
    }

  if ( this->MRMLScene->GetNodeByID( this->SliceCompositeNode->GetLabelVolumeID() ) == NULL )
    {
    this->SliceCompositeNode->SetLabelVolumeID(NULL);
    }

  if ( this->MRMLScene->GetNodeByID( this->SliceCompositeNode->GetBackgroundVolumeID() ) == NULL )
    {
    this->SliceCompositeNode->SetBackgroundVolumeID(NULL);
    }
    
  if (event != vtkMRMLScene::NewSceneEvent) 
    {
    this->UpdatePipeline();
    }
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
  if ( this->LabelLayer == NULL )
    {
    vtkSlicerSliceLayerLogic *layer = vtkSlicerSliceLayerLogic::New();
    this->SetLabelLayer (layer);
    layer->Delete();
    }

  // Update slice plane geometry
  if (this->SliceNode != NULL && this->GetSliceModelNode() != NULL 
      && this->MRMLScene->GetNodeByID( this->SliceModelNode->GetID() ) != NULL && 
        this->SliceModelNode->GetPolyData() != NULL )
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
    vtkMRMLModelDisplayNode *modelDisplayNode = this->SliceModelNode->GetDisplayNode();
    if ( modelDisplayNode )
      {
      modelDisplayNode->SetVisibility( this->SliceNode->GetSliceVisible() );
      }
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
  if (this->LabelLayer)
    {
    this->LabelLayer->SetSliceNode(sliceNode);
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
void vtkSlicerSliceLogic::SetLabelLayer(vtkSlicerSliceLayerLogic *LabelLayer)
{
  if (this->LabelLayer)
    {
    this->LabelLayer->RemoveObserver( this->LogicCallbackCommand );
    this->LabelLayer->SetAndObserveMRMLScene( NULL );
    this->LabelLayer->Delete();
    }
  this->LabelLayer = LabelLayer;

  if (this->LabelLayer)
    {
    this->LabelLayer->Register(this);
    this->LabelLayer->SetAndObserveMRMLScene( this->MRMLScene );
    this->LabelLayer->SetSliceNode(SliceNode);
    this->LabelLayer->AddObserver( vtkCommand::ModifiedEvent, this->LogicCallbackCommand );
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
void vtkSlicerSliceLogic::SetLabelOpacity(double LabelOpacity)
{
  this->LabelOpacity = LabelOpacity;

  if ( this->Blend->GetOpacity(2) != this->LabelOpacity )
    {
    this->Blend->SetOpacity(2, this->LabelOpacity);
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

    // Label
    id = this->SliceCompositeNode->GetLabelVolumeID();
    vtkMRMLScalarVolumeNode *lbnode = NULL;
    if (id)
      {
      lbnode = vtkMRMLScalarVolumeNode::SafeDownCast (this->MRMLScene->GetNodeByID(id));
      }
    
    if (this->LabelLayer)
      {
      this->LabelLayer->SetVolumeNode (lbnode);
      }

    // Now update the image blend with the background and foreground and label
    // -- layer 0 opacity is ignored, but since not all inputs may be non-null, 
    //    we keep track so that someone could, for example, have a NULL background
    //    with a non-null foreground and label and everything will work with the 
    //    label opacity
    this->Blend->RemoveAllInputs ( );
    int layerIndex = 0;
    if ( this->BackgroundLayer && this->BackgroundLayer->GetImageData() )
      {
      this->Blend->AddInput( this->BackgroundLayer->GetImageData() );
      this->Blend->SetOpacity( layerIndex++, 1.0 );
      }
    if ( this->ForegroundLayer && this->ForegroundLayer->GetImageData() )
      {
      this->Blend->AddInput( this->ForegroundLayer->GetImageData() );
      this->Blend->SetOpacity( layerIndex++, this->SliceCompositeNode->GetForegroundOpacity() );
      }
    if ( this->LabelLayer && this->LabelLayer->GetImageData() )
      {
      this->Blend->AddInput( this->LabelLayer->GetImageData() );
      this->Blend->SetOpacity( layerIndex++, this->SliceCompositeNode->GetLabelOpacity() );
      }

    if ( this->SliceModelNode && 
          this->SliceModelNode->GetDisplayNode() &&
            this->SliceNode ) 
      {
      this->SliceModelNode->GetDisplayNode()->SetVisibility( this->SliceNode->GetSliceVisible() );
      this->SliceModelNode->GetDisplayNode()->SetAndObserveTextureImageData(this->GetImageData());
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
  os << indent << "LabelLayer: " <<
    (this->LabelLayer ? "not null" : "(none)") << "\n";
  os << indent << "Blend: " <<
    (this->Blend ? "not null" : "(none)") << "\n";
  os << indent << "ForegroundOpacity: " << this->ForegroundOpacity << "\n";
  os << indent << "LabelOpacity: " << this->LabelOpacity << "\n";

}

//----------------------------------------------------------------------------
void vtkSlicerSliceLogic::CreateSliceModel()
{
  if ( this->SliceModelNode == NULL) 
  {
    this->SliceModelNode = vtkMRMLModelNode::New();
    this->SliceModelNode->SetScene(this->GetMRMLScene());
    this->SliceModelNode->SetHideFromEditors(1);

    // create plane slice
    this->PlaneSource = vtkPlaneSource::New();
    this->PlaneSource->GetOutput()->Update();
    this->SliceModelNode->SetAndObservePolyData(this->PlaneSource->GetOutput());

    // create display node and set texture
    this->SliceModelDisplayNode = vtkMRMLModelDisplayNode::New();
    this->SliceModelDisplayNode->SetScene(this->GetMRMLScene());
    this->SliceModelDisplayNode->SetVisibility(0);
    this->SliceModelDisplayNode->SetOpacity(1);
    this->SliceModelDisplayNode->SetColor(1,1,1);
    this->SliceModelDisplayNode->SetAmbient(1);
    this->SliceModelDisplayNode->SetDiffuse(0);
    this->SliceModelDisplayNode->SetAndObserveTextureImageData(this->GetImageData());

    std::stringstream ss;
    char name[256];
    ss << this->Name << " Volume Slice";
    ss.getline(name,256);
    this->SliceModelNode->SetName(name);
    this->SliceModelNode->SetAndObserveDisplayNodeID(this->SliceModelDisplayNode->GetID());
  }

  if (this->SliceModelNode != NULL && this->MRMLScene->GetNodeByID( this->GetSliceModelNode()->GetID() ) == NULL )
    {
    vtkIntArray *events = vtkIntArray::New();
    for (int i=0; i<this->Events->GetNumberOfTuples(); i++)
      {
      events->InsertNextValue(this->Events->GetValue(i));
      }
    this->SetMRMLScene(this->GetMRMLScene());
    this->MRMLScene->AddNode(this->SliceModelDisplayNode);
    this->MRMLScene->AddNode(this->SliceModelNode);
    this->SliceModelNode->SetAndObserveDisplayNodeID(this->SliceModelDisplayNode->GetID());
    this->SliceModelDisplayNode->SetAndObserveTextureImageData(this->GetImageData());
    this->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
    events->Delete();
    }
}



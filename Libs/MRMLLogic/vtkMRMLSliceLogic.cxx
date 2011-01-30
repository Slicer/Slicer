/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSliceLogic.cxx,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

// MRMLLogic includes
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceLayerLogic.h"

// MRML includes
#include <vtkMRMLCrosshairNode.h>
#include <vtkMRMLDiffusionTensorVolumeNode.h>
#include <vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLProceduralColorNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkImageBlend.h>
#include <vtkImageMathematics.h>
#include <vtkImageReslice.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPlaneSource.h>
#include <vtkPoints.h>
#include <vtkPolyDataCollection.h>
#include <vtkSmartPointer.h>

// STD includes
#include <sstream>
#include <iostream>
#include <cassert>

//----------------------------------------------------------------------------
// Convenient macros
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

//----------------------------------------------------------------------------
const int vtkMRMLSliceLogic::SLICE_INDEX_ROTATED=-1;
const int vtkMRMLSliceLogic::SLICE_INDEX_OUT_OF_VOLUME=-2;
const int vtkMRMLSliceLogic::SLICE_INDEX_NO_VOLUME=-3;

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkMRMLSliceLogic, "$Revision$");
vtkStandardNewMacro(vtkMRMLSliceLogic);

//----------------------------------------------------------------------------
vtkMRMLSliceLogic::vtkMRMLSliceLogic()
{
  this->Initialized = false;
  this->Name = 0;
  this->BackgroundLayer = 0;
  this->ForegroundLayer = 0;
  this->LabelLayer = 0;
  this->SliceNode = 0;
  this->SliceCompositeNode = 0;
  this->ForegroundOpacity = 0.5; // Start by blending fg/bg
  this->LabelOpacity = 1.0;
  this->Blend = vtkImageBlend::New();
  this->ExtractModelTexture = vtkImageReslice::New();
  this->ExtractModelTexture->SetOutputDimensionality (2);
  this->PolyDataCollection = vtkPolyDataCollection::New();
  this->LookupTableCollection = vtkCollection::New();
  this->SetForegroundOpacity(this->ForegroundOpacity);
  this->SetLabelOpacity(this->LabelOpacity);
  this->SliceModelNode = 0;
  this->SliceModelTransformNode = 0;
  this->Name = 0;
  this->SetName("");
  this->SliceModelDisplayNode = 0;
  this->ImageData = 0;
  this->SliceSpacing[0] = this->SliceSpacing[1] = this->SliceSpacing[2] = 1;
}

//----------------------------------------------------------------------------
vtkMRMLSliceLogic::~vtkMRMLSliceLogic()
{
  this->SetName(0);
  this->SetSliceNode(0);

  if (this->ImageData)
    {
    this->ImageData->Delete();
    }

  if (this->Blend)
    {
    this->Blend->Delete();
    this->Blend = 0;
    }
  if (this->ExtractModelTexture)
    {
    this->ExtractModelTexture->Delete();
    this->ExtractModelTexture = 0;
    }
  this->PolyDataCollection->Delete();
  this->LookupTableCollection->Delete();
 
  this->SetBackgroundLayer (0);
  this->SetForegroundLayer (0);
  this->SetLabelLayer (0);
 
  if (this->SliceCompositeNode)
    {
    vtkSetAndObserveMRMLNodeMacro( this->SliceCompositeNode, 0);
    }

  this->SetName(0);

  this->DeleteSliceModel();

}

//----------------------------------------------------------------------------
bool vtkMRMLSliceLogic::IsInitialized()
{
  return this->Initialized;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::Initialize(vtkMRMLSliceNode* newSliceNode)
{
  if (this->Initialized)
    {
    vtkWarningMacro(<< "vtkMRMLSliceLogic already initialized");
    return;
    }

  // Sanity checks
  if (!newSliceNode)
    {
    vtkWarningMacro(<< "Initialize - newSliceNode is NULL");
    return;
    }

  this->SetSliceNode(newSliceNode);

  this->Initialized = true;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  // Sanity checks
  if (!this->GetName() || strlen(this->GetName()) == 0)
    {
    vtkErrorMacro(<< "Name is NULL - Make sure you call SetName before SetMRMLScene !");
    return;
    }

  // List of events the slice logics should listen
  vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
  events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneAboutToBeClosedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneRestoredEvent);
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

  this->SetAndObserveMRMLSceneEventsInternal(newScene, events);

  this->ProcessLogicEvents();
  this->ProcessMRMLEvents(newScene, vtkCommand::ModifiedEvent, 0);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdateSliceNode()
{
  // find SliceNode in the scene
  vtkMRMLSliceNode *node= 0;
  int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceNode");
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLSliceNode::SafeDownCast (
          this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLSliceNode"));
    if (node->GetLayoutName() && !strcmp(node->GetLayoutName(), this->GetName()))
      {
      break;
      }
    node = 0;
    }

  if ( this->SliceNode != 0 && node != 0 &&
        this->SliceCompositeNode &&
       (this->SliceCompositeNode->GetID() == 0 ||
        strcmp(this->SliceNode->GetID(), node->GetID()) != 0 ))
    {
    // local SliceNode is out of sync with the scene
    this->SetSliceNode (0);
    }
  
  if ( this->SliceNode == 0 )
    {
    if ( node == 0 )
      {
      node = vtkMRMLSliceNode::New();
      node->SetName(this->GetName());
      node->SetLayoutName(this->GetName());
      this->SetSliceNode (node);
      this->UpdateSliceNodeFromLayout();
      node->Delete();
      }
    else
      {

      this->SetSliceNode (node);
      }
    }

  if ( this->GetMRMLScene()->GetNodeByID(this->SliceNode->GetID()) == 0)
    {
    // local node not in the scene
    node = this->SliceNode;
    node->Register(this);
    this->SetSliceNode (0);
    this->GetMRMLScene()->AddNode(node);
    this->SetSliceNode (node);
    node->UnRegister(this);
    }

}

//----------------------------------------------------------------------------

void vtkMRMLSliceLogic::UpdateSliceNodeFromLayout()
{
  if (this->SliceNode == 0)
    {
    return;
    }

  if ( !strcmp( this->GetName(), "Red" ) )
    {
    this->SliceNode->SetOrientationToAxial();
    }
  if ( !strcmp( this->GetName(), "Yellow" ) )
    {
    this->SliceNode->SetOrientationToSagittal();
    }
  if ( !strcmp( this->GetName(), "Green" ) )
    {
    this->SliceNode->SetOrientationToCoronal();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdateSliceCompositeNode()
{
  // find SliceCompositeNode in the scene
  vtkMRMLSliceCompositeNode *node= 0;
  int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLSliceCompositeNode::SafeDownCast (
          this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLSliceCompositeNode"));
    if (node->GetLayoutName() && !strcmp(node->GetLayoutName(), this->GetName()))
      {
      break;
      }
    node = 0;
    }

  if ( this->SliceCompositeNode != 0 && node != 0 &&
       (this->SliceCompositeNode->GetID() == 0 ||
        strcmp(this->SliceCompositeNode->GetID(), node->GetID()) != 0) )
    {
    // local SliceCompositeNode is out of sync with the scene
    this->SetSliceCompositeNode (0);
    }

  if ( this->SliceCompositeNode == 0 )
    {
    if ( node == 0 )
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

  if ( this->GetMRMLScene()->GetNodeByID(this->SliceCompositeNode->GetID()) == 0)
    {
    // local node not in the scene
    node = this->SliceCompositeNode;
    node->Register(this);
    this->SetSliceCompositeNode(0);
    this->GetMRMLScene()->AddNode(node);
    this->SetSliceCompositeNode(node);
    node->UnRegister(this);
    }

}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::ProcessMRMLEvents(vtkObject * caller, 
                                            unsigned long event, 
                                            void * callData )
{
  if (vtkMRMLScene::SafeDownCast(caller) == this->GetMRMLScene())
    {
    if (event == vtkMRMLScene::NodeAddedEvent || event == vtkMRMLScene::NodeRemovedEvent)
      {
      vtkMRMLNode *node =  reinterpret_cast<vtkMRMLNode*> (callData);
      if (!node)
        {
        return;
        }
      // Return if different from SliceCompositeNode, SliceNode or VolumeNode
      if (!(node->IsA("vtkMRMLSliceCompositeNode")
        || node->IsA("vtkMRMLSliceNode")
        || node->IsA("vtkMRMLVolumeNode")))
        {
        return;
        }
      }

    if (event == vtkMRMLScene::SceneAboutToBeClosedEvent ||
        caller == 0)
      {
      this->UpdateSliceNodeFromLayout();
      this->DeleteSliceModel();
      return;
      }
    }
  if (this->GetMRMLScene()->GetIsClosing())
    {
    return;
    }

  // Set up the nodes and models
  this->CreateSliceModel();
  this->UpdateSliceNode();
  this->UpdateSliceCompositeNode();

  //
  // check that our referenced nodes exist, and if not set to None
  //

  /** PROBABLY DONT NEED TO DO THAT And it causes load scene to override ID's
  if ( this->GetMRMLScene()->GetNodeByID( this->SliceCompositeNode->GetForegroundVolumeID() ) == 0 )
    {
    this->SliceCompositeNode->SetForegroundVolumeID(0);
    }

  if ( this->GetMRMLScene()->GetNodeByID( this->SliceCompositeNode->GetLabelVolumeID() ) == 0 )
    {
    this->SliceCompositeNode->SetLabelVolumeID(0);
    }

  if ( this->GetMRMLScene()->GetNodeByID( this->SliceCompositeNode->GetBackgroundVolumeID() ) == 0 )
    {
    this->SliceCompositeNode->SetBackgroundVolumeID(0);
    }
   **/

  if (event != vtkMRMLScene::NewSceneEvent) 
    {
    this->UpdatePipeline();
    }

  //
  // On a new scene or restore, create the singleton for the default crosshair
  // for navigation or cursor if it doesn't already exist in scene
  //
  if ( vtkMRMLScene::SafeDownCast(caller) && 
        ( event == vtkMRMLScene::NewSceneEvent || event == vtkMRMLScene::SceneRestoredEvent ) )
    {
    vtkMRMLScene *scene =  vtkMRMLScene::SafeDownCast(caller);
    int n, numberOfCrosshairs = scene->GetNumberOfNodesByClass("vtkMRMLCrosshairNode");
    int foundDefault = 0;
    vtkMRMLCrosshairNode *crosshair;
    for (n = 0; n < numberOfCrosshairs; n++)
      {
      crosshair = vtkMRMLCrosshairNode::SafeDownCast(
          scene->GetNthNodeByClass(n, "vtkMRMLCrosshairNode"));
      if (crosshair && !strcmp( "default", crosshair->GetCrosshairName()))
        {
        foundDefault = 1;
        }
      }

    if (!foundDefault)
      {
      crosshair = vtkMRMLCrosshairNode::New();
      crosshair->SetCrosshairName("default");
      crosshair->NavigationOn();
      scene->AddNode( crosshair );
      crosshair->Delete();
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::ProcessLogicEvents()
{

  //
  // if we don't have layers yet, create them 
  //
  if ( this->BackgroundLayer == 0 )
    {
    vtkMRMLSliceLayerLogic *layer = vtkMRMLSliceLayerLogic::New();
    this->SetBackgroundLayer (layer);
    layer->Delete();
    }
  if ( this->ForegroundLayer == 0 )
    {
    vtkMRMLSliceLayerLogic *layer = vtkMRMLSliceLayerLogic::New();
    this->SetForegroundLayer (layer);
    layer->Delete();
    }
  if ( this->LabelLayer == 0 )
    {
    vtkMRMLSliceLayerLogic *layer = vtkMRMLSliceLayerLogic::New();
    // turn on using the label outline only in this layer
    layer->IsLabelLayerOn();
    this->SetLabelLayer (layer);
    layer->Delete();
    }
  // Update slice plane geometry
  if (this->SliceNode != 0
      && this->GetSliceModelNode() != 0
      && this->GetMRMLScene() != 0
      && this->GetMRMLScene()->GetNodeByID( this->SliceModelNode->GetID() ) != 0
      && this->SliceModelNode->GetPolyData() != 0 )
    {


    vtkPoints *points = this->SliceModelNode->GetPolyData()->GetPoints();
    int *dims = this->SliceNode->GetDimensions();
    vtkMatrix4x4 *xyToRAS = this->SliceNode->GetXYToRAS();

    // set the plane corner point for use in a model
    double inPt[4]={0,0,0,1};
    double outPt[4];
    double *outPt3 = outPt;

    // set the z position to be the active slice (from the lightbox)
    inPt[2] = this->SliceNode->GetActiveSlice();

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

    this->UpdatePipeline();
    this->SliceModelNode->GetPolyData()->Modified();
    vtkMRMLModelDisplayNode *modelDisplayNode = this->SliceModelNode->GetModelDisplayNode();
    if ( modelDisplayNode )
      {
      modelDisplayNode->SetVisibility( this->SliceNode->GetSliceVisible() );
      if ( this->SliceCompositeNode != 0 )
        {
        modelDisplayNode->SetSliceIntersectionVisibility( this->SliceCompositeNode->GetSliceIntersectionVisibility() );
        }
      }
    }

  // This is called when a slice layer is modified, so pass it on
  // to anyone interested in changes to this sub-pipeline
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetSliceNode(vtkMRMLSliceNode * newSliceNode)
{
  if (this->SliceNode == newSliceNode)
    {
    return;
    }

  // Don't directly observe the slice node -- the layers will observe it and
  // will notify us when things have changed.
  // This class takes care of passing the one slice node to each of the layers
  // so that users of this class only need to set the node one place.
  vtkSetMRMLNodeMacro( this->SliceNode, newSliceNode );

  if (this->BackgroundLayer)
    {
    this->BackgroundLayer->SetSliceNode(newSliceNode);
    }
  if (this->ForegroundLayer)
    {
    this->ForegroundLayer->SetSliceNode(newSliceNode);
    }
  if (this->LabelLayer)
    {
    this->LabelLayer->SetSliceNode(newSliceNode);
    }

  this->Modified();

}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetSliceCompositeNode(vtkMRMLSliceCompositeNode *sliceCompositeNode)
{
  // Observe the composite node, since this holds the parameters for this pipeline
  vtkSetAndObserveMRMLNodeMacro( this->SliceCompositeNode, sliceCompositeNode );
  this->UpdatePipeline();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetBackgroundLayer(vtkMRMLSliceLayerLogic *backgroundLayer)
{
  if (this->BackgroundLayer)
    {
    this->BackgroundLayer->SetAndObserveMRMLScene( 0 );
    this->BackgroundLayer->Delete();
    }
  this->BackgroundLayer = backgroundLayer;

  if (this->BackgroundLayer)
    {
    this->BackgroundLayer->Register(this);

    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
    events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
    events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
    events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
    this->BackgroundLayer->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
    events->Delete();

    this->BackgroundLayer->SetSliceNode(SliceNode);
    vtkEventBroker::GetInstance()->AddObservation(
        this->BackgroundLayer, vtkCommand::ModifiedEvent, this, this->GetLogicCallbackCommand());
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetForegroundLayer(vtkMRMLSliceLayerLogic *foregroundLayer)
{
  if (this->ForegroundLayer)
    {
    this->ForegroundLayer->SetAndObserveMRMLScene( 0 );
    this->ForegroundLayer->Delete();
    }
  this->ForegroundLayer = foregroundLayer;

  if (this->ForegroundLayer)
    {
    this->ForegroundLayer->Register(this);

    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
    events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
    events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
    events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
    this->ForegroundLayer->SetAndObserveMRMLSceneEvents( this->GetMRMLScene(), events);
    events->Delete();

    this->ForegroundLayer->SetSliceNode(SliceNode);
    vtkEventBroker::GetInstance()->AddObservation(
        this->ForegroundLayer, vtkCommand::ModifiedEvent, this, this->GetLogicCallbackCommand());
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetLabelLayer(vtkMRMLSliceLayerLogic *labelLayer)
{
  if (this->LabelLayer)
    {
    this->LabelLayer->SetAndObserveMRMLScene( 0 );
    this->LabelLayer->Delete();
    }
  this->LabelLayer = labelLayer;

  if (this->LabelLayer)
    {
    this->LabelLayer->Register(this);

    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
    events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
    events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
    events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
    this->LabelLayer->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
    events->Delete();

    this->LabelLayer->SetSliceNode(SliceNode);
    vtkEventBroker::GetInstance()->AddObservation(
        this->LabelLayer, vtkCommand::ModifiedEvent, this, this->GetLogicCallbackCommand());
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetForegroundOpacity(double foregroundOpacity)
{
  this->ForegroundOpacity = foregroundOpacity;

  if ( this->Blend->GetOpacity(1) != this->ForegroundOpacity )
    {
    this->Blend->SetOpacity(1, this->ForegroundOpacity);
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetLabelOpacity(double labelOpacity)
{
  this->LabelOpacity = labelOpacity;

  if ( this->Blend->GetOpacity(2) != this->LabelOpacity )
    {
    this->Blend->SetOpacity(2, this->LabelOpacity);
    this->Modified();
    }
}

//----------------------------------------------------------------------------
vtkImageData * vtkMRMLSliceLogic::GetImageData()
{
   if ( (this->GetBackgroundLayer() != 0 && this->GetBackgroundLayer()->GetImageData() != 0) ||
       (this->GetForegroundLayer() != 0 && this->GetForegroundLayer()->GetImageData() != 0) ||
       (this->GetLabelLayer() != 0 && this->GetLabelLayer()->GetImageData() != 0) )
    {
    return this->ImageData;
    }
   else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdateImageData ()
{
  if ( (this->GetBackgroundLayer() != 0 && this->GetBackgroundLayer()->GetImageData() != 0) ||
       (this->GetForegroundLayer() != 0 && this->GetForegroundLayer()->GetImageData() != 0) ||
       (this->GetLabelLayer() != 0 && this->GetLabelLayer()->GetImageData() != 0) )
    {
    if ( this->Blend->GetInput(0) != 0 )
      {
      this->Blend->Update();
      }
    //this->ImageData = this->Blend->GetOutput();
    if (this->ImageData== 0 || this->Blend->GetOutput()->GetMTime() > this->ImageData->GetMTime())
      {
      if (this->ImageData== 0)
        {
        this->ImageData = vtkImageData::New();
        }
      this->ImageData->DeepCopy( this->Blend->GetOutput());
      this->ExtractModelTexture->SetInput( this->ImageData );
      vtkTransform *activeSliceTransform = vtkTransform::New();
      activeSliceTransform->Identity();
      activeSliceTransform->Translate(0, 0, this->SliceNode->GetActiveSlice() );
      this->ExtractModelTexture->SetResliceTransform( activeSliceTransform );
      activeSliceTransform->Delete();
      }
    }
  else
    {
    if (this->ImageData)
      {
      this->ImageData->Delete();
      }
    this->ImageData=0;
    this->ExtractModelTexture->SetInput( this->ImageData );
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdatePipeline()
{
  int modified = 0;

  if ( this->SliceCompositeNode )
    {
    // get the background and foreground image data from the layers
    // so we can use them as input to the image blend
    // TODO: change logic to use a volume node superclass rather than
    // a scalar volume node once the superclass is sorted out for vector/tensor Volumes

    const char *id;
    
    // Background
    id = this->SliceCompositeNode->GetBackgroundVolumeID();
    vtkMRMLVolumeNode *bgnode = 0;
    if (id)
      {
      bgnode = vtkMRMLVolumeNode::SafeDownCast (this->GetMRMLScene()->GetNodeByID(id));
      }
    
    if (this->BackgroundLayer)
      {
      if ( this->BackgroundLayer->GetVolumeNode() != bgnode ) 
        {
        this->BackgroundLayer->SetVolumeNode (bgnode);
        modified = 1;
        }
      }
    
    // Foreground
    id = this->SliceCompositeNode->GetForegroundVolumeID();
    vtkMRMLVolumeNode *fgnode = 0;
    if (id)
      {
      fgnode = vtkMRMLVolumeNode::SafeDownCast (this->GetMRMLScene()->GetNodeByID(id));
      }
    
    if (this->ForegroundLayer)
      {
      if ( this->ForegroundLayer->GetVolumeNode() != fgnode ) 
        {
        this->ForegroundLayer->SetVolumeNode (fgnode);
        modified = 1;
        }
      }
      
    // Label
    id = this->SliceCompositeNode->GetLabelVolumeID();
    vtkMRMLVolumeNode *lbnode = 0;
    if (id)
      {
      lbnode = vtkMRMLVolumeNode::SafeDownCast (this->GetMRMLScene()->GetNodeByID(id));
      }
    
    if (this->LabelLayer)
      {
      if ( this->LabelLayer->GetVolumeNode() != lbnode ) 
        {
        this->LabelLayer->SetVolumeNode (lbnode);
        modified = 1;
        }
      }

    // update the slice intersection visibility to track the composite node setting
    if ( this->SliceModelNode )
      {
      vtkMRMLModelDisplayNode *modelDisplayNode = this->SliceModelNode->GetModelDisplayNode();
      if ( modelDisplayNode )
        {
        if ( this->SliceCompositeNode != 0 )
          {
          modelDisplayNode->SetSliceIntersectionVisibility( this->SliceCompositeNode->GetSliceIntersectionVisibility() );
          }
        }
      }

    // Now update the image blend with the background and foreground and label
    // -- layer 0 opacity is ignored, but since not all inputs may be non-0,
    //    we keep track so that someone could, for example, have a 0 background
    //    with a non-0 foreground and label and everything will work with the
    //    label opacity
    //

    const int sliceCompositing = this->SliceCompositeNode->GetCompositing();
    // alpha blend or reverse alpha blend
    bool alphaBlending = (sliceCompositing == vtkMRMLSliceCompositeNode::Alpha ||
                          sliceCompositing == vtkMRMLSliceCompositeNode::ReverseAlpha);
    
    vtkImageData* backgroundImage = this->BackgroundLayer ? this->BackgroundLayer->GetImageData() : 0;
    vtkImageData* foregroundImage = this->ForegroundLayer ? this->ForegroundLayer->GetImageData() : 0;
    if (!alphaBlending)
      {
      if (!backgroundImage || !foregroundImage)
        {
        // not enough inputs for add/subtract, so use alpha blending
        // pipeline
        alphaBlending = true;
        }
      }
    unsigned long int oldBlendMTime = this->Blend->GetMTime();

    int layerIndex = 0;

    if (!alphaBlending)
      {
      vtkImageMathematics *tempMath = vtkImageMathematics::New();
      if (sliceCompositing == vtkMRMLSliceCompositeNode::Add)
        {
        // add the foreground and background
        tempMath->SetOperationToAdd();
        }
      else if (sliceCompositing == vtkMRMLSliceCompositeNode::Subtract)
        {
        // subtract the foreground and background
        tempMath->SetOperationToSubtract();
        }
      
      tempMath->SetInput1( foregroundImage );
      tempMath->SetInput2( backgroundImage );
      tempMath->GetOutput()->SetScalarType(VTK_SHORT);
      
      vtkImageCast *tempCast = vtkImageCast::New();
      tempCast->SetInput( tempMath->GetOutput() );
      tempCast->SetOutputScalarTypeToUnsignedChar();
      
      this->Blend->SetInput( layerIndex, tempCast->GetOutput() );
      this->Blend->SetOpacity( layerIndex++, 1.0 );
        
      tempMath->Delete();  // Blend may still be holding a reference
      tempCast->Delete();  // Blend may still be holding a reference
      }
    else
      {
      if (sliceCompositing ==  vtkMRMLSliceCompositeNode::Alpha)
        {
        if ( backgroundImage )
          {
          this->Blend->SetInput( layerIndex, backgroundImage );
          this->Blend->SetOpacity( layerIndex++, 1.0 );
          }
        if ( foregroundImage )
          {
          this->Blend->SetInput( layerIndex, foregroundImage );
          this->Blend->SetOpacity( layerIndex++, this->SliceCompositeNode->GetForegroundOpacity() );
          }
        }
      else if (sliceCompositing == vtkMRMLSliceCompositeNode::ReverseAlpha)
        {
        if ( foregroundImage )
          {
          this->Blend->SetInput( layerIndex, foregroundImage );
          this->Blend->SetOpacity( layerIndex++, 1.0 );
          }
        if ( backgroundImage )
          {
          this->Blend->SetInput( layerIndex, backgroundImage );
          this->Blend->SetOpacity( layerIndex++, this->SliceCompositeNode->GetForegroundOpacity() );
          }
        
        }
      }
    // always blending the label layer
    vtkImageData* labelImage = this->LabelLayer ? this->LabelLayer->GetImageData() : 0;
    if ( labelImage )
      {
      this->Blend->SetInput( layerIndex, labelImage );
      this->Blend->SetOpacity( layerIndex++, this->SliceCompositeNode->GetLabelOpacity() );
      }
    while (this->Blend->GetNumberOfInputs() > layerIndex)
      {
      // it decreases the number of inputs
      this->Blend->SetInput(this->Blend->GetNumberOfInputs() - 1, 0);
      }
    if (this->Blend->GetMTime() > oldBlendMTime)
      {
      modified = 1;
      }

    //Models
    this->UpdateImageData();
    vtkMRMLDisplayNode* displayNode = this->SliceModelNode ? this->SliceModelNode->GetModelDisplayNode() : 0;
    if ( displayNode && this->SliceNode )
      {
      if (displayNode->GetVisibility() != this->SliceNode->GetSliceVisible() )
        {
        displayNode->SetVisibility( this->SliceNode->GetSliceVisible() );
        }

      if (!((backgroundImage != 0) || (foregroundImage != 0) || (labelImage != 0) )  )
        {
        displayNode->SetAndObserveTextureImageData(0);
        }
      else if (displayNode->GetTextureImageData() != this->ExtractModelTexture->GetOutput())
        {
        // upadte texture
        this->ExtractModelTexture->Update();
        displayNode->SetAndObserveTextureImageData(this->ExtractModelTexture->GetOutput());
        }
       
      }
    if ( modified )
      {
      if (this->SliceModelNode && this->SliceModelNode->GetPolyData())
        {
        this->SliceModelNode->GetPolyData()->Modified();
        }
      this->Modified();
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  vtkIndent nextIndent;
  nextIndent = indent.GetNextIndent();

  os << indent << "SlicerSliceLogic:             " << this->GetClassName() << "\n";

  if (this->SliceNode)
    {
    os << indent << "SliceNode: ";
    os << (this->SliceNode->GetID() ? this->SliceNode->GetID() : "(0 ID)") << "\n";
    this->SliceNode->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "SliceNode: (none)\n";
    }

  if (this->SliceCompositeNode)
    {
    os << indent << "SliceCompositeNode: ";
    os << (this->SliceCompositeNode->GetID() ? this->SliceCompositeNode->GetID() : "(0 ID)") << "\n";
    this->SliceCompositeNode->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "SliceCompositeNode: (none)\n";
    }

  if (this->BackgroundLayer)
    {
    os << indent << "BackgroundLayer: ";
    this->BackgroundLayer->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "BackgroundLayer: (none)\n";
    }

  if (this->ForegroundLayer)
    {
    os << indent << "ForegroundLayer: ";
    this->ForegroundLayer->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "ForegroundLayer: (none)\n";
    }

  if (this->LabelLayer)
    {
    os << indent << "LabelLayer: ";
    this->LabelLayer->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "LabelLayer: (none)\n";
    }

  if (this->Blend)
    {
    os << indent << "Blend: ";
    this->Blend->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "Blend: (none)\n";
    }

  os << indent << "ForegroundOpacity: " << this->ForegroundOpacity << "\n";
  os << indent << "LabelOpacity: " << this->LabelOpacity << "\n";

}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::DeleteSliceModel()
{
  // Remove References
  if (this->SliceModelNode != 0)
    {
    this->SliceModelNode->SetAndObserveDisplayNodeID(0);
    this->SliceModelNode->SetAndObserveTransformNodeID(0);
    this->SliceModelNode->SetAndObservePolyData(0);
    }
  if (this->SliceModelDisplayNode != 0)
    {
    this->SliceModelDisplayNode->SetAndObserveTextureImageData(0);
    }

  // Remove Nodes 
  if (this->SliceModelNode != 0)
    {
    if (this->GetMRMLScene() && this->GetMRMLScene()->IsNodePresent(this->SliceModelNode))
      {
      this->GetMRMLScene()->RemoveNode(this->SliceModelNode);
      }
    this->SliceModelNode->Delete();
    this->SliceModelNode = 0;
    }
  if (this->SliceModelDisplayNode != 0)
    {
    if (this->GetMRMLScene() && this->GetMRMLScene()->IsNodePresent(this->SliceModelDisplayNode))
      {
      this->GetMRMLScene()->RemoveNode(this->SliceModelDisplayNode);
      }
    this->SliceModelDisplayNode->Delete();
    this->SliceModelDisplayNode = 0;
    }
  if (this->SliceModelTransformNode != 0)
    {
    if (this->GetMRMLScene() && this->GetMRMLScene()->IsNodePresent(this->SliceModelTransformNode))
      {
      this->GetMRMLScene()->RemoveNode(this->SliceModelTransformNode);
      }
    this->SliceModelTransformNode->Delete();
    this->SliceModelTransformNode = 0;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::CreateSliceModel()
{
  assert(this->GetMRMLScene());
  if(!this->GetMRMLScene())
    {
    return;
    }

  if (this->SliceModelNode != 0 &&
      this->GetMRMLScene()->GetNodeByID(this->GetSliceModelNode()->GetID()) == 0 )
    {
    this->DeleteSliceModel();
    }

  if ( this->SliceModelNode == 0)
    {
    this->SliceModelNode = vtkMRMLModelNode::New();
    this->SliceModelNode->SetScene(this->GetMRMLScene());
    this->SliceModelNode->SetDisableModifiedEvent(1);

    this->SliceModelNode->SetHideFromEditors(1);
    this->SliceModelNode->SetSelectable(0);
    this->SliceModelNode->SetSaveWithScene(0);

    // create plane slice
    vtkPlaneSource *planeSource;
    planeSource = vtkPlaneSource::New();
    planeSource->GetOutput()->Update();
    this->SliceModelNode->SetAndObservePolyData(planeSource->GetOutput());
    planeSource->Delete();
    this->SliceModelNode->SetDisableModifiedEvent(0);

    // create display node and set texture
    this->SliceModelDisplayNode = vtkMRMLModelDisplayNode::New();
    this->SliceModelDisplayNode->SetScene(this->GetMRMLScene());
    this->SliceModelDisplayNode->SetDisableModifiedEvent(1);

    this->SliceModelDisplayNode->SetPolyData(this->SliceModelNode->GetPolyData());
    this->SliceModelDisplayNode->SetVisibility(0);
    this->SliceModelDisplayNode->SetOpacity(1);
    this->SliceModelDisplayNode->SetColor(1,1,1);
    // try to auto-set the colors based on the slice name
    // cannot use the vtkSlicerColor class since it is in the GUI
    // TODO: need a better mapping than this
    // 
    if (this->Name != 0)
      {
      if ( !strcmp(this->Name, "Red") )
        {
        this->SliceModelDisplayNode->SetColor(0.952941176471, 0.290196078431, 0.2);
        }
      if ( !strcmp(this->Name, "Green") )
        {
        this->SliceModelDisplayNode->SetColor(0.43137254902, 0.690196078431, 0.294117647059);
        }
      if ( !strcmp(this->Name, "Yellow") )
        {
        this->SliceModelDisplayNode->SetColor(0.929411764706, 0.835294117647, 0.298039215686);
        }
      }
    this->SliceModelDisplayNode->SetAmbient(1);
    this->SliceModelDisplayNode->SetBackfaceCulling(0);
    this->SliceModelDisplayNode->SetDiffuse(0);
    this->SliceModelDisplayNode->SetAndObserveTextureImageData(this->ExtractModelTexture->GetOutput());
    this->SliceModelDisplayNode->SetSaveWithScene(0);
    this->SliceModelDisplayNode->SetDisableModifiedEvent(0);

    // Turn slice intersection off by default - there is a higher level GUI control
    // in the SliceCompositeNode that tells if slices should be enabled for a given
    // slice viewer
    this->SliceModelDisplayNode->SetSliceIntersectionVisibility(0);

    std::string name = std::string(this->Name) + " Volume Slice";
    this->SliceModelNode->SetName (name.c_str());

    // make the xy to RAS transform
    this->SliceModelTransformNode = vtkMRMLLinearTransformNode::New();
    this->SliceModelTransformNode->SetScene(this->GetMRMLScene());
    this->SliceModelTransformNode->SetDisableModifiedEvent(1);

    this->SliceModelTransformNode->SetHideFromEditors(1);
    this->SliceModelTransformNode->SetSelectable(0);
    this->SliceModelTransformNode->SetSaveWithScene(0);
    // set the transform for the slice model for use by an image actor in the viewer
    this->SliceModelTransformNode->GetMatrixTransformToParent()->Identity();

    this->SliceModelTransformNode->SetDisableModifiedEvent(0);

    }

  if (this->SliceModelNode != 0 && this->GetMRMLScene()->GetNodeByID( this->GetSliceModelNode()->GetID() ) == 0 )
    {
    this->GetMRMLScene()->AddNode(this->SliceModelDisplayNode);
    this->GetMRMLScene()->AddNode(this->SliceModelTransformNode);
    this->GetMRMLScene()->AddNode(this->SliceModelNode);
    this->SliceModelNode->SetAndObserveDisplayNodeID(this->SliceModelDisplayNode->GetID());
    this->SliceModelDisplayNode->SetAndObserveTextureImageData(this->ExtractModelTexture->GetOutput());
    this->SliceModelNode->SetAndObserveTransformNodeID(this->SliceModelTransformNode->GetID());
    }

  // update the description to refer back to the slice and composite nodes
  // TODO: this doesn't need to be done unless the ID change, but it needs
  // to happen after they have been set, so do it every event for now
  if ( this->SliceModelNode != 0 )
    {
    char description[256];
    std::stringstream ssD;
    vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
    if ( sliceNode && sliceNode->GetID() )
      {
      ssD << " SliceID " << sliceNode->GetID();
      }
    vtkMRMLSliceCompositeNode *compositeNode = this->GetSliceCompositeNode();
    if ( compositeNode && compositeNode->GetID() )
      {
      ssD << " CompositeID " << compositeNode->GetID();
      }

    ssD.getline(description,256);
    this->SliceModelNode->SetDescription(description);
    }
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode *vtkMRMLSliceLogic::GetLayerVolumeNode(int layer)
{
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  vtkMRMLSliceCompositeNode *compositeNode = this->GetSliceCompositeNode();

  if ( !sliceNode || !compositeNode )
    {
    return (0);
    }
  
  char *id = 0;
  switch (layer)
    {
    case 0:
      {
      id = compositeNode->GetBackgroundVolumeID();
      break;
      }
    case 1:
      {
      id = compositeNode->GetForegroundVolumeID();
      break;
      }
    case 2:
      {
      id = compositeNode->GetLabelVolumeID();
      break;
      }
    }
  vtkMRMLScene* scene = this->GetMRMLScene();
  return scene ? vtkMRMLVolumeNode::SafeDownCast(
    scene->GetNodeByID( id )) : 0;
}

//----------------------------------------------------------------------------
// Get the size of the volume, transformed to RAS space
void vtkMRMLSliceLogic::GetVolumeRASBox(vtkMRMLVolumeNode *volumeNode, double rasDimensions[3], double rasCenter[3])
{
  rasCenter[0] = rasDimensions[0] = 0.0;
  rasCenter[1] = rasDimensions[1] = 0.0;
  rasCenter[2] = rasDimensions[2] = 0.0;


  vtkImageData *volumeImage;
  if ( !volumeNode || ! (volumeImage = volumeNode->GetImageData()) )
    {
    return;
    }

  //
  // Get the size of the volume in RAS space
  // - map the size of the volume in IJK into RAS
  // - map the middle of the volume to RAS for the center
  //   (IJK space always has origin at first pixel)
  //
  vtkSmartPointer<vtkMatrix4x4> ijkToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
  volumeNode->GetIJKToRASMatrix (ijkToRAS);
  vtkMRMLTransformNode *transformNode = volumeNode->GetParentTransformNode();
  if ( transformNode )
    {
    vtkSmartPointer<vtkMatrix4x4> rasToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
    transformNode->GetMatrixTransformToWorld(rasToRAS);
    vtkMatrix4x4::Multiply4x4 (rasToRAS, ijkToRAS, ijkToRAS);
    }

  int dimensions[3];
  int i,j,k;
  volumeImage->GetDimensions(dimensions);
  double doubleDimensions[4], rasHDimensions[4];
  double minBounds[3], maxBounds[3];

  for ( i=0; i<3; i++) 
    {
    minBounds[i] = 1.0e10;
    maxBounds[i] = -1.0e10;
    }
  for ( i=0; i<2; i++) 
    {
    for ( j=0; j<2; j++) 
      {
      for ( k=0; k<2; k++) 
        {
        doubleDimensions[0] = i*(dimensions[0] - 1);
        doubleDimensions[1] = j*(dimensions[1] - 1);
        doubleDimensions[2] = k*(dimensions[2] - 1);
        doubleDimensions[3] = 1;
        ijkToRAS->MultiplyPoint( doubleDimensions, rasHDimensions );
        for (int n=0; n<3; n++) {
          if (rasHDimensions[n] < minBounds[n])
            {
            minBounds[n] = rasHDimensions[n];
            }
          if (rasHDimensions[n] > maxBounds[n])
            {
            maxBounds[n] = rasHDimensions[n];
            }
          }
        }
      }
    }
  
   for ( i=0; i<3; i++) 
    {
    rasDimensions[i] = maxBounds[i] - minBounds[i];
    rasCenter[i] = 0.5*(maxBounds[i] + minBounds[i]);
    }
}

//----------------------------------------------------------------------------
// Get the size of the volume, transformed to RAS space
void vtkMRMLSliceLogic::GetVolumeSliceDimensions(vtkMRMLVolumeNode *volumeNode, double sliceDimensions[3], double sliceCenter[3])
{
  sliceCenter[0] = sliceDimensions[0] = 0.0;
  sliceCenter[1] = sliceDimensions[1] = 0.0;
  sliceCenter[2] = sliceDimensions[2] = 0.0;

  double sliceBounds[6];

  this->GetVolumeSliceBounds(volumeNode, sliceBounds);

  for (int i=0; i<3; i++) 
    {
    sliceDimensions[i] = sliceBounds[2*i+1] - sliceBounds[2*i];
    sliceCenter[i] = 0.5*(sliceBounds[2*i+1] + sliceBounds[2*i]);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::GetVolumeSliceBounds(vtkMRMLVolumeNode *volumeNode, double sliceBounds[6])
{
  sliceBounds[0] = sliceBounds[1] = 0.0;
  sliceBounds[2] = sliceBounds[3] = 0.0;
  sliceBounds[4] = sliceBounds[5] = 0.0;

  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();

  if ( !sliceNode )
    {
    return;
    }
  
  double rasDimensions[3], rasCenter[3];

  this->GetVolumeRASBox(volumeNode, rasDimensions, rasCenter);

  //
  // figure out how big that volume is on this particular slice plane
  //
  vtkSmartPointer<vtkMatrix4x4> rasToSlice = vtkSmartPointer<vtkMatrix4x4>::New();
  rasToSlice->DeepCopy(sliceNode->GetSliceToRAS());
  rasToSlice->SetElement(0, 3, 0.0);
  rasToSlice->SetElement(1, 3, 0.0);
  rasToSlice->SetElement(2, 3, 0.0);
  rasToSlice->Invert();

  double minBounds[3], maxBounds[3];
  double rasCorner[4], sliceCorner[4];
  int i,j,k;

  for ( i=0; i<3; i++) 
    {
    minBounds[i] = 1.0e10;
    maxBounds[i] = -1.0e10;
    }
  for ( i=-1; i<=1; i=i+2) 
    {
    for ( j=-1; j<=1; j=j+2) 
      {
      for ( k=-1; k<=1; k=k+2) 
        {
        rasCorner[0] = rasCenter[0] + i * rasDimensions[0] / 2.;
        rasCorner[1] = rasCenter[1] + j * rasDimensions[1] / 2.;
        rasCorner[2] = rasCenter[2] + k * rasDimensions[2] / 2.;
        rasCorner[3] = 1.;

        rasToSlice->MultiplyPoint( rasCorner, sliceCorner );

        for (int n=0; n<3; n++) {
          if (sliceCorner[n] < minBounds[n])
            {
            minBounds[n] = sliceCorner[n];
            }
          if (sliceCorner[n] > maxBounds[n])
            {
            maxBounds[n] = sliceCorner[n];
            }
          }
        }
      }
    }

  // ignore homogeneous coordinate
  sliceBounds[0] = minBounds[0];
  sliceBounds[1] = maxBounds[0];
  sliceBounds[2] = minBounds[1];
  sliceBounds[3] = maxBounds[1];
  sliceBounds[4] = minBounds[2];
  sliceBounds[5] = maxBounds[2];
}

//----------------------------------------------------------------------------
// Get the spacing of the volume, transformed to slice space
double *vtkMRMLSliceLogic::GetVolumeSliceSpacing(vtkMRMLVolumeNode *volumeNode)
{
  if ( !volumeNode )
    {
    return (this->SliceSpacing);
    }

  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();

  if ( !sliceNode )
    {
    return (this->SliceSpacing);
    }

  if (sliceNode->GetSliceSpacingMode() == vtkMRMLSliceNode::PrescribedSliceSpacingMode)
    {
    // jvm - should we cache the PrescribedSliceSpacing in SliceSpacing?
    double *pspacing = sliceNode->GetPrescribedSliceSpacing();
    this->SliceSpacing[0] = pspacing[0];
    this->SliceSpacing[1] = pspacing[1];
    this->SliceSpacing[2] = pspacing[2];
    return (pspacing);
    }
  
  vtkSmartPointer<vtkMatrix4x4> ijkToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
  vtkSmartPointer<vtkMatrix4x4> rasToSlice = vtkSmartPointer<vtkMatrix4x4>::New();
  vtkSmartPointer<vtkMatrix4x4> ijkToSlice = vtkSmartPointer<vtkMatrix4x4>::New();

  volumeNode->GetIJKToRASMatrix(ijkToRAS);

  // Apply the transform, if it exists
  vtkMRMLTransformNode *transformNode = volumeNode->GetParentTransformNode();
  if ( transformNode != 0 )
    {
    if ( transformNode->IsTransformToWorldLinear() )
      {
      vtkSmartPointer<vtkMatrix4x4> rasToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
      transformNode->GetMatrixTransformToWorld( rasToRAS );
      rasToRAS->Invert();
      vtkMatrix4x4::Multiply4x4(rasToRAS, ijkToRAS, ijkToRAS); 
      }
    }

  rasToSlice->DeepCopy(sliceNode->GetSliceToRAS());
  rasToSlice->Invert();

  ijkToSlice->Multiply4x4(rasToSlice, ijkToRAS, ijkToSlice);

  double invector[4];
  invector[0] = invector[1] = invector[2] = 1.0;
  invector[3] = 0.0;
  double spacing[4];
  ijkToSlice->MultiplyPoint(invector, spacing);
  int i;
  for (i = 0; i < 3; i++)
    {
    this->SliceSpacing[i] = fabs(spacing[i]);
    }

  return (this->SliceSpacing);
}

//----------------------------------------------------------------------------
// adjust the node's field of view to match the extent of current volume
void vtkMRMLSliceLogic::FitSliceToVolume(vtkMRMLVolumeNode *volumeNode, int width, int height)
{
  vtkImageData *volumeImage;
  if ( !volumeNode || ! (volumeImage = volumeNode->GetImageData()) )
    {
    return;
    }

  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();

  if ( !sliceNode )
    {
    return;
    }

  double rasDimensions[3], rasCenter[3];
  this->GetVolumeRASBox (volumeNode, rasDimensions, rasCenter);
  double sliceDimensions[3], sliceCenter[3];
  this->GetVolumeSliceDimensions (volumeNode, sliceDimensions, sliceCenter);

  double fitX, fitY, fitZ, displayX, displayY;
  displayX = fitX = fabs(sliceDimensions[0]);
  displayY = fitY = fabs(sliceDimensions[1]);
  fitZ = this->GetVolumeSliceSpacing(volumeNode)[2] * sliceNode->GetDimensions()[2];


  // fit fov to min dimension of window
  double pixelSize;
  if ( height > width )
    {
    pixelSize = fitX / (1.0 * width);
    fitY = pixelSize * height;
    }
  else
    {
    pixelSize = fitY / (1.0 * height);
    fitX = pixelSize * width;
    }

  // if volume is still too big, shrink some more
  if ( displayX > fitX )
    {
    fitY = fitY / ( fitX / (displayX * 1.0) );
    fitX = displayX;
    }
  if ( displayY > fitY )
    {
    fitX = fitX / ( fitY / (displayY * 1.0) );
    fitY = displayY;
    }

  sliceNode->SetFieldOfView(fitX, fitY, fitZ);

  //
  // set the origin to be the center of the volume in RAS
  //
  vtkSmartPointer<vtkMatrix4x4> sliceToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
  sliceToRAS->DeepCopy(sliceNode->GetSliceToRAS());
  sliceToRAS->SetElement(0, 3, rasCenter[0]);
  sliceToRAS->SetElement(1, 3, rasCenter[1]);
  sliceToRAS->SetElement(2, 3, rasCenter[2]);
  sliceNode->GetSliceToRAS()->DeepCopy(sliceToRAS);
  sliceNode->UpdateMatrices( );
}

//----------------------------------------------------------------------------
// Get the size of the volume, transformed to RAS space
void vtkMRMLSliceLogic::GetBackgroundRASBox(double rasDimensions[3], double rasCenter[3])
{
  vtkMRMLVolumeNode *backgroundNode = 0;
  backgroundNode = this->GetLayerVolumeNode (0);
  this->GetVolumeRASBox( backgroundNode, rasDimensions, rasCenter );
}

//----------------------------------------------------------------------------
// Get the size of the volume, transformed to RAS space
void vtkMRMLSliceLogic::GetBackgroundSliceDimensions(double sliceDimensions[3], double sliceCenter[3])
{
  vtkMRMLVolumeNode *backgroundNode = 0;
  backgroundNode = this->GetLayerVolumeNode (0);
  this->GetVolumeSliceDimensions( backgroundNode, sliceDimensions, sliceCenter );
}

//----------------------------------------------------------------------------
// Get the spacing of the volume, transformed to slice space
double *vtkMRMLSliceLogic::GetBackgroundSliceSpacing()
{
  vtkMRMLVolumeNode *backgroundNode = 0;
  backgroundNode = this->GetLayerVolumeNode (0);
  return (this->GetVolumeSliceSpacing( backgroundNode ));
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::GetBackgroundSliceBounds(double sliceBounds[6])
{
  vtkMRMLVolumeNode *backgroundNode = 0;
  backgroundNode = this->GetLayerVolumeNode (0);
  this->GetVolumeSliceBounds(backgroundNode, sliceBounds);
}

//----------------------------------------------------------------------------
// adjust the node's field of view to match the extent of current background volume
void vtkMRMLSliceLogic::FitSliceToBackground(int width, int height)
{
  vtkMRMLVolumeNode *backgroundNode = 0;
  backgroundNode = this->GetLayerVolumeNode (0);
  this->FitSliceToVolume( backgroundNode, width, height );
}

//----------------------------------------------------------------------------
// adjust the node's field of view to match the extent of all volume layers
void vtkMRMLSliceLogic::FitSliceToAll(int width, int height)
{
  // Use SliceNode dimensions if width and height parameters are omitted
  if (width < 0 || height < 0)
    {
    int* dimensions = this->SliceNode->GetDimensions();
    width = dimensions ? dimensions[0] : -1;
    height = dimensions ? dimensions[1] : -1;
    }

  if (width < 0 || height < 0)
    {
    vtkErrorMacro(<< __FUNCTION__ << "- Invalid size:" << width
                  << "x" << height);
    return;
    }

  vtkMRMLVolumeNode *volumeNode;
  for ( int layer=0; layer < 3; layer++ )
    {
    volumeNode = this->GetLayerVolumeNode (layer);
    if (volumeNode)
      {
      this->FitSliceToVolume( volumeNode, width, height );
      return;
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::FitFOVToBackground(double fov)
{
  // get backgroundNode  and imagedata
  vtkMRMLScalarVolumeNode* backgroundNode =
    vtkMRMLScalarVolumeNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID(
        this->SliceCompositeNode->GetBackgroundVolumeID() ));
  vtkImageData *backgroundImage =
    backgroundNode ? backgroundNode->GetImageData() : 0;
  if (!backgroundImage)
    {
    return;
    }
  // get viewer's width and height. we may be using a LightBox
  // display, so base width and height on renderer0 in the SliceViewer.
  int width = this->SliceNode->GetDimensions()[0];
  int height = this->SliceNode->GetDimensions()[1];

  int dimensions[3];
  double rasDimensions[4];
  double doubleDimensions[4];
  vtkSmartPointer<vtkMatrix4x4> ijkToRAS =
    vtkSmartPointer<vtkMatrix4x4>::New();

  // what are the actual dimensions of the imagedata?
  backgroundImage->GetDimensions(dimensions);
  doubleDimensions[0] = static_cast<double>(dimensions[0]);
  doubleDimensions[1] = static_cast<double>(dimensions[1]);
  doubleDimensions[2] = static_cast<double>(dimensions[2]);
  doubleDimensions[3] = 0.0;
  backgroundNode->GetIJKToRASMatrix(ijkToRAS);
  ijkToRAS->MultiplyPoint(doubleDimensions, rasDimensions);

  // and what are their slice dimensions?
  vtkSmartPointer<vtkMatrix4x4> rasToSlice =
    vtkSmartPointer<vtkMatrix4x4>::New();
  double sliceDimensions[4];
  rasToSlice->DeepCopy(this->SliceNode->GetSliceToRAS());
  rasToSlice->SetElement(0, 3, 0.0);
  rasToSlice->SetElement(1, 3, 0.0);
  rasToSlice->SetElement(2, 3, 0.0);
  rasToSlice->Invert();
  rasToSlice->MultiplyPoint(rasDimensions, sliceDimensions);

  double fovh, fovv;
  // which is bigger, slice viewer width or height?
  // assign user-specified fov to smaller slice window
  // dimension
  if ( width < height )
    {
    fovh = fov;
    fovv = fov * height/width;
    }
  else
    {
    fovv = fov;
    fovh = fov * width/height;
    }
  // we want to compute the slice dimensions of the
  // user-specified fov (note that the slice node's z field of
  // view is NOT changed)
  this->SliceNode->SetFieldOfView(fovh, fovv, this->SliceNode->GetFieldOfView()[2]);

  vtkSmartPointer<vtkMatrix4x4> sliceToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
  sliceToRAS->DeepCopy(this->SliceNode->GetSliceToRAS());
  this->SliceNode->GetSliceToRAS()->DeepCopy(sliceToRAS);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::ResizeSliceNode(double newWidth, double newHeight)
{
  if (!this->SliceNode)
    {
    return;
    }

  // New size must be the active slice vtkRenderer size. It's the same than the window
  // if the layout is 1x1.
  newWidth /= this->SliceNode->GetLayoutGridColumns();
  newHeight /= this->SliceNode->GetLayoutGridRows();

  // The following was previously in SliceSWidget.tcl
  double sliceStep = this->SliceSpacing[2];
  int oldDimensions[3];
  this->SliceNode->GetDimensions(oldDimensions);
  double oldFOV[3];
  this->SliceNode->GetFieldOfView(oldFOV);

  double scalingX = (oldDimensions[0] != 0 ? newWidth / oldDimensions[0] : 1.);
  double scalingY = (oldDimensions[1] != 0 ? newHeight / oldDimensions[1] : 1.);

  double magnitudeX = (scalingX >= 1. ? scalingX : 1. / scalingX);
  double magnitudeY = (scalingY >= 1. ? scalingY : 1. / scalingY);

  double newFOV[3];
  if (magnitudeX < magnitudeY)
    {
    newFOV[0] = oldFOV[0];
    newFOV[1] = oldFOV[1] * scalingY / scalingX;
    }
  else
    {
    newFOV[0] = oldFOV[0] * scalingX / scalingY;
    newFOV[1] = oldFOV[1];
    }
  newFOV[2] = sliceStep * oldDimensions[2];
  double windowAspect = (newWidth != 0. ? newHeight / newWidth : 1.);
  double planeAspect = (newFOV[0] != 0. ? newFOV[1] / newFOV[0] : 1.);
  if (windowAspect != planeAspect)
    {
    newFOV[0] = (windowAspect != 0. ? newFOV[1] / windowAspect : newFOV[0]);
    }
  int disabled = this->SliceNode->StartModify();
  this->SliceNode->SetDimensions(newWidth, newHeight, oldDimensions[2]);
  this->SliceNode->SetFieldOfView(newFOV[0], newFOV[1], newFOV[2]);
  this->SliceNode->EndModify(disabled);
}

//----------------------------------------------------------------------------
double *vtkMRMLSliceLogic::GetLowestVolumeSliceSpacing()
{
  vtkMRMLVolumeNode *volumeNode;
  for ( int layer=0; layer < 3; layer++ )
    {
    volumeNode = this->GetLayerVolumeNode (layer);
    if (volumeNode)
      {
      return this->GetVolumeSliceSpacing( volumeNode );
      }
    }
  return (this->SliceSpacing);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::GetLowestVolumeSliceBounds(double sliceBounds[6])
{
  vtkMRMLVolumeNode *volumeNode;
  for ( int layer=0; layer < 3; layer++ )
    {
    volumeNode = this->GetLayerVolumeNode (layer);
    if (volumeNode)
      {
      return this->GetVolumeSliceBounds( volumeNode, sliceBounds );
      }
    }
  // return the default values
  return this->GetVolumeSliceBounds( 0, sliceBounds );
}

//----------------------------------------------------------------------------
// Get/Set the current distance from the origin to the slice plane
double vtkMRMLSliceLogic::GetSliceOffset()
{
  // this method has been moved to vtkMRMLSliceNode
  // the API stays for backwards compatibility

  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();

  if ( !sliceNode )
    {
    return 0.0;
    }

  return sliceNode->GetSliceOffset();

}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetSliceOffset(double offset)
{

  // this method has been moved to vtkMRMLSliceNode
  // the API stays for backwards compatibility

  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();

  if ( !sliceNode )
    {
    return;
    }

  this->GetSliceNode()->SetSliceOffset(offset);

}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SnapSliceOffsetToIJK()
{
  double offset, *spacing, bounds[6];
  double oldOffset = this->GetSliceOffset();
  spacing = this->GetLowestVolumeSliceSpacing();
  this->GetLowestVolumeSliceBounds( bounds );
  
  // number of slices along the offset dimension (depends on ijkToRAS and Transforms)
  double slice = (oldOffset - bounds[4]) / spacing[2];
  int intSlice = static_cast<int> (0.5 + slice);  
  offset = intSlice * spacing[2] + bounds[4];
  this->SetSliceOffset( offset );
}



//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::GetPolyDataAndLookUpTableCollections(vtkPolyDataCollection *polyDataCollection,
                                                               vtkCollection *lookupTableCollection)
{
  this->PolyDataCollection->RemoveAllItems();
  this->LookupTableCollection->RemoveAllItems();
  
  // Add glyphs. Get them from Background or Foreground slice layers.
  vtkMRMLSliceLayerLogic *layerLogic = this->GetBackgroundLayer();
  this->AddSliceGlyphs(layerLogic);
  
  layerLogic = this->GetForegroundLayer();
  this->AddSliceGlyphs(layerLogic);
  
  polyDataCollection = this->PolyDataCollection;
  lookupTableCollection = this->LookupTableCollection;
} 

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::AddSliceGlyphs(vtkMRMLSliceLayerLogic *layerLogic)
{
 if (layerLogic && layerLogic->GetVolumeNode()) 
    {
    vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast (layerLogic->GetVolumeNode());
    vtkMRMLGlyphableVolumeDisplayNode *displayNode = vtkMRMLGlyphableVolumeDisplayNode::SafeDownCast( layerLogic->GetVolumeNode()->GetDisplayNode() );
    if (displayNode)
      {
      std::vector< vtkMRMLGlyphableVolumeSliceDisplayNode*> dnodes  = displayNode->GetSliceGlyphDisplayNodes(volumeNode);
      for (unsigned int i=0; i<dnodes.size(); i++)
        {
        vtkMRMLGlyphableVolumeSliceDisplayNode* dnode = dnodes[i];
        if (dnode->GetVisibility() && layerLogic->GetSliceNode() 
          && layerLogic->GetSliceNode()->GetLayoutName() 
          &&!strcmp(layerLogic->GetSliceNode()->GetLayoutName(), dnode->GetName()) )
          {
          vtkPolyData* poly = dnode->GetPolyDataTransformedToSlice();
          if (poly)
            {
            this->PolyDataCollection->AddItem(poly);
            if (dnode->GetColorNode())
              {
              if (dnode->GetColorNode()->GetLookupTable()) 
                {
                this->LookupTableCollection->AddItem(dnode->GetColorNode()->GetLookupTable());
                }
              else if (vtkMRMLProceduralColorNode::SafeDownCast(dnode->GetColorNode())->GetColorTransferFunction())
                {
                this->LookupTableCollection->AddItem((vtkScalarsToColors*)(vtkMRMLProceduralColorNode::SafeDownCast(dnode->GetColorNode())->GetColorTransferFunction()));
                }
              }
            }
            break;
          }
        }
      }//  if (volumeNode)
    }// if (layerLogic && layerLogic->GetVolumeNode()) 
    
}

//----------------------------------------------------------------------------
std::vector< vtkMRMLDisplayNode*> vtkMRMLSliceLogic::GetPolyDataDisplayNodes()
{
  std::vector< vtkMRMLDisplayNode*> nodes;
  std::vector<vtkMRMLSliceLayerLogic *> layerLogics;
  layerLogics.push_back(this->GetBackgroundLayer());
  layerLogics.push_back(this->GetForegroundLayer());
  for (unsigned int i=0; i<layerLogics.size(); i++) 
    {
    vtkMRMLSliceLayerLogic *layerLogic = layerLogics[i];
    if (layerLogic && layerLogic->GetVolumeNode()) 
      {
      vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast (layerLogic->GetVolumeNode());
      vtkMRMLGlyphableVolumeDisplayNode *displayNode = vtkMRMLGlyphableVolumeDisplayNode::SafeDownCast( layerLogic->GetVolumeNode()->GetDisplayNode() );
      if (displayNode)
        {
        std::vector< vtkMRMLGlyphableVolumeSliceDisplayNode*> dnodes  = displayNode->GetSliceGlyphDisplayNodes(volumeNode);
        for (unsigned int n=0; n<dnodes.size(); n++)
          {
          vtkMRMLGlyphableVolumeSliceDisplayNode* dnode = dnodes[n];
          if (layerLogic->GetSliceNode() 
            && layerLogic->GetSliceNode()->GetLayoutName()
            && !strcmp(layerLogic->GetSliceNode()->GetLayoutName(), dnode->GetName()) )
            {
            nodes.push_back(dnode);
            }
          }
        }//  if (volumeNode)
      }// if (layerLogic && layerLogic->GetVolumeNode()) 
    }
  return nodes;
}

//----------------------------------------------------------------------------
int vtkMRMLSliceLogic::GetSliceIndexFromOffset(double sliceOffset, vtkMRMLVolumeNode *volumeNode)
{
  if ( !volumeNode )
    {
    return SLICE_INDEX_NO_VOLUME;
    }
  vtkImageData *volumeImage=0;
  if ( !(volumeImage = volumeNode->GetImageData()) )
    {
    return SLICE_INDEX_NO_VOLUME;
    }
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if ( !sliceNode )
    {
    return SLICE_INDEX_NO_VOLUME;
    }

  vtkSmartPointer<vtkMatrix4x4> ijkToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
  volumeNode->GetIJKToRASMatrix (ijkToRAS);
  vtkMRMLTransformNode *transformNode = volumeNode->GetParentTransformNode();
  if ( transformNode )
    {
    vtkSmartPointer<vtkMatrix4x4> rasToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
    transformNode->GetMatrixTransformToWorld(rasToRAS);
    vtkMatrix4x4::Multiply4x4 (rasToRAS, ijkToRAS, ijkToRAS);
    }

  // Get the slice normal in RAS

  vtkSmartPointer<vtkMatrix4x4> rasToSlice = vtkSmartPointer<vtkMatrix4x4>::New();
  rasToSlice->DeepCopy(sliceNode->GetSliceToRAS());
  rasToSlice->Invert();

  double sliceNormal_IJK[4]={0,0,1,0};  // slice normal vector in IJK coordinate system
  double sliceNormal_RAS[4]={0,0,0,0};  // slice normal vector in RAS coordinate system
  sliceNode->GetSliceToRAS()->MultiplyPoint(sliceNormal_IJK, sliceNormal_RAS);
  
  // Find an axis normal that has the same orientation as the slice normal
  double axisDirection_RAS[3]={0,0,0};  
  int axisIndex=0;  
  double volumeSpacing=1.0; // spacing along axisIndex
  for (axisIndex=0; axisIndex<3; axisIndex++)
    {
    axisDirection_RAS[0]=ijkToRAS->GetElement(0,axisIndex);
    axisDirection_RAS[1]=ijkToRAS->GetElement(1,axisIndex);
    axisDirection_RAS[2]=ijkToRAS->GetElement(2,axisIndex);
    volumeSpacing=vtkMath::Norm(axisDirection_RAS); // spacing along axisIndex
    vtkMath::Normalize(sliceNormal_RAS);
    vtkMath::Normalize(axisDirection_RAS);
    double dotProd=vtkMath::Dot(sliceNormal_RAS, axisDirection_RAS);
    // Due to numerical inaccuracies the dot product of two normalized vectors
    // can be slightly bigger than 1 (and acos cannot be computed) - fix that.
    if (dotProd>1.0)
      {
      dotProd=1.0;
      }
    else if (dotProd<-1.0)
      {
      dotProd=-1.0;
      }
    double axisMisalignmentDegrees=acos(dotProd)*180.0/vtkMath::Pi();
    if (fabs(axisMisalignmentDegrees)<0.1)
      {
      // found an axis that is aligned to the slice normal
      break;
      }
    if (fabs(axisMisalignmentDegrees-180)<0.1 || fabs(axisMisalignmentDegrees+180)<0.1)
      {
      // found an axis that is aligned to the slice normal, just points to the opposite direction
      volumeSpacing*=-1.0;
      break;
      }
    }

  if (axisIndex>=3)
    {
    // no aligned axis is found
    return SLICE_INDEX_ROTATED;
    }
  
  // Determine slice index
  double originPos_RAS[4]={
    ijkToRAS->GetElement( 0, 3 ),
    ijkToRAS->GetElement( 1, 3 ),
    ijkToRAS->GetElement( 2, 3 ),
    0};
  double originPos_Slice[4]={0,0,0,0};
  rasToSlice->MultiplyPoint(originPos_RAS, originPos_Slice);
  double volumeOriginOffset=originPos_Slice[2];
  double sliceShift=sliceOffset-volumeOriginOffset;
  double normalizedSliceShift=sliceShift/volumeSpacing;
  int sliceIndex=vtkMath::Round(normalizedSliceShift)+1; // +0.5 because the slice plane is displayed in the center of the slice
  
  // Check if slice index is within the volume
  int sliceCount=volumeImage->GetDimensions()[axisIndex];
  if (sliceIndex<1 || sliceIndex>sliceCount)
    {
    sliceIndex=SLICE_INDEX_OUT_OF_VOLUME;
    }

  return sliceIndex;
}

//----------------------------------------------------------------------------
// sliceIndex: DICOM slice index, 1-based
int vtkMRMLSliceLogic::GetSliceIndexFromOffset(double sliceOffset)
{
  vtkMRMLVolumeNode *volumeNode;
  for (int layer=0; layer < 3; layer++ )
    {
    volumeNode = this->GetLayerVolumeNode (layer);
    if (volumeNode)
      {
      int sliceIndex=this->GetSliceIndexFromOffset( sliceOffset, volumeNode );
      // return the result for the first available layer
      return sliceIndex;
      }
    }
  // slice is not aligned to any of the layers or out of the volume
  return SLICE_INDEX_NO_VOLUME;
}

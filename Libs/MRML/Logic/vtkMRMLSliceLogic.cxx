/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
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
#include <vtkEventBroker.h>
#include <vtkMRMLCrosshairNode.h>
#include <vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h>
#include <vtkMRMLGlyphableVolumeDisplayNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLProceduralColorNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkImageBlend.h>
#include <vtkImageResample.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageMathematics.h>
#include <vtkImageReslice.h>
#include <vtkInformation.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataCollection.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTransform.h>
#include <vtkVersion.h>

// STD includes

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
const std::string vtkMRMLSliceLogic::SLICE_MODEL_NODE_NAME_SUFFIX = std::string("Volume Slice");

//----------------------------------------------------------------------------
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
  this->Blend = vtkImageBlend::New();
  this->BlendUVW = vtkImageBlend::New();

  this->ExtractModelTexture = vtkImageReslice::New();
  this->ExtractModelTexture->SetOutputDimensionality (2);
  this->ExtractModelTexture->SetInputConnection(BlendUVW->GetOutputPort());

  this->ActiveSliceTransform = vtkTransform::New();
  this->PolyDataCollection = vtkPolyDataCollection::New();
  this->LookupTableCollection = vtkCollection::New();
  this->SliceModelNode = 0;
  this->SliceModelTransformNode = 0;
  this->Name = 0;
  this->SetName("");
  this->SliceModelDisplayNode = 0;
  this->ImageDataConnection = 0;
  this->SliceSpacing[0] = this->SliceSpacing[1] = this->SliceSpacing[2] = 1;
  this->AddingSliceModelNodes = false;
}

//----------------------------------------------------------------------------
vtkMRMLSliceLogic::~vtkMRMLSliceLogic()
{
  this->SetName(0);
  this->SetSliceNode(0);

  if (this->ImageDataConnection)
    {
    this->ImageDataConnection = 0;
    }

  if (this->Blend)
    {
    this->Blend->Delete();
    this->Blend = 0;
    }
  if (this->BlendUVW)
    {
    this->BlendUVW->Delete();
    this->BlendUVW = 0;
    }
  if (this->ExtractModelTexture)
    {
    this->ExtractModelTexture->Delete();
    this->ExtractModelTexture = 0;
    }
  if (this->ActiveSliceTransform)
    {
    this->ActiveSliceTransform->Delete();
    this->ActiveSliceTransform = 0;
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

  this->DeleteSliceModel();
}

//----------------------------------------------------------------------------
// TODO: Remove from API
bool vtkMRMLSliceLogic::IsInitialized()
{
  return this->Initialized;
}

//----------------------------------------------------------------------------
// TODO: Remove from API
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
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  events->InsertNextValue(vtkMRMLScene::StartCloseEvent);
  events->InsertNextValue(vtkMRMLScene::EndImportEvent);
  events->InsertNextValue(vtkMRMLScene::EndRestoreEvent);
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());

  this->ProcessMRMLLogicsEvents();

  this->BackgroundLayer->SetMRMLScene(newScene);
  this->ForegroundLayer->SetMRMLScene(newScene);
  this->LabelLayer->SetMRMLScene(newScene);

  this->ProcessMRMLSceneEvents(newScene, vtkMRMLScene::EndBatchProcessEvent, 0);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdateSliceNode()
{
  if (!this->GetMRMLScene())
    {
    this->SetSliceNode(0);
    return;
    }
  // find SliceNode in the scene
  vtkMRMLSliceNode *node = vtkMRMLSliceLogic::GetSliceNode(this->GetMRMLScene(), this->GetName());

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
      node = vtkMRMLSliceNode::SafeDownCast(
            this->GetMRMLScene()->CreateNodeByClass("vtkMRMLSliceNode"));
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

  if (this->SliceNode->GetNumberOfSliceOrientationPresets() < 3)
    {
    vtkErrorMacro("UpdateSliceNodeFromLayout: " << this->GetName() <<
                  " could not set its slice node orientation. At least 3"
                  " orientation preset names are required.")
    return;
    }

  vtkNew<vtkStringArray> namedOrientations;
  this->SliceNode->GetSliceOrientationPresetNames(namedOrientations.GetPointer());

  if ( !strcmp( this->GetName(), "Red" ) )
    {
    this->SliceNode->SetOrientation(namedOrientations->GetValue(0));
    }
  if ( !strcmp( this->GetName(), "Yellow" ) )
    {
    this->SliceNode->SetOrientation(namedOrientations->GetValue(1));
    }
  if ( !strcmp( this->GetName(), "Green" ) )
    {
    this->SliceNode->SetOrientation(namedOrientations->GetValue(2));
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdateSliceCompositeNode()
{
  if (!this->GetMRMLScene())
    {
    this->SetSliceCompositeNode(0);
    return;
    }
  // find SliceCompositeNode in the scene
  vtkMRMLSliceCompositeNode *node= vtkMRMLSliceLogic::GetSliceCompositeNode(this->GetMRMLScene(), this->GetName());

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
bool vtkMRMLSliceLogic::EnterMRMLCallback()const
{
  return this->AddingSliceModelNodes == false;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdateFromMRMLScene()
{
  this->UpdateSliceNodes();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!(node->IsA("vtkMRMLSliceCompositeNode")
        || node->IsA("vtkMRMLSliceNode")
        || node->IsA("vtkMRMLVolumeNode")))
    {
    return;
    }
  this->UpdateSliceNodes();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (!(node->IsA("vtkMRMLSliceCompositeNode")
        || node->IsA("vtkMRMLSliceNode")
        || node->IsA("vtkMRMLVolumeNode")))
    {
    return;
    }
  this->UpdateSliceNodes();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::OnMRMLSceneStartClose()
{
  this->UpdateSliceNodeFromLayout();
  this->DeleteSliceModel();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::OnMRMLSceneEndImport()
{
  this->SetupCrosshairNode();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::OnMRMLSceneEndRestore()
{
  this->SetupCrosshairNode();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdateSliceNodes()
{
  if (this->GetMRMLScene()
      && this->GetMRMLScene()->IsBatchProcessing())
    {
    return;
    }
  // Set up the nodes
  this->UpdateSliceNode();
  this->UpdateSliceCompositeNode();

  // Set up the models
  this->CreateSliceModel();

  this->UpdatePipeline();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetupCrosshairNode()
{
  //
  // On a new scene or restore, create the singleton for the default crosshair
  // for navigation or cursor if it doesn't already exist in scene
  //
  bool foundDefault = false;
  vtkMRMLNode* node;
  vtkCollectionSimpleIterator it;
  vtkSmartPointer<vtkCollection> crosshairs = vtkSmartPointer<vtkCollection>::Take(this->GetMRMLScene()->GetNodesByClass("vtkMRMLCrosshairNode"));
  for (crosshairs->InitTraversal(it);
       (node = (vtkMRMLNode*)crosshairs->GetNextItemAsObject(it)) ;)
    {
    vtkMRMLCrosshairNode* crosshairNode =
      vtkMRMLCrosshairNode::SafeDownCast(node);
    if (crosshairNode
        && crosshairNode->GetCrosshairName() == std::string("default"))
      {
      foundDefault = true;
      break;
      }
    }

  if (!foundDefault)
    {
    vtkNew<vtkMRMLCrosshairNode> crosshair;
    this->GetMRMLScene()->AddNode(crosshair.GetPointer());
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::OnMRMLNodeModified(vtkMRMLNode* node)
{
  assert(node);
  if (this->GetMRMLScene()->IsBatchProcessing())
    {
    return;
    }

  /// set slice extents in the layes
  this->SetSliceExtentsToSliceNode();

  // Update from SliceNode
  if (node == this->SliceNode)
    {
    // assert (sliceNode == this->SliceNode); not an assert because the node
    // might have change in CreateSliceModel() or UpdateSliceNode()
    vtkMRMLDisplayNode* sliceDisplayNode =
      this->SliceModelNode ? this->SliceModelNode->GetModelDisplayNode() : 0;
    if ( sliceDisplayNode)
      {
      sliceDisplayNode->SetVisibility( this->SliceNode->GetSliceVisible() );
      sliceDisplayNode->SetViewNodeIDs( this->SliceNode->GetThreeDViewIDs());
      }
    }
  else if (node == this->SliceCompositeNode)
    {
    this->UpdatePipeline();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic
::ProcessMRMLLogicsEvents(vtkObject* vtkNotUsed(caller),
                          unsigned long vtkNotUsed(event),
                          void* vtkNotUsed(callData))
{
  this->ProcessMRMLLogicsEvents();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::ProcessMRMLLogicsEvents()
{

  //
  // if we don't have layers yet, create them
  //
  if ( this->BackgroundLayer == 0 )
    {
    vtkNew<vtkMRMLSliceLayerLogic> layer;
    this->SetBackgroundLayer(layer.GetPointer());
    }
  if ( this->ForegroundLayer == 0 )
    {
    vtkNew<vtkMRMLSliceLayerLogic> layer;
    this->SetForegroundLayer(layer.GetPointer());
    }
  if ( this->LabelLayer == 0 )
    {
    vtkNew<vtkMRMLSliceLayerLogic> layer;
    // turn on using the label outline only in this layer
    layer->IsLabelLayerOn();
    this->SetLabelLayer(layer.GetPointer());
    }
  // Update slice plane geometry
  if (this->SliceNode != 0
      && this->GetSliceModelNode() != 0
      && this->GetMRMLScene() != 0
      && this->GetMRMLScene()->GetNodeByID( this->SliceModelNode->GetID() ) != 0
      && this->SliceModelNode->GetPolyData() != 0 )
    {
    int *dims1=0;
    int dims[3];
    vtkMatrix4x4 *textureToRAS = 0;
    if (this->SliceNode->GetSliceResolutionMode() != vtkMRMLSliceNode::SliceResolutionMatch2DView)
      {
      textureToRAS = this->SliceNode->GetUVWToRAS();
      dims1 = this->SliceNode->GetUVWDimensions();
      dims[0] = dims1[0]-1;
      dims[1] = dims1[1]-1;
      }
    else
      {
      textureToRAS = this->SliceNode->GetXYToRAS();
      dims1 = this->SliceNode->GetDimensions();
      dims[0] = dims1[0];
      dims[1] = dims1[1];
      }
    // set the plane corner point for use in a model
    double inPt[4]={0,0,0,1};
    double outPt[4];
    double *outPt3 = outPt;

    // set the z position to be the active slice (from the lightbox)
    inPt[2] = this->SliceNode->GetActiveSlice();

    vtkPlaneSource* plane = vtkPlaneSource::SafeDownCast(
      this->SliceModelNode->GetPolyDataConnection()->GetProducer());

    textureToRAS->MultiplyPoint(inPt, outPt);
    plane->SetOrigin(outPt3);
    inPt[0] = dims[0];
    textureToRAS->MultiplyPoint(inPt, outPt);
    plane->SetPoint1(outPt3);

    inPt[0] = 0;
    inPt[1] = dims[1];
    textureToRAS->MultiplyPoint(inPt, outPt);
    plane->SetPoint2(outPt3);


    this->UpdatePipeline();
    /// \tbd Ideally it should not be fired if the output polydata is not
    /// modified.
    plane->Modified();

    vtkMRMLModelDisplayNode *modelDisplayNode = this->SliceModelNode->GetModelDisplayNode();
    if ( modelDisplayNode )
      {
      if (this->LabelLayer && this->LabelLayer->GetImageDataConnectionUVW())
        {
        modelDisplayNode->SetInterpolateTexture(0);
        }
      else
        {
        modelDisplayNode->SetInterpolateTexture(1);
        }
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

  // Observe the slice node for general properties like slice visibility.
  // But the slice layers will also notify us when things like transforms have
  // changed.
  // This class takes care of passing the one slice node to each of the layers
  // so that users of this class only need to set the node one place.
  vtkSetAndObserveMRMLNodeMacro( this->SliceNode, newSliceNode );

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
  if (this->SliceCompositeNode == sliceCompositeNode)
    {
    return;
    }

  // Observe the composite node, since this holds the parameters for this pipeline
  vtkSetAndObserveMRMLNodeMacro( this->SliceCompositeNode, sliceCompositeNode );
  this->UpdatePipeline();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetBackgroundLayer(vtkMRMLSliceLayerLogic *backgroundLayer)
{
  // TODO: Simplify the whole set using a macro similar to vtkMRMLSetAndObserve
  if (this->BackgroundLayer)
    {
    this->BackgroundLayer->SetMRMLScene( 0 );
    this->BackgroundLayer->Delete();
    }
  this->BackgroundLayer = backgroundLayer;

  if (this->BackgroundLayer)
    {
    this->BackgroundLayer->Register(this);

    this->BackgroundLayer->SetMRMLScene(this->GetMRMLScene());

    this->BackgroundLayer->SetSliceNode(SliceNode);
    vtkEventBroker::GetInstance()->AddObservation(
      this->BackgroundLayer, vtkCommand::ModifiedEvent,
      this, this->GetMRMLLogicsCallbackCommand());
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetForegroundLayer(vtkMRMLSliceLayerLogic *foregroundLayer)
{
  // TODO: Simplify the whole set using a macro similar to vtkMRMLSetAndObserve
  if (this->ForegroundLayer)
    {
    this->ForegroundLayer->SetMRMLScene( 0 );
    this->ForegroundLayer->Delete();
    }
  this->ForegroundLayer = foregroundLayer;

  if (this->ForegroundLayer)
    {
    this->ForegroundLayer->Register(this);
    this->ForegroundLayer->SetMRMLScene( this->GetMRMLScene());

    this->ForegroundLayer->SetSliceNode(SliceNode);
    vtkEventBroker::GetInstance()->AddObservation(
      this->ForegroundLayer, vtkCommand::ModifiedEvent,
      this, this->GetMRMLLogicsCallbackCommand());
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetLabelLayer(vtkMRMLSliceLayerLogic *labelLayer)
{
  // TODO: Simplify the whole set using a macro similar to vtkMRMLSetAndObserve
  if (this->LabelLayer)
    {
    this->LabelLayer->SetMRMLScene( 0 );
    this->LabelLayer->Delete();
    }
  this->LabelLayer = labelLayer;

  if (this->LabelLayer)
    {
    this->LabelLayer->Register(this);

    this->LabelLayer->SetMRMLScene(this->GetMRMLScene());

    this->LabelLayer->SetSliceNode(SliceNode);
    vtkEventBroker::GetInstance()->AddObservation(
      this->LabelLayer, vtkCommand::ModifiedEvent,
      this, this->GetMRMLLogicsCallbackCommand());
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic
::SetBackgroundWindowLevel(double newWindow, double newLevel)
{
  vtkMRMLScalarVolumeNode* volumeNode =
    vtkMRMLScalarVolumeNode::SafeDownCast( this->GetLayerVolumeNode (0) );
    // 0 is background layer, defined in this::GetLayerVolumeNode
  vtkMRMLScalarVolumeDisplayNode* volumeDisplayNode =
    volumeNode ? volumeNode->GetScalarVolumeDisplayNode() : 0;
  if (!volumeDisplayNode)
    {
    return;
    }
  int disabledModify = volumeDisplayNode->StartModify();
  volumeDisplayNode->SetAutoWindowLevel(0);
  volumeDisplayNode->SetWindowLevel(newWindow, newLevel);
  volumeDisplayNode->EndModify(disabledModify);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic
::GetBackgroundWindowLevelAndRange(double& window, double& level,
                                         double& rangeLow, double& rangeHigh)
{
  vtkMRMLScalarVolumeNode* volumeNode =
    vtkMRMLScalarVolumeNode::SafeDownCast( this->GetLayerVolumeNode (0) );
    // 0 is background layer, definied in this::GetLayerVolumeNode
  vtkMRMLScalarVolumeDisplayNode* volumeDisplayNode = NULL;
  if (volumeNode)
    {
     volumeDisplayNode =
      vtkMRMLScalarVolumeDisplayNode::SafeDownCast( volumeNode->GetVolumeDisplayNode() );
    }
  vtkImageData* imageData;
  if (volumeDisplayNode && (imageData = volumeNode->GetImageData()) )
    {
    window = volumeDisplayNode->GetWindow();
    level = volumeDisplayNode->GetLevel();
    double range[2];
    imageData->GetScalarRange(range);
    rangeLow = range[0];
    rangeHigh = range[1];
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic
::GetForegroundWindowLevelAndRange(double& window, double& level,
                                         double& rangeLow, double& rangeHigh)
{
  vtkMRMLScalarVolumeNode* volumeNode =
    vtkMRMLScalarVolumeNode::SafeDownCast( this->GetLayerVolumeNode (1) );
    // 0 is background layer, definied in this::GetLayerVolumeNode
  vtkMRMLScalarVolumeDisplayNode* volumeDisplayNode = NULL;
  if (volumeNode)
    {
     volumeDisplayNode =
      vtkMRMLScalarVolumeDisplayNode::SafeDownCast( volumeNode->GetVolumeDisplayNode() );
    }
  vtkImageData* imageData;
  if (volumeDisplayNode && (imageData = volumeNode->GetImageData()) )
    {
    window = volumeDisplayNode->GetWindow();
    level = volumeDisplayNode->GetLevel();
    double range[2];
    imageData->GetScalarRange(range);
    rangeLow = range[0];
    rangeHigh = range[1];
    }
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput * vtkMRMLSliceLogic::GetImageDataConnection()
{
/*   if ( (this->GetBackgroundLayer() != 0 && this->GetBackgroundLayer()->GetImageDataConnection() != 0) ||
       (this->GetForegroundLayer() != 0 && this->GetForegroundLayer()->GetImageDataConnection() != 0) ||
       (this->GetLabelLayer() != 0 && this->GetLabelLayer()->GetImageDataConnection() != 0) )
    {
*/
    return this->ImageDataConnection;
/*
    }
   else
    {
    return 0;
    }
*/
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdateImageData ()
{
  if (this->SliceNode->GetSliceResolutionMode() == vtkMRMLSliceNode::SliceResolutionMatch2DView)
    {
    this->ExtractModelTexture->SetInputConnection( this->Blend->GetOutputPort() );
    this->ImageDataConnection = this->Blend->GetOutputPort();
    }
  else
    {
    this->ExtractModelTexture->SetInputConnection( this->BlendUVW->GetOutputPort() );
    }
  // It seems very strange that the imagedata can be null.
  // It should probably be always a valid imagedata with invalid bounds if needed

  if ( (this->GetBackgroundLayer() != 0 && this->GetBackgroundLayer()->GetImageDataConnection() != 0) ||
       (this->GetForegroundLayer() != 0 && this->GetForegroundLayer()->GetImageDataConnection() != 0) ||
       (this->GetLabelLayer() != 0 && this->GetLabelLayer()->GetImageDataConnection() != 0) )
    {
    if (this->ImageDataConnection == 0 || this->Blend->GetOutputPort()->GetMTime() > this->ImageDataConnection->GetMTime())
      {
      this->ImageDataConnection = this->Blend->GetOutputPort();
      }
    }
  else
    {
    this->ImageDataConnection = 0;
    if (this->SliceNode->GetSliceResolutionMode() == vtkMRMLSliceNode::SliceResolutionMatch2DView)
      {
      this->ExtractModelTexture->SetInputConnection( this->ImageDataConnection );
      }
    else
      {
      this->ExtractModelTexture->SetInputConnection( this->BlendUVW->GetOutputPort() );
      }
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

    /// set slice extents in the layers
    if (modified)
      {
      this->SetSliceExtentsToSliceNode();
      }
    // update the slice intersection visibility to track the composite node setting
    vtkMRMLModelDisplayNode *modelDisplayNode =
      this->SliceModelNode ? this->SliceModelNode->GetModelDisplayNode() : 0;
    if ( modelDisplayNode )
      {
      modelDisplayNode->SetSliceIntersectionVisibility(
        this->SliceCompositeNode->GetSliceIntersectionVisibility() );
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

    vtkAlgorithmOutput* backgroundImagePort = this->BackgroundLayer ? this->BackgroundLayer->GetImageDataConnection() : 0;
    vtkAlgorithmOutput* foregroundImagePort = this->ForegroundLayer ? this->ForegroundLayer->GetImageDataConnection() : 0;

    vtkAlgorithmOutput* backgroundImagePortUVW = this->BackgroundLayer ? this->BackgroundLayer->GetImageDataConnectionUVW() : 0;
    vtkAlgorithmOutput* foregroundImagePortUVW = this->ForegroundLayer ? this->ForegroundLayer->GetImageDataConnectionUVW() : 0;

    if (!alphaBlending)
      {
      if (!backgroundImagePort || !foregroundImagePort)
        {
        // not enough inputs for add/subtract, so use alpha blending
        // pipeline
        alphaBlending = true;
        }
      }
    vtkMTimeType oldBlendMTime = this->Blend->GetMTime();
    vtkMTimeType oldBlendUVWMTime = this->BlendUVW->GetMTime();

    int layerIndex = 0;
    int layerIndexUVW = 0;

    if (!alphaBlending)
      {
      vtkNew<vtkImageMathematics> tempMath;
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
      tempMath->SetInputConnection(0, foregroundImagePort );
      tempMath->SetInputConnection(1, backgroundImagePort );
      vtkInformation *tempMathOutInfo = tempMath->GetOutputInformation(0);
      vtkDataObject::SetPointDataActiveScalarInfo(tempMathOutInfo, VTK_SHORT,
        vtkImageData::GetNumberOfScalarComponents(tempMathOutInfo));

      vtkNew<vtkImageCast> tempCast;
      tempCast->SetInputConnection( tempMath->GetOutputPort() );
      tempCast->SetOutputScalarTypeToUnsignedChar();

      this->Blend->SetInputConnection( tempCast->GetOutputPort() );
      this->Blend->SetOpacity( layerIndex++, 1.0 );

      // UVW pipeline
      vtkNew<vtkImageMathematics> tempMathUVW;
      if (sliceCompositing == vtkMRMLSliceCompositeNode::Add)
        {
        // add the foreground and background
        tempMathUVW->SetOperationToAdd();
        }
      else if (sliceCompositing == vtkMRMLSliceCompositeNode::Subtract)
        {
        // subtract the foreground and background
        tempMathUVW->SetOperationToSubtract();
        }

      tempMathUVW->SetInputConnection(0, foregroundImagePortUVW );
      tempMathUVW->SetInputConnection(1, backgroundImagePortUVW );
      vtkInformation *tempMathUVWOutInfo = tempMathUVW->GetOutputInformation(0);
      vtkDataObject::SetPointDataActiveScalarInfo(tempMathUVWOutInfo, VTK_SHORT,
        vtkImageData::GetNumberOfScalarComponents(tempMathUVWOutInfo));

      vtkNew<vtkImageCast> tempCastUVW;
      tempCastUVW->SetInputConnection( tempMathUVW->GetOutputPort() );
      tempCastUVW->SetOutputScalarTypeToUnsignedChar();

      this->BlendUVW->AddInputConnection( tempCastUVW->GetOutputPort() );
      this->BlendUVW->SetOpacity( layerIndexUVW++, 1.0 );
      }
    else
      {
      this->Blend->RemoveAllInputs();
      this->BlendUVW->RemoveAllInputs();
      if (sliceCompositing ==  vtkMRMLSliceCompositeNode::Alpha)
        {
        if ( backgroundImagePort )
          {
          this->Blend->AddInputConnection( backgroundImagePort );
          this->Blend->SetOpacity( layerIndex++, 1.0 );
          }
        if ( foregroundImagePort )
          {
          this->Blend->AddInputConnection( foregroundImagePort );
          this->Blend->SetOpacity( layerIndex++, this->SliceCompositeNode->GetForegroundOpacity() );
          }
        if ( backgroundImagePortUVW )
          {
          this->BlendUVW->AddInputConnection( backgroundImagePortUVW );
          this->BlendUVW->SetOpacity( layerIndexUVW++, 1.0 );
          }
        if ( foregroundImagePortUVW )
          {
          this->BlendUVW->AddInputConnection( foregroundImagePortUVW );
          this->BlendUVW->SetOpacity( layerIndexUVW++, this->SliceCompositeNode->GetForegroundOpacity() );
          }
        }
      else if (sliceCompositing == vtkMRMLSliceCompositeNode::ReverseAlpha)
        {
        if ( foregroundImagePort )
          {
          this->Blend->AddInputConnection( foregroundImagePort );
          this->Blend->SetOpacity( layerIndex++, 1.0 );
          }
        if ( backgroundImagePort )
          {
          this->Blend->AddInputConnection( backgroundImagePort );
          this->Blend->SetOpacity( layerIndex++, this->SliceCompositeNode->GetForegroundOpacity() );
          }
        if ( foregroundImagePortUVW )
          {
          this->BlendUVW->AddInputConnection( foregroundImagePortUVW );
          this->BlendUVW->SetOpacity( layerIndexUVW++, 1.0 );
          }
        if ( backgroundImagePortUVW )
          {
          this->BlendUVW->AddInputConnection( backgroundImagePortUVW );
          this->BlendUVW->SetOpacity( layerIndexUVW++, this->SliceCompositeNode->GetForegroundOpacity() );
          }
        }
      }
    // always blending the label layer
    vtkAlgorithmOutput* labelImagePort = this->LabelLayer ? this->LabelLayer->GetImageDataConnection() : 0;
    vtkAlgorithmOutput* labelImagePortUVW = this->LabelLayer ? this->LabelLayer->GetImageDataConnectionUVW() : 0;
    if ( labelImagePort )
      {
      this->Blend->AddInputConnection( labelImagePort );
      this->Blend->SetOpacity( layerIndex++, this->SliceCompositeNode->GetLabelOpacity() );
      }
    if ( labelImagePortUVW )
      {
      this->BlendUVW->AddInputConnection( labelImagePortUVW );
      this->BlendUVW->SetOpacity( layerIndexUVW++, this->SliceCompositeNode->GetLabelOpacity() );
      }
    while (this->Blend->GetNumberOfInputConnections(0) > layerIndex)
      {
      // it decreases the number of inputs
      this->Blend->RemoveInputConnection(0, this->Blend->GetNumberOfInputConnections(0) - 1);
      }
    while (this->BlendUVW->GetNumberOfInputConnections(0) > layerIndexUVW)
      {
      // it decreases the number of inputs
      this->BlendUVW->RemoveInputConnection(0, this->BlendUVW->GetNumberOfInputConnections(0) - 1);
      }
    if (this->Blend->GetMTime() > oldBlendMTime)
      {
      modified = 1;
      }
    if (this->BlendUVW->GetMTime() > oldBlendUVWMTime)
      {
      modified = 1;
      }

    //Models
    this->UpdateImageData();
    vtkMRMLDisplayNode* displayNode = this->SliceModelNode ? this->SliceModelNode->GetModelDisplayNode() : 0;
    if ( displayNode && this->SliceNode )
      {
      displayNode->SetVisibility( this->SliceNode->GetSliceVisible() );
      displayNode->SetViewNodeIDs( this->SliceNode->GetThreeDViewIDs());
      if ( (this->SliceNode->GetSliceResolutionMode() != vtkMRMLSliceNode::SliceResolutionMatch2DView &&
          !((backgroundImagePortUVW != 0) || (foregroundImagePortUVW != 0) || (labelImagePortUVW != 0) ) ) ||
          (this->SliceNode->GetSliceResolutionMode() == vtkMRMLSliceNode::SliceResolutionMatch2DView &&
          !((backgroundImagePort != 0) || (foregroundImagePort != 0) || (labelImagePort != 0) ) ))
        {
        displayNode->SetTextureImageDataConnection(0);
        }
      else if (displayNode->GetTextureImageDataConnection() != this->ExtractModelTexture->GetOutputPort())
        {
        displayNode->SetTextureImageDataConnection(this->ExtractModelTexture->GetOutputPort());
        }
        if ( this->LabelLayer && this->LabelLayer->GetImageDataConnection())
          {
          modelDisplayNode->SetInterpolateTexture(0);
          }
        else
          {
          modelDisplayNode->SetInterpolateTexture(1);
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

  if (this->BlendUVW)
    {
    os << indent << "BlendUVW: ";
    this->BlendUVW->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "BlendUVW: (none)\n";
    }

  os << indent << "SLICE_MODEL_NODE_NAME_SUFFIX: " << this->SLICE_MODEL_NODE_NAME_SUFFIX << "\n";

}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::DeleteSliceModel()
{
  // Remove References
  if (this->SliceModelNode != 0)
    {
    this->SliceModelNode->SetAndObserveDisplayNodeID(0);
    this->SliceModelNode->SetAndObserveTransformNodeID(0);
    this->SliceModelNode->SetPolyDataConnection(0);
    }
  if (this->SliceModelDisplayNode != 0)
    {
    this->SliceModelDisplayNode->SetTextureImageDataConnection(0);
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
    vtkNew<vtkPlaneSource> planeSource;
    planeSource->Update();
    this->SliceModelNode->SetPolyDataConnection(planeSource->GetOutputPort());
    this->SliceModelNode->SetDisableModifiedEvent(0);

    // create display node and set texture
    this->SliceModelDisplayNode = vtkMRMLModelDisplayNode::New();
    this->SliceModelDisplayNode->SetScene(this->GetMRMLScene());
    this->SliceModelDisplayNode->SetDisableModifiedEvent(1);

    //this->SliceModelDisplayNode->SetInputPolyData(this->SliceModelNode->GetOutputPolyData());
    this->SliceModelDisplayNode->SetVisibility(0);
    this->SliceModelDisplayNode->SetOpacity(1);
    this->SliceModelDisplayNode->SetColor(1,1,1);
    if (this->SliceNode)
      {
      // Auto-set the colors based on the slice node
      this->SliceModelDisplayNode->SetColor(this->SliceNode->GetLayoutColor());
      }
    this->SliceModelDisplayNode->SetAmbient(1);
    this->SliceModelDisplayNode->SetBackfaceCulling(0);
    this->SliceModelDisplayNode->SetDiffuse(0);
    this->SliceModelDisplayNode->SetTextureImageDataConnection(this->ExtractModelTexture->GetOutputPort());
    this->SliceModelDisplayNode->SetSaveWithScene(0);
    this->SliceModelDisplayNode->SetDisableModifiedEvent(0);
    // set an attribute to distinguish this from regular model display nodes
    this->SliceModelDisplayNode->SetAttribute("SliceLogic.IsSliceModelDisplayNode", "True");
    std::string displayName = std::string(this->Name) + std::string(" Display");
    this->SliceModelDisplayNode->SetName(displayName.c_str());
    // Turn slice intersection off by default - there is a higher level GUI control
    // in the SliceCompositeNode that tells if slices should be enabled for a given
    // slice viewer
    this->SliceModelDisplayNode->SetSliceIntersectionVisibility(0);

    std::string name = std::string(this->Name) + std::string(" ") + this->SLICE_MODEL_NODE_NAME_SUFFIX;
    this->SliceModelNode->SetName (name.c_str());

    // make the xy to RAS transform
    this->SliceModelTransformNode = vtkMRMLLinearTransformNode::New();
    this->SliceModelTransformNode->SetScene(this->GetMRMLScene());
    this->SliceModelTransformNode->SetDisableModifiedEvent(1);

    this->SliceModelTransformNode->SetHideFromEditors(1);
    this->SliceModelTransformNode->SetSelectable(0);
    this->SliceModelTransformNode->SetSaveWithScene(0);
    // set the transform for the slice model for use by an image actor in the viewer
    vtkNew<vtkMatrix4x4> identity;
    identity->Identity();
    this->SliceModelTransformNode->SetMatrixTransformToParent(identity.GetPointer());

    this->SliceModelTransformNode->SetDisableModifiedEvent(0);

    }

  if (this->SliceModelNode != 0 && this->GetMRMLScene()->GetNodeByID( this->GetSliceModelNode()->GetID() ) == 0 )
    {
    this->AddingSliceModelNodes = true;
    this->GetMRMLScene()->AddNode(this->SliceModelDisplayNode);
    this->GetMRMLScene()->AddNode(this->SliceModelTransformNode);
    this->SliceModelNode->SetAndObserveDisplayNodeID(this->SliceModelDisplayNode->GetID());
    this->GetMRMLScene()->AddNode(this->SliceModelNode);
    this->AddingSliceModelNodes = false;
    this->SliceModelDisplayNode->SetTextureImageDataConnection(this->ExtractModelTexture->GetOutputPort());
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

  double bounds[6];
  volumeNode->GetRASBounds(bounds);

  for (int i=0; i<3; i++)
    {
    rasDimensions[i] = bounds[2*i+1] - bounds[2*i];
    rasCenter[i] = 0.5*(bounds[2*i+1] + bounds[2*i]);
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
  vtkNew<vtkMatrix4x4> rasToSlice;
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

  vtkNew<vtkMatrix4x4> ijkToRAS;
  vtkNew<vtkMatrix4x4> rasToSlice;
  vtkNew<vtkMatrix4x4> ijkToSlice;

  volumeNode->GetIJKToRASMatrix(ijkToRAS.GetPointer());

  // Apply the transform, if it exists
  vtkMRMLTransformNode *transformNode = volumeNode->GetParentTransformNode();
  if ( transformNode != 0 )
    {
    if ( transformNode->IsTransformToWorldLinear() )
      {
      vtkNew<vtkMatrix4x4> rasToRAS;
      transformNode->GetMatrixTransformToWorld( rasToRAS.GetPointer() );
      rasToRAS->Invert();
      vtkMatrix4x4::Multiply4x4(rasToRAS.GetPointer(), ijkToRAS.GetPointer(), ijkToRAS.GetPointer());
      }
    }

  rasToSlice->DeepCopy(sliceNode->GetSliceToRAS());
  rasToSlice->Invert();

  ijkToSlice->Multiply4x4(rasToSlice.GetPointer(), ijkToRAS.GetPointer(), ijkToSlice.GetPointer());

  double invector[4] = {1., 1., 1., 0.};
  double spacing[4];
  ijkToSlice->MultiplyPoint(invector, spacing);
  for (int i = 0; i < 3; ++i)
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
  vtkNew<vtkMatrix4x4> sliceToRAS;
  sliceToRAS->DeepCopy(sliceNode->GetSliceToRAS());
  sliceToRAS->SetElement(0, 3, rasCenter[0]);
  sliceToRAS->SetElement(1, 3, rasCenter[1]);
  sliceToRAS->SetElement(2, 3, rasCenter[2]);
  sliceNode->GetSliceToRAS()->DeepCopy(sliceToRAS.GetPointer());
  sliceNode->SetSliceOrigin(0,0,0);
  //sliceNode->SetSliceOffset(offset);

  //TODO Fit UVW space
  this->SnapSliceOffsetToIJK();
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
  vtkNew<vtkMatrix4x4> ijkToRAS;

  // what are the actual dimensions of the imagedata?
  backgroundImage->GetDimensions(dimensions);
  doubleDimensions[0] = static_cast<double>(dimensions[0]);
  doubleDimensions[1] = static_cast<double>(dimensions[1]);
  doubleDimensions[2] = static_cast<double>(dimensions[2]);
  doubleDimensions[3] = 0.0;
  backgroundNode->GetIJKToRASMatrix(ijkToRAS.GetPointer());
  ijkToRAS->MultiplyPoint(doubleDimensions, rasDimensions);

  // and what are their slice dimensions?
  vtkNew<vtkMatrix4x4> rasToSlice;
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

  vtkNew<vtkMatrix4x4> sliceToRAS;
  sliceToRAS->DeepCopy(this->SliceNode->GetSliceToRAS());
  this->SliceNode->GetSliceToRAS()->DeepCopy(sliceToRAS.GetPointer());
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

  double scalingX = (newWidth != 0 && oldDimensions[0] != 0 ? newWidth / oldDimensions[0] : 1.);
  double scalingY = (newHeight != 0 && oldDimensions[1] != 0 ? newHeight / oldDimensions[1] : 1.);

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
  // TBD: Doesn't return the lowest slice spacing, just the first valid spacing
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

#define LARGE_BOUNDS_NUM 1.0e10
#define SMALL_BOUNDS_NUM -1.0e10
//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::GetSliceBounds(double sliceBounds[6])
{
  int i;
  for (i=0; i<3; i++)
    {
    sliceBounds[2*i]   = LARGE_BOUNDS_NUM;
    sliceBounds[2*i+1] = SMALL_BOUNDS_NUM;
    }

  vtkMRMLVolumeNode *volumeNode;
  for ( int layer=0; layer < 3; layer++ )
    {
    volumeNode = this->GetLayerVolumeNode (layer);
    if (volumeNode)
      {
      double bounds[6];
      this->GetVolumeSliceBounds( volumeNode, bounds );
      for (i=0; i<3; i++)
        {
        if (bounds[2*i] < sliceBounds[2*i])
          {
          sliceBounds[2*i] = bounds[2*i];
          }
        if (bounds[2*i+1] > sliceBounds[2*i+1])
          {
          sliceBounds[2*i+1] = bounds[2*i+1];
          }
        }
      }
    }

  // default
  for (i=0; i<3; i++)
    {
    if (sliceBounds[2*i] == LARGE_BOUNDS_NUM)
      {
      sliceBounds[2*i] = -100;
      }
    if (sliceBounds[2*i+1] == SMALL_BOUNDS_NUM)
      {
      sliceBounds[2*i+1] = 100;
      }
    }

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

  sliceNode->SetSliceOffset(offset);

}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::StartSliceCompositeNodeInteraction(unsigned int parameters)
{
  vtkMRMLSliceCompositeNode *compositeNode = this->GetSliceCompositeNode();

  // Cache the flags on what parameters are going to be modified. Need
  // to this this outside the conditional on HotLinkedControl and LinkedControl
  compositeNode->SetInteractionFlags(parameters);

  // If we have hot linked controls, then we want to broadcast changes
  if (compositeNode &&
      compositeNode->GetHotLinkedControl() && compositeNode->GetLinkedControl())
    {
    if (compositeNode)
      {
      compositeNode->InteractingOn();
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::EndSliceCompositeNodeInteraction()
{
  vtkMRMLSliceCompositeNode *compositeNode = this->GetSliceCompositeNode();

  // If we have linked controls, then we want to broadcast changes
  if (compositeNode && compositeNode->GetLinkedControl())
    {
    if (compositeNode)
      {
      // Need to trigger a final message to broadcast to all the nodes
      // that are linked
      compositeNode->InteractingOn();
      compositeNode->Modified();
      compositeNode->InteractingOff();
      compositeNode->SetInteractionFlags(0);
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::StartSliceNodeInteraction(unsigned int parameters)
{
  if (this->SliceNode == NULL || this->SliceCompositeNode == NULL)
    {
    return;
    }

  // Cache the flags on what parameters are going to be modified. Need
  // to this this outside the conditional on HotLinkedControl and LinkedControl
  this->SliceNode->SetInteractionFlags(parameters);

  // If we have hot linked controls, then we want to broadcast changes
  if ((this->SliceCompositeNode->GetHotLinkedControl() || parameters == vtkMRMLSliceNode::MultiplanarReformatFlag)
      && this->SliceCompositeNode->GetLinkedControl())
    {
    this->SliceNode->InteractingOn();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetSliceExtentsToSliceNode()
{
  if (this->SliceNode == NULL)
    {
    return;
    }

  double sliceBounds[6];
  this->GetSliceBounds( sliceBounds );

  double extents[3];
  extents[0] = sliceBounds[1] - sliceBounds[0];
  extents[1] = sliceBounds[3] - sliceBounds[2];
  extents[2] = sliceBounds[5] - sliceBounds[4];

  if (this->SliceNode->GetSliceResolutionMode() == vtkMRMLSliceNode::SliceResolutionMatch2DView)
    {
    this->SliceNode->SetUVWExtentsAndDimensions(this->SliceNode->GetFieldOfView(),
                                                this->SliceNode->GetUVWDimensions());
    }
 else if (this->SliceNode->GetSliceResolutionMode() == vtkMRMLSliceNode::SliceResolutionMatchVolumes)
    {
    // TODO: the GetLowestVolumeSliceSpacing currently returns spacing not lowest spacing
    double *spacing = this->GetLowestVolumeSliceSpacing();
    double minSpacing = spacing[0];
    minSpacing = minSpacing < spacing[1] ? minSpacing:spacing[1];
    minSpacing = minSpacing < spacing[2] ? minSpacing:spacing[2];

    int sliceResolutionMax = 200;
    if (minSpacing > 0.0)
      {
      double maxExtent = extents[0];
      maxExtent = maxExtent > extents[1] ? maxExtent:extents[1];
      maxExtent = maxExtent > extents[2] ? maxExtent:extents[2];

      sliceResolutionMax = maxExtent/minSpacing;
      }
    int dimensions[]={sliceResolutionMax, sliceResolutionMax, 1};

    this->SliceNode->SetUVWExtentsAndDimensions(extents, dimensions);
    }
  else if (this->SliceNode->GetSliceResolutionMode() == vtkMRMLSliceNode::SliceFOVMatch2DViewSpacingMatchVolumes)
    {
    // TODO: the GetLowestVolumeSliceSpacing currently returns spacing not lowest spacing
    double *spacing = this->GetLowestVolumeSliceSpacing();
    double minSpacing = spacing[0];
    minSpacing = minSpacing < spacing[1] ? minSpacing:spacing[1];
    minSpacing = minSpacing < spacing[2] ? minSpacing:spacing[2];

    double fov[3];
    int dimensions[]={0,0,1};
    this->SliceNode->GetFieldOfView(fov);
    for (int i=0; i<2; i++)
      {
       dimensions[i] = ceil(fov[i]/minSpacing +0.5);
      }
    this->SliceNode->SetUVWExtentsAndDimensions(fov, dimensions);
    }
  else if (this->SliceNode->GetSliceResolutionMode() == vtkMRMLSliceNode::SliceFOVMatchVolumesSpacingMatch2DView)
    {
    // compute RAS spacing in 2D view
    vtkMatrix4x4 *xyToRAS = this->SliceNode->GetXYToRAS();
    int  dims[3];

    //
    double inPt[4]={0,0,0,1};
    double outPt0[4];
    double outPt1[4];
    double outPt2[4];

    // set the z position to be the active slice (from the lightbox)
    inPt[2] = this->SliceNode->GetActiveSlice();

    // transform XYZ = (0,0,0)
    xyToRAS->MultiplyPoint(inPt, outPt0);

    // transform XYZ = (1,0,0)
    inPt[0] = 1;
    xyToRAS->MultiplyPoint(inPt, outPt1);

    // transform XYZ = (0,1,0)
    inPt[0] = 0;
    inPt[1] = 1;
    xyToRAS->MultiplyPoint(inPt, outPt2);

    double xSpacing = sqrt(vtkMath::Distance2BetweenPoints(outPt0, outPt1));
    double ySpacing = sqrt(vtkMath::Distance2BetweenPoints(outPt0, outPt2));

    dims[0] = extents[0]/xSpacing+1;
    dims[1] = extents[2]/ySpacing+1;
    dims[2] = 1;

    this->SliceNode->SetUVWExtentsAndDimensions(extents, dims);
    }

}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::EndSliceNodeInteraction()
{
  if (this->SliceNode == NULL || this->SliceCompositeNode == NULL)
    {
    return;
    }

  // If we have linked controls, then we want to broadcast changes
  if (this->SliceCompositeNode->GetLinkedControl())
    {
    // Need to trigger a final message to broadcast to all the nodes
    // that are linked
    this->SliceNode->InteractingOn();
    this->SliceNode->Modified();
    this->SliceNode->InteractingOff();
    this->SliceNode->SetInteractionFlags(0);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::StartSliceOffsetInteraction()
{
  // This method is here in case we want to do something specific when
  // we start SliceOffset interactions

  this->StartSliceNodeInteraction(vtkMRMLSliceNode::SliceToRASFlag);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::EndSliceOffsetInteraction()
{
  // This method is here in case we want to do something specific when
  // we complete SliceOffset interactions

  this->EndSliceNodeInteraction();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SnapSliceOffsetToIJK()
{
  double offset, *spacing, bounds[6];
  double oldOffset = this->GetSliceOffset();
  spacing = this->GetLowestVolumeSliceSpacing();
  this->GetLowestVolumeSliceBounds( bounds );

  // number of slices along the offset dimension (depends on ijkToRAS and Transforms)
  // - find the slice index corresponding to the current slice offset
  // - move the offset to the middle of that slice
  // - note that bounds[4] 'furthest' edge of the volume from the point of view of this slice
  // - note also that spacing[2] may correspond to i, j, or k depending on ijkToRAS and sliceToRAS
  double slice = (oldOffset - bounds[4]) / spacing[2];
  int intSlice = static_cast<int> (slice);
  offset = (intSlice + 0.5) * spacing[2] + bounds[4];
  this->SetSliceOffset( offset );
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

  vtkNew<vtkMatrix4x4> ijkToRAS;
  volumeNode->GetIJKToRASMatrix (ijkToRAS.GetPointer());
  vtkMRMLTransformNode *transformNode = volumeNode->GetParentTransformNode();
  if ( transformNode )
    {
    vtkNew<vtkMatrix4x4> rasToRAS;
    transformNode->GetMatrixTransformToWorld(rasToRAS.GetPointer());
    vtkMatrix4x4::Multiply4x4 (rasToRAS.GetPointer(), ijkToRAS.GetPointer(), ijkToRAS.GetPointer());
    }

  // Get the slice normal in RAS

  vtkNew<vtkMatrix4x4> rasToSlice;
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

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode* vtkMRMLSliceLogic
::GetSliceCompositeNode(vtkMRMLSliceNode* sliceNode)
{
  return sliceNode ? vtkMRMLSliceLogic::GetSliceCompositeNode(
    sliceNode->GetScene(), sliceNode->GetLayoutName()) : 0;
}

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode* vtkMRMLSliceLogic
::GetSliceCompositeNode(vtkMRMLScene* scene, const char* layoutName)
{
  if (!scene || !layoutName)
    {
    return 0;
    }
  vtkMRMLNode* node;
  vtkCollectionSimpleIterator it;
  for (scene->GetNodes()->InitTraversal(it);
       (node = (vtkMRMLNode*)scene->GetNodes()->GetNextItemAsObject(it)) ;)
    {
    vtkMRMLSliceCompositeNode* sliceCompositeNode =
      vtkMRMLSliceCompositeNode::SafeDownCast(node);
    if (sliceCompositeNode &&
        sliceCompositeNode->GetLayoutName() &&
        !strcmp(sliceCompositeNode->GetLayoutName(), layoutName))
      {
      return sliceCompositeNode;
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLSliceLogic
::GetSliceNode(vtkMRMLSliceCompositeNode* sliceCompositeNode)
{
  if (!sliceCompositeNode)
    {
    return 0;
    }
  return sliceCompositeNode ? vtkMRMLSliceLogic::GetSliceNode(
    sliceCompositeNode->GetScene(), sliceCompositeNode->GetLayoutName()) : 0;
}

//----------------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLSliceLogic
::GetSliceNode(vtkMRMLScene* scene, const char* layoutName)
{
  if (!scene || !layoutName)
    {
    return 0;
    }
  vtkObject* itNode = NULL;
  vtkCollectionSimpleIterator it;
  for (scene->GetNodes()->InitTraversal(it); (itNode = scene->GetNodes()->GetNextItemAsObject(it));)
    {
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(itNode);
    if (!sliceNode)
      {
      continue;
      }
    if (sliceNode->GetLayoutName() &&
      !strcmp(sliceNode->GetLayoutName(), layoutName))
      {
      return sliceNode;
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceLogic::IsSliceModelNode(vtkMRMLNode *mrmlNode)
{
  if (mrmlNode != NULL &&
      mrmlNode->IsA("vtkMRMLModelNode") &&
      mrmlNode->GetName() != NULL &&
      strstr(mrmlNode->GetName(), vtkMRMLSliceLogic::SLICE_MODEL_NODE_NAME_SUFFIX.c_str()) != NULL)
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceLogic::IsSliceModelDisplayNode(vtkMRMLDisplayNode *mrmlDisplayNode)
{
  if (mrmlDisplayNode != NULL &&
      mrmlDisplayNode->IsA("vtkMRMLModelDisplayNode"))
    {
    const char *attrib = mrmlDisplayNode->GetAttribute("SliceLogic.IsSliceModelDisplayNode");
    // allow the attribute to be set to anything but 0
    if (attrib != NULL &&
        strcmp(attrib, "0") != 0)
      {
      return true;
      }
    }
  return false;
}

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
#include "vtkMRMLApplicationLogic.h"
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
#include <vtkMRMLSliceDisplayNode.h>

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkGeneralTransform.h>
#include <vtkImageAppendComponents.h>
#include <vtkImageBlend.h>
#include <vtkImageResample.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageMathematics.h>
#include <vtkImageReslice.h>
#include <vtkImageThreshold.h>
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

// VTKAddon includes
#include <vtkAddonMathUtilities.h>

// STD includes
#include <algorithm>

//----------------------------------------------------------------------------
const int vtkMRMLSliceLogic::SLICE_INDEX_ROTATED=-1;
const int vtkMRMLSliceLogic::SLICE_INDEX_OUT_OF_VOLUME=-2;
const int vtkMRMLSliceLogic::SLICE_INDEX_NO_VOLUME=-3;
const std::string vtkMRMLSliceLogic::SLICE_MODEL_NODE_NAME_SUFFIX = std::string("Volume Slice");

//----------------------------------------------------------------------------
struct SliceLayerInfo
  {
  SliceLayerInfo(vtkAlgorithmOutput* blendInput, double opacity)
    {
    this->BlendInput = blendInput;
    this->Opacity = opacity;
    }
  vtkSmartPointer<vtkAlgorithmOutput> BlendInput;
  double Opacity;
  };

//----------------------------------------------------------------------------
struct BlendPipeline
{
  BlendPipeline()
  {
    /*
    // AlphaBlending, ReverseAlphaBlending:
    //
    //   foreground \
    //               > Blend
    //   background /
    //
    // Add, Subtract:
    //
    //   Casting is needed to avoid overflow during adding (or subtracting).
    //
    //   AddSubMath adds/subtracts alpha channel, therefore we copy RGB and alpha
    //   components and copy of the background's alpha channel to the output.
    //   Splitting and appending channels is probably quite inefficient, but there does not
    //   seem to be simpler pipeline to do this in VTK.
    //
    //   foreground > AddSubForegroundCast \
    //                                      > AddSubMath > AddSubOutputCast ...
    //   background > AddSubBackroundCast  /
    //
    //
    //     ... AddSubOutputCast > AddSubExtractRGB \
    //                                              > AddSubAppendRGBA > Blend
    //             background > AddSubExtractAlpha /
    */

    this->AddSubForegroundCast->SetOutputScalarTypeToShort();
    this->AddSubBackgroundCast->SetOutputScalarTypeToShort();
    this->AddSubMath->SetOperationToAdd();
    this->AddSubMath->SetInputConnection(0, this->AddSubBackgroundCast->GetOutputPort());
    this->AddSubMath->SetInputConnection(1, this->AddSubForegroundCast->GetOutputPort());
    this->AddSubOutputCast->SetInputConnection(this->AddSubMath->GetOutputPort());

    this->AddSubExtractRGB->SetInputConnection(this->AddSubOutputCast->GetOutputPort());
    this->AddSubExtractRGB->SetComponents(0, 1, 2);
    this->AddSubExtractAlpha->SetComponents(3);
    this->AddSubAppendRGBA->AddInputConnection(this->AddSubExtractRGB->GetOutputPort());
    this->AddSubAppendRGBA->AddInputConnection(this->AddSubExtractAlpha->GetOutputPort());

    this->AddSubOutputCast->SetOutputScalarTypeToUnsignedChar();
    this->AddSubOutputCast->ClampOverflowOn();
  }

  void AddLayers(std::deque<SliceLayerInfo>& layers, int sliceCompositing,
    vtkAlgorithmOutput* backgroundImagePort,
    vtkAlgorithmOutput* foregroundImagePort, double foregroundOpacity,
    vtkAlgorithmOutput* labelImagePort, double labelOpacity)
  {
    if (sliceCompositing == vtkMRMLSliceCompositeNode::Add || sliceCompositing == vtkMRMLSliceCompositeNode::Subtract)
      {
      if (!backgroundImagePort || !foregroundImagePort)
        {
        // not enough inputs for add/subtract, so use alpha blending pipeline
        sliceCompositing = vtkMRMLSliceCompositeNode::Alpha;
        }
      }

    if (sliceCompositing == vtkMRMLSliceCompositeNode::Alpha)
      {
      if (backgroundImagePort)
        {
        layers.emplace_back(backgroundImagePort, 1.0);
        }
      if (foregroundImagePort)
        {
        layers.emplace_back(foregroundImagePort, foregroundOpacity);
        }
      }
    else if (sliceCompositing == vtkMRMLSliceCompositeNode::ReverseAlpha)
      {
      if (foregroundImagePort)
        {
        layers.emplace_back(foregroundImagePort, 1.0);
        }
      if (backgroundImagePort)
        {
        layers.emplace_back(backgroundImagePort, foregroundOpacity);
        }
      }
    else
      {
      this->AddSubForegroundCast->SetInputConnection(foregroundImagePort);
      this->AddSubBackgroundCast->SetInputConnection(backgroundImagePort);
      this->AddSubExtractAlpha->SetInputConnection(backgroundImagePort);
      if (sliceCompositing == vtkMRMLSliceCompositeNode::Add)
        {
        this->AddSubMath->SetOperationToAdd();
        }
      else
        {
        this->AddSubMath->SetOperationToSubtract();
        }
      layers.emplace_back(this->AddSubAppendRGBA->GetOutputPort(), 1.0);
      }

    // always blending the label layer
    if (labelImagePort)
      {
      layers.emplace_back(labelImagePort, labelOpacity);
      }
  }

  vtkNew<vtkImageCast> AddSubForegroundCast;
  vtkNew<vtkImageCast> AddSubBackgroundCast;
  vtkNew<vtkImageMathematics> AddSubMath;
  vtkNew<vtkImageExtractComponents> AddSubExtractRGB;
  vtkNew<vtkImageExtractComponents> AddSubExtractAlpha;
  vtkNew<vtkImageAppendComponents> AddSubAppendRGBA;
  vtkNew<vtkImageCast> AddSubOutputCast;
  vtkNew<vtkImageBlend> Blend;
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLSliceLogic);

//----------------------------------------------------------------------------
vtkMRMLSliceLogic::vtkMRMLSliceLogic()
{
  this->BackgroundLayer = nullptr;
  this->ForegroundLayer = nullptr;
  this->LabelLayer = nullptr;
  this->SliceNode = nullptr;
  this->SliceCompositeNode = nullptr;

  this->Pipeline = new BlendPipeline;
  this->PipelineUVW = new BlendPipeline;

  this->ExtractModelTexture = vtkImageReslice::New();
  this->ExtractModelTexture->SetOutputDimensionality (2);
  this->ExtractModelTexture->SetInputConnection(this->PipelineUVW->Blend->GetOutputPort());

  this->SliceModelNode = nullptr;
  this->SliceModelTransformNode = nullptr;
  this->SliceModelDisplayNode = nullptr;
  this->ImageDataConnection = nullptr;
  this->SliceSpacing[0] = this->SliceSpacing[1] = this->SliceSpacing[2] = 1;
  this->AddingSliceModelNodes = false;
}

//----------------------------------------------------------------------------
vtkMRMLSliceLogic::~vtkMRMLSliceLogic()
{
  this->SetSliceNode(nullptr);

  if (this->ImageDataConnection)
    {
    this->ImageDataConnection = nullptr;
    }

  delete this->Pipeline;
  delete this->PipelineUVW;

  if (this->ExtractModelTexture)
    {
    this->ExtractModelTexture->Delete();
    this->ExtractModelTexture = nullptr;
    }

  this->SetBackgroundLayer (nullptr);
  this->SetForegroundLayer (nullptr);
  this->SetLabelLayer (nullptr);

  if (this->SliceCompositeNode)
    {
    vtkSetAndObserveMRMLNodeMacro( this->SliceCompositeNode, 0);
    }

  this->DeleteSliceModel();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
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

  this->ProcessMRMLSceneEvents(newScene, vtkMRMLScene::EndBatchProcessEvent, nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdateSliceNode()
{
  if (!this->GetMRMLScene())
    {
    this->SetSliceNode(nullptr);
    }

}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdateSliceNodeFromLayout()
{
  if (this->SliceNode == nullptr)
    {
    return;
    }
  this->SliceNode->SetOrientationToDefault();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdateSliceCompositeNode()
{
  if (!this->GetMRMLScene() || !this->SliceNode)
    {
    this->SetSliceCompositeNode(nullptr);
    return;
    }

  // find SliceCompositeNode in the scene
  std::string layoutName = (this->SliceNode->GetLayoutName() ? this->SliceNode->GetLayoutName() : "");
  vtkMRMLSliceCompositeNode* updatedSliceCompositeNode = vtkMRMLSliceLogic::GetSliceCompositeNode(this->GetMRMLScene(), layoutName.c_str());

  if (this->SliceCompositeNode && updatedSliceCompositeNode &&
       (!this->SliceCompositeNode->GetID() || strcmp(this->SliceCompositeNode->GetID(), updatedSliceCompositeNode->GetID()) != 0) )
    {
    // local SliceCompositeNode is out of sync with the scene
    this->SetSliceCompositeNode(nullptr);
    }

  if (!this->SliceCompositeNode)
    {
    if (!updatedSliceCompositeNode && !layoutName.empty())
      {
      // Use CreateNodeByClass instead of New to use default node specified in the scene
      updatedSliceCompositeNode = vtkMRMLSliceCompositeNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLSliceCompositeNode"));
      updatedSliceCompositeNode->SetLayoutName(layoutName.c_str());
      this->GetMRMLScene()->AddNode(updatedSliceCompositeNode);
      this->SetSliceCompositeNode(updatedSliceCompositeNode);
      updatedSliceCompositeNode->Delete();
      }
    else
      {
      this->SetSliceCompositeNode(updatedSliceCompositeNode);
      }
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
      this->SliceModelNode ? this->SliceModelNode->GetModelDisplayNode() : nullptr;
    if ( sliceDisplayNode)
      {
      sliceDisplayNode->SetVisibility( this->SliceNode->GetSliceVisible() );
      sliceDisplayNode->SetViewNodeIDs( this->SliceNode->GetThreeDViewIDs());
      }
    }
  else if (node == this->SliceCompositeNode)
    {
    this->UpdatePipeline();
    this->InvokeEvent(CompositeModifiedEvent);
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
  // Slice update may trigger redrawing many nodes, pause the render to
  // not spend time with intermediate renderings
  vtkMRMLApplicationLogic* appLogic = this->GetMRMLApplicationLogic();
  if (appLogic)
    {
    appLogic->PauseRender();
    }

  //
  // if we don't have layers yet, create them
  //
  if ( this->BackgroundLayer == nullptr )
    {
    vtkNew<vtkMRMLSliceLayerLogic> layer;
    this->SetBackgroundLayer(layer.GetPointer());
    }
  if ( this->ForegroundLayer == nullptr )
    {
    vtkNew<vtkMRMLSliceLayerLogic> layer;
    this->SetForegroundLayer(layer.GetPointer());
    }
  if ( this->LabelLayer == nullptr )
    {
    vtkNew<vtkMRMLSliceLayerLogic> layer;
    // turn on using the label outline only in this layer
    layer->IsLabelLayerOn();
    this->SetLabelLayer(layer.GetPointer());
    }
  // Update slice plane geometry
  if (this->SliceNode != nullptr
      && this->GetSliceModelNode() != nullptr
      && this->GetMRMLScene() != nullptr
      && this->GetMRMLScene()->GetNodeByID( this->SliceModelNode->GetID() ) != nullptr
      && this->SliceModelNode->GetPolyData() != nullptr )
    {
    int *dims1=nullptr;
    int dims[3];
    vtkMatrix4x4 *textureToRAS = nullptr;
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

    // Force non-zero dimension to avoid "Bad plane coordinate system"
    // error from vtkPlaneSource when slice viewers have a height or width
    // of zero.
    dims[0] = std::max(1, dims[0]);
    dims[1] = std::max(1, dims[1]);

    // set the plane corner point for use in a model
    double inPt[4]={0,0,0,1};
    double outPt[4];
    double *outPt3 = outPt;

    // set the z position to be the active slice (from the lightbox)
    inPt[2] = this->SliceNode->GetActiveSlice();

    vtkPlaneSource* plane = vtkPlaneSource::SafeDownCast(
      this->SliceModelNode->GetPolyDataConnection()->GetProducer());

    int wasModified = this->SliceModelNode->StartModify();

    textureToRAS->MultiplyPoint(inPt, outPt);
    plane->SetOrigin(outPt3);

    inPt[0] = dims[0];
    textureToRAS->MultiplyPoint(inPt, outPt);
    plane->SetPoint1(outPt3);

    inPt[0] = 0;
    inPt[1] = dims[1];
    textureToRAS->MultiplyPoint(inPt, outPt);
    plane->SetPoint2(outPt3);

    this->SliceModelNode->EndModify(wasModified);

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
      }
    }

  // This is called when a slice layer is modified, so pass it on
  // to anyone interested in changes to this sub-pipeline
  this->Modified();

  // All the updates are done, allow rendering again
  if (appLogic)
    {
    appLogic->ResumeRender();
    }
}

//----------------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLSliceLogic::AddSliceNode(const char* layoutName)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("vtkMRMLSliceLogic::AddSliceNode failed: scene is not set");
    return nullptr;
    }
  vtkSmartPointer<vtkMRMLSliceNode> node = vtkSmartPointer<vtkMRMLSliceNode>::Take(
    vtkMRMLSliceNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLSliceNode")));
  node->SetName(layoutName);
  node->SetLayoutName(layoutName);
  this->GetMRMLScene()->AddNode(node);
  this->SetSliceNode(node);
  this->UpdateSliceNodeFromLayout();
  return node;
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

  this->UpdateSliceCompositeNode();

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
    this->BackgroundLayer->SetMRMLScene( nullptr );
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
    this->ForegroundLayer->SetMRMLScene( nullptr );
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
    this->LabelLayer->SetMRMLScene( nullptr );
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
::SetWindowLevel(double newWindow, double newLevel, int layer)
{
  vtkMRMLScalarVolumeNode* volumeNode =
    vtkMRMLScalarVolumeNode::SafeDownCast( this->GetLayerVolumeNode (layer) );
  vtkMRMLScalarVolumeDisplayNode* volumeDisplayNode =
    volumeNode ? volumeNode->GetScalarVolumeDisplayNode() : nullptr;
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
::SetBackgroundWindowLevel(double newWindow, double newLevel)
{
  // 0 is background layer, defined in this::GetLayerVolumeNode
  SetWindowLevel(newWindow, newLevel, 0);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic
::SetForegroundWindowLevel(double newWindow, double newLevel)
{
  // 1 is foreground layer, defined in this::GetLayerVolumeNode
  SetWindowLevel(newWindow, newLevel, 1);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic
::GetBackgroundWindowLevelAndRange(double& window, double& level,
                                         double& rangeLow, double& rangeHigh)
{
  bool autoWindowLevel; // unused, just a placeholder to allow calling the method
  this->GetBackgroundWindowLevelAndRange(window, level, rangeLow, rangeHigh, autoWindowLevel);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic
::GetBackgroundWindowLevelAndRange(double& window, double& level,
                                         double& rangeLow, double& rangeHigh, bool& autoWindowLevel)
{
  vtkMRMLScalarVolumeNode* volumeNode =
    vtkMRMLScalarVolumeNode::SafeDownCast( this->GetLayerVolumeNode (0) );
    // 0 is background layer, defined in this::GetLayerVolumeNode
  vtkMRMLScalarVolumeDisplayNode* volumeDisplayNode = nullptr;
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
    autoWindowLevel = (volumeDisplayNode->GetAutoWindowLevel() != 0);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic
::GetForegroundWindowLevelAndRange(double& window, double& level,
                                         double& rangeLow, double& rangeHigh)
{
  bool autoWindowLevel; // unused, just a placeholder to allow calling the method
  this->GetForegroundWindowLevelAndRange(window, level, rangeLow, rangeHigh, autoWindowLevel);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic
::GetForegroundWindowLevelAndRange(double& window, double& level,
                                         double& rangeLow, double& rangeHigh, bool& autoWindowLevel)
{
  vtkMRMLScalarVolumeNode* volumeNode =
    vtkMRMLScalarVolumeNode::SafeDownCast( this->GetLayerVolumeNode (1) );
    // 0 is background layer, defined in this::GetLayerVolumeNode
  vtkMRMLScalarVolumeDisplayNode* volumeDisplayNode = nullptr;
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
    autoWindowLevel = (volumeDisplayNode->GetAutoWindowLevel() != 0);
    }
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput * vtkMRMLSliceLogic::GetImageDataConnection()
{
  return this->ImageDataConnection;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdateImageData ()
{
  if (this->SliceNode->GetSliceResolutionMode() == vtkMRMLSliceNode::SliceResolutionMatch2DView)
    {
    this->ExtractModelTexture->SetInputConnection( this->Pipeline->Blend->GetOutputPort() );
    this->ImageDataConnection = this->Pipeline->Blend->GetOutputPort();
    }
  else
    {
    this->ExtractModelTexture->SetInputConnection( this->PipelineUVW->Blend->GetOutputPort() );
    }
  // It seems very strange that the imagedata can be null.
  // It should probably be always a valid imagedata with invalid bounds if needed

  if ( (this->GetBackgroundLayer() != nullptr && this->GetBackgroundLayer()->GetImageDataConnection() != nullptr) ||
       (this->GetForegroundLayer() != nullptr && this->GetForegroundLayer()->GetImageDataConnection() != nullptr) ||
       (this->GetLabelLayer() != nullptr && this->GetLabelLayer()->GetImageDataConnection() != nullptr) )
    {
    if (this->ImageDataConnection == nullptr || this->Pipeline->Blend->GetOutputPort()->GetMTime() > this->ImageDataConnection->GetMTime())
      {
      this->ImageDataConnection = this->Pipeline->Blend->GetOutputPort();
      }
    }
  else
    {
    this->ImageDataConnection = nullptr;
    if (this->SliceNode->GetSliceResolutionMode() == vtkMRMLSliceNode::SliceResolutionMatch2DView)
      {
      this->ExtractModelTexture->SetInputConnection( this->ImageDataConnection );
      }
    else
      {
      this->ExtractModelTexture->SetInputConnection(this->PipelineUVW->Blend->GetOutputPort());
      }
    }
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceLogic::UpdateBlendLayers(vtkImageBlend* blend, const std::deque<SliceLayerInfo> &layers)
{
  const int blendPort = 0;
  vtkMTimeType oldBlendMTime = blend->GetMTime();

  bool layersChanged = false;
  int numberOfLayers = layers.size();
  if (numberOfLayers == blend->GetNumberOfInputConnections(blendPort))
    {
    int layerIndex = 0;
    for (std::deque<SliceLayerInfo>::const_iterator layerIt = layers.begin(); layerIt != layers.end(); ++layerIt, ++layerIndex)
      {
      if (layerIt->BlendInput != blend->GetInputConnection(blendPort, layerIndex))
        {
        layersChanged = true;
        break;
        }
      }
    }
  else
    {
    layersChanged = true;
    }
  if (layersChanged)
    {
    blend->RemoveAllInputs();
    int layerIndex = 0;
    for (std::deque<SliceLayerInfo>::const_iterator layerIt = layers.begin(); layerIt != layers.end(); ++layerIt, ++layerIndex)
      {
      blend->AddInputConnection(layerIt->BlendInput);
      }
    }

  // Update opacities
    {
    int layerIndex = 0;
    for (std::deque<SliceLayerInfo>::const_iterator layerIt = layers.begin(); layerIt != layers.end(); ++layerIt, ++layerIndex)
      {
      blend->SetOpacity(layerIndex, layerIt->Opacity);
      }
    }

  bool modified = (blend->GetMTime() > oldBlendMTime);
  return modified;
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
    vtkMRMLVolumeNode *bgnode = nullptr;
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
    vtkMRMLVolumeNode *fgnode = nullptr;
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
    vtkMRMLVolumeNode *lbnode = nullptr;
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

    // Now update the image blend with the background and foreground and label
    // -- layer 0 opacity is ignored, but since not all inputs may be non-0,
    //    we keep track so that someone could, for example, have a 0 background
    //    with a non-0 foreground and label and everything will work with the
    //    label opacity
    //

    vtkAlgorithmOutput* backgroundImagePort = this->BackgroundLayer ? this->BackgroundLayer->GetImageDataConnection() : nullptr;
    vtkAlgorithmOutput* foregroundImagePort = this->ForegroundLayer ? this->ForegroundLayer->GetImageDataConnection() : nullptr;

    vtkAlgorithmOutput* backgroundImagePortUVW = this->BackgroundLayer ? this->BackgroundLayer->GetImageDataConnectionUVW() : nullptr;
    vtkAlgorithmOutput* foregroundImagePortUVW = this->ForegroundLayer ? this->ForegroundLayer->GetImageDataConnectionUVW() : nullptr;

    vtkAlgorithmOutput* labelImagePort = this->LabelLayer ? this->LabelLayer->GetImageDataConnection() : nullptr;
    vtkAlgorithmOutput* labelImagePortUVW = this->LabelLayer ? this->LabelLayer->GetImageDataConnectionUVW() : nullptr;

    std::deque<SliceLayerInfo> layers;
    std::deque<SliceLayerInfo> layersUVW;

    this->Pipeline->AddLayers(layers, this->SliceCompositeNode->GetCompositing(),
      backgroundImagePort, foregroundImagePort, this->SliceCompositeNode->GetForegroundOpacity(),
      labelImagePort, this->SliceCompositeNode->GetLabelOpacity());
    this->PipelineUVW->AddLayers(layersUVW, this->SliceCompositeNode->GetCompositing(),
      backgroundImagePortUVW, foregroundImagePortUVW, this->SliceCompositeNode->GetForegroundOpacity(),
      labelImagePortUVW, this->SliceCompositeNode->GetLabelOpacity());

    if (this->UpdateBlendLayers(this->Pipeline->Blend.GetPointer(), layers))
      {
      modified = 1;
      }
    if (this->UpdateBlendLayers(this->PipelineUVW->Blend.GetPointer(), layersUVW))
      {
      modified = 1;
      }

    //Models
    this->UpdateImageData();
    vtkMRMLDisplayNode* displayNode = this->SliceModelNode ? this->SliceModelNode->GetModelDisplayNode() : nullptr;
    if ( displayNode && this->SliceNode )
      {
      displayNode->SetVisibility( this->SliceNode->GetSliceVisible() );
      displayNode->SetViewNodeIDs( this->SliceNode->GetThreeDViewIDs());
      if ( (this->SliceNode->GetSliceResolutionMode() != vtkMRMLSliceNode::SliceResolutionMatch2DView &&
          !((backgroundImagePortUVW != nullptr) || (foregroundImagePortUVW != nullptr) || (labelImagePortUVW != nullptr) ) ) ||
          (this->SliceNode->GetSliceResolutionMode() == vtkMRMLSliceNode::SliceResolutionMatch2DView &&
          !((backgroundImagePort != nullptr) || (foregroundImagePort != nullptr) || (labelImagePort != nullptr) ) ))
        {
        displayNode->SetTextureImageDataConnection(nullptr);
        }
      else if (displayNode->GetTextureImageDataConnection() != this->ExtractModelTexture->GetOutputPort())
        {
        displayNode->SetTextureImageDataConnection(this->ExtractModelTexture->GetOutputPort());
        }
        if ( this->LabelLayer && this->LabelLayer->GetImageDataConnection())
          {
          displayNode->SetInterpolateTexture(0);
          }
        else
          {
          displayNode->SetInterpolateTexture(1);
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

  if (this->Pipeline->Blend.GetPointer())
    {
    os << indent << "Blend: ";
    this->Pipeline->Blend->PrintSelf(os, nextIndent);
    }
  else
    {
    os << indent << "Blend: (none)\n";
    }

  if (this->PipelineUVW->Blend.GetPointer())
    {
    os << indent << "BlendUVW: ";
    this->PipelineUVW->Blend->PrintSelf(os, nextIndent);
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
  if (this->SliceModelNode != nullptr)
    {
    this->SliceModelNode->SetAndObserveDisplayNodeID(nullptr);
    this->SliceModelNode->SetAndObserveTransformNodeID(nullptr);
    this->SliceModelNode->SetPolyDataConnection(nullptr);
    }
  if (this->SliceModelDisplayNode != nullptr)
    {
    this->SliceModelDisplayNode->SetTextureImageDataConnection(nullptr);
    }

  // Remove Nodes
  if (this->SliceModelNode != nullptr)
    {
    if (this->GetMRMLScene() && this->GetMRMLScene()->IsNodePresent(this->SliceModelNode))
      {
      this->GetMRMLScene()->RemoveNode(this->SliceModelNode);
      }
    this->SliceModelNode->Delete();
    this->SliceModelNode = nullptr;
    }
  if (this->SliceModelDisplayNode != nullptr)
    {
    if (this->GetMRMLScene() && this->GetMRMLScene()->IsNodePresent(this->SliceModelDisplayNode))
      {
      this->GetMRMLScene()->RemoveNode(this->SliceModelDisplayNode);
      }
    this->SliceModelDisplayNode->Delete();
    this->SliceModelDisplayNode = nullptr;
    }
  if (this->SliceModelTransformNode != nullptr)
    {
    if (this->GetMRMLScene() && this->GetMRMLScene()->IsNodePresent(this->SliceModelTransformNode))
      {
      this->GetMRMLScene()->RemoveNode(this->SliceModelTransformNode);
      }
    this->SliceModelTransformNode->Delete();
    this->SliceModelTransformNode = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::CreateSliceModel()
{
  if(!this->GetMRMLScene())
    {
    return;
    }

  if (this->SliceModelNode != nullptr &&
      this->GetMRMLScene()->GetNodeByID(this->GetSliceModelNode()->GetID()) == nullptr )
    {
    this->DeleteSliceModel();
    }

  if ( this->SliceModelNode == nullptr)
    {
    this->SliceModelNode = vtkMRMLModelNode::New();
    this->SliceModelNode->SetScene(this->GetMRMLScene());
    this->SliceModelNode->SetDisableModifiedEvent(1);

    this->SliceModelNode->SetHideFromEditors(1);
    // allow point picking (e.g., placing a markups point on the slice node)
    this->SliceModelNode->SetSelectable(1);
    this->SliceModelNode->SetSaveWithScene(0);

    // create plane slice
    vtkNew<vtkPlaneSource> planeSource;
    planeSource->Update();
    this->SliceModelNode->SetPolyDataConnection(planeSource->GetOutputPort());
    this->SliceModelNode->SetDisableModifiedEvent(0);

    // create display node and set texture
    this->SliceModelDisplayNode = vtkMRMLSliceDisplayNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLSliceDisplayNode"));
    this->SliceModelDisplayNode->SetScene(this->GetMRMLScene());
    this->SliceModelDisplayNode->SetDisableModifiedEvent(1);

    //this->SliceModelDisplayNode->SetInputPolyData(this->SliceModelNode->GetOutputPolyData());
    this->SliceModelDisplayNode->SetVisibility(0);
    this->SliceModelDisplayNode->SetOpacity(1);
    this->SliceModelDisplayNode->SetColor(1,1,1);
    std::string displayName = "Slice Display";
    std::string modelNodeName = "Slice " + this->SLICE_MODEL_NODE_NAME_SUFFIX;
    std::string transformNodeName = "Slice Transform";
    if (this->SliceNode && this->SliceNode->GetLayoutName())
      {
      // Auto-set the colors based on the slice node
      this->SliceModelDisplayNode->SetColor(this->SliceNode->GetLayoutColor());
      displayName = this->SliceNode->GetLayoutName() + std::string(" Display");
      modelNodeName = this->SliceNode->GetLayoutName() + std::string(" ") + this->SLICE_MODEL_NODE_NAME_SUFFIX;
      transformNodeName = this->SliceNode->GetLayoutName() + std::string(" Transform");
      }
    this->SliceModelDisplayNode->SetAmbient(1);
    this->SliceModelDisplayNode->SetBackfaceCulling(0);
    this->SliceModelDisplayNode->SetDiffuse(0);
    this->SliceModelDisplayNode->SetTextureImageDataConnection(this->ExtractModelTexture->GetOutputPort());
    this->SliceModelDisplayNode->SetSaveWithScene(0);
    this->SliceModelDisplayNode->SetDisableModifiedEvent(0);
    // set an attribute to distinguish this from regular model display nodes
    this->SliceModelDisplayNode->SetAttribute("SliceLogic.IsSliceModelDisplayNode", "True");
    this->SliceModelDisplayNode->SetName(this->GetMRMLScene()->GenerateUniqueName(displayName).c_str());

    this->SliceModelNode->SetName(modelNodeName.c_str());

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
    this->SliceModelTransformNode->SetName(this->GetMRMLScene()->GenerateUniqueName(transformNodeName).c_str());

    this->SliceModelTransformNode->SetDisableModifiedEvent(0);

    }

  if (this->SliceModelNode != nullptr && this->GetMRMLScene()->GetNodeByID( this->GetSliceModelNode()->GetID() ) == nullptr )
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
  if ( this->SliceModelNode != nullptr )
    {
    char description[256];
    std::stringstream ssD;
    if (this->SliceNode && this->SliceNode->GetID() )
      {
      ssD << " SliceID " << this->SliceNode->GetID();
      }
    if (this->SliceCompositeNode && this->SliceCompositeNode->GetID() )
      {
      ssD << " CompositeID " << this->SliceCompositeNode->GetID();
      }

    ssD.getline(description,256);
    this->SliceModelNode->SetDescription(description);
    }
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode *vtkMRMLSliceLogic::GetLayerVolumeNode(int layer)
{
  if (!this->SliceNode || !this->SliceCompositeNode)
    {
    return (nullptr);
    }

  const char *id = nullptr;
  switch (layer)
    {
    case LayerBackground:
      {
      id = this->SliceCompositeNode->GetBackgroundVolumeID();
      break;
      }
    case LayerForeground:
      {
      id = this->SliceCompositeNode->GetForegroundVolumeID();
      break;
      }
    case LayerLabel:
      {
      id = this->SliceCompositeNode->GetLabelVolumeID();
      break;
      }
    }
  vtkMRMLScene* scene = this->GetMRMLScene();
  return scene ? vtkMRMLVolumeNode::SafeDownCast(
    scene->GetNodeByID( id )) : nullptr;
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
void vtkMRMLSliceLogic::GetVolumeSliceBounds(vtkMRMLVolumeNode *volumeNode,
  double sliceBounds[6], bool useVoxelCenter/*=false*/)
{
  if (this->SliceNode == nullptr || volumeNode == nullptr)
    {
    sliceBounds[0] = sliceBounds[1] = 0.0;
    sliceBounds[2] = sliceBounds[3] = 0.0;
    sliceBounds[4] = sliceBounds[5] = 0.0;
    return;
    }
  //
  // figure out how big that volume is on this particular slice plane
  //
  vtkNew<vtkMatrix4x4> rasToSlice;
  rasToSlice->DeepCopy(this->SliceNode->GetSliceToRAS());
  rasToSlice->SetElement(0, 3, 0.0);
  rasToSlice->SetElement(1, 3, 0.0);
  rasToSlice->SetElement(2, 3, 0.0);
  rasToSlice->Invert();

  volumeNode->GetSliceBounds(sliceBounds, rasToSlice.GetPointer(), useVoxelCenter);
}

//----------------------------------------------------------------------------
// Get the spacing of the volume, transformed to slice space
double *vtkMRMLSliceLogic::GetVolumeSliceSpacing(vtkMRMLVolumeNode *volumeNode)
{
  if ( !volumeNode )
    {
    return (this->SliceSpacing);
    }

  if (!this->SliceNode)
    {
    return (this->SliceSpacing);
    }

  if (this->SliceNode->GetSliceSpacingMode() == vtkMRMLSliceNode::PrescribedSliceSpacingMode)
    {
    // jvm - should we cache the PrescribedSliceSpacing in SliceSpacing?
    double *pspacing = this->SliceNode->GetPrescribedSliceSpacing();
    this->SliceSpacing[0] = pspacing[0];
    this->SliceSpacing[1] = pspacing[1];
    this->SliceSpacing[2] = pspacing[2];
    return (pspacing);
    }

  // Compute slice spacing from the volume axis closest matching the slice axis, projected to the slice axis.

  vtkNew<vtkMatrix4x4> ijkToWorld;
  volumeNode->GetIJKToRASMatrix(ijkToWorld);

  // Apply transform to the volume axes, if the volume is transformed with a linear transform
  vtkMRMLTransformNode *transformNode = volumeNode->GetParentTransformNode();
  if ( transformNode != nullptr &&  transformNode->IsTransformToWorldLinear() )
    {
    vtkNew<vtkMatrix4x4> volumeRASToWorld;
    transformNode->GetMatrixTransformToWorld(volumeRASToWorld);
    //rasToRAS->Invert();
    vtkMatrix4x4::Multiply4x4(volumeRASToWorld, ijkToWorld, ijkToWorld);
    }

  vtkNew<vtkMatrix4x4> worldToIJK;
  vtkMatrix4x4::Invert(ijkToWorld, worldToIJK);
  vtkNew<vtkMatrix4x4> sliceToIJK;
  vtkMatrix4x4::Multiply4x4(worldToIJK, this->SliceNode->GetSliceToRAS(), sliceToIJK);
  vtkNew<vtkMatrix4x4> ijkToSlice;
  vtkMatrix4x4::Invert(sliceToIJK, ijkToSlice);

  // Find the volume IJK axis that has the most similar direction to the slice axis.
  // Use the spacing component of this volume IJK axis parallel to the slice axis.
  double scale[3]; // unused
  vtkAddonMathUtilities::NormalizeOrientationMatrixColumns(sliceToIJK, scale);
  // after normalization, sliceToIJK only contains slice axis directions
  for (int sliceAxisIndex = 0; sliceAxisIndex < 3; sliceAxisIndex++)
    {
    // Slice axis direction in IJK coordinate system
    double sliceAxisDirection_I = fabs(sliceToIJK->GetElement(0, sliceAxisIndex));
    double sliceAxisDirection_J = fabs(sliceToIJK->GetElement(1, sliceAxisIndex));
    double sliceAxisDirection_K = fabs(sliceToIJK->GetElement(2, sliceAxisIndex));
    if (sliceAxisDirection_I > sliceAxisDirection_J)
      {
      if (sliceAxisDirection_I > sliceAxisDirection_K)
        {
        // this sliceAxis direction is closest volume I axis direction
        this->SliceSpacing[sliceAxisIndex] = fabs(ijkToSlice->GetElement(sliceAxisIndex, 0 /*I*/));
        }
      else
        {
        // this sliceAxis direction is closest volume K axis direction
        this->SliceSpacing[sliceAxisIndex] = fabs(ijkToSlice->GetElement(sliceAxisIndex, 2 /*K*/));
        }
      }
    else
      {
      if (sliceAxisDirection_J > sliceAxisDirection_K)
        {
        // this sliceAxis direction is closest volume J axis direction
        this->SliceSpacing[sliceAxisIndex] = fabs(ijkToSlice->GetElement(sliceAxisIndex, 1 /*J*/));
        }
      else
        {
        // this sliceAxis direction is closest volume K axis direction
        this->SliceSpacing[sliceAxisIndex] = fabs(ijkToSlice->GetElement(sliceAxisIndex, 2 /*K*/));
        }
      }
    }

  return this->SliceSpacing;
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

  if (!this->SliceNode)
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
  fitZ = this->GetVolumeSliceSpacing(volumeNode)[2] * this->SliceNode->GetDimensions()[2];


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

  this->SliceNode->SetFieldOfView(fitX, fitY, fitZ);

  //
  // set the origin to be the center of the volume in RAS
  //
  vtkNew<vtkMatrix4x4> sliceToRAS;
  sliceToRAS->DeepCopy(this->SliceNode->GetSliceToRAS());
  sliceToRAS->SetElement(0, 3, rasCenter[0]);
  sliceToRAS->SetElement(1, 3, rasCenter[1]);
  sliceToRAS->SetElement(2, 3, rasCenter[2]);
  this->SliceNode->GetSliceToRAS()->DeepCopy(sliceToRAS.GetPointer());
  this->SliceNode->SetSliceOrigin(0,0,0);
  //sliceNode->SetSliceOffset(offset);

  //TODO Fit UVW space
  this->SnapSliceOffsetToIJK();
  this->SliceNode->UpdateMatrices( );
}

//----------------------------------------------------------------------------
// Get the size of the volume, transformed to RAS space
void vtkMRMLSliceLogic::GetBackgroundRASBox(double rasDimensions[3], double rasCenter[3])
{
  vtkMRMLVolumeNode *backgroundNode = nullptr;
  backgroundNode = this->GetLayerVolumeNode (0);
  this->GetVolumeRASBox( backgroundNode, rasDimensions, rasCenter );
}

//----------------------------------------------------------------------------
// Get the size of the volume, transformed to RAS space
void vtkMRMLSliceLogic::GetBackgroundSliceDimensions(double sliceDimensions[3], double sliceCenter[3])
{
  vtkMRMLVolumeNode *backgroundNode = nullptr;
  backgroundNode = this->GetLayerVolumeNode (0);
  this->GetVolumeSliceDimensions( backgroundNode, sliceDimensions, sliceCenter );
}

//----------------------------------------------------------------------------
// Get the spacing of the volume, transformed to slice space
double *vtkMRMLSliceLogic::GetBackgroundSliceSpacing()
{
  vtkMRMLVolumeNode *backgroundNode = nullptr;
  backgroundNode = this->GetLayerVolumeNode (0);
  return (this->GetVolumeSliceSpacing( backgroundNode ));
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::GetBackgroundSliceBounds(double sliceBounds[6])
{
  vtkMRMLVolumeNode *backgroundNode = nullptr;
  backgroundNode = this->GetLayerVolumeNode (0);
  this->GetVolumeSliceBounds(backgroundNode, sliceBounds);
}

//----------------------------------------------------------------------------
// adjust the node's field of view to match the extent of current background volume
void vtkMRMLSliceLogic::FitSliceToBackground(int width, int height)
{
  vtkMRMLVolumeNode *backgroundNode = nullptr;
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
    backgroundNode ? backgroundNode->GetImageData() : nullptr;
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
void vtkMRMLSliceLogic::GetLowestVolumeSliceBounds(double sliceBounds[6], bool useVoxelCenter/*=false*/)
{
  vtkMRMLVolumeNode *volumeNode;
  for ( int layer=0; layer < 3; layer++ )
    {
    volumeNode = this->GetLayerVolumeNode (layer);
    if (volumeNode)
      {
      return this->GetVolumeSliceBounds(volumeNode, sliceBounds, useVoxelCenter);
      }
    }
  // return the default values
  return this->GetVolumeSliceBounds(nullptr, sliceBounds, useVoxelCenter);
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

  if ( !this->SliceNode )
    {
    return 0.0;
    }

  return this->SliceNode->GetSliceOffset();

}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetSliceOffset(double offset)
{
  // this method has been moved to vtkMRMLSliceNode
  // the API stays for backwards compatibility
  if (!this->SliceNode)
    {
    return;
    }
  this->SliceNode->SetSliceOffset(offset);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::StartSliceCompositeNodeInteraction(unsigned int parameters)
{
  if (!this->SliceCompositeNode)
    {
    return;
    }

  // Cache the flags on what parameters are going to be modified. Need
  // to this this outside the conditional on HotLinkedControl and LinkedControl
  this->SliceCompositeNode->SetInteractionFlags(parameters);

  // If we have hot linked controls, then we want to broadcast changes
  if (this->SliceCompositeNode->GetHotLinkedControl() && this->SliceCompositeNode->GetLinkedControl())
    {
    this->SliceCompositeNode->InteractingOn();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::EndSliceCompositeNodeInteraction()
{
  if (!this->SliceCompositeNode)
    {
    return;
    }
  // If we have linked controls, then we want to broadcast changes
  if (this->SliceCompositeNode->GetLinkedControl())
    {
    // Need to trigger a final message to broadcast to all the nodes
    // that are linked
    this->SliceCompositeNode->InteractingOn();
    this->SliceCompositeNode->Modified();
    this->SliceCompositeNode->InteractingOff();
    this->SliceCompositeNode->SetInteractionFlags(0);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::StartSliceNodeInteraction(unsigned int parameters)
{
  if (this->SliceNode == nullptr || this->SliceCompositeNode == nullptr)
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
  if (this->SliceNode == nullptr)
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
  if (this->SliceNode == nullptr || this->SliceCompositeNode == nullptr)
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
  vtkImageData *volumeImage=nullptr;
  if ( !(volumeImage = volumeNode->GetImageData()) )
    {
    return SLICE_INDEX_NO_VOLUME;
    }
  if (!this->SliceNode)
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
  rasToSlice->DeepCopy(this->SliceNode->GetSliceToRAS());
  rasToSlice->Invert();

  double sliceNormal_IJK[4]={0,0,1,0};  // slice normal vector in IJK coordinate system
  double sliceNormal_RAS[4]={0,0,0,0};  // slice normal vector in RAS coordinate system
  this->SliceNode->GetSliceToRAS()->MultiplyPoint(sliceNormal_IJK, sliceNormal_RAS);

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
    sliceNode->GetScene(), sliceNode->GetLayoutName()) : nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode* vtkMRMLSliceLogic
::GetSliceCompositeNode(vtkMRMLScene* scene, const char* layoutName)
{
  if (!scene || !layoutName)
    {
    return nullptr;
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
  return nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLSliceLogic
::GetSliceNode(vtkMRMLSliceCompositeNode* sliceCompositeNode)
{
  if (!sliceCompositeNode)
    {
    return nullptr;
    }
  return sliceCompositeNode ? vtkMRMLSliceLogic::GetSliceNode(
    sliceCompositeNode->GetScene(), sliceCompositeNode->GetLayoutName()) : nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLSliceLogic
::GetSliceNode(vtkMRMLScene* scene, const char* layoutName)
{
  if (!scene || !layoutName)
    {
    return nullptr;
    }
  vtkObject* itNode = nullptr;
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
  return nullptr;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceLogic::IsSliceModelNode(vtkMRMLNode *mrmlNode)
{
  if (mrmlNode != nullptr &&
      mrmlNode->IsA("vtkMRMLModelNode") &&
      mrmlNode->GetName() != nullptr &&
      strstr(mrmlNode->GetName(), vtkMRMLSliceLogic::SLICE_MODEL_NODE_NAME_SUFFIX.c_str()) != nullptr)
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceLogic::IsSliceModelDisplayNode(vtkMRMLDisplayNode *mrmlDisplayNode)
{
  if (vtkMRMLSliceDisplayNode::SafeDownCast(mrmlDisplayNode))
    {
    return true;
    }
  if (mrmlDisplayNode != nullptr &&
      mrmlDisplayNode->IsA("vtkMRMLModelDisplayNode"))
    {
    const char *attrib = mrmlDisplayNode->GetAttribute("SliceLogic.IsSliceModelDisplayNode");
    // allow the attribute to be set to anything but 0
    if (attrib != nullptr &&
        strcmp(attrib, "0") != 0)
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
vtkImageBlend* vtkMRMLSliceLogic::GetBlend()
{
  return this->Pipeline->Blend.GetPointer();
}

//----------------------------------------------------------------------------
vtkImageBlend* vtkMRMLSliceLogic::GetBlendUVW()
{
  return this->PipelineUVW->Blend.GetPointer();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::RotateSliceToLowestVolumeAxes(bool forceSlicePlaneToSingleSlice/*=true*/)
{
  vtkMRMLVolumeNode* volumeNode;
  for (int layer = 0; layer < 3; layer++)
    {
    volumeNode = this->GetLayerVolumeNode(layer);
    if (volumeNode)
      {
      break;
      }
    }
  if (!volumeNode)
    {
    return;
    }
  vtkMRMLSliceNode* sliceNode = this->GetSliceNode();
  if (!sliceNode)
    {
    return;
    }
  sliceNode->RotateToVolumePlane(volumeNode, forceSlicePlaneToSingleSlice);
  this->SnapSliceOffsetToIJK();
}

//----------------------------------------------------------------------------
int vtkMRMLSliceLogic::GetEditableLayerAtWorldPosition(double worldPos[3],
  bool backgroundVolumeEditable/*=true*/, bool foregroundVolumeEditable/*=true*/)
{
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if (!sliceNode)
    {
    return vtkMRMLSliceLogic::LayerNone;
    }
  vtkMRMLSliceCompositeNode *sliceCompositeNode = this->GetSliceCompositeNode();
  if (!sliceCompositeNode)
    {
    return vtkMRMLSliceLogic::LayerNone;
    }

  bool foregroundEditable = this->VolumeWindowLevelEditable(sliceCompositeNode->GetForegroundVolumeID())
    && foregroundVolumeEditable;
  bool backgroundEditable = this->VolumeWindowLevelEditable(sliceCompositeNode->GetBackgroundVolumeID())
    && backgroundVolumeEditable;

  if (!foregroundEditable && !backgroundEditable)
    {
    // window/level editing is disabled on both volumes
    return vtkMRMLSliceLogic::LayerNone;
    }
  // By default adjust background volume, if available
  bool adjustForeground = !backgroundEditable;

  // If both foreground and background volumes are visible then choose adjustment of
  // foreground volume, if foreground volume is visible in current mouse position
  if (foregroundEditable && backgroundEditable)
    {
    adjustForeground = (sliceCompositeNode->GetForegroundOpacity() >= 0.01)
      && this->IsEventInsideVolume(true, worldPos)   // inside background (used as mask for displaying foreground)
      && this->vtkMRMLSliceLogic::IsEventInsideVolume(false, worldPos); // inside foreground
    }

  return (adjustForeground ? vtkMRMLSliceLogic::LayerForeground : vtkMRMLSliceLogic::LayerBackground);
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceLogic::VolumeWindowLevelEditable(const char* volumeNodeID)
{
  if (!volumeNodeID)
    {
    return false;
    }
  vtkMRMLScene *scene = this->GetMRMLScene();
  if (!scene)
    {
    return false;
    }
  vtkMRMLVolumeNode* volumeNode =
    vtkMRMLVolumeNode::SafeDownCast(scene->GetNodeByID(volumeNodeID));
  if (volumeNode == nullptr)
    {
    return false;
    }
  vtkMRMLScalarVolumeDisplayNode* scalarVolumeDisplayNode =
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(volumeNode->GetVolumeDisplayNode());
  if (!scalarVolumeDisplayNode)
    {
    return false;
    }
  return !scalarVolumeDisplayNode->GetWindowLevelLocked();
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceLogic::IsEventInsideVolume(bool background, double worldPos[3])
{
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if (!sliceNode)
    {
    return false;
    }
  vtkMRMLSliceLayerLogic* layerLogic = background ?
    this->GetBackgroundLayer() : this->GetForegroundLayer();
  if (!layerLogic)
    {
    return false;
    }
  vtkMRMLVolumeNode* volumeNode = layerLogic->GetVolumeNode();
  if (!volumeNode || !volumeNode->GetImageData())
    {
    return false;
    }

  vtkNew<vtkGeneralTransform> inputVolumeIJKToWorldTransform;
  inputVolumeIJKToWorldTransform->PostMultiply();

  vtkNew<vtkMatrix4x4> inputVolumeIJK2RASMatrix;
  volumeNode->GetIJKToRASMatrix(inputVolumeIJK2RASMatrix);
  inputVolumeIJKToWorldTransform->Concatenate(inputVolumeIJK2RASMatrix);

  vtkNew<vtkGeneralTransform> inputVolumeRASToWorld;
  vtkMRMLTransformNode::GetTransformBetweenNodes(volumeNode->GetParentTransformNode(), nullptr, inputVolumeRASToWorld);
  inputVolumeIJKToWorldTransform->Concatenate(inputVolumeRASToWorld);

  double ijkPos[3] = { 0.0, 0.0, 0.0 };
  inputVolumeIJKToWorldTransform->GetInverse()->TransformPoint(worldPos, ijkPos);

  int volumeExtent[6] = { 0 };
  volumeNode->GetImageData()->GetExtent(volumeExtent);
  for (int i = 0; i < 3; i++)
    {
    if (ijkPos[i]<volumeExtent[i * 2] || ijkPos[i]>volumeExtent[i * 2 + 1])
      {
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
vtkMRMLSliceDisplayNode* vtkMRMLSliceLogic::GetSliceDisplayNode()
{
  return vtkMRMLSliceDisplayNode::SafeDownCast(this->GetSliceModelDisplayNode());
}

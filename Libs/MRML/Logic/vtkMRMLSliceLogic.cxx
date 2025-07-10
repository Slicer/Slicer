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
#include "vtkMRMLSliceLayerLogic.h"
#include "vtkMRMLSliceLogic.h"

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLCrosshairNode.h>
#include <vtkMRMLGlyphableVolumeDisplayNode.h>
#include <vtkMRMLGlyphableVolumeSliceDisplayNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLProceduralColorNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceDisplayNode.h>

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkCollectionIterator.h>
#include <vtkGeneralTransform.h>
#include <vtkImageAppendComponents.h>
#include <vtkImageBlend.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageMathematics.h>
#include <vtkImageReslice.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlaneSource.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTransform.h>

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
    //   foreground[N] \
    //                 .
    //                 .
    //                 .
    //   foreground[0] \
    //                  > Blend
    //      background /
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
    //
    //            foreground[N] > AddSubCasts[N] > FractionMaths[N] \
    //                                                              .
    //                                                              .
    //                                                              .
    //            foreground[0] > AddSubCasts[0] > FractionMaths[0] \
    //                                                               > AddSubMath > AddSubOutputCast ...
    //                            background > AddSubBackgroundCast /
    //
    //
    //                          ... AddSubOutputCast > AddSubExtractRGB \
    //                                                                   > AddSubAppendRGBA
    //         background > AddSubExtractBackgroundAlpha - > BlendAlpha /
    //                                                    /
    //            foreground[0] > AddSubExtractAlphas[0] /
    //                                                   .
    //                                                   .
    //                                                   .
    //            additional[N] > AddSubExtractAlphas[N] /
    //
    */

    this->AddSubBackgroundCast->SetOutputScalarTypeToShort();
    // See UpdateStages() for update to AddSubBackgroundCast, AddSubCasts
    // and FractionMaths input connections.

    this->AddSubMath->SetOperationToAdd();
    // See UpdateStages() for update to AddSubMath input connections.

    this->AddSubOutputCast->SetInputConnection(this->AddSubMath->GetOutputPort());
    this->AddSubOutputCast->SetOutputScalarTypeToUnsignedChar();
    this->AddSubOutputCast->ClampOverflowOn();

    this->AddSubExtractRGB->SetInputConnection(this->AddSubOutputCast->GetOutputPort());
    this->AddSubExtractRGB->SetComponents(0, 1, 2);

    this->AddSubExtractBackgroundAlpha->SetComponents(3);
    // See UpdateStages() for update to AddSubExtractBackgroundAlpha, AddSubExtractAlphas
    // and BlendAlpha input connections.

    this->AddSubAppendRGBA->AddInputConnection(this->AddSubExtractRGB->GetOutputPort());
    this->AddSubAppendRGBA->AddInputConnection(this->BlendAlpha->GetOutputPort());
  }

  //----------------------------------------------------------------------------
  bool UpdateStages(const std::vector<vtkAlgorithmOutput*>& imagePorts)
  {
    std::vector<vtkAlgorithmOutput*> foregroundImagePorts = imagePorts;
    if (imagePorts.size() > 0)
    {
      foregroundImagePorts.erase(foregroundImagePorts.begin());
    }

    bool stagesChanged = false;
    int numberOfStages = static_cast<int>(foregroundImagePorts.size());
    int currentNumberOfStages = static_cast<int>(this->AddSubExtractAlphas.size());
    if (numberOfStages > currentNumberOfStages)
    {
      // Add missing stages
      for (int count = 0; count < numberOfStages - currentNumberOfStages; ++count)
      {
        vtkNew<vtkImageCast> addSubCast;
        addSubCast->SetOutputScalarTypeToShort();
        this->AddSubCasts.push_back(addSubCast);

        vtkNew<vtkImageMathematics> fractionMath;
        fractionMath->SetConstantK(1.0);
        fractionMath->SetOperationToMultiplyByK();
        fractionMath->SetInputConnection(0, addSubCast->GetOutputPort());
        this->FractionMaths.push_back(fractionMath);

        vtkNew<vtkImageExtractComponents> addSubExtractAlpha;
        addSubExtractAlpha->SetComponents(3);
        this->AddSubExtractAlphas.push_back(addSubExtractAlpha);
      }
      stagesChanged = true;
    }
    else if (numberOfStages < currentNumberOfStages)
    {
      // Truncate
      for (int count = 0; count < currentNumberOfStages - numberOfStages; ++count)
      {
        this->AddSubCasts.pop_back();
        this->FractionMaths.pop_back();
        this->AddSubExtractAlphas.pop_back();
      }
      stagesChanged = true;
    }
    if (stagesChanged)
    {
      // Reset BlendAlpha connections
      this->BlendAlpha->RemoveAllInputs();
      this->BlendAlpha->AddInputConnection(this->AddSubExtractBackgroundAlpha->GetOutputPort());
      for (vtkSmartPointer<vtkImageExtractComponents>& addSubExtractAlpha: this->AddSubExtractAlphas)
      {
        this->BlendAlpha->AddInputConnection(addSubExtractAlpha->GetOutputPort());
      }
      // Clear AddSubMath inputs
      this->AddSubMath->RemoveAllInputs();
    }
    return stagesChanged;
  }

  //----------------------------------------------------------------------------
  void AddLayers(std::deque<SliceLayerInfo>& layers,
    int sliceCompositing, bool clipToBackgroundVolume,
    const std::vector<vtkAlgorithmOutput*>& imagePorts, const std::vector<double>& opacities,
    vtkAlgorithmOutput* labelImagePort, double labelOpacity)
  {

    if (sliceCompositing == vtkMRMLSliceCompositeNode::Add || sliceCompositing == vtkMRMLSliceCompositeNode::Subtract)
    {
      if (imagePorts.size() < 2)
      {
        // not enough inputs for add/subtract, so use alpha blending pipeline
        sliceCompositing = vtkMRMLSliceCompositeNode::Alpha;
      }
    }

    if (sliceCompositing == vtkMRMLSliceCompositeNode::Alpha)
    {
      for (int index = 0; index < static_cast<int>(imagePorts.size()); ++index)
      {
        layers.emplace_back(imagePorts[index], opacities[index]);
      }
    }
    else if (sliceCompositing == vtkMRMLSliceCompositeNode::ReverseAlpha)
    {
      for (int index = static_cast<int>(imagePorts.size()) - 1; index >= 0; --index)
      {
        layers.emplace_back(imagePorts[index], opacities[index]);
      }
    }
    else
    {
      // Background
      vtkAlgorithmOutput* backgroundImagePort = imagePorts.front();
      this->AddSubBackgroundCast->SetInputConnection(backgroundImagePort);
      this->AddSubMath->SetInputConnection(0, this->AddSubBackgroundCast->GetOutputPort());
      // See UpdateAddSubOperation() for update to AddSubMath operation.
      this->AddSubExtractBackgroundAlpha->SetInputConnection(backgroundImagePort);

      // Foreground(s)
      std::vector<vtkAlgorithmOutput*> foregroundPorts = imagePorts;
      foregroundPorts.erase(foregroundPorts.begin());
      for (int stageIndex = 0; stageIndex < static_cast<int>(foregroundPorts.size()); ++stageIndex)
      {
        this->AddSubCasts[stageIndex]->SetInputConnection(foregroundPorts[stageIndex]);
        this->AddSubExtractAlphas[stageIndex]->SetInputConnection(foregroundPorts[stageIndex]);
        this->AddSubMath->SetInputConnection(stageIndex, this->FractionMaths[stageIndex]->GetOutputPort());
      }

      // If clip to background is disabled, blending occurs over the entire extent
      // of all layers, not just within the background volume region.
      if (!clipToBackgroundVolume)
      {
        this->BlendAlpha->SetOpacity(0, 0.5); // Background
        for (int index = 1; index < static_cast<int>(imagePorts.size()); ++index)
        {
          this->BlendAlpha->SetOpacity(index, 0.5);
        }
      }
      else
      {
        this->BlendAlpha->SetOpacity(0, 1.); // Background
        for (int index = 1; index < static_cast<int>(imagePorts.size()); ++index)
        {
          this->BlendAlpha->SetOpacity(index, 0.0);
        }
      }

      layers.emplace_back(this->AddSubAppendRGBA->GetOutputPort(), 1.0);
    }

    // always blending the label layer
    if (labelImagePort)
    {
      layers.emplace_back(labelImagePort, labelOpacity);
    }
  }

  vtkNew<vtkImageCast> AddSubBackgroundCast;
  std::vector<vtkSmartPointer<vtkImageCast>> AddSubCasts;

  vtkNew<vtkImageMathematics> AddSubMath;
  std::vector<vtkSmartPointer<vtkImageMathematics>> FractionMaths;

  vtkNew<vtkImageExtractComponents> AddSubExtractRGB;
  vtkNew<vtkImageExtractComponents> AddSubExtractBackgroundAlpha;
  std::vector<vtkSmartPointer<vtkImageExtractComponents>> AddSubExtractAlphas;

  vtkNew<vtkImageBlend> BlendAlpha;
  vtkNew<vtkImageAppendComponents> AddSubAppendRGBA;
  vtkNew<vtkImageCast> AddSubOutputCast;
  vtkNew<vtkImageBlend> Blend;
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLSliceLogic);

//----------------------------------------------------------------------------
vtkMRMLSliceLogic::vtkMRMLSliceLogic()
{
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

  for (int layerIndex = 0; layerIndex < static_cast<int>(this->Layers.size()); ++layerIndex)
  {
    this->SetNthLayer(layerIndex, nullptr);
  }

  if (this->SliceCompositeNode)
  {
    vtkSetAndObserveMRMLNodeMacro( this->SliceCompositeNode, 0);
  }

  this->DeleteSliceModel();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
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

  // ProcessMRMLLogicsEvents() ensures that the background, foreground, and label layers exist.
  this->ProcessMRMLLogicsEvents();

  this->GetBackgroundLayer()->SetMRMLScene(newScene);
  this->GetForegroundLayer()->SetMRMLScene(newScene);
  this->GetLabelLayer()->SetMRMLScene(newScene);

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
  vtkSmartPointer<vtkMRMLSliceCompositeNode> updatedSliceCompositeNode =
      vtkMRMLSliceLogic::GetSliceCompositeNode(this->GetMRMLScene(), layoutName.c_str());

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
      updatedSliceCompositeNode = vtkSmartPointer<vtkMRMLSliceCompositeNode>::Take(vtkMRMLSliceCompositeNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLSliceCompositeNode")));
      updatedSliceCompositeNode->SetLayoutName(layoutName.c_str());
      this->GetMRMLScene()->AddNode(updatedSliceCompositeNode);
      this->SetSliceCompositeNode(updatedSliceCompositeNode);
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

    vtkMRMLSliceLogic::UpdateReconstructionSlab(this, this->GetBackgroundLayer());
    vtkMRMLSliceLogic::UpdateReconstructionSlab(this, this->GetForegroundLayer());
    for (int additionalLayerIndex = 0;
        additionalLayerIndex < this->SliceCompositeNode->GetNumberOfAdditionalLayers();
        ++additionalLayerIndex)
    {
      vtkMRMLSliceLogic::UpdateReconstructionSlab(
            this, this->GetNthLayer(vtkMRMLSliceLogic::Layer_Last + additionalLayerIndex));
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
  if (this->GetBackgroundLayer() == nullptr)
  {
    vtkNew<vtkMRMLSliceLayerLogic> layer;
    this->SetBackgroundLayer(layer.GetPointer());
  }
  if (this->GetForegroundLayer() == nullptr)
  {
    vtkNew<vtkMRMLSliceLayerLogic> layer;
    this->SetForegroundLayer(layer.GetPointer());
  }
  if (this->GetLabelLayer() == nullptr)
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
    int* dims1 = nullptr;
    int dims[3];
    vtkSmartPointer<vtkMatrix4x4> textureToRAS;
    // If the slice resolution mode is not set to match the 2D view, use UVW dimensions
    if (this->SliceNode->GetSliceResolutionMode() != vtkMRMLSliceNode::SliceResolutionMatch2DView)
    {
      textureToRAS = this->SliceNode->GetUVWToRAS();
      dims1 = this->SliceNode->GetUVWDimensions();
      dims[0] = dims1[0]-1;
      dims[1] = dims1[1]-1;
    }
    else // If the slice resolution mode is set to match the 2D view, use texture computed by slice view
    {
      // Create a new textureToRAS matrix with translation to correct texture pixel origin
      //
      // Since the OpenGL texture pixel origin is in the pixel corner and the
      // VTK pixel origin is in the pixel center, we need to shift the coordinate
      // by half voxel.
      //
      // Considering that the translation matrix is almost an identity matrix, the
      // computation easily and efficiently performed by elementary operations on
      // the matrix elements.
      textureToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
      textureToRAS->DeepCopy(this->SliceNode->GetXYToRAS());
      textureToRAS->SetElement(0, 3, textureToRAS->GetElement(0, 3)
        - 0.5 * textureToRAS->GetElement(0, 0) - 0.5 * textureToRAS->GetElement(0, 1)); // Shift by half voxel
      textureToRAS->SetElement(1, 3, textureToRAS->GetElement(1, 3)
        - 0.5 * textureToRAS->GetElement(1, 0) - 0.5 * textureToRAS->GetElement(1, 1)); // Shift by half voxel

      // Use XY dimensions for slice node if resolution mode is set to match 2D view
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
    double inPoint[4]={0,0,0,1};
    double outPoint[4];
    double* outPoint3 = outPoint;

    // set the z position to be the active slice (from the lightbox)
    inPoint[2] = this->SliceNode->GetActiveSlice();

    vtkPlaneSource* plane = vtkPlaneSource::SafeDownCast(
      this->SliceModelNode->GetPolyDataConnection()->GetProducer());

    int wasModified = this->SliceModelNode->StartModify();

    textureToRAS->MultiplyPoint(inPoint, outPoint);
    plane->SetOrigin(outPoint3);

    inPoint[0] = dims[0];
    textureToRAS->MultiplyPoint(inPoint, outPoint);
    plane->SetPoint1(outPoint3);

    inPoint[0] = 0;
    inPoint[1] = dims[1];
    textureToRAS->MultiplyPoint(inPoint, outPoint);
    plane->SetPoint2(outPoint3);

    this->SliceModelNode->EndModify(wasModified);

    this->UpdatePipeline();
    /// \tbd Ideally it should not be fired if the output polydata is not
    /// modified.
    plane->Modified();
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
void vtkMRMLSliceLogic::SetSliceNode(vtkMRMLSliceNode* newSliceNode)
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

  for (LayerListIterator iterator = this->Layers.begin();
       iterator != this->Layers.end();
       ++iterator)
  {
    vtkMRMLSliceLayerLogic* layer = *iterator;
    if (layer != nullptr)
    {
      layer->SetSliceNode(newSliceNode);
    }
  }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetSliceCompositeNode(vtkMRMLSliceCompositeNode* sliceCompositeNode)
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
vtkMRMLSliceLayerLogic* vtkMRMLSliceLogic::GetBackgroundLayer()
{
  return this->GetNthLayer(vtkMRMLSliceLogic::LayerBackground);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetBackgroundLayer(vtkMRMLSliceLayerLogic* backgroundLayer)
{
  this->SetNthLayer(vtkMRMLSliceLogic::LayerBackground, backgroundLayer);
}

//----------------------------------------------------------------------------
vtkMRMLSliceLayerLogic* vtkMRMLSliceLogic::GetForegroundLayer()
{
  return this->GetNthLayer(vtkMRMLSliceLogic::LayerForeground);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetForegroundLayer(vtkMRMLSliceLayerLogic* foregroundLayer)
{
  this->SetNthLayer(vtkMRMLSliceLogic::LayerForeground, foregroundLayer);
}

//----------------------------------------------------------------------------
vtkMRMLSliceLayerLogic* vtkMRMLSliceLogic::GetLabelLayer()
{
  return this->GetNthLayer(vtkMRMLSliceLogic::LayerLabel);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetLabelLayer(vtkMRMLSliceLayerLogic* labelLayer)
{
  this->SetNthLayer(vtkMRMLSliceLogic::LayerLabel, labelLayer);
}

//----------------------------------------------------------------------------
vtkMRMLSliceLayerLogic* vtkMRMLSliceLogic::GetNthLayer(int layerIndex)
{
  if (layerIndex < 0)
  {
    vtkErrorMacro(<< "GetNthLayer: Non-negative layer index is expected.");
    return nullptr;
  }
  int maxLayerIndex = static_cast<int>(this->Layers.size());
  if (layerIndex >= maxLayerIndex)
  {
    return nullptr;
  }
  return this->Layers.at(layerIndex);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetNthLayer(int layerIndex, vtkMRMLSliceLayerLogic* layer)
{
  // TODO: Simplify the whole set using a macro similar to vtkMRMLSetAndObserve
  if (layerIndex < 0)
  {
    vtkErrorMacro(<< "SetNthLayer: Non-negative layer index is expected.");
    return;
  }
  vtkMRMLSliceLayerLogic* currentLayer = this->GetNthLayer(layerIndex);
  if (currentLayer)
    {
    currentLayer->SetMRMLScene(0);
    }
  if (layerIndex >= static_cast<int>(this->Layers.size()))
    {
    this->Layers.resize(layerIndex + 1);
    }
  this->Layers.at(layerIndex) = layer;
  if (layer)
    {
    layer->SetMRMLScene(this->GetMRMLScene());

    layer->SetSliceNode(this->SliceNode);
    vtkEventBroker::GetInstance()->AddObservation(
      layer, vtkCommand::ModifiedEvent,
      this, this->GetMRMLLogicsCallbackCommand());
    }
  this->Modified();
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLSliceLogic::GetNthLayerImageDataConnection(int layerIndex)
{
  return this->GetNthLayer(layerIndex) ? this->GetNthLayer(layerIndex)->GetImageDataConnection() : nullptr;
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLSliceLogic::GetNthLayerImageDataConnectionUVW(int layerIndex)
{
  return this->GetNthLayer(layerIndex) ? this->GetNthLayer(layerIndex)->GetImageDataConnectionUVW() : nullptr;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::SetNthLayerVolumeNode(int layerIndex, vtkMRMLVolumeNode* volumeNode)
{
  if (!this->SliceCompositeNode)
  {
    vtkErrorMacro(<< "SetNthLayerVolumeNode: Slice composite node is NULL.");
    return;
  }
  if (layerIndex < 0)
  {
    vtkErrorMacro(<< "SetNthLayerVolumeNode: Non-negative layer index is expected.");
    return;
  }
  if (!this->GetNthLayer(layerIndex))
  {
    vtkErrorMacro(<< "SetNthLayerVolumeNode: Failed to get Nth layer logic (N=" << layerIndex << ").");
    return;
  }
  this->GetNthLayer(layerIndex)->SetVolumeNode(volumeNode);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic
::SetWindowLevel(int layer, double newWindow, double newLevel)
{
  vtkMRMLScalarVolumeNode* volumeNode =
    vtkMRMLScalarVolumeNode::SafeDownCast(this->GetNthLayerVolumeNode(layer));
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
::GetWindowLevelAndRange(int layer, double& window, double& level,
  double& rangeLow, double& rangeHigh, bool& autoWindowLevel)
{
  vtkMRMLScalarVolumeNode* volumeNode =
    vtkMRMLScalarVolumeNode::SafeDownCast(this->GetNthLayerVolumeNode(layer));
  vtkMRMLScalarVolumeDisplayNode* volumeDisplayNode =
    volumeNode ? volumeNode->GetScalarVolumeDisplayNode() : nullptr;
  vtkImageData* imageData = (volumeDisplayNode && volumeNode) ? volumeNode->GetImageData() : nullptr;
  if (imageData)
    {
    window = volumeDisplayNode->GetWindow();
    level = volumeDisplayNode->GetLevel();
    double range[2] = {0.0, 255.0};
    imageData->GetScalarRange(range);
    rangeLow = range[0];
    rangeHigh = range[1];
    autoWindowLevel = (volumeDisplayNode->GetAutoWindowLevel() != 0);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic
::SetBackgroundWindowLevel(double newWindow, double newLevel)
{
  SetWindowLevel(vtkMRMLSliceLogic::LayerBackground, newWindow, newLevel);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic
::SetForegroundWindowLevel(double newWindow, double newLevel)
{
  SetWindowLevel(vtkMRMLSliceLogic::LayerForeground, newWindow, newLevel);
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
  this->GetWindowLevelAndRange(vtkMRMLSliceLogic::LayerBackground, window, level, rangeLow, rangeHigh, autoWindowLevel);
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
  this->GetWindowLevelAndRange(vtkMRMLSliceLogic::LayerForeground, window, level, rangeLow, rangeHigh, autoWindowLevel);
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLSliceLogic::GetImageDataConnection()
{
  return this->ImageDataConnection;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceLogic::HasInputs()
{
  for (LayerListIterator iterator = this->Layers.begin();
       iterator != this->Layers.end();
       ++iterator)
  {
    vtkMRMLSliceLayerLogic* layer = *iterator;
    if (layer != nullptr && layer->GetImageDataConnection() != nullptr)
    {
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceLogic::HasUVWInputs()
{
  for (LayerListIterator iterator = this->Layers.begin();
       iterator != this->Layers.end();
       ++iterator)
  {
    vtkMRMLSliceLayerLogic* layer = *iterator;
    if (layer != nullptr && layer->GetImageDataConnectionUVW() != nullptr)
    {
      return true;
    }
  }
  return false;
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

  if (this->HasInputs())
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
bool vtkMRMLSliceLogic::UpdateBlendLayers(vtkImageBlend* blend, const std::deque<SliceLayerInfo> &layers, bool clipToBackgroundVolume)
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

  // Update blend mode: if clip to background is disabled, blending occurs over the entire extent
  // of all layers, not just within the background volume region.
  if (clipToBackgroundVolume)
  {
    blend->BlendAlphaOff();
  }
  else
  {
    blend->BlendAlphaOn();
  }

  bool modified = (blend->GetMTime() > oldBlendMTime);
  return modified;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceLogic::UpdateAddSubOperation(vtkImageMathematics* addSubMath, int compositing)
{
  if (compositing != vtkMRMLSliceCompositeNode::Add && compositing != vtkMRMLSliceCompositeNode::Subtract)
  {
    vtkErrorWithObjectMacro(nullptr, << "UpdateAddSubOperation: Unexpected compositing mode (" << compositing << "). "
                            << "Supported values are "
                            << "Add (" << vtkMRMLSliceCompositeNode::Add << ") or "
                            << "Subtract(" << vtkMRMLSliceCompositeNode::Subtract << ")");
    return false;
  }
  vtkMTimeType oldAddSubMathMTime = addSubMath->GetMTime();
  if (compositing == vtkMRMLSliceCompositeNode::Add)
  {
    addSubMath->SetOperationToAdd();
  }
  else
  {
    addSubMath->SetOperationToSubtract();
  }
  bool modified = (addSubMath->GetMTime() > oldAddSubMathMTime);
  return modified;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceLogic::UpdateFractions(vtkImageMathematics* fraction, double opacity)
{
  if (!fraction)
  {
    return false;
  }
  vtkMTimeType oldMTime = fraction->GetMTime();
  fraction->SetConstantK(opacity);
  bool modified = (fraction->GetMTime() > oldMTime);
  return modified;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceLogic::UpdateFractions(BlendPipeline* pipeline, const std::vector<vtkAlgorithmOutput*>& imagePorts, const std::vector<double>& opacities)
{
  if (!pipeline)
  {
    return false;
  }
  bool modified = false;
  // Start at 1 to skip background port
  for (int index = 1, stageIndex = 0; index < static_cast<int>(imagePorts.size()); ++index, ++stageIndex)
  {
    if (!imagePorts[index])
    {
      continue;
    }
    if (!pipeline->FractionMaths[stageIndex].GetPointer())
    {
      vtkErrorWithObjectMacro(nullptr, << "UpdateFractions: Failed to get Nth FractionMath Pipeline filter (N=" << stageIndex << ").");
      continue;
    }
    if (vtkMRMLSliceLogic::UpdateFractions(
          pipeline->FractionMaths[stageIndex].GetPointer(),
          opacities[index]))
    {
      modified = true;
    }
  }
  return modified;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdateReconstructionSlab(vtkMRMLSliceLogic* sliceLogic, vtkMRMLSliceLayerLogic* sliceLayerLogic)
{
  if (!sliceLogic || !sliceLayerLogic || !sliceLogic->GetSliceNode() || !sliceLayerLogic->GetSliceNode())
  {
    return;
  }

  vtkImageReslice* reslice = sliceLayerLogic->GetReslice();
  vtkMRMLSliceNode* sliceNode = sliceLayerLogic->GetSliceNode();

  double sliceSpacing;
  if (sliceNode->GetSliceSpacingMode() == vtkMRMLSliceNode::PrescribedSliceSpacingMode)
  {
    sliceSpacing = sliceNode->GetPrescribedSliceSpacing()[2];
  }
  else
  {
    sliceSpacing = sliceLogic->GetLowestVolumeSliceSpacing()[2];
  }

  int slabNumberOfSlices = 1;
  if (sliceNode->GetSlabReconstructionEnabled()
      && sliceSpacing > 0
      && sliceNode->GetSlabReconstructionThickness() > sliceSpacing
      )
  {
    slabNumberOfSlices = static_cast<int>(sliceNode->GetSlabReconstructionThickness() / sliceSpacing);
  }
  reslice->SetSlabNumberOfSlices(slabNumberOfSlices);

  reslice->SetSlabMode(sliceNode->GetSlabReconstructionType());

  double slabSliceSpacingFraction = sliceSpacing / sliceNode->GetSlabReconstructionOversamplingFactor();
  reslice->SetSlabSliceSpacingFraction(slabSliceSpacingFraction);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::UpdatePipeline()
{
  int modified = 0;

  if (this->SliceNode && this->SliceCompositeNode)
  {
    // Ensure all slice layers (background, foreground, additional layers) have an associated logic
    // and update them with the corresponding volume nodes.
    for (int layerIndex = 0; layerIndex < vtkMRMLSliceLogic::Layer_Last + this->SliceCompositeNode->GetNumberOfAdditionalLayers(); ++layerIndex)
    {
      vtkMRMLVolumeNode* layerNode = this->SliceCompositeNode->GetNthLayerVolume(layerIndex);
      vtkMRMLSliceLayerLogic* layerLogic = this->GetNthLayer(layerIndex);

      // Ensure additional layers have an associated logic.
      // Predefined layers are instantiated in ProcessMRMLLogicsEvents().
      if (!layerLogic && layerIndex >= vtkMRMLSliceLogic::Layer_Last)
      {
        this->SetNthLayer(layerIndex, vtkNew<vtkMRMLSliceLayerLogic>());
        layerLogic = this->GetNthLayer(layerIndex);
        layerLogic->SetMRMLScene(this->GetMRMLScene());
        layerLogic->SetVolumeNode(layerNode);
      }

      if (!layerLogic)
      {
        vtkErrorMacro(<< "UpdatePipeline: Failed to get Nth layer logic (N=" << layerIndex << ").");
        continue;
      }

      // Update the layer logic with the corresponding volume node if necessary
      if (layerLogic->GetVolumeNode() != layerNode )
      {
        this->SetNthLayerVolumeNode(layerIndex, layerNode);
        modified = 1;
      }
    }

    // Update slice extents in the layers if any modification was made
    if (modified)
    {
      this->SetSliceExtentsToSliceNode();
    }

    // Collect valid (non-null) image data connections and associated opacities for each layer (excluding label layer)
    std::vector<double> layerOpacities;
    std::vector<vtkAlgorithmOutput*> layerPorts;
    std::vector<double> layerUVWOpacities;
    std::vector<vtkAlgorithmOutput*> layerUVWPorts;
    for (int layerIndex = 0;
        layerIndex < vtkMRMLSliceLogic::Layer_Last + this->SliceCompositeNode->GetNumberOfAdditionalLayers();
        ++layerIndex)
    {
      if (layerIndex == vtkMRMLSliceLogic::LayerLabel)
      {
        continue; // Skip label layer as it is handled separately
      }
      if (this->GetNthLayerImageDataConnection(layerIndex))
      {
        layerOpacities.push_back(this->SliceCompositeNode->GetNthLayerOpacity(layerIndex));
        layerPorts.push_back(this->GetNthLayerImageDataConnection(layerIndex));
      }
      if (this->GetNthLayerImageDataConnectionUVW(layerIndex))
      {
        layerUVWOpacities.push_back(this->SliceCompositeNode->GetNthLayerOpacity(layerIndex));
        layerUVWPorts.push_back(this->GetNthLayerImageDataConnectionUVW(layerIndex));
      }
    }

    // Update pipeline stages
    if (this->Pipeline->UpdateStages(layerPorts))
    {
      modified = 1;
    }

    // Construct the blending pipeline
    std::deque<SliceLayerInfo> layers;
    this->Pipeline->AddLayers(
          layers,
          this->SliceCompositeNode->GetCompositing(),
          this->SliceCompositeNode->GetClipToBackgroundVolume(),
          // Layers
          layerPorts,
          layerOpacities,
          // Label
          this->GetNthLayerImageDataConnection(vtkMRMLSliceLogic::LayerLabel),
          this->SliceCompositeNode->GetNthLayerOpacity(vtkMRMLSliceLogic::LayerLabel)
          );

    // Update UVW pipeline stages
    if (this->PipelineUVW->UpdateStages(layerUVWPorts))
    {
      modified = 1;
    }

    // Construct the UVW blending pipeline
    std::deque<SliceLayerInfo> layersUVW;
    this->PipelineUVW->AddLayers(
          layersUVW,
          this->SliceCompositeNode->GetCompositing(),
          this->SliceCompositeNode->GetClipToBackgroundVolume(),
          // Layers
          layerUVWPorts,
          layerOpacities,
          // Label
          this->GetNthLayerImageDataConnectionUVW(vtkMRMLSliceLogic::LayerLabel),
          this->SliceCompositeNode->GetNthLayerOpacity(vtkMRMLSliceLogic::LayerLabel)
          );

    if (this->SliceCompositeNode->GetCompositing() == vtkMRMLSliceCompositeNode::Add
        || this->SliceCompositeNode->GetCompositing() == vtkMRMLSliceCompositeNode::Subtract)
    {
      // Update add/subtract operations in the pipeline
      if (vtkMRMLSliceLogic::UpdateAddSubOperation(this->Pipeline->AddSubMath.GetPointer(), this->SliceCompositeNode->GetCompositing()))
      {
        modified = 1;
      }
      if (vtkMRMLSliceLogic::UpdateAddSubOperation(this->PipelineUVW->AddSubMath.GetPointer(), this->SliceCompositeNode->GetCompositing()))
      {
        modified = 1;
      }
    }

    // Update opacity fractions for additional layers in add/subtract blending mode
    if (vtkMRMLSliceLogic::UpdateFractions(this->Pipeline, layerPorts, layerOpacities))
    {
      modified = 1;
    }
    if (vtkMRMLSliceLogic::UpdateFractions(this->PipelineUVW, layerUVWPorts, layerUVWOpacities))
    {
      modified = 1;
    }

    // Update alpha blending configuration for the layers
    if (vtkMRMLSliceLogic::UpdateBlendLayers(this->Pipeline->Blend.GetPointer(), layers, this->SliceCompositeNode->GetClipToBackgroundVolume()))
    {
      modified = 1;
    }
    if (vtkMRMLSliceLogic::UpdateBlendLayers(this->PipelineUVW->Blend.GetPointer(), layersUVW, this->SliceCompositeNode->GetClipToBackgroundVolume()))
    {
      modified = 1;
    }

    // Update models
    this->UpdateImageData();
    vtkMRMLDisplayNode* displayNode = this->SliceModelNode ? this->SliceModelNode->GetModelDisplayNode() : nullptr;
    if (displayNode)
    {
      displayNode->SetVisibility( this->SliceNode->GetSliceVisible() );
      displayNode->SetViewNodeIDs( this->SliceNode->GetThreeDViewIDs());

      // Manage texture interpolation based on input availability
      if ( (this->SliceNode->GetSliceResolutionMode() != vtkMRMLSliceNode::SliceResolutionMatch2DView &&
            !this->HasUVWInputs()) ||
          (this->SliceNode->GetSliceResolutionMode() == vtkMRMLSliceNode::SliceResolutionMatch2DView &&
           !this->HasInputs()) )
      {
        displayNode->SetTextureImageDataConnection(nullptr);
      }
      else if (displayNode->GetTextureImageDataConnection() != this->ExtractModelTexture->GetOutputPort())
      {
        displayNode->SetTextureImageDataConnection(this->ExtractModelTexture->GetOutputPort());
      }

      // Disable interpolation if label layer is present
      if (this->GetNthLayerImageDataConnection(vtkMRMLSliceLogic::LayerLabel) ||
          this->GetNthLayerImageDataConnectionUVW(vtkMRMLSliceLogic::LayerLabel))
        {
          displayNode->SetInterpolateTexture(0);
        }
        else
        {
          displayNode->SetInterpolateTexture(1);
        }
    }

    // Mark the pipeline as modified if any updates were performed
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

  if (this->GetBackgroundLayer())
  {
    os << indent << "BackgroundLayer:\n";
    this->GetBackgroundLayer()->PrintSelf(os, nextIndent);
  }
  else
  {
    os << indent << "BackgroundLayer: (none)\n";
  }

  if (this->GetForegroundLayer())
  {
    os << indent << "ForegroundLayer:\n";
    this->GetForegroundLayer()->PrintSelf(os, nextIndent);
  }
  else
  {
    os << indent << "ForegroundLayer: (none)\n";
  }

  if (this->GetLabelLayer())
  {
    os << indent << "LabelLayer:\n";
    this->GetLabelLayer()->PrintSelf(os, nextIndent);
  }
  else
  {
    os << indent << "LabelLayer: (none)\n";
  }

  os << indent << "AdditionalLayers:\n";
  if (this->SliceCompositeNode)
  {
    for (int additionalLayerIndex = 0; additionalLayerIndex < this->SliceCompositeNode->GetNumberOfAdditionalLayers(); ++additionalLayerIndex)
    {
      vtkMRMLSliceLayerLogic* layerLogic = this->GetNthLayer(vtkMRMLSliceLogic::Layer_Last + additionalLayerIndex);
      if (layerLogic)
      {
        os << nextIndent << "Layer " << additionalLayerIndex << ": \n";
        layerLogic->PrintSelf(os, nextIndent.GetNextIndent());
      }
      else
      {
        os << nextIndent << "Layer " << additionalLayerIndex << ": (none)\n";
      }
    }
  }

  if (this->Pipeline->Blend.GetPointer())
  {
    os << indent << "Blend:\n";
    this->Pipeline->Blend->PrintSelf(os, nextIndent);
  }
  else
  {
    os << indent << "Blend: (none)\n";
  }

  if (this->PipelineUVW->Blend.GetPointer())
  {
    os << indent << "BlendUVW:\n";
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
  if (!this->GetMRMLScene())
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
    vtkMRMLSliceDisplayNode* sliceDisplayNode = vtkMRMLSliceDisplayNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLSliceDisplayNode"));
    this->SliceModelDisplayNode = sliceDisplayNode;
    this->SliceModelDisplayNode->SetScene(this->GetMRMLScene());
    this->SliceModelDisplayNode->SetDisableModifiedEvent(1);

    //this->SliceModelDisplayNode->SetInputPolyData(this->SliceModelNode->GetOutputPolyData());
    this->SliceModelDisplayNode->SetVisibility(0);
    this->SliceModelDisplayNode->SetOpacity(1);
    this->SliceModelDisplayNode->SetColor(1,1,1);

    // Show intersecting slices in new slice views if this is currently enabled in the application.
    vtkMRMLApplicationLogic* appLogic = this->GetMRMLApplicationLogic();
    if (appLogic)
    {
      // Intersection
      sliceDisplayNode->SetIntersectingSlicesVisibility(appLogic->GetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesVisibility));
      sliceDisplayNode->SetIntersectingSlicesInteractive(appLogic->GetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesInteractive));
      sliceDisplayNode->SetIntersectingSlicesTranslationEnabled(appLogic->GetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesTranslation));
      sliceDisplayNode->SetIntersectingSlicesRotationEnabled(appLogic->GetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesRotation));
      // ThickSlab
      sliceDisplayNode->SetIntersectingThickSlabInteractive(appLogic->GetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesThickSlabInteractive));
    }

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
    std::string description;
    std::stringstream ssD;
    if (this->SliceNode && this->SliceNode->GetID() )
    {
      ssD << " SliceID " << this->SliceNode->GetID();
    }
    if (this->SliceCompositeNode && this->SliceCompositeNode->GetID() )
    {
      ssD << " CompositeID " << this->SliceCompositeNode->GetID();
    }

    std::getline(ssD, description);
    this->SliceModelNode->SetDescription(description.c_str());
  }
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLSliceLogic::GetLayerVolumeNode(int layer)
{
  vtkWarningMacro("Deprecated, please use GetNthLayerVolumeNode instead");
  return this->GetNthLayerVolumeNode(layer);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLSliceLogic::GetNthLayerVolumeNode(int layer)
{
  if (!this->SliceNode || !this->SliceCompositeNode)
  {
    return (nullptr);
  }
  if (layer < 0)
  {
    vtkErrorMacro(<< "GetNthLayerVolumeNode: Non-negative layer index is expected.");
    return nullptr;
  }
  int maxLayerIndex = vtkMRMLSliceLogic::Layer_Last + this->SliceCompositeNode->GetNumberOfAdditionalLayers();
  if (layer >= maxLayerIndex)
  {
    return nullptr;
  }
  const char* id = this->SliceCompositeNode->GetNthLayerVolumeID(layer);
  vtkMRMLScene* scene = this->GetMRMLScene();
  return scene ? vtkMRMLVolumeNode::SafeDownCast(
    scene->GetNodeByID( id )) : nullptr;
}

//----------------------------------------------------------------------------
// Get the size of the volume, transformed to RAS space
void vtkMRMLSliceLogic::GetVolumeRASBox(vtkMRMLVolumeNode* volumeNode, double rasDimensions[3], double rasCenter[3])
{
  rasCenter[0] = rasDimensions[0] = 0.0;
  rasCenter[1] = rasDimensions[1] = 0.0;
  rasCenter[2] = rasDimensions[2] = 0.0;

  vtkImageData* volumeImage;
  if ( !volumeNode || ! (volumeImage = volumeNode->GetImageData()) )
  {
    return;
  }

  double bounds[6];
  volumeNode->GetRASBounds(bounds);

  for (int axis = 0; axis < 3; axis++)
  {
    rasDimensions[axis] = bounds[2 * axis + 1] - bounds[2 * axis];
    rasCenter[axis] = 0.5 * (bounds[2 * axis + 1] + bounds[2 * axis]);
  }
}

//----------------------------------------------------------------------------
// Get the size of the volume, transformed to RAS space
void vtkMRMLSliceLogic::GetVolumeSliceDimensions(vtkMRMLVolumeNode* volumeNode,
  double sliceDimensions[3], double sliceCenter[3])
{
  sliceCenter[0] = sliceDimensions[0] = 0.0;
  sliceCenter[1] = sliceDimensions[1] = 0.0;
  sliceCenter[2] = sliceDimensions[2] = 0.0;

  double sliceBounds[6];
  this->GetVolumeSliceBounds(volumeNode, sliceBounds);

  for (int axis = 0; axis < 3; axis++)
  {
    sliceDimensions[axis] = sliceBounds[2 * axis + 1] - sliceBounds[2 * axis];
    sliceCenter[axis] = 0.5*(sliceBounds[2 * axis + 1] + sliceBounds[2 * axis]);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::GetVolumeSliceBounds(vtkMRMLVolumeNode* volumeNode,
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
double* vtkMRMLSliceLogic::GetVolumeSliceSpacing(vtkMRMLVolumeNode* volumeNode)
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
    double* pspacing = this->SliceNode->GetPrescribedSliceSpacing();
    this->SliceSpacing[0] = pspacing[0];
    this->SliceSpacing[1] = pspacing[1];
    this->SliceSpacing[2] = pspacing[2];
    return (pspacing);
  }

  // Compute slice spacing from the volume axis closest matching the slice axis, projected to the slice axis.

  vtkNew<vtkMatrix4x4> ijkToWorld;
  volumeNode->GetIJKToRASMatrix(ijkToWorld);

  // Apply transform to the volume axes, if the volume is transformed with a linear transform
  vtkMRMLTransformNode* transformNode = volumeNode->GetParentTransformNode();
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
void vtkMRMLSliceLogic::FitSliceToVolume(vtkMRMLVolumeNode* volumeNode, int width, int height)
{
  if (!volumeNode)
  {
    return;
  }
  vtkNew<vtkCollection> volumeNodes;
  volumeNodes->AddItem(volumeNode);
  this->FitSliceToVolumes(volumeNodes.GetPointer(), width, height);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::FitSliceToVolumes(vtkCollection* volumeNodes, int width, int height)
{
  if (!this->SliceNode)
  {
    return;
  }

  if (!volumeNodes || volumeNodes->GetNumberOfItems() == 0)
  {
    return;
  }

  vtkBoundingBox volumeBounds_Slice;
  vtkBoundingBox volumeBounds_RAS;
  double sliceSpacingZ = 0.;

  vtkSmartPointer<vtkCollectionIterator> iterator = vtkSmartPointer<vtkCollectionIterator>::New();
  iterator->SetCollection(volumeNodes);

  bool firstVolumeFound = false;
  for (iterator->InitTraversal(); !iterator->IsDoneWithTraversal(); iterator->GoToNextItem())
  {
    vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(iterator->GetCurrentObject());
    if (!volumeNode || !volumeNode->GetImageData())
    {
      continue;
    }

    double sliceBounds[6] = {0.0, -1.0, 0.0, -1.0, 0.0, -1.0};
    this->GetVolumeSliceBounds(volumeNode, sliceBounds);
    volumeBounds_Slice.AddBounds(sliceBounds);

    double rasBounds[6] = {0.0, -1.0, 0.0, -1.0, 0.0, -1.0};
    volumeNode->GetRASBounds(rasBounds);
    volumeBounds_RAS.AddBounds(rasBounds);

    // Set sliceSpacingZ for the first volume found
    if (!firstVolumeFound)
    {
      sliceSpacingZ = this->GetVolumeSliceSpacing(volumeNode)[2];
      firstVolumeFound = true;
    }
  }

  // Calculate the slice dimensions for all volumes
  double sliceDimensions[3] = { 0., 0., 0. };
  volumeBounds_Slice.GetLengths(sliceDimensions);

  double fitX, fitY, fitZ, displayX, displayY;
  displayX = fitX = fabs(sliceDimensions[0]);
  displayY = fitY = fabs(sliceDimensions[1]);
  fitZ = sliceSpacingZ * this->SliceNode->GetDimensions()[2];

  // fit fov to min dimension of window
  double pixelSize;
  if (height > width)
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
  if (displayX > fitX)
  {
    fitY = fitY / (fitX / (displayX * 1.0));
    fitX = displayX;
  }
  if (displayY > fitY)
  {
    fitX = fitX / (fitY / (displayY * 1.0));
    fitY = displayY;
  }

  this->SliceNode->SetFieldOfView(fitX, fitY, fitZ);

  //
  // set the origin to be the center of the volume in RAS
  //
  vtkNew<vtkMatrix4x4> sliceToRAS;
  sliceToRAS->DeepCopy(this->SliceNode->GetSliceToRAS());
  double sliceCenter_RAS[3] = { 0.0, 0.0, 0.0};
  volumeBounds_RAS.GetCenter(sliceCenter_RAS);
  sliceToRAS->SetElement(0, 3, sliceCenter_RAS[0]);
  sliceToRAS->SetElement(1, 3, sliceCenter_RAS[1]);
  sliceToRAS->SetElement(2, 3, sliceCenter_RAS[2]);
  this->SliceNode->GetSliceToRAS()->DeepCopy(sliceToRAS);
  this->SliceNode->SetSliceOrigin(0,0,0);
  //sliceNode->SetSliceOffset(offset);

  //TODO Fit UVW space
  this->SnapSliceOffsetToIJK();
  this->SliceNode->UpdateMatrices();
}

//----------------------------------------------------------------------------
// Get the size of the volume, transformed to RAS space
void vtkMRMLSliceLogic::GetBackgroundRASBox(double rasDimensions[3], double rasCenter[3])
{
  vtkMRMLVolumeNode* backgroundNode = this->GetNthLayerVolumeNode(vtkMRMLSliceLogic::LayerBackground);
  this->GetVolumeRASBox(backgroundNode, rasDimensions, rasCenter);
}

//----------------------------------------------------------------------------
// Get the size of the volume, transformed to RAS space
void vtkMRMLSliceLogic::GetBackgroundSliceDimensions(double sliceDimensions[3], double sliceCenter[3])
{
  vtkMRMLVolumeNode* backgroundNode = this->GetNthLayerVolumeNode(vtkMRMLSliceLogic::LayerBackground);
  this->GetVolumeSliceDimensions(backgroundNode, sliceDimensions, sliceCenter);
}

//----------------------------------------------------------------------------
// Get the spacing of the volume, transformed to slice space
double* vtkMRMLSliceLogic::GetBackgroundSliceSpacing()
{
  vtkMRMLVolumeNode* backgroundNode = this->GetNthLayerVolumeNode(vtkMRMLSliceLogic::LayerBackground);
  return (this->GetVolumeSliceSpacing(backgroundNode));
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::GetBackgroundSliceBounds(double sliceBounds[6])
{
  vtkMRMLVolumeNode* backgroundNode = this->GetNthLayerVolumeNode(vtkMRMLSliceLogic::LayerBackground);
  this->GetVolumeSliceBounds(backgroundNode, sliceBounds);
}

//----------------------------------------------------------------------------
// adjust the node's field of view to match the extent of the first selected volume (background, foregorund, labelmap)
void vtkMRMLSliceLogic::FitSliceToFirst(int width, int height)
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

  vtkMRMLVolumeNode* volumeNode = vtkMRMLSliceLogic::GetFirstVolumeNode();
  if (volumeNode)
  {
    this->FitSliceToVolume(volumeNode, width, height);
  }
}

//----------------------------------------------------------------------------
// adjust the node's field of view to match the extent of current background volume
void vtkMRMLSliceLogic::FitSliceToBackground(int width, int height)
{
  if (!this->SliceCompositeNode)
  {
    return;
  }
  if (this->SliceCompositeNode->GetClipToBackgroundVolume())
  {
    this->FitSliceToFirst(width, height);
  }
  else
  {
    this->FitSliceToAll(width, height);
  }
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

  vtkNew<vtkCollection> volumeNodes;
  for (int layer = 0;
       layer < vtkMRMLSliceLogic::Layer_Last + this->SliceCompositeNode->GetNumberOfAdditionalLayers();
       layer++)
  {
    vtkMRMLVolumeNode* volumeNode = this->GetNthLayerVolumeNode(layer);
    if (volumeNode)
    {
      volumeNodes->AddItem(volumeNode);
    }
  }

  this->FitSliceToVolumes(volumeNodes, width, height);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::FitFOVToBackground(double fov)
{
  // get backgroundNode  and imagedata
  vtkMRMLScalarVolumeNode* backgroundNode =
    vtkMRMLScalarVolumeNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID(
        this->SliceCompositeNode->GetBackgroundVolumeID() ));
  vtkImageData* backgroundImage =
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

  int oldDimensions[3];
  this->SliceNode->GetDimensions(oldDimensions);
  double oldFOV[3];
  this->SliceNode->GetFieldOfView(oldFOV);
  double newFOV[3];
  newFOV[0] = oldFOV[0];
  newFOV[1] = oldFOV[1];
  newFOV[2] = this->SliceSpacing[2] * oldDimensions[2];
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
double* vtkMRMLSliceLogic::GetLowestVolumeSliceSpacing()
{
  // Note that "lowest" refers to the lowest (first) volume layer, not the one with the lowest slice spacing
  vtkMRMLVolumeNode* volumeNode = vtkMRMLSliceLogic::GetFirstVolumeNode();
  if (volumeNode)
  {
    return this->GetVolumeSliceSpacing(volumeNode);
  }
  return (this->SliceSpacing);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::GetLowestVolumeSliceBounds(double sliceBounds[6], bool useVoxelCenter/*=false*/)
{
  vtkMRMLVolumeNode* volumeNode = vtkMRMLSliceLogic::GetFirstVolumeNode();
  if (volumeNode)
  {
    return this->GetVolumeSliceBounds(volumeNode, sliceBounds, useVoxelCenter);
  }
  // return the default values
  return this->GetVolumeSliceBounds(nullptr, sliceBounds, useVoxelCenter);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLogic::GetSliceBounds(double sliceBounds[6])
{
  if (!this->SliceCompositeNode)
  {
    return;
  }
  vtkBoundingBox sliceBoundingBox;
  for (int layer = 0;
       layer < vtkMRMLSliceLogic::Layer_Last + this->SliceCompositeNode->GetNumberOfAdditionalLayers();
       layer++)
  {
    vtkMRMLVolumeNode* volumeNode = this->GetNthLayerVolumeNode(layer);
    if (volumeNode)
    {
      double bounds[6];
      this->GetVolumeSliceBounds(volumeNode, bounds);
      sliceBoundingBox.AddBounds(bounds);
    }
  }

  if (sliceBoundingBox.IsValid())
  {
    sliceBoundingBox.GetBounds(sliceBounds);
  }
  else
  {
    // no volumes are shown, set some valid bounds to avoid singularities
    for (int axis = 0; axis < 3; axis++)
    {
      sliceBounds[2 * axis] = -100;
      sliceBounds[2 * axis + 1] = 100;
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
  }

  this->SliceCompositeNode->SetInteractionFlags(0);
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
    double* spacing = this->GetLowestVolumeSliceSpacing();
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
    double* spacing = this->GetLowestVolumeSliceSpacing();
    double minSpacing = spacing[0];
    minSpacing = minSpacing < spacing[1] ? minSpacing:spacing[1];
    minSpacing = minSpacing < spacing[2] ? minSpacing:spacing[2];

    double fov[3];
    int dimensions[]={0,0,1};
    this->SliceNode->GetFieldOfView(fov);
    for (int axis = 0; axis < 2; axis++)
    {
       dimensions[axis] = ceil(fov[axis]/minSpacing + 0.5);
    }
    this->SliceNode->SetUVWExtentsAndDimensions(fov, dimensions);
  }
  else if (this->SliceNode->GetSliceResolutionMode() == vtkMRMLSliceNode::SliceFOVMatchVolumesSpacingMatch2DView)
  {
    // compute RAS spacing in 2D view
    vtkMatrix4x4* xyToRAS = this->SliceNode->GetXYToRAS();
    int  dims[3];

    //
    double inPoint[4]={0,0,0,1};
    double outPoint0[4];
    double outPoint1[4];
    double outPoint2[4];

    // set the z position to be the active slice (from the lightbox)
    inPoint[2] = this->SliceNode->GetActiveSlice();

    // transform XYZ = (0,0,0)
    xyToRAS->MultiplyPoint(inPoint, outPoint0);

    // transform XYZ = (1,0,0)
    inPoint[0] = 1;
    xyToRAS->MultiplyPoint(inPoint, outPoint1);

    // transform XYZ = (0,1,0)
    inPoint[0] = 0;
    inPoint[1] = 1;
    xyToRAS->MultiplyPoint(inPoint, outPoint2);

    double xSpacing = sqrt(vtkMath::Distance2BetweenPoints(outPoint0, outPoint1));
    double ySpacing = sqrt(vtkMath::Distance2BetweenPoints(outPoint0, outPoint2));

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
  }

  this->SliceNode->SetInteractionFlags(0);
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
std::vector<vtkMRMLDisplayNode*> vtkMRMLSliceLogic::GetPolyDataDisplayNodes()
{
  std::vector<vtkMRMLDisplayNode*> nodes;
  std::vector<vtkMRMLSliceLayerLogic*> layerLogics;
  layerLogics.push_back(this->GetBackgroundLayer());
  layerLogics.push_back(this->GetForegroundLayer());
  if (this->SliceCompositeNode)
  {
    for (int additionalLayerIndex = 0;
        additionalLayerIndex < this->SliceCompositeNode->GetNumberOfAdditionalLayers();
        ++additionalLayerIndex)
    {
      layerLogics.push_back(this->GetNthLayer(vtkMRMLSliceLogic::Layer_Last + this->SliceCompositeNode->GetNumberOfAdditionalLayers()));
    }
  }
  for (unsigned int layerIndex = 0; layerIndex < layerLogics.size(); layerIndex++)
  {
    vtkMRMLSliceLayerLogic* layerLogic = layerLogics[layerIndex];
    if (layerLogic && layerLogic->GetVolumeNode())
    {
      vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast (layerLogic->GetVolumeNode());
      vtkMRMLGlyphableVolumeDisplayNode* displayNode = vtkMRMLGlyphableVolumeDisplayNode::SafeDownCast( layerLogic->GetVolumeNode()->GetDisplayNode() );
      if (displayNode)
      {
        std::vector<vtkMRMLGlyphableVolumeSliceDisplayNode*> dnodes  = displayNode->GetSliceGlyphDisplayNodes(volumeNode);
        for (unsigned int n = 0; n<dnodes.size(); n++)
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
int vtkMRMLSliceLogic::GetSliceIndexFromOffset(double sliceOffset, vtkMRMLVolumeNode* volumeNode)
{
  if ( !volumeNode )
  {
    return SLICE_INDEX_NO_VOLUME;
  }
  vtkImageData* volumeImage = nullptr;
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
  vtkMRMLTransformNode* transformNode = volumeNode->GetParentTransformNode();
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
  int axisIndex = 0;
  double volumeSpacing = 1.0; // spacing along axisIndex
  for (axisIndex=0; axisIndex<3; axisIndex++)
  {
    axisDirection_RAS[0]=ijkToRAS->GetElement(0,axisIndex);
    axisDirection_RAS[1]=ijkToRAS->GetElement(1,axisIndex);
    axisDirection_RAS[2]=ijkToRAS->GetElement(2,axisIndex);
    volumeSpacing=vtkMath::Norm(axisDirection_RAS); // spacing along axisIndex
    vtkMath::Normalize(sliceNormal_RAS);
    vtkMath::Normalize(axisDirection_RAS);
    double dotProd = vtkMath::Dot(sliceNormal_RAS, axisDirection_RAS);
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
    double axisMisalignmentDegrees = acos(dotProd)*180.0/vtkMath::Pi();
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
  double volumeOriginOffset = originPos_Slice[2];
  double sliceShift = sliceOffset-volumeOriginOffset;
  double normalizedSliceShift = sliceShift/volumeSpacing;
  int sliceIndex = vtkMath::Round(normalizedSliceShift)+1; // +0.5 because the slice plane is displayed in the center of the slice

  // Check if slice index is within the volume
  int sliceCount = volumeImage->GetDimensions()[axisIndex];
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
  vtkMRMLVolumeNode* volumeNode = vtkMRMLSliceLogic::GetFirstVolumeNode();
  if (volumeNode)
  {
    int sliceIndex = this->GetSliceIndexFromOffset(sliceOffset, volumeNode);
    // return the result for the first available layer
    return sliceIndex;
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
bool vtkMRMLSliceLogic::IsSliceModelNode(vtkMRMLNode* mrmlNode)
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
bool vtkMRMLSliceLogic::IsSliceModelDisplayNode(vtkMRMLDisplayNode* mrmlDisplayNode)
{
  if (vtkMRMLSliceDisplayNode::SafeDownCast(mrmlDisplayNode))
  {
    return true;
  }
  if (mrmlDisplayNode != nullptr &&
      mrmlDisplayNode->IsA("vtkMRMLModelDisplayNode"))
  {
    const char* attrib = mrmlDisplayNode->GetAttribute("SliceLogic.IsSliceModelDisplayNode");
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
  vtkMRMLVolumeNode* volumeNode = vtkMRMLSliceLogic::GetFirstVolumeNode();
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
  vtkMRMLSliceNode* sliceNode = this->GetSliceNode();
  if (!sliceNode)
  {
    return vtkMRMLSliceLogic::LayerNone;
  }
  vtkMRMLSliceCompositeNode* sliceCompositeNode = this->GetSliceCompositeNode();
  if (!sliceCompositeNode)
  {
    return vtkMRMLSliceLogic::LayerNone;
  }

  if (!foregroundVolumeEditable && !backgroundVolumeEditable)
  {
    // window/level editing is disabled on both volumes
    return vtkMRMLSliceLogic::LayerNone;
  }
  // By default adjust background volume, if available
  bool adjustForeground = !backgroundVolumeEditable || !sliceCompositeNode->GetBackgroundVolumeID();

  // If both foreground and background volumes are visible then choose adjustment of
  // foreground volume, if foreground volume is visible in current mouse position
  if (sliceCompositeNode->GetBackgroundVolumeID() && sliceCompositeNode->GetForegroundVolumeID())
  {
    if (foregroundVolumeEditable && backgroundVolumeEditable)
    {
    adjustForeground = (sliceCompositeNode->GetForegroundOpacity() >= 0.01)
      && this->IsEventInsideVolume(true, worldPos)   // inside background (used as mask for displaying foreground)
      && this->vtkMRMLSliceLogic::IsEventInsideVolume(false, worldPos); // inside foreground
    }
  }

  return (adjustForeground ? vtkMRMLSliceLogic::LayerForeground : vtkMRMLSliceLogic::LayerBackground);
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceLogic::IsEventInsideVolume(bool background, double worldPos[3])
{
  vtkMRMLSliceNode* sliceNode = this->GetSliceNode();
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
  for (int axis = 0; axis < 3; axis++)
  {
    // In VTK, the voxel coordinate refers to the center of the voxel and so the image bounds
    // go beyond the position of the first and last voxels by half voxel. Therefore include 0.5 shift.
    if (ijkPos[axis] < volumeExtent[axis * 2] - 0.5 || ijkPos[axis] > volumeExtent[axis * 2 + 1] + 0.5)
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


//----------------------------------------------------------------------------
bool vtkMRMLSliceLogic::GetSliceOffsetRangeResolution(double range[2], double& resolution)
{
  // Calculate the number of slices in the current range.
  // Extent is between the farthest voxel centers (not voxel sides).
  double sliceBounds[6] = {0, -1, 0, -1, 0, -1};
  this->GetLowestVolumeSliceBounds(sliceBounds, true);

  const double* sliceSpacing = this->GetLowestVolumeSliceSpacing();
  if (!sliceSpacing)
  {
    range[0] = -1.0;
    range[1] = 1.0;
    resolution = 1.0;
    return false;
  }

  // Set the scale increments to match the z spacing (rotated into slice space)
  resolution = sliceSpacing ? sliceSpacing[2] : 1.0;

  bool singleSlice = ((sliceBounds[5] - sliceBounds[4]) < resolution);
  if (singleSlice)
  {
    // add one blank slice before and after the current slice to make the slider appear in the center when
    // we are centered on the slice
    double centerPos = (sliceBounds[4] + sliceBounds[5]) / 2.0;
    range[0] = centerPos - resolution;
    range[1] = centerPos + resolution;
  }
  else
  {
    // there are at least two slices in the range
    range[0] = sliceBounds[4];
    range[1] = sliceBounds[5];
  }

  return true;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLSliceLogic::GetFirstVolumeNode()
{
  if (!this->SliceCompositeNode)
  {
    return nullptr;
  }
  for (int layer = 0;
       layer < vtkMRMLSliceLogic::Layer_Last + this->SliceCompositeNode->GetNumberOfAdditionalLayers();
       layer++)
  {
    vtkMRMLVolumeNode* volumeNode = this->GetNthLayerVolumeNode(layer);
    if (volumeNode)
    {
      return volumeNode;
    }
  }
  return nullptr;
}

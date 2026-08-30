#include "vtkMRMLLayerDisplayableManager.h"

// Layer DM includes
#include "vtkMRMLLayerDMPipelineFactory.h"
#include "vtkMRMLLayerDMPipelineManager.h"

// Slicer includes
#include "vtkMRMLAbstractViewNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceViewDisplayableManagerFactory.h"
#include "vtkMRMLThreeDViewDisplayableManagerFactory.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

// STD includes
#include <cstring>

vtkStandardNewMacro(vtkMRMLLayerDisplayableManager);

vtkMRMLLayerDisplayableManager::vtkMRMLLayerDisplayableManager()
  : PipelineManager(nullptr)
{
}

bool vtkMRMLLayerDisplayableManager::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)
{
  if (!this->PipelineManager)
  {
    return false;
  }

  return this->PipelineManager->CanProcessInteractionEvent(eventData, distance2);
}

bool vtkMRMLLayerDisplayableManager::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  if (!this->PipelineManager)
  {
    return false;
  }

  return this->PipelineManager->ProcessInteractionEvent(eventData);
}

void vtkMRMLLayerDisplayableManager::RegisterInDefaultViews()
{
  vtkMRMLLayerDisplayableManager::RegisterInFactory(vtkMRMLSliceViewDisplayableManagerFactory::GetInstance());
  vtkMRMLLayerDisplayableManager::RegisterInFactory(vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance());
}

void vtkMRMLLayerDisplayableManager::RegisterInFactory(vtkMRMLDisplayableManagerFactory* factory)
{
  if (!factory || vtkMRMLLayerDisplayableManager::IsRegisteredInFactory(factory))
  {
    return;
  }

  const vtkNew<vtkMRMLLayerDisplayableManager> dm;
  factory->RegisterDisplayableManager(dm->GetClassName());
}

bool vtkMRMLLayerDisplayableManager::IsRegisteredInFactory(vtkMRMLDisplayableManagerFactory* factory)
{
  if (!factory)
  {
    return false;
  }

  const vtkNew<vtkMRMLLayerDisplayableManager> dm;
  return factory->IsDisplayableManagerRegistered(dm->GetClassName());
}

vtkSmartPointer<vtkMRMLLayerDMPipeline> vtkMRMLLayerDisplayableManager::GetNodePipeline(vtkMRMLNode* node) const
{
  if (!this->PipelineManager)
  {
    return nullptr;
  }
  return this->PipelineManager->GetNodePipeline(node);
}

void vtkMRMLLayerDisplayableManager::OnMRMLSceneStartBatchProcess()
{
  if (!this->PipelineManager)
  {
    return;
  }
  this->PipelineManager->BlockRequestRender(true);
}

void vtkMRMLLayerDisplayableManager::OnMRMLSceneEndBatchProcess()
{
  if (!this->PipelineManager)
  {
    return;
  }
  this->PipelineManager->BlockRequestRender(false);
  this->PipelineManager->RequestRender();
}

void vtkMRMLLayerDisplayableManager::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!this->PipelineManager)
  {
    return;
  }
  this->PipelineManager->AddNode(node);
}

void vtkMRMLLayerDisplayableManager::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (!this->PipelineManager)
  {
    return;
  }
  this->PipelineManager->RemoveNode(node);
}

void vtkMRMLLayerDisplayableManager::UnobserveMRMLScene()
{
  if (!this->PipelineManager)
  {
    return;
  }
  this->PipelineManager->ClearDisplayableNodes();
}

void vtkMRMLLayerDisplayableManager::UpdateFromMRML()
{
  this->SetUpdateFromMRMLRequested(false);

  if (!this->PipelineManager)
  {
    return;
  }
  this->PipelineManager->SetScene(this->GetMRMLScene());
  this->PipelineManager->UpdateFromScene();
}

void vtkMRMLLayerDisplayableManager::OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller)
{
  auto viewNode = vtkMRMLAbstractViewNode::SafeDownCast(caller);
  if (!viewNode || !this->PipelineManager)
  {
    return;
  }

  this->PipelineManager->SetViewNode(viewNode);
}

int vtkMRMLLayerDisplayableManager::GetMouseCursor()
{
  if (!this->PipelineManager)
  {
    return vtkMRMLAbstractDisplayableManager::GetMouseCursor();
  }
  return this->PipelineManager->GetMouseCursor();
}

void vtkMRMLLayerDisplayableManager::Create()
{
  vtkRenderer* renderer = this->GetRenderer();
  if (!renderer || !renderer->GetRenderWindow())
  {
    vtkErrorMacro("vtkMRMLLayerDisplayableManager::Create() failed: renderer is invalid");
    return;
  }

  if (!this->PipelineManager)
  {
    this->PipelineManager = vtkSmartPointer<vtkMRMLLayerDMPipelineManager>::New();
  }

  this->PipelineManager->SetRenderWindow(renderer->GetRenderWindow());
  this->PipelineManager->SetRenderer(renderer);
  this->PipelineManager->SetFactory(vtkMRMLLayerDMPipelineFactory::GetInstance());
  this->PipelineManager->SetScene(this->GetMRMLScene());
  this->PipelineManager->SetViewNode(vtkMRMLAbstractViewNode::SafeDownCast(this->GetMRMLDisplayableNode()));
  this->PipelineManager->SetRequestRender([this] { this->RequestRender(); });

  // Make sure the DM is up to date with the current scene state
  this->UpdateFromMRML();
}

void vtkMRMLLayerDisplayableManager::SetRenderer(vtkRenderer* newRenderer)
{
  Superclass::SetRenderer(newRenderer);
  if (!this->PipelineManager)
  {
    return;
  }

  this->PipelineManager->SetRenderWindow(newRenderer ? newRenderer->GetRenderWindow() : nullptr);
  this->PipelineManager->SetRenderer(newRenderer);
}

void vtkMRMLLayerDisplayableManager::SetHasFocus(bool hasFocus, vtkMRMLInteractionEventData* eventData)
{
  Superclass ::SetHasFocus(hasFocus, eventData);
  if (this->PipelineManager && !hasFocus)
  {
    this->PipelineManager->LoseFocus(eventData);
  }
}

vtkSmartPointer<vtkImageData> vtkMRMLLayerDisplayableManager::RenderWindowBufferToImage(vtkRenderWindow* window)
{
  auto imageData = vtkSmartPointer<vtkImageData>::New();
  RenderWindowBufferToImage(window, imageData);
  return imageData;
}

void vtkMRMLLayerDisplayableManager::RenderWindowBufferToImage(vtkRenderWindow* window, const vtkSmartPointer<vtkImageData>& imageData)
{
  if (!window || !imageData)
  {
    return;
  }

  // Set image bounds to full RW bounds
  const auto size = window->GetSize();
  int imageBounds[4] = { 0, size[0] - 1, 0, size[1] - 1 };

  // Read pixel data from the back buffer
  const auto pixels = window->GetPixelData(imageBounds[0], imageBounds[2], imageBounds[1], imageBounds[3], 0);

  if (!pixels)
  {
    return;
  }

  // Configure imageData extent and allocate scalars for RGB
  imageData->SetExtent(imageBounds[0], imageBounds[1], imageBounds[2], imageBounds[3], 0, 0);
  imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 3);

  // Copy pixel buffer into vtkImageData
  const auto dest = static_cast<unsigned char*>(imageData->GetScalarPointer());
  const auto numPixels = static_cast<size_t>(size[0]) * size[1];
  std::memcpy(dest, pixels, numPixels * 3 * sizeof(unsigned char));

  // Free the pixel buffer allocated by VTK
  delete[] pixels;
}

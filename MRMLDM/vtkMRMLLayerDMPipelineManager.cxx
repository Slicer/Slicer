#include "vtkMRMLLayerDMPipelineManager.h"

// Layer DM includes
#include "vtkMRMLLayerDMCameraSynchronizer.h"
#include "vtkMRMLLayerDMInteractionLogic.h"
#include "vtkMRMLLayerDMLayerManager.h"
#include "vtkMRMLLayerDMPipelineFactory.h"
#include "vtkMRMLLayerDMPipelineI.h"
#include "vtkObjectEventObserver.h"

// Slicer includes
#include "vtkMRMLAbstractViewNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(vtkMRMLLayerDMPipelineManager);

bool vtkMRMLLayerDMPipelineManager::CreatePipelineForNode(vtkMRMLNode* displayNode)
{
  // Early return if manager is not yet created
  if (!this->m_factory || !this->m_viewNode)
  {
    return false;
  }

  auto pipeline = this->m_factory->CreatePipeline(this->m_viewNode, displayNode);
  if (!pipeline)
  {
    return false;
  }

  auto wasBlocked = pipeline->BlockResetDisplay(true);
  pipeline->SetPipelineManager(this);
  pipeline->SetScene(this->m_scene);
  pipeline->SetViewNode(this->m_viewNode);
  pipeline->SetDisplayNode(displayNode);
  this->m_pipelineMap[displayNode] = pipeline;
  this->m_layerManager->AddPipeline(pipeline);
  this->m_interactionLogic->AddPipeline(pipeline);
  pipeline->BlockResetDisplay(wasBlocked);
  this->UpdatePipeline(pipeline);
  this->InvokeEvent(vtkCommand::ModifiedEvent);
  return true;
}

void vtkMRMLLayerDMPipelineManager::ClearDisplayableNodes()
{
  this->m_pipelineMap.clear();
}

bool vtkMRMLLayerDMPipelineManager::AddNode(vtkMRMLNode* node)
{
  if (auto pipeline = this->GetNodePipeline(node))
  {
    return false;
  }

  return this->CreatePipelineForNode(node);
}

void vtkMRMLLayerDMPipelineManager::UpdateAllPipelines() const
{
  for (const auto& pipeline : m_pipelineMap)
  {
    this->UpdatePipeline(pipeline.second);
  }
}

bool vtkMRMLLayerDMPipelineManager::RemovePipeline(vtkMRMLNode* displayNode)
{
  auto pipeline = this->GetNodePipeline(displayNode);
  if (!pipeline)
  {
    return false;
  }

  this->m_layerManager->RemovePipeline(pipeline);
  this->m_interactionLogic->RemovePipeline(pipeline);
  this->m_pipelineMap.erase(displayNode);
  this->InvokeEvent(vtkCommand::ModifiedEvent);
  return true;
}

void vtkMRMLLayerDMPipelineManager::SetRenderWindow(vtkRenderWindow* renderWindow) const
{
  this->m_layerManager->SetRenderWindow(renderWindow);
}

void vtkMRMLLayerDMPipelineManager::SetViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  if (this->m_viewNode == viewNode)
  {
    return;
  }

  this->m_viewNode = viewNode;
  this->m_cameraSync->SetViewNode(viewNode);
  this->m_interactionLogic->SetViewNode(viewNode);
  this->UpdateAllPipelines();
}

void vtkMRMLLayerDMPipelineManager::SetFactory(const vtkSmartPointer<vtkMRMLLayerDMPipelineFactory>& factory)
{
  if (this->m_factory == factory)
  {
    return;
  }

  this->m_eventObs->UpdateObserver(this->m_factory, factory);
  this->m_factory = factory;
  this->UpdateFromScene();
}

int vtkMRMLLayerDMPipelineManager::GetMouseCursor() const
{
  auto lastFocused = this->m_interactionLogic->GetLastFocusedPipeline();
  return lastFocused ? lastFocused->GetMouseCursor() : VTK_CURSOR_DEFAULT;
}

bool vtkMRMLLayerDMPipelineManager::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2) const
{
  return this->m_interactionLogic->CanProcessInteractionEvent(eventData, distance2);
}

void vtkMRMLLayerDMPipelineManager::LoseFocus(vtkMRMLInteractionEventData* eventData) const
{
  this->m_interactionLogic->LoseFocus(eventData);
}

void vtkMRMLLayerDMPipelineManager::LoseFocus() const
{
  this->m_interactionLogic->LoseFocus();
}

bool vtkMRMLLayerDMPipelineManager::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) const
{
  return this->m_interactionLogic->ProcessInteractionEvent(eventData);
}

bool vtkMRMLLayerDMPipelineManager::RemoveNode(vtkMRMLNode* node)
{
  return this->RemovePipeline(node);
}

void vtkMRMLLayerDMPipelineManager::ResetCameraClippingRange()
{
  if (this->m_isResettingClippingRange)
  {
    return;
  }

  this->m_isResettingClippingRange = true;
  this->m_layerManager->ResetCameraClippingRange();
  this->m_isResettingClippingRange = false;
}

void vtkMRMLLayerDMPipelineManager::RequestRender()
{
  this->ResetCameraClippingRange();
  this->m_requestRender();
}

void vtkMRMLLayerDMPipelineManager::OnDefaultCameraModified() const
{
  for (const auto& pipeline : m_pipelineMap)
  {
    pipeline.second->OnDefaultCameraModified(this->m_defaultCamera);
  }
}

vtkMRMLLayerDMPipelineManager::vtkMRMLLayerDMPipelineManager()
  : m_factory{ nullptr }
  , m_layerManager(vtkSmartPointer<vtkMRMLLayerDMLayerManager>::New())
  , m_cameraSync(vtkSmartPointer<vtkMRMLLayerDMCameraSynchronizer>::New())
  , m_interactionLogic(vtkSmartPointer<vtkMRMLLayerDMInteractionLogic>::New())
  , m_eventObs(vtkSmartPointer<vtkObjectEventObserver>::New())
  , m_defaultCamera(vtkSmartPointer<vtkCamera>::New())
  , m_viewNode{ nullptr }
  , m_scene{ nullptr }
  , m_pipelineMap{}
  , m_requestRender{ [] {} }
  , m_isResettingClippingRange(false)
{
  this->m_layerManager->SetDefaultCamera(this->m_defaultCamera);
  this->m_cameraSync->SetDefaultCamera(this->m_defaultCamera);

  this->m_eventObs->SetUpdateCallback(
    [this](vtkObject* obj)
    {
      if (obj == this->m_factory)
      {
        this->UpdateFromScene();
      }

      if (obj == this->m_defaultCamera && !this->m_isResettingClippingRange)
      {
        this->ResetCameraClippingRange();
        this->OnDefaultCameraModified();
      }
    });

  // Monitor camera updates
  this->m_eventObs->UpdateObserver(nullptr, this->m_defaultCamera);
}

void vtkMRMLLayerDMPipelineManager::UpdatePipeline(const vtkSmartPointer<vtkMRMLLayerDMPipelineI>& pipeline) const
{
  if (!pipeline)
  {
    return;
  }

  auto prev = pipeline->BlockResetDisplay(true);
  pipeline->SetViewNode(this->m_viewNode);
  pipeline->BlockResetDisplay(prev);
  pipeline->ResetDisplay();
}

vtkSmartPointer<vtkMRMLLayerDMPipelineI> vtkMRMLLayerDMPipelineManager::GetNodePipeline(vtkMRMLNode* node) const
{
  const auto found = this->m_pipelineMap.find(node);
  if (found == std::end(this->m_pipelineMap))
  {
    return {};
  }
  return found->second;
}

void vtkMRMLLayerDMPipelineManager::SetRenderer(vtkRenderer* renderer) const
{
  this->m_cameraSync->SetRenderer(renderer);
}

void vtkMRMLLayerDMPipelineManager::SetRequestRender(const std::function<void()>& requestRender)
{
  this->m_requestRender = requestRender;
  this->UpdateAllPipelines();
}

vtkCamera* vtkMRMLLayerDMPipelineManager::GetDefaultCamera() const
{
  return this->m_defaultCamera;
}

void vtkMRMLLayerDMPipelineManager::RemoveOutdatedPipelines()
{
  if (!this->m_scene)
  {
    return;
  }

  for (const auto& pipe : m_pipelineMap)
  {
    if (!pipe.first || !this->m_scene->GetNodeByID(pipe.first->GetID()))
    {
      this->RemovePipeline(pipe.first);
    }
  }
}

void vtkMRMLLayerDMPipelineManager::AddMissingPipelines()
{
  if (!this->m_scene)
  {
    return;
  }

  int nNodes = this->m_scene->GetNumberOfNodes();
  for (int iNode = 0; iNode < nNodes; iNode++)
  {
    if (auto node = vtkMRMLNode::SafeDownCast(this->m_scene->GetNodes()->GetItemAsObject(iNode)))
    {
      this->AddNode(node);
    }
  }
}

void vtkMRMLLayerDMPipelineManager::UpdateFromScene()
{
  if (!this->m_scene)
  {
    return;
  }

  this->RemoveOutdatedPipelines();
  this->AddMissingPipelines();
}

void vtkMRMLLayerDMPipelineManager::SetScene(vtkMRMLScene* scene)
{
  if (this->m_scene == scene)
  {
    return;
  }

  this->m_scene = scene;
  for (const auto& [node, pipeline] : m_pipelineMap)
  {
    pipeline->SetScene(scene);
  }
}

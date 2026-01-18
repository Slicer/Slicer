#include "vtkMRMLLayerDMPipelineManager.h"

// Layer DM includes
#include "vtkMRMLLayerDMCameraSynchronizer.h"
#include "vtkMRMLLayerDMInteractionLogic.h"
#include "vtkMRMLLayerDMLayerManager.h"
#include "vtkMRMLLayerDMNodeReferenceObserver.h"
#include "vtkMRMLLayerDMObjectEventObserver.h"
#include "vtkMRMLLayerDMPipelineFactory.h"
#include "vtkMRMLLayerDMPipelineI.h"

// Slicer includes
#include "vtkMRMLAbstractViewNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(vtkMRMLLayerDMPipelineManager);

/// Helper struct to block reset display and reset display once when deleting
struct ResetPipelineDisplayOnceGuard
{
  explicit ResetPipelineDisplayOnceGuard(vtkSmartPointer<vtkMRMLLayerDMPipelineI> pipeline)
    : m_pipeline{ std::move(pipeline) }
  {
    if (m_pipeline)
    {
      m_wasBlocked = m_pipeline->BlockResetDisplay(true);
    }
  }

  ~ResetPipelineDisplayOnceGuard()
  {
    if (m_pipeline)
    {
      m_pipeline->BlockResetDisplay(m_wasBlocked);
      m_pipeline->ResetDisplay();
    }
  }

  vtkSmartPointer<vtkMRMLLayerDMPipelineI> m_pipeline;
  bool m_wasBlocked{};
};

/// Helper struct to block rendering and request render once when deleting
struct RequestRenderOnceGuard
{
  explicit RequestRenderOnceGuard(vtkMRMLLayerDMPipelineManager& pipelineManager)
    : m_pipelineManager{ pipelineManager }
  {
    m_wasBlocked = m_pipelineManager.BlockRequestRender(true);
  }

  ~RequestRenderOnceGuard()
  {
    m_pipelineManager.BlockRequestRender(m_wasBlocked);
    m_pipelineManager.RequestRender();
  }

  vtkMRMLLayerDMPipelineManager& m_pipelineManager;
  bool m_wasBlocked{};
};

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

  RequestRenderOnceGuard renderGuard{ *this };
  ResetPipelineDisplayOnceGuard resetPipelineGuard{ pipeline };
  pipeline->SetViewNode(this->m_viewNode);
  pipeline->SetPipelineManager(this);
  pipeline->SetScene(this->m_scene);
  pipeline->SetViewNode(this->m_viewNode);
  pipeline->SetDisplayNode(displayNode);
  pipeline->OnDefaultCameraModified(this->m_defaultCamera);
  this->m_pipelineMap[displayNode] = pipeline;
  this->m_layerManager->AddPipeline(pipeline);
  this->m_interactionLogic->AddPipeline(pipeline);
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

void vtkMRMLLayerDMPipelineManager::UpdateAllPipelines()
{
  RequestRenderOnceGuard renderGuard{ *this };
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

  RequestRenderOnceGuard renderGuard{ *this };
  this->m_layerManager->RemovePipeline(pipeline);
  this->m_interactionLogic->RemovePipeline(pipeline);
  this->m_pipelineMap.erase(displayNode);
  this->InvokeEvent(vtkCommand::ModifiedEvent);
  return true;
}

void vtkMRMLLayerDMPipelineManager::SetRenderWindow(vtkRenderWindow* renderWindow)
{
  // Observe window resize updates (bound to default camera changed update for representations which depend on the camera / display properties)
  this->m_eventObs->UpdateObserver(this->m_renderWindow, renderWindow, vtkCommand::WindowResizeEvent);
  this->m_renderWindow = renderWindow;
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

void vtkMRMLLayerDMPipelineManager::ResetCameraClippingRange() const
{
  // Block camera sync update triggers during clipping range refresh
  const auto wasBlocked = this->m_cameraSync->BlockModified(true);
  this->m_layerManager->ResetCameraClippingRange();
  this->m_cameraSync->BlockModified(wasBlocked);
}

void vtkMRMLLayerDMPipelineManager::RequestRender()
{
  if (this->m_isRequestRenderBlocked || !this->m_renderWindow)
  {
    return;
  }

  this->BlockRequestRender(true);
  this->ResetCameraClippingRange();
  this->m_requestRender();
  this->BlockRequestRender(false);
}

void vtkMRMLLayerDMPipelineManager::OnDefaultCameraModified()
{
  RequestRenderOnceGuard renderGuard{ *this };
  for (const auto& pipeline : this->m_pipelineMap)
  {
    pipeline.second->OnDefaultCameraModified(this->m_defaultCamera);
  }
}

vtkMRMLLayerDMPipelineManager::vtkMRMLLayerDMPipelineManager()
  : m_factory{ nullptr }
  , m_layerManager(vtkSmartPointer<vtkMRMLLayerDMLayerManager>::New())
  , m_cameraSync(vtkSmartPointer<vtkMRMLLayerDMCameraSynchronizer>::New())
  , m_interactionLogic(vtkSmartPointer<vtkMRMLLayerDMInteractionLogic>::New())
  , m_eventObs(vtkSmartPointer<vtkMRMLLayerDMObjectEventObserver>::New())
  , m_defaultCamera(vtkSmartPointer<vtkCamera>::New())
  , m_nodeRefObs{ vtkSmartPointer<vtkMRMLLayerDMNodeReferenceObserver>::New() }
  , m_viewNode{ nullptr }
  , m_scene{ nullptr }
  , m_pipelineMap{}
  , m_requestRender{ [] {} }
{
  this->m_cameraSync->SetDefaultCamera(this->m_defaultCamera);
  this->m_layerManager->SetDefaultCamera(this->m_defaultCamera);

  this->m_nodeRefObs->SetReferenceModifiedCallBack(
    [this](vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role, int eventType)
    {
      auto pipeline = this->GetNodePipeline(toNode);
      if (!pipeline)
      {
        return;
      }
      if (eventType == vtkMRMLLayerDMNodeReferenceObserver::ReferenceAddedEvent)
      {
        pipeline->OnReferenceToDisplayNodeAdded(fromNode, role);
      }
      else
      {
        pipeline->OnReferenceToDisplayNodeRemoved(fromNode, role);
      }
    });

  this->m_eventObs->SetUpdateCallback(
    [this](vtkObject* obj)
    {
      if (obj == this->m_factory)
      {
        this->UpdateFromScene();
      }

      if (obj == this->m_cameraSync || obj == this->m_renderWindow)
      {
        this->OnDefaultCameraModified();
      }
    });

  // Monitor camera updates
  this->m_eventObs->UpdateObserver(nullptr, this->m_cameraSync);
}

void vtkMRMLLayerDMPipelineManager::UpdatePipeline(const vtkSmartPointer<vtkMRMLLayerDMPipelineI>& pipeline) const
{
  if (!pipeline)
  {
    return;
  }

  ResetPipelineDisplayOnceGuard resetPipelineGuard{ pipeline };
  pipeline->SetViewNode(this->m_viewNode);
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

int vtkMRMLLayerDMPipelineManager::GetNumberOfPipelines() const
{
  return this->m_pipelineMap.size();
}

vtkMRMLLayerDMPipelineI* vtkMRMLLayerDMPipelineManager::GetNthPipeline(int iPipeline) const
{
  if (iPipeline < 0 || iPipeline >= this->m_pipelineMap.size())
  {
    return nullptr;
  }

  return std::next(this->m_pipelineMap.begin(), iPipeline)->second;
}

void vtkMRMLLayerDMPipelineManager::SetRenderer(vtkRenderer* renderer) const
{
  // Pass the renderer to the camera sync
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

  std::vector<vtkWeakPointer<vtkMRMLNode>> outdatedPipelines;
  for (const auto& pipe : m_pipelineMap)
  {
    if (!pipe.first || !this->m_scene->GetNodeByID(pipe.first->GetID()))
    {
      outdatedPipelines.emplace_back(pipe.first);
    }
  }

  for (const auto& pipe : outdatedPipelines)
  {
    this->RemovePipeline(pipe);
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

  RequestRenderOnceGuard renderGuard{ *this };
  this->RemoveOutdatedPipelines();
  this->AddMissingPipelines();
}

bool vtkMRMLLayerDMPipelineManager::BlockRequestRender(bool isBlocked)
{
  const auto wasBlocked = this->m_isRequestRenderBlocked;
  this->m_isRequestRenderBlocked = isBlocked;
  return wasBlocked;
}

void vtkMRMLLayerDMPipelineManager::SetScene(vtkMRMLScene* scene)
{
  if (this->m_scene == scene)
  {
    return;
  }

  this->m_scene = scene;
  this->m_nodeRefObs->SetScene(scene);
  for (const auto& [node, pipeline] : m_pipelineMap)
  {
    pipeline->SetScene(scene);
  }
}

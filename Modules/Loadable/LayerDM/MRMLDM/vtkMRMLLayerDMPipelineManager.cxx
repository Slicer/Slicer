#include "vtkMRMLLayerDMPipelineManager.h"

// Layer DM includes
#include "vtkMRMLLayerDMCameraSynchronizer.h"
#include "vtkMRMLLayerDMInteractionLogic.h"
#include "vtkMRMLLayerDMLayerManager.h"
#include "vtkMRMLLayerDMNodeReferenceObserver.h"
#include "vtkMRMLLayerDMObjectEventObserver.h"
#include "vtkMRMLLayerDMPipelineCreator.h"
#include "vtkMRMLLayerDMPipelineFactory.h"
#include "vtkMRMLLayerDMPipeline.h"

// Slicer includes
#include "vtkMRMLAbstractViewNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(vtkMRMLLayerDMPipelineManager);

/// Helper struct to block display updates and update display once when deleting
struct UpdatePipelineDisplayOnceGuard
{
  explicit UpdatePipelineDisplayOnceGuard(vtkSmartPointer<vtkMRMLLayerDMPipeline> pipeline)
    : Pipeline{ std::move(pipeline) }
  {
    if (this->Pipeline)
    {
      this->WasBlocked = this->Pipeline->BlockUpdateDisplay(true);
    }
  }

  ~UpdatePipelineDisplayOnceGuard()
  {
    if (this->Pipeline)
    {
      this->Pipeline->BlockUpdateDisplay(this->WasBlocked);
      this->Pipeline->UpdateDisplay();
    }
  }

  vtkSmartPointer<vtkMRMLLayerDMPipeline> Pipeline;
  bool WasBlocked{};
};

/// Helper struct to block rendering and request render once when deleting
struct RequestRenderOnceGuard
{
  explicit RequestRenderOnceGuard(vtkMRMLLayerDMPipelineManager& pipelineManager)
    : PipelineManager{ pipelineManager }
  {
    this->WasBlocked = this->PipelineManager.BlockRequestRender(true);
  }

  ~RequestRenderOnceGuard()
  {
    this->PipelineManager.BlockRequestRender(this->WasBlocked);
    this->PipelineManager.RequestRender();
  }

  vtkMRMLLayerDMPipelineManager& PipelineManager;
  bool WasBlocked{};
};

bool vtkMRMLLayerDMPipelineManager::CreatePipelineForNode(vtkMRMLNode* displayNode)
{
  // Early return if manager is not yet created
  if (!this->Factory || !this->ViewNode)
  {
    return false;
  }

  auto pipeline = this->Factory->CreatePipeline(this->ViewNode, displayNode);
  if (!pipeline)
  {
    return false;
  }

  RequestRenderOnceGuard renderGuard{ *this };
  UpdatePipelineDisplayOnceGuard updatePipelineGuard{ pipeline };
  pipeline->SetViewNode(this->ViewNode);
  pipeline->SetPipelineManager(this);
  pipeline->SetScene(this->Scene);
  pipeline->SetViewNode(this->ViewNode);
  pipeline->SetDisplayNode(displayNode);
  pipeline->OnDefaultCameraModified(this->DefaultCamera);
  this->PipelineMap[displayNode] = pipeline;
  this->PipelineCreatorMap[displayNode] = this->Factory->GetLastCreator();
  this->LayerManager->AddPipeline(pipeline);
  this->InteractionLogic->AddPipeline(pipeline);
  this->UpdatePipeline(pipeline);
  this->InvokeEvent(vtkCommand::ModifiedEvent);
  return true;
}

void vtkMRMLLayerDMPipelineManager::ClearDisplayableNodes()
{
  this->PipelineMap.clear();
  this->PipelineCreatorMap.clear();
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
  for (const auto& pipeline : this->PipelineMap)
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
  pipeline->SetFrozen(true);
  // Let interaction logic process the removal first if the pipeline needs to lose focus.
  this->InteractionLogic->RemovePipeline(pipeline);
  this->LayerManager->RemovePipeline(pipeline);
  this->PipelineMap.erase(displayNode);
  this->PipelineCreatorMap.erase(displayNode);
  this->InvokeEvent(vtkCommand::ModifiedEvent);
  return true;
}

void vtkMRMLLayerDMPipelineManager::SetRenderWindow(vtkRenderWindow* renderWindow)
{
  // Observe window resize updates (bound to default camera changed update for representations which depend on the camera / display properties)
  this->EventObserver->UpdateObserver(this->RenderWindow, renderWindow, vtkCommand::WindowResizeEvent);
  this->RenderWindow = renderWindow;
  this->LayerManager->SetRenderWindow(renderWindow);
}

void vtkMRMLLayerDMPipelineManager::SetViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  if (this->ViewNode == viewNode)
  {
    return;
  }

  this->ViewNode = viewNode;
  this->CameraSynchronizer->SetViewNode(viewNode);
  this->InteractionLogic->SetViewNode(viewNode);
  this->UpdateAllPipelines();
}

void vtkMRMLLayerDMPipelineManager::SetFactory(const vtkSmartPointer<vtkMRMLLayerDMPipelineFactory>& factory)
{
  if (this->Factory == factory)
  {
    return;
  }

  this->EventObserver->UpdateObserver(this->Factory, factory);
  this->Factory = factory;
  this->UpdateFromScene();
}

int vtkMRMLLayerDMPipelineManager::GetMouseCursor() const
{
  auto lastFocused = this->InteractionLogic->GetLastFocusedPipeline();
  return lastFocused ? lastFocused->GetMouseCursor() : VTK_CURSOR_DEFAULT;
}

bool vtkMRMLLayerDMPipelineManager::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2) const
{
  return this->InteractionLogic->CanProcessInteractionEvent(eventData, distance2);
}

void vtkMRMLLayerDMPipelineManager::LoseFocus(vtkMRMLInteractionEventData* eventData) const
{
  this->InteractionLogic->LoseFocus(eventData);
}

void vtkMRMLLayerDMPipelineManager::LoseFocus() const
{
  this->InteractionLogic->LoseFocus();
}

bool vtkMRMLLayerDMPipelineManager::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) const
{
  return this->InteractionLogic->ProcessInteractionEvent(eventData);
}

bool vtkMRMLLayerDMPipelineManager::RemoveNode(vtkMRMLNode* node)
{
  return this->RemovePipeline(node);
}

void vtkMRMLLayerDMPipelineManager::ResetCameraClippingRange() const
{
  // Block camera sync update triggers during clipping range refresh
  const auto wasBlocked = this->CameraSynchronizer->BlockModified(true);
  this->LayerManager->ResetCameraClippingRange();
  this->CameraSynchronizer->BlockModified(wasBlocked);
}

void vtkMRMLLayerDMPipelineManager::RequestRender()
{
  if (this->IsRequestRenderBlocked || !this->RenderWindow)
  {
    return;
  }

  this->BlockRequestRender(true);
  this->ResetCameraClippingRange();
  this->RequestRenderCallback();
  this->BlockRequestRender(false);
}

void vtkMRMLLayerDMPipelineManager::OnDefaultCameraModified()
{
  RequestRenderOnceGuard renderGuard{ *this };
  for (const auto& pipeline : this->PipelineMap)
  {
    pipeline.second->OnDefaultCameraModified(this->DefaultCamera);
  }
}

vtkMRMLLayerDMPipelineManager::vtkMRMLLayerDMPipelineManager()
  : Factory{ nullptr }
  , LayerManager(vtkSmartPointer<vtkMRMLLayerDMLayerManager>::New())
  , CameraSynchronizer(vtkSmartPointer<vtkMRMLLayerDMCameraSynchronizer>::New())
  , InteractionLogic(vtkSmartPointer<vtkMRMLLayerDMInteractionLogic>::New())
  , EventObserver(vtkSmartPointer<vtkMRMLLayerDMObjectEventObserver>::New())
  , DefaultCamera(vtkSmartPointer<vtkCamera>::New())
  , NodeReferenceObserver{ vtkSmartPointer<vtkMRMLLayerDMNodeReferenceObserver>::New() }
  , ViewNode{ nullptr }
  , Scene{ nullptr }
  , PipelineMap{}
  , RequestRenderCallback{ [] {} }
{
  this->CameraSynchronizer->SetDefaultCamera(this->DefaultCamera);
  this->LayerManager->SetDefaultCamera(this->DefaultCamera);

  this->NodeReferenceObserver->SetReferenceModifiedCallBack(
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

  this->EventObserver->SetUpdateCallback(
    [this](vtkObject* obj)
    {
      if (obj == this->Factory)
      {
        this->UpdateFromScene();
      }

      if (obj == this->CameraSynchronizer || obj == this->RenderWindow)
      {
        this->OnDefaultCameraModified();
      }
    });

  // Monitor camera updates
  this->EventObserver->UpdateObserver(nullptr, this->CameraSynchronizer);
}

void vtkMRMLLayerDMPipelineManager::UpdatePipeline(const vtkSmartPointer<vtkMRMLLayerDMPipeline>& pipeline) const
{
  if (!pipeline)
  {
    return;
  }

  UpdatePipelineDisplayOnceGuard updatePipelineGuard{ pipeline };
  pipeline->SetViewNode(this->ViewNode);
}

vtkSmartPointer<vtkMRMLLayerDMPipeline> vtkMRMLLayerDMPipelineManager::GetNodePipeline(vtkMRMLNode* node) const
{
  const auto found = this->PipelineMap.find(node);
  if (found == std::end(this->PipelineMap))
  {
    return {};
  }
  return found->second;
}

int vtkMRMLLayerDMPipelineManager::GetNumberOfPipelines() const
{
  return this->PipelineMap.size();
}

vtkMRMLLayerDMPipeline* vtkMRMLLayerDMPipelineManager::GetNthPipeline(int iPipeline) const
{
  if (iPipeline < 0 || iPipeline >= this->PipelineMap.size())
  {
    return nullptr;
  }

  return std::next(this->PipelineMap.begin(), iPipeline)->second;
}

void vtkMRMLLayerDMPipelineManager::SetRenderer(vtkRenderer* renderer) const
{
  // Pass the renderer to the camera sync
  this->CameraSynchronizer->SetRenderer(renderer);
}

void vtkMRMLLayerDMPipelineManager::SetRequestRender(const std::function<void()>& requestRender)
{
  this->RequestRenderCallback = requestRender;
  this->UpdateAllPipelines();
}

vtkCamera* vtkMRMLLayerDMPipelineManager::GetDefaultCamera() const
{
  return this->DefaultCamera;
}

void vtkMRMLLayerDMPipelineManager::RemoveOutdatedPipelines()
{
  if (!this->Scene)
  {
    return;
  }

  std::vector<vtkWeakPointer<vtkMRMLNode>> outdatedPipelines;
  for (const auto& pipe : this->PipelineMap)
  {
    if (!pipe.first || !this->Scene->GetNodeByID(pipe.first->GetID()) || this->IsPipelineCreatorOutdated(pipe.first))
    {
      outdatedPipelines.emplace_back(pipe.first);
    }
  }

  for (const auto& pipe : outdatedPipelines)
  {
    this->RemovePipeline(pipe);
  }
}

bool vtkMRMLLayerDMPipelineManager::IsPipelineCreatorOutdated(vtkMRMLNode* node) const
{
  const auto found = this->PipelineCreatorMap.find(node);
  if (found == std::end(this->PipelineCreatorMap))
  {
    return false;
  }
  return !found->second || !this->Factory || !this->Factory->ContainsPipelineCreator(found->second.GetPointer());
}

void vtkMRMLLayerDMPipelineManager::AddMissingPipelines()
{
  if (!this->Scene)
  {
    return;
  }

  int nNodes = this->Scene->GetNumberOfNodes();
  for (int iNode = 0; iNode < nNodes; iNode++)
  {
    if (auto node = vtkMRMLNode::SafeDownCast(this->Scene->GetNodes()->GetItemAsObject(iNode)))
    {
      this->AddNode(node);
    }
  }
}

void vtkMRMLLayerDMPipelineManager::UpdateFromScene()
{
  if (!this->Scene)
  {
    return;
  }

  RequestRenderOnceGuard renderGuard{ *this };
  this->RemoveOutdatedPipelines();
  this->AddMissingPipelines();
}

bool vtkMRMLLayerDMPipelineManager::BlockRequestRender(bool isBlocked)
{
  const auto wasBlocked = this->IsRequestRenderBlocked;
  this->IsRequestRenderBlocked = isBlocked;
  return wasBlocked;
}

void vtkMRMLLayerDMPipelineManager::SetScene(vtkMRMLScene* scene)
{
  if (this->Scene == scene)
  {
    return;
  }

  this->Scene = scene;
  this->NodeReferenceObserver->SetScene(scene);
  for (const auto& [node, pipeline] : this->PipelineMap)
  {
    pipeline->SetScene(scene);
  }
}

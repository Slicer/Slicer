/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

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
#include <vtkCollection.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

// STD includes
#include <vector>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLLayerDMPipelineManager);

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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
  // Observe the node destruction so that the maps keyed by this node can be cleaned up while its address is
  // still valid. \sa vtkMRMLLayerDMObjectEventObserver::SetDeleteCallback
  this->EventObserver->UpdateObservation(nullptr, displayNode, vtkCommand::DeleteEvent);
  this->LayerManager->AddPipeline(pipeline);
  this->InteractionLogic->AddPipeline(pipeline);
  this->UpdatePipeline(pipeline);
  this->InvokeEvent(vtkCommand::ModifiedEvent);
  return true;
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineManager::ClearDisplayableNodes()
{
  RequestRenderOnceGuard renderGuard{ *this };

  // The interaction logic holds strong references to the pipelines and the layer manager holds
  // their renderers, so clearing the maps alone would leave the pipelines alive, with their
  // actors still displayed and still receiving interaction events.
  for (const auto& [node, pipeline] : this->PipelineMap)
  {
    if (!pipeline)
    {
      continue;
    }
    pipeline->SetFrozen(true);
    this->InteractionLogic->RemovePipeline(pipeline);
    this->LayerManager->RemovePipeline(pipeline);
    this->EventObserver->RemoveObservations(node);
  }

  this->PipelineMap.clear();
  this->PipelineCreatorMap.clear();
}

//-----------------------------------------------------------------------------
bool vtkMRMLLayerDMPipelineManager::AddNode(vtkMRMLNode* node)
{
  if (auto pipeline = this->GetNodePipeline(node))
  {
    return false;
  }

  return this->CreatePipelineForNode(node);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineManager::UpdateAllPipelines()
{
  RequestRenderOnceGuard renderGuard{ *this };
  for (const auto& pipeline : this->PipelineMap)
  {
    this->UpdatePipeline(pipeline.second);
  }
}

//-----------------------------------------------------------------------------
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
  this->EventObserver->RemoveObservations(displayNode);
  this->PipelineMap.erase(displayNode);
  this->PipelineCreatorMap.erase(displayNode);
  this->InvokeEvent(vtkCommand::ModifiedEvent);
  return true;
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineManager::SetRenderWindow(vtkRenderWindow* renderWindow)
{
  // Observe window resize updates (bound to default camera changed update for representations which depend on the camera / display properties)
  this->EventObserver->UpdateObservation(this->RenderWindow, renderWindow, vtkCommand::WindowResizeEvent);
  this->RenderWindow = renderWindow;
  this->LayerManager->SetRenderWindow(renderWindow);
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineManager::SetFactory(const vtkSmartPointer<vtkMRMLLayerDMPipelineFactory>& factory)
{
  if (this->Factory == factory)
  {
    return;
  }

  this->EventObserver->UpdateObservation(this->Factory, factory);
  this->Factory = factory;
  this->UpdateFromScene();
}

//-----------------------------------------------------------------------------
int vtkMRMLLayerDMPipelineManager::GetMouseCursor() const
{
  auto lastFocused = this->InteractionLogic->GetLastFocusedPipeline();
  return lastFocused ? lastFocused->GetMouseCursor() : VTK_CURSOR_DEFAULT;
}

//-----------------------------------------------------------------------------
bool vtkMRMLLayerDMPipelineManager::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2) const
{
  return this->InteractionLogic->CanProcessInteractionEvent(eventData, distance2);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineManager::LoseFocus(vtkMRMLInteractionEventData* eventData) const
{
  this->InteractionLogic->LoseFocus(eventData);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineManager::LoseFocus() const
{
  this->InteractionLogic->LoseFocus();
}

//-----------------------------------------------------------------------------
bool vtkMRMLLayerDMPipelineManager::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) const
{
  return this->InteractionLogic->ProcessInteractionEvent(eventData);
}

//-----------------------------------------------------------------------------
bool vtkMRMLLayerDMPipelineManager::RemoveNode(vtkMRMLNode* node)
{
  return this->RemovePipeline(node);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineManager::ResetCameraClippingRange() const
{
  // Block camera sync update triggers during clipping range refresh
  const auto wasBlocked = this->CameraSynchronizer->BlockModified(true);
  this->LayerManager->ResetCameraClippingRange();
  this->CameraSynchronizer->BlockModified(wasBlocked);
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineManager::OnDefaultCameraModified()
{
  RequestRenderOnceGuard renderGuard{ *this };
  for (const auto& pipeline : this->PipelineMap)
  {
    pipeline.second->OnDefaultCameraModified(this->DefaultCamera);
  }
}

//-----------------------------------------------------------------------------
vtkMRMLLayerDMPipelineManager::vtkMRMLLayerDMPipelineManager()
  : LayerManager(vtkSmartPointer<vtkMRMLLayerDMLayerManager>::New())
  , CameraSynchronizer(vtkSmartPointer<vtkMRMLLayerDMCameraSynchronizer>::New())
  , InteractionLogic(vtkSmartPointer<vtkMRMLLayerDMInteractionLogic>::New())
  , EventObserver(vtkSmartPointer<vtkMRMLLayerDMObjectEventObserver>::New())
  , DefaultCamera(vtkSmartPointer<vtkCamera>::New())
  , NodeReferenceObserver{ vtkSmartPointer<vtkMRMLLayerDMNodeReferenceObserver>::New() }
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

  // A display node destroyed without being removed from the scene first would otherwise leave its pipeline
  // running, and its address in the pipeline maps, where its weak pointer would null itself in place and break
  // their ordering.
  this->EventObserver->SetDeleteCallback(
    [this](vtkObject* obj)
    {
      if (auto node = vtkMRMLNode::SafeDownCast(obj))
      {
        this->RemovePipeline(node);
      }
    });

  // Monitor camera updates
  this->EventObserver->UpdateObservation(nullptr, this->CameraSynchronizer);
}

//-----------------------------------------------------------------------------
vtkMRMLLayerDMPipelineManager::~vtkMRMLLayerDMPipelineManager()
{
  // Releasing the observed objects below destroys them, which would otherwise invoke the delete callback and
  // re-enter this object while its members are being destroyed.
  this->EventObserver->ClearCallbacks();
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineManager::UpdatePipeline(const vtkSmartPointer<vtkMRMLLayerDMPipeline>& pipeline) const
{
  if (!pipeline)
  {
    return;
  }

  UpdatePipelineDisplayOnceGuard updatePipelineGuard{ pipeline };
  pipeline->SetViewNode(this->ViewNode);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkMRMLLayerDMPipeline> vtkMRMLLayerDMPipelineManager::GetNodePipeline(vtkMRMLNode* node) const
{
  const auto found = this->PipelineMap.find(node);
  if (found == std::end(this->PipelineMap))
  {
    return {};
  }
  return found->second;
}

//-----------------------------------------------------------------------------
int vtkMRMLLayerDMPipelineManager::GetNumberOfPipelines() const
{
  return static_cast<int>(this->PipelineMap.size());
}

//-----------------------------------------------------------------------------
vtkMRMLLayerDMPipeline* vtkMRMLLayerDMPipelineManager::GetNthPipeline(int iPipeline) const
{
  if (iPipeline < 0 || iPipeline >= this->GetNumberOfPipelines())
  {
    return nullptr;
  }

  return std::next(this->PipelineMap.begin(), iPipeline)->second;
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineManager::SetRenderer(vtkRenderer* renderer) const
{
  // Pass the renderer to the camera sync
  this->CameraSynchronizer->SetRenderer(renderer);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineManager::SetRequestRender(const std::function<void()>& requestRender)
{
  this->RequestRenderCallback = requestRender;
  this->UpdateAllPipelines();
}

//-----------------------------------------------------------------------------
vtkCamera* vtkMRMLLayerDMPipelineManager::GetDefaultCamera() const
{
  return this->DefaultCamera;
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineManager::RemoveOutdatedPipelines()
{
  if (!this->Scene)
  {
    return;
  }

  // The map only contains live nodes, as a node is removed from it when it is destroyed, so the nodes can
  // safely be dereferenced here. \sa vtkMRMLLayerDMObjectEventObserver::SetDeleteCallback
  std::vector<vtkMRMLNode*> outdatedPipelines;
  for (const auto& [node, pipeline] : this->PipelineMap)
  {
    if (!this->Scene->GetNodeByID(node->GetID()) || this->IsPipelineCreatorOutdated(node))
    {
      outdatedPipelines.emplace_back(node);
    }
  }

  for (const auto& node : outdatedPipelines)
  {
    this->RemovePipeline(node);
  }
}

//-----------------------------------------------------------------------------
bool vtkMRMLLayerDMPipelineManager::IsPipelineCreatorOutdated(vtkMRMLNode* node) const
{
  const auto found = this->PipelineCreatorMap.find(node);
  if (found == std::end(this->PipelineCreatorMap))
  {
    return false;
  }
  return !found->second || !this->Factory || !this->Factory->ContainsPipelineCreator(found->second.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineManager::AddMissingPipelines()
{
  if (!this->Scene)
  {
    return;
  }

  // Traverse the collection with an iterator: vtkCollection::GetItemAsObject walks the collection from its
  // first item on every call, which makes an indexed scan quadratic in the number of nodes.
  // Collect the nodes first, as adding a pipeline invokes events and can run pipeline creator code, which may
  // in turn modify the scene.
  std::vector<vtkSmartPointer<vtkMRMLNode>> sceneNodes;
  vtkObject* item = nullptr;
  vtkCollectionSimpleIterator it;
  for (this->Scene->GetNodes()->InitTraversal(it); (item = this->Scene->GetNodes()->GetNextItemAsObject(it));)
  {
    if (auto node = vtkMRMLNode::SafeDownCast(item))
    {
      sceneNodes.emplace_back(node);
    }
  }

  for (const auto& node : sceneNodes)
  {
    this->AddNode(node);
  }
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
bool vtkMRMLLayerDMPipelineManager::BlockRequestRender(bool isBlocked)
{
  const auto wasBlocked = this->IsRequestRenderBlocked;
  this->IsRequestRenderBlocked = isBlocked;
  return wasBlocked;
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Number of pipelines: " << this->PipelineMap.size() << std::endl;
  os << indent << "View node: " << (this->ViewNode ? "set" : "(none)") << std::endl;
  os << indent << "Scene: " << (this->Scene ? "set" : "(none)") << std::endl;
  os << indent << "Render window: " << (this->RenderWindow ? "set" : "(none)") << std::endl;
  os << indent << "Request render blocked: " << (this->IsRequestRenderBlocked ? "true" : "false") << std::endl;
}

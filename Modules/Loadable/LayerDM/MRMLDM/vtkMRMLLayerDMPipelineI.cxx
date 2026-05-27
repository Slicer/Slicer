#include "vtkMRMLLayerDMPipelineI.h"

// Layer DM includes
#include "vtkMRMLAbstractWidget.h"
#include "vtkMRMLLayerDMPipelineManager.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLLayerDMObjectEventObserver.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(vtkMRMLLayerDMPipelineI);

void vtkMRMLLayerDMPipelineI::UpdatePipeline() {}

void vtkMRMLLayerDMPipelineI::OnRendererRemoved(vtkRenderer* renderer) {}

void vtkMRMLLayerDMPipelineI::OnRendererAdded(vtkRenderer* renderer) {}

void vtkMRMLLayerDMPipelineI::SetDisplayNode(vtkMRMLNode* displayNode)
{
  this->UpdateObserver(this->m_displayNode, displayNode);
  this->m_displayNode = displayNode;
}

void vtkMRMLLayerDMPipelineI::ResetDisplay()
{
  if (this->m_isResetDisplayBlocked || !this->m_viewNode)
  {
    return;
  }

  // Make sure to avoid looping reset display during processing
  this->BlockResetDisplay(true);
  this->UpdatePipeline();
  this->RequestRender();
  this->BlockResetDisplay(false);
}

void vtkMRMLLayerDMPipelineI::SetViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  this->UpdateObserver(this->m_viewNode, viewNode);
  this->m_viewNode = viewNode;
}

bool vtkMRMLLayerDMPipelineI::BlockResetDisplay(bool isBlocked)
{
  if (this->m_isFrozen)
  {
    return true;
  }

  bool prev = this->m_isResetDisplayBlocked;
  this->m_isResetDisplayBlocked = isBlocked;
  return prev;
}

bool vtkMRMLLayerDMPipelineI::BlockInteractionProcessing(bool isBlocked)
{
  if (this->m_isFrozen)
  {
    return true;
  }

  const auto prev = this->m_isInteractionProcessingBlocked;
  this->m_isInteractionProcessingBlocked = isBlocked;
  return prev;
}

bool vtkMRMLLayerDMPipelineI::IsInteractionProcessingBlocked() const
{
  return this->m_isInteractionProcessingBlocked;
}

bool vtkMRMLLayerDMPipelineI::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)
{
  return false;
}

bool vtkMRMLLayerDMPipelineI::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  return false;
}

int vtkMRMLLayerDMPipelineI::GetMouseCursor() const
{
  return 0;
}

int vtkMRMLLayerDMPipelineI::GetWidgetState() const
{
  return vtkMRMLAbstractWidget::WidgetStateIdle;
}

void vtkMRMLLayerDMPipelineI::LoseFocus(vtkMRMLInteractionEventData* eventData) {}

void vtkMRMLLayerDMPipelineI::OnDefaultCameraModified(vtkCamera* camera) {}

void vtkMRMLLayerDMPipelineI::OnReferenceToDisplayNodeAdded(vtkMRMLNode* fromNode, const std::string& role)
{
  this->OnUpdate(this->GetDisplayNode(), vtkMRMLNode::ReferenceAddedEvent, nullptr);
}

void vtkMRMLLayerDMPipelineI::OnReferenceToDisplayNodeRemoved(vtkMRMLNode* fromNode, const std::string& role)
{
  this->OnUpdate(this->GetDisplayNode(), vtkMRMLNode::ReferenceRemovedEvent, nullptr);
}

bool vtkMRMLLayerDMPipelineI::UpdateObserver(vtkObject* prevObj, vtkObject* obj, unsigned long event) const
{
  return this->m_obs->UpdateObserver(prevObj, obj, event);
}

bool vtkMRMLLayerDMPipelineI::UpdateObserver(vtkObject* prevObj, vtkObject* obj, const std::vector<unsigned long>& events) const
{
  return this->m_obs->UpdateObserver(prevObj, obj, events);
}

unsigned int vtkMRMLLayerDMPipelineI::GetRenderOrder() const
{
  return 0;
}

std::vector<vtkRenderer*> vtkMRMLLayerDMPipelineI::GetRenderers() const
{
  std::vector<vtkRenderer*> renderers;
  for (const auto& [_, renderer] : this->m_renderersMap)
  {
    renderers.emplace_back(renderer);
  }
  return renderers;
}

std::vector<unsigned int> vtkMRMLLayerDMPipelineI::GetRenderOrders() const
{
  return { this->GetRenderOrder() };
}

vtkCamera* vtkMRMLLayerDMPipelineI::GetCustomCamera() const
{
  return nullptr;
}

vtkCamera* vtkMRMLLayerDMPipelineI::GetCustomCamera(unsigned int renderOrder) const
{
  return GetCustomCamera();
}

unsigned int vtkMRMLLayerDMPipelineI::GetMaxRenderOrder() const
{
  const auto renderOrders = GetRenderOrders();
  if (renderOrders.empty())
  {
    return 0;
  }
  return *std::max_element(renderOrders.begin(), renderOrders.end());
}

unsigned int vtkMRMLLayerDMPipelineI::GetVtkRendererOrder(const vtkRenderer* renderer) const
{
  if (!renderer)
  {
    return 0;
  }

  for (const auto& [order, mappedRenderers] : this->m_renderersMap)
  {
    if (mappedRenderers.GetPointer() == renderer)
    {
      return order;
    }
  }
  return 0;
}

void vtkMRMLLayerDMPipelineI::SetRenderers(const std::vector<vtkRenderer*>& renderers, const std::vector<unsigned int>& renderOrders)
{
  if (this->RenderersMatchPipelineRenderers(renderers, renderOrders))
  {
    return;
  }

  if (renderers.size() != renderOrders.size())
  {
    const std::string viewId = this->GetViewNode() ? this->GetViewNode()->GetID() : "null";
    const std::string displayId = this->GetDisplayNode() ? this->GetDisplayNode()->GetID() : "null";
    vtkErrorMacro("Renderer / render order mismatch for DM pipeline ViewNode: " << viewId << ", DisplayNode: " << displayId);
    return;
  }

  // Remove the previous renderers
  for (const auto& [order, renderer] : this->m_renderersMap)
  {
    this->OnRendererRemoved(renderer);
  }
  this->m_renderersMap.clear();

  // Early return if not currently added to any renderer
  if (renderers.empty())
  {
    return;
  }

  // Add the new renderers
  for (size_t i = 0; i < renderers.size(); ++i)
  {
    this->m_renderersMap[renderOrders[i]] = renderers[i];
    this->OnRendererAdded(renderers[i]);
  }
  this->ResetDisplay();
}

bool vtkMRMLLayerDMPipelineI::RenderersMatchPipelineRenderers(const std::vector<vtkRenderer*>& renderers, const std::vector<unsigned int>& renderOrders)
{
  if (renderers.size() != this->m_renderersMap.size() || renderOrders.size() != this->m_renderersMap.size())
  {
    return false;
  }

  for (size_t i = 0; i < renderOrders.size(); ++i)
  {
    const auto expectedOrder = renderOrders[i];
    const auto expectedRenderer = renderers[i];

    if (const auto it = this->m_renderersMap.find(expectedOrder); it == this->m_renderersMap.end() || it->second.GetPointer() != expectedRenderer)
    {
      return false;
    }
  }
  return true;
}

void vtkMRMLLayerDMPipelineI::SetRenderer(vtkRenderer* renderer)
{
  this->SetRenderers({ renderer }, { this->GetRenderOrder() });
}

void vtkMRMLLayerDMPipelineI::SetScene(vtkMRMLScene* scene)
{
  this->m_scene = scene;
}

vtkMRMLLayerDMPipelineI* vtkMRMLLayerDMPipelineI::GetNodePipeline(vtkMRMLNode* node) const
{
  if (!this->m_pipelineManager)
  {
    return nullptr;
  }
  return this->m_pipelineManager->GetNodePipeline(node);
}

vtkMRMLLayerDMPipelineManager* vtkMRMLLayerDMPipelineI::GetPipelineManager() const
{
  return m_pipelineManager;
}

vtkMRMLAbstractViewNode* vtkMRMLLayerDMPipelineI::GetViewNode() const
{
  return this->m_viewNode;
}

bool vtkMRMLLayerDMPipelineI::BlockUpdateObserver(bool isBlocked) const
{
  if (this->m_isFrozen)
  {
    return true;
  }

  return this->m_obs->SetBlocked(isBlocked);
}

bool vtkMRMLLayerDMPipelineI::IsUpdateObserverBlocked() const
{
  return this->m_obs->IsBlocked();
}

void vtkMRMLLayerDMPipelineI::SetFrozen(bool isFrozen)
{
  if (this->m_isFrozen == isFrozen)
  {
    return;
  }

  // Block states are only updated when the pipeline is not frozen.
  // Unfreeze to update all before saving the frozen state.
  this->m_isFrozen = false;
  this->BlockInteractionProcessing(isFrozen);
  this->BlockResetDisplay(isFrozen);
  this->BlockUpdateObserver(isFrozen);
  this->m_isFrozen = isFrozen;
}

bool vtkMRMLLayerDMPipelineI::IsFrozen() const
{
  return this->m_isFrozen;
}

vtkMRMLNode* vtkMRMLLayerDMPipelineI::GetDisplayNode() const
{
  return this->m_displayNode;
}

vtkRenderer* vtkMRMLLayerDMPipelineI::GetRenderer() const
{
  if (this->m_renderersMap.empty())
  {
    return nullptr;
  }
  return this->m_renderersMap.begin()->second;
}

vtkRenderer* vtkMRMLLayerDMPipelineI::GetRenderer(unsigned int renderOrder) const
{
  const auto it = this->m_renderersMap.find(renderOrder);
  if (it == std::end(this->m_renderersMap))
  {
    return nullptr;
  }
  return it->second;
}

vtkMRMLScene* vtkMRMLLayerDMPipelineI::GetScene() const
{
  return this->m_scene;
}

void vtkMRMLLayerDMPipelineI::OnUpdate(vtkObject* obj, unsigned long eventId, void* callData) {}

void vtkMRMLLayerDMPipelineI::RemoveObserver(vtkObject* prevObj) const
{
  this->m_obs->RemoveObserver(prevObj);
}

void vtkMRMLLayerDMPipelineI::RequestRender() const
{
  if (this->m_pipelineManager)
  {
    this->m_pipelineManager->RequestRender();
  }
}

void vtkMRMLLayerDMPipelineI::SetPipelineManager(vtkMRMLLayerDMPipelineManager* pipelineManager)
{
  this->m_pipelineManager = pipelineManager;
}

vtkMRMLLayerDMPipelineI::vtkMRMLLayerDMPipelineI()
  : m_viewNode{ nullptr }
  , m_displayNode{ nullptr }
  , m_renderersMap{}
  , m_isResetDisplayBlocked{ false }
  , m_isFrozen{ false }
  , m_isInteractionProcessingBlocked{ false }
  , m_obs(vtkSmartPointer<vtkMRMLLayerDMObjectEventObserver>::New())
  , m_pipelineManager(nullptr)
{
  this->m_obs->SetUpdateCallback([this](vtkObject* obj, unsigned long eventId, void* callData) { this->OnUpdate(obj, eventId, callData); });
}

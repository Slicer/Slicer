#include "vtkMRMLLayerDMPipeline.h"

// Layer DM includes
#include "vtkMRMLAbstractWidget.h"
#include "vtkMRMLLayerDMPipelineManager.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLLayerDMObjectEventObserver.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(vtkMRMLLayerDMPipeline);

void vtkMRMLLayerDMPipeline::UpdateFromMRML() {}

void vtkMRMLLayerDMPipeline::OnRendererRemoved(vtkRenderer* renderer) {}

void vtkMRMLLayerDMPipeline::OnRendererAdded(vtkRenderer* renderer) {}

void vtkMRMLLayerDMPipeline::SetDisplayNode(vtkMRMLNode* displayNode)
{
  this->UpdateObserver(this->m_displayNode, displayNode);
  this->m_displayNode = displayNode;
}

void vtkMRMLLayerDMPipeline::UpdateDisplay()
{
  if (this->m_isUpdateDisplayBlocked || !this->m_viewNode)
  {
    return;
  }

  // Make sure to avoid looping display update during processing
  this->BlockUpdateDisplay(true);
  this->UpdateFromMRML();
  this->RequestRender();
  this->BlockUpdateDisplay(false);
}

void vtkMRMLLayerDMPipeline::SetViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  this->UpdateObserver(this->m_viewNode, viewNode);
  this->m_viewNode = viewNode;
}

bool vtkMRMLLayerDMPipeline::BlockUpdateDisplay(bool isBlocked)
{
  if (this->m_isFrozen)
  {
    return true;
  }

  bool prev = this->m_isUpdateDisplayBlocked;
  this->m_isUpdateDisplayBlocked = isBlocked;
  return prev;
}

bool vtkMRMLLayerDMPipeline::BlockInteractionProcessing(bool isBlocked)
{
  if (this->m_isFrozen)
  {
    return true;
  }

  const auto prev = this->m_isInteractionProcessingBlocked;
  this->m_isInteractionProcessingBlocked = isBlocked;
  return prev;
}

bool vtkMRMLLayerDMPipeline::IsInteractionProcessingBlocked() const
{
  return this->m_isInteractionProcessingBlocked;
}

bool vtkMRMLLayerDMPipeline::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)
{
  return false;
}

bool vtkMRMLLayerDMPipeline::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  return false;
}

int vtkMRMLLayerDMPipeline::GetMouseCursor() const
{
  return 0;
}

int vtkMRMLLayerDMPipeline::GetWidgetState() const
{
  return vtkMRMLAbstractWidget::WidgetStateIdle;
}

void vtkMRMLLayerDMPipeline::LoseFocus(vtkMRMLInteractionEventData* eventData) {}

void vtkMRMLLayerDMPipeline::OnDefaultCameraModified(vtkCamera* camera) {}

void vtkMRMLLayerDMPipeline::OnReferenceToDisplayNodeAdded(vtkMRMLNode* fromNode, const std::string& role)
{
  this->OnUpdate(this->GetDisplayNode(), vtkMRMLNode::ReferenceAddedEvent, nullptr);
}

void vtkMRMLLayerDMPipeline::OnReferenceToDisplayNodeRemoved(vtkMRMLNode* fromNode, const std::string& role)
{
  this->OnUpdate(this->GetDisplayNode(), vtkMRMLNode::ReferenceRemovedEvent, nullptr);
}

bool vtkMRMLLayerDMPipeline::UpdateObserver(vtkObject* prevObj, vtkObject* obj, unsigned long event) const
{
  return this->m_obs->UpdateObserver(prevObj, obj, event);
}

bool vtkMRMLLayerDMPipeline::UpdateObserver(vtkObject* prevObj, vtkObject* obj, const std::vector<unsigned long>& events) const
{
  return this->m_obs->UpdateObserver(prevObj, obj, events);
}

unsigned int vtkMRMLLayerDMPipeline::GetRenderOrder() const
{
  return 0;
}

std::vector<vtkRenderer*> vtkMRMLLayerDMPipeline::GetRenderers() const
{
  std::vector<vtkRenderer*> renderers;
  for (const auto& [_, renderer] : this->m_renderersMap)
  {
    renderers.emplace_back(renderer);
  }
  return renderers;
}

std::vector<unsigned int> vtkMRMLLayerDMPipeline::GetRenderOrders() const
{
  return { this->GetRenderOrder() };
}

vtkCamera* vtkMRMLLayerDMPipeline::GetCustomCamera() const
{
  return nullptr;
}

vtkCamera* vtkMRMLLayerDMPipeline::GetCustomCamera(unsigned int renderOrder) const
{
  return GetCustomCamera();
}

unsigned int vtkMRMLLayerDMPipeline::GetMaxRenderOrder() const
{
  const auto renderOrders = GetRenderOrders();
  if (renderOrders.empty())
  {
    return 0;
  }
  return *std::max_element(renderOrders.begin(), renderOrders.end());
}

unsigned int vtkMRMLLayerDMPipeline::GetVtkRendererOrder(const vtkRenderer* renderer) const
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

void vtkMRMLLayerDMPipeline::SetRenderers(const std::vector<vtkRenderer*>& renderers, const std::vector<unsigned int>& renderOrders)
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
  this->UpdateDisplay();
}

bool vtkMRMLLayerDMPipeline::RenderersMatchPipelineRenderers(const std::vector<vtkRenderer*>& renderers, const std::vector<unsigned int>& renderOrders)
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

void vtkMRMLLayerDMPipeline::SetRenderer(vtkRenderer* renderer)
{
  this->SetRenderers({ renderer }, { this->GetRenderOrder() });
}

void vtkMRMLLayerDMPipeline::SetScene(vtkMRMLScene* scene)
{
  this->m_scene = scene;
}

vtkMRMLLayerDMPipeline* vtkMRMLLayerDMPipeline::GetNodePipeline(vtkMRMLNode* node) const
{
  if (!this->m_pipelineManager)
  {
    return nullptr;
  }
  return this->m_pipelineManager->GetNodePipeline(node);
}

vtkMRMLLayerDMPipelineManager* vtkMRMLLayerDMPipeline::GetPipelineManager() const
{
  return m_pipelineManager;
}

vtkMRMLAbstractViewNode* vtkMRMLLayerDMPipeline::GetViewNode() const
{
  return this->m_viewNode;
}

bool vtkMRMLLayerDMPipeline::BlockUpdateObserver(bool isBlocked) const
{
  if (this->m_isFrozen)
  {
    return true;
  }

  return this->m_obs->SetBlocked(isBlocked);
}

bool vtkMRMLLayerDMPipeline::IsUpdateObserverBlocked() const
{
  return this->m_obs->IsBlocked();
}

void vtkMRMLLayerDMPipeline::SetFrozen(bool isFrozen)
{
  if (this->m_isFrozen == isFrozen)
  {
    return;
  }

  // Block states are only updated when the pipeline is not frozen.
  // Unfreeze to update all before saving the frozen state.
  this->m_isFrozen = false;
  this->BlockInteractionProcessing(isFrozen);
  this->BlockUpdateDisplay(isFrozen);
  this->BlockUpdateObserver(isFrozen);
  this->m_isFrozen = isFrozen;
}

bool vtkMRMLLayerDMPipeline::IsFrozen() const
{
  return this->m_isFrozen;
}

vtkMRMLNode* vtkMRMLLayerDMPipeline::GetDisplayNode() const
{
  return this->m_displayNode;
}

vtkRenderer* vtkMRMLLayerDMPipeline::GetRenderer() const
{
  if (this->m_renderersMap.empty())
  {
    return nullptr;
  }
  return this->m_renderersMap.begin()->second;
}

vtkRenderer* vtkMRMLLayerDMPipeline::GetRenderer(unsigned int renderOrder) const
{
  const auto it = this->m_renderersMap.find(renderOrder);
  if (it == std::end(this->m_renderersMap))
  {
    return nullptr;
  }
  return it->second;
}

vtkMRMLScene* vtkMRMLLayerDMPipeline::GetScene() const
{
  return this->m_scene;
}

void vtkMRMLLayerDMPipeline::OnUpdate(vtkObject* obj, unsigned long eventId, void* callData) {}

void vtkMRMLLayerDMPipeline::RemoveObserver(vtkObject* prevObj) const
{
  this->m_obs->RemoveObserver(prevObj);
}

void vtkMRMLLayerDMPipeline::RequestRender() const
{
  if (this->m_pipelineManager)
  {
    this->m_pipelineManager->RequestRender();
  }
}

void vtkMRMLLayerDMPipeline::SetPipelineManager(vtkMRMLLayerDMPipelineManager* pipelineManager)
{
  this->m_pipelineManager = pipelineManager;
}

vtkMRMLLayerDMPipeline::vtkMRMLLayerDMPipeline()
  : m_viewNode{ nullptr }
  , m_displayNode{ nullptr }
  , m_renderersMap{}
  , m_isUpdateDisplayBlocked{ false }
  , m_isFrozen{ false }
  , m_isInteractionProcessingBlocked{ false }
  , m_obs(vtkSmartPointer<vtkMRMLLayerDMObjectEventObserver>::New())
  , m_pipelineManager(nullptr)
{
  this->m_obs->SetUpdateCallback([this](vtkObject* obj, unsigned long eventId, void* callData) { this->OnUpdate(obj, eventId, callData); });
}

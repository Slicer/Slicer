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

void vtkMRMLLayerDMPipelineI::SetRenderer(vtkRenderer* renderer)
{
  if (this->m_renderer == renderer)
  {
    return;
  }

  this->OnRendererRemoved(this->m_renderer);
  this->m_renderer = renderer;
  this->OnRendererAdded(this->m_renderer);
  this->ResetDisplay();
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

vtkCamera* vtkMRMLLayerDMPipelineI::GetCustomCamera() const
{
  return nullptr;
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
  return this->m_renderer;
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
  , m_renderer{ nullptr }
  , m_isResetDisplayBlocked{ false }
  , m_isFrozen{ false }
  , m_isInteractionProcessingBlocked{ false }
  , m_obs(vtkSmartPointer<vtkMRMLLayerDMObjectEventObserver>::New())
  , m_pipelineManager(nullptr)
{
  this->m_obs->SetUpdateCallback([this](vtkObject* obj, unsigned long eventId, void* callData) { this->OnUpdate(obj, eventId, callData); });
}

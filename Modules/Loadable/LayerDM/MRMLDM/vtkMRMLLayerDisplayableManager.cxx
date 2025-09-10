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
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(vtkMRMLLayerDisplayableManager);

vtkMRMLLayerDisplayableManager::vtkMRMLLayerDisplayableManager()
  : m_pipelineManager(nullptr)
{
}

bool vtkMRMLLayerDisplayableManager::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)
{
  if (!this->m_pipelineManager)
  {
    return false;
  }

  return this->m_pipelineManager->CanProcessInteractionEvent(eventData, distance2);
}

bool vtkMRMLLayerDisplayableManager::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  if (!this->m_pipelineManager)
  {
    return false;
  }

  return this->m_pipelineManager->ProcessInteractionEvent(eventData);
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

void vtkMRMLLayerDisplayableManager::OnMRMLSceneEndClose()
{
  this->UpdateFromMRML();
}

void vtkMRMLLayerDisplayableManager::OnMRMLSceneEndBatchProcess()
{
  this->UpdateFromMRML();
}

void vtkMRMLLayerDisplayableManager::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (this->GetMRMLScene()->IsBatchProcessing() || !this->m_pipelineManager)
  {
    return;
  }

  if (this->m_pipelineManager->AddNode(node))
  {
    this->RequestRender();
  }
}

void vtkMRMLLayerDisplayableManager::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (this->GetMRMLScene()->IsBatchProcessing() || !this->m_pipelineManager)
  {
    return;
  }

  if (this->m_pipelineManager->RemoveNode(node))
  {
    this->RequestRender();
  }
}

void vtkMRMLLayerDisplayableManager::UnobserveMRMLScene()
{
  if (!this->m_pipelineManager)
  {
    return;
  }
  this->m_pipelineManager->ClearDisplayableNodes();
}

void vtkMRMLLayerDisplayableManager::UpdateFromMRML()
{
  this->SetUpdateFromMRMLRequested(false);

  if (!this->m_pipelineManager)
  {
    return;
  }
  this->m_pipelineManager->SetScene(this->GetMRMLScene());
  this->m_pipelineManager->UpdateFromScene();
}

void vtkMRMLLayerDisplayableManager::OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller)
{
  auto viewNode = vtkMRMLAbstractViewNode::SafeDownCast(caller);
  if (!viewNode || !this->m_pipelineManager)
  {
    return;
  }

  this->m_pipelineManager->SetViewNode(viewNode);
}

int vtkMRMLLayerDisplayableManager::GetMouseCursor()
{
  if (!this->m_pipelineManager)
  {
    return vtkMRMLAbstractDisplayableManager::GetMouseCursor();
  }
  return this->m_pipelineManager->GetMouseCursor();
}

void vtkMRMLLayerDisplayableManager::Create()
{
  vtkRenderer* renderer = this->GetRenderer();
  if (!renderer || !renderer->GetRenderWindow())
  {
    vtkErrorMacro("vtkMRMLLayerDisplayableManager::Create() failed: renderer is invalid");
    return;
  }

  if (!this->m_pipelineManager)
  {
    this->m_pipelineManager = vtkSmartPointer<vtkMRMLLayerDMPipelineManager>::New();
  }

  this->m_pipelineManager->SetRenderWindow(renderer->GetRenderWindow());
  this->m_pipelineManager->SetRenderer(renderer);
  this->m_pipelineManager->SetFactory(vtkMRMLLayerDMPipelineFactory::GetInstance());
  this->m_pipelineManager->SetScene(this->GetMRMLScene());
  this->m_pipelineManager->SetViewNode(vtkMRMLAbstractViewNode::SafeDownCast(this->GetMRMLDisplayableNode()));
  this->m_pipelineManager->SetRequestRender([this] { this->RequestRender(); });

  // Make sure the DM is up to date with the current scene state
  this->UpdateFromMRML();
}

void vtkMRMLLayerDisplayableManager::SetHasFocus(bool hasFocus, vtkMRMLInteractionEventData* eventData)
{
  Superclass ::SetHasFocus(hasFocus, eventData);
  if (this->m_pipelineManager && !hasFocus)
  {
    this->m_pipelineManager->LoseFocus(eventData);
  }
}

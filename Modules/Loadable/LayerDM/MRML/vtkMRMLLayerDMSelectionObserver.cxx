// LayerDM includes
#include "vtkMRMLLayerDMObjectEventObserver.h"
#include "vtkMRMLLayerDMSelectionObserver.h"

// Slicer includes
#include <vtkMRMLScene.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLApplicationLogic.h>

// VTK includes
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkMRMLLayerDMSelectionObserver);

vtkMRMLLayerDMSelectionObserver::vtkMRMLLayerDMSelectionObserver()
  : m_obs{ vtkSmartPointer<vtkMRMLLayerDMObjectEventObserver>::New() }
{
  m_obs->SetUpdateCallback(
    [this](vtkObject* obj)
    {
      if (obj == this->m_interactionNode || obj == this->m_selectionNode)
      {
        this->Modified();
      }
    });
}

vtkMRMLLayerDMSelectionObserver::~vtkMRMLLayerDMSelectionObserver() = default;

void vtkMRMLLayerDMSelectionObserver::SetScene(vtkMRMLScene* scene)
{
  this->UpdateNodesFromScene(scene);
}

void vtkMRMLLayerDMSelectionObserver::UpdateNodesFromScene(vtkMRMLScene* scene)
{
  bool didModify{};
  didModify |= this->SetInteractionNode(vtkMRMLInteractionNode::SafeDownCast(scene ? scene->GetNodeByID("vtkMRMLInteractionNodeSingleton") : nullptr));
  didModify |= this->SetSelectionNode(vtkMRMLSelectionNode::SafeDownCast(scene ? scene->GetNodeByID("vtkMRMLSelectionNodeSingleton") : nullptr));
  if (didModify)
  {
    this->Modified();
  }
}

void vtkMRMLLayerDMSelectionObserver::UpdateNodesFromApplicationLogic(vtkMRMLApplicationLogic* logic)
{
  bool didModify{};
  didModify |= this->SetInteractionNode(logic ? logic->GetInteractionNode() : nullptr);
  didModify |= this->SetSelectionNode(logic ? logic->GetSelectionNode() : nullptr);
  if (didModify)
  {
    this->Modified();
  }
}

bool vtkMRMLLayerDMSelectionObserver::SetInteractionNode(vtkMRMLInteractionNode* interactionNode)
{
  const auto didModify = this->m_obs->UpdateObserver(m_interactionNode, interactionNode);
  this->m_interactionNode = interactionNode;
  return didModify;
}

vtkMRMLInteractionNode* vtkMRMLLayerDMSelectionObserver::GetInteractionNode() const
{
  return this->m_interactionNode;
}

bool vtkMRMLLayerDMSelectionObserver::SetSelectionNode(vtkMRMLSelectionNode* selectionNode)
{
  const auto didModify = this->m_obs->UpdateObserver(m_selectionNode, selectionNode);
  this->m_selectionNode = selectionNode;
  return didModify;
}

vtkMRMLSelectionNode* vtkMRMLLayerDMSelectionObserver::GetSelectionNode() const
{
  return this->m_selectionNode;
}

bool vtkMRMLLayerDMSelectionObserver::IsPlacing(vtkMRMLNode* node) const
{
  if (!node)
  {
    return false;
  }

  return this->IsPlacing() && (this->GetActivePlaceNodeID() == std::string(node->GetID()));
}

bool vtkMRMLLayerDMSelectionObserver::IsPlacing() const
{
  if (!this->m_interactionNode)
  {
    return false;
  }

  return this->m_interactionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place;
}

void vtkMRMLLayerDMSelectionObserver::StartPlace(vtkMRMLNode* node, bool isPersistent)
{
  if (!node || !this->m_interactionNode || !this->m_selectionNode)
  {
    return;
  }

  {
    vtkMRMLLayerDMObjectEventObserver::UpdateGuard guard(m_obs);
    this->m_selectionNode->SetActivePlaceNodeID(node->GetID());
    this->m_interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::Place);
    this->m_interactionNode->SetPlaceModePersistence(isPersistent);
  }
  this->Modified();
}

void vtkMRMLLayerDMSelectionObserver::StopPlace() const
{
  this->SetInteractionMode(vtkMRMLInteractionNode::ViewTransform);
}

std::string vtkMRMLLayerDMSelectionObserver::GetActivePlaceNodeID() const
{
  if (!this->m_selectionNode || !this->m_selectionNode->GetActivePlaceNodeID())
  {
    return "";
  }
  return this->m_selectionNode->GetActivePlaceNodeID();
}

void vtkMRMLLayerDMSelectionObserver::SetInteractionMode(int interactionMode) const
{
  if (!m_interactionNode)
  {
    return;
  }
  m_interactionNode->SetCurrentInteractionMode(interactionMode);
}

int vtkMRMLLayerDMSelectionObserver::GetCurrentInteractionMode() const
{
  if (!this->m_interactionNode)
  {
    return 0;
  }
  return this->m_interactionNode->GetCurrentInteractionMode();
}

bool vtkMRMLLayerDMSelectionObserver::GetPlaceModePersistence() const
{
  if (!m_interactionNode)
  {
    return false;
  }
  return m_interactionNode->GetPlaceModePersistence();
}

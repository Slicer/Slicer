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
  : Observer{ vtkSmartPointer<vtkMRMLLayerDMObjectEventObserver>::New() }
{
  this->Observer->SetUpdateCallback(
    [this](vtkObject* obj)
    {
      if (obj == this->InteractionNode || obj == this->SelectionNode)
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
  const auto didModify = this->Observer->UpdateObserver(this->InteractionNode, interactionNode);
  this->InteractionNode = interactionNode;
  return didModify;
}

vtkMRMLInteractionNode* vtkMRMLLayerDMSelectionObserver::GetInteractionNode() const
{
  return this->InteractionNode;
}

bool vtkMRMLLayerDMSelectionObserver::SetSelectionNode(vtkMRMLSelectionNode* selectionNode)
{
  const auto didModify = this->Observer->UpdateObserver(this->SelectionNode, selectionNode);
  this->SelectionNode = selectionNode;
  return didModify;
}

vtkMRMLSelectionNode* vtkMRMLLayerDMSelectionObserver::GetSelectionNode() const
{
  return this->SelectionNode;
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
  if (!this->InteractionNode)
  {
    return false;
  }

  return this->InteractionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place;
}

void vtkMRMLLayerDMSelectionObserver::StartPlace(vtkMRMLNode* node, bool isPersistent)
{
  if (!node || !this->InteractionNode || !this->SelectionNode)
  {
    return;
  }

  // Avoid triggering modified if the node is already in place mode
  if (IsPlacing(node))
  {
    this->InteractionNode->SetPlaceModePersistence(isPersistent);
    return;
  }

  {
    vtkMRMLLayerDMObjectEventObserver::UpdateGuard guard(this->Observer);
    this->SelectionNode->SetActivePlaceNodeClassName(node->GetClassName());
    this->SelectionNode->SetActivePlaceNodeID(node->GetID());
    this->InteractionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::Place);
    this->InteractionNode->SetPlaceModePersistence(isPersistent);
  }
  this->Modified();
}

void vtkMRMLLayerDMSelectionObserver::StopPlace() const
{
  this->SetInteractionMode(vtkMRMLInteractionNode::ViewTransform);
}

std::string vtkMRMLLayerDMSelectionObserver::GetActivePlaceNodeID() const
{
  if (!this->SelectionNode || !this->SelectionNode->GetActivePlaceNodeID())
  {
    return "";
  }
  return this->SelectionNode->GetActivePlaceNodeID();
}

void vtkMRMLLayerDMSelectionObserver::SetInteractionMode(int interactionMode) const
{
  if (!this->InteractionNode)
  {
    return;
  }
  this->InteractionNode->SetCurrentInteractionMode(interactionMode);
}

int vtkMRMLLayerDMSelectionObserver::GetCurrentInteractionMode() const
{
  if (!this->InteractionNode)
  {
    return 0;
  }
  return this->InteractionNode->GetCurrentInteractionMode();
}

bool vtkMRMLLayerDMSelectionObserver::GetPlaceModePersistence() const
{
  if (!this->InteractionNode)
  {
    return false;
  }
  return this->InteractionNode->GetPlaceModePersistence();
}

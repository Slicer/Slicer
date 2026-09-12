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

#include "vtkMRMLLayerDMNodeReferenceObserver.h"

// LayerDM includes
#include "vtkMRMLLayerDMObjectEventObserver.h"

// Slicer includes
#include <vtkCollection.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// STD includes
#include <algorithm>

namespace
{
//-----------------------------------------------------------------------------
/// Simple class to expose the content of the vtkMRMLNodeReference ToNode / Role.
/// Not meant to be used in the scene nor exposed externally.
class vtkMRMLNodeReferenceFacade : public vtkMRMLNode
{
public:
  static vtkMRMLNodeReferenceFacade* New();
  vtkTypeMacro(vtkMRMLNodeReferenceFacade, vtkMRMLNode);

  const char* GetNodeTagName() override { return "NodeRefFacade"; }
  vtkMRMLNode* CreateNodeInstance() override;

  static std::tuple<vtkMRMLNode*, std::string> GetToNodeAndRoleFromTypeErasedNodeRef(void* callData)
  {
    const auto ref = CastCallData(callData);
    return { ref->GetReferencedNode(), ref->GetReferenceRole() };
  }

protected:
  vtkMRMLNodeReferenceFacade() = default;
  ~vtkMRMLNodeReferenceFacade() override = default;

private:
  vtkMRMLNodeReferenceFacade(const vtkMRMLNodeReferenceFacade&);
  void operator=(const vtkMRMLNodeReferenceFacade&);

  static vtkMRMLNodeReference* CastCallData(void* callData) { return static_cast<vtkMRMLNodeReference*>(callData); }
};
//-----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLNodeReferenceFacade);
} // namespace

namespace
{
//-----------------------------------------------------------------------------
/// Add the reference to the list if it is not already there.
/// The reference lists are unordered, so uniqueness is not provided by the container itself.
void InsertUniqueRef(std::vector<vtkMRMLLayerDMNodeReferenceObserver::RefT>& refs, const vtkMRMLLayerDMNodeReferenceObserver::RefT& ref)
{
  if (std::find(refs.begin(), refs.end(), ref) == refs.end())
  {
    refs.emplace_back(ref);
  }
}
} // namespace

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLLayerDMNodeReferenceObserver);

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMNodeReferenceObserver::SetReferenceModifiedCallBack(const CallBackT& modifiedCallback)
{
  this->ReferenceModifiedCallback = modifiedCallback;
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMNodeReferenceObserver::SetScene(vtkMRMLScene* scene)
{
  if (this->Scene == scene)
  {
    return;
  }

  this->Observer->UpdateObservation(this->Scene, scene, { vtkMRMLScene::NodeAddedEvent, vtkMRMLScene::NodeRemovedEvent });
  this->Scene = scene;
  this->UpdateFromScene();
}

//-----------------------------------------------------------------------------
vtkMRMLLayerDMNodeReferenceObserver::vtkMRMLLayerDMNodeReferenceObserver()
  : Observer(vtkSmartPointer<vtkMRMLLayerDMObjectEventObserver>::New())
{
  this->Observer->SetUpdateCallback(
    [this](vtkObject* obj, unsigned long eventId, void* callData)
    {
      if (obj == this->Scene)
      {
        switch (eventId)
        {
          case vtkMRMLScene::NodeAddedEvent: this->OnNodeAdded(static_cast<vtkMRMLNode*>(callData)); break;
          case vtkMRMLScene::NodeRemovedEvent: this->OnNodeRemoved(static_cast<vtkMRMLNode*>(callData)); break;
          default: break;
        }
      }
      else
      {
        auto fromNode = vtkMRMLNode::SafeDownCast(obj);
        if (!fromNode)
        {
          return;
        }

        const auto [toNode, role] = vtkMRMLNodeReferenceFacade::GetToNodeAndRoleFromTypeErasedNodeRef(callData);
        switch (eventId)
        {
          case vtkMRMLNode::ReferenceAddedEvent: this->OnReferenceAdded(fromNode, toNode, role); break;
          case vtkMRMLNode::ReferenceRemovedEvent: this->OnReferenceRemoved(fromNode, toNode, role); break;
          case vtkMRMLNode::ReferenceModifiedEvent: this->OnReferenceModified(fromNode, toNode, role); break;
          default: break;
        }
      }
    });

  // No delete callback is needed here: Nodes and the reference maps hold their nodes with owning pointers, so
  // an observed node cannot be destroyed while it is tracked. Cleaning up from a delete callback would in fact
  // be unsafe, as looking a node up in those containers builds a temporary owning pointer, which would
  // resurrect and destroy the node a second time while it is being destroyed.
}

//-----------------------------------------------------------------------------
vtkMRMLLayerDMNodeReferenceObserver::~vtkMRMLLayerDMNodeReferenceObserver()
{
  // Releasing the observed nodes below destroys them, which would otherwise invoke the delete callback and
  // re-enter this object while its members are being destroyed.
  this->Observer->ClearCallbacks();
}

namespace
{
//-----------------------------------------------------------------------------
std::set<vtkSmartPointer<vtkMRMLNode>> GetSceneNodes(vtkMRMLScene* scene)
{
  if (!scene)
  {
    return {};
  }

  // Traverse the collection with an iterator: vtkCollection::GetItemAsObject walks the collection from its
  // first item on every call, which makes an indexed scan quadratic in the number of nodes.
  std::set<vtkSmartPointer<vtkMRMLNode>> nodes;
  vtkObject* item = nullptr;
  vtkCollectionSimpleIterator it;
  for (scene->GetNodes()->InitTraversal(it); (item = scene->GetNodes()->GetNextItemAsObject(it));)
  {
    if (auto node = vtkMRMLNode::SafeDownCast(item))
    {
      nodes.insert(node);
    }
  }
  return nodes;
}

//-----------------------------------------------------------------------------
std::tuple<std::vector<vtkSmartPointer<vtkMRMLNode>>, std::vector<vtkSmartPointer<vtkMRMLNode>>> GetNodesRemovedAddedFromScene(
  vtkMRMLScene* scene,
  const std::set<vtkSmartPointer<vtkMRMLNode>>& currentNodes)
{
  auto sceneNodes = GetSceneNodes(scene);
  std::vector<vtkSmartPointer<vtkMRMLNode>> nodesRemoved, nodesAdded;

  for (const auto& sceneNode : sceneNodes)
  {
    if (currentNodes.find(sceneNode) == currentNodes.end())
    {
      nodesAdded.emplace_back(sceneNode);
    }
  }

  for (const auto& currentNode : currentNodes)
  {
    if (sceneNodes.find(currentNode) == sceneNodes.end())
    {
      nodesRemoved.emplace_back(currentNode);
    }
  }

  return { nodesRemoved, nodesAdded };
}
} // namespace

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMNodeReferenceObserver::UpdateFromScene()
{
  auto [nodesRemoved, nodesAdded] = GetNodesRemovedAddedFromScene(this->Scene, this->Nodes);
  for (const auto& node : nodesRemoved)
  {
    this->OnNodeRemoved(node);
  }
  for (const auto& node : nodesAdded)
  {
    this->OnNodeAdded(node);
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMNodeReferenceObserver::OnNodeRemoved(vtkMRMLNode* node)
{
  auto eraseKeyInMap = [&](std::map<vtkSmartPointer<vtkMRMLNode>, std::vector<RefT>>& map, vtkMRMLNode* keyNode)
  {
    if (map.find(keyNode) == map.end())
    {
      return;
    }
    map.erase(keyNode);
  };

  // Notify all nodes that references was removed
  for (const auto& [toNode, role] : this->GetNodeToReferences(node))
  {
    this->OnReferenceRemoved(node, toNode, role);
  }

  // Remove any observer on the node
  this->Observer->RemoveObservations(node);

  // Erase the node from the different maps to avoid any dangling pointers
  this->Nodes.erase(node);
  eraseKeyInMap(this->NodeFromReferences, node);
  eraseKeyInMap(this->NodeToReferences, node);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMNodeReferenceObserver::OnNodeAdded(vtkMRMLNode* node)
{
  this->Nodes.insert(node);
  this->Observer->UpdateObservation(nullptr, node, { vtkMRMLNode::ReferenceAddedEvent, vtkMRMLNode::ReferenceModifiedEvent, vtkMRMLNode::ReferenceRemovedEvent });
  for (const auto& [toNode, role] : GetNodeReferencesFromScene(node))
  {
    this->OnReferenceAdded(node, toNode, role);
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMNodeReferenceObserver::OnReferenceAdded(vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role)
{
  InsertUniqueRef(this->NodeToReferences[fromNode], { toNode, role });
  InsertUniqueRef(this->NodeFromReferences[toNode], { fromNode, role });
  this->TriggerReferenceAdded(fromNode, toNode, role);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMNodeReferenceObserver::OnReferenceRemoved(vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role)
{
  auto eraseRefInMap = [&](std::map<vtkSmartPointer<vtkMRMLNode>, std::vector<RefT>>& map, vtkMRMLNode* keyNode, vtkMRMLNode* valueNode)
  {
    if (map.find(keyNode) == map.end())
    {
      return;
    }
    auto& refs = map[keyNode];
    refs.erase(std::remove(refs.begin(), refs.end(), RefT{ valueNode, role }), refs.end());
    if (map[keyNode].empty())
    {
      map.erase(keyNode);
    }
  };

  eraseRefInMap(this->NodeToReferences, fromNode, toNode);
  eraseRefInMap(this->NodeFromReferences, toNode, fromNode);
  this->TriggerReferenceRemoved(fromNode, toNode, role);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMNodeReferenceObserver::RemoveOutdatedReferences(vtkMRMLNode* fromNode)
{
  auto sceneRefs = GetNodeReferencesFromScene(fromNode);
  for (const auto& ref : this->GetNodeToReferences(fromNode))
  {
    if (std::find(sceneRefs.begin(), sceneRefs.end(), ref) == sceneRefs.end())
    {
      this->OnReferenceRemoved(fromNode, std::get<0>(ref), std::get<1>(ref));
    }
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMNodeReferenceObserver::OnReferenceModified(vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role)
{
  this->RemoveOutdatedReferences(fromNode);
  this->OnReferenceAdded(fromNode, toNode, role);
}

//-----------------------------------------------------------------------------
std::vector<vtkMRMLLayerDMNodeReferenceObserver::RefT> vtkMRMLLayerDMNodeReferenceObserver::GetNodeToReferences(vtkMRMLNode* node) const
{
  if (const auto it = this->NodeToReferences.find(node); it != this->NodeToReferences.end())
  {
    return it->second;
  }
  return {};
}

//-----------------------------------------------------------------------------
std::vector<vtkMRMLLayerDMNodeReferenceObserver::RefT> vtkMRMLLayerDMNodeReferenceObserver::GetNodeFromReferences(vtkMRMLNode* node) const
{
  if (const auto it = this->NodeFromReferences.find(node); it != this->NodeFromReferences.end())
  {
    return it->second;
  }
  return {};
}

//-----------------------------------------------------------------------------
int vtkMRMLLayerDMNodeReferenceObserver::GetReferenceToSize() const
{
  return static_cast<int>(this->NodeToReferences.size());
}

//-----------------------------------------------------------------------------
int vtkMRMLLayerDMNodeReferenceObserver::GetReferenceFromSize() const
{
  return static_cast<int>(this->NodeFromReferences.size());
}

//-----------------------------------------------------------------------------
int vtkMRMLLayerDMNodeReferenceObserver::GetNumberOfNodes() const
{
  return static_cast<int>(this->Nodes.size());
}

//-----------------------------------------------------------------------------
std::vector<vtkMRMLLayerDMNodeReferenceObserver::RefT> vtkMRMLLayerDMNodeReferenceObserver::GetNodeReferencesFromScene(vtkMRMLNode* node)
{
  if (!node)
  {
    return {};
  }
  std::vector<RefT> references;
  std::vector<std::string> roles;
  node->GetNodeReferenceRoles(roles);
  for (const auto& role : roles)
  {
    for (int iNode = 0; iNode < node->GetNumberOfNodeReferences(role.c_str()); iNode++)
    {
      auto toNode = node->GetNthNodeReference(role.c_str(), iNode);
      InsertUniqueRef(references, { toNode, role });
    }
  }
  return references;
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMNodeReferenceObserver::TriggerReferenceAdded(vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role) const
{
  TriggerCallback(this->ReferenceModifiedCallback, fromNode, toNode, role, ReferenceAddedEvent);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMNodeReferenceObserver::TriggerReferenceRemoved(vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role) const
{
  TriggerCallback(this->ReferenceModifiedCallback, fromNode, toNode, role, ReferenceRemovedEvent);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMNodeReferenceObserver::TriggerCallback(const CallBackT& callback, vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role, int eventType)
{
  if (!callback || !fromNode || !toNode)
  {
    return;
  }
  callback(fromNode, toNode, role, eventType);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMNodeReferenceObserver::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Scene: " << (this->Scene ? "set" : "(none)") << std::endl;
  os << indent << "Number of observed nodes: " << this->Nodes.size() << std::endl;
  os << indent << "Number of nodes with outgoing references: " << this->NodeToReferences.size() << std::endl;
  os << indent << "Number of nodes with incoming references: " << this->NodeFromReferences.size() << std::endl;
  os << indent << "Reference modified callback: " << (this->ReferenceModifiedCallback ? "set" : "(none)") << std::endl;
}

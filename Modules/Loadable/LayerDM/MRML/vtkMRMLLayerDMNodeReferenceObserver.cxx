#include "vtkMRMLLayerDMNodeReferenceObserver.h"

// LayerDM includes
#include "vtkMRMLLayerDMObjectEventObserver.h"

// Slicer includes
#include <vtkCollection.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

namespace
{
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
vtkMRMLNodeNewMacro(vtkMRMLNodeReferenceFacade);
} // namespace

vtkStandardNewMacro(vtkMRMLLayerDMNodeReferenceObserver);

void vtkMRMLLayerDMNodeReferenceObserver::SetReferenceModifiedCallBack(const CallBackT& modifiedCallback)
{
  m_onRefModified = modifiedCallback;
}

void vtkMRMLLayerDMNodeReferenceObserver::SetScene(vtkMRMLScene* scene)
{
  if (m_scene == scene)
  {
    return;
  }

  this->m_obs->UpdateObserver(m_scene, scene, { vtkMRMLScene::NodeAddedEvent, vtkMRMLScene::NodeRemovedEvent });
  this->m_scene = scene;
  this->UpdateFromScene();
}

vtkMRMLLayerDMNodeReferenceObserver::vtkMRMLLayerDMNodeReferenceObserver()
  : m_obs(vtkSmartPointer<vtkMRMLLayerDMObjectEventObserver>::New())
{
  m_obs->SetUpdateCallback(
    [this](vtkObject* obj, unsigned long eventId, void* callData)
    {
      if (obj == this->m_scene)
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
}

inline std::set<vtkSmartPointer<vtkMRMLNode>> GetSceneNodes(vtkMRMLScene* scene)
{
  if (!scene)
  {
    return {};
  }

  std::set<vtkSmartPointer<vtkMRMLNode>> nodes;
  for (int iNode = 0; iNode < scene->GetNumberOfNodes(); iNode++)
  {
    auto node = vtkMRMLNode::SafeDownCast(scene->GetNodes()->GetItemAsObject(iNode));
    if (node)
    {
      nodes.insert(node);
    }
  }
  return nodes;
}

inline std::tuple<std::vector<vtkSmartPointer<vtkMRMLNode>>, std::vector<vtkSmartPointer<vtkMRMLNode>>> GetNodesRemovedAddedFromScene(
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

void vtkMRMLLayerDMNodeReferenceObserver::UpdateFromScene()
{
  auto [nodesRemoved, nodesAdded] = GetNodesRemovedAddedFromScene(this->m_scene, this->m_nodes);
  for (const auto& node : nodesRemoved)
  {
    this->OnNodeRemoved(node);
  }
  for (const auto& node : nodesAdded)
  {
    this->OnNodeAdded(node);
  }
}

void vtkMRMLLayerDMNodeReferenceObserver::OnNodeRemoved(vtkMRMLNode* node)
{
  auto eraseKeyInMap = [&](std::map<vtkSmartPointer<vtkMRMLNode>, std::set<RefT>>& map, vtkMRMLNode* keyNode)
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
  m_obs->RemoveObserver(node);

  // Erase the node from the different maps to avoid any dangling pointers
  m_nodes.erase(node);
  eraseKeyInMap(m_refFrom, node);
  eraseKeyInMap(m_refTo, node);
}

void vtkMRMLLayerDMNodeReferenceObserver::OnNodeAdded(vtkMRMLNode* node)
{
  m_nodes.insert(node);
  m_obs->UpdateObserver(nullptr, node, { vtkMRMLNode::ReferenceAddedEvent, vtkMRMLNode::ReferenceModifiedEvent, vtkMRMLNode::ReferenceRemovedEvent });
  for (const auto& [toNode, role] : GetNodeReferencesFromScene(node))
  {
    this->OnReferenceAdded(node, toNode, role);
  }
}

void vtkMRMLLayerDMNodeReferenceObserver::OnReferenceAdded(vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role)
{
  m_refTo[fromNode].insert({ toNode, role });
  m_refFrom[toNode].insert({ fromNode, role });
  this->TriggerReferenceAdded(fromNode, toNode, role);
}

void vtkMRMLLayerDMNodeReferenceObserver::OnReferenceRemoved(vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role)
{
  auto eraseRefInMap = [&](std::map<vtkSmartPointer<vtkMRMLNode>, std::set<RefT>>& map, vtkMRMLNode* keyNode, vtkMRMLNode* valueNode)
  {
    if (map.find(keyNode) == map.end())
    {
      return;
    }
    map[keyNode].erase({ valueNode, role });
    if (map[keyNode].empty())
    {
      map.erase(keyNode);
    }
  };

  eraseRefInMap(m_refTo, fromNode, toNode);
  eraseRefInMap(m_refFrom, toNode, fromNode);
  this->TriggerReferenceRemoved(fromNode, toNode, role);
}

void vtkMRMLLayerDMNodeReferenceObserver::RemoveOutdatedReferences(vtkMRMLNode* fromNode)
{
  auto sceneRefs = GetNodeReferencesFromScene(fromNode);
  for (const auto& ref : GetNodeToReferences(fromNode))
  {
    if (sceneRefs.find(ref) == sceneRefs.end())
    {
      this->OnReferenceRemoved(fromNode, std::get<0>(ref), std::get<1>(ref));
    }
  }
}

void vtkMRMLLayerDMNodeReferenceObserver::OnReferenceModified(vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role)
{
  this->RemoveOutdatedReferences(fromNode);
  this->OnReferenceAdded(fromNode, toNode, role);
}

std::set<vtkMRMLLayerDMNodeReferenceObserver::RefT> vtkMRMLLayerDMNodeReferenceObserver::GetNodeToReferences(vtkMRMLNode* node) const
{
  if (const auto it = m_refTo.find(node); it != m_refTo.end())
  {
    return it->second;
  }
  return {};
}

std::set<vtkMRMLLayerDMNodeReferenceObserver::RefT> vtkMRMLLayerDMNodeReferenceObserver::GetNodeFromReferences(vtkMRMLNode* node) const
{
  if (const auto it = m_refFrom.find(node); it != m_refFrom.end())
  {
    return it->second;
  }
  return {};
}

int vtkMRMLLayerDMNodeReferenceObserver::GetReferenceToSize() const
{
  return static_cast<int>(m_refTo.size());
}

int vtkMRMLLayerDMNodeReferenceObserver::GetReferenceFromSize() const
{
  return static_cast<int>(m_refFrom.size());
}

int vtkMRMLLayerDMNodeReferenceObserver::GetNumberOfNodes() const
{
  return static_cast<int>(m_nodes.size());
}

std::set<vtkMRMLLayerDMNodeReferenceObserver::RefT> vtkMRMLLayerDMNodeReferenceObserver::GetNodeReferencesFromScene(vtkMRMLNode* node)
{
  if (!node)
  {
    return {};
  }
  std::set<RefT> references;
  std::vector<std::string> roles;
  node->GetNodeReferenceRoles(roles);
  for (const auto& role : roles)
  {
    for (int iNode = 0; iNode < node->GetNumberOfNodeReferences(role.c_str()); iNode++)
    {
      auto toNode = node->GetNthNodeReference(role.c_str(), iNode);
      references.insert({ toNode, role });
    }
  }
  return references;
}

void vtkMRMLLayerDMNodeReferenceObserver::TriggerReferenceAdded(vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role) const
{
  TriggerCallback(m_onRefModified, fromNode, toNode, role, ReferenceAddedEvent);
}

void vtkMRMLLayerDMNodeReferenceObserver::TriggerReferenceRemoved(vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role) const
{
  TriggerCallback(m_onRefModified, fromNode, toNode, role, ReferenceRemovedEvent);
}

void vtkMRMLLayerDMNodeReferenceObserver::TriggerCallback(const CallBackT& callback, vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role, int eventType)
{
  if (!callback || !fromNode || !toNode)
  {
    return;
  }
  callback(fromNode, toNode, role, eventType);
}

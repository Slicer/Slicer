#pragma once

#include "vtkSlicerLayerDMModuleLogicExport.h"

// Slicer includes
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLScene.h>
#include <vtkSlicerModuleLogic.h>

class vtkMRMLLayerDMWidgetEventTranslationNode;

/// Logic class for the LayerDM module.
/// Provides helper templates to simplify registering and accessing custom display nodes.
class VTK_SLICER_LAYERDM_MODULE_LOGIC_EXPORT vtkSlicerLayerDMLogic : public vtkSlicerModuleLogic
{
public:
  static vtkSlicerLayerDMLogic* New();
  vtkTypeMacro(vtkSlicerLayerDMLogic, vtkSlicerModuleLogic);

  vtkSlicerLayerDMLogic(const vtkSlicerLayerDMLogic&) = delete;
  void operator=(const vtkSlicerLayerDMLogic&) = delete;

  /// Registers the LayerDM MRML nodes in the scene
  void RegisterNodes() override;

  /// Attach a translation node ref to the input node
  static void SetWidgetEventTranslationNode(vtkMRMLNode* node, vtkMRMLLayerDMWidgetEventTranslationNode* translationNode);

  /// \return the widget event translation node attached to the input node.
  static vtkMRMLLayerDMWidgetEventTranslationNode* GetWidgetEventTranslationNode(vtkMRMLNode* node);

  /// Get the singleton translation node with given ID if it exists in the scene.
  static vtkMRMLLayerDMWidgetEventTranslationNode* GetWidgetEventTranslationSingleton(vtkMRMLScene* scene, const std::string& singletonId);

  /// Get the singleton translation node with given ID if it exists in the scene.
  /// If it doesn't exist in the scene, creates the singleton and configures the singleton given the input configure function.
  static vtkMRMLLayerDMWidgetEventTranslationNode* GetWidgetEventTranslationSingleton(vtkMRMLScene* scene,
                                                                                      const std::string& singletonId,
                                                                                      const std::function<void(vtkMRMLLayerDMWidgetEventTranslationNode*)>& configureF);

  /// Create a singleton translation node with given ID.
  ///
  /// By default, the created singleton is set to not be saved with the scene to avoid changing interaction events when
  /// loading scenes coming from other users.
  static vtkMRMLLayerDMWidgetEventTranslationNode* CreateWidgetEventTranslationSingleton(vtkMRMLScene* scene, const std::string& singletonId);

  /// If the input node doesn't have any translation node, sets the singleton TL node as TL node.
  /// If the singleton doesn't yet exist in the scene, creates and initializes the TL node singleton given the input configure method.
  ///
  /// If the node already has a TL node, does nothing.
  /// If the node doesn't have a TL node and isn't set in the Scene, does nothing.
  static void CreateDefaultEventTranslation(vtkMRMLNode* node, const std::string& singletonId, const std::function<void(vtkMRMLLayerDMWidgetEventTranslationNode*)>& configureF);

  /// \return the role associated with widget event translation
  static std::string GetEventTranslationRole();

  /// \return the role associated with display nodes (equivalent to vtkMRMLDisplayableNode::GetDisplayRole but non-virtual and static)
  static std::string GetDisplayRole();

  /// Register node if the node isn't already registered in the scene.
  /// Otherwise, registering multiple time the same node will overwrite the previous node and generate a warning.
  template <typename T>
  static void RegisterNodeIfNeeded(vtkMRMLScene* scene);

  /// \return the first display node attached to the input displayable node with given type
  template <typename T>
  static T* GetDisplayNode(vtkMRMLNode* node);

  /// \return the first reference node of given type attached to the input node with given role.
  template <typename T>
  static T* GetReferenceNode(vtkMRMLNode* node, const char* role);

  /// \return the first reference node of given type attached to the input node with given role.
  /// Expects T to have a static T::GetReferenceRole method returning its reference role.
  template <typename T>
  static T* GetReferenceNode(vtkMRMLNode* node);

  /// \return all the reference nodes of given type attached to the input node with given role.
  template <typename T>
  static std::vector<T*> GetReferenceNodes(vtkMRMLNode* node, const char* role);

  /// \return all the reference nodes of given type attached to the input node with given role.
  /// Expects T to have a static T::GetReferenceRole method returning its reference role.
  template <typename T>
  static std::vector<T*> GetReferenceNodes(vtkMRMLNode* node);

  /// Creates a display node of given type and attach it to the input node
  ///
  /// \param node: instance of node for which the display node will be created. If the node is nullptr or not attached
  ///   to a scene, will not create any node.
  /// \param allowMultiple: when false, will return the existing display node if a display node of given type / role is
  ///   already attached to the input node.
  template <typename T>
  static T* CreateDisplayNode(vtkMRMLNode* node, bool allowMultiple = false);

  /// Creates a reference node of given type and role and attach it to the input node.
  /// The DisplayNode's modified event will be triggerred once the reference is set.
  ///
  /// \param node: instance of node for which the display node will be created. If the node is nullptr or not attached
  ///   to a scene, will not create any node.
  /// \param role: name of the role for the created and attached reference node. Role can be any arbitrary string.
  /// \param allowMultiple: when false, will return the existing ref node if a ref node of given type / role is
  ///   already attached to the input node.
  template <typename T>
  static T* CreateReferenceNode(vtkMRMLNode* node, const char* role, bool allowMultiple = false);

  /// Creates a reference node of given type and role and attach it to the input node.
  /// The DisplayNode's modified event will be triggerred once the reference is set.
  ///
  /// \param node: instance of node for which the display node will be created. If the node is nullptr or not attached
  ///   to a scene, will not create any node.
  /// \param allowMultiple: when false, will return the existing ref node if a ref node of given type / role is
  ///   already attached to the input node.
  ///
  /// Expects T to have a static T::GetReferenceRole method returning its reference role.
  template <typename T>
  static T* CreateReferenceNode(vtkMRMLNode* node, bool allowMultiple = false);

  /// @{
  /// Utility method change the node references of the input node using nodeRef and given role.
  /// If either the node or nodeRef is nullptr, does nothing.
  /// Will invoke a Modified event on the nodeRef after modification.
  template <typename T>
  static T* AddReferenceNode(vtkMRMLNode* node, T* nodeRef, const char* role);

  template <typename T>
  static T* SetReferenceNode(vtkMRMLNode* node, T* nodeRef, const char* role);

  template <typename T>
  static T* SetNthReferenceNode(vtkMRMLNode* node, T* nodeRef, const char* role, int nthRef);
  /// @}

protected:
  vtkSlicerLayerDMLogic() = default;
  ~vtkSlicerLayerDMLogic() override = default;

private:
  template <typename T>
  static T* ModifyNodeReference(vtkMRMLNode* node, vtkMRMLNode* nodeRef, const std::function<void()>& modifyF)
  {
    if (!node || !nodeRef)
    {
      return nullptr;
    }

    modifyF();
    nodeRef->Modified();
    return T::SafeDownCast(nodeRef);
  }

  static constexpr auto EventTranslationRole{ "widgetEventTranslation" };
  static constexpr auto DisplayRole{ "display" }; // copied from vtkMRMLDisplayableNode
};

//----------------------------------------------------------------------------
template <typename T>
void vtkSlicerLayerDMLogic::RegisterNodeIfNeeded(vtkMRMLScene* scene)
{
  if (!scene)
  {
    return;
  }

  vtkNew<T> node;
  if (!scene->IsNodeClassRegistered(node.Get()->GetClassName()))
  {
    scene->RegisterNodeClass(node.Get());
  }
}

template <typename T>
T* vtkSlicerLayerDMLogic::GetDisplayNode(vtkMRMLNode* node)
{
  if (!node)
  {
    return {};
  }

  return GetReferenceNode<T>(node, DisplayRole);
}

template <typename T>
T* vtkSlicerLayerDMLogic::GetReferenceNode(vtkMRMLNode* node, const char* role)
{
  const auto referenceNodes = GetReferenceNodes<T>(node, role);
  return referenceNodes.empty() ? nullptr : referenceNodes[0];
}

template <typename T>
T* vtkSlicerLayerDMLogic::GetReferenceNode(vtkMRMLNode* node)
{
  return GetReferenceNode<T>(node, T::GetReferenceRole());
}

template <typename T>
std::vector<T*> vtkSlicerLayerDMLogic::GetReferenceNodes(vtkMRMLNode* node, const char* role)
{
  if (!node)
  {
    return {};
  }

  std::vector<T*> referenceNodes;
  for (int i_node = 0; i_node < node->GetNumberOfNodeReferences(role); i_node++)
  {
    if (auto refNode = T::SafeDownCast(node->GetNthNodeReference(role, i_node)))
    {
      referenceNodes.emplace_back(refNode);
    }
  }
  return referenceNodes;
}

template <typename T>
std::vector<T*> vtkSlicerLayerDMLogic::GetReferenceNodes(vtkMRMLNode* node)
{
  return GetReferenceNodes<T>(node, T::GetReferenceRole());
}

template <typename T>
T* vtkSlicerLayerDMLogic::CreateDisplayNode(vtkMRMLNode* node, bool allowMultiple)
{
  if (!node)
  {
    return {};
  }

  return CreateReferenceNode<T>(node, DisplayRole, allowMultiple);
}

template <typename T>
T* vtkSlicerLayerDMLogic::CreateReferenceNode(vtkMRMLNode* node, const char* role, bool allowMultiple)
{
  auto scene = node ? node->GetScene() : nullptr;
  if (!node || !scene)
  {
    return {};
  }

  auto nodeRef = GetReferenceNode<T>(node, role);
  if (nodeRef && !allowMultiple)
  {
    return nodeRef;
  }

  nodeRef = T::SafeDownCast(scene->AddNewNodeByClass(vtkSmartPointer<T>::New()->GetClassName()));
  return AddReferenceNode<T>(node, nodeRef, role);
}

template <typename T>
T* vtkSlicerLayerDMLogic::CreateReferenceNode(vtkMRMLNode* node, bool allowMultiple)
{
  return CreateReferenceNode<T>(node, T::GetReferenceRole(), allowMultiple);
}

template <typename T>
T* vtkSlicerLayerDMLogic::AddReferenceNode(vtkMRMLNode* node, T* nodeRef, const char* role)
{
  return ModifyNodeReference<T>(node, nodeRef, [=] { node->AddAndObserveNodeReferenceID(role, nodeRef->GetID()); });
}

template <typename T>
T* vtkSlicerLayerDMLogic::SetReferenceNode(vtkMRMLNode* node, T* nodeRef, const char* role)
{
  return ModifyNodeReference<T>(node, nodeRef, [=] { node->SetAndObserveNodeReferenceID(role, nodeRef->GetID()); });
}

template <typename T>
T* vtkSlicerLayerDMLogic::SetNthReferenceNode(vtkMRMLNode* node, T* nodeRef, const char* role, int nthRef)
{
  return ModifyNodeReference<T>(node, nodeRef, [=] { node->SetAndObserveNthNodeReferenceID(role, nthRef, nodeRef->GetID()); });
}

#pragma once

// LayerDM includes
#include "vtkSlicerLayerDMModuleMRMLExport.h"

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// STL includes
#include <functional>
#include <map>
#include <set>

class vtkMRMLNode;
class vtkMRMLLayerDMObjectEventObserver;
class vtkMRMLScene;

/// Reference node observer.
/// Triggers node ref added / removed when references change in the scene.
/// Allows to appropriately update pipelines when references to a given display node are added / removed.
class VTK_SLICER_LAYERDM_MODULE_MRML_EXPORT vtkMRMLLayerDMNodeReferenceObserver : public vtkObject
{
public:
  using RefT = std::tuple<vtkWeakPointer<vtkMRMLNode>, std::string>;

  enum Event
  {
    ReferenceAddedEvent = 0,
    ReferenceRemovedEvent
  };

  static vtkMRMLLayerDMNodeReferenceObserver* New();

  /// @{
  /// Get references to / from node
  std::set<RefT> GetNodeToReferences(vtkMRMLNode* node) const;
  std::set<RefT> GetNodeFromReferences(vtkMRMLNode* node) const;
  /// @}

  /// @{
  /// Returns the number of nodes currently stored in the observer.
  /// Used for testing / debugging purposes.
  /// Usage in normal context should be unnecessary.
  int GetReferenceToSize() const;
  int GetReferenceFromSize() const;
  int GetNumberOfNodes() const;
  /// @}

  /// @{
  /// Set the callback triggerred when a reference from a node to another node with a given role is added / removed.
  /// If the callbacks are defined before the scene is set to the object, the callbacks will be triggerred for existing nodes in the scene.
  using CallBackT = std::function<void(vtkMRMLNode*, vtkMRMLNode*, const std::string&, int eventType)>;
  void SetReferenceModifiedCallBack(const CallBackT& modifiedCallback);
  /// @}

  /// Setting the MRML scene will trigger node added / removed callbacks if they are set.
  void SetScene(vtkMRMLScene* scene);

protected:
  vtkMRMLLayerDMNodeReferenceObserver();
  ~vtkMRMLLayerDMNodeReferenceObserver() override = default;

private:
  vtkMRMLLayerDMNodeReferenceObserver(const vtkMRMLLayerDMNodeReferenceObserver&);
  void operator=(const vtkMRMLLayerDMNodeReferenceObserver&);

  void UpdateFromScene();
  void OnNodeRemoved(vtkMRMLNode* node);
  void OnNodeAdded(vtkMRMLNode* node);
  void OnReferenceAdded(vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role);
  void OnReferenceRemoved(vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role);
  void RemoveOutdatedReferences(vtkMRMLNode* fromNode);
  void OnReferenceModified(vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role);

  static std::set<RefT> GetNodeReferencesFromScene(vtkMRMLNode* node);
  void TriggerReferenceAdded(vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role) const;
  void TriggerReferenceRemoved(vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role) const;
  static void TriggerCallback(const CallBackT& callback, vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role, int eventType);

  vtkWeakPointer<vtkMRMLScene> m_scene{};
  vtkSmartPointer<vtkMRMLLayerDMObjectEventObserver> m_obs{};
  std::map<vtkSmartPointer<vtkMRMLNode>, std::set<RefT>> m_refTo{};
  std::map<vtkSmartPointer<vtkMRMLNode>, std::set<RefT>> m_refFrom{};
  std::set<vtkSmartPointer<vtkMRMLNode>> m_nodes{};

  CallBackT m_onRefModified{};
};

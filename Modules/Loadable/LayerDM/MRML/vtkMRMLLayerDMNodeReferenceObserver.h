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

#ifndef __vtkMRMLLayerDMNodeReferenceObserver_h
#define __vtkMRMLLayerDMNodeReferenceObserver_h

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
#include <vector>

class vtkMRMLNode;
class vtkMRMLLayerDMObjectEventObserver;
class vtkMRMLScene;

/// Reference node observer.
/// Triggers node ref added / removed when references change in the scene.
/// Allows to appropriately update pipelines when references to a given display node are added / removed.
class VTK_SLICER_LAYERDM_MODULE_MRML_EXPORT vtkMRMLLayerDMNodeReferenceObserver : public vtkObject
{
public:
  /// Reference to a node with a given role.
  ///
  /// The node is held by a weak pointer, unlike the other LayerDM containers, because these lists can outlive
  /// the node: OnNodeRemoved drops the owning key of the map holding the opposite direction of the reference
  /// without notifying the nodes referencing it, so their lists keep the reference until they are updated.
  /// Reading such an entry must therefore check the node before using it. The lists are unordered for the same
  /// reason: a weak pointer nulls itself in place, which would silently change the value of an element of an
  /// ordered container and break its ordering.
  ///
  /// \sa NodeToReferences
  /// \sa NodeFromReferences
  using RefT = std::tuple<vtkWeakPointer<vtkMRMLNode>, std::string>;

  enum Event
  {
    ReferenceAddedEvent = 0,
    ReferenceRemovedEvent
  };

  static vtkMRMLLayerDMNodeReferenceObserver* New();
  vtkTypeMacro(vtkMRMLLayerDMNodeReferenceObserver, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// @{
  /// Get references to / from node
  std::vector<RefT> GetNodeToReferences(vtkMRMLNode* node) const;
  std::vector<RefT> GetNodeFromReferences(vtkMRMLNode* node) const;
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
  /// Set the callback triggered when a reference from a node to another node with a given role is added / removed.
  /// If the callbacks are defined before the scene is set to the object, the callbacks will be triggered for existing nodes in the scene.
  using CallBackT = std::function<void(vtkMRMLNode*, vtkMRMLNode*, const std::string&, int eventType)>;
  void SetReferenceModifiedCallBack(const CallBackT& modifiedCallback);
  /// @}

  /// Setting the MRML scene will trigger node added / removed callbacks if they are set.
  void SetScene(vtkMRMLScene* scene);

protected:
  vtkMRMLLayerDMNodeReferenceObserver();
  ~vtkMRMLLayerDMNodeReferenceObserver() override;

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

  static std::vector<RefT> GetNodeReferencesFromScene(vtkMRMLNode* node);
  void TriggerReferenceAdded(vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role) const;
  void TriggerReferenceRemoved(vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role) const;
  static void TriggerCallback(const CallBackT& callback, vtkMRMLNode* fromNode, vtkMRMLNode* toNode, const std::string& role, int eventType);

  vtkWeakPointer<vtkMRMLScene> Scene;
  vtkSmartPointer<vtkMRMLLayerDMObjectEventObserver> Observer;

  /// @{
  /// References from and to each observed node.
  ///
  /// The keys own their node, which keeps every node referenced by a RefT of the opposite map alive.
  ///
  /// \sa RefT
  /// \sa OnNodeRemoved
  std::map<vtkSmartPointer<vtkMRMLNode>, std::vector<RefT>> NodeToReferences;
  std::map<vtkSmartPointer<vtkMRMLNode>, std::vector<RefT>> NodeFromReferences;
  /// @}

  std::set<vtkSmartPointer<vtkMRMLNode>> Nodes;

  CallBackT ReferenceModifiedCallback;
};

#endif

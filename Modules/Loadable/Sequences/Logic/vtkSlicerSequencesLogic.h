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

// .NAME vtkSlicerSequencesLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerSequencesLogic_h
#define __vtkSlicerSequencesLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerSequencesModuleLogicExport.h"

class vtkMRMLNode;
class vtkMRMLSequenceNode;
class vtkMRMLSequenceBrowserNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_SEQUENCES_MODULE_LOGIC_EXPORT vtkSlicerSequencesLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerSequencesLogic *New();
  vtkTypeMacro(vtkSlicerSequencesLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///
  /// Add into the scene a new mrml sequence node and
  /// read its data from a specified file
  /// A storage node is also added into the scene
  vtkMRMLSequenceNode* AddSequence(const char* filename);

  /// Refreshes the output of all the active browser nodes. Called regularly by a timer.
  void UpdateAllProxyNodes();

  /// Updates the contents of all the proxy nodes (all the nodes copied from the master and synchronized sequences to the scene)
  void UpdateProxyNodesFromSequences(vtkMRMLSequenceBrowserNode* browserNode);

  /// Updates the sequence from a changed proxy node (if saving of state changes is allowed)
  void UpdateSequencesFromProxyNodes(vtkMRMLSequenceBrowserNode* browserNode, vtkMRMLNode* proxyNode);

  /// Deprecated method!
  void UpdateVirtualOutputNodes(vtkMRMLSequenceBrowserNode* browserNode)
    {
    static bool warningLogged = false;
    if (!warningLogged)
      {
      vtkWarningMacro("vtkSlicerSequenceBrowserLogic::UpdateVirtualOutputNodes is deprecated,"
        << " use vtkSlicerSequenceBrowserLogic::UpdateProxyNodes method instead");
      warningLogged = true;
      }
    this->UpdateProxyNodesFromSequences(browserNode);
    }

  /// Add a synchronized sequence node and virtual output node pair to the browser node for playback/recording
  /// \param sequenceNode Sequence node to add. If nullptr, then a new node is created.
  /// \param proxyNode Proxy node to use to represent selected item in the scene. May be nullptr.
  /// Returns the added/created sequence node, nullptr on error.
  vtkMRMLSequenceNode* AddSynchronizedNode(vtkMRMLNode* sequenceNode, vtkMRMLNode* proxyNode, vtkMRMLNode* browserNode);

  void GetCompatibleNodesFromScene(vtkCollection* compatibleNodes, vtkMRMLSequenceNode* sequenceNode);

  static bool IsNodeCompatibleForBrowsing(vtkMRMLSequenceNode* masterNode, vtkMRMLSequenceNode* testedNode);

  /// Get collection of browser nodes that use a specific sequence node.
  void GetBrowserNodesForSequenceNode(vtkMRMLSequenceNode* sequenceNode, vtkCollection* foundBrowserNodes);

  /// Get first browser node that use a specific sequence node. This is a convenience method for
  /// cases when it is known that a sequence is only used in one browser node. In general case,
  /// use GetBrowserNodesForSequenceNode instead.
  vtkMRMLSequenceBrowserNode* GetFirstBrowserNodeForSequenceNode(vtkMRMLSequenceNode* sequenceNode);

protected:
  vtkSlicerSequencesLogic();
  ~vtkSlicerSequencesLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  void ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event, void *callData) override;

  bool IsDataConnectorNode(vtkMRMLNode*);

  // Time of the last update of each browser node (in universal time)
  std::map< vtkMRMLSequenceBrowserNode*, double > LastSequenceBrowserUpdateTimeSec;

private:

  bool UpdateProxyNodesFromSequencesInProgress{false};
  bool UpdateSequencesFromProxyNodesInProgress{false};

  vtkSlicerSequencesLogic(const vtkSlicerSequencesLogic&); // Not implemented
  void operator=(const vtkSlicerSequencesLogic&);               // Not implemented
};

#endif

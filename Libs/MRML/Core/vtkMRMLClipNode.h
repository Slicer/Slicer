/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

#ifndef __vtkMRMLClipNode_h
#define __vtkMRMLClipNode_h

#include "vtkMRMLNode.h"

class vtkImplicitBoolean;
class vtkImplicitFunction;
class vtkImplicitFunctionCollection;
class vtkPlaneCollection;

/// \brief MRML node to represent three clipping planes.
///
/// The vtkMRMLClipNode MRML node stores
/// the direction of clipping for each of the three clipping planes.
/// It also stores the type of combined clipping operation as either an
/// intersection or union.
class VTK_MRML_EXPORT vtkMRMLClipNode : public vtkMRMLNode
{
public:
  static vtkMRMLClipNode* New();
  vtkTypeMacro(vtkMRMLClipNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLClipNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override { return "Clip"; }

  enum
  {
    ClipNodeModifiedEvent = 53001,
  };

  enum
  {
    ClipIntersection = 0,
    ClipUnion = 1
  };

  ///
  /// Indicates the type of clipping
  /// "Intersection" or "Union"
  void SetClipType(int clipType);
  int GetClipType();

  /// Get or set the clipping node index.
  int GetClippingNodeIndex(const char* nodeID);

  enum
  {
    ClipOff = 0,
    ClipPositiveSpace = 1,
    ClipNegativeSpace = 2,
  };

  ///@{
  /// Get or set the clipping node state. (Positive/Negative/Off)
  /// If no index is specified, the state is set for the clipping node or matching id.
  int GetClippingNodeState(vtkMRMLNode* node);
  int GetClippingNodeState(const char* nodeID);
  int GetNthClippingNodeState(int n);
  void SetClippingNodeState(vtkMRMLNode*, int);
  void SetClippingNodeState(const char* nodeID, int);
  void SetNthClippingNodeState(int n, int state);
  ///@}

  ///
  /// Get the node reference property name for the clipping node state.
  const char* GetClippingNodeStatePropertyName() const;

  ///@{
  ///Indicates what clipping method should be used
  ///Straight cut, whole cell extraction, or whole cell extraction with boundary cells
  enum ClippingMethodType
  {
    Straight = 0,
    WholeCells,
    WholeCellsWithBoundary,
  };
  ///@}

  ///@{
  /// Get or set the clipping method.
  /// Options are Straight, WholeCells, WholeCellsWithBoundary.
  vtkGetMacro(ClippingMethod, int);
  vtkSetMacro(ClippingMethod, int);
  ///@}

  /// Get or set the clipping type from a string
  static int GetClipTypeFromString(const char* name);

  /// Get the clipping type as a string from the enum.
  static const char* GetClipTypeAsString(int type);

  /// Get the clipping method as a string from the enum.
  static int GetClippingMethodFromString(const char* name);

  /// Get the clipping method as a string from the enum.
  static const char* GetClippingMethodAsString(int type);

  /// Get the clipping state as a string from the enum.
  static int GetClippingStateFromString(const char* name);
  static const char* GetClippingStateAsString(int state);

  /// Adds a clip plane node ID to the list of clip plane node IDs
  /// Clip planes can be vtkMRMLSliceNode or vtkMRMLMarkupsPlaneNode
  /// \sa SetAndObserveClipNodeID(int, const char*)
  void AddAndObserveClippingNodeID(const char* clippingNodeID);

  ///@{
  /// Set and observe the clip plane node ID
  /// Clip planes can be vtkMRMLSliceNode or vtkMRMLMarkupsPlaneNode
  /// \sa AddAndObserveClipNodeID(const char*)
  void SetAndObserveClippingNodeID(const char* clippingNodeID);
  void SetAndObserveNthClippingNodeID(int n, const char* ClipNodeID);
  ///@}

  ///@{
  /// Remove the clip plane node
  void RemoveClippingNodeID(const char* ClipNodeID);
  void RemoveNthClippingNodeID(int n);
  void RemoveAllClippingNodeIDs();
  ///@}

  ///@{
  /// Get the clip plane node ID
  const char* GetNthClippingNodeID(int n);
  const char* GetClippingNodeID();
  ///@}

  bool HasClippingNodeID(const char* ClipNodeID);
  int GetNumberOfClippingNodes();
  int GetClippingNodeIndexFromID(const char* ClipNodeID);

  ///@{
  /// Get the clipping node
  vtkMRMLNode* GetNthClippingNode(int n);
  vtkMRMLNode* GetClippingNode();
  ///@}

  /// Node reference role used for storing clip plane node references
  const char* GetClippingNodeReferenceRole() const;

  /// Get the implicit function that represents the clipping function created from the referenced clipping nodes.
  vtkImplicitFunction* GetImplicitFunctionWorld();

  void ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData) override;

  /// Get all the plane implicit functions from the clipping nodes.
  void GetPlanes(vtkPlaneCollection* planes);

protected:

  /// Update the implicit function based on the clipping nodes.
  void UpdateImplicitFunction();

protected:
  vtkMRMLClipNode();
  ~vtkMRMLClipNode() override;
  vtkMRMLClipNode(const vtkMRMLClipNode&);
  void operator=(const vtkMRMLClipNode&);

  ///
  /// Called when a node reference ID is added (list size increased).
  void OnNodeReferenceAdded(vtkMRMLNodeReference* reference) override;

  /// Called when a node reference ID is modified.
  void OnNodeReferenceModified(vtkMRMLNodeReference* reference) override;

  ///
  /// Called after a node reference ID is removed (list size decreased).
  void OnNodeReferenceRemoved(vtkMRMLNodeReference* reference) override;

  static const char* ClippingNodeReferenceRole;
  static const char* ClippingNodeReferenceRef;
  static const char* ClippingNodeStatePropertyName;

  int ClippingMethod;

  vtkSmartPointer<vtkImplicitBoolean> ImplicitFunction;
};

#endif

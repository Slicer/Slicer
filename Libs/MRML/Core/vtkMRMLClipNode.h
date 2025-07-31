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
  and was supported in part through NIH grant R01 HL153166.

==============================================================================*/

#ifndef __vtkMRMLClipNode_h
#define __vtkMRMLClipNode_h

#include "vtkMRMLNode.h"

class vtkImplicitBoolean;
class vtkImplicitFunction;
class vtkImplicitFunctionCollection;
class vtkMRMLMessageCollection;
class vtkPlaneCollection;

/// \brief MRML node to represent three clipping planes.
///
/// The vtkMRMLClipNode MRML node stores references to nodes that represent clipping functions.
/// The clipping function can be from:
/// - Planes with vtkMRMLSliceNode or vtkMRMLMarkupsPlaneNode, vtkMRMLMarkupsROINode
/// - Polydata vtkMRMLModelNode
/// - Implicit functions from other vtkMRMLClipNode
/// - Any other vtkMRMLTransformableNode node types that implement GetImplicitFunctionWorld().
/// Each clipping node reference can be enabled, disabled, or inverted by setting the clipping state.
/// The clipping functions are combined using a vtkImplicitBoolean operation using either an intersection or union operation.
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

  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLClipNode);

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override { return "Clip"; }

  /// ClipNodeModifiedEvent is invoked when:
  /// - a clipping node is added or removed
  /// - a clipping node is modified
  enum
  {
    ClipNodeModifiedEvent = 53001
  };

  enum ClipTypeType
  {
    ClipIntersection = 0,
    ClipUnion = 1
  };

  //@{
  /// Indicates the type of clipping
  /// "Intersection" or "Union"
  /// \sa ClipTypeType
  void SetClipType(int clipType);
  int GetClipType();
  //@}

  //@{
  /// Get the clip type from/as a string.
  /// \sa ClipTypeType
  static int GetClipTypeFromString(const char* name);
  static const char* GetClipTypeAsString(int type);
  //@}

  /// Get or set the clipping node index.
  int GetClippingNodeIndex(const char* nodeID);

  /// Clipping state for each clipping node reference
  enum ClippingStateType
  {
    ClipOff = 0,           // Clipping disabled
    ClipPositiveSpace = 1, // Keep +ve region of the implicit function
    ClipNegativeSpace = 2, // Keep -ve region of the implicit function
  };

  //@{
  /// Get or set the clipping node state.
  /// \sa ClippingStateType
  int GetClippingNodeState(vtkMRMLNode* node);
  int GetClippingNodeState(const char* nodeID);
  int GetNthClippingNodeState(int n);
  void SetClippingNodeState(vtkMRMLNode*, int);
  void SetClippingNodeState(const char* nodeID, int);
  void SetNthClippingNodeState(int n, int state);
  //@}

  //@{
  /// Indicates if the Red slice clipping is Off, Positive space, or Negative space
  /// \deprecated Prefer SetClippingNodeState() and GetClippingNodeState()
  int GetRedSliceClipState();
  void SetRedSliceClipState(int);
  //@}

  //@{
  /// Indicates if the Yellow slice clipping is Off, Positive space, or Negative space
  /// \deprecated Prefer SetClippingNodeState() and GetClippingNodeState()
  int GetYellowSliceClipState();
  void SetYellowSliceClipState(int);
  //@}

  //@{
  /// Indicates if the Green slice clipping is Off, Positive space, or Negative space
  /// \deprecated Prefer SetClippingNodeState() and GetClippingNodeState()
  int GetGreenSliceClipState();
  void SetGreenSliceClipState(int);
  //@}

  /// Get the node reference property name for the clipping node state.
  static const char* GetClippingNodeStatePropertyName();

  //@{
  /// Get the clipping state from/as a string
  /// \sa ClippingStateType
  static int GetClippingStateFromString(const char* name);
  static const char* GetClippingStateAsString(int state);
  //@}

  //@{
  /// Indicates what clipping method should be used
  /// Straight cut, whole cell extraction, or whole cell extraction with boundary cells
  enum ClippingMethodType
  {
    Straight = 0,
    WholeCells,
    WholeCellsWithBoundary,
  };
  //@}

  //@{
  /// Get or set the clipping method.
  /// The clipping method determines how the polygons are clipped by the planes.
  /// \sa ClippingMethodType
  vtkGetMacro(ClippingMethod, int);
  vtkSetMacro(ClippingMethod, int);
  //@}

  //@{
  /// Get the clipping method from/as a string
  /// \sa ClippingMethodType
  static int GetClippingMethodFromString(const char* name);
  static const char* GetClippingMethodAsString(int type);
  //@}

  /// Adds a clip plane node ID to the list of clip plane node IDs
  /// Clip planes can be vtkMRMLSliceNode or vtkMRMLMarkupsPlaneNode
  /// \sa SetAndObserveClipNodeID(int, const char*)
  void AddAndObserveClippingNodeID(const char* clippingNodeID);

  //@{
  /// Set and observe the clip plane node ID
  /// Clip planes can be vtkMRMLSliceNode or vtkMRMLMarkupsPlaneNode
  /// \sa AddAndObserveClipNodeID(const char*)
  void SetAndObserveClippingNodeID(const char* clippingNodeID);
  void SetAndObserveNthClippingNodeID(int n, const char* clippingNodeID);
  //@}

  //@{
  /// Remove the clip plane node
  void RemoveClippingNodeID(const char* clippingNodeID);
  void RemoveNthClippingNodeID(int n);
  void RemoveAllClippingNodeIDs();
  //@}

  //@{
  /// Get the clip plane node ID
  const char* GetNthClippingNodeID(int n);
  const char* GetClippingNodeID();
  //@}

  /// Check if the clip plane node ID exists
  bool HasClippingNodeID(const char* clippingNodeID);

  /// Get the number of clip plane nodes
  int GetNumberOfClippingNodes();

  /// Get the index of the clipping node
  int GetClippingNodeIndexFromID(const char* clippingNodeID);

  //@{
  /// Get the clipping node
  vtkMRMLNode* GetNthClippingNode(int n);
  vtkMRMLNode* GetClippingNode();
  //@}

  /// Node reference role used for storing clip plane node references
  const char* GetClippingNodeReferenceRole() const;

  /// Get the implicit function that represents the clipping function created from the referenced clipping nodes.
  vtkImplicitFunction* GetImplicitFunctionWorld();

  /// Get all the plane implicit functions from the clipping nodes.
  void GetPlanes(vtkPlaneCollection* planes);

  /// Overridden to update the implicit function when the clipping nodes are modified.
  void ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData) override;

  /// Overridden to update the implicit function when the clipping nodes are modified.
  void CopyReferences(vtkMRMLNode* node) override;

  /// Get the list of all clipping planes in the clip node, including from nested clip nodes.
  /// Returns false if not all of the functions in the clip node are planes.
  bool GetClippingPlanes(vtkPlaneCollection* planeCollection, bool invert = false, vtkMRMLMessageCollection* messages = nullptr);

protected:
  /// Update the implicit function based on the clipping nodes.
  void UpdateImplicitFunction();

  int GetSliceClipState(const char* nodeID);
  void SetSliceClipState(const char* nodeID, int state);

  /// Get the list of all clipping planes in the implicit function.
  /// If invert is true, then the plane normals will be flipped.
  /// Returns false if not all of the functions in the clip node are planes.
  static bool GetClippingPlanesFromFunction(vtkImplicitFunction* function, vtkPlaneCollection* planeCollection, bool invert = false);

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

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

  ///
  /// Indicates the type of clipping
  /// "Intersection" or "Union"
  void SetClipType(int clipType);
  int GetClipType();

  enum
  {
    ClipIntersection = 0,
    ClipUnion = 1
  };

  enum
  {
    ClipOff = 0,
    ClipPositiveSpace = 1,
    ClipNegativeSpace = 2,
  };
  int GetClippingNodeIndex(const char* nodeID);
  int GetClippingNodeState(vtkMRMLNode* node);
  int GetClippingNodeState(const char* nodeID);
  int GetNthClippingNodeState(int n);
  void SetClippingNodeState(vtkMRMLNode*, int);
  void SetClippingNodeState(const char* nodeID, int);
  void SetNthClippingNodeState(int n, int state);
  const char* GetClippingNodeStatePropertyName() const;

  ///
  ///Indicates what clipping method should be used
  ///Straight cut, whole cell extraction, or whole cell extraction with boundary cells
  enum ClippingMethodType
  {
    Straight = 0,
    WholeCells,
    WholeCellsWithBoundary,
  };

  vtkGetMacro(ClippingMethod, int);
  vtkSetMacro(ClippingMethod, int);

  //Convert between enum and string
  static int GetClipTypeFromString(const char* name);
  static const char* GetClipTypeAsString(int type);

  static int GetClippingMethodFromString(const char* name);
  static const char* GetClippingMethodAsString(int type);

  static int GetClippingStateFromString(const char* name);
  static const char* GetClippingStateAsString(int state);

  enum
  {
    ClipNodeModifiedEvent = 53001,
  };

  ///
  /// Adds a clip plane node ID to the list of clip plane node IDs
  /// Clip planes can be vtkMRMLSliceNode or vtkMRMLMarkupsPlaneNode
  /// \sa SetAndObserveClipNodeID(int, const char*)
  void AddAndObserveClippingNodeID(const char* clippingNodeID);
  void SetAndObserveClippingNodeID(const char* clippingNodeID);
  void RemoveClippingNodeID(const char* ClipNodeID);
  void RemoveNthClippingNodeID(int n);
  void RemoveAllClippingNodeIDs();
  void SetAndObserveNthClippingNodeID(int n, const char* ClipNodeID);
  bool HasClippingNodeID(const char* ClipNodeID);
  int GetNumberOfClippingNodes();
  const char* GetNthClippingNodeID(int n);
  int GetClippingNodeIndexFromID(const char* ClipNodeID);
  const char* GetClippingNodeID();
  vtkMRMLNode* GetNthClippingNode(int n);
  vtkMRMLNode* GetClippingNode();

  /// Node reference role used for storing clip plane node references
  const char* GetClippingNodeReferenceRole() const;

  vtkImplicitFunction* GetImplicitFunctionWorld();

  void ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData) override;

  void GetPlanes(vtkPlaneCollection* planes);

protected:
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

  int ClippingMethod;

  static const char* ClippingNodeReferenceRole;
  static const char* ClippingNodeReferenceRef;
  static const char* ClippingNodeStatePropertyName;

  vtkSmartPointer<vtkImplicitBoolean> ImplicitFunction;
};

#endif

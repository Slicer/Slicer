/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// .NAME vtkMRMLSubjectHierarchyNode - MRML node to represent subject hierarchy of DICOM objects
// .SECTION Description
// N/A
//

#ifndef __vtkMRMLSubjectHierarchyNode_h
#define __vtkMRMLSubjectHierarchyNode_h

#include "vtkSlicerSubjectHierarchyModuleMRMLExport.h"

// MRML includes
#include <vtkMRMLHierarchyNode.h>

// STD includes
#include <map>

class vtkMRMLTransformNode;

/// \ingroup Slicer_QtModules_SubjectHierarchy
/// \brief MRML node to represent a subject hierarchy object
///   Separate node type has the advantage of identifying itself faster (type vs string comparison)
///   and providing utility functions within the class
class VTK_SLICER_SUBJECTHIERARCHY_MODULE_MRML_EXPORT vtkMRMLSubjectHierarchyNode : public vtkMRMLHierarchyNode
{
public:
  // Separator characters for (de)serializing the UID map
  static const std::string SUBJECTHIERARCHY_UID_ITEM_SEPARATOR;
  static const std::string SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR;

public:
  static vtkMRMLSubjectHierarchyNode *New();
  vtkTypeMacro(vtkMRMLSubjectHierarchyNode,vtkMRMLHierarchyNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// Read node attributes from XML file
  virtual void ReadXMLAttributes(const char** atts);

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// Get node XML tag name (like Volume, Contour)
  virtual const char* GetNodeTagName();

public:
  /// Find subject hierarchy node according to a UID
  static vtkMRMLSubjectHierarchyNode* GetSubjectHierarchyNodeByUID(vtkMRMLScene* scene, const char* uidName, const char* uidValue);

  /// Get associated subject hierarchy node for a MRML node
  /// Note: This must be used instead of vtkMRMLHierarchyNode::GetAssociatedHierarchyNode, because nested associations have been introduced to avoid conflicts.
  /// E.g. a data node is associated to both a ModelHierarchy and a SubjectHierarchy node. In that case the first associated hierarchy
  /// node is returned by the utility function, which is a non-deterministic behavior. To avoid this we use nested associations. In the
  /// example case the associations are as follows: SubjectHierarchy -> ModelHierarchy -> DataNode
  /// \param associatedNode The node for which we want the associated hierarchy node
  /// \param scene MRML scene pointer (in case the associated node is not in the scene any more). If not specified, then the scene of the argument node is used.
  /// \return If associatedNode is a subject hierarchy node, then return that. Otherwise the first subject hierarchy node to which the given node is associated to.
  static vtkMRMLSubjectHierarchyNode* GetAssociatedSubjectHierarchyNode(vtkMRMLNode *associatedNode, vtkMRMLScene* scene=NULL);

  /// Get child subject hierarchy node with specific name
  /// \param parent Parent subject hierarchy node to start from. If NULL, then looking for top-level nodes
  /// \param name Name to find
  /// \param scene MRML scene (in case the parent node is not given)
  /// \return Child node whose name without postfix is the same as the given attribute
  static vtkMRMLSubjectHierarchyNode* GetChildWithName(vtkMRMLSubjectHierarchyNode* parent, const char* name, vtkMRMLScene* scene=NULL);

  /// Create subject hierarchy node in the scene under a specified parent
  /// \param scene MRML scene
  /// \param parent Parent node under which the created node is put. If NULL, then the child will be a top-level node
  /// \param level Level string of the created node
  /// \param nodeName Name of the node (subject hierarchy postfix is added to it)
  /// \param associatedNode Data node to associate with the created subject hierarchy node. If NULL, then no node will be associated
  static vtkMRMLSubjectHierarchyNode* CreateSubjectHierarchyNode(vtkMRMLScene* scene,
                                                                 vtkMRMLSubjectHierarchyNode* parent,
                                                                 const char* level,
                                                                 const char* nodeName,
                                                                 vtkMRMLNode* associatedNode=NULL);

public:
  /// Get node associated with this hierarchy node.
  /// Note: Override of vtkMRMLHierarchyNode::GetAssociatedNode to handle nested associations to avoid conflicts.
  /// E.g. a data node is associated to both a ModelHierarchy and a SubjectHierarchy node. In that case the first associated hierarchy
  /// node is returned by the utility function, which is a non-deterministic behavior. To avoid this we use nested associations. In the
  /// example case the associations are as follows: <pre>SubjectHierarchy -> ModelHierarchy -> DataNode</pre>
  virtual vtkMRMLNode* GetAssociatedNode();

  /// Find all associated children nodes of a specified class in the hierarchy.
  /// Re-implemented to handle nested associations \sa GetAssociatedNode
  /// \param children Collection updated with the list of children nodes.
  /// \param childClass Name of the class we are looking for. NULL returns all associated children nodes.
  virtual void GetAssociatedChildrenNodes(vtkCollection *children, const char* childClass=NULL);

  /// Set subject hierarchy branch visibility
  void SetDisplayVisibilityForBranch(int visible);

  /// Get subject hierarchy branch visibility
  /// \return Visibility value (0:Hidden, 1:Visible, 2:PartiallyVisible)
  int GetDisplayVisibilityForBranch();

  /// Determine if a node is of a certain level
  /// \param level Level to check
  /// \return True if the node is of the specified level, false otherwise
  bool IsLevel(const char* level);

  /// Get attribute value for a node from an upper level in the subject hierarchy
  /// \param attributeName Name of the requested attribute
  /// \param level Level of the ancestor node we look for the attribute in (e.g. SubjectHierarchy_LEVEL_STUDY). If NULL, then look all the way up to the subject
  /// \return Attribute value from the lowest level ancestor where the attribute can be found
  const char* GetAttributeFromAncestor(const char* attributeName, const char* level=NULL);

  /// Get ancestor subject hierarchy node at a certain level
  /// \param level Level of the ancestor node we start searching.
  vtkMRMLSubjectHierarchyNode* GetAncestorAtLevel(const char* level);

  /// Get node name without the subject hierarchy postfix
  std::string GetNameWithoutPostfix()const;

  /// Determine if any of the children of this node is transformed (has a parent transform applied), except for an optionally given node
  /// \param exceptionNode The function still returns true if the only applied transform found is this specified node
  bool IsAnyNodeInBranchTransformed(vtkMRMLTransformNode* exceptionNode=NULL);

  /// Apply transform node as parent transform on itself and on all children, recursively
  /// \param transformNode Transform node to apply
  /// \param hardenExistingTransforms Mode of handling already transformed nodes. If true (default), then the occurrent parent transforms
  ///   of target nodes are hardened before applying the specified transform. If false, then the already applied parent transforms are
  ///   transformed with the specified transform (Note: this latter approach may result in unwanted transformations of other nodes)
  void TransformBranch(vtkMRMLTransformNode* transformNode, bool hardenExistingTransforms=true);

  /// Harden transform on itself and on all children, recursively
  void HardenTransformOnBranch();

public:
  /// Set level
  vtkSetStringMacro(Level);
  /// Get level
  vtkGetStringMacro(Level);

  /// Set owner plugin name
  virtual void SetOwnerPluginName(const char* pluginName);
  /// Get owner plugin name
  vtkGetStringMacro(OwnerPluginName);

  /// Set plugin auto search flag
  vtkBooleanMacro(OwnerPluginAutoSearch, bool);
  /// Set plugin auto search flag
  vtkSetMacro(OwnerPluginAutoSearch, bool);
  /// Get plugin auto search flag
  vtkGetMacro(OwnerPluginAutoSearch, bool);

  /// Add UID to the subject hierarchy node
  void AddUID(const char* uidName, const char* uidValue);
  /// Add UID to the subject hierarchy node
  void AddUID(std::string uidName, std::string uidValue);

  /// Get a UID with a known name
  /// \return The UID value if exists, empty string if does not
  std::string GetUID(std::string uidName);

  /// Get UID map for this subject hierarchy node
  std::map<std::string, std::string> GetUIDs()const;

public:
  enum
  {
    OwnerPluginChangedEvent = 24000
  };

protected:
  /// Level identifier (default levels are Subject and Study)
  char* Level;

  /// Name of the owner plugin that claimed this node
  char* OwnerPluginName;

  /// Flag indicating whether a plugin automatic search needs to be performed when the node is modified
  /// By default it is true. It is usually only set to false when the user has manually overridden the
  /// automatic choice. In that case the manual selection is not automatically overridden.
  bool OwnerPluginAutoSearch;

  /// List of UIDs of this subject hierarchy node
  /// UIDs can be DICOM UIDs, MIDAS urls, etc.
  std::map<std::string, std::string> UIDs;

protected:
  vtkMRMLSubjectHierarchyNode();
  ~vtkMRMLSubjectHierarchyNode();
  vtkMRMLSubjectHierarchyNode(const vtkMRMLSubjectHierarchyNode&);
  void operator=(const vtkMRMLSubjectHierarchyNode&);
};

#endif

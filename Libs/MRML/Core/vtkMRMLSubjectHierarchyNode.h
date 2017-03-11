/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

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

// .NAME vtkMRMLSubjectHierarchyNode
// .SECTION Description
// MRML node to represent subject hierarchy tree
//

#ifndef __vtkMRMLSubjectHierarchyNode_h
#define __vtkMRMLSubjectHierarchyNode_h

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLSubjectHierarchyConstants.h>

class vtkCallbackCommand;
class vtkMRMLTransformNode;

/// \ingroup Slicer_MRML_Core
/// \brief MRML node to represent a complete subject hierarchy tree
///
///   There can be only one such node in the scene, as subject hierarchy is to contain all the supported
///   data nodes in the scene, so that the user can navigate the data loaded throughout the session.
///   It is not singleton in either the common or the MRML sense, instead, the subject hierarchy logic
///   makes sure that any added subject hierarchy nodes are merged in the first one, and if the last one
///   is removed, a new one is created. The used subject hierarchy node can be accessed using the static
///   function \sa GetSubjectHierarchyNode().
///
///   The node entries are encapsulated in SubjectHierarchyItem classes, which contain the hierarchy
///   information for the contained nodes, and represent the non-leaf nodes of the tree. Accessor functions
///   can be used to get/set the properties of the individual items.
///
///   The node's Modified event triggers re-building the views from scratch, while the per-item events
///   are used for more granular updates.
///
class VTK_MRML_EXPORT vtkMRMLSubjectHierarchyNode : public vtkMRMLNode
{
public:
  static const vtkIdType INVALID_ITEM_ID;

  // Separator characters for (de)serializing the UID and the attributes map
  static const std::string SUBJECTHIERARCHY_SEPARATOR;
  static const std::string SUBJECTHIERARCHY_NAME_VALUE_SEPARATOR;

  // Attribute name to identify subject hierarchy 2.0 nodes in scene MRML files
  static const std::string SUBJECTHIERARCHY_VERSION_ATTRIBUTE_NAME;

  enum
  {
    SubjectHierarchyItemAddedEvent = 62000,
    SubjectHierarchyItemAboutToBeRemovedEvent,
    SubjectHierarchyItemRemovedEvent,
    SubjectHierarchyItemModifiedEvent,
    /// Event invoked when UID is added to subject hierarchy item. Useful when using UIDs
    /// to find related nodes, and the nodes are loaded sequentially in unspecified order.
    SubjectHierarchyItemUIDAddedEvent,
    /// Event invoked when a subject hierarchy item or its data node changed in a way that
    /// will influence the best owner plugin. Typically invoked by \sa RequestOwnerPluginSearch
    /// after setting an attribute indicating a desired role for the item
    SubjectHierarchyItemOwnerPluginSearchRequested
  };

public:
  static vtkMRMLSubjectHierarchyNode *New();
  vtkTypeMacro(vtkMRMLSubjectHierarchyNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// Read item from XML
  virtual void ReadItemFromXML(const char** atts);
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);
  /// Write this node's body to a MRML file in XML format.
  virtual void WriteNodeBodyXML(ostream& of, int indent);

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// Get node XML tag name (like Volume, Contour)
  virtual const char* GetNodeTagName();

// Get/Set methods
public:
  /// Get the (practically) singleton subject hierarchy node from MRML scene.
  /// Merges subject hierarchy nodes if multiple found, and returns the merged one.
  static vtkMRMLSubjectHierarchyNode* GetSubjectHierarchyNode(vtkMRMLScene* scene);

  /// Get ID of root subject hierarchy item (which can be interpreted as the scene in terms of hierarchy)
  vtkIdType GetSceneItemID();
  /// Get data node for a subject hierarchy item
  /// Note: There is no setter function because a subject hierarchy item can be associated to only one data node during
  ///   its lifetime. This is due to simplicity reasons so that plugin search does not need to re-run when item is modified.
  ///   It is very easy to create a new item for a new data node if needed using \sa CreateItem
  vtkMRMLNode* GetItemDataNode(vtkIdType itemID);
  /// Set name for a subject hierarchy item
  void SetItemName(vtkIdType itemID, std::string name);
  /// Get name for a subject hierarchy item
  /// \return Name of the associated data node if any, otherwise the name of the item
  std::string GetItemName(vtkIdType itemID);
  /// Set level for a subject hierarchy item
  void SetItemLevel(vtkIdType itemID, std::string level);
  /// Get level for a subject hierarchy item
  std::string GetItemLevel(vtkIdType itemID);
  /// Set owner plugin name (role) for a subject hierarchy item
  void SetItemOwnerPluginName(vtkIdType itemID, std::string ownerPluginName);
  /// Get owner plugin name (role) for a subject hierarchy item
  std::string GetItemOwnerPluginName(vtkIdType itemID);
  /// Set expanded flag for a subject hierarchy item (only for internal use, do not set explicitly)
  void SetItemExpanded(vtkIdType itemID, bool expanded);
  /// Get expanded flag for a subject hierarchy item
  bool GetItemExpanded(vtkIdType itemID);

  /// Set UID to the subject hierarchy item
  void SetItemUID(vtkIdType itemID, std::string uidName, std::string uidValue);
  /// Get a UID with a given name
  /// \return The UID value if exists, empty string if does not
  std::string GetItemUID(vtkIdType itemID, std::string uidName);

  /// Add attribute to the subject hierarchy item
  void SetItemAttribute(vtkIdType itemID, std::string attributeName, std::string attributeValue);
  /// Remove attribute from subject hierarchy item
  /// \return True if attribute was removed, false if item or attribute is not found
  bool RemoveItemAttribute(vtkIdType itemID, std::string attributeName);
  /// Get an attribute with a given name for a subject hierarchy item
  /// \return The attribute value if exists, empty string if does not (also if attribute value is empty! \sa HasItemAttribute)
  std::string GetItemAttribute(vtkIdType itemID, std::string attributeName);
  /// Get attribute names for a subject hierarchy item
  /// \return List of attribute names
  std::vector<std::string> GetItemAttributeNames(vtkIdType itemID);
  /// Determine if a given attribute is present in an item.
  /// Especially useful if need to determine whether an attribute value is empty string or the attribute is missing
  /// \return True if attribute exists, false if item or attribute is not found
  bool HasItemAttribute(vtkIdType itemID, std::string attributeName);

  /// Invoke item modified event (that triggers per-item update in the views). Useful if a property of the item
  /// changes that does not originate in the subject hierarchy item (such as visibility or transform of data node)
  void ItemModified(vtkIdType itemID);

  /// Invoke SubjectHierarchyItemOwnerPluginSearchRequested event for the item, which results in a search for owner plugin.
  /// Typically called after setting an attribute indicating a desired role for the item.
  void RequestOwnerPluginSearch(vtkIdType itemID);
  /// Invoke SubjectHierarchyItemOwnerPluginSearchRequested event for a node in the scene, which results in a search for owner plugin.
  /// If the data node is not in the subject hierarchy yet then it will be added.
  /// Typically called after setting an attribute indicating a desired role for the item.
  void RequestOwnerPluginSearch(vtkMRMLNode* dataNode);

  /// Python accessor for the invalid ID
  static vtkIdType GetInvalidItemID() { return INVALID_ITEM_ID; }

// Hierarchy related methods
public:
  /// Create subject hierarchy item in the hierarchy under a specified parent.
  /// Used for creating hierarchy items (folder, patient, study, etc.)
  /// \param parentItemID Parent item under which the created item is inserted. If top-level then use \sa GetSceneItemID
  /// \param name Name of the item. Only used if there is no data node associated
  /// \param level Level string of the created item (\sa vtkMRMLSubjectHierarchyConstants)
  /// \return ID of the item in the hierarchy that was assigned automatically when adding
  vtkIdType CreateItem( vtkIdType parentItemID,
                        std::string name,
                        std::string level=vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder() );
  /// Create subject hierarchy item for a data node.
  /// Can be used to add nodes that were not added automatically (e.g. HideFromEditors was on, or exclude attribute was set)
  /// \param parentItemID Parent item under which the created item is inserted. If top-level then use \sa GetSceneItemID
  /// \param dataNode Associated data MRML node
  /// \param level Level string of the created item (\sa vtkMRMLSubjectHierarchyConstants)
  /// \return ID of the item in the hierarchy that was assigned automatically when adding
  vtkIdType CreateItem( vtkIdType parentItemID,
                        vtkMRMLNode* dataNode,
                        std::string level=vtkMRMLSubjectHierarchyConstants::GetDICOMLevelSeries() );

  /// Remove subject hierarchy item or branch from the tree
  /// \param itemID Item to remove
  /// \param removeDataNode Flag determining whether to remove associated data node from the scene if any. On by default
  /// \param recursive Flag determining whether to remove children recursively (the whole branch). On by default
  bool RemoveItem(vtkIdType itemID, bool removeDataNode=true, bool recursive=true);
  /// Remove child items of a subject hierarchy item
  /// \param itemID Parent of items to remove
  /// \param removeDataNodes Flag determining whether to remove associated data nodes from the scene if any. On by default
  /// \param recursive Flag determining whether to remove children recursively, or just the direct children. On by default
  bool RemoveItemChildren(vtkIdType itemID, bool removeDataNodes=true, bool recursive=true);
  /// Remove all items from hierarchy
  /// \param removeDataNode Flag determining whether to remove associated data node from the scene if any. False by default,
  ///   because as opposed to the method \sa RemoveItem that is usually initiated by the user, this method is
  ///   called when subject hierarchy is re-built from the scene
  void RemoveAllItems(bool removeDataNode=false);

  /// Set the parent of a subject hierarchy item
  void SetItemParent(vtkIdType itemID, vtkIdType parentItemID);
  /// Get ID of the parent of a subject hierarchy item
  /// \return Parent item ID, INVALID_ITEM_ID if there is no parent
  vtkIdType GetItemParent(vtkIdType itemID);
  /// Get position of item under its parent
  /// \return Position of item under its parent. -1 on failure
  int GetItemPositionUnderParent(vtkIdType itemID);
  /// Get item under parent by position
  /// \return ID of item found in given position. Invalid if no item found at that position
  vtkIdType GetItemByPositionUnderParent(vtkIdType parentItemID, int position);

  /// Get IDs of the children of a subject hierarchy item
  /// \param childIDs Output vector containing the children. It will not contain the given item itself.
  ///   The order of the items is depth-first, in a way that the child of an item in the vector always comes after the item.
  /// \param recursive If false then collect direct children, if true then the whole branch. False by default
  void GetItemChildren(vtkIdType itemID, std::vector<vtkIdType>& childIDs, bool recursive=false);
  /// Python accessibility function to get children of a subject hierarchy item
  void GetItemChildren(vtkIdType itemID, vtkIdList* childIDs, bool recursive=false);

  /// Set new parent to a subject hierarchy item under item associated to specified data node
  /// \return Success flag
  bool ReparentItemByDataNode(vtkIdType itemID, vtkMRMLNode* newParentNode);
  /// Move item within the same branch before given item
  /// \param beforeItemID Item to move given item before. If INVALID_ITEM_ID then insert to the end
  /// \return Success flag
  bool MoveItem(vtkIdType itemID, vtkIdType beforeItemID);

// Item finder methods
public:
  /// Find subject hierarchy item according to a UID (by exact match)
  /// \param uidName UID string to lookup
  /// \param uidValue UID string that needs to _exactly match_ the UID string of the subject hierarchy item
  /// \sa GetUID()
  vtkIdType GetItemByUID(const char* uidName, const char* uidValue);

  /// Find subject hierarchy item according to a UID (by containing). For example find UID in instance UID list
  /// \param uidName UID string to lookup
  /// \param uidValue UID string that needs to be _contained_ in the UID string of the subject hierarchy item
  /// \return First match
  /// \sa GetUID()
  vtkIdType GetItemByUIDList(const char* uidName, const char* uidValue);

  /// Get subject hierarchy item associated to a data MRML node
  /// \param dataNode The node for which we want the associated hierarchy node
  /// \return The first subject hierarchy item ID to which the given node is associated to.
  vtkIdType GetItemByDataNode(vtkMRMLNode* dataNode);

  /// Get direct child subject hierarchy item with specific name (not recursive)
  /// \param parent Parent subject hierarchy item to start from
  /// \param name Name to find
  /// \return Child node whose name without postfix is the same as the given attribute
  vtkIdType GetItemChildWithName(vtkIdType parentItemID, std::string name);

  /// Find all associated data nodes of a specified class in a branch of the hierarchy.
  /// Re-implemented to handle nested associations \sa GetAssociatedNode
  /// \param itemID Parent item of the branch
  /// \param dataNodeCollection Collection updated with the list of data nodes
  /// \param childClass Name of the class we are looking for. NULL returns associated data nodes of any kind
  void GetDataNodesInBranch(vtkIdType itemID, vtkCollection* dataNodeCollection, const char* childClass=NULL);

  /// Get data node associated to the parent of the item associated to a data node.
  /// \param recursive Flag determining whether only the direct parent is considered (false), or
  ///   also further up in the tree (true). Default is false
  /// \return Data node associated to the parent of the given data node's item
  vtkMRMLNode* GetParentDataNode(vtkMRMLNode* dataNode, bool recursive=false);

// Utility functions
public:
  /// Set subject hierarchy branch visibility
  void SetDisplayVisibilityForBranch(vtkIdType itemID, int visible);

  /// Get subject hierarchy branch visibility
  /// \return Visibility value (0:Hidden, 1:Visible, 2:PartiallyVisible)
  int GetDisplayVisibilityForBranch(vtkIdType itemID);

  /// Determine if an item is of a certain level
  /// \param level Level to check
  /// \return True if the item is of the specified level, false otherwise
  bool IsItemLevel(vtkIdType itemID, std::string level);

  /// Get attribute value for a item from an upper level in the subject hierarchy
  /// \param attributeName Name of the requested attribute
  /// \param level Level of the ancestor item we look for the attribute in
  ///   (e.g. value of vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelStudy()).
  ///   If empty, then look all the way up to the subject and return first attribute found with specified name
  /// \return Attribute value from the lowest level ancestor where the attribute can be found
  std::string GetAttributeFromItemAncestor(vtkIdType itemID, std::string attributeName, std::string level="");

  /// Get ancestor subject hierarchy item at a certain level
  /// \param level Level of the ancestor item we start searching.
  vtkIdType GetItemAncestorAtLevel(vtkIdType itemID, std::string level);

  /// Determine if any of the children of this item is transformed (has a parent transform applied), except for an optionally given node
  /// \param exceptionNode The function still returns true if the only applied transform found is this specified node
  bool IsAnyNodeInBranchTransformed(vtkIdType itemID, vtkMRMLTransformNode* exceptionNode=NULL);

  /// Deserialize a UID list string (stored in the UID map) into a vector of UID strings
  static void DeserializeUIDList(std::string uidListString, std::vector<std::string>& deserializedUIDList);

  /// Get subject hierarchy items that are referenced from a given item by DICOM.
  /// Finds the series items that contain the SOP instance UIDs that are listed in
  /// the attribute of this item containing the referenced SOP instance UIDs
  /// \sa vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName()
  std::vector<vtkIdType> GetItemsReferencedFromItemByDICOM(vtkIdType itemID);

  /// Merge given subject hierarchy into this one. Should not be called manually, as it is automatically handled by the logic
  /// \param otherShNode Subject hierarchy node to merge into this one. It is removed after merging.
  /// \return Success flag
  bool MergeSubjectHierarchy(vtkMRMLSubjectHierarchyNode* otherShNode);

  /// Generate unique item name
  std::string GenerateUniqueItemName(std::string name);

  /// Get number of items under the scene, excluding the scene
  int GetNumberOfItems();

  /// Get number of children for an item
  /// \param recursive If true, then get total number of items in the branch, only direct children otherwise. False by default
  int GetNumberOfItemChildren(vtkIdType itemID, bool recursive=false);

  /// Print subject hierarchy item info on stream
  void PrintItem(vtkIdType itemID, ostream& os, vtkIndent indent);

protected:
  /// Callback function for all events from the subject hierarchy items
  static void ItemEventCallback(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

protected:
  vtkMRMLSubjectHierarchyNode();
  ~vtkMRMLSubjectHierarchyNode();
  vtkMRMLSubjectHierarchyNode(const vtkMRMLSubjectHierarchyNode&);
  void operator=(const vtkMRMLSubjectHierarchyNode&);

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;

  /// Command handling events from subject hierarchy items
  vtkSmartPointer<vtkCallbackCommand> ItemEventCallbackCommand;
};

#endif

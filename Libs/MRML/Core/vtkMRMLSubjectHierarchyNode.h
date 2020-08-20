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
class vtkMRMLDisplayNode;
class vtkMRMLTransformNode;

/// \ingroup Slicer_MRML_Core
/// \brief MRML node to represent a complete subject hierarchy tree
///
///   There can be only one such node in the scene, as subject hierarchy is to contain all the supported
///   data nodes in the scene, so that the user can navigate the data loaded throughout the session.
///   It is not singleton in either the common or the MRML sense, instead, the subject hierarchy logic
///   makes sure that any added subject hierarchy nodes are merged in the first one, and if the last one
///   is removed, a new one is created. The used subject hierarchy node can be accessed using the static
///   function \sa GetSubjectHierarchyNode(). Resolution of multiple SH nodes or stale nodes can be
///   performed via \sa ResolveSubjectHierarchy()
///
///   The node entries are encapsulated in SubjectHierarchyItem classes, which contain the hierarchy
///   information for the contained nodes, and represent the non-leaf nodes of the tree. Accessor functions
///   can be used to get/set the properties of the individual items.
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
    SubjectHierarchyItemDisplayModifiedEvent,
    SubjectHierarchyItemTransformModifiedEvent,
    SubjectHierarchyItemReparentedEvent,
    /// Event invoked when UID is added to subject hierarchy item. Useful when using UIDs
    /// to find related nodes, and the nodes are loaded sequentially in unspecified order.
    SubjectHierarchyItemUIDAddedEvent,
    /// Event invoked when a subject hierarchy item or its data node changed in a way that
    /// will influence the best owner plugin. Typically invoked by \sa RequestOwnerPluginSearch
    /// after setting an attribute indicating a desired role for the item
    SubjectHierarchyItemOwnerPluginSearchRequested,
    /// Event invoked when item resolving starts (e.g. after scene import)
    SubjectHierarchyStartResolveEvent,
    /// Event invoked when item resolving finished (e.g. after scene import)
    SubjectHierarchyEndResolveEvent
  };

public:
  static vtkMRMLSubjectHierarchyNode *New();
  vtkTypeMacro(vtkMRMLSubjectHierarchyNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Read item from XML
  virtual void ReadItemFromXML(const char** atts);
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;
  /// Write this node's body to a MRML file in XML format.
  void WriteNodeBodyXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  /// Get node XML tag name (like Volume, Contour)
  const char* GetNodeTagName() override;

// Get/Set methods
public:
  /// Get the (practically) singleton subject hierarchy node from MRML scene.
  /// Merges subject hierarchy nodes if multiple found, and returns the merged one.
  static vtkMRMLSubjectHierarchyNode* GetSubjectHierarchyNode(vtkMRMLScene* scene);

  /// Get ID of root subject hierarchy item (which can be interpreted as the scene in terms of hierarchy)
  vtkIdType GetSceneItemID();
  /// Get data node for a subject hierarchy item
  vtkMRMLNode* GetItemDataNode(vtkIdType itemID);
  /// Set name for a subject hierarchy item
  void SetItemName(vtkIdType itemID, std::string name);
  /// Get name for a subject hierarchy item
  /// \return Name of the associated data node if any, otherwise the name of the item
  std::string GetItemName(vtkIdType itemID);
  /// Convenience function to set level attribute for a subject hierarchy item
  void SetItemLevel(vtkIdType itemID, std::string level);
  /// Convenience function to get level attribute for a subject hierarchy item
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

  /// Set data node for a subject hierarchy item
  /// Note: This function is solely for setting nodes associated to hierarchy items (e.g. folder), and returns
  ///   with error if a data node is already associated to the item. Does not call item modified event!
  ///   This is due to simplicity reasons so that plugin search does not need to re-run when item is modified.
  ///   Should never need to call manually!
  void SetItemDataNode(vtkIdType itemID, vtkMRMLNode* dataNode);

// Hierarchy related methods
public:
  /// Create subject hierarchy item for a data node.
  /// Can be used to add nodes that were not added automatically (e.g. private scene, HideFromEditors on, or exclude attribute set)
  /// \param parentItemID Parent item under which the created item is inserted. If top-level then use \sa GetSceneItemID
  /// \param dataNode Associated data MRML node
  /// \return ID of the item in the hierarchy that was assigned automatically when adding
  vtkIdType CreateItem(vtkIdType parentItemID, vtkMRMLNode* dataNode, const char* ownerPluginName=nullptr);
  /// Generic function to create hierarchy items of given level. Convenience functions are available for frequently used levels
  /// \sa CreateSubjectItem, \sa CreateStudyItem, \sa CreateFolderItem
  /// \param parentItemID Parent item under which the created item is inserted. If top-level then use \sa GetSceneItemID
  /// \param name Name of the item. Only used if there is no data node associated
  /// \param level Level of the hierarchy item. It will be stored as attribute (\sa vtkMRMLSubjectHierarchyConstants)
  /// \return ID of the item in the hierarchy that was assigned automatically when adding
  vtkIdType CreateHierarchyItem(vtkIdType parentItemID, std::string name, std::string level);
  /// Convenience function to create subject item in the hierarchy under a specified parent.
  /// \param parentItemID Parent item under which the created item is inserted. If top-level then use \sa GetSceneItemID
  /// \param name Name of the item. Only used if there is no data node associated
  /// \return ID of the item in the hierarchy that was assigned automatically when adding
  vtkIdType CreateSubjectItem(vtkIdType parentItemID, std::string name);
  /// Convenience function to create study item in the hierarchy under a specified parent.
  /// \param parentItemID Parent item under which the created item is inserted
  /// \param name Name of the item. Only used if there is no data node associated
  /// \return ID of the item in the hierarchy that was assigned automatically when adding
  vtkIdType CreateStudyItem(vtkIdType parentItemID, std::string name);
  /// Convenience function to create folder item in the hierarchy under a specified parent.
  /// \param parentItemID Parent item folder which the created item is inserted. If top-level then use \sa GetSceneItemID
  /// \param name Name of the item. Only used if there is no data node associated
  /// \return ID of the item in the hierarchy that was assigned automatically when adding
  vtkIdType CreateFolderItem(vtkIdType parentItemID, std::string name);

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
  /// \param enableCircularCheck Option to do a safety check for circular parenthood in performance-critical cases. On by default.
  void SetItemParent(vtkIdType itemID, vtkIdType parentItemID, bool enableCircularCheck=true);
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

  /// Get item in whole subject hierarchy by a given name
  /// \param name Name to find
  /// \return Item ID of the first item found by name using exact match. Warning is logged if more than one found
  vtkIdType GetItemByName(std::string name);

  /// Get items in whole subject hierarchy by a given name
  /// \param name Name to find
  /// \param foundItemIds List of found items
  /// \param contains Flag whether string containment is enough to determine match. True means a substring is searched
  ///   (case insensitive), false means that the name needs to match exactly (case sensitive). False by default.
  /// \return Item ID of the first item found by name using exact match. Warning is logged if more than one found
  void GetItemsByName(std::string name, vtkIdList* foundItemIds, bool contains=false);

  /// Get child subject hierarchy item with specific name
  /// \param parent Parent subject hierarchy item to start from
  /// \param name Name to find
  /// \param recursive Flag determining whether search is recursive or not. False by default
  /// \return Child item ID whose name is the same as the given string
  vtkIdType GetItemChildWithName(vtkIdType parentItemID, std::string name, bool recursive=false);

  /// Find all associated data nodes of a specified class in a branch of the hierarchy.
  /// \param itemID Parent item of the branch
  /// \param dataNodeCollection Collection updated with the list of data nodes
  /// \param childClass Name of the class we are looking for. nullptr returns associated data nodes of any kind
  void GetDataNodesInBranch(vtkIdType itemID, vtkCollection* dataNodeCollection, const char* childClass=nullptr);

  /// Get data node associated to the parent of the item associated to a data node.
  /// \param recursive Flag determining whether only the direct parent is considered (false), or
  ///   also further up in the tree (true). Default is false
  /// \return Data node associated to the parent of the given data node's item
  vtkMRMLNode* GetParentDataNode(vtkMRMLNode* dataNode, bool recursive=false);

// Utility functions
public:
  /// Set subject hierarchy branch visibility
  /// \deprecated Kept only for backward compatibility. \sa SetItemDisplayVisibility
  void SetDisplayVisibilityForBranch(vtkIdType itemID, int visible);
  /// Get subject hierarchy branch visibility
  /// \deprecated Kept only for backward compatibility. \sa GetItemDisplayVisibility
  /// \return Visibility value (0:Hidden, 1:Visible, 2:PartiallyVisible, -1:NotDisplayable)
  int GetDisplayVisibilityForBranch(vtkIdType itemID);

  /// Set subject hierarchy item visibility.
  /// If the item is a hierarchy item (folder, study, etc.), then it will apply to the whole branch,
  /// as the displayable managers consider hierarchy visibility information.
  void SetItemDisplayVisibility(vtkIdType itemID, int visible);
  /// Get subject hierarchy item visibility
  int GetItemDisplayVisibility(vtkIdType itemID);

  /// Get display node associated to a given item, either directly (folder, study, etc.)
  /// or indirectly (displayable nodes with an associated display node)
  vtkMRMLDisplayNode* GetDisplayNodeForItem(vtkIdType itemID);

  /// Determine if an item is of a certain level
  /// \param level Level to check
  /// \return True if the item is of the specified level, false otherwise
  bool IsItemLevel(vtkIdType itemID, std::string level);

  /// Determine whether a given item is the parent of a virtual branch
  /// (Items in virtual branches are invalid without the parent item, as they represent the item's data node's content, so
  /// they are removed automatically when the parent item of the virtual branch is removed)
  bool IsItemVirtualBranchParent(vtkIdType itemID);

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
  /// \param includeParentItem Determine whether the given item (the parent of the branch) should be included in the search. True by default
  /// \param exceptionNode The function returns false if the only applied transform found is this specified node.
  bool IsAnyNodeInBranchTransformed(vtkIdType itemID, bool includeParentItem=true, vtkMRMLTransformNode* exceptionNode=nullptr);

  /// Deserialize a UID list string (stored in the UID map) into a vector of UID strings
  static void DeserializeUIDList(std::string uidListString, std::vector<std::string>& deserializedUIDList);

  /// Get subject hierarchy items that are referenced from a given item by DICOM.
  /// Finds the series items that contain the SOP instance UIDs that are listed in
  /// the attribute of this item containing the referenced SOP instance UIDs
  /// \sa vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName()
  std::vector<vtkIdType> GetItemsReferencedFromItemByDICOM(vtkIdType itemID);
  /// Python compatibility method to get items that are referenced from a given item by DICOM.
  /// \sa GetItemsReferencedFromItemByDICOM
  void GetItemsReferencedFromItemByDICOM(vtkIdType itemID, vtkIdList* referencingIdList);

  /// Get subject hierarchy items that reference a given item by DICOM.
  /// Finds the series items that contain the SOP instance UID of the item among their
  /// referenced SOP instance UIDs.
  /// \sa vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName()
  std::vector<vtkIdType> GetItemsReferencingItemByDICOM(vtkIdType itemID);
  /// Python compatibility method to get items that are referenced from a given item by DICOM.
  /// \sa GetItemsReferencingItemByDICOM
  void GetItemsReferencingItemByDICOM(vtkIdType itemID, vtkIdList* referencingIdList);

  /// Generate unique item name
  std::string GenerateUniqueItemName(std::string name);

  /// Get number of items under the scene, excluding the scene
  int GetNumberOfItems();

  /// Get number of children for an item
  /// \param recursive If true, then get total number of items in the branch, only direct children otherwise. False by default
  int GetNumberOfItemChildren(vtkIdType itemID, bool recursive=false);

  /// Print subject hierarchy item info on stream
  void PrintItem(vtkIdType itemID, ostream& os, vtkIndent indent);

  /// Ensure the consistency and validity of the SH node in the scene
  static vtkMRMLSubjectHierarchyNode* ResolveSubjectHierarchy(vtkMRMLScene* scene);

protected:
  /// Callback function for all events from the subject hierarchy items
  static void ItemEventCallback(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

protected:
  vtkMRMLSubjectHierarchyNode();
  ~vtkMRMLSubjectHierarchyNode() override;
  vtkMRMLSubjectHierarchyNode(const vtkMRMLSubjectHierarchyNode&);
  void operator=(const vtkMRMLSubjectHierarchyNode&);

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;

  /// Command handling events from subject hierarchy items
  vtkSmartPointer<vtkCallbackCommand> ItemEventCallbackCommand;
};

#endif

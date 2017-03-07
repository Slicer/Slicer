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

// Subject Hierarchy includes
#include "vtkMRMLSubjectHierarchyNode.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
#include <vtkCallbackCommand.h>

// STD includes
#include <sstream>
#include <set>
#include <map>
#include <algorithm>

//----------------------------------------------------------------------------
const vtkIdType vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID = VTK_UNSIGNED_LONG_MAX;
const std::string vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_SEPARATOR = std::string("; ");
const std::string vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_NAME_VALUE_SEPARATOR = std::string(":");

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSubjectHierarchyNode);

//----------------------------------------------------------------------------
class vtkSubjectHierarchyItem : public vtkObject
{
public:
  static vtkSubjectHierarchyItem *New();
  vtkTypeMacro(vtkSubjectHierarchyItem, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  void ReadXMLAttributes(const char** atts);
  void WriteXML(ostream& of, int indent);

  typedef std::vector<vtkSmartPointer<vtkSubjectHierarchyItem> > ChildVector;

public:
  /// Incremental unique identifier of the subject hierarchy item.
  /// This number is used to reference to an item from outside the MRML node
  vtkIdType ID;

  /// Pointer to the data node associated to this subject hierarchy node
  vtkWeakPointer<vtkMRMLNode> DataNode;
  /// Name of the item (only used if there is no data node, for example subject, study, folder)
  std::string Name;

  /// Parent
  vtkSubjectHierarchyItem* Parent;
  /// Ordered list of children
  ChildVector Children;

  /// Level identifier (default levels are Subject and Study)
  std::string Level;

  /// Name of the owner plugin that claimed this node
  std::string OwnerPluginName;

  /// Flag indicating whether the branch under the item is expanded in the view
  bool Expanded;

  /// List of UIDs of this subject hierarchy node
  /// UIDs can be DICOM UIDs, MIDAS urls, etc.
  std::map<std::string, std::string> UIDs;

  /// Attributes (metadata, referenced IDs, etc.)
  std::map<std::string, std::string> Attributes;

  /// Member to temporarily store data node ID in case of scene import.
  /// The ID is first updated then resolved to pointers after import ends, and the string is emptied.
  std::string DataNodeID;
  /// Member to temporarily store parent item ID in case of scene import.
  /// The ID is resolved to pointer after import ends, and the ID is set to INVALID_ITEM_ID.
  vtkIdType ParentItemID;
  /// Member to temporarily store children item IDs in case of scene import.
  /// The IDs are resolved to pointers after import ends, and the vector is emptied.
  std::vector<vtkIdType> ChildrenItemIDs;

// Get/set functions
public:
  /// Add item to tree under parent, specifying basic properties
  /// \param parent Parent item pointer under which this item is inserted
  /// \param dataNode Associated data MRML node
  /// \param level Level string of the item (\sa vtkMRMLSubjectHierarchyConstants)
  /// \param name Name of the item. Only used if there is no data node associated
  ///   (in which case the name of that MRML node is used)
  /// \return ID of the item in the hierarchy that was assigned automatically when adding
  vtkIdType AddItemToTree(
    vtkSubjectHierarchyItem* parent,
    vtkMRMLNode* dataNode=NULL,
    std::string level=vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder(),
    std::string name="" );

  /// Get name of the item. If has data node associated then return name of data node, \sa Name member otherwise
  std::string GetName();

  /// Set UID to the item
  void SetUID(std::string uidName, std::string uidValue);
  /// Get a UID with a given name
  /// \return The UID value if exists, empty string if does not
  std::string GetUID(std::string uidName);
  /// Set attribute to item
  /// \parameter attributeValue Value of attribute. If empty string, then attribute is removed
  void SetAttribute(std::string attributeName, std::string attributeValue);
  /// Remove attribute from item
  /// \return True if attribute was removed, false if not found
  bool RemoveAttribute(std::string attributeName);
  /// Get an attribute with a given name
  /// \return The attribute value if exists, empty string if does not
  std::string GetAttribute(std::string attributeName);
  /// Get attribute names
  /// \return List of attribute names
  std::vector<std::string> GetAttributeNames();
  /// Determine if a given attribute is present in an item.
  /// Especially useful if need to determine whether an attribute value is empty string or the attribute is missing
  bool HasAttribute(std::string attributeName);

// Child related functions
public:
  /// Determine whether this item has any children
  bool HasChildren();
  /// Determine whether this item is the parent of a virtual branch
  /// Items in virtual branches are invalid without the parent item, as they represent the item's data node's content, so
  /// they are removed automatically when the parent item of the virtual branch is removed
  bool IsVirtualBranchParent();
  /// Find child by ID
  /// \param itemID ID to find
  /// \param recursive Flag whether to find only direct children (false) or in the whole branch (true). True by default
  /// \return Item if found, NULL otherwise
  vtkSubjectHierarchyItem* FindChildByID(vtkIdType itemID, bool recursive=true);
  /// Find child by associated data MRML node
  /// \param dataNode Data MRML node to find
  /// \param recursive Flag whether to find only direct children (false) or in the whole branch (true). True by default
  /// \return Item if found, NULL otherwise
  vtkSubjectHierarchyItem* FindChildByDataNode(vtkMRMLNode* dataNode, bool recursive=true);
  /// Find child by UID (exact match)
  /// \param recursive Flag whether to find only direct children (false) or in the whole branch (true). True by default
  /// \return Item if found, NULL otherwise
  vtkSubjectHierarchyItem* FindChildByUID(std::string uidName, std::string uidValue, bool recursive=true);
  /// Find child by UID list (containing). For example find UID in instance UID list
  /// \param recursive Flag whether to find only direct children (false) or in the whole branch (true). True by default
  /// \return Item if found, NULL otherwise
  vtkSubjectHierarchyItem* FindChildByUIDList(std::string uidName, std::string uidValue, bool recursive=true);
  /// Find children by name
  /// \param name Name (or part of a name) to find
  /// \param foundItemIDs List of found item IDs. Needs to be empty when passing as argument!
  /// \param contains Flag whether string containment is enough to determine match. True means a substring is searched
  ///   (case insensitive), false means that the name needs to match exactly (case sensitive)
  /// \param recursive Flag whether to find only direct children (false) or in the whole branch (true). True by default
  /// \return Item if found, NULL otherwise
  void FindChildrenByName( std::string name, std::vector<vtkIdType> &foundItemIDs,
                           bool contains=false, bool recursive=true );
  /// Get data nodes (of a certain type) associated to items in the branch of this item
  void GetDataNodesInBranch(vtkCollection *children, const char* childClass=NULL);
  /// Get all children in the branch recursively
  void GetAllChildren(std::vector<vtkIdType> &childIDs);
  /// Get list of IDs of all direct children of this item
  void GetDirectChildren(std::vector<vtkIdType> &childIDs);
  /// Print all children with correct indentation
  void PrintAllChildren(ostream& os, vtkIndent indent);

  /// Reparent item under new parent
  bool Reparent(vtkSubjectHierarchyItem* newParentItem);
  /// Move item before given item under the same parent
  /// \param beforeItem Item to move given item before. If NULL then insert to the end
  /// \return Success flag
  bool Move(vtkSubjectHierarchyItem* beforeItem);
  /// Get position of item under its parent
  /// \return Position of item under its parent. -1 on failure.
  int GetPositionUnderParent();

  /// Remove given item from children by item pointer
  /// \return Success flag
  bool RemoveChild(vtkSubjectHierarchyItem* item);
  /// Remove given item from children by ID
  /// \return Success flag
  bool RemoveChild(vtkIdType itemID);
  /// Remove all direct children. Do not delete data nodes from the scene. Used in destructor
  void RemoveAllChildren();
  /// Remove all observers from item and its data node if any
  //void RemoveAllObservers(); //TODO: Needed? (the callback object belongs to the SH node so introduction of a new member would be needed)

// Utility functions
public:
  /// Get attribute value from an upper level in the subject hierarchy
  /// \param attributeName Name of the requested attribute
  /// \param level Level of the ancestor item we look for the attribute in
  ///   (e.g. value of vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelStudy()).
  ///   If empty, then look all the way up to the subject and return first attribute found with specified name
  /// \return Attribute value from the lowest level ancestor where the attribute can be found
  std::string GetAttributeFromAncestor(std::string attributeName, std::string level);
  /// Get ancestor subject hierarchy item at a certain level
  /// \param level Level of the ancestor node we start searching.
  vtkSubjectHierarchyItem* GetAncestorAtLevel(std::string level);

public:
  vtkSubjectHierarchyItem();
  ~vtkSubjectHierarchyItem();

private:
  /// Incremental ID used to uniquely identify subject hierarchy items
  static vtkIdType NextSubjectHierarchyItemID;

  vtkSubjectHierarchyItem(const vtkSubjectHierarchyItem&); // Not implemented
  void operator=(const vtkSubjectHierarchyItem&);          // Not implemented
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSubjectHierarchyItem);

vtkIdType vtkSubjectHierarchyItem::NextSubjectHierarchyItemID = 0;

//---------------------------------------------------------------------------
// vtkSubjectHierarchyItem methods

//---------------------------------------------------------------------------
vtkSubjectHierarchyItem::vtkSubjectHierarchyItem()
  : ID(vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  , DataNode(NULL)
  , Name("")
  , Level(vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder())
  , Parent(NULL)
  , OwnerPluginName("")
  , Expanded(true)
  , DataNodeID("")
  , ParentItemID(vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
{
  this->Children.clear();
  this->Attributes.clear();
  this->UIDs.clear();
  this->ChildrenItemIDs.clear();
}

//---------------------------------------------------------------------------
vtkSubjectHierarchyItem::~vtkSubjectHierarchyItem()
{
  this->RemoveAllChildren();

  this->Attributes.clear();
  this->UIDs.clear();
  this->ChildrenItemIDs.clear();
}

//---------------------------------------------------------------------------
vtkIdType vtkSubjectHierarchyItem::AddItemToTree(
  vtkSubjectHierarchyItem* parent, vtkMRMLNode* dataNode/*=NULL*/, std::string level/*="Folder"*/, std::string name/*=""*/ )
{
  this->ID = vtkSubjectHierarchyItem::NextSubjectHierarchyItemID;
  vtkSubjectHierarchyItem::NextSubjectHierarchyItemID++;
  if (vtkSubjectHierarchyItem::NextSubjectHierarchyItemID == VTK_UNSIGNED_LONG_MAX)
    {
    // There is a negligible chance that it reaches maximum, report error in that case
    vtkErrorMacro("AddItemToTree: Next subject hierarchy item ID reached its maximum value! Item is not added to the tree");
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }

  // Set basic properties
  this->DataNode = dataNode;
  this->Name = name;
  this->Level = level;

  this->Parent = parent;
  if (parent)
    {
    vtkSmartPointer<vtkSubjectHierarchyItem> childPointer = vtkSmartPointer<vtkSubjectHierarchyItem>::Take(this);
    this->Parent->Children.push_back(childPointer);
    this->Parent->Modified(); //TODO: Needed?
    }
  else if (name.compare("Scene") || level.compare("Scene")) // Only the scene item can have NULL parent
    {
    vtkErrorMacro("AddItemToTree: Invalid parent of non-scene item to add");
    }

  this->InvokeEvent(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAddedEvent, this);

  return this->ID;
}

//----------------------------------------------------------------------------
void vtkSubjectHierarchyItem::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "ID: " << this->ID << "\n";

  os << indent << "DataNode: " << (this->DataNode.GetPointer() ? this->DataNode->GetID() : "(none)") << "\n";
  os << indent << "Name: " <<
    ( this->DataNode.GetPointer() ? std::string(this->DataNode->GetName()) + " (from data node)" : this->Name ) << "\n";

  os << indent << "Parent: " << (this->Parent ? this->Parent->ID : vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID) << "\n";
  os << indent << "Children: ";
  for (ChildVector::iterator childIt=this->Children.begin(); childIt!=this->Children.end(); ++childIt)
    {
    os << childIt->GetPointer()->ID << " ";
    }
  os << "\n";

  os << indent << "Level: " << this->Level << "\n";

  os << indent << "OwnerPluginName: " << this->OwnerPluginName << "\n";

  os << indent << "Expanded: " << (this->Expanded ? "true" : "false") << "\n";

  if (!this->UIDs.empty())
    {
    os << indent << "UIDs:\n";
    for (std::map<std::string, std::string>::iterator uidIt = this->UIDs.begin(); uidIt != this->UIDs.end(); ++uidIt)
      {
      os << indent.GetNextIndent() << uidIt->first << ":" << uidIt->second << "\n";
      }
    }

  if (!this->Attributes.empty())
    {
    os << indent << "Attributes:\n";
    for (std::map<std::string, std::string>::iterator attIt = this->Attributes.begin(); attIt != this->Attributes.end(); ++attIt)
      {
      os << indent.GetNextIndent() << attIt->first << ":" << attIt->second << "\n";
      }
    }
}

//---------------------------------------------------------------------------
void vtkSubjectHierarchyItem::ReadXMLAttributes(const char** atts)
{
  const char* attName = NULL;
  const char* attValue = NULL;

  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    //TODO:
    }

  vtkErrorMacro("ReadXMLAttributes: Not yet implemented");
}

//---------------------------------------------------------------------------
void vtkSubjectHierarchyItem::WriteXML(ostream& of, int nIndent)
{
  vtkIndent indent(nIndent);

  of << indent << " ID=\"" << this->ID << "\"";

  of << indent << " DataNode=\"" << (this->DataNode.GetPointer() ? this->DataNode->GetID() : "NULL") << "\"";
  of << indent << " Name=\"" << this->Name << "\"";

  of << indent << " Parent=\"" << (this->Parent ? this->Parent->ID : vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID) << "\"";
  of << indent << " Children=\"";
  for (ChildVector::iterator childIt=this->Children.begin(); childIt!=this->Children.end(); ++childIt)
    {
    of << childIt->GetPointer()->ID << vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_SEPARATOR;
    }
  of << "\"";

  of << indent << " Level=\"" << this->Level << "\"";

  of << indent << " OwnerPluginName=\"" << this->OwnerPluginName << "\"";

  of << indent << " Expanded=\"" << (this->Expanded ? "true" : "false") << "\"";

  of << indent << " UIDs=\"";
  for (std::map<std::string, std::string>::iterator uidIt = this->UIDs.begin(); uidIt != this->UIDs.end(); ++uidIt)
    {
    of << uidIt->first << vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_NAME_VALUE_SEPARATOR
       << uidIt->second << vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_SEPARATOR;
    }
  of << "\"";

  of << indent << " Attributes=\"";
  for (std::map<std::string, std::string>::iterator attIt = this->Attributes.begin(); attIt != this->Attributes.end(); ++attIt)
    {
    of << attIt->first << vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_NAME_VALUE_SEPARATOR
       << attIt->second << vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_SEPARATOR;
    }
  of << "\"";
}

//---------------------------------------------------------------------------
std::string vtkSubjectHierarchyItem::GetName()
{
  if (this->DataNode.GetPointer() && this->DataNode->GetName())
    {
    return std::string(this->DataNode->GetName());
    }
  return this->Name;
}

//---------------------------------------------------------------------------
bool vtkSubjectHierarchyItem::HasChildren()
{
  return !this->Children.empty();
}

//---------------------------------------------------------------------------
bool vtkSubjectHierarchyItem::IsVirtualBranchParent()
{
  return !this->GetAttribute(
    vtkMRMLSubjectHierarchyConstants::GetVirtualBranchSubjectHierarchyNodeAttributeName() ).empty();
}

//---------------------------------------------------------------------------
vtkSubjectHierarchyItem* vtkSubjectHierarchyItem::FindChildByID(vtkIdType itemID, bool recursive/*=true*/)
{
  ChildVector::iterator childIt;
  for (childIt=this->Children.begin(); childIt!=this->Children.end(); ++childIt)
    {
    vtkSubjectHierarchyItem* currentItem = childIt->GetPointer();
    if (itemID == currentItem->ID)
      {
      return currentItem;
      }
    if (recursive)
      {
      vtkSubjectHierarchyItem* foundItemInBranch = currentItem->FindChildByID(itemID);
      if (foundItemInBranch)
        {
        return foundItemInBranch;
        }
      }
    }
  return NULL;
}

//---------------------------------------------------------------------------
vtkSubjectHierarchyItem* vtkSubjectHierarchyItem::FindChildByDataNode(vtkMRMLNode* dataNode, bool recursive/*=true*/)
{
  if (!dataNode)
    {
    return NULL;
    }

  ChildVector::iterator childIt;
  for (childIt=this->Children.begin(); childIt!=this->Children.end(); ++childIt)
    {
    vtkSubjectHierarchyItem* currentItem = childIt->GetPointer();
    if (dataNode == currentItem->DataNode.GetPointer())
      {
      return currentItem;
      }
    if (recursive)
      {
      vtkSubjectHierarchyItem* foundItemInBranch = currentItem->FindChildByDataNode(dataNode);
      if (foundItemInBranch)
        {
        return foundItemInBranch;
        }
      }
    }
  return NULL;
}

//---------------------------------------------------------------------------
vtkSubjectHierarchyItem* vtkSubjectHierarchyItem::FindChildByUID(std::string uidName, std::string uidValue, bool recursive/*=true*/)
{
  if (uidName.empty() || uidValue.empty())
    {
    return NULL;
    }
  ChildVector::iterator childIt;
  for (childIt=this->Children.begin(); childIt!=this->Children.end(); ++childIt)
    {
    vtkSubjectHierarchyItem* currentItem = childIt->GetPointer();
    if (!currentItem->GetUID(uidName).compare(uidValue))
      {
      return currentItem;
      }
    if (recursive)
      {
      vtkSubjectHierarchyItem* foundItemInBranch = currentItem->FindChildByUID(uidName, uidValue);
      if (foundItemInBranch)
        {
        return foundItemInBranch;
        }
      }
    }
  return NULL;
}

//---------------------------------------------------------------------------
vtkSubjectHierarchyItem* vtkSubjectHierarchyItem::FindChildByUIDList(std::string uidName, std::string uidValue, bool recursive/*=true*/)
{
  if (uidName.empty() || uidValue.empty())
    {
    return NULL;
    }
  ChildVector::iterator childIt;
  for (childIt=this->Children.begin(); childIt!=this->Children.end(); ++childIt)
    {
    vtkSubjectHierarchyItem* currentItem = childIt->GetPointer();
    if (currentItem->GetUID(uidName).find(uidValue) != std::string::npos)
      {
      return currentItem;
      }
    if (recursive)
      {
      vtkSubjectHierarchyItem* foundItemInBranch = currentItem->FindChildByUID(uidName, uidValue);
      if (foundItemInBranch)
        {
        return foundItemInBranch;
        }
      }
    }
  return NULL;
}

//---------------------------------------------------------------------------
void vtkSubjectHierarchyItem::FindChildrenByName(std::string name, std::vector<vtkIdType> &foundItemIDs, bool contains/*=false*/, bool recursive/*=true*/)
{
  if (contains && !name.empty())
    {
    std::transform(name.begin(), name.end(), name.begin(), ::tolower); // Make it lowercase for case-insensitive comparison
    }

  ChildVector::iterator childIt;
  for (childIt=this->Children.begin(); childIt!=this->Children.end(); ++childIt)
    {
    vtkSubjectHierarchyItem* currentItem = childIt->GetPointer();
    std::string currentName = currentItem->GetName();
    if (name.empty())
      {
      // If given name is empty (e.g. GetAllChildrenIDs is called), then it is quicker not to do the unnecessary string operations
      foundItemIDs.push_back(currentItem->ID);
      }
    else if (contains)
      {
      std::transform(currentName.begin(), currentName.end(), currentName.begin(), ::tolower); // Make it lowercase for case-insensitive comparison
      if (currentName.find(name) != std::string::npos)
        {
        foundItemIDs.push_back(currentItem->ID);
        }
      }
    else if (!currentName.compare(name))
      {
      foundItemIDs.push_back(currentItem->ID);
      }
    if (recursive)
      {
      currentItem->FindChildrenByName(name, foundItemIDs, contains);
      }
    }
}

//---------------------------------------------------------------------------
void vtkSubjectHierarchyItem::GetDataNodesInBranch(vtkCollection* dataNodeCollection, const char* childClass/*=NULL*/)
{
  if (dataNodeCollection == NULL)
    {
    vtkErrorMacro("GetDataNodesInBranch: Output collection must be created before calling the method");
    return;
    }
  std::string nodeClass("vtkMRMLNode");
  if (childClass)
    {
    nodeClass = childClass;
    }

  ChildVector::iterator childIt;
  for (childIt=this->Children.begin(); childIt!=this->Children.end(); ++childIt)
    {
    vtkSubjectHierarchyItem* currentItem = childIt->GetPointer();
    vtkMRMLNode* currentDataNode = currentItem->DataNode.GetPointer();
    if (currentDataNode)
      {
      if (currentDataNode->IsA(nodeClass.c_str()))
        {
        dataNodeCollection->AddItem(currentDataNode);
        }
      }

    // Find associated data nodes recursively
    currentItem->GetDataNodesInBranch(dataNodeCollection, childClass);
    }
}

//---------------------------------------------------------------------------
void vtkSubjectHierarchyItem::GetAllChildren(std::vector<vtkIdType> &childIDs)
{
  childIDs.clear();
  this->FindChildrenByName("", childIDs, true);
}

//---------------------------------------------------------------------------
void vtkSubjectHierarchyItem::GetDirectChildren(std::vector<vtkIdType> &childIDs)
{
  childIDs.clear();
  for (ChildVector::iterator childIt=this->Children.begin(); childIt!=this->Children.end(); ++childIt)
    {
    childIDs.push_back((*childIt)->ID);
    }
}

//---------------------------------------------------------------------------
void vtkSubjectHierarchyItem::PrintAllChildren(ostream& os, vtkIndent indent)
{
  ChildVector::iterator childIt;
  for (childIt=this->Children.begin(); childIt!=this->Children.end(); ++childIt)
    {
    vtkSubjectHierarchyItem* currentItem = childIt->GetPointer();
    currentItem->PrintSelf(os, indent);
    os << indent << "----\n"; // Add separator for readability

    currentItem->PrintAllChildren(os, indent.GetNextIndent());
    }
}

//---------------------------------------------------------------------------
bool vtkSubjectHierarchyItem::Reparent(vtkSubjectHierarchyItem* newParentItem)
{
  if (!newParentItem)
    {
    vtkErrorMacro("Reparent: Invalid new parent");
    return false;
    }

  vtkSubjectHierarchyItem* formerParent = this->Parent;

  // Nothing to do if given parent item is the same as current parent
  if (formerParent == newParentItem)
    {
    return true;
    }

  // Remove item from former parent
  vtkSubjectHierarchyItem::ChildVector::iterator childIt;
  for (childIt=formerParent->Children.begin(); childIt!=formerParent->Children.end(); ++childIt)
    {
    if (this == childIt->GetPointer())
      {
      break;
      }
    }
  if (childIt == formerParent->Children.end())
    {
    vtkErrorMacro("Reparent: Subject hierarchy item '" << this->GetName() << "' not found under item '" << formerParent->GetName() << "'");
    return false;
    }

  // Prevent deletion of the item from memory until the events are processed
  vtkSmartPointer<vtkSubjectHierarchyItem> removedItem = (*childIt);
  // Remove item from former parent
  formerParent->Children.erase(childIt);

  // Add item to new parent
  this->Parent = newParentItem;
  vtkSmartPointer<vtkSubjectHierarchyItem> childPointer = this;
  newParentItem->Children.push_back(childPointer);

  // Invoke modified events on all affected items
  formerParent->Modified();
  newParentItem->Modified();
  this->Modified();

  return true;
}

//---------------------------------------------------------------------------
bool vtkSubjectHierarchyItem::Move(vtkSubjectHierarchyItem* beforeItem)
{
  if (!this->Parent)
    {
    vtkErrorMacro("Move: Scene item cannot be moved");
    return false;
    }

  // Remove item from parent
  ChildVector::iterator childIt;
  for (childIt=this->Parent->Children.begin(); childIt!=this->Parent->Children.end(); ++childIt)
    {
    if (this == childIt->GetPointer())
      {
      break;
      }
    }
  if (childIt == this->Parent->Children.end())
    {
    vtkErrorMacro("Move: Failed to find subject hierarchy item '" << this->GetName()
      << "' in its parent '" << this->Parent->GetName() << "'");
    return false;
    }

  // Prevent deletion of the item from memory until the events are processed
  vtkSmartPointer<vtkSubjectHierarchyItem> removedItem = (*childIt);
  // Remove item from former parent
  this->Parent->Children.erase(childIt);

  // Re-insert item to the requested position (before beforeItem)
  if (!beforeItem)
    {
    vtkSmartPointer<vtkSubjectHierarchyItem> childPointer = vtkSmartPointer<vtkSubjectHierarchyItem>::Take(this);
    this->Parent->Children.push_back(childPointer);
    return true;
    }

  for (childIt=this->Parent->Children.begin(); childIt!=this->Parent->Children.end(); ++childIt)
    {
    if (beforeItem == childIt->GetPointer())
      {
      break;
      }
    }
  if (childIt == this->Parent->Children.end())
    {
    vtkErrorMacro("Move: Failed to find subject hierarchy item '" << beforeItem->GetName()
      << "' as insertion position in item '" << this->Parent->GetName() << "'");
    return false;
    }
  this->Parent->Children.insert(childIt, this);
  this->Modified();

  return true;
}

//---------------------------------------------------------------------------
int vtkSubjectHierarchyItem::GetPositionUnderParent()
{
  if (!this->Parent)
    {
    // Scene item is the only item in its branch
    return 0;
    }

  int position = 0;
  ChildVector::iterator childIt;
  for (childIt=this->Parent->Children.begin(); childIt!=this->Parent->Children.end(); ++childIt, ++position)
    {
    if (childIt->GetPointer() == this)
      {
      return position;
      }
    }
  // Failed to find item
  vtkErrorMacro("Failed to find subject hierarchy item " << this->Name << " under its parent");
  return -1;
}

//---------------------------------------------------------------------------
bool vtkSubjectHierarchyItem::RemoveChild(vtkSubjectHierarchyItem* item)
{
  if (!item)
    {
    vtkErrorMacro("RemoveChild: Invalid subject hierarchy item given to remove from item '" << this->GetName() << "'");
    return false;
    }

  ChildVector::iterator childIt;
  for (childIt=this->Children.begin(); childIt!=this->Children.end(); ++childIt)
    {
    if (item == childIt->GetPointer())
      {
      break;
      }
    }
  if (childIt == this->Children.end())
    {
    vtkErrorMacro("RemoveChild: Subject hierarchy item '" << item->GetName() << "' not found in item '" << this->GetName() << "'");
    return false;
    }

  // If child is a virtual branch (meaning that its children are invalid without the item,
  // as they represent the item's data node's content), then remove virtual branch
  if ((*childIt)->IsVirtualBranchParent())
    {
    (*childIt)->RemoveAllChildren();
    }

  // Remove child
  this->InvokeEvent(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAboutToBeRemovedEvent, item);
  this->Children.erase(childIt);
  this->InvokeEvent(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemRemovedEvent, item);
  this->Modified();

  return true;
}

//---------------------------------------------------------------------------
bool vtkSubjectHierarchyItem::RemoveChild(vtkIdType itemID)
{
  ChildVector::iterator childIt;
  for (childIt=this->Children.begin(); childIt!=this->Children.end(); ++childIt)
    {
    if (itemID == (*childIt)->ID)
      {
      break;
      }
    }
  if (childIt == this->Children.end())
    {
    vtkErrorMacro("RemoveChild: Subject hierarchy item with ID " << itemID << " not found in item '" << this->GetName() << "'");
    return false;
    }

  // Prevent deletion of the item from memory until the events are processed
  vtkSmartPointer<vtkSubjectHierarchyItem> removedItem = (*childIt);

  // If child is a virtual branch (meaning that its children are invalid without the item,
  // as they represent the item's data node's content), then remove virtual branch
  if (removedItem->IsVirtualBranchParent())
    {
    removedItem->RemoveAllChildren();
    }

  // Remove child
  this->InvokeEvent(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAboutToBeRemovedEvent, removedItem.GetPointer());
  this->Children.erase(childIt);
  this->InvokeEvent(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemRemovedEvent, removedItem.GetPointer());
  this->Modified();

  return true;
}

//---------------------------------------------------------------------------
void vtkSubjectHierarchyItem::RemoveAllChildren()
{
  std::vector<vtkIdType> childIDs;
  this->GetDirectChildren(childIDs);

  std::vector<vtkIdType>::iterator childIt;
  for (childIt=childIDs.begin(); childIt!=childIDs.end(); ++childIt)
    {
    this->RemoveChild(*childIt);
    }
}

//---------------------------------------------------------------------------
//void vtkSubjectHierarchyItem::RemoveAllObservers()
//{
//  if (this->DataNode)
//    {
//    this->DataNode->RemoveObservers(vtkCommand::ModifiedEvent, this->ItemEventCallbackCommand);
//    this->DataNode->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent, this->ItemEventCallbackCommand);
//    this->DataNode->RemoveObservers(vtkMRMLDisplayableNode::DisplayModifiedEvent, this->ItemEventCallbackCommand);
//    }
//}

//---------------------------------------------------------------------------
void vtkSubjectHierarchyItem::SetUID(std::string uidName, std::string uidValue)
{
  // Use the find function to prevent adding an empty UID to the map
  if (this->UIDs.find(uidName) != this->UIDs.end())
    {
    // Log warning if the new UID value is different than the one already set
    if (this->UIDs[uidName].compare(uidValue))
      {
      vtkWarningMacro( "SetUID: UID with name '" << uidName << "' already exists in subject hierarchy item '" << this->GetName()
        << "' with value '" << this->UIDs[uidName] << "'. Replacing it with value '" << uidValue << "'!" );
      }
    else
      {
      return; // Do nothing if the UID values match
      }
    }
  this->UIDs[uidName] = uidValue;
  this->InvokeEvent(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemUIDAddedEvent, this);
  this->Modified();
}

//---------------------------------------------------------------------------
std::string vtkSubjectHierarchyItem::GetUID(std::string uidName)
{
  // Use the find function to prevent adding an empty UID to the map
  if (this->UIDs.find(uidName) != this->UIDs.end())
    {
    return this->UIDs[uidName];
    }
  return std::string();
}

//---------------------------------------------------------------------------
void vtkSubjectHierarchyItem::SetAttribute(std::string attributeName, std::string attributeValue)
{
  if (attributeName.empty())
    {
    vtkErrorMacro("SetAttribute: Name parameter is expected to have at least one character.");
    return;
    }
  // Use the find function to prevent adding an empty attribute to the map
  if ( this->Attributes.find(attributeName) != this->Attributes.end()
    && !attributeValue.compare(this->Attributes[attributeName]) )
    {
    return; // Attribute to set is same as original value, nothing to do
    }
  this->Attributes[attributeName] = attributeValue;
  this->Modified();
}

//---------------------------------------------------------------------------
bool vtkSubjectHierarchyItem::RemoveAttribute(std::string attributeName)
{
  if (this->Attributes.size() == 0)
    {
    return false;
    }

  // Use the find function to prevent adding an empty attribute to the map
  if (this->Attributes.find(attributeName) != this->Attributes.end())
    {
    this->Attributes.erase(attributeName);
    return true;
    }
  return false;
}

//---------------------------------------------------------------------------
std::string vtkSubjectHierarchyItem::GetAttribute(std::string attributeName)
{
  // Use the find function to prevent adding an empty attribute to the map
  if ( this->Attributes.size() > 0
    && this->Attributes.find(attributeName) != this->Attributes.end() )
    {
    return this->Attributes[attributeName];
    }
  return std::string();
}

//---------------------------------------------------------------------------
std::vector<std::string> vtkSubjectHierarchyItem::GetAttributeNames()
{
  std::vector<std::string> attributeNameList;
  std::map<std::string, std::string>::iterator attributeIt;
  for (attributeIt=this->Attributes.begin(); attributeIt!=this->Attributes.end(); ++attributeIt)
    {
    attributeNameList.push_back(attributeIt->first);
    }
  return attributeNameList;
}

//---------------------------------------------------------------------------
bool vtkSubjectHierarchyItem::HasAttribute(std::string attributeName)
{
  return ( this->Attributes.size() > 0
        && this->Attributes.find(attributeName) != this->Attributes.end() );
}

//---------------------------------------------------------------------------
std::string vtkSubjectHierarchyItem::GetAttributeFromAncestor(std::string attributeName, std::string level)
{
  if (attributeName.empty())
    {
    vtkErrorMacro("GetAttributeFromAncestor: Empty attribute name!");
    return std::string();
    }

  std::string attributeValue("");
  vtkSubjectHierarchyItem* currentItem = this;
  while (currentItem && currentItem->Parent)
    {
    currentItem = currentItem->Parent;
    if (!currentItem)
      {
      break;
      }
    else if (!level.empty() && currentItem->Level.compare(level))
      {
      continue;
      }

    attributeValue = currentItem->GetAttribute(attributeName);
    if (!attributeValue.empty())
      {
      return attributeValue;
      }
    }

  return attributeValue;
}

//---------------------------------------------------------------------------
vtkSubjectHierarchyItem* vtkSubjectHierarchyItem::GetAncestorAtLevel(std::string level)
{
  if (level.empty())
    {
    vtkErrorMacro("GetAncestorAtLevel: Empty subject hierarchy level");
    return NULL;
    }

  // We do not return source node even if it is at the requested level, we only look in the ancestors
  vtkSubjectHierarchyItem* currentItem = this;
  while (currentItem && currentItem->Parent)
    {
    currentItem = currentItem->Parent;
    if (currentItem && !currentItem->Level.compare(level))
      {
      // Level found
      return currentItem;
      }
    }

  vtkWarningMacro("GetAncestorAtLevel: No ancestor found for item '" << this->GetName() << "' at level '" << level);
  return NULL;
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class vtkMRMLSubjectHierarchyNode::vtkInternal
{
public:
  vtkInternal(vtkMRMLSubjectHierarchyNode* external);
  ~vtkInternal();

  /// Utility function to find item in whole subject hierarchy by ID, including the scene item
  vtkSubjectHierarchyItem* FindItemByID(vtkIdType itemID);

public:
  vtkSubjectHierarchyItem* SceneItem;
  vtkIdType SceneItemID;

private:
  vtkMRMLSubjectHierarchyNode* External;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode::vtkInternal::vtkInternal(vtkMRMLSubjectHierarchyNode* external)
: External(external)
{
  this->SceneItem = vtkSubjectHierarchyItem::New();
  this->SceneItemID = this->SceneItem->AddItemToTree(NULL, NULL, "Scene", "Scene");
}

//---------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode::vtkInternal::~vtkInternal()
{
  if (this->SceneItem)
    {
    this->SceneItem->Delete();
    this->SceneItem = NULL;
    }
}

//---------------------------------------------------------------------------
vtkSubjectHierarchyItem* vtkMRMLSubjectHierarchyNode::vtkInternal::FindItemByID(vtkIdType itemID)
{
  // If scene item ID was given, then simply return the scene item
  if (itemID == this->SceneItemID)
    {
    return this->SceneItem;
    }

  // Find item under the scene item
  return this->SceneItem->FindChildByID(itemID);
}

//----------------------------------------------------------------------------
// vtkMRMLSubjectHierarchyNode members
//----------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode::vtkMRMLSubjectHierarchyNode()
{
  this->Internal = new vtkInternal(this);

  this->ItemEventCallbackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->ItemEventCallbackCommand->SetClientData(reinterpret_cast<void*>(this));
  this->ItemEventCallbackCommand->SetCallback(vtkMRMLSubjectHierarchyNode::ItemEventCallback);
}

//----------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode::~vtkMRMLSubjectHierarchyNode()
{
  delete this->Internal;
  this->Internal = NULL;

  // Make sure this callback cannot call this object
  this->ItemEventCallbackCommand->SetClientData(NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  if (this->Internal->SceneItem == NULL)
    {
    os << indent << " No items in the tree" << "\n";
    }
  else
    {
    os << indent << "Items:\n";
    this->Internal->SceneItem->PrintAllChildren(os, indent.GetNextIndent());
    }
}

//----------------------------------------------------------------------------
const char* vtkMRMLSubjectHierarchyNode::GetNodeTagName()
{
  return "SubjectHierarchy";
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  //TODO:
  //const char* attName;
  //const char* attValue;
  //while (*atts != NULL)
  //  {
  //  attName = *(atts++);
  //  attValue = *(atts++);
  //  if (!strcmp(attName, "Level"))
  //    {
  //    this->SetLevel(attValue);
  //    }
  //  else if (!strcmp(attName, "OwnerPluginName"))
  //    {
  //    this->SetOwnerPluginName(attValue);
  //    }
  //  else if (!strcmp(attName, "UIDs"))
  //    {
  //    std::stringstream ss;
  //    ss << attValue;
  //    std::string valueStr = ss.str();

  //    this->UIDs.clear();
  //    size_t itemSeparatorPosition = valueStr.find(vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_UID_ITEM_SEPARATOR);
  //    while (itemSeparatorPosition != std::string::npos)
  //      {
  //      std::string itemStr = valueStr.substr(0, itemSeparatorPosition);
  //      size_t nameValueSeparatorPosition = itemStr.find(vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR);

  //      std::string name = itemStr.substr(0, nameValueSeparatorPosition);
  //      std::string value = itemStr.substr(nameValueSeparatorPosition + vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR.size());
  //      this->AddUID(name, value);

  //      valueStr = valueStr.substr(itemSeparatorPosition + vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_UID_ITEM_SEPARATOR.size());
  //      itemSeparatorPosition = valueStr.find(vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_UID_ITEM_SEPARATOR);
  //      }
  //    if (! valueStr.empty() )
  //      {
  //      std::string itemStr = valueStr.substr(0, itemSeparatorPosition);
  //      size_t tagLevelSeparatorPosition = itemStr.find(vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR);

  //      std::string name = itemStr.substr(0, tagLevelSeparatorPosition);
  //      std::string value = itemStr.substr(tagLevelSeparatorPosition + vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR.size());
  //      this->AddUID(name, value);
  //      }
  //    }
  //  }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);

  vtkIndent indent(nIndent);

  //TODO:
  //of << indent << " Level=\""
  //  << (this->Level ? this->Level : "NULL" ) << "\"";

  //of << indent << " OwnerPluginName=\""
  //  << (this->OwnerPluginName ? this->OwnerPluginName : "NULL" ) << "\"";

  //of << indent << " UIDs=\"";
  //for (std::map<std::string, std::string>::iterator uidsIt = this->UIDs.begin(); uidsIt != this->UIDs.end(); ++uidsIt)
  //  {
  //  of << uidsIt->first << vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR
  //    << uidsIt->second << vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_UID_ITEM_SEPARATOR;
  //  }
  //of << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLSubjectHierarchyNode *node = (vtkMRMLSubjectHierarchyNode*)anode;

  //TODO:
  //this->SetLevel(node->Level);
  //this->SetOwnerPluginName(node->OwnerPluginName);

  //this->UIDs = node->GetUIDs();
  vtkErrorMacro("Not implemented!");

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
vtkIdType vtkMRMLSubjectHierarchyNode::GetSceneItemID()
{
  return this->Internal->SceneItemID;
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLSubjectHierarchyNode::GetItemDataNode(vtkIdType itemID)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("GetItemDataNode: Failed to find subject hierarchy item by ID " << itemID);
    return NULL;
    }

  return item->DataNode.GetPointer();
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::SetItemName(vtkIdType itemID, std::string name)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("SetItemName: Failed to find subject hierarchy item by ID " << itemID);
    return;
    }

  // Set data node name if there is a data node
  // (because it is used when data node exists, so this is how it's consistent)
  bool nameChanged = false;
  if (item->DataNode)
    {
    if ( !item->DataNode->GetName()
      || (item->DataNode->GetName() && name.compare(item->DataNode->GetName())) )
      {
      item->Name = "";
      item->DataNode->SetName(name.c_str());
      nameChanged = true;
      }
    }
  else
    {
    if (name.compare(item->Name))
      {
      item->Name = name;
      nameChanged = true;
      }
    }

  if (nameChanged)
    {
    this->InvokeCustomModifiedEvent(SubjectHierarchyItemModifiedEvent, (void*)&itemID);
    //this->Modified(); //TODO: Needed? SH node modified event should be used for updating the whole view (every item)
    }
}

//----------------------------------------------------------------------------
std::string vtkMRMLSubjectHierarchyNode::GetItemName(vtkIdType itemID)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("GetItemName: Failed to find subject hierarchy item by ID " << itemID);
    return std::string();
    }

  return item->GetName();
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::SetItemLevel(vtkIdType itemID, std::string level)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("SetItemLevel: Failed to find subject hierarchy item by ID " << itemID);
    return;
    }

  item->Level = level;
  this->InvokeCustomModifiedEvent(SubjectHierarchyItemModifiedEvent, (void*)&itemID);
  //this->Modified(); //TODO: Needed? SH node modified event should be used for updating the whole view (every item)
}

//----------------------------------------------------------------------------
std::string vtkMRMLSubjectHierarchyNode::GetItemLevel(vtkIdType itemID)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("GetItemLevel: Failed to find subject hierarchy item by ID " << itemID);
    return std::string();
    }

  return item->Level;
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::SetItemOwnerPluginName(vtkIdType itemID, std::string owherPluginName)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("SetItemOwnerPluginName: Failed to find subject hierarchy item by ID " << itemID);
    return;
    }

  item->OwnerPluginName = owherPluginName;
  this->InvokeCustomModifiedEvent(SubjectHierarchyItemModifiedEvent, (void*)&itemID);
  //this->Modified(); //TODO: Needed? SH node modified event should be used for updating the whole view (every item)
}

//----------------------------------------------------------------------------
std::string vtkMRMLSubjectHierarchyNode::GetItemOwnerPluginName(vtkIdType itemID)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("GetItemOwnerPluginName: Failed to find subject hierarchy item by ID " << itemID);
    return std::string();
    }

  return item->OwnerPluginName;
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::SetItemExpanded(vtkIdType itemID, bool expanded)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("SetItemExpanded: Failed to find subject hierarchy item by ID " << itemID);
    return;
    }

  item->Expanded = expanded;
}

//----------------------------------------------------------------------------
bool vtkMRMLSubjectHierarchyNode::GetItemExpanded(vtkIdType itemID)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("GetItemExpanded: Failed to find subject hierarchy item by ID " << itemID);
    return false;
    }

  return item->Expanded;
}

//----------------------------------------------------------------------------
int vtkMRMLSubjectHierarchyNode::GetItemPositionUnderParent(vtkIdType itemID)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("GetItemPositionUnderParent: Failed to find subject hierarchy item by ID " << itemID);
    return false;
    }
  return item->GetPositionUnderParent();
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::SetItemUID(vtkIdType itemID, std::string uidName, std::string uidValue)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("SetItemUID: Failed to find subject hierarchy item by ID " << itemID);
    return;
    }

  item->SetUID(uidName, uidValue); // Events are invoked within this call
  //this->Modified(); //TODO: Needed? SH node modified event should be used for updating the whole view (every item)
}

//----------------------------------------------------------------------------
std::string vtkMRMLSubjectHierarchyNode::GetItemUID(vtkIdType itemID, std::string uidName)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("GetItemUID: Failed to find subject hierarchy item by ID " << itemID);
    return std::string();
    }

  return item->GetUID(uidName);
}


//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::SetItemAttribute(vtkIdType itemID, std::string attributeName, std::string attributeValue)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("SetItemAttribute: Failed to find subject hierarchy item by ID " << itemID);
    return;
    }

  item->SetAttribute(attributeName, attributeValue); // Events are invoked within this call
  //this->Modified(); //TODO: Needed? SH node modified event should be used for updating the whole view (every item)
}

//---------------------------------------------------------------------------
bool vtkMRMLSubjectHierarchyNode::RemoveItemAttribute(vtkIdType itemID, std::string attributeName)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("RemoveItemAttribute: Failed to find subject hierarchy item by ID " << itemID);
    return false;
    }

  bool result = item->RemoveAttribute(attributeName); // Events are invoked within this call
  //this->Modified(); //TODO: Needed? SH node modified event should be used for updating the whole view (every item)
  return result;
}

//----------------------------------------------------------------------------
std::string vtkMRMLSubjectHierarchyNode::GetItemAttribute(vtkIdType itemID, std::string attributeName)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("GetItemAttribute: Failed to find subject hierarchy item by ID " << itemID);
    return std::string();
    }

  return item->GetAttribute(attributeName);
}

//---------------------------------------------------------------------------
std::vector<std::string> vtkMRMLSubjectHierarchyNode::GetItemAttributeNames(vtkIdType itemID)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("GetItemAttributeNames: Failed to find subject hierarchy item by ID " << itemID);
    return std::vector<std::string>();
    }

  return item->GetAttributeNames();
}

//---------------------------------------------------------------------------
bool vtkMRMLSubjectHierarchyNode::HasItemAttribute(vtkIdType itemID, std::string attributeName)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("HasItemAttribute: Failed to find subject hierarchy item by ID " << itemID);
    return false;
    }

  return item->HasAttribute(attributeName);
}

//---------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::ItemModified(vtkIdType itemID)
{
  // Not used, but we need to make sure that the item exists
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("ItemModified: Failed to find subject hierarchy item by ID " << itemID);
    return;
    }

  // Invoke the node event directly, thus saving an extra callback round
  this->InvokeCustomModifiedEvent(SubjectHierarchyItemModifiedEvent, (void*)&itemID);
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLSubjectHierarchyNode::CreateItem(
  vtkIdType parentItemID,
  vtkMRMLNode* dataNode/*=NULL*/,
  std::string level/*=vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder()*/,
  std::string name/*=""*/ )
{
  // Use existing subject hierarchy item if found (only one subject hierarchy item can be associated with a data node)
  vtkIdType itemID = INVALID_ITEM_ID;
  if (dataNode)
    {
    itemID = this->GetItemByDataNode(dataNode);
    }
  if (itemID != INVALID_ITEM_ID)
    {
    // Set properties if item already existed for data node
    // This should be the case every time data node is not NULL, because subject hierarchy items are added automatically
    // for supported data MRML nodes (supported = there is a plugin that can "own" it)
    vtkSubjectHierarchyItem* item = this->Internal->SceneItem->FindChildByID(itemID);

    // No matter what the user gave, the name of the data node is used (because it exists)
    item->Name = "";
    // Reparent if given parent is valid and different than the current one
    if (item->Parent && item->Parent->ID != parentItemID && parentItemID != INVALID_ITEM_ID)
      {
      vtkSubjectHierarchyItem* parentItem = this->Internal->FindItemByID(parentItemID);
      if (!parentItem)
        {
        vtkErrorMacro("CreateItem: Failed to find subject hierarchy item (to be the parent) by ID " << parentItemID);
        return INVALID_ITEM_ID;
        }
      item->Reparent(parentItem);
      }

    if (item->Level.compare(level))
      {
      item->Level = level;
      }
    }
  // No data node was specified, or no subject hierarchy item was found for the given data node
  else
    {
    vtkSubjectHierarchyItem* parentItem = this->Internal->FindItemByID(parentItemID);
    if (!parentItem)
      {
      vtkErrorMacro("CreateItem: Failed to find parent subject hierarchy item by ID " << parentItemID);
      return INVALID_ITEM_ID;
      }

    // Create subject hierarchy item
    vtkSubjectHierarchyItem* item = vtkSubjectHierarchyItem::New();

    // Make item connections
    item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAddedEvent, this->ItemEventCallbackCommand);
    item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAboutToBeRemovedEvent, this->ItemEventCallbackCommand);
    item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemRemovedEvent, this->ItemEventCallbackCommand);
    item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemUIDAddedEvent, this->ItemEventCallbackCommand);
    item->AddObserver(vtkCommand::ModifiedEvent, this->ItemEventCallbackCommand);

    // Observe data node events
    if (dataNode)
      {
      dataNode->AddObserver(vtkCommand::ModifiedEvent, this->ItemEventCallbackCommand);
      dataNode->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent, this->ItemEventCallbackCommand);
      dataNode->AddObserver(vtkMRMLDisplayableNode::DisplayModifiedEvent, this->ItemEventCallbackCommand);
      }

    // Add item to the tree
    itemID = item->AddItemToTree( parentItem, dataNode, level, (dataNode ? "" : name) ); // Name is not used if valid data node is given
    }

  return itemID;
}

//----------------------------------------------------------------------------
bool vtkMRMLSubjectHierarchyNode::RemoveItem(vtkIdType itemID, bool removeDataNode/*=true*/, bool recursive/*=true*/)
{
  vtkSubjectHierarchyItem* item = this->Internal->SceneItem->FindChildByID(itemID);
  if (!item)
    {
    vtkErrorMacro("RemoveItem: Failed to find non-scene subject hierarchy item by ID " << itemID);
    return false;
    }

  // Remove all children if recursive deletion is requested or if it's the parent of a virtual branch
  // Start with the leaf nodes so that triggered updates are faster (no reparenting done after deleting intermediate items)
  if (recursive || item->IsVirtualBranchParent())
    {
    std::vector<vtkIdType> childIDs;
    item->GetAllChildren(childIDs);
    while (childIDs.size())
      {
      // Remove first leaf node found
      // (or if the parent of a virtual branch, in which the items are automatically removed when their parent is removed)
      std::vector<vtkIdType>::iterator childIt;
      for (childIt=childIDs.begin(); childIt!=childIDs.end(); ++childIt)
        {
        vtkSubjectHierarchyItem* currentItem = this->Internal->SceneItem->FindChildByID(*childIt);
        if ( !currentItem->HasChildren() || currentItem->IsVirtualBranchParent() )
          {
          // Remove data node from scene if requested
          if (removeDataNode && currentItem->DataNode && this->Scene)
            {
            this->Scene->RemoveNode(currentItem->DataNode.GetPointer());
            }

          // Remove leaf item from its parent if not in virtual branch (if in virtual branch, then they will be removed
          // automatically when their parent is removed)
          if (!currentItem->Parent->IsVirtualBranchParent())
            {
            currentItem->Parent->RemoveChild(*childIt);
            }

          // Remove ID of deleted item (or virtual branch leaf) from list and keep deleting until empty
          childIDs.erase(childIt);
          break;
          }
        }
      }
    }

  // Remove data node of given item from scene if requested
  if (removeDataNode && item->DataNode && this->Scene)
    {
    this->Scene->RemoveNode(item->DataNode.GetPointer());
    }

  // Remove given item itself if it's not the scene
  // (the scene item must always exist, and it doesn't have the parent that can perform the removal)
  if (itemID != this->Internal->SceneItemID)
    {
    item->Parent->RemoveChild(item);
    }
  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::RemoveAllItems(bool removeDataNode/*=false*/)
{
  this->RemoveItem(this->Internal->SceneItemID, removeDataNode, true);
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::SetItemParent(vtkIdType itemID, vtkIdType parentItemID)
{
  vtkSubjectHierarchyItem* item = this->Internal->SceneItem->FindChildByID(itemID);
  if (!item)
    {
    vtkErrorMacro("SetItemParent: Failed to find non-scene subject hierarchy item by ID " << itemID);
    return;
    }
  vtkSubjectHierarchyItem* parentItem = this->Internal->SceneItem->FindChildByID(parentItemID);
  if (!parentItem)
    {
    vtkErrorMacro("SetItemParent: Failed to find non-scene subject hierarchy item by ID " << parentItemID);
    return;
    }

  // Perform reparenting
  if (item->Reparent(parentItem))
    {
    //this->Modified(); //TODO: Needed? SH node modified event should be used for updating the whole view (every item)
    }
}

//----------------------------------------------------------------------------
vtkIdType vtkMRMLSubjectHierarchyNode::GetItemParent(vtkIdType itemID)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("GetItemParent: Failed to find subject hierarchy item by ID " << itemID);
    return INVALID_ITEM_ID;
    }
  if (!item->Parent)
    {
    return INVALID_ITEM_ID;
    }

  return item->Parent->ID;
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::GetItemChildren(vtkIdType itemID, std::vector<vtkIdType>& childIDs, bool recursive/*=false*/)
{
  childIDs.clear();

  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("GetItemChildren: Failed to find subject hierarchy item by ID " << itemID);
    return;
    }

  if (recursive)
    {
    item->GetAllChildren(childIDs);
    }
  else
    {
    item->GetDirectChildren(childIDs);
    }
}

//----------------------------------------------------------------------------
bool vtkMRMLSubjectHierarchyNode::ReparentItemByDataNode(vtkIdType itemID, vtkMRMLNode* newParentNode)
{
  vtkSubjectHierarchyItem* item = this->Internal->SceneItem->FindChildByID(itemID);
  if (!item)
    {
    vtkErrorMacro("ReparentItem: Failed to find non-scene subject hierarchy item by ID " << itemID);
    return false;
    }
  vtkSubjectHierarchyItem* formerParent = item->Parent;
  if (!formerParent)
    {
    vtkErrorMacro("ReparentItem: Cannot reparent scene item (ID " << itemID << ")");
    return false;
    }

  // Nothing to do if given parent node is the same as current parent
  if (formerParent->DataNode == newParentNode)
    {
    return true;
    }

  // Get new parent item by the given data node
  vtkSubjectHierarchyItem* newParentItem = this->Internal->SceneItem->FindChildByDataNode(newParentNode);
  if (!newParentItem)
    {
    vtkErrorMacro("ReparentItem: Failed to find subject hierarchy item by data MRML node " << newParentNode->GetName());
    return false;
    }

  // Perform reparenting
  if (item->Reparent(newParentItem))
    {
    //this->Modified(); //TODO: Needed? SH node modified event should be used for updating the whole view (every item)
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLSubjectHierarchyNode::MoveItem(vtkIdType itemID, vtkIdType beforeItemID)
{
  vtkSubjectHierarchyItem* item = this->Internal->SceneItem->FindChildByID(itemID);
  if (!item)
    {
    vtkErrorMacro("MoveItem: Failed to find non-scene subject hierarchy item by ID " << itemID);
    return false;
    }
  vtkSubjectHierarchyItem* beforeItem = this->Internal->SceneItem->FindChildByID(beforeItemID);
  if (!beforeItem)
    {
    vtkErrorMacro("MoveItem: Failed to find non-scene subject hierarchy item by ID " << beforeItemID);
    return false;
    }

  // Perform move
  if (item->Move(beforeItem))
    {
    //this->Modified(); //TODO: Needed? SH node modified event should be used for updating the whole view (every item)
    return true;
    }
  return false;
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLSubjectHierarchyNode::GetItemByUID(const char* uidName, const char* uidValue)
{
  if (!uidName || !uidValue)
    {
    vtkErrorMacro("GetSubjectHierarchyNodeByUID: Invalid UID name or value!");
    return INVALID_ITEM_ID;
    }
  vtkSubjectHierarchyItem* item = this->Internal->SceneItem->FindChildByUID(uidName, uidValue);
  return (item ? item->ID : INVALID_ITEM_ID);
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLSubjectHierarchyNode::GetItemByUIDList(const char* uidName, const char* uidValue)
{
  if (!uidName || !uidValue)
    {
    vtkErrorMacro("GetSubjectHierarchyItemByUIDList: Invalid UID name or value!");
    return INVALID_ITEM_ID;
    }
  vtkSubjectHierarchyItem* item = this->Internal->SceneItem->FindChildByUIDList(uidName, uidValue);
  return (item ? item->ID : INVALID_ITEM_ID);
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLSubjectHierarchyNode::GetItemByDataNode(vtkMRMLNode* dataNode)
{
  if (!dataNode)
    {
    vtkErrorMacro("GetSubjectHierarchyItemByDataNode: Invalid data node to find!");
    return INVALID_ITEM_ID;
    }

  vtkSubjectHierarchyItem* item = this->Internal->SceneItem->FindChildByDataNode(dataNode);
  return (item ? item->ID : INVALID_ITEM_ID);
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLSubjectHierarchyNode::GetItemChildWithName(vtkIdType parentItemID, std::string name)
{
  vtkSubjectHierarchyItem* parentItem = this->Internal->FindItemByID(parentItemID);
  if (!parentItem)
    {
    vtkErrorMacro("GetItemChildWithName: Failed to find subject hierarchy item by ID " << parentItemID);
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }

  // Search only one level (not recursive)
  std::vector<vtkIdType> foundItemIDs;
  parentItem->FindChildrenByName(name, foundItemIDs, false, false);
  if (foundItemIDs.size() == 0)
    {
    vtkErrorMacro("GetItemChildWithName: Failed to find subject hierarchy item with name '" << name
      << "' under item with ID " << parentItemID);
    return INVALID_ITEM_ID;
    }
  else if (foundItemIDs.size() > 1)
    {
    vtkWarningMacro("GetItemChildWithName: Multiple subject hierarchy item found with name '" << name
      << "' under item with ID " << parentItemID << ". Returning first");
    }
  return foundItemIDs[0];
}

//---------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::GetDataNodesInBranch(vtkIdType itemID, vtkCollection* dataNodeCollection, const char* childClass/*=NULL*/)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("GetDataNodesInBranch: Failed to find subject hierarchy item by ID " << itemID);
    return;
    }

  item->GetDataNodesInBranch(dataNodeCollection, childClass);
}

//---------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::SetDisplayVisibilityForBranch(vtkIdType itemID, int visible)
{
  if (visible != 0 && visible != 1)
    {
    vtkErrorMacro( "SetDisplayVisibilityForBranch: Invalid visibility value to set: " << visible
      << ". Needs to be one of the following: 0:Hidden, 1:Visible, 2:PartiallyVisible" );
    return;
    }
  if (this->Scene->IsBatchProcessing())
    {
    //vtkDebugMacro("SetDisplayVisibilityForBranch: Batch processing is on, returning");
    return;
    }

  // Get all child displayable nodes for branch
  vtkNew<vtkCollection> childDisplayableNodes;
  this->GetDataNodesInBranch(itemID, childDisplayableNodes.GetPointer(), "vtkMRMLDisplayableNode");

  childDisplayableNodes->InitTraversal();
  std::set<vtkMRMLSubjectHierarchyNode*> parentNodes;
  for (int childNodeIndex = 0;
       childNodeIndex < childDisplayableNodes->GetNumberOfItems();
       ++childNodeIndex)
    {
    vtkMRMLDisplayableNode* displayableNode =
        vtkMRMLDisplayableNode::SafeDownCast(childDisplayableNodes->GetItemAsObject(childNodeIndex));
    if (displayableNode)
      {
      // Create default display node is there is no display node associated
      vtkMRMLDisplayNode* displayNode = displayableNode->GetDisplayNode();
      if (!displayNode)
        {
        displayableNode->CreateDefaultDisplayNodes();
        }

      // Set display visibility
      displayableNode->SetDisplayVisibility(visible);

      // Set slice intersection visibility through display node
      displayNode = displayableNode->GetDisplayNode();
      if (displayNode)
        {
        displayNode->SetSliceIntersectionVisibility(visible);
        }
      displayableNode->Modified();
      }
    }

  this->Modified();
}

//---------------------------------------------------------------------------
int vtkMRMLSubjectHierarchyNode::GetDisplayVisibilityForBranch(vtkIdType itemID)
{
  int visible = -1;

  // Get all child displayable nodes for branch
  vtkNew<vtkCollection> childDisplayableNodes;
  this->GetDataNodesInBranch(itemID, childDisplayableNodes.GetPointer(), "vtkMRMLDisplayableNode");

  // Add associated displayable node for the parent item too
  vtkMRMLDisplayableNode* associatedDisplayableDataNode = vtkMRMLDisplayableNode::SafeDownCast(
    this->GetItemDataNode(itemID));
  if (associatedDisplayableDataNode)
    {
    childDisplayableNodes->AddItem(associatedDisplayableDataNode);
    }

  // Determine visibility state based on all displayable nodes involved
  childDisplayableNodes->InitTraversal();
  for (int childNodeIndex=0; childNodeIndex<childDisplayableNodes->GetNumberOfItems(); ++childNodeIndex)
    {
    vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(
      childDisplayableNodes->GetItemAsObject(childNodeIndex) );
    // Omit volume nodes from the process (they are displayed differently than every other type)
    // TODO: This is not very elegant or safe, it would be better to distinguish between visibility modes, or overhaul the visibility features completely
    if ( displayableNode
      && ( !displayableNode->IsA("vtkMRMLVolumeNode")
        || !strcmp(displayableNode->GetClassName(), "vtkMRMLSegmentationNode") ) )
      {
      // If we set visibility
      if (visible == -1)
        {
        visible = displayableNode->GetDisplayVisibility();

        // We expect only 0 or 1 from leaf nodes
        if (visible == 2)
          {
          vtkWarningMacro("GetDisplayVisibilityForBranch: Unexpected visibility value for node " << displayableNode->GetName());
          }
        }
      // If the current node visibility does not match the found visibility, then set partial visibility
      else if (displayableNode->GetDisplayVisibility() != visible)
        {
        return 2;
        }
      }
    }

  return visible;
}

//---------------------------------------------------------------------------
bool vtkMRMLSubjectHierarchyNode::IsItemLevel(vtkIdType itemID, std::string level)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("IsItemLevel: Failed to find subject hierarchy item by ID " << itemID);
    return false;
    }

  return !item->Level.compare(level);
}

//---------------------------------------------------------------------------
std::string vtkMRMLSubjectHierarchyNode::GetAttributeFromItemAncestor(vtkIdType itemID, std::string attributeName, std::string level)
{
  vtkSubjectHierarchyItem* item = this->Internal->SceneItem->FindChildByID(itemID);
  if (!item)
    {
    vtkErrorMacro("GetAttributeFromAncestor: Failed to find non-scene subject hierarchy item by ID " << itemID);
    return std::string();
    }

  return item->GetAttributeFromAncestor(attributeName, level);
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLSubjectHierarchyNode::GetItemAncestorAtLevel(vtkIdType itemID, std::string level)
{
  vtkSubjectHierarchyItem* item = this->Internal->SceneItem->FindChildByID(itemID);
  if (!item)
    {
    vtkErrorMacro("GetItemAncestorAtLevel: Failed to find non-scene subject hierarchy item by ID " << itemID);
    return INVALID_ITEM_ID;
    }

  vtkSubjectHierarchyItem* ancestorItem = item->GetAncestorAtLevel(level);
  if (ancestorItem)
    {
    return ancestorItem->ID;
    }
  return INVALID_ITEM_ID;
}

//---------------------------------------------------------------------------
bool vtkMRMLSubjectHierarchyNode::IsAnyNodeInBranchTransformed(vtkIdType itemID, vtkMRMLTransformNode* exceptionNode/*=NULL*/)
{
  // Check transformable node from the item itself if any
  vtkSubjectHierarchyItem* item = this->Internal->SceneItem->FindChildByID(itemID);
  if (!item)
    {
    vtkErrorMacro("IsAnyNodeInBranchTransformed: Failed to find non-scene subject hierarchy item by ID " << itemID);
    return false;
    }
  if (item->DataNode)
    {
    vtkMRMLTransformableNode* transformableDataNode = vtkMRMLTransformableNode::SafeDownCast(item->DataNode);
    if ( transformableDataNode && transformableDataNode->GetParentTransformNode()
      && transformableDataNode->GetParentTransformNode() != exceptionNode)
      {
      return true;
      }
    }

  // Check all child transformable nodes for branch
  vtkNew<vtkCollection> childTransformableNodes;
  this->GetDataNodesInBranch(itemID, childTransformableNodes.GetPointer(), "vtkMRMLTransformableNode");
  childTransformableNodes->InitTraversal();

  for (int childNodeIndex=0; childNodeIndex<childTransformableNodes->GetNumberOfItems(); ++childNodeIndex)
    {
    vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(
      childTransformableNodes->GetItemAsObject(childNodeIndex) );
    vtkMRMLTransformNode* parentTransformNode = NULL;
    if (transformableNode && (parentTransformNode = transformableNode->GetParentTransformNode()))
      {
      if (parentTransformNode != exceptionNode)
        {
        return true;
        }
      }
    }

  return false;
}

//---------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::DeserializeUIDList(std::string uidListString, std::vector<std::string>& deserializedUIDList)
{
  deserializedUIDList.clear();
  char separatorCharacter = ' ';
  size_t separatorPosition = uidListString.find( separatorCharacter );
  while (separatorPosition != std::string::npos)
    {
    std::string uid = uidListString.substr(0, separatorPosition);
    deserializedUIDList.push_back(uid);
    uidListString = uidListString.substr( separatorPosition+1 );
    separatorPosition = uidListString.find( separatorCharacter );
    }
  // Add last UID in case there was no space at the end (which is default behavior)
  if (!uidListString.empty() && uidListString.find(separatorCharacter) == std::string::npos)
    {
    deserializedUIDList.push_back(uidListString);
    }
}

//---------------------------------------------------------------------------
std::vector<vtkIdType> vtkMRMLSubjectHierarchyNode::GetItemsReferencedFromItemByDICOM(vtkIdType itemID)
{
  std::vector<vtkIdType> referencedItemIDs;
  vtkSubjectHierarchyItem* item = this->Internal->SceneItem->FindChildByID(itemID);
  if (!item)
    {
    vtkErrorMacro("GetItemsReferencedFromItemByDICOM: Failed to find non-scene subject hierarchy item by ID " << itemID);
    return referencedItemIDs;
    }

  // Get referenced SOP instance UIDs
  std::string referencedInstanceUIDsAttribute = item->GetAttribute(
    vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName() );
  if (referencedInstanceUIDsAttribute.empty())
    {
    return referencedItemIDs;
    }

  // De-serialize SOP instance UID list
  std::vector<vtkSubjectHierarchyItem*> referencedItems;
  std::vector<std::string> referencedSopInstanceUids;
  this->DeserializeUIDList(referencedInstanceUIDsAttribute, referencedSopInstanceUids);

  // Find subject hierarchy items by SOP instance UIDs
  for (std::vector<std::string>::iterator uidIt=referencedSopInstanceUids.begin(); uidIt!=referencedSopInstanceUids.end(); ++uidIt)
    {
    // Find first referenced item in the subject hierarchy tree
    if (referencedItems.empty())
      {
      vtkSubjectHierarchyItem* referencedItem = this->Internal->SceneItem->FindChildByUIDList(
        vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName(), (*uidIt) );
      if (referencedItem)
        {
        referencedItems.push_back(referencedItem);
        }
      }
    else
      {
      // If we found a referenced node, check the other instances in those nodes first to save time
      bool foundUidInFoundReferencedItems = false;
      for (std::vector<vtkSubjectHierarchyItem*>::iterator itemIt=referencedItems.begin(); itemIt!=referencedItems.end(); ++itemIt)
        {
        // Get instance UIDs of the referenced item
        std::string uids = (*itemIt)->GetUID(vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName());
        if (uids.find(*uidIt) != std::string::npos)
          {
          // If we found the UID in the already found referenced items, then we don't need to do anything
          foundUidInFoundReferencedItems = true;
          break;
          }
        }
      // If the referenced SOP instance UID is not contained in the already found referenced items, then we look in the tree
      if (!foundUidInFoundReferencedItems)
        {
        vtkSubjectHierarchyItem* referencedItem = this->Internal->SceneItem->FindChildByUIDList(
          vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName(), (*uidIt) );
        if (referencedItem)
          {
          referencedItems.push_back(referencedItem);
          }
        }
      }
    }

  // Copy item IDs into output vector
  for (std::vector<vtkSubjectHierarchyItem*>::iterator itemIt=referencedItems.begin(); itemIt!=referencedItems.end(); ++itemIt)
    {
    referencedItemIDs.push_back( (*itemIt)->ID );
    }

  return referencedItemIDs;
}

//---------------------------------------------------------------------------
bool vtkMRMLSubjectHierarchyNode::MergeSubjectHierarchy(vtkMRMLSubjectHierarchyNode* otherShNode)
{
  if (otherShNode)
    {
    vtkErrorMacro("MergeSubjectHierarchy: Invalid subject hierarchy node to merge");
    return false;
    }
  vtkMRMLScene* scene = this->GetScene();
  if (!scene || scene != otherShNode->GetScene())
    {
    vtkErrorMacro("MergeSubjectHierarchy: Invalid MRML scene");
    }

  // Mapping old IDs in the other node to new IDs in this node
  std::map<vtkIdType,vtkIdType> idMap;
  idMap[otherShNode->GetSceneItemID()] = this->Internal->SceneItemID;

  // Collect items starting from the scene item
  std::vector<vtkIdType> itemsToCopy;
  vtkIdType otherSceneItemID = otherShNode->GetSceneItemID();
  // The items in the vector are ordered so that the child of an item always comes after the item, so there always be a valid parent
  otherShNode->GetItemChildren(otherSceneItemID, itemsToCopy, true);
  for (std::vector<vtkIdType>::iterator itemIt=itemsToCopy.begin(); itemIt!=itemsToCopy.end(); ++itemIt)
    {
    vtkIdType otherID = (*itemIt);

    // Copy subject hierarchy item into this hierarchy
    vtkIdType copiedID = this->CreateItem(
      idMap[otherShNode->GetItemParent(otherID)],
      otherShNode->GetItemDataNode(otherID),
      otherShNode->GetItemLevel(otherID),
      otherShNode->GetItemName(otherID) );

    // Save copied ID so that it can be referenced later for parenting
    idMap[otherID] = copiedID;
    }

  // Remove other subject hierarchy node
  scene->RemoveNode(otherShNode);
  return true;
}

//---------------------------------------------------------------------------
std::string vtkMRMLSubjectHierarchyNode::GenerateUniqueItemName(std::string name)
{
  std::vector<vtkIdType> foundItemIDs;
  this->Internal->SceneItem->FindChildrenByName(name, foundItemIDs, false, true);
  if (foundItemIDs.size() == 0)
    {
    // If no item found with that name then return input name as it's unique already
    return name;
    }

  // If name found in the hierarchy, then get items that contain that name to determine postfix
  int postfixNumber = 1;
  this->Internal->SceneItem->FindChildrenByName(name, foundItemIDs, true, true);
  std::vector<vtkIdType>::iterator itemIt;
  for (itemIt=foundItemIDs.begin(); itemIt!=foundItemIDs.end(); ++itemIt)
    {
    // Get current item
    vtkSubjectHierarchyItem* currentItem = this->Internal->SceneItem->FindChildByID(*itemIt);
    if (!currentItem)
      {
      vtkErrorMacro("GenerateUniqueItemName: Failed to find non-scene subject hierarchy item by ID " << (*itemIt));
      break;
      }
    // Only consider item if its name starts with the given name
    std::string currentPrefix = currentItem->Name.substr(0, name.size());
    if (currentPrefix.compare(name))
      {
      continue;
      }
    // Further, the next character needs to be an underscore
    if (currentItem->Name.substr(name.size(), 1).compare("_"))
      {
      continue;
      }
    // Get postfix number from the current name
    int currentPostfixNumber = vtkVariant(currentItem->Name.substr(name.size()+1)).ToInt();

    // Make postfix number one larger than the maximum of the postfix numbers found so far
    if (currentPostfixNumber >= postfixNumber)
      {
      postfixNumber = currentPostfixNumber + 1;
      }
    }

  std::stringstream uniqueNameStream;
  uniqueNameStream << name << "_" << postfixNumber;
  return uniqueNameStream.str();
}

//---------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::ItemEventCallback(vtkObject* caller, unsigned long eid, void* clientData, void* callData)
{
  vtkMRMLSubjectHierarchyNode* self = reinterpret_cast<vtkMRMLSubjectHierarchyNode*>(clientData);
  if (!self)
    {
    return;
    }

  // Invoke event from node with item ID
  switch (eid)
    {
    case vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAddedEvent:
    case vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAboutToBeRemovedEvent:
    case vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemRemovedEvent:
    case vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemUIDAddedEvent:
      {
      // Get item from call data
      vtkSubjectHierarchyItem* item = reinterpret_cast<vtkSubjectHierarchyItem*>(callData);
      if (item)
        {
        // Invoke event of same type with item ID
        self->InvokeCustomModifiedEvent(eid, (void*)&item->ID);
        }
      }
      break;
    case vtkCommand::ModifiedEvent:
      {
      vtkSubjectHierarchyItem* item = vtkSubjectHierarchyItem::SafeDownCast(caller);
      vtkMRMLNode* dataNode = vtkMRMLNode::SafeDownCast(caller);
      if (item)
        {
        // Propagate item modified event
        self->InvokeCustomModifiedEvent(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent, (void*)&item->ID);
        }
      else if (dataNode)
        {
        // Trigger view update also if data node was modified
        vtkIdType itemID = self->GetItemByDataNode(dataNode);
        if (itemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
          {
          self->InvokeCustomModifiedEvent(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent, (void*)&itemID);
          }
        }
      }
      break;
    case vtkMRMLTransformableNode::TransformModifiedEvent:
    case vtkMRMLDisplayableNode::DisplayModifiedEvent:
      {
      vtkMRMLNode* dataNode = vtkMRMLNode::SafeDownCast(caller);
      if (dataNode)
        {
        // Trigger view update if data node's transform or display was modified
        vtkIdType itemID = self->GetItemByDataNode(dataNode);
        if (itemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
          {
          self->InvokeCustomModifiedEvent(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent, (void*)&itemID);
          }
        }
      }
      break;
    default:
      vtkErrorWithObjectMacro(self, "vtkMRMLSubjectHierarchyNode::ItemEventCallback: Unknown event ID " << eid);
      return;
    }
}

//---------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(vtkMRMLScene* scene)
  {
  if (!scene)
    {
    vtkGenericWarningMacro("vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode: Invalid scene given");
    return NULL;
    }
  if (scene->GetNumberOfNodesByClass("vtkMRMLSubjectHierarchyNode") == 0)
    {
    vtkSmartPointer<vtkMRMLSubjectHierarchyNode> newShNode = vtkSmartPointer<vtkMRMLSubjectHierarchyNode>::New();
    newShNode->SetName("SubjectHierarchy");
    scene->AddNode(newShNode);

    vtkDebugWithObjectMacro(newShNode.GetPointer(), "vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode: "
      "New subject hierarchy node created as none was found in the scene");
    return newShNode;
    }

  // Return subject hierarchy node if there is only one
  scene->InitTraversal();
  vtkMRMLSubjectHierarchyNode* firstShNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(
    scene->GetNextNodeByClass("vtkMRMLSubjectHierarchyNode"));
  if (scene->GetNumberOfNodesByClass("vtkMRMLSubjectHierarchyNode") == 1)
    {
    return firstShNode;
    }

  // Do not perform merge operations while the scene is processing
  if (scene->IsBatchProcessing() || scene->IsImporting() || scene->IsClosing())
    {
    vtkWarningWithObjectMacro(scene, "vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode: "
      "Scene is processing, merging subject hierarchies is not possible");
    return NULL;
    }

  // Merge subject hierarchy nodes into the first one found
  for (vtkMRMLNode* node=NULL; (node=scene->GetNextNodeByClass("vtkMRMLSubjectHierarchyNode"));)
    {
    vtkMRMLSubjectHierarchyNode* currentShNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(node);
    if (currentShNode)
      {
      if (!firstShNode->MergeSubjectHierarchy(currentShNode))
        {
        //TODO: The node will probably be invalid, so it needs to be completely re-built
        vtkErrorWithObjectMacro(scene, "vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode: Failed to merge subject hierarchy nodes");
        return firstShNode;
        }
      }
    }
  // Return the first (and now only) subject hierarchy node into which the others were merged
  return firstShNode;
  }

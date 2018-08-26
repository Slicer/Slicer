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
const vtkIdType vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID = 0;
const std::string vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_SEPARATOR = std::string("|");
const std::string vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_NAME_VALUE_SEPARATOR = std::string("^");
const std::string vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_VERSION_ATTRIBUTE_NAME = std::string("SubjectHierarchyVersion");

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSubjectHierarchyNode);

//----------------------------------------------------------------------------
class vtkSubjectHierarchyItem : public vtkObject
{
public:
  static vtkSubjectHierarchyItem *New();
  vtkTypeMacro(vtkSubjectHierarchyItem, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;
  void ReadXMLAttributes(const char** atts);
  void WriteXML(ostream& of, int indent, vtkMRMLNode* xmlEncoderNode);
  void DeepCopy(vtkSubjectHierarchyItem* item, bool copyChildren=true);

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

  /// Name of the owner plugin that claimed this node
  std::string OwnerPluginName;

  /// Flag indicating whether the branch under the item is expanded in the view
  bool Expanded;

  /// List of UIDs of this subject hierarchy node
  /// UIDs can be DICOM UIDs, MIDAS urls, etc.
  std::map<std::string, std::string> UIDs;

  /// Attributes (metadata, referenced IDs, etc.)
  std::map<std::string, std::string> Attributes;

  /// Member to temporarily store item ID in case of scene import.
  /// The ID is resolved after import ends, and this member is set to INVALID_ITEM_ID.
  vtkIdType TemporaryID;
  /// Member to temporarily store data node ID in case of scene import.
  /// The ID is first updated then resolved to pointers after import ends, and the string is emptied.
  std::string TemporaryDataNodeID;
  /// Member to temporarily store parent item ID in case of scene import.
  /// The ID is resolved to pointer after import ends, and this member is set to INVALID_ITEM_ID.
  vtkIdType TemporaryParentItemID;

  /// Item cache to speed up lookup by ID that needs to be performed many times.
  /// It can be static as the item IDs are unique in one application session.
  static std::map<vtkIdType, vtkSubjectHierarchyItem*> ItemCache;

// Get/set functions
public:
  /// Add data item to tree under parent, specifying basic properties
  /// \param parent Parent item pointer under which this item is inserted
  /// \param dataNode Associated data MRML node. Name of the data node provides the name of the item
  /// \return ID of the item in the hierarchy that was assigned automatically when adding
  vtkIdType AddToTree(vtkSubjectHierarchyItem* parent, vtkMRMLNode* dataNode);

  /// Add hierarchy item to tree under parent, specifying basic properties
  /// \param parent Parent item pointer under which this item is inserted
  /// \param name Name of the item
  /// \param level Level string of the item (\sa vtkMRMLSubjectHierarchyConstants). It will be added as a special attribute
  /// \return ID of the item in the hierarchy that was assigned automatically when adding
  vtkIdType AddToTree(vtkSubjectHierarchyItem* parent, std::string name, std::string level);

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
  /// Get IDs of all children in the branch recursively
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
  /// Get child item by position
  /// \return ID of child item found in given position. Invalid if no item found at that position
  vtkIdType GetChildByPositionUnderParent(int position);

  /// Remove given item from children by item pointer
  /// \return Success flag
  bool RemoveChild(vtkSubjectHierarchyItem* item);
  /// Remove given item from children by ID
  /// \return Success flag
  bool RemoveChild(vtkIdType itemID);
  /// Reparent children of the item to the item's parent.
  /// Called right before deleting the item so that its children do not become orphans thus lost to the hierarchy
  void ReparentChildrenToParent();
  /// Remove all children. Do not delete data nodes from the scene. Used in destructor, and for deleting virtual branches
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

vtkIdType vtkSubjectHierarchyItem::NextSubjectHierarchyItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID + 1;

std::map<vtkIdType, vtkSubjectHierarchyItem*> vtkSubjectHierarchyItem::ItemCache = std::map<vtkIdType, vtkSubjectHierarchyItem*>();

//---------------------------------------------------------------------------
// vtkSubjectHierarchyItem methods

//---------------------------------------------------------------------------
vtkSubjectHierarchyItem::vtkSubjectHierarchyItem()
  : ID(vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  , DataNode(NULL)
  , Name("")
  , Parent(NULL)
  , OwnerPluginName("")
  , Expanded(true)
  , TemporaryID(vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  , TemporaryDataNodeID("")
  , TemporaryParentItemID(vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
{
  this->Children.clear();
  this->Attributes.clear();
  this->UIDs.clear();
}

//---------------------------------------------------------------------------
vtkSubjectHierarchyItem::~vtkSubjectHierarchyItem()
{
  this->RemoveAllChildren();

  this->Attributes.clear();
  this->UIDs.clear();
}

//---------------------------------------------------------------------------
vtkIdType vtkSubjectHierarchyItem::AddToTree(vtkSubjectHierarchyItem* parent, vtkMRMLNode* dataNode)
{
  this->ID = vtkSubjectHierarchyItem::NextSubjectHierarchyItemID;
  vtkSubjectHierarchyItem::NextSubjectHierarchyItemID++;
  if (vtkSubjectHierarchyItem::NextSubjectHierarchyItemID == static_cast<vtkIdType>(VTK_UNSIGNED_LONG_MAX))
    {
    // There is a negligible chance that it reaches maximum, but if it happens then report error
    vtkErrorMacro("AddToTree: Next subject hierarchy item ID reached its maximum value! Item is not added to the tree");
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }

  // Set basic properties
  this->DataNode = dataNode;
  this->Name = ""; // Use name of the data node

  this->Parent = parent;
  if (parent)
    {
    // Add under parent
    vtkSmartPointer<vtkSubjectHierarchyItem> childPointer(this);
    this->Parent->Children.push_back(childPointer);

    // Add to cache
    vtkSubjectHierarchyItem::ItemCache[this->ID] = this;
    }
  else
    {
    vtkErrorMacro("AddToTree: Invalid parent of non-scene item to add");
    }

  this->InvokeEvent(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAddedEvent, this);

  return this->ID;
}

//---------------------------------------------------------------------------
vtkIdType vtkSubjectHierarchyItem::AddToTree(vtkSubjectHierarchyItem* parent, std::string name, std::string level)
{
  this->ID = vtkSubjectHierarchyItem::NextSubjectHierarchyItemID;
  vtkSubjectHierarchyItem::NextSubjectHierarchyItemID++;
  if (vtkSubjectHierarchyItem::NextSubjectHierarchyItemID == static_cast<vtkIdType>(VTK_UNSIGNED_LONG_MAX))
    {
    // There is a negligible chance that it reaches maximum, report error in that case
    vtkErrorMacro("AddToTree: Next subject hierarchy item ID reached its maximum value! Item is not added to the tree");
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }

  // Set basic properties
  this->DataNode = NULL;
  this->Name = name;
  this->Attributes[vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelAttributeName()] = level;

  this->Parent = parent;
  if (parent)
    {
    // Add under parent
    vtkSmartPointer<vtkSubjectHierarchyItem> childPointer(this);
    this->Parent->Children.push_back(childPointer);

    // Add to cache
    vtkSubjectHierarchyItem::ItemCache[this->ID] = this;
    }
  else if (! ( (!name.compare("Scene") && !level.compare("Scene"))
            || (!name.compare("UnresolvedItems") && !level.compare("UnresolvedItems")) ) )
    {
    // Only the scene item or the unresolved items parent can have NULL parent
    vtkErrorMacro("AddToTree: Invalid parent of non-scene item to add");
    }

  this->InvokeEvent(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAddedEvent, this);

  return this->ID;
}

//----------------------------------------------------------------------------
void vtkSubjectHierarchyItem::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "ID: " << this->ID << "\n";

  os << indent << "DataNode: " << (this->DataNode.GetPointer() ? this->DataNode->GetID() : "(none)") << "\n";
  os << indent << "Name";
  if (this->DataNode.GetPointer())
    {
    os << " (from DataNode): " << (this->DataNode->GetName() ? this->DataNode->GetName() : "(none)") << "\n";
    }
  else
    {
    os << ": " << this->Name << "\n";
    }

  os << indent << "Parent: "
    << (this->Parent ? this->Parent->ID : vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    << (this->Parent && !this->Parent->Parent ? " (scene)" : "") << "\n";
  os << indent << "Children: ";
  for (ChildVector::iterator childIt=this->Children.begin(); childIt!=this->Children.end(); ++childIt)
    {
    os << childIt->GetPointer()->ID << " ";
    }
  os << "\n";

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
    attValue = *(atts++); // already XML-decoded (e.g., &amp; decoded to &)

    if (!strcmp(attName, "id"))
      {
      this->ID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
      this->TemporaryID = vtkVariant(attValue).ToLongLong();
      }
    if (!strcmp(attName, "dataNode"))
      {
      if (!this->Name.empty())
        {
        vtkErrorMacro("ReadXMLAttributes: Item cannot have data node and name specified at the same time. "
                      << "Data node is not set for item named " << this->Name);
        continue;
        }

      this->DataNode = NULL;
      this->TemporaryDataNodeID = std::string(attValue);
      }
    if (!strcmp(attName, "name"))
      {
      if (!this->TemporaryDataNodeID.empty())
        {
        vtkErrorMacro("ReadXMLAttributes: Item cannot have data node and name specified at the same time. "
                      << "Name is not set for item for data node with ID " << this->TemporaryDataNodeID);
        continue;
        }

      this->Name = std::string(attValue);
      }
    if (!strcmp(attName, "parent"))
      {
      this->Parent = NULL;
      this->TemporaryParentItemID = vtkVariant(attValue).ToLongLong();
      }
    else if (!strcmp(attName, "type"))
      {
      this->OwnerPluginName = std::string(attValue);
      }
    if (!strcmp(attName, "expanded"))
      {
      this->Expanded =  (strcmp(attValue,"true") ? false : true);
      }
    else if (!strcmp(attName, "uids"))
      {
      std::stringstream ss;
      ss << attValue;
      std::string valueStr = ss.str();

      this->UIDs.clear();
      size_t itemSeparatorPosition = valueStr.find(vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_SEPARATOR);
      while (itemSeparatorPosition != std::string::npos)
        {
        std::string itemStr = valueStr.substr(0, itemSeparatorPosition);
        size_t nameValueSeparatorPosition = itemStr.find(vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_NAME_VALUE_SEPARATOR);

        std::string name = itemStr.substr(0, nameValueSeparatorPosition);
        std::string value = itemStr.substr(nameValueSeparatorPosition + vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_NAME_VALUE_SEPARATOR.size());
        this->UIDs[name] = value;

        valueStr = valueStr.substr(itemSeparatorPosition + vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_SEPARATOR.size());
        itemSeparatorPosition = valueStr.find(vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_SEPARATOR);
        }
      if (!valueStr.empty() )
        {
        std::string itemStr = valueStr.substr(0, itemSeparatorPosition);
        size_t nameValueSeparatorPosition = itemStr.find(vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_NAME_VALUE_SEPARATOR);

        std::string name = itemStr.substr(0, nameValueSeparatorPosition);
        std::string value = itemStr.substr(nameValueSeparatorPosition + vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_NAME_VALUE_SEPARATOR.size());
        this->UIDs[name] = value;
        }
      }
    else if (!strcmp(attName, "attributes"))
      {
      std::stringstream ss;
      ss << attValue;
      std::string valueStr = ss.str();

      this->Attributes.clear();
      size_t itemSeparatorPosition = valueStr.find(vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_SEPARATOR);
      while (itemSeparatorPosition != std::string::npos)
        {
        std::string itemStr = valueStr.substr(0, itemSeparatorPosition);
        size_t nameValueSeparatorPosition = itemStr.find(vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_NAME_VALUE_SEPARATOR);

        std::string name = itemStr.substr(0, nameValueSeparatorPosition);
        std::string value = itemStr.substr(nameValueSeparatorPosition + vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_NAME_VALUE_SEPARATOR.size());
        this->Attributes[name] = value;

        valueStr = valueStr.substr(itemSeparatorPosition + vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_SEPARATOR.size());
        itemSeparatorPosition = valueStr.find(vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_SEPARATOR);
        }
      if (!valueStr.empty() )
        {
        std::string itemStr = valueStr.substr(0, itemSeparatorPosition);
        size_t nameValueSeparatorPosition = itemStr.find(vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_NAME_VALUE_SEPARATOR);

        std::string name = itemStr.substr(0, nameValueSeparatorPosition);
        std::string value = itemStr.substr(nameValueSeparatorPosition + vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_NAME_VALUE_SEPARATOR.size());
        this->Attributes[name] = value;
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkSubjectHierarchyItem::WriteXML(ostream& of, int nIndent, vtkMRMLNode* xmlEncoderNode)
{
  if (!xmlEncoderNode)
    {
    vtkErrorMacro("WriteXML: Invalid XML encoder node");
    return;
    }

  vtkIndent indent(nIndent);

  // Start item element
  of << "\n" << indent << "<SubjectHierarchyItem";

  // Write item properties
  of << " id=\"" << (this->ID ? this->ID : this->TemporaryID) << "\"";

  if (this->DataNode.GetPointer())
    {
    if (this->DataNode.GetPointer()->GetSaveWithScene())
      {
      of << " dataNode=\"" << this->DataNode->GetID() << "\"";
      }
    else
      {
      // Node is not saved with the scene, but may still have children, attributes, etc.
      // so write it to the scene but without data node ID.
      of << " name=\"" << xmlEncoderNode->XMLAttributeEncodeString(this->DataNode->GetName() ? this->DataNode->GetName() : "") << "\"";
      }
    }
  else if (!this->TemporaryDataNodeID.empty())
    {
    of << " dataNode=\"" << this->TemporaryDataNodeID << "\"";
    }
  else
    {
    of << " name=\"" << xmlEncoderNode->XMLAttributeEncodeString(this->Name) << "\"";
    }

  //TODO: Storing the parent is necessary if we want to avoid adding extra features to vtkMRMLParser.
  //      A more generic mechanism that passes the non-node child elements to the containing node for parsing
  //      could make this unnecessary
  if (this->TemporaryParentItemID)
    {
    // If there is temporary parent, then write that (happens in case of scene views)
    of << " parent=\"" << this->TemporaryParentItemID << "\"";
    }
  else
    {
    of << " parent=\"" << (this->Parent ? this->Parent->ID : vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID) << "\"";
    }

  of << " type=\"" << this->OwnerPluginName << "\"";

  of << " expanded=\"" << (this->Expanded ? "true" : "false") << "\"";

  if (this->UIDs.size() > 0)
    {
    of << " uids=\"";
    for (std::map<std::string, std::string>::iterator uidIt = this->UIDs.begin(); uidIt != this->UIDs.end(); ++uidIt)
      {
      of << xmlEncoderNode->XMLAttributeEncodeString(uidIt->first)
         << vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_NAME_VALUE_SEPARATOR
         << xmlEncoderNode->XMLAttributeEncodeString(uidIt->second)
         << vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_SEPARATOR;
      }
    of << "\"";
    }

  if (this->Attributes.size() > 0)
    {
    of << " attributes=\"";
    for (std::map<std::string, std::string>::iterator attIt = this->Attributes.begin(); attIt != this->Attributes.end(); ++attIt)
      {
      of << xmlEncoderNode->XMLAttributeEncodeString(attIt->first)
         << vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_NAME_VALUE_SEPARATOR
         << xmlEncoderNode->XMLAttributeEncodeString(attIt->second)
         << vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_SEPARATOR;
      }
    of << "\"";
    }

  // Close item element opening tag
  of << ">";

  // Write elements for all children
  ChildVector::iterator childIt;
  for (childIt=this->Children.begin(); childIt!=this->Children.end(); ++childIt)
    {
    vtkSubjectHierarchyItem* childItem = childIt->GetPointer();
    childItem->WriteXML(of, nIndent + 2, xmlEncoderNode);
    }

  // Close item element (in same line)
  of << "</SubjectHierarchyItem>";
}

//---------------------------------------------------------------------------
void vtkSubjectHierarchyItem::DeepCopy(vtkSubjectHierarchyItem* item, bool copyChildren/*=true*/)
{
  if (!item)
    {
    return;
    }

  // Skip copying ID and data node and parent item pointers. Duplicate IDs potentially cause problems
  // Copying is strictly related to scene or scene view operations, so the copied items will end up
  // in UnresolvedItems and need to be resolved. Otherwise they need to be added to the tree
  // explicitly using AddToTree
  this->Name = item->Name;
  this->OwnerPluginName = item->OwnerPluginName;
  this->Expanded = item->Expanded;
  this->UIDs = item->UIDs;
  this->Attributes = item->Attributes;

  // Copy temporary members if they are valid, otherwise save from live members
  if (item->TemporaryID)
    {
    this->TemporaryID = item->TemporaryID;
    }
  else
    {
    this->TemporaryID = item->ID;
    }
  if (!item->TemporaryDataNodeID.empty())
    {
    this->TemporaryDataNodeID = item->TemporaryDataNodeID;
    }
  else
    {
    this->TemporaryDataNodeID = std::string(item->DataNode ? item->DataNode->GetID() : "");
    }
  if (item->TemporaryParentItemID)
    {
    this->TemporaryParentItemID = item->TemporaryParentItemID;
    }
  else
    {
    this->TemporaryParentItemID = (item->Parent ? item->Parent->ID : vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID);
    }

  // Deep copy all children if requested
  if (copyChildren)
    {
    // Remove all children first
    this->RemoveAllChildren();

    ChildVector::iterator childIt;
    for (childIt=item->Children.begin(); childIt!=item->Children.end(); ++childIt)
      {
      vtkSmartPointer<vtkSubjectHierarchyItem> copiedChildItem = vtkSmartPointer<vtkSubjectHierarchyItem>::New();
      copiedChildItem->DeepCopy(childIt->GetPointer(), true);
      this->Children.push_back(copiedChildItem);
      copiedChildItem->Parent = this;
      }
    }
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
    vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyVirtualBranchAttributeName() ).empty();
}

//---------------------------------------------------------------------------
vtkSubjectHierarchyItem* vtkSubjectHierarchyItem::FindChildByID(vtkIdType itemID, bool recursive/*=true*/)
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    // No need to look up item ID if it is the invalid ID
    return NULL;
    }

  // Try to find item in cache
  std::map<vtkIdType, vtkSubjectHierarchyItem*>::iterator itemIt = vtkSubjectHierarchyItem::ItemCache.find(itemID);
  if (itemIt != vtkSubjectHierarchyItem::ItemCache.end())
    {
    return itemIt->second;
    }
  else
    {
    vtkWarningMacro("FindChildByID: Item cache does not contain requested ID " << itemID);
    }

  // On failure to look up in cache (should not happen), traverse tree to find item
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
      vtkSubjectHierarchyItem* foundItemInBranch = currentItem->FindChildByUIDList(uidName, uidValue);
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

  // Add data node of item if it's of the requested type
  vtkMRMLNode* currentDataNode = this->DataNode.GetPointer();
  if (currentDataNode)
    {
    if (currentDataNode->IsA(nodeClass.c_str()))
      {
      dataNodeCollection->AddItem(currentDataNode);
      }
    }

  // Traverse children recursively
  ChildVector::iterator childIt;
  for (childIt=this->Children.begin(); childIt!=this->Children.end(); ++childIt)
    {
    vtkSubjectHierarchyItem* currentItem = childIt->GetPointer();
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

  vtkSubjectHierarchyItem* formerParentItem = this->Parent;

  // Nothing to do if given parent item is the same as current parent
  if (formerParentItem == newParentItem)
    {
    return true;
    }

  // Remove item from former parent
  vtkSubjectHierarchyItem::ChildVector::iterator childIt;
  for (childIt=formerParentItem->Children.begin(); childIt!=formerParentItem->Children.end(); ++childIt)
    {
    if (this == childIt->GetPointer())
      {
      break;
      }
    }
  if (childIt == formerParentItem->Children.end())
    {
    vtkErrorMacro("Reparent: Subject hierarchy item '" << this->GetName() << "' not found under item '" << formerParentItem->GetName() << "'");
    return false;
    }

  // Prevent deletion of the item from memory until the events are processed
  vtkSmartPointer<vtkSubjectHierarchyItem> thisPointer = this;

  // Remove item from former parent
  formerParentItem->Children.erase(childIt);

  // Add item to new parent
  this->Parent = newParentItem;
  newParentItem->Children.push_back(thisPointer);

  // Invoke modified events on all affected items
  formerParentItem->Modified();
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
  if (beforeItem == this)
    {
    vtkErrorMacro("Move: Item cannot be moved before itself");
    return false;
    }

  // Remove item from parent
  ChildVector::iterator removedIt;
  for (removedIt=this->Parent->Children.begin(); removedIt!=this->Parent->Children.end(); ++removedIt)
    {
    if (this == removedIt->GetPointer())
      {
      break;
      }
    }
  if (removedIt == this->Parent->Children.end())
    {
    vtkErrorMacro("Move: Failed to find subject hierarchy item '" << this->GetName()
      << "' in its parent '" << this->Parent->GetName() << "'");
    return false;
    }

  // Prevent deletion of the item from memory until the events are processed
  vtkSmartPointer<vtkSubjectHierarchyItem> thisPointer = this;

  // Remove item from former parent
  this->Parent->Children.erase(removedIt);

  // Re-insert item to the requested position (before beforeItem)
  if (!beforeItem)
    {
    this->Parent->Children.push_back(thisPointer);
    return true;
    }

  ChildVector::iterator beforeIt;
  for (beforeIt=this->Parent->Children.begin(); beforeIt!=this->Parent->Children.end(); ++beforeIt)
    {
    if (beforeItem == beforeIt->GetPointer())
      {
      break;
      }
    }
  if (beforeIt == this->Parent->Children.end())
    {
    vtkErrorMacro("Move: Failed to find subject hierarchy item '" << beforeItem->GetName()
      << "' as insertion position in item '" << this->Parent->GetName() << "'");
    return false;
    }
  this->Parent->Children.insert(beforeIt, thisPointer);
  this->Parent->Modified();

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
  vtkErrorMacro("GetPositionUnderParent: Failed to find subject hierarchy item " << this->Name << " under its parent");
  return -1;
}

//---------------------------------------------------------------------------
vtkIdType vtkSubjectHierarchyItem::GetChildByPositionUnderParent(int position)
{
  int currentPosition = 0;
  ChildVector::iterator childIt;
  for (childIt=this->Children.begin(); childIt!=this->Children.end(); ++childIt, ++currentPosition)
    {
    if (currentPosition == position)
      {
      return childIt->GetPointer()->ID;
      }
    }
  // Failed to find item
  vtkErrorMacro("GetChildByPositionUnderParent: Failed to find subject hierarchy item under parent " << this->Name << " at position " << position);
  return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
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

  // Prevent deletion of the item from memory until the events are processed
  vtkSmartPointer<vtkSubjectHierarchyItem> removedItem = (*childIt);

  // If child is a virtual branch (meaning that its children are invalid without the item,
  // as they represent the item's data node's content), then remove virtual branch
  if (removedItem->IsVirtualBranchParent())
    {
    removedItem->RemoveAllChildren();
    }

  // Remove child
  this->InvokeEvent(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAboutToBeRemovedEvent, item);
  this->Children.erase(childIt);

  // Reparent children to parent node (to avoid them becoming orphans and thus lost to the hierarchy)
  removedItem->ReparentChildrenToParent();

  // Remove from cache
  vtkSubjectHierarchyItem::ItemCache.erase(removedItem->ID);

  // Invoke events
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

  // Reparent children to parent node (to avoid them becoming orphans and thus lost to the hierarchy)
  removedItem->ReparentChildrenToParent();

  // Remove from cache
  vtkSubjectHierarchyItem::ItemCache.erase(removedItem->ID);

  // Invoke events
  this->InvokeEvent(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemRemovedEvent, removedItem.GetPointer());
  this->Modified();

  return true;
}

//---------------------------------------------------------------------------
void vtkSubjectHierarchyItem::ReparentChildrenToParent()
{
  if (!this->Parent)
    {
    // This function is not applicable on the scene item
    return;
    }

  // Use item IDs because the children vector is changing within the for loop
  std::vector<vtkIdType> childIDs;
  std::vector<vtkIdType>::iterator childIDIt;
  this->GetDirectChildren(childIDs);
  for (childIDIt=childIDs.begin(); childIDIt!=childIDs.end(); ++childIDIt)
    {
    vtkSubjectHierarchyItem::ChildVector::iterator childIt;
    for (childIt=this->Children.begin(); childIt!=this->Children.end(); ++childIt)
      {
      vtkSubjectHierarchyItem* childItem = (*childIt);
      if (childIt->GetPointer()->ID == (*childIDIt))
        {
        // Prevent deletion of the item from memory until the events are processed
        vtkSmartPointer<vtkSubjectHierarchyItem> childSmartPointer = childItem;
        // Remove child from this item
        this->Children.erase(childIt);
        // Add child item to this item's parent
        childItem->Parent = this->Parent;
        this->Parent->Children.push_back(childSmartPointer);
        childItem->Modified();
        break;
        }
      }
    }

  this->Parent->Modified();
}

//---------------------------------------------------------------------------
void vtkSubjectHierarchyItem::RemoveAllChildren()
{
  std::vector<vtkIdType> childIDs;
  this->GetAllChildren(childIDs);
  while (childIDs.size())
    {
    // Remove first leaf item found
    std::vector<vtkIdType>::iterator childIt;
    for (childIt=childIDs.begin(); childIt!=childIDs.end(); ++childIt)
      {
      if ((*childIt) == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
        {
        // This can happen when UnresolvedItems are deleted. In that case the items will automatically deconstruct
        childIDs.erase(childIt);
        break;
        }
      vtkSubjectHierarchyItem* currentItem = this->FindChildByID(*childIt);
      if (currentItem && !currentItem->HasChildren())
        {
        // Remove leaf item
        currentItem->Parent->RemoveChild(*childIt);
        // Remove ID of deleted item from list and keep deleting until empty
        childIDs.erase(childIt);
        break;
        }
      }
    } // While there are children to delete
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
        << "' with value '" << this->UIDs[uidName] << "'. Replacing it with value '" << uidValue << "'" );
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
  this->InvokeEvent(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemOwnerPluginSearchRequested, this);
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
    this->InvokeEvent(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemOwnerPluginSearchRequested, this);
    this->Modified();
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
    vtkErrorMacro("GetAttributeFromAncestor: Empty attribute name");
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
    else if ( !level.empty()
      && currentItem->GetAttribute(vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelAttributeName()).compare(level) )
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
    if ( currentItem
      && !currentItem->GetAttribute(vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelAttributeName()).compare(level))
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

  /// Resolve all unresolved items in this subject hierarchy node.
  /// Used when merging a subject hierarchy node into the singleton one after scene import.
  /// Builds the tree and the data node connections from the temporary properties, then adds the items
  /// into the active tree under the scene item
  /// \return Success flag
  bool ResolveUnresolvedItems();

  /// Copy all items from another subject hierarchy nodes to the unresolved list of this node
  void CopyAsUnresolved(vtkMRMLSubjectHierarchyNode* otherShNode);

public:
  /// Scene subject hierarchy item. This is the ancestor of all subject hierarchy items in the tree
  vtkSubjectHierarchyItem* SceneItem;
  /// ID of the scene subject hierarchy item. It is used to access the item from outside the node
  vtkIdType SceneItemID;

  /// Parent item for unresolved items during scene import. Only stores the unresolved children items
  /// in a flat list, contains no other data.
  /// These items are resolved automatically after importing finished. The alternative of this container
  /// would be to store all unresolved items right under the scene, which would result invalid items
  /// potentially being handled as normal, resolved subject hierarchy items.
  vtkSubjectHierarchyItem* UnresolvedItems;

  /// Flag determining whether to skip processing any events. Used only internally
  bool EventsDisabled;
  /// Flag indicating whether resolving unresolved items is underway (after scene import or restore)
  bool IsResolving;

private:
  vtkMRMLSubjectHierarchyNode* External;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode::vtkInternal::vtkInternal(vtkMRMLSubjectHierarchyNode* external)
: EventsDisabled(false)
, IsResolving(false)
, External(external)
{
  // Create scene item
  this->SceneItem = vtkSubjectHierarchyItem::New();
  this->SceneItemID = this->SceneItem->AddToTree(NULL, "Scene", "Scene");

  // Create mock item containing unresolved items
  this->UnresolvedItems = vtkSubjectHierarchyItem::New();
  this->UnresolvedItems->AddToTree(NULL, "UnresolvedItems", "UnresolvedItems");
}

//---------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode::vtkInternal::~vtkInternal()
{
  if (this->SceneItem)
    {
    this->SceneItem->Delete();
    this->SceneItem = NULL;
    }
  if (this->UnresolvedItems)
    {
    this->UnresolvedItems->Delete();
    this->UnresolvedItems = NULL;
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

//---------------------------------------------------------------------------
bool vtkMRMLSubjectHierarchyNode::vtkInternal::ResolveUnresolvedItems()
{
  if (this->UnresolvedItems->Children.size() == 0)
    {
    // If there are no unresolved items then there is nothing to do
    return true;
    }

  // Get scene
  vtkMRMLScene* scene = this->External->GetScene();
  if (!scene)
    {
    vtkErrorWithObjectMacro(this->External, "ResolveUnresolvedItems: Invalid MRML scene");
    return false;
    }

  // Indicate that resolving unresolved items is underway
  bool wasResolving = this->IsResolving;
  this->IsResolving = true;

  // Find unresolved scene item so that later the top-level items can be found and added under the actual scene item instead
  vtkIdType unresolvedSceneItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
  for ( vtkSubjectHierarchyItem::ChildVector::iterator itemIt=this->UnresolvedItems->Children.begin();
        itemIt!=this->UnresolvedItems->Children.end(); ++itemIt )
    {
    if (!itemIt->GetPointer()->GetAttribute(vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelAttributeName()).compare("Scene"))
      {
      unresolvedSceneItemID = itemIt->GetPointer()->TemporaryID;
      this->UnresolvedItems->Children.erase(itemIt);
      break; // There is only one scene item
      }
    }
  if (!unresolvedSceneItemID)
    {
    // For backwards-compatibility reasons, in case of scenes with Level member instead of attribute
    for ( vtkSubjectHierarchyItem::ChildVector::iterator itemIt=this->UnresolvedItems->Children.begin();
          itemIt!=this->UnresolvedItems->Children.end(); ++itemIt )
      {
      if (!itemIt->GetPointer()->Name.compare("Scene") && itemIt->GetPointer()->OwnerPluginName.empty())
        {
        unresolvedSceneItemID = itemIt->GetPointer()->TemporaryID;
        this->UnresolvedItems->Children.erase(itemIt);
        break; // There is only one scene item
        }
      }
    }
  if (!unresolvedSceneItemID)
    {
    vtkErrorWithObjectMacro(this->External, "ResolveUnresolvedItems: Failed to find scene item among unresolved items");
    this->IsResolving = false;
    return false;
    }

  // Mapping unresolved temporary IDs to new resolved IDs
  std::map<vtkIdType,vtkIdType> idMap;

  // Resolve each item and add it to its proper place in the tree under the scene
  unsigned int numberOfUnresolvedItems = this->UnresolvedItems->Children.size(); // Safeguard for checking if an item is resolved in each iteration
  while (this->UnresolvedItems->Children.size())
    {
    // Resolve first item whose parent is the scene or an already resolved item
    for ( vtkSubjectHierarchyItem::ChildVector::iterator itemIt=this->UnresolvedItems->Children.begin();
          itemIt!=this->UnresolvedItems->Children.end(); ++itemIt )
      {
      vtkSubjectHierarchyItem* item = itemIt->GetPointer();
      vtkSubjectHierarchyItem* parentItem = NULL;

      // Add to scene if top-level
      if (item->TemporaryParentItemID == unresolvedSceneItemID)
        {
        parentItem = this->SceneItem;
        }
      else
        {
        // Look for resolved parent
        std::map<vtkIdType,vtkIdType>::iterator parentIt = idMap.find(item->TemporaryParentItemID);
        if (parentIt == idMap.end())
          {
          // Skip if the item's parent is not resolved yet
          continue;
          }
        // Get resolved parent
        parentItem = this->FindItemByID(parentIt->second);
        if (!parentItem)
          {
          vtkErrorWithObjectMacro(this->External, "ResolveUnresolvedItems: Unable to find parent item with ID " << item->TemporaryParentItemID);
          }
        }
      // Parent is resolved and found, clear temporary parent item ID
      item->TemporaryParentItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;

      // Get changed ID of the data node with the ID in the imported scene stored by the unresolved item
      vtkMRMLNode* dataNode = NULL;
      if (!item->TemporaryDataNodeID.empty())
        {
        const char* newDataNodeID = scene->GetChangedID(item->TemporaryDataNodeID.c_str());
        if (newDataNodeID)
          {
          dataNode = scene->GetNodeByID(newDataNodeID);
          }
        else
          {
          // If ID did not change (because there was no conflict with the current scene), then use the original ID
          dataNode = scene->GetNodeByID(item->TemporaryDataNodeID);
          }
        if (!dataNode)
          {
          vtkErrorWithObjectMacro(this->External, "ResolveUnresolvedItems: Unable to find data node with ID " << (newDataNodeID ? newDataNodeID : "(NULL)"));
          }
        // Resolve data node pointer
        item->DataNode = dataNode;
        item->TemporaryDataNodeID = std::string();
        }

      // Add observer for item added event to allow item to be processed externally
      item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAddedEvent, this->External->ItemEventCallbackCommand);

      // Add item to tree
      if (dataNode)
        {
        idMap[item->TemporaryID] = item->AddToTree(parentItem, dataNode);
        }
      else
        {
        idMap[item->TemporaryID] = item->AddToTree(parentItem, item->Name, item->GetAttribute(vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelAttributeName()));
        }

      // Create remaining observations for added item
      item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAboutToBeRemovedEvent, this->External->ItemEventCallbackCommand);
      item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemRemovedEvent, this->External->ItemEventCallbackCommand);
      item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemUIDAddedEvent, this->External->ItemEventCallbackCommand);
      item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemOwnerPluginSearchRequested, this->External->ItemEventCallbackCommand);
      item->AddObserver(vtkCommand::ModifiedEvent, this->External->ItemEventCallbackCommand);

      // Clear temporary ID now that it is resolved
      item->TemporaryID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;

      // Item successfully resolved and added. Remove it from unresolved items and restart search for resolvable item
      this->UnresolvedItems->Children.erase(itemIt);
      break;
      }

    // Make sure that an item was resolved in this iteration. If the number of unresolved items after the iteration is the same
    // as before that, then none of the unresolved items left in the list can be resolved, which means fatal error
    if (numberOfUnresolvedItems == this->UnresolvedItems->Children.size())
      {
      vtkErrorWithObjectMacro(this->External, "ResolveUnresolvedItems: Failed to process " << numberOfUnresolvedItems << " unresolved items");
      this->IsResolving = false;
      return false;
      }
    numberOfUnresolvedItems = this->UnresolvedItems->Children.size();
    }

  // Indicate end of resolving
  this->IsResolving = wasResolving;

  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::vtkInternal::CopyAsUnresolved(vtkMRMLSubjectHierarchyNode* otherShNode)
{
  if (!otherShNode)
    {
    return;
    }

  // Get all children under the scene item in the other subject hierarchy node
  std::vector<vtkIdType> otherItemIDs;
  otherShNode->Internal->SceneItem->GetAllChildren(otherItemIDs);
  // Add the scene item itself, as it is needed to resolve the unresolved items
  otherItemIDs.push_back(otherShNode->Internal->SceneItemID);

  // Add each item to the unresolved flat list
  std::vector<vtkIdType>::iterator otherItemIt;
  for (otherItemIt=otherItemIDs.begin(); otherItemIt!=otherItemIDs.end(); ++otherItemIt)
    {
    vtkSubjectHierarchyItem* currentOtherItem =
      ( ((*otherItemIt) == otherShNode->Internal->SceneItemID) ? otherShNode->Internal->SceneItem
        : otherShNode->Internal->SceneItem->FindChildByID(*otherItemIt) );
    vtkSmartPointer<vtkSubjectHierarchyItem> copiedItem = vtkSmartPointer<vtkSubjectHierarchyItem>::New();
    copiedItem->DeepCopy(currentOtherItem, false); // Do not copy children, only properties
    this->UnresolvedItems->Children.push_back(copiedItem);
    copiedItem->Parent = this->UnresolvedItems;
    }
}

//----------------------------------------------------------------------------
// vtkMRMLSubjectHierarchyNode members
//----------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode::vtkMRMLSubjectHierarchyNode()
{
  this->Internal = new vtkInternal(this);

  // Create subject hierarchy item callback
  this->ItemEventCallbackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->ItemEventCallbackCommand->SetClientData(reinterpret_cast<void*>(this));
  this->ItemEventCallbackCommand->SetCallback(vtkMRMLSubjectHierarchyNode::ItemEventCallback);

  // Connect scene item to events related to removal (the parent item invokes these events, not the removed item itself)
  this->Internal->SceneItem->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAboutToBeRemovedEvent, this->ItemEventCallbackCommand);
  this->Internal->SceneItem->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemRemovedEvent, this->ItemEventCallbackCommand);
}

//----------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode::~vtkMRMLSubjectHierarchyNode()
{
  // Clean up observation
  this->Internal->SceneItem->RemoveAllObservers();
  this->ItemEventCallbackCommand->SetClientData(NULL);

  delete this->Internal;
  this->Internal = NULL;
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "SceneItemID: " << this->Internal->SceneItemID << "\n";
  os << indent << "UnresolvedItemsID: "
    << (this->Internal->UnresolvedItems ? this->Internal->UnresolvedItems->ID : vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID) << "\n";

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
void vtkMRMLSubjectHierarchyNode::PrintItem(vtkIdType itemID, ostream& os, vtkIndent indent)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("PrintItem: Failed to find subject hierarchy item by ID " << itemID);
    return;
    }

  item->PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
const char* vtkMRMLSubjectHierarchyNode::GetNodeTagName()
{
  return "SubjectHierarchy";
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::ReadItemFromXML(const char** atts)
{
  // Create subject hierarchy item, and read its attributes
  vtkSmartPointer<vtkSubjectHierarchyItem> item = vtkSmartPointer<vtkSubjectHierarchyItem>::New();
  item->ReadXMLAttributes(atts);

  // Add item to the unresolved items.
  // These items will be resolved (item ID, parent and children item pointers, data node ID and pointer)
  // after scene import, and moved to the proper position in the tree contained by the already existing
  // singleton subject hierarchy node in vtkInternal::ResolveUnresolvedItems
  this->Internal->UnresolvedItems->Children.push_back(item);
  item->Parent = this->Internal->UnresolvedItems;
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::WriteXML(ostream& of, int nIndent)
{
  // Add attribute to unambiguously identify subject hierarchy 2.0 node in scene MRML files
  this->SetAttribute(SUBJECTHIERARCHY_VERSION_ATTRIBUTE_NAME.c_str(), "2");

  Superclass::WriteXML(of,nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::WriteNodeBodyXML(ostream& of, int indent)
{
  if (this->Internal->SceneItem)
    {
    // Have the scene item write the whole tree
    this->Internal->SceneItem->WriteXML(of, indent+2, this);
    }
  if (this->Internal->UnresolvedItems && this->Internal->UnresolvedItems->Children.size())
    {
    // Write unresolved items
    this->Internal->UnresolvedItems->WriteXML(of, indent+2, this);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  // IMPORTANT! Subject hierarchy nodes should not be copied!
  // The only valid case when it is copied, is when a Scene View node stores or restores the hierarchy.

  vtkMRMLSubjectHierarchyNode* otherShNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(anode);

  // 1. Scene is restoring -> Copy unresolved items into this subject hierarchy
  //                          (resolve in plugin logic when restoring finished)
  if (this->GetScene() && this->GetScene()->IsRestoring())
    {
    // Prevent events from being invoked from this subject hierarchy node
    bool wereEventsDisabled = this->Internal->EventsDisabled;
    this->Internal->EventsDisabled = true;
    // Remove all items. The items from the other node will replace each item in this node
    // after resolving them after restoring finished
    this->Internal->SceneItem->RemoveAllChildren();
    // Copy all unresolved items from the other subject hierarchy node into this node
    this->Internal->UnresolvedItems->DeepCopy(otherShNode->Internal->UnresolvedItems);
    // Unblock events
    this->Internal->EventsDisabled = wereEventsDisabled;
    }
  // 2. Scene is being stored -> Copy all items into the unresolved item list
  else
    {
    this->Internal->CopyAsUnresolved(otherShNode);
    }

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

  if (item->GetAttribute(vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelAttributeName()).compare(level))
    {
    item->SetAttribute(vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelAttributeName(), level);
    }
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

  return item->GetAttribute(vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelAttributeName());
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::SetItemOwnerPluginName(vtkIdType itemID, std::string ownerPluginName)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("SetItemOwnerPluginName: Failed to find subject hierarchy item by ID " << itemID);
    return;
    }

  if (item->OwnerPluginName.compare(ownerPluginName))
    {
    item->OwnerPluginName = ownerPluginName;
    this->InvokeCustomModifiedEvent(SubjectHierarchyItemModifiedEvent, (void*)&itemID);
    }
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

  if (item->Expanded != expanded)
    {
    item->Expanded = expanded;
    this->InvokeCustomModifiedEvent(SubjectHierarchyItemModifiedEvent, (void*)&itemID);
    }
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
vtkIdType vtkMRMLSubjectHierarchyNode::GetItemByPositionUnderParent(vtkIdType parentItemID, int position)
{
  vtkSubjectHierarchyItem* parentItem = this->Internal->FindItemByID(parentItemID);
  if (!parentItem)
    {
    vtkErrorMacro("GetItemByPositionUnderParent: Failed to find subject hierarchy item by ID " << parentItemID);
    return false;
    }
  return parentItem->GetChildByPositionUnderParent(position);
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
void vtkMRMLSubjectHierarchyNode::RequestOwnerPluginSearch(vtkIdType itemID)
{
  // Not used, but we need to make sure that the item exists
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("RequestOwnerPluginSearch: Failed to find subject hierarchy item by ID " << itemID);
    return;
    }

  // Invoke the node event directly, thus saving an extra callback round
  this->InvokeCustomModifiedEvent(SubjectHierarchyItemOwnerPluginSearchRequested, (void*)&itemID);
}

//---------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::RequestOwnerPluginSearch(vtkMRMLNode* dataNode)
{
  vtkIdType itemID = this->GetItemByDataNode(dataNode);
  if (itemID == this->GetInvalidItemID())
    {
    itemID = this->CreateItem(this->GetSceneItemID(), dataNode);
    }
  this->RequestOwnerPluginSearch(itemID);
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLSubjectHierarchyNode::CreateItem(vtkIdType parentItemID, vtkMRMLNode* dataNode)
{
  if (!dataNode)
    {
    vtkErrorMacro("CreateItem: Valid data node must be given to this function. Call CreateItem with name argument to create item without data node");
    return INVALID_ITEM_ID;
    }

  // Use existing subject hierarchy item if found (only one subject hierarchy item can be associated with a data node)
  vtkIdType itemID = this->GetItemByDataNode(dataNode);
  if (itemID != INVALID_ITEM_ID)
    {
    // Set properties if item already existed for data node
    // This should be the case every time this function is called for a reason other than
    // merging two hierarchies, because subject hierarchy items are added automatically
    // for supported data MRML nodes (supported = there is a plugin that can "own" it)
    vtkSubjectHierarchyItem* item = this->Internal->SceneItem->FindChildByID(itemID);

    // The name of the data node is used, so empty name is set
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
    }
  // No subject hierarchy item was found for the given data node
  else
    {
    vtkSubjectHierarchyItem* parentItem = this->Internal->FindItemByID(parentItemID);
    if (!parentItem)
      {
      vtkErrorMacro("CreateItem: Failed to find parent subject hierarchy item by ID " << parentItemID);
      return INVALID_ITEM_ID;
      }

    // Create subject hierarchy item
    vtkSmartPointer<vtkSubjectHierarchyItem> item = vtkSmartPointer<vtkSubjectHierarchyItem>::New();

    // Make item connections
    item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAddedEvent, this->ItemEventCallbackCommand);
    item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAboutToBeRemovedEvent, this->ItemEventCallbackCommand);
    item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemRemovedEvent, this->ItemEventCallbackCommand);
    item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemUIDAddedEvent, this->ItemEventCallbackCommand);
    item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemOwnerPluginSearchRequested, this->ItemEventCallbackCommand);
    item->AddObserver(vtkCommand::ModifiedEvent, this->ItemEventCallbackCommand);

    // Observe data node events
    if (dataNode)
      {
      dataNode->AddObserver(vtkCommand::ModifiedEvent, this->ItemEventCallbackCommand);
      dataNode->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent, this->ItemEventCallbackCommand);
      dataNode->AddObserver(vtkMRMLDisplayableNode::DisplayModifiedEvent, this->ItemEventCallbackCommand);
      }

    // Add item to the tree
    itemID = item->AddToTree(parentItem, dataNode);
    }

  // Request owner plugin search (it may depend on the parent, data node etc.)
  this->RequestOwnerPluginSearch(itemID);

  return itemID;
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLSubjectHierarchyNode::CreateHierarchyItem(vtkIdType parentItemID, std::string name, std::string level)
{
  vtkSubjectHierarchyItem* parentItem = this->Internal->FindItemByID(parentItemID);
  if (!parentItem)
    {
    vtkErrorMacro("CreateHierarchyItem: Failed to find parent subject hierarchy item by ID " << parentItemID);
    return INVALID_ITEM_ID;
    }

  // Create subject hierarchy item
  vtkSmartPointer<vtkSubjectHierarchyItem> item = vtkSmartPointer<vtkSubjectHierarchyItem>::New();

  // Make item connections
  item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAddedEvent, this->ItemEventCallbackCommand);
  item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAboutToBeRemovedEvent, this->ItemEventCallbackCommand);
  item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemRemovedEvent, this->ItemEventCallbackCommand);
  item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemUIDAddedEvent, this->ItemEventCallbackCommand);
  item->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemOwnerPluginSearchRequested, this->ItemEventCallbackCommand);
  item->AddObserver(vtkCommand::ModifiedEvent, this->ItemEventCallbackCommand);

  // Add item to the tree
  vtkIdType itemID = item->AddToTree(parentItem, name, level);

  // Request owner plugin search (it may depend on the parent etc.)
  this->RequestOwnerPluginSearch(itemID);

  return itemID;
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLSubjectHierarchyNode::CreateSubjectItem(vtkIdType parentItemID, std::string name)
{
  return this->CreateHierarchyItem(parentItemID, name, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient());
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLSubjectHierarchyNode::CreateStudyItem(vtkIdType parentItemID, std::string name)
{
  return this->CreateHierarchyItem(parentItemID, name, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy());
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLSubjectHierarchyNode::CreateFolderItem(vtkIdType parentItemID, std::string name)
{
  return this->CreateHierarchyItem(parentItemID, name, vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder());
}

//----------------------------------------------------------------------------
bool vtkMRMLSubjectHierarchyNode::RemoveItem(vtkIdType itemID, bool removeDataNode/*=true*/, bool recursive/*=true*/)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("RemoveItem: Failed to find subject hierarchy item by ID " << itemID);
    return false;
    }

  // Remove all children if recursive deletion is requested or if it's the parent of a virtual branch
  // Start with the leaf nodes so that triggered updates are faster (no reparenting done after deleting intermediate items)
  if (recursive || item->IsVirtualBranchParent())
    {
    std::vector<vtkIdType> virtualItemIDs; // List of virtual item IDs that do not need to be explicitly deleted
    std::vector<vtkIdType> childIDs;
    item->GetAllChildren(childIDs);
    while (childIDs.size())
      {
      // Remove first leaf node found
      // (or if the parent of a virtual branch, in which the items are automatically removed when their parent is removed)
      std::vector<vtkIdType>::iterator childIt;
      for (childIt=childIDs.begin(); childIt!=childIDs.end(); ++childIt)
        {
        // Skip if virtual item, because its data node and subject hierarchy item was already deleted
        if (std::find(virtualItemIDs.begin(), virtualItemIDs.end(), (*childIt)) != virtualItemIDs.end())
          {
          childIDs.erase(childIt);
          break;
          }

        // Get item by ID and delete it if leaf or virtual branch parent
        vtkSubjectHierarchyItem* currentItem = this->Internal->SceneItem->FindChildByID(*childIt);
        if (!currentItem)
          {
          // Already deleted item ID was in the list
          vtkErrorMacro("RemoveItem: Failed to find subject hierarchy item by ID " << (*childIt));
          childIDs.erase(childIt);
          break;
          }
        if ( !currentItem->HasChildren() || currentItem->IsVirtualBranchParent() )
          {
          // Mark child item IDs in case it is a virtual branch parent, because those items do not need to be explicitly deleted
          if (currentItem->IsVirtualBranchParent())
            {
            std::vector<vtkIdType> currentVirtualItemIDs;
            currentItem->GetDirectChildren(currentVirtualItemIDs);
            virtualItemIDs.insert(virtualItemIDs.end(), currentVirtualItemIDs.begin(), currentVirtualItemIDs.end());
            }

          // Remove data node from scene if requested.. In that case removing the item explicitly
          // is not necessary because removing the node triggers removing the item automatically
          if (removeDataNode && currentItem->DataNode && this->Scene)
            {
            this->Scene->RemoveNode(currentItem->DataNode.GetPointer());
            }
          // Remove leaf item from its parent if not in virtual branch (if in virtual branch, then they will be removed
          // automatically when their parent is removed)
          else if (!currentItem->Parent->IsVirtualBranchParent())
            {
            currentItem->Parent->RemoveChild(*childIt);
            }

          // Remove ID of deleted item (or virtual branch leaf) from list and keep deleting until empty
          childIDs.erase(childIt);
          break;
          }
        }
      } // While there are children to delete
    }

  // Remove data node of given item from scene if requested. In that case removing the item explicitly
  // is not necessary because removing the node triggers removing the item automatically
  if (removeDataNode && item->DataNode && this->Scene)
    {
    this->Scene->RemoveNode(item->DataNode.GetPointer());
    }
  // Remove given item itself if it's not the scene
  // (the scene item must always exist, and it doesn't have the parent that can perform the removal)
  else if (itemID != this->Internal->SceneItemID)
    {
    item->Parent->RemoveChild(item);
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLSubjectHierarchyNode::RemoveItemChildren(vtkIdType itemID, bool removeDataNodes/*=true*/, bool recursive/*=true*/)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("RemoveItem: Failed to find subject hierarchy item by ID " << itemID);
    return false;
    }

  // Remove all direct children
  bool success = true;
  std::vector<vtkIdType> childIDs;
  item->GetDirectChildren(childIDs);
  std::vector<vtkIdType>::iterator childIt;
  for (childIt=childIDs.begin(); childIt!=childIDs.end(); ++childIt)
    {
    if (!this->RemoveItem((*childIt), removeDataNodes, recursive))
      {
      success = false;
      }
    }

  return success;
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
  vtkSubjectHierarchyItem* parentItem = this->Internal->FindItemByID(parentItemID);
  if (!parentItem)
    {
    vtkErrorMacro("SetItemParent: Failed to find subject hierarchy item by ID " << parentItemID);
    return;
    }

  // Perform reparenting
  item->Reparent(parentItem);
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
void vtkMRMLSubjectHierarchyNode::GetItemChildren(vtkIdType itemID, vtkIdList* childIDs, bool recursive/*=false*/)
{
  if (!childIDs)
    {
    vtkErrorMacro("GetItemChildren: Invalid output child list");
    return;
    }
  childIDs->Initialize();

  std::vector<vtkIdType> childIdVector;
  this->GetItemChildren(itemID, childIdVector, recursive);

  std::vector<vtkIdType>::iterator childIt;
  for (childIt=childIdVector.begin(); childIt!=childIdVector.end(); ++childIt)
    {
    childIDs->InsertNextId(*childIt);
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
  return item->Reparent(newParentItem);
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

  // If before item ID is invalid (and the item is NULL), then move to the end
  vtkSubjectHierarchyItem* beforeItem = this->Internal->SceneItem->FindChildByID(beforeItemID);

  // Perform move
  return item->Move(beforeItem);
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLSubjectHierarchyNode::GetItemByUID(const char* uidName, const char* uidValue)
{
  if (!uidName || !uidValue)
    {
    vtkErrorMacro("GetSubjectHierarchyNodeByUID: Invalid UID name or value");
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
    vtkErrorMacro("GetSubjectHierarchyItemByUIDList: Invalid UID name or value");
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
    vtkErrorMacro("GetItemByDataNode: Invalid data node to find");
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
    return INVALID_ITEM_ID;
    }

  // Search only one level (not recursive)
  std::vector<vtkIdType> foundItemIDs;
  parentItem->FindChildrenByName(name, foundItemIDs, false, false);
  if (foundItemIDs.size() == 0)
    {
    vtkDebugMacro("GetItemChildWithName: Failed to find subject hierarchy item with name '" << name
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
vtkMRMLNode* vtkMRMLSubjectHierarchyNode::GetParentDataNode(vtkMRMLNode* dataNode, bool recursive/*=false*/)
{
  vtkIdType itemID = this->GetItemByDataNode(dataNode);
  if (itemID == INVALID_ITEM_ID)
    {
    vtkErrorMacro("GetParentDataNode: Given data node (" << (dataNode ? dataNode->GetName() : "NULL")
      << ") cannot be found in subject hierarchy");
    return NULL;
    }

  vtkIdType parentItemID = this->GetItemParent(itemID);
  vtkMRMLNode* parentDataNode = this->GetItemDataNode(parentItemID);
  while (recursive && parentItemID != INVALID_ITEM_ID && parentDataNode == NULL)
    {
    parentItemID = this->GetItemParent(parentItemID);
    parentDataNode = this->GetItemDataNode(parentItemID);
    }
  return parentDataNode;
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
    return;
    }

  // Get all child displayable nodes for branch
  vtkNew<vtkCollection> childDisplayableNodes;
  this->GetDataNodesInBranch(itemID, childDisplayableNodes.GetPointer(), "vtkMRMLDisplayableNode");

  childDisplayableNodes->InitTraversal();
  std::set<vtkIdType> parentItems;
  for (int childNodeIndex = 0; childNodeIndex < childDisplayableNodes->GetNumberOfItems(); ++childNodeIndex)
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
      bool wereEventsDisabled = this->Internal->EventsDisabled;
      this->Internal->EventsDisabled = true; // Prevent the views from updating before all the display nodes are modified
      displayableNode->SetDisplayVisibility(visible);
      this->Internal->EventsDisabled = wereEventsDisabled;

      // Set slice intersection visibility through display node
      displayNode = displayableNode->GetDisplayNode();
      if (displayNode)
        {
        displayNode->SetSliceIntersectionVisibility(visible);
        }
      displayableNode->Modified();

      // Collect all parents
      vtkIdType itemForDisplayableNode = this->GetItemByDataNode(displayableNode);
      if (itemForDisplayableNode == INVALID_ITEM_ID)
        {
        vtkErrorMacro("SetDisplayVisibilityForBranch: Failed to find subject hierarchy item for node " << displayableNode->GetName())
        continue;
        }
      this->ItemModified(itemID);
      vtkIdType parentItemID = itemID;
      while ( (parentItemID = this->GetItemParent(parentItemID)) != this->Internal->SceneItemID ) // The double parentheses avoids a Linux build warning
        {
        parentItems.insert(parentItemID);
        }
      }
    }

  // Invoke modified event for all parent items so that their icons are refreshed in the view
  for (std::set<vtkIdType>::iterator parentIt = parentItems.begin(); parentIt != parentItems.end(); ++ parentIt)
    {
    this->ItemModified((*parentIt));
    }
}

//---------------------------------------------------------------------------
int vtkMRMLSubjectHierarchyNode::GetDisplayVisibilityForBranch(vtkIdType itemID)
{
  int visible = -1;

  // Get all child displayable nodes for branch
  vtkNew<vtkCollection> childDisplayableNodes;
  this->GetDataNodesInBranch(itemID, childDisplayableNodes.GetPointer(), "vtkMRMLDisplayableNode");

  // Determine visibility state based on all displayable nodes involved
  childDisplayableNodes->InitTraversal();
  for (int childNodeIndex=0; childNodeIndex<childDisplayableNodes->GetNumberOfItems(); ++childNodeIndex)
    {
    vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(
      childDisplayableNodes->GetItemAsObject(childNodeIndex) );
    // Omit volume nodes from the process (they are displayed differently than every other type)
    // TODO: This is not very elegant or safe, it would be better to distinguish between visibility modes, or overhaul the visibility features completely
    if (displayableNode && !displayableNode->IsA("vtkMRMLVolumeNode"))
      {
      // If we set visibility
      if (visible == -1)
        {
        visible = displayableNode->GetDisplayVisibility();
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

  return !item->GetAttribute(vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelAttributeName()).compare(level);
}

//---------------------------------------------------------------------------
bool vtkMRMLSubjectHierarchyNode::IsItemVirtualBranchParent(vtkIdType itemID)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("IsItemVirtualBranchParent: Failed to find subject hierarchy item by ID " << itemID);
    return false;
    }

  return item->IsVirtualBranchParent();
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
void vtkMRMLSubjectHierarchyNode::GetItemsReferencedFromItemByDICOM(vtkIdType itemID, vtkIdList* referencedIdList)
{
  if (!referencedIdList)
    {
    vtkErrorMacro("GetItemsReferencedFromItemByDICOM: Invalid output ID list");
    return;
    }

  referencedIdList->Reset();
  std::vector<vtkIdType> referencedItemIDs = this->GetItemsReferencedFromItemByDICOM(itemID);
  std::vector<vtkIdType>::iterator itemIt;
  for (itemIt=referencedItemIDs.begin(); itemIt!=referencedItemIDs.end(); ++itemIt)
    {
    referencedIdList->InsertNextId(*itemIt);
    }
}

//---------------------------------------------------------------------------
std::vector<vtkIdType> vtkMRMLSubjectHierarchyNode::GetItemsReferencingItemByDICOM(vtkIdType itemID)
{
  std::vector<vtkIdType> referencingItemIDs;
  vtkSubjectHierarchyItem* item = this->Internal->SceneItem->FindChildByID(itemID);
  if (!item)
    {
    vtkErrorMacro("GetItemsReferencingItemByDICOM: Failed to find non-scene subject hierarchy item by ID " << itemID);
    return referencingItemIDs;
    }

  // Get first SOP instance UID
  std::string uidsString = item->GetUID(vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName());
  if (uidsString.empty())
    {
    vtkDebugMacro("GetItemsReferencingItemByDICOM: No DICOM UIDs in item with ID " << itemID);
    return referencingItemIDs;
    }
  std::vector<std::string> uidVector;
  this->DeserializeUIDList(uidsString, uidVector);

  // Find subject hierarchy items containing first SOP instance UID in referenced UIDs attribute
  std::vector<vtkIdType> allItemIDs;
  this->Internal->SceneItem->GetAllChildren(allItemIDs);
  for (std::vector<vtkIdType>::iterator itemIt=allItemIDs.begin(); itemIt!=allItemIDs.end(); ++itemIt)
    {
    vtkSubjectHierarchyItem* currentItem =this->Internal->SceneItem->FindChildByID(*itemIt);
    std::string referencedUids = currentItem->GetAttribute(vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName());
    bool referencesUid = false;
    for (std::vector<std::string>::iterator uidIt=uidVector.begin(); uidIt!=uidVector.end(); ++uidIt)
      {
      if (referencedUids.find(*uidIt) != std::string::npos)
        {
        referencesUid = true;
        break;
        }
      }
    if (referencesUid)
      {
      // UID is referenced, add referencing item to the list
      referencingItemIDs.push_back(*itemIt);
      }
    }

  return referencingItemIDs;
}

//---------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::GetItemsReferencingItemByDICOM(vtkIdType itemID, vtkIdList* referencingIdList)
{
  if (!referencingIdList)
    {
    vtkErrorMacro("GetItemsReferencingItemByDICOM: Invalid output ID list");
    return;
    }

  referencingIdList->Reset();
  std::vector<vtkIdType> referencingItemIDs = this->GetItemsReferencingItemByDICOM(itemID);
  std::vector<vtkIdType>::iterator itemIt;
  for (itemIt=referencingItemIDs.begin(); itemIt!=referencingItemIDs.end(); ++itemIt)
    {
    referencingIdList->InsertNextId(*itemIt);
    }
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
int vtkMRMLSubjectHierarchyNode::GetNumberOfItems()
{
  std::vector<vtkIdType> allItems;
  this->Internal->SceneItem->GetAllChildren(allItems);
  return allItems.size();
}

//---------------------------------------------------------------------------
int vtkMRMLSubjectHierarchyNode::GetNumberOfItemChildren(vtkIdType itemID, bool recursive/*=false*/)
{
  vtkSubjectHierarchyItem* item = this->Internal->FindItemByID(itemID);
  if (!item)
    {
    vtkErrorMacro("GetNumberOfItemChildren: Failed to find subject hierarchy item by ID " << itemID);
    return -1;
    }

  std::vector<vtkIdType> childIDs;
  if (recursive)
    {
    item->GetAllChildren(childIDs);
    }
  else
    {
    item->GetDirectChildren(childIDs);
    }
  return childIDs.size();
}

//---------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyNode::ItemEventCallback(vtkObject* caller, unsigned long eid, void* clientData, void* callData)
{
  vtkMRMLSubjectHierarchyNode* self = reinterpret_cast<vtkMRMLSubjectHierarchyNode*>(clientData);
  if (!self || self->Internal->EventsDisabled)
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
    case vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemOwnerPluginSearchRequested:
      {
      // Get item from call data
      vtkSubjectHierarchyItem* item = reinterpret_cast<vtkSubjectHierarchyItem*>(callData);
      if (item)
        {
        self->InvokeCustomModifiedEvent(eid, (void*)&item->ID);
        self->Modified(); // Indicate that the content of the subject hierarchy node has changed, so it needs to be saved
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
        self->Modified(); // Indicate that the content of the subject hierarchy node has changed, so it needs to be saved
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
  std::vector<vtkMRMLNode*> shNodesInScene;
  scene->GetNodesByClass("vtkMRMLSubjectHierarchyNode", shNodesInScene);

  if (shNodesInScene.size() == 0)
    {
    vtkSmartPointer<vtkMRMLSubjectHierarchyNode> newShNode = vtkSmartPointer<vtkMRMLSubjectHierarchyNode>::New();
    newShNode->SetName("SubjectHierarchy");
    scene->AddNode(newShNode);

    vtkDebugWithObjectMacro(newShNode.GetPointer(), "vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode: "
      "New subject hierarchy node created as none was found in the scene");
    return newShNode;
    }

  // Return subject hierarchy node if there is only one

  vtkMRMLSubjectHierarchyNode* firstShNode = NULL;
  firstShNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(shNodesInScene[0]);
  if (!firstShNode)
    {
    vtkErrorWithObjectMacro( scene,
      "vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode: Invalid first subject hierarchy node" );
    return firstShNode;
    }
  if (shNodesInScene.size() == 1)
    {
    // Resolve unresolved items. There are unresolved items after importing or restoring a scene
    // (do not perform this consolidation operation while the scene is processing)
    if ( ! ( scene->IsBatchProcessing() || scene->IsImporting() || scene->IsRestoring() || scene->IsClosing()
          || firstShNode == NULL || firstShNode->Internal->IsResolving )
      && !firstShNode->Internal->ResolveUnresolvedItems() )
      {
      // Remove invalid subject hierarchy node so that it can be rebuilt from scratch
      scene->RemoveNode(firstShNode);
      vtkErrorWithObjectMacro( scene,
        "vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode: Failed to resolve unresolved subject hierarchy items, re-building subject hierarchy from scratch" );
      return NULL;
      }

    return firstShNode;
    }

  // Do not perform merge operations while the scene is processing, simply return the first node
  if ( scene->IsBatchProcessing() || scene->IsImporting() || scene->IsClosing()
    || firstShNode->Internal->IsResolving )
    {
    return firstShNode;
    }

  // Enter resolving state for merging
  bool wasResolving = firstShNode->Internal->IsResolving;
  firstShNode->Internal->IsResolving = true;
  // Invoke event marking the end of the resolving operation
  firstShNode->InvokeCustomModifiedEvent(SubjectHierarchyStartResolveEvent);

  // Merge subject hierarchy nodes into the first one found
  std::vector<vtkMRMLSubjectHierarchyNode*> mergedShNodes;
  for (std::vector<vtkMRMLNode*>::iterator shNodeIt=shNodesInScene.begin()+1; shNodeIt!=shNodesInScene.end(); ++shNodeIt)
    {
    vtkMRMLSubjectHierarchyNode* currentShNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(*shNodeIt);
    if (currentShNode == NULL)
      {
      continue;
      }
    // Prevent events from being invoked from both subject hierarchy nodes
    bool wereEventsDisabled = firstShNode->Internal->EventsDisabled;
    firstShNode->Internal->EventsDisabled = true;

    // Copy all unresolved items from the merged subject hierarchy node into first node
    firstShNode->Internal->UnresolvedItems->DeepCopy(currentShNode->Internal->UnresolvedItems);

    // Mark node as merged so that it is removed later
    mergedShNodes.push_back(currentShNode);

    // Re-enable events
    firstShNode->Internal->EventsDisabled = wereEventsDisabled;

    // Resolve all unresolved items (only SubjectHierarchyItemAddedEvent is invoked from the node in the process)
    if (!firstShNode->Internal->ResolveUnresolvedItems())
      {
      // Remove all subject hierarchy nodes including the invalid one so that it can be rebuilt from scratch

      // When a node is deleted, other nodes may be deleted as a side effect, therefore we retrieve all the node IDs
      // and look up each node by ID when deleting. This way we can detect if a node has been already deleted
      // and do not attempt to delete it again.

      // Retrieve node IDs
      std::vector<std::string> allShNodeIDs;
      std::vector<vtkMRMLNode*> allShNodes;
      scene->GetNodesByClass("vtkMRMLSubjectHierarchyNode", allShNodes);
      for (std::vector<vtkMRMLNode*>::iterator allShNodeIt=allShNodes.begin(); allShNodeIt!=allShNodes.end(); ++allShNodeIt)
        {
        const char* nodeID = (*allShNodeIt)->GetID();
        if (nodeID)
          {
          allShNodeIDs.push_back(nodeID);
          }
        }

      // Delete nodes by node IDs:
      for (std::vector<std::string>::iterator allShNodeIDIt = allShNodeIDs.begin(); allShNodeIDIt != allShNodeIDs.end(); ++allShNodeIDIt)
        {
        vtkMRMLNode* node = scene->GetNodeByID(*allShNodeIDIt);
        if (node)
          {
          scene->RemoveNode(node);
          }
        }

      vtkErrorWithObjectMacro( scene,
        "vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode: Failed to merge subject hierarchy nodes, re-building subject hierarchy from scratch" );
      return vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
      }
    }
  // Remove merged subject hierarchy nodes from the scene
  for (std::vector<vtkMRMLSubjectHierarchyNode*>::iterator mergedShIt=mergedShNodes.begin(); mergedShIt!=mergedShNodes.end(); ++mergedShIt)
    {
    scene->RemoveNode(*mergedShIt);
    }

  // Indicate that resolving unresolved items is underway
  firstShNode->Internal->IsResolving = wasResolving;
  // Invoke event marking the end of the resolving operation
  firstShNode->InvokeCustomModifiedEvent(SubjectHierarchyEndResolveEvent);

  // Return the first (and now only) subject hierarchy node into which the others were merged
  return firstShNode;
}

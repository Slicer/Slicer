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
#include "vtkMRMLSubjectHierarchyLegacyNode.h"
#include "vtkMRMLSubjectHierarchyConstants.h"

// MRML includes
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
const std::string vtkMRMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_ITEM_SEPARATOR = std::string(":");
const std::string vtkMRMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR = std::string("; ");

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSubjectHierarchyLegacyNode);

//----------------------------------------------------------------------------
vtkMRMLSubjectHierarchyLegacyNode::vtkMRMLSubjectHierarchyLegacyNode()
{
  this->SetLevel("Other");

  this->UIDs.clear();
}

//----------------------------------------------------------------------------
vtkMRMLSubjectHierarchyLegacyNode::~vtkMRMLSubjectHierarchyLegacyNode()
{
  this->UIDs.clear();

  this->SetLevel(nullptr);
  this->SetOwnerPluginName(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyLegacyNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << " Level=\""
    << (this->Level ? this->Level : "NULL" ) << "\n";

  os << indent << " OwnerPluginName=\""
    << (this->OwnerPluginName ? this->OwnerPluginName : "NULL" ) << "\n";

  os << indent << " UIDs=\"";
  for (std::map<std::string, std::string>::iterator uidsIt = this->UIDs.begin(); uidsIt != this->UIDs.end(); ++uidsIt)
    {
    os << uidsIt->first << vtkMRMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR.c_str()
      << uidsIt->second << vtkMRMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_ITEM_SEPARATOR.c_str();
    }
  os << "\"";
}

//----------------------------------------------------------------------------
const char* vtkMRMLSubjectHierarchyLegacyNode::GetNodeTagName()
{
  return "SubjectHierarchyLegacy";
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyLegacyNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "Level"))
      {
      this->SetLevel(attValue);
      }
    else if (!strcmp(attName, "OwnerPluginName"))
      {
      this->SetOwnerPluginName(attValue);
      }
    else if (!strcmp(attName, "UIDs"))
      {
      std::stringstream ss;
      ss << attValue;
      std::string valueStr = ss.str();

      this->UIDs.clear();
      size_t itemSeparatorPosition = valueStr.find(vtkMRMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_ITEM_SEPARATOR);
      while (itemSeparatorPosition != std::string::npos)
        {
        std::string itemStr = valueStr.substr(0, itemSeparatorPosition);
        size_t nameValueSeparatorPosition = itemStr.find(vtkMRMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR);

        std::string name = itemStr.substr(0, nameValueSeparatorPosition);
        std::string value = itemStr.substr(nameValueSeparatorPosition + vtkMRMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR.size());
        this->UIDs[name] = value;

        valueStr = valueStr.substr(itemSeparatorPosition + vtkMRMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_ITEM_SEPARATOR.size());
        itemSeparatorPosition = valueStr.find(vtkMRMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_ITEM_SEPARATOR);
        }
      if (! valueStr.empty() )
        {
        std::string itemStr = valueStr.substr(0, itemSeparatorPosition);
        size_t tagLevelSeparatorPosition = itemStr.find(vtkMRMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR);

        std::string name = itemStr.substr(0, tagLevelSeparatorPosition);
        std::string value = itemStr.substr(tagLevelSeparatorPosition + vtkMRMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR.size());
        this->UIDs[name] = value;
        }
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyLegacyNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);

  of << " Level=\"" << (this->Level ? this->Level : "" ) << "\"";

  of << " OwnerPluginName=\""
    << (this->OwnerPluginName ? this->OwnerPluginName : "" ) << "\"";

  of << " UIDs=\"";
  for (std::map<std::string, std::string>::iterator uidsIt = this->UIDs.begin(); uidsIt != this->UIDs.end(); ++uidsIt)
    {
    of << vtkMRMLNode::XMLAttributeEncodeString(uidsIt->first)
       << vtkMRMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR
       << vtkMRMLNode::XMLAttributeEncodeString(uidsIt->second)
       << vtkMRMLSubjectHierarchyLegacyNode::SUBJECTHIERARCHY_UID_ITEM_SEPARATOR;
    }
  of << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyLegacyNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLSubjectHierarchyLegacyNode* otherNode = vtkMRMLSubjectHierarchyLegacyNode::SafeDownCast(anode);
  if (otherNode)
    {
    this->SetLevel(otherNode->Level);
    this->SetOwnerPluginName(otherNode->OwnerPluginName);

    this->UIDs = otherNode->UIDs;
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
std::string vtkMRMLSubjectHierarchyLegacyNode::GetUID(std::string uidName)
{
  // Use the find function to prevent adding an empty UID to the list
  if (this->UIDs.find(uidName) == this->UIDs.end())
    {
    return std::string();
    }
  return this->UIDs[uidName];
}

//----------------------------------------------------------------------------
std::map<std::string, std::string> vtkMRMLSubjectHierarchyLegacyNode::GetUIDs()const
{
  return this->UIDs;
}

//---------------------------------------------------------------------------
vtkMRMLSubjectHierarchyLegacyNode* vtkMRMLSubjectHierarchyLegacyNode::GetSubjectHierarchyLegacyNodeByUID(
  vtkMRMLScene* scene, const char* uidName, const char* uidValue)
{
  if (!scene || !uidName || !uidValue)
    {
    std::cerr << "vtkMRMLSubjectHierarchyLegacyNode::GetSubjectHierarchyNodeByUID: Invalid scene or searched UID!" << std::endl;
    return nullptr;
    }

  std::vector<vtkMRMLNode*> subjectHierarchyNodes;
  unsigned int numberOfNodes = scene->GetNodesByClass("vtkMRMLSubjectHierarchyLegacyNode", subjectHierarchyNodes);
  for (unsigned int shNodeIndex=0; shNodeIndex<numberOfNodes; shNodeIndex++)
    {
    vtkMRMLSubjectHierarchyLegacyNode* node = vtkMRMLSubjectHierarchyLegacyNode::SafeDownCast(subjectHierarchyNodes[shNodeIndex]);
    if (node)
      {
      std::string nodeUidValueStr = node->GetUID(uidName);
      const char* nodeUidValue = nodeUidValueStr.c_str();
      if (nodeUidValue && !strcmp(uidValue, nodeUidValue))
        {
        return node;
        }
      }
    }

  return nullptr;
}

//---------------------------------------------------------------------------
vtkMRMLSubjectHierarchyLegacyNode* vtkMRMLSubjectHierarchyLegacyNode::GetSubjectHierarchyLegacyNodeByUIDList(
  vtkMRMLScene* scene, const char* uidName, const char* uidValue)
{
  if (!scene || !uidName || !uidValue)
    {
    std::cerr << "vtkMRMLSubjectHierarchyLegacyNode::GetSubjectHierarchyNodeByUID: Invalid scene or searched UID!" << std::endl;
    return nullptr;
    }

  std::vector<vtkMRMLNode*> subjectHierarchyNodes;
  unsigned int numberOfNodes = scene->GetNodesByClass("vtkMRMLSubjectHierarchyLegacyNode", subjectHierarchyNodes);
  for (unsigned int shNodeIndex=0; shNodeIndex<numberOfNodes; shNodeIndex++)
    {
    vtkMRMLSubjectHierarchyLegacyNode* node = vtkMRMLSubjectHierarchyLegacyNode::SafeDownCast(subjectHierarchyNodes[shNodeIndex]);
    if (node)
      {
      std::string nodeUidValueStr = node->GetUID(uidName);
      if (nodeUidValueStr.find(uidValue) != std::string::npos)
        {
        return node;
        }
      }
    }

  return nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLSubjectHierarchyLegacyNode::DeserializeUIDList(std::string uidListString, std::vector<std::string>& deserializedUIDList)
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
std::vector<vtkMRMLSubjectHierarchyLegacyNode*> vtkMRMLSubjectHierarchyLegacyNode::GetSubjectHierarchyNodesReferencedByDICOM()
{
  std::vector<vtkMRMLSubjectHierarchyLegacyNode*> referencedNodes;

  vtkMRMLScene* scene = this->GetScene();
  if (!scene)
    {
    vtkErrorMacro("GetSubjectHierarchyNodesReferencedByDICOM: Invalid MRML scene!");
    return referencedNodes;
    }

  // Get referenced SOP instance UIDs
  const char* referencedInstanceUIDsAttribute = this->GetAttribute(
    vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName().c_str() );
  if (!referencedInstanceUIDsAttribute)
    {
    return referencedNodes;
    }

  // De-serialize SOP instance UID list
  std::vector<std::string> referencedSopInstanceUids;
  this->DeserializeUIDList(referencedInstanceUIDsAttribute, referencedSopInstanceUids);

  // Find subject hierarchy nodes by SOP instance UIDs
  std::vector<std::string>::iterator uidIt;
  for (uidIt = referencedSopInstanceUids.begin(); uidIt != referencedSopInstanceUids.end(); ++uidIt)
    {
    // Find first referenced node in whole scene
    if (referencedNodes.empty())
      {
      vtkMRMLSubjectHierarchyLegacyNode* node = vtkMRMLSubjectHierarchyLegacyNode::GetSubjectHierarchyLegacyNodeByUIDList(
        scene, vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName(), (*uidIt).c_str() );
      if (node)
        {
        referencedNodes.push_back(node);
        }
      }
    else
      {
      // If we found a referenced node, check the other instances in those nodes first to save time
      bool foundUidInFoundReferencedNodes = false;
      std::vector<vtkMRMLSubjectHierarchyLegacyNode*>::iterator nodeIt;
      for (nodeIt = referencedNodes.begin(); nodeIt != referencedNodes.end(); ++nodeIt)
        {
        // Get instance UIDs of the referenced node
        std::string uids = (*nodeIt)->GetUID( vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName() );
        if (uids.find(*uidIt) != std::string::npos)
          {
          // If we found the UID in the already found referenced nodes, then we don't need to do anything
          foundUidInFoundReferencedNodes = true;
          break;
          }
        }
      // If the referenced SOP instance UID is not contained in the already found referenced nodes, then we look in the scene
      if (!foundUidInFoundReferencedNodes)
        {
        vtkMRMLSubjectHierarchyLegacyNode* node = vtkMRMLSubjectHierarchyLegacyNode::GetSubjectHierarchyLegacyNodeByUIDList(
          scene, vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName(), (*uidIt).c_str() );
        if (node)
          {
          referencedNodes.push_back(node);
          }
        }
      }
    }

  return referencedNodes;
}

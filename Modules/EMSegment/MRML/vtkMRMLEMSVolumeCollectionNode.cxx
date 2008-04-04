#include "vtkMRMLEMSVolumeCollectionNode.h"
#include "vtkSlicerVolumesLogic.h"
#include <sstream>
#include "vtkMRMLScene.h"
#include <algorithm>
#include "vtkMRMLScalarVolumeNode.h"

vtkMRMLEMSVolumeCollectionNode* 
vtkMRMLEMSVolumeCollectionNode::
New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSVolumeCollectionNode");
  if(ret)
    {
    return (vtkMRMLEMSVolumeCollectionNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSVolumeCollectionNode;
}

vtkMRMLNode* 
vtkMRMLEMSVolumeCollectionNode::
CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSVolumeCollectionNode");
  if(ret)
    {
    return (vtkMRMLEMSVolumeCollectionNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSVolumeCollectionNode;
}

vtkMRMLEMSVolumeCollectionNode::vtkMRMLEMSVolumeCollectionNode()
{
  // nothing to do here
}

vtkMRMLEMSVolumeCollectionNode::~vtkMRMLEMSVolumeCollectionNode()
{
  // nothing to do here
}

void vtkMRMLEMSVolumeCollectionNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << "VolumeNodeIDs=\"";
  for (KeyConstIterator i = this->KeyList.begin(); i != this->KeyList.end(); 
       ++i)
    {
    std::string key   = *i;
    std::string value = this->KeyToVolumeNodeIDMap[*i];

    if (!key.empty() && !value.empty())
      {
      of << "\n";
      of << "Key " << key << " VolumeNodeID " << value;
      }
    }
  of << "\" ";
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSVolumeCollectionNode::
UpdateReferenceID(const char* oldID, const char* newID)
{
  for (KeyIterator i = this->KeyList.begin(); i != this->KeyList.end(); ++i)
    {
    std::string mrmlID = this->KeyToVolumeNodeIDMap[*i];
    if (oldID && newID && mrmlID == vtksys_stl::string(oldID))
      {
      // update volID to name map
      this->VolumeNodeIDToKeyMap.erase(oldID);
      this->VolumeNodeIDToKeyMap[newID] = *i;

      // update name to volID map
      this->KeyToVolumeNodeIDMap[*i] = newID;
      }
    }
}

//-----------------------------------------------------------------------------
void 
vtkMRMLEMSVolumeCollectionNode::
UpdateReferences()
{
  Superclass::UpdateReferences();

  for (KeyIterator i = this->KeyList.begin(); i != this->KeyList.end();)
    {
    std::string mrmlID = this->KeyToVolumeNodeIDMap[*i];
    if (!mrmlID.empty() && 
        this->Scene->GetNodeByID(mrmlID.c_str()) == NULL)
      {
      // remove key/value pair from mappings
      this->VolumeNodeIDToKeyMap.erase(mrmlID);
      this->KeyToVolumeNodeIDMap.erase(*i);

      // remove key/value pair from list
      this->KeyList.erase(i++);
      }
    else
      {
      ++i;
      }
    }
}

void vtkMRMLEMSVolumeCollectionNode::ReadXMLAttributes(const char** attrs)
{
  Superclass::ReadXMLAttributes(attrs);

  const char* key;
  const char* val;
  while (*attrs != NULL)
    {
    key = *attrs++;
    val = *attrs++;
    
    if (!strcmp(key, "VolumeNodeIDs"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      vtksys_stl::string k1;
      vtksys_stl::string k2;
      vtksys_stl::string v1;
      vtksys_stl::string v2;
      
      while (ss >> k1 && ss >> v1 && ss >> k2 && ss >> v2)
        {
        // moving to AddVolume
        //this->Scene->AddReferencedNodeID(v2.c_str(), this);
        this->AddVolume(v1.c_str(), v2.c_str());
        }
      }
    }
}

void vtkMRMLEMSVolumeCollectionNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
  vtkMRMLEMSVolumeCollectionNode* node = 
    (vtkMRMLEMSVolumeCollectionNode*) rhs;

  this->KeyToVolumeNodeIDMap  = node->KeyToVolumeNodeIDMap;
  this->VolumeNodeIDToKeyMap  = node->VolumeNodeIDToKeyMap;
  this->KeyList               = node->KeyList;
}

void vtkMRMLEMSVolumeCollectionNode::CloneVolumes(const vtkMRMLNode *rhs)
{
  vtkMRMLEMSVolumeCollectionNode* node = 
    (vtkMRMLEMSVolumeCollectionNode*) rhs;

  this->KeyToVolumeNodeIDMap  = node->KeyToVolumeNodeIDMap;
  this->VolumeNodeIDToKeyMap  = node->VolumeNodeIDToKeyMap;
  this->KeyList               = node->KeyList;

  // clone each image
  vtkSlicerVolumesLogic* volumeLogic = vtkSlicerVolumesLogic::New();
  for (int i = 0; i < node->GetNumberOfVolumes(); ++i)
  {    
    vtkMRMLScalarVolumeNode* clonedVolume = 
      volumeLogic->CloneVolume(this->GetScene(),
                               this->GetNthVolumeNode(i),
                               this->GetNthVolumeNode(i)->GetName());
    this->SetNthVolumeNodeID(i, clonedVolume->GetID());
  }
  volumeLogic->Delete();
}

void vtkMRMLEMSVolumeCollectionNode::PrintSelf(ostream& os, 
                                               vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  os << indent << "VolumeNodeIDs: " << "\n";
  for (KeyConstIterator i = this->KeyList.begin(); i != this->KeyList.end(); 
       ++i)
    {
    std::string mrmlID = this->KeyToVolumeNodeIDMap[*i];
    os << "Key=" << *i << " VolumeNodeID=" << mrmlID << "\n";
    }
}

void
vtkMRMLEMSVolumeCollectionNode::
AddVolume(const char* key, const char* volumeNodeID)
{
  // enforce unique keys
  this->KeyList.remove(key);
  this->KeyList.push_back(key);
  this->KeyToVolumeNodeIDMap[key] = volumeNodeID;
  this->VolumeNodeIDToKeyMap[volumeNodeID] = key;
  this->Scene->AddReferencedNodeID(volumeNodeID, this);
}

void
vtkMRMLEMSVolumeCollectionNode::
SetNthVolumeNodeID(int n, const char* volumeNodeID)
{
  KeyIterator i = this->KeyList.begin();
  vtksys_stl::advance(i, n);
  this->KeyToVolumeNodeIDMap[*i] = volumeNodeID;
  this->Scene->AddReferencedNodeID(volumeNodeID, this);
}

int
vtkMRMLEMSVolumeCollectionNode::
GetNumberOfVolumes() const
{
  return this->KeyList.size();
}

void
vtkMRMLEMSVolumeCollectionNode::
RemoveAllVolumes()
{
  this->KeyList.clear();
  this->KeyToVolumeNodeIDMap.clear();
  this->VolumeNodeIDToKeyMap.clear();
}

void
vtkMRMLEMSVolumeCollectionNode::
RemoveVolumeByKey(const char* key)
{
  std::string mrmlID = this->KeyToVolumeNodeIDMap[key];
  if (!mrmlID.empty())
    {
    this->VolumeNodeIDToKeyMap.erase(mrmlID);
    this->KeyToVolumeNodeIDMap.erase(key);
    this->KeyList.remove(key);
    }
}

void
vtkMRMLEMSVolumeCollectionNode::
RemoveVolumeByNodeID(const char* nodeID)
{
  std::string key = this->VolumeNodeIDToKeyMap[nodeID];
  if (!key.empty())
    {
    this->VolumeNodeIDToKeyMap.erase(nodeID);
    this->KeyToVolumeNodeIDMap.erase(key);
    this->KeyList.remove(key);
    }
}

void
vtkMRMLEMSVolumeCollectionNode::
RemoveNthVolume(int n)
{
  KeyIterator i = this->KeyList.begin();
  vtksys_stl::advance(i, n);
  std::string key    = *i;
  std::string mrmlID = this->KeyToVolumeNodeIDMap[key];

  this->KeyToVolumeNodeIDMap.erase(key);
  this->VolumeNodeIDToKeyMap.erase(mrmlID);
  this->KeyList.remove(key);
}

const char*
vtkMRMLEMSVolumeCollectionNode::
GetVolumeNodeIDByKey(const char* key) const
{
  return this->KeyToVolumeNodeIDMap[key].c_str();
}

const char*
vtkMRMLEMSVolumeCollectionNode::
GetKeyByVolumeNodeID(const char* nodeID) const
{
  return this->VolumeNodeIDToKeyMap[nodeID].c_str();
}

int
vtkMRMLEMSVolumeCollectionNode::
GetIndexByKey(const char* key) const
{
  KeyIterator location = vtkstd::find(this->KeyList.begin(),
                                      this->KeyList.end(), key);
  if (location == this->KeyList.end())
    {
    return -1;
    }
  else
    {
    return vtkstd::distance(this->KeyList.begin(), location);
    }
}

int
vtkMRMLEMSVolumeCollectionNode::
GetIndexByVolumeNodeID(const char* nodeID) const
{
  return this->GetIndexByKey(this->VolumeNodeIDToKeyMap[nodeID].c_str());
}

const char*
vtkMRMLEMSVolumeCollectionNode::
GetNthVolumeNodeID(int n) const
{
  KeyIterator i = this->KeyList.begin();
  vtksys_stl::advance(i, n);
  return this->KeyToVolumeNodeIDMap[*i].c_str();
}

const char*
vtkMRMLEMSVolumeCollectionNode::
GetNthKey(int n) const
{
  KeyIterator i = this->KeyList.begin();
  vtksys_stl::advance(i, n);
  return (*i).c_str();
}

vtkMRMLVolumeNode*
vtkMRMLEMSVolumeCollectionNode::
GetNthVolumeNode(int n) const
{
  vtkMRMLVolumeNode* node = NULL;
  if (const_cast<vtkMRMLEMSVolumeCollectionNode*>(this)->GetScene() && 
      this->GetNthVolumeNodeID(n))
    {
    vtkMRMLNode* snode = const_cast<vtkMRMLEMSVolumeCollectionNode*>(this)->
      GetScene()->GetNodeByID(this->GetNthVolumeNodeID(n));
    node = vtkMRMLVolumeNode::SafeDownCast(snode);
    }
  return node;  
}

void
vtkMRMLEMSVolumeCollectionNode::
MoveNthVolume(int n, int toIndex)
{  
  if (toIndex == n)
    {
    return;
    }

  KeyIterator start    = this->KeyList.begin();
  KeyIterator iterFrom = start;
  vtksys_stl::advance(iterFrom, n);
  KeyIterator iterTo   = start;
  vtksys_stl::advance(iterTo, toIndex);

  vtkstd::string keyFrom = *iterFrom;

  if (iterFrom != this->KeyList.end() && iterTo != this->KeyList.end())
    {
    KeyList.erase(iterFrom);
    KeyList.insert(iterTo, keyFrom);
    }
}

#include "vtkMRMLEMSCollectionNode.h"
#include <sstream>
#include "vtkMRMLScene.h"
#include <algorithm>
#include <vtksys/stl/string>
#include <vtkObjectFactory.h>

vtkMRMLEMSCollectionNode* 
vtkMRMLEMSCollectionNode::
New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSCollectionNode");
  if(ret)
    {
    return (vtkMRMLEMSCollectionNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSCollectionNode;
}

vtkMRMLNode* 
vtkMRMLEMSCollectionNode::
CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSCollectionNode");
  if(ret)
    {
    return (vtkMRMLEMSCollectionNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSCollectionNode;
}

vtkMRMLEMSCollectionNode::vtkMRMLEMSCollectionNode()
{
  // nothing to do here
}

vtkMRMLEMSCollectionNode::~vtkMRMLEMSCollectionNode()
{
  // nothing to do here
}

void vtkMRMLEMSCollectionNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << " NodeIDs=\"";
  for (KeyConstIterator i = this->KeyList.begin(); i != this->KeyList.end(); 
       ++i)
    {
    std::string key   = *i;
    std::string value = this->KeyToNodeIDMap[*i];

    if (!key.empty() && !value.empty())
      {
      of << "\n";
      of << "Key " << key << " NodeID " << value;
      }
    }
  of << "\" ";
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSCollectionNode::
UpdateReferenceID(const char* oldID, const char* newID)
{
  for (KeyIterator i = this->KeyList.begin(); i != this->KeyList.end(); ++i)
    {
    std::string mrmlID = this->KeyToNodeIDMap[*i];
    if (oldID && newID && mrmlID == vtksys_stl::string(oldID))
      {
      // update volID to name map
      this->NodeIDToKeyMap.erase(oldID);
      this->NodeIDToKeyMap[newID] = *i;

      // update name to volID map
      this->KeyToNodeIDMap[*i] = newID;
      }
    }
}

//-----------------------------------------------------------------------------
void 
vtkMRMLEMSCollectionNode::
UpdateReferences()
{
  Superclass::UpdateReferences();

  for (KeyIterator i = this->KeyList.begin(); i != this->KeyList.end();)
    {
    std::string mrmlID = this->KeyToNodeIDMap[*i];
    if (!mrmlID.empty() && 
        this->Scene->GetNodeByID(mrmlID.c_str()) == NULL)
      {
      // remove key/value pair from mappings
      this->NodeIDToKeyMap.erase(mrmlID);
      this->KeyToNodeIDMap.erase(*i);

      // remove key/value pair from list
      this->KeyList.erase(i++);
      }
    else
      {
      ++i;
      }
    }
}

void vtkMRMLEMSCollectionNode::ReadXMLAttributes(const char** attrs)
{
  Superclass::ReadXMLAttributes(attrs);

  const char* key;
  const char* val;
  while (*attrs != NULL)
    {
    key = *attrs++;
    val = *attrs++;
    
    if (!strcmp(key, "NodeIDs"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      vtksys_stl::string k1;
      vtksys_stl::string k2;
      vtksys_stl::string v1;
      vtksys_stl::string v2;
      
      while (ss >> k1 && ss >> v1 && ss >> k2 && ss >> v2)
        {
        // moving to Add Node
        //this->Scene->AddReferencedNodeID(v2.c_str(), this);
        this->AddNode(v1.c_str(), v2.c_str());
        }
      }
    }
}

void vtkMRMLEMSCollectionNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
  vtkMRMLEMSCollectionNode* node = 
    (vtkMRMLEMSCollectionNode*) rhs;

  this->KeyToNodeIDMap  = node->KeyToNodeIDMap;
  this->NodeIDToKeyMap  = node->NodeIDToKeyMap;
  this->KeyList               = node->KeyList;
}

void vtkMRMLEMSCollectionNode::PrintSelf(ostream& os, 
                                               vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  os << indent << "NodeIDs: " << "\n";
  for (KeyConstIterator i = this->KeyList.begin(); i != this->KeyList.end(); 
       ++i)
    {
    std::string mrmlID = this->KeyToNodeIDMap[*i];
    os << "Key=" << *i << " NodeID=" << mrmlID << "\n";
    }
}

int
vtkMRMLEMSCollectionNode::
AddNode(const char* key, const char* volumeNodeID)
{
  // check if already in list - if so do not do anyything
  if ((this->GetIndexByKey(key) > -1) && ( !strcmp(this->GetNodeIDByKey(key),volumeNodeID)))
    {
      return 0;
    }

    // enforce unique keys
    this->KeyList.remove(key);
    this->KeyList.push_back(key);
    this->KeyToNodeIDMap[key] = volumeNodeID;
    this->NodeIDToKeyMap[volumeNodeID] = key;
    // cout << "this->Scene->AddReferencedNodeID " << volumeNodeID << " " << this << endl;
    this->Scene->AddReferencedNodeID(volumeNodeID, this);
    return 1;
}

void
vtkMRMLEMSCollectionNode::
SetNthNodeID(int n, const char* volumeNodeID)
{
  KeyIterator i = this->KeyList.begin();
  vtksys_stl::advance(i, n);
  this->KeyToNodeIDMap[*i] = volumeNodeID;
  this->Scene->AddReferencedNodeID(volumeNodeID, this);
  // cout << "setnth this->Scene->AddReferencedNodeID " << volumeNodeID << " " << this << endl;
}

int
vtkMRMLEMSCollectionNode::
GetNumberOfNodes() const
{
  return this->KeyList.size();
}

void
vtkMRMLEMSCollectionNode::
RemoveAllNodes()
{
  // if you do not do that then even after one clears all the volumes the  node still references to the volume node according to the scene 
  for (KeyConstIterator i = this->KeyList.begin(); i != this->KeyList.end(); ++i)
    {
    std::string volumeNodeID = this->KeyToNodeIDMap[*i];
    this->Scene->RemoveReferencedNodeID(volumeNodeID.c_str(), this);
    // cout << "this->Scene->RemoveReferencedNodeID " << volumeNodeID << " " << this << endl;
    }
 
  // vtkCollection* blub = this->Scene->GetReferencedNodes(this);
  // vtkIndent t;
  // blub->PrintSelf(cout,t);

  this->KeyList.clear();
  this->KeyToNodeIDMap.clear();
  this->NodeIDToKeyMap.clear();
}

void
vtkMRMLEMSCollectionNode::
RemoveNodeByKey(const char* key)
{
  std::string mrmlID = this->KeyToNodeIDMap[key];
  if (!mrmlID.empty())
    {
    this->Scene->RemoveReferencedNodeID(mrmlID.c_str(), this);
    this->NodeIDToKeyMap.erase(mrmlID);
    this->KeyToNodeIDMap.erase(key);
    this->KeyList.remove(key);
    }
}

void
vtkMRMLEMSCollectionNode::
RemoveNodeByNodeID(const char* nodeID)
{
  std::string key = this->NodeIDToKeyMap[nodeID];
  if (!key.empty())
    {
    this->Scene->RemoveReferencedNodeID(nodeID, this);
    this->NodeIDToKeyMap.erase(nodeID);
    this->KeyToNodeIDMap.erase(key);
    this->KeyList.remove(key);
    }
}

void
vtkMRMLEMSCollectionNode::
RemoveNthNode(int n)
{
  KeyIterator i = this->KeyList.begin();
  vtksys_stl::advance(i, n);
  std::string key    = *i;
  std::string mrmlID = this->KeyToNodeIDMap[key];

  this->Scene->RemoveReferencedNodeID(mrmlID.c_str(), this);
  this->KeyToNodeIDMap.erase(key);
  this->NodeIDToKeyMap.erase(mrmlID);
  this->KeyList.remove(key);
}

const char*
vtkMRMLEMSCollectionNode::
GetNodeIDByKey(const char* key) const
{
  return this->KeyToNodeIDMap[key].c_str();
}

const char*
vtkMRMLEMSCollectionNode::
GetKeyByNodeID(const char* nodeID) const
{
  return this->NodeIDToKeyMap[nodeID].c_str();
}

int
vtkMRMLEMSCollectionNode::
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
vtkMRMLEMSCollectionNode::
GetIndexByNodeID(const char* nodeID) const
{
  return this->GetIndexByKey(this->NodeIDToKeyMap[nodeID].c_str());
}

const char*
vtkMRMLEMSCollectionNode::
GetNthNodeID(int n) const
{
  if (this->GetNumberOfNodes() <= n) 
    {
      return NULL;
    }
  KeyIterator i = this->KeyList.begin();
  vtksys_stl::advance(i, n);
  return this->KeyToNodeIDMap[*i].c_str();
}

const char*
vtkMRMLEMSCollectionNode::
GetNthKey(int n) const
{
  KeyIterator i = this->KeyList.begin();
  vtksys_stl::advance(i, n);
  return (*i).c_str();
}

vtkMRMLNode*
vtkMRMLEMSCollectionNode::
GetNthNode(int n) const
{
  if (const_cast<vtkMRMLEMSCollectionNode*>(this)->GetScene() && 
      this->GetNthNodeID(n))
    {
    return  const_cast<vtkMRMLEMSCollectionNode*>(this)->GetScene()->GetNodeByID(this->GetNthNodeID(n));
    }
  return NULL;
}

void
vtkMRMLEMSCollectionNode::
MoveNthNode(int n, int toIndex)
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

#include "vtkMRMLEMSTreeNode.h"
#include "vtkMRMLScene.h"
#include <sstream>

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeNode* 
vtkMRMLEMSTreeNode::
New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSTreeNode");
  if(ret)
    {
    return (vtkMRMLEMSTreeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSTreeNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* 
vtkMRMLEMSTreeNode::
CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSTreeNode");
  if(ret)
    {
    return (vtkMRMLEMSTreeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSTreeNode;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeNode::vtkMRMLEMSTreeNode()
{
  this->ParentNodeID           = NULL;
  this->TreeParametersNodeID   = NULL;
  this->Label                  = NULL;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeNode::~vtkMRMLEMSTreeNode()
{
  this->SetParentNodeID(NULL);
  this->SetTreeParametersNodeID(NULL);
  this->SetLabel(NULL);
  // !!! ??? child nodes? unreference???
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTreeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << "ParentNodeID=\"" 
     << (this->ParentNodeID ? this->ParentNodeID : "NULL") << "\" ";
  of << indent << "TreeParametersNodeID=\"" 
     << (this->TreeParametersNodeID ? this->TreeParametersNodeID : "NULL")
     << "\" ";
  of << indent << "Label=\"" 
     << (this->Label ? this->Label : "NULL")
     << "\" ";
  of << indent << "ChildNodeIDs=\"";
  for (unsigned int i = 0; i < this->ChildNodeIDs.size(); ++i)
    {
    of << this->ChildNodeIDs[i];
    if (i < this->ChildNodeIDs.size() - 1)
      {
      of << " ";
      }
    }
  of << "\" ";
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeNode::
UpdateReferenceID(const char* oldID, const char* newID)
{
  if (this->ParentNodeID && !strcmp(oldID, this->ParentNodeID))
    {
    this->SetParentNodeID(newID);
    }
  if (this->TreeParametersNodeID && !strcmp(oldID, this->TreeParametersNodeID))
    {
    this->SetTreeParametersNodeID(newID);
    }
  for (unsigned int i = 0; i < this->ChildNodeIDs.size(); ++i)
    {
    if (oldID && newID && (this->ChildNodeIDs[i] == std::string(oldID)))
      {
      this->ChildNodeIDs[i] = newID;
      }
    }
}

//-----------------------------------------------------------------------------
void 
vtkMRMLEMSTreeNode::
UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->ParentNodeID != NULL && 
      this->Scene->GetNodeByID(this->ParentNodeID) == NULL)
    {
    this->SetParentNodeID(NULL);
    }
  if (this->TreeParametersNodeID != NULL && 
      this->Scene->GetNodeByID(this->TreeParametersNodeID) == NULL)
    {
    this->SetTreeParametersNodeID(NULL);
    }

  std::vector<std::string> childIDsCopy(this->ChildNodeIDs);
  this->ChildNodeIDs.clear();
  for (unsigned int i = 0; i < childIDsCopy.size(); ++i)
    {
    if (childIDsCopy[i].empty() ||
        this->Scene->GetNodeByID(childIDsCopy[i].c_str()) != NULL)
      {
      this->ChildNodeIDs.push_back(childIDsCopy[i]);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTreeNode::ReadXMLAttributes(const char** attrs)
{
  Superclass::ReadXMLAttributes(attrs);

  //
  // we assume an even number of elements
  //
  const char* key;
  const char* val;
  while (*attrs != NULL)
    {
    key = *attrs++;
    val = *attrs++;
    
    if (!strcmp(key, "ParentNodeID"))
      {
      this->SetParentNodeID(val);
      }
    else if (!strcmp(key, "TreeParametersNodeID"))
      {
      this->SetTreeParametersNodeID(val);
      }
    else if (!strcmp(key, "Label"))
      {
      this->SetLabel(val);
      }

    else if (!strcmp(key, "ChildNodeIDs"))
      {
      vtksys_stl::stringstream ss;
      ss << val;
      vtksys_stl::string currentID;
      while (ss >> currentID)
        {
        this->AddChildNode(currentID.c_str());
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTreeNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
  vtkMRMLEMSTreeNode* node = (vtkMRMLEMSTreeNode*) rhs;

  this->SetParentNodeID(node->ParentNodeID);
  this->SetTreeParametersNodeID(node->TreeParametersNodeID);
  this->SetLabel(node->Label);
  this->ChildNodeIDs = node->ChildNodeIDs;

  // !!! is it correct to add references here???
  for (unsigned int i = 0; i < this->ChildNodeIDs.size(); ++i)
    {
    this->Scene->AddReferencedNodeID(this->ChildNodeIDs[i].c_str(), this);
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTreeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  
  os << indent << "ParentNodeID: " <<
    (this->ParentNodeID ? this->ParentNodeID : "(none)") << "\n";
  
  os << indent << "TreeParametersNodeID: " <<
    (this->TreeParametersNodeID ? this->TreeParametersNodeID : "(none)")
     << "\n";

  os << indent << "Label: " <<
    (this->Label ? this->Label : "(none)")
     << "\n";

  os << indent << "ChildNodeIDs: " << "\n";
  for (unsigned int i = 0; i < this->ChildNodeIDs.size(); ++i)
    {
    std::string mrmlID = this->ChildNodeIDs[i];
    os << indent << "  " << mrmlID << "\n";
    }
}

//-----------------------------------------------------------------------------
int
vtkMRMLEMSTreeNode::
GetNumberOfChildNodes()
{
  return this->ChildNodeIDs.size();
}

//-----------------------------------------------------------------------------
const char*
vtkMRMLEMSTreeNode::
GetNthChildNodeID(int n)
{
  if (n >= 0 && n < static_cast<int>(this->ChildNodeIDs.size()))
    {
    return this->ChildNodeIDs[n].c_str();
    }
  else
    {
    return NULL;
    }
}

//-----------------------------------------------------------------------------
int
vtkMRMLEMSTreeNode::
GetChildIndexByMRMLID(const char* childID)
{
  std::string searchID(childID);
  for (int i = 0; i < static_cast<int>(this->ChildNodeIDs.size()); ++i)
    {
    if (this->ChildNodeIDs[i] == searchID)
      {
      return i;
      }
    }
  // didn't find it
  return -1;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeNode*
vtkMRMLEMSTreeNode::
GetParentNode()
{
  vtkMRMLEMSTreeNode* node = NULL;
  if (this->GetScene() && this->GetParentNodeID())
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->ParentNodeID);
    node = vtkMRMLEMSTreeNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeParametersNode*
vtkMRMLEMSTreeNode::
GetParametersNode()
{
  vtkMRMLEMSTreeParametersNode* node = NULL;
  if (this->GetScene() && this->GetTreeParametersNodeID())
    {
    vtkMRMLNode* snode = this->GetScene()->
      GetNodeByID(this->TreeParametersNodeID);
    node = vtkMRMLEMSTreeParametersNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeNode*
vtkMRMLEMSTreeNode::
GetNthChildNode(int n)
{
  vtkMRMLEMSTreeNode* node = NULL;
  if (this->GetScene() && this->GetNthChildNodeID(n))
    {
    vtkMRMLNode* snode = this->GetScene()->
      GetNodeByID(this->GetNthChildNodeID(n));
    node = vtkMRMLEMSTreeNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeNode::
MoveNthChildNode(int fromIndex, int toIndex)
{
  std::string movingParam = this->ChildNodeIDs[fromIndex];
  this->ChildNodeIDs.erase(this->ChildNodeIDs.begin() + 
                                  fromIndex);
  this->ChildNodeIDs.insert(this->ChildNodeIDs.begin() + 
                                   toIndex, movingParam);

  if (this->GetParametersNode() != NULL)
    {
    this->GetParametersNode()->MoveNthChildNode(fromIndex, toIndex);
    }
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeNode::
RemoveNthChildNode(int n)
{
  std::string removedChildNodeID = this->ChildNodeIDs[n];
  this->ChildNodeIDs.erase(ChildNodeIDs.begin() + n);
  // !!! how to remove reference dependency?

  if (this->GetParametersNode() != NULL)
    {
    this->GetParametersNode()->RemoveNthChildNode(n);
    }
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeNode::
AddChildNode(const char* mrmlID)
{
  this->ChildNodeIDs.push_back(mrmlID);
  this->Scene->AddReferencedNodeID(mrmlID, this);  

  if (this->GetParametersNode() != NULL)
    {
    this->GetParametersNode()->AddChildNode(mrmlID);
    }
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTreeNode::
SetNthChildNode(int n, const char* mrmlID)
{
  this->ChildNodeIDs[n] = mrmlID;
  this->Scene->AddReferencedNodeID(mrmlID, this);  
}


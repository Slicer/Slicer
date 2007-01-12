#include "vtkMRMLEMSTreeNode.h"
#include "vtkMRMLScene.h"

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
}

//-----------------------------------------------------------------------------
int
vtkMRMLEMSTreeNode::
GetNumberOfChildNodes()
{
  int numChildren = 0;
  int numTreeNodes = this->GetScene()->
    GetNumberOfNodesByClass("vtkMRMLEMSTreeNode");

  for (int i = 0; i < numTreeNodes; ++i)
  {
    vtkMRMLNode* node = this->GetScene()->
      GetNthNodeByClass(i, "vtkMRMLEMSTreeNode");

    vtkMRMLEMSTreeNode* treeNode = vtkMRMLEMSTreeNode::SafeDownCast(node);
    
    if (treeNode->GetParentNodeID() != NULL &&
        vtksys_stl::string(this->GetID()) == 
        vtksys_stl::string(treeNode->GetParentNodeID()))
    {
      ++numChildren;
    }
  }

  return numChildren;
}

//-----------------------------------------------------------------------------
char*
vtkMRMLEMSTreeNode::
GetNthChildNodeID(int n)
{
  int numChildren = 0;
  int numTreeNodes = this->GetScene()->
    GetNumberOfNodesByClass("vtkMRMLEMSTreeNode");
  for (int i = 0; i < numTreeNodes; ++i)
  {
    vtkMRMLNode* node = this->GetScene()->
      GetNthNodeByClass(i,"vtkMRMLEMSTreeNode");
    vtkMRMLEMSTreeNode* treeNode = vtkMRMLEMSTreeNode::SafeDownCast(node);

    if (treeNode->GetParentNodeID() != NULL &&
        vtksys_stl::string(this->GetID()) == 
        vtksys_stl::string(treeNode->GetParentNodeID()))
    {
      if (numChildren++ == n)
      {
        return treeNode->GetID();
      }
    }
  }

  // didn't find it
  return NULL;
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

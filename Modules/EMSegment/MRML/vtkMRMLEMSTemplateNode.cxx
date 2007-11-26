#include "vtkMRMLEMSTemplateNode.h"
#include "vtkMRMLScene.h"

//-----------------------------------------------------------------------------
vtkMRMLEMSTemplateNode* 
vtkMRMLEMSTemplateNode::
New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSTemplateNode");
  if(ret)
    {
    return (vtkMRMLEMSTemplateNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSTemplateNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* 
vtkMRMLEMSTemplateNode::
CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSTemplateNode");
  if(ret)
    {
    return (vtkMRMLEMSTemplateNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSTemplateNode;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTemplateNode::vtkMRMLEMSTemplateNode()
{
  this->TreeNodeID           = NULL;
  this->GlobalParametersNodeID   = NULL;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTemplateNode::~vtkMRMLEMSTemplateNode()
{
  this->SetTreeNodeID(NULL);
  this->SetGlobalParametersNodeID(NULL);
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTemplateNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << "TreeNodeID=\"" 
     << (this->TreeNodeID ? this->TreeNodeID : "NULL") << "\" ";
  of << indent << "GlobalParametersNodeID=\"" 
     << (this->GlobalParametersNodeID ? this->GlobalParametersNodeID : "NULL" )
     << "\" ";
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSTemplateNode::
UpdateReferenceID(const char* oldID, const char* newID)
{
  if (this->TreeNodeID && !strcmp(oldID, this->TreeNodeID))
    {
    this->SetTreeNodeID(newID);
    }
  if (this->GlobalParametersNodeID && 
      !strcmp(oldID, this->GlobalParametersNodeID))
    {
    this->SetGlobalParametersNodeID(newID);
    }
}

//-----------------------------------------------------------------------------
void 
vtkMRMLEMSTemplateNode::
UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->GlobalParametersNodeID != NULL && 
      this->Scene->GetNodeByID(this->GlobalParametersNodeID) == NULL)
    {
    this->SetGlobalParametersNodeID(NULL);
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTemplateNode::ReadXMLAttributes(const char** attrs)
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
    
    if (!strcmp(key, "TreeNodeID"))
      {
      this->SetTreeNodeID(val);
      //this->Scene->AddReferencedNodeID(this->TreeNodeID, this);
      }
    else if (!strcmp(key, "GlobalParametersNodeID"))
      {
      this->SetGlobalParametersNodeID(val);
      //this->Scene->AddReferencedNodeID(this->GlobalParametersNodeID, this);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTemplateNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
  vtkMRMLEMSTemplateNode* node = (vtkMRMLEMSTemplateNode*) rhs;

  this->SetTreeNodeID(node->TreeNodeID);
  this->SetGlobalParametersNodeID(node->GlobalParametersNodeID);
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTemplateNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  
  os << indent << "TreeNodeID: " <<
    (this->TreeNodeID ? this->TreeNodeID : "(none)") << "\n";
  
  os << indent << "GlobalParametersNodeID: " <<
    (this->GlobalParametersNodeID ? this->GlobalParametersNodeID : "(none)")
     << "\n";
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTreeNode*
vtkMRMLEMSTemplateNode::
GetTreeNode()
{
  vtkMRMLEMSTreeNode* node = NULL;
  if (this->GetScene() && this->GetTreeNodeID() )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->TreeNodeID);
    node = vtkMRMLEMSTreeNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSGlobalParametersNode*
vtkMRMLEMSTemplateNode::
GetGlobalParametersNode()
{
  vtkMRMLEMSGlobalParametersNode* node = NULL;
  if (this->GetScene() && this->GetGlobalParametersNodeID() )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->GlobalParametersNodeID);
    node = vtkMRMLEMSGlobalParametersNode::SafeDownCast(snode);
    }
  return node;
}

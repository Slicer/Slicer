#include "vtkMRMLEMSWorkingDataNode.h"
#include <sstream>
#include "vtkMRMLScene.h"
#include "vtkMRMLEMSTargetNode.h"
#include "vtkMRMLEMSAtlasNode.h"

//-----------------------------------------------------------------------------
vtkMRMLEMSWorkingDataNode* 
vtkMRMLEMSWorkingDataNode::
New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSWorkingDataNode");
  if(ret)
    {
    return (vtkMRMLEMSWorkingDataNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSWorkingDataNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* 
vtkMRMLEMSWorkingDataNode::
CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSWorkingDataNode");
  if(ret)
    {
    return (vtkMRMLEMSWorkingDataNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSWorkingDataNode;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSWorkingDataNode::vtkMRMLEMSWorkingDataNode()
{
  this->InputTargetNodeID                = NULL;
  this->NormalizedTargetNodeID           = NULL;
  this->AlignedTargetNodeID              = NULL;
  this->InputAtlasNodeID                 = NULL;
  this->AlignedAtlasNodeID               = NULL;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSWorkingDataNode::~vtkMRMLEMSWorkingDataNode()
{
  this->SetInputTargetNodeID(NULL);
  this->SetNormalizedTargetNodeID(NULL);
  this->SetAlignedTargetNodeID(NULL);
  this->SetInputAtlasNodeID(NULL);
  this->SetAlignedAtlasNodeID(NULL);
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSWorkingDataNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

 of << indent << "InputTargetNodeID=\"" 
     << (this->InputTargetNodeID ? this->InputTargetNodeID : "NULL")
     << "\" ";
  of << indent << "NormalizedTargetNodeID=\"" 
     << (this->NormalizedTargetNodeID ? this->NormalizedTargetNodeID : "NULL")
     << "\" ";
  of << indent << "AlignedTargetNodeID=\"" 
     << (this->AlignedTargetNodeID ? this->AlignedTargetNodeID : "NULL")
     << "\" ";
 of << indent << "InputAtlasNodeID=\"" 
     << (this->InputAtlasNodeID ? this->InputAtlasNodeID : "NULL")
     << "\" ";
  of << indent << "AlignedAtlasNodeID=\"" 
     << (this->AlignedAtlasNodeID ? this->AlignedAtlasNodeID : "NULL")
     << "\" ";
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSWorkingDataNode::
UpdateReferenceID(const char* oldID, const char* newID)
{
  if (this->InputTargetNodeID && 
      !strcmp(oldID, this->InputTargetNodeID))
    {
    this->SetInputTargetNodeID(newID);
    }
  if (this->NormalizedTargetNodeID && 
      !strcmp(oldID, this->NormalizedTargetNodeID))
    {
    this->SetNormalizedTargetNodeID(newID);
    }
  if (this->AlignedTargetNodeID && 
      !strcmp(oldID, this->AlignedTargetNodeID))
    {
    this->SetAlignedTargetNodeID(newID);
    }
  if (this->InputAtlasNodeID && 
      !strcmp(oldID, this->InputAtlasNodeID))
    {
    this->SetInputAtlasNodeID(newID);
    }
  if (this->AlignedAtlasNodeID && 
      !strcmp(oldID, this->AlignedAtlasNodeID))
    {
    this->SetAlignedAtlasNodeID(newID);
    }
}

//-----------------------------------------------------------------------------
void 
vtkMRMLEMSWorkingDataNode::
UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->InputTargetNodeID != NULL && 
      this->Scene->GetNodeByID(this->InputTargetNodeID) == NULL)
    {
    this->SetInputTargetNodeID(NULL);
    }
  if (this->NormalizedTargetNodeID != NULL && 
      this->Scene->GetNodeByID(this->NormalizedTargetNodeID) == NULL)
    {
    this->SetNormalizedTargetNodeID(NULL);
    }
  if (this->AlignedTargetNodeID != NULL && 
      this->Scene->GetNodeByID(this->AlignedTargetNodeID) == NULL)
    {
    this->SetAlignedTargetNodeID(NULL);
    }
  if (this->InputAtlasNodeID != NULL && 
      this->Scene->GetNodeByID(this->InputAtlasNodeID) == NULL)
    {
    this->SetInputAtlasNodeID(NULL);
    }
  if (this->AlignedAtlasNodeID != NULL && 
      this->Scene->GetNodeByID(this->AlignedAtlasNodeID) == NULL)
    {
    this->SetAlignedAtlasNodeID(NULL);
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSWorkingDataNode::ReadXMLAttributes(const char** attrs)
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
    
    if (!strcmp(key, "InputTargetNodeID"))
      {
      this->SetInputTargetNodeID(val);
      //this->Scene->AddReferencedNodeID(this->InputTargetNodeID, this);
      }
    else if (!strcmp(key, "NormalizedTargetNodeID"))
      {
      this->SetNormalizedTargetNodeID(val);
      //this->Scene->AddReferencedNodeID(this->NormalizedTargetNodeID, this);
      }
    else if (!strcmp(key, "AlignedTargetNodeID"))
      {
      this->SetAlignedTargetNodeID(val);
      //this->Scene->AddReferencedNodeID(this->AlignedTargetNodeID, this);
      }
    else if (!strcmp(key, "InputAtlasNodeID"))
      {
      this->SetInputAtlasNodeID(val);
      //this->Scene->AddReferencedNodeID(this->InputAtlasNodeID, this);
      }
    else if (!strcmp(key, "AlignedAtlasNodeID"))
      {
      this->SetAlignedAtlasNodeID(val);
      //this->Scene->AddReferencedNodeID(this->AlignedAtlasNodeID, this);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSWorkingDataNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
  vtkMRMLEMSWorkingDataNode* node = (vtkMRMLEMSWorkingDataNode*) rhs;

  this->SetInputTargetNodeID(node->InputTargetNodeID);
  this->SetNormalizedTargetNodeID(node->NormalizedTargetNodeID);
  this->SetAlignedTargetNodeID(node->AlignedTargetNodeID);
  this->SetInputAtlasNodeID(node->InputAtlasNodeID);
  this->SetAlignedAtlasNodeID(node->AlignedAtlasNodeID);
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSWorkingDataNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  
  os << indent << "InputTargetNodeID: " <<
    (this->InputTargetNodeID ? this->InputTargetNodeID : "(none)") 
     << "\n";
  os << indent << "NormalizedTargetNodeID: " <<
    (this->NormalizedTargetNodeID ? this->NormalizedTargetNodeID : "(none)") 
     << "\n";
  os << indent << "AlignedTargetNodeID: " <<
    (this->AlignedTargetNodeID ? this->AlignedTargetNodeID : "(none)") 
     << "\n";
  os << indent << "InputAtlasNodeID: " <<
    (this->InputAtlasNodeID ? this->InputAtlasNodeID : "(none)") 
     << "\n";
  os << indent << "AlignedAtlasNodeID: " <<
    (this->AlignedAtlasNodeID ? this->AlignedAtlasNodeID : "(none)") 
     << "\n";
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTargetNode*
vtkMRMLEMSWorkingDataNode::
GetInputTargetNode()
{
  vtkMRMLEMSTargetNode* node = NULL;
  if (this->GetScene() && this->InputTargetNodeID)
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->InputTargetNodeID);
    node = vtkMRMLEMSTargetNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTargetNode*
vtkMRMLEMSWorkingDataNode::
GetNormalizedTargetNode()
{
  vtkMRMLEMSTargetNode* node = NULL;
  if (this->GetScene() && this->NormalizedTargetNodeID )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->NormalizedTargetNodeID);
    node = vtkMRMLEMSTargetNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTargetNode*
vtkMRMLEMSWorkingDataNode::
GetAlignedTargetNode()
{
  vtkMRMLEMSTargetNode* node = NULL;
  if (this->GetScene() && this->AlignedTargetNodeID )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->AlignedTargetNodeID);
    node = vtkMRMLEMSTargetNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSAtlasNode*
vtkMRMLEMSWorkingDataNode::
GetInputAtlasNode()
{
  vtkMRMLEMSAtlasNode* node = NULL;
  if (this->GetScene() && this->InputAtlasNodeID )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->InputAtlasNodeID);
    node = vtkMRMLEMSAtlasNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSAtlasNode*
vtkMRMLEMSWorkingDataNode::
GetAlignedAtlasNode()
{
  vtkMRMLEMSAtlasNode* node = NULL;
  if (this->GetScene() && this->AlignedAtlasNodeID )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->AlignedAtlasNodeID);
    node = vtkMRMLEMSAtlasNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTargetNode*
vtkMRMLEMSWorkingDataNode::
GetWorkingTargetNode()
{
  if (this->GetAlignedTargetNode())
    {
    return this->GetAlignedTargetNode();
    }
  if (this->GetNormalizedTargetNode())
    {
    return this->GetNormalizedTargetNode();
    }
  return this->GetInputTargetNode();
}

//-----------------------------------------------------------------------------
vtkMRMLEMSAtlasNode*
vtkMRMLEMSWorkingDataNode::
GetWorkingAtlasNode()
{
  if (this->GetAlignedAtlasNode())
    {
    return this->GetAlignedAtlasNode();
    }
  return this->GetInputAtlasNode();
}

#include "vtkMRMLEMSTemplateNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLEMSAtlasNode.h"
#include "vtkMRMLEMSGlobalParametersNode.h"
#include "vtkMRMLEMSTreeNode.h"
#include <vtkObjectFactory.h>
#include "vtkMRMLEMSWorkingDataNode.h"

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
  this->SpatialAtlasNodeID                 = NULL;
  this->SubParcellationNodeID       = NULL;
  this->EMSWorkingDataNodeID        = NULL;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTemplateNode::~vtkMRMLEMSTemplateNode()
{
  this->SetTreeNodeID(NULL);
  this->SetGlobalParametersNodeID(NULL);
  this->SetSpatialAtlasNodeID(NULL);
  this->SetSubParcellationNodeID(NULL);
  this->SetEMSWorkingDataNodeID(NULL);
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSTemplateNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << " TreeNodeID=\"" 
     << (this->TreeNodeID ? this->TreeNodeID : "NULL") << "\" ";
  of << indent << "GlobalParametersNodeID=\"" 
     << (this->GlobalParametersNodeID ? this->GlobalParametersNodeID : "NULL" )
     << "\" ";
  of << indent << "SpatialAtlasNodeID=\"" 
     << (this->SpatialAtlasNodeID ? this->SpatialAtlasNodeID : "NULL")
     << "\" ";
  of << indent << "SubParcellationNodeID=\"" 
     << (this->SubParcellationNodeID ? this->SubParcellationNodeID : "NULL")
     << "\" ";
  of << indent << "EMSWorkingDataNodeID=\"" 
     << (this->EMSWorkingDataNodeID ? this->EMSWorkingDataNodeID : "NULL") 
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
  if (this->SpatialAtlasNodeID && 
      !strcmp(oldID, this->SpatialAtlasNodeID))
    {
    this->SetSpatialAtlasNodeID(newID);
    }
  if (this->SubParcellationNodeID && 
      !strcmp(oldID, this->SubParcellationNodeID))
    {
    this->SetSubParcellationNodeID(newID);
    }
  if (this->EMSWorkingDataNodeID && !strcmp(oldID, this->EMSWorkingDataNodeID))
    {
    this->SetEMSWorkingDataNodeID(newID);
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

  if (this->SpatialAtlasNodeID != NULL && 
      this->Scene->GetNodeByID(this->SpatialAtlasNodeID) == NULL)
    {
    this->SetSpatialAtlasNodeID(NULL);
    }

  if (this->SubParcellationNodeID != NULL && 
      this->Scene->GetNodeByID(this->SubParcellationNodeID) == NULL)
    {
    this->SetSubParcellationNodeID(NULL);
    }

  if (this->EMSWorkingDataNodeID != NULL && 
      this->Scene->GetNodeByID(this->EMSWorkingDataNodeID) == NULL)
    {
    this->SetEMSWorkingDataNodeID(NULL);
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
    else if (!strcmp(key, "SpatialAtlasNodeID"))
      {
      this->SetSpatialAtlasNodeID(val);
      }
    else if (!strcmp(key, "SubParcellationNodeID"))
      {
      this->SetSubParcellationNodeID(val);
      }
    else if (!strcmp(key, "EMSWorkingDataNodeID"))
      {
      this->SetEMSWorkingDataNodeID(val);
      //this->Scene->AddReferencedNodeID(this->EMSWorkingDataNodeID, this);
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
  this->SetSpatialAtlasNodeID(node->SpatialAtlasNodeID);
  this->SetSubParcellationNodeID(node->SubParcellationNodeID);
  this->SetEMSWorkingDataNodeID(node->EMSWorkingDataNodeID);
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
  os << indent << "SpatialAtlasNodeID: " <<
    (this->SpatialAtlasNodeID ? this->SpatialAtlasNodeID : "(none)") 
     << "\n";
  os << indent << "SubParcellationNodeID: " <<
    (this->SubParcellationNodeID ? this->SubParcellationNodeID : "(none)") 
     << "\n";
   os << indent << "EMSWorkingDataNodeID: " 
     << (this->EMSWorkingDataNodeID ? this->EMSWorkingDataNodeID : "(none)" )
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
//-----------------------------------------------------------------------------
vtkMRMLEMSAtlasNode* 
vtkMRMLEMSTemplateNode::GetSpatialAtlasNode()
{
  vtkMRMLEMSAtlasNode* node = NULL;
  if (this->GetScene() && this->SpatialAtlasNodeID )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->SpatialAtlasNodeID);
    node = vtkMRMLEMSAtlasNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSVolumeCollectionNode*
vtkMRMLEMSTemplateNode::
GetSubParcellationNode()
{
  vtkMRMLEMSVolumeCollectionNode* node = NULL;
  if (this->GetScene() && this->SubParcellationNodeID )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->SubParcellationNodeID);
    node = vtkMRMLEMSVolumeCollectionNode::SafeDownCast(snode);
    }
  return node;
}

vtkMRMLEMSWorkingDataNode* vtkMRMLEMSTemplateNode::GetEMSWorkingDataNode()
{
  vtkMRMLEMSWorkingDataNode* node = NULL;
  if (this->GetScene() && this->GetEMSWorkingDataNodeID() )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->EMSWorkingDataNodeID);
    node = vtkMRMLEMSWorkingDataNode::SafeDownCast(snode);
    }
  return node;
}


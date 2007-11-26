#include "vtkMRMLEMSSegmenterNode.h"
#include <sstream>
#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLEMSWorkingDataNode.h"

//-----------------------------------------------------------------------------
vtkMRMLEMSSegmenterNode* 
vtkMRMLEMSSegmenterNode::
New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSSegmenterNode");
  if(ret)
    {
    return (vtkMRMLEMSSegmenterNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSSegmenterNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* 
vtkMRMLEMSSegmenterNode::
CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSSegmenterNode");
  if(ret)
    {
    return (vtkMRMLEMSSegmenterNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSSegmenterNode;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSSegmenterNode::vtkMRMLEMSSegmenterNode()
{
  this->TemplateNodeID           = NULL;
  this->AtlasNodeID              = NULL;
  this->TargetNodeID             = NULL;
  this->OutputVolumeNodeID       = NULL;
  this->WorkingDataNodeID        = NULL;
  this->WorkingDirectory         = NULL;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSSegmenterNode::~vtkMRMLEMSSegmenterNode()
{
  this->SetTemplateNodeID(NULL);
  this->SetAtlasNodeID(NULL);
  this->SetTargetNodeID(NULL);
  this->SetOutputVolumeNodeID(NULL);
  this->SetWorkingDataNodeID(NULL);
  this->SetWorkingDirectory(NULL);
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSSegmenterNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << "TemplateNodeID=\"" 
     << (this->TemplateNodeID ? this->TemplateNodeID : "NULL") << "\" ";
  of << indent << "AtlasNodeID=\"" 
     << (this->AtlasNodeID ? this->AtlasNodeID : "NULL") 
     << "\" ";
  of << indent << "TargetNodeID=\"" 
     << (this->TargetNodeID ? this->TargetNodeID : "NULL") 
     << "\" ";
  of << indent << "OutputVolumeNodeID=\"" 
     << (this->OutputVolumeNodeID ? this->OutputVolumeNodeID : "NULL") 
     << "\" ";
  of << indent << "WorkingDataNodeID=\"" 
     << (this->WorkingDataNodeID ? this->WorkingDataNodeID : "NULL") 
     << "\" ";
  of << indent << "WorkingDirectory=\"" 
     << (this->WorkingDirectory ? this->WorkingDirectory : "NULL") << "\" ";
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSSegmenterNode::
UpdateReferenceID(const char* oldID, const char* newID)
{
  if (this->TemplateNodeID && !strcmp(oldID, this->TemplateNodeID))
    {
    this->SetTemplateNodeID(newID);
    }
  if (this->AtlasNodeID && !strcmp(oldID, this->AtlasNodeID))
    {
    this->SetAtlasNodeID(newID);
    }
  if (this->TargetNodeID && !strcmp(oldID, this->TargetNodeID))
    {
    this->SetTargetNodeID(newID);
    }
  if (this->OutputVolumeNodeID && !strcmp(oldID, this->OutputVolumeNodeID))
    {
    this->SetOutputVolumeNodeID(newID);
    }
  if (this->WorkingDataNodeID && !strcmp(oldID, this->WorkingDataNodeID))
    {
    this->SetWorkingDataNodeID(newID);
    }
}

//-----------------------------------------------------------------------------
void 
vtkMRMLEMSSegmenterNode::
UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->TemplateNodeID != NULL && 
      this->Scene->GetNodeByID(this->TemplateNodeID) == NULL)
    {
    this->SetTemplateNodeID(NULL);
    }
  if (this->AtlasNodeID != NULL && 
      this->Scene->GetNodeByID(this->AtlasNodeID) == NULL)
    {
    this->SetAtlasNodeID(NULL);
    }
  if (this->TargetNodeID != NULL && 
      this->Scene->GetNodeByID(this->TargetNodeID) == NULL)
    {
    this->SetTargetNodeID(NULL);
    }
  if (this->OutputVolumeNodeID != NULL && 
      this->Scene->GetNodeByID(this->OutputVolumeNodeID) == NULL)
    {
    this->SetOutputVolumeNodeID(NULL);
    }
  if (this->WorkingDataNodeID != NULL && 
      this->Scene->GetNodeByID(this->WorkingDataNodeID) == NULL)
    {
    this->SetWorkingDataNodeID(NULL);
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSSegmenterNode::ReadXMLAttributes(const char** attrs)
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
    
    if (!strcmp(key, "TemplateNodeID"))
      {
      this->SetTemplateNodeID(val);
      //this->Scene->AddReferencedNodeID(this->TemplateNodeID, this);
      }
    else if (!strcmp(key, "AtlasNodeID"))
      {
      this->SetAtlasNodeID(val);
      //this->Scene->AddReferencedNodeID(this->AtlasNodeID, this);
      }
    else if (!strcmp(key, "TargetNodeID"))
      {
      this->SetTargetNodeID(val);
      //this->Scene->AddReferencedNodeID(this->TargetNodeID, this);
      }
    else if (!strcmp(key, "OutputVolumeNodeID"))
      {
      this->SetOutputVolumeNodeID(val);
      //this->Scene->AddReferencedNodeID(this->OutputVolumeNodeID, this);
      }
    else if (!strcmp(key, "WorkingDataNodeID"))
      {
      this->SetWorkingDataNodeID(val);
      //this->Scene->AddReferencedNodeID(this->WorkingDataNodeID, this);
      }
    else if (!strcmp(key, "WorkingDirectory"))
      {
      this->SetWorkingDirectory(val);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSSegmenterNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
  vtkMRMLEMSSegmenterNode* node = (vtkMRMLEMSSegmenterNode*) rhs;

  this->SetTemplateNodeID(node->TemplateNodeID);
  this->SetAtlasNodeID(node->AtlasNodeID);
  this->SetTargetNodeID(node->TargetNodeID);
  this->SetOutputVolumeNodeID(node->OutputVolumeNodeID);
  this->SetWorkingDataNodeID(node->WorkingDataNodeID);
  this->SetWorkingDirectory(node->WorkingDirectory);
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSSegmenterNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  
  os << indent << "TemplateNodeID: " 
     << (this->TemplateNodeID ? this->TemplateNodeID : "(none)") << "\n";
  
  os << indent << "AtlasNodeID: " 
     << (this->AtlasNodeID ? this->AtlasNodeID : "(none)") 
     << "\n";

  os << indent << "TargetNodeID: " 
     << (this->TargetNodeID ? this->TargetNodeID : "(none)" ) 
     << "\n";

  os << indent << "OutputVolumeNodeID: " 
     << (this->OutputVolumeNodeID ? this->OutputVolumeNodeID : "(none)" )
     << "\n";

  os << indent << "WorkingDataNodeID: " 
     << (this->WorkingDataNodeID ? this->WorkingDataNodeID : "(none)" )
     << "\n";

  os << indent << "WorkingDirectory: " 
     << (this->WorkingDirectory ? this->WorkingDirectory : "(none)" )
     << "\n";
}

//-----------------------------------------------------------------------------
vtkMRMLEMSAtlasNode*
vtkMRMLEMSSegmenterNode::
GetAtlasNode()
{
  vtkMRMLEMSAtlasNode* node = NULL;
  if (this->GetScene() && this->GetAtlasNodeID() )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->AtlasNodeID);
    node = vtkMRMLEMSAtlasNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTemplateNode*
vtkMRMLEMSSegmenterNode::
GetTemplateNode()
{
  vtkMRMLEMSTemplateNode* node = NULL;
  if (this->GetScene() && this->GetTemplateNodeID() )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->TemplateNodeID);
    node = vtkMRMLEMSTemplateNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSTargetNode*
vtkMRMLEMSSegmenterNode::
GetTargetNode()
{
  vtkMRMLEMSTargetNode* node = NULL;
  if (this->GetScene() && this->GetTargetNodeID() )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->TargetNodeID);
    node = vtkMRMLEMSTargetNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode*
vtkMRMLEMSSegmenterNode::
GetOutputVolumeNode()
{
  vtkMRMLScalarVolumeNode* node = NULL;
  if (this->GetScene() && this->GetOutputVolumeNodeID() )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->OutputVolumeNodeID);
    node = vtkMRMLScalarVolumeNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSWorkingDataNode*
vtkMRMLEMSSegmenterNode::
GetWorkingDataNode()
{
  vtkMRMLEMSWorkingDataNode* node = NULL;
  if (this->GetScene() && this->GetWorkingDataNodeID() )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->WorkingDataNodeID);
    node = vtkMRMLEMSWorkingDataNode::SafeDownCast(snode);
    }
  return node;
}

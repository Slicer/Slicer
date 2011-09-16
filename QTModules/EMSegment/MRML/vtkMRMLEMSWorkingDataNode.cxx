#include "vtkMRMLEMSWorkingDataNode.h"
#include <sstream>
#include "vtkMRMLScene.h"
#include "vtkMRMLEMSVolumeCollectionNode.h"
#include <vtkObjectFactory.h>
#include "vtkMRMLEMSAtlasNode.h"
#include <vtksys/stl/algorithm>
#include <vtksys/stl/string>
#include "vtkMRMLScalarVolumeNode.h"
// for some reason it was otherwise not wrapping it in tcl
// maybe take it out later 

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
  this->AlignedTargetNodeID              = NULL;
  this->AlignedAtlasNodeID               = NULL;
  this->AlignedSubParcellationNodeID     = NULL;
  this->OutputSegmentationNodeID               = NULL;

  this->InputTargetNodeIsValid           = 0;
  this->AlignedTargetNodeIsValid         = 0;  
  this->InputAtlasNodeIsValid            = 0;  
  this->AlignedAtlasNodeIsValid          = 0;  

  // Legacy 
  this->InputAtlasNodeID                 = NULL;
  this->InputSubParcellationNodeID       = NULL;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSWorkingDataNode::~vtkMRMLEMSWorkingDataNode()
{
  this->SetInputTargetNodeID(NULL);
  this->SetAlignedTargetNodeID(NULL);
  this->SetAlignedAtlasNodeID(NULL);
  this->SetAlignedSubParcellationNodeID(NULL);
  this->SetOutputSegmentationNodeID(NULL);

  this->SetInputAtlasNodeID(NULL);
  this->SetInputSubParcellationNodeID(NULL);

}

//-----------------------------------------------------------------------------
void vtkMRMLEMSWorkingDataNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << " InputTargetNodeID=\"" 
     << (this->InputTargetNodeID ? this->InputTargetNodeID : "NULL")
     << "\" ";
  of << indent << " InputTargetNodeIsValid=\""   << this->InputTargetNodeIsValid << "\" ";

  of << indent << "AlignedTargetNodeID=\"" 
     << (this->AlignedTargetNodeID ? this->AlignedTargetNodeID : "NULL")
     << "\" ";
  of << indent << " AlignedTargetNodeIsValid=\"" << this->AlignedTargetNodeIsValid  << "\" ";

  of << indent << " InputAtlasNodeIsValid=\""    << this->InputAtlasNodeIsValid  << "\" ";

  of << indent << "AlignedAtlasNodeID=\"" 
     << (this->AlignedAtlasNodeID ? this->AlignedAtlasNodeID : "NULL")
     << "\" ";
  of << indent << " AlignedAtlasNodeIsValid=\"" <<  this->AlignedAtlasNodeIsValid  << "\" ";

  of << indent << "AlignedSubParcellationNodeID=\"" 
     << (this->AlignedSubParcellationNodeID ? this->AlignedSubParcellationNodeID : "NULL")
     << "\" ";

  of << indent << "OutputSegmentationNodeID=\"" 
     << (this->OutputSegmentationNodeID ? this->OutputSegmentationNodeID : "NULL") 
     << "\" ";

  if (this->InputAtlasNodeID)
    {
      of << indent << "InputAtlasNodeID=\"" <<  this->InputAtlasNodeID << "\" ";
    }

  if (this->InputSubParcellationNodeID)
    {
      of << indent << "InputSubParcellationNodeID=\"" <<  this->InputSubParcellationNodeID << "\" ";
    }
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
  if (this->AlignedTargetNodeID && 
      !strcmp(oldID, this->AlignedTargetNodeID))
    {
    this->SetAlignedTargetNodeID(newID);
    }
  if (this->AlignedAtlasNodeID && 
      !strcmp(oldID, this->AlignedAtlasNodeID))
    {
    this->SetAlignedAtlasNodeID(newID);
    }
  if (this->AlignedSubParcellationNodeID && 
      !strcmp(oldID, this->AlignedSubParcellationNodeID))
    {
    this->SetAlignedSubParcellationNodeID(newID);
    }

  if (this->OutputSegmentationNodeID && !strcmp(oldID, this->OutputSegmentationNodeID))
    {
    this->SetOutputSegmentationNodeID(newID);
    }

  if (this->InputAtlasNodeID && 
      !strcmp(oldID, this->InputAtlasNodeID))
    {
    this->SetInputAtlasNodeID(newID);
    }

  if (this->InputSubParcellationNodeID && 
      !strcmp(oldID, this->InputSubParcellationNodeID))
    {
    this->SetInputSubParcellationNodeID(newID);
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
  if (this->AlignedTargetNodeID != NULL && 
      this->Scene->GetNodeByID(this->AlignedTargetNodeID) == NULL)
    {
    this->SetAlignedTargetNodeID(NULL);
    }
  if (this->AlignedAtlasNodeID != NULL && 
      this->Scene->GetNodeByID(this->AlignedAtlasNodeID) == NULL)
    {
    this->SetAlignedAtlasNodeID(NULL);
    }
  if (this->AlignedSubParcellationNodeID != NULL && 
      this->Scene->GetNodeByID(this->AlignedSubParcellationNodeID) == NULL)
    {
    this->SetAlignedSubParcellationNodeID(NULL);
    }

  if (this->OutputSegmentationNodeID != NULL && 
      this->Scene->GetNodeByID(this->OutputSegmentationNodeID) == NULL)
    {
    this->SetOutputSegmentationNodeID(NULL);
    }

    if (this->InputAtlasNodeID != NULL && 
      this->Scene->GetNodeByID(this->InputAtlasNodeID) == NULL)
    {
    this->SetInputAtlasNodeID(NULL);
    }

   if (this->InputSubParcellationNodeID != NULL && 
      this->Scene->GetNodeByID(this->InputSubParcellationNodeID) == NULL)
    {
    this->SetInputSubParcellationNodeID(NULL);
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
      }
    else if (!strcmp(key, "InputTargetNodeIsValid"))
      {
    vtksys_stl::stringstream ss;
    ss << val;
    ss >> this->InputTargetNodeIsValid;
      }
    else if (!strcmp(key, "AlignedTargetNodeID"))
      {
      this->SetAlignedTargetNodeID(val);
      }
    else if (!strcmp(key, "AlignedTargetNodeIsValid"))
      {
    vtksys_stl::stringstream ss;
    ss << val;
    ss >> this->AlignedTargetNodeIsValid;
      }
    else if (!strcmp(key, "InputAtlasNodeIsValid"))
      {
    vtksys_stl::stringstream ss;
    ss << val;
    ss >> this->InputAtlasNodeIsValid;
      }
    else if (!strcmp(key, "AlignedAtlasNodeIsValid"))
      {
    vtksys_stl::stringstream ss;
    ss << val;
    ss >> this->AlignedAtlasNodeIsValid;
      }
    else if (!strcmp(key, "AlignedAtlasNodeID"))
      {
      this->SetAlignedAtlasNodeID(val);
      }
    else if (!strcmp(key, "AlignedSubParcellationNodeID"))
      {
      this->SetAlignedSubParcellationNodeID(val);
      }
    else if (!strcmp(key, "OutputSegmentationNodeID"))
      {
      this->SetOutputSegmentationNodeID(val);
      }
    else if (!strcmp(key, "InputAtlasNodeID"))
      {
      this->SetInputAtlasNodeID(val);
      }
    else if (!strcmp(key, "InputSubParcellationNodeID"))
      {
      this->SetInputSubParcellationNodeID(val);
      }
    }

 
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSWorkingDataNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
  vtkMRMLEMSWorkingDataNode* node = (vtkMRMLEMSWorkingDataNode*) rhs;

  this->SetInputTargetNodeID(node->InputTargetNodeID);
  this->SetAlignedTargetNodeID(node->AlignedTargetNodeID);
  this->SetAlignedAtlasNodeID(node->AlignedAtlasNodeID);
  this->SetAlignedSubParcellationNodeID(node->AlignedSubParcellationNodeID);
  this->SetOutputSegmentationNodeID(node->OutputSegmentationNodeID);
  this->InputTargetNodeIsValid           = node->InputTargetNodeIsValid;
  this->AlignedTargetNodeIsValid         = node->AlignedTargetNodeIsValid;  
  this->InputAtlasNodeIsValid            = node->InputAtlasNodeIsValid;  
  this->AlignedAtlasNodeIsValid          = node->AlignedAtlasNodeIsValid;  
  
  this->SetInputAtlasNodeID(node->InputAtlasNodeID);
  this->SetInputSubParcellationNodeID(node->InputSubParcellationNodeID);
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSWorkingDataNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  
  os << indent << "InputTargetNodeID: " <<
    (this->InputTargetNodeID ? this->InputTargetNodeID : "(none)") 
     << "\n";
  os << indent << "AlignedTargetNodeID: " <<
    (this->AlignedTargetNodeID ? this->AlignedTargetNodeID : "(none)") 
     << "\n";

  os << indent << "AlignedAtlasNodeID: " <<
    (this->AlignedAtlasNodeID ? this->AlignedAtlasNodeID : "(none)") 
     << "\n";

  os << indent << "AlignedSubParcellationNodeID: " <<
    (this->AlignedSubParcellationNodeID ? this->AlignedSubParcellationNodeID : "(none)") 
     << "\n";

  os << indent << "OutputSegmentationNodeID: " 
     << (this->OutputSegmentationNodeID ? this->OutputSegmentationNodeID : "(none)" )
     << "\n";

  os << indent << "InputTargetNodeIsValid: " << this->InputTargetNodeIsValid << "\n";
  os << indent << "AlignedTargetNodeIsValid: " << this->AlignedTargetNodeIsValid << "\n";
  os << indent << "InputAtlasNodeIsValid: " << this->InputAtlasNodeIsValid  << "\n";
  os << indent << "AlignedAtlasNodeIsValid: " << this->AlignedAtlasNodeIsValid << "\n";

  if (this->InputAtlasNodeID)
    {  
     os << indent << "InputAtlasNodeID: " << this->InputAtlasNodeID << "\n";
    }

  if (this->InputSubParcellationNodeID)
    {
        os << indent << "InputSubParcellationNodeID: " << this->InputSubParcellationNodeID  << "\n";
    }
}

//-----------------------------------------------------------------------------
vtkMRMLEMSVolumeCollectionNode*
vtkMRMLEMSWorkingDataNode::
GetInputTargetNode()
{
  vtkMRMLEMSVolumeCollectionNode* node = NULL;
  if (this->GetScene() && this->InputTargetNodeID)
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->InputTargetNodeID);
    node = vtkMRMLEMSVolumeCollectionNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSVolumeCollectionNode*
vtkMRMLEMSWorkingDataNode::
GetAlignedTargetNode()
{
  vtkMRMLEMSVolumeCollectionNode* node = NULL;
  if (this->GetScene() && this->AlignedTargetNodeID )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->AlignedTargetNodeID);
    if (snode == NULL)
      {
      vtkErrorMacro("AlignedTargetNode with ID " << this->AlignedTargetNodeID << " is null");
      }
    node = vtkMRMLEMSVolumeCollectionNode::SafeDownCast(snode);
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
vtkMRMLEMSVolumeCollectionNode*
vtkMRMLEMSWorkingDataNode::
GetAlignedSubParcellationNode()
{
  vtkMRMLEMSVolumeCollectionNode* node = NULL;
  if (this->GetScene() && this->AlignedSubParcellationNodeID )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->AlignedSubParcellationNodeID);
    node = vtkMRMLEMSVolumeCollectionNode::SafeDownCast(snode);
    }
  return node;
}


//-----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode*
vtkMRMLEMSWorkingDataNode::
GetOutputSegmentationNode()
{
  vtkMRMLScalarVolumeNode* node = NULL;
  if (this->GetScene() && this->GetOutputSegmentationNodeID() )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->OutputSegmentationNodeID);
    node = vtkMRMLScalarVolumeNode::SafeDownCast(snode);
    }
  return node;
}


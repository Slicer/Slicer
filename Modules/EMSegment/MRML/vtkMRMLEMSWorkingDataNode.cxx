#include "vtkMRMLEMSWorkingDataNode.h"
#include <sstream>
#include "vtkMRMLScene.h"
#include "vtkMRMLEMSTargetNode.h"
#include "vtkMRMLEMSAtlasNode.h"

// for some reason it was otherwise not wrapping it in tcl
// maybe take it out later 
#define vtkSetReferenceStringMacroWDN(name) \
void vtkMRMLEMSWorkingDataNode::Set##name (const char* _arg) \
  { \
  if ( this->name == NULL && _arg == NULL) { return;} \
  if ( this->name && _arg && (!strcmp(this->name,_arg))) { return;} \
  std::string oldValue; \
  if (this->name) { oldValue = this->name; delete [] this->name;  } \
  if (_arg) \
    { \
    size_t n = strlen(_arg) + 1; \
    char *cp1 =  new char[n]; \
    const char *cp2 = (_arg); \
    this->name = cp1; \
    do { *cp1++ = *cp2++; } while ( --n ); \
    } \
   else \
    { \
    this->name = NULL; \
    } \
  this->Modified(); \
  if (this->Scene && this->name) \
    { \
    if (oldValue.size() > 0) \
      { \
      this->Scene->RemoveReferencedNodeID(oldValue.c_str(), this); \
      } \
    this->Scene->AddReferencedNodeID(this->name, this); \
    } \
  } 

vtkSetReferenceStringMacroWDN(InputTargetNodeID)
vtkSetReferenceStringMacroWDN(NormalizedTargetNodeID)
vtkSetReferenceStringMacroWDN(AlignedTargetNodeID);
vtkSetReferenceStringMacroWDN(InputAtlasNodeID);
vtkSetReferenceStringMacroWDN(AlignedAtlasNodeID);

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

  this->InputTargetNodeIsValid           = 0;
  this->NormalizedTargetNodeIsValid      = 0;  
  this->AlignedTargetNodeIsValid         = 0;  
  this->InputAtlasNodeIsValid            = 0;  
  this->AlignedAtlasNodeIsValid          = 0;  
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

  of << indent << " InputTargetNodeID=\"" 
     << (this->InputTargetNodeID ? this->InputTargetNodeID : "NULL")
     << "\" ";
  of << indent << " InputTargetNodeIsValid=\""   << this->InputTargetNodeIsValid << "\" ";

  of << indent << "NormalizedTargetNodeID=\"" 
     << (this->NormalizedTargetNodeID ? this->NormalizedTargetNodeID : "NULL")
     << "\" ";
  of << indent << "AlignedTargetNodeID=\"" 
     << (this->AlignedTargetNodeID ? this->AlignedTargetNodeID : "NULL")
     << "\" ";
  of << indent << " AlignedTargetNodeIsValid=\"" << this->AlignedTargetNodeIsValid  << "\" ";

  of << indent << "InputAtlasNodeID=\"" 
     << (this->InputAtlasNodeID ? this->InputAtlasNodeID : "NULL")
     << "\" ";
  of << indent << " InputAtlasNodeIsValid=\""    << this->InputAtlasNodeIsValid  << "\" ";

  of << indent << "AlignedAtlasNodeID=\"" 
     << (this->AlignedAtlasNodeID ? this->AlignedAtlasNodeID : "NULL")
     << "\" ";
  of << indent << " AlignedAtlasNodeIsValid=\"" <<  this->AlignedAtlasNodeIsValid  << "\" ";

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
    else if (!strcmp(key, "InputTargetNodeIsValid"))
      {
    vtksys_stl::stringstream ss;
    ss << val;
    ss >> this->InputTargetNodeIsValid;
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
    else if (!strcmp(key, "AlignedTargetNodeIsValid"))
      {
    vtksys_stl::stringstream ss;
    ss << val;
    ss >> this->AlignedTargetNodeIsValid;
      }
    else if (!strcmp(key, "InputAtlasNodeID"))
      {
      this->SetInputAtlasNodeID(val);
      //this->Scene->AddReferencedNodeID(this->InputAtlasNodeID, this);
      }
    else if (!strcmp(key, "InputAtlasNodeIsValid"))
      {
    vtksys_stl::stringstream ss;
    ss << val;
    ss >> this->InputAtlasNodeIsValid;
      }
    else if (!strcmp(key, "AlignedAtlasNodeID"))
      {
      this->SetAlignedAtlasNodeID(val);
      //this->Scene->AddReferencedNodeID(this->AlignedAtlasNodeID, this);
      }
    else if (!strcmp(key, "AlignedAtlasNodeIsValid"))
      {
    vtksys_stl::stringstream ss;
    ss << val;
    ss >> this->AlignedAtlasNodeIsValid;
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
  this->InputTargetNodeIsValid           = node->InputTargetNodeIsValid;
  this->AlignedTargetNodeIsValid         = node->AlignedTargetNodeIsValid;  
  this->InputAtlasNodeIsValid            = node->InputAtlasNodeIsValid;  
  this->AlignedAtlasNodeIsValid          = node->AlignedAtlasNodeIsValid;  
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSWorkingDataNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  
  os << indent << "InputTargetNodeID: " <<
    (this->InputTargetNodeID ? this->InputTargetNodeID : "(none)") 
     << "\n";
  os << indent << "InputTargetNodeIsValid: " << this->InputTargetNodeIsValid << "\n";

  os << indent << "NormalizedTargetNodeID: " <<
    (this->NormalizedTargetNodeID ? this->NormalizedTargetNodeID : "(none)") 
     << "\n";
  os << indent << "AlignedTargetNodeID: " <<
    (this->AlignedTargetNodeID ? this->AlignedTargetNodeID : "(none)") 
     << "\n";
  os << indent << "AlignedTargetNodeIsValid: " << this->AlignedTargetNodeIsValid << "\n";

  os << indent << "InputAtlasNodeID: " <<
    (this->InputAtlasNodeID ? this->InputAtlasNodeID : "(none)") 
     << "\n";
  os << indent << "InputAtlasNodeIsValid: " << this->InputAtlasNodeIsValid  << "\n";

  os << indent << "AlignedAtlasNodeID: " <<
    (this->AlignedAtlasNodeID ? this->AlignedAtlasNodeID : "(none)") 
     << "\n";
  os << indent << "AlignedAtlasNodeIsValid: " << this->AlignedAtlasNodeIsValid << "\n";

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



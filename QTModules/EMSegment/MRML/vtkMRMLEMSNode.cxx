#include "vtkMRMLEMSNode.h"
#include <sstream>
#include <vtkObjectFactory.h>
#include "vtkMRMLScene.h"

#include <vtksys/ios/sstream>
#include "vtkMRMLEMSSegmenterNode.h"

//-----------------------------------------------------------------------------
vtkMRMLEMSNode* 
vtkMRMLEMSNode::
New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSNode");
  if(ret)
    {
    return (vtkMRMLEMSNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* 
vtkMRMLEMSNode::
CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSNode");
  if(ret)
    {
    return (vtkMRMLEMSNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSNode;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSNode::vtkMRMLEMSNode()
{
  this->SegmenterNodeID               = NULL;
  this->TemplateFilename              = NULL;
  this->SaveTemplateAfterSegmentation = 0;
  this->TclTaskFilename = NULL;
  this->TaskPreprocessingSetting = NULL;

  this->SetTclTaskFilename(this->GetDefaultTclTaskFilename());
}

//-----------------------------------------------------------------------------
vtkMRMLEMSNode::~vtkMRMLEMSNode()
{
  this->SetSegmenterNodeID(NULL);
  this->SetTemplateFilename(NULL);
  this->SetTclTaskFilename(NULL);
  this->SetTaskPreprocessingSetting(NULL);
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << " SegmenterNodeID=\"" 
     << (this->SegmenterNodeID ? this->SegmenterNodeID : "NULL") << "\" ";
  of << indent << "TemplateFilename=\"" 
     << (this->TemplateFilename ? this->TemplateFilename : "NULL") << "\" ";
  of << indent << "SaveTemplateAfterSegmentation=\"" 
     << this->SaveTemplateAfterSegmentation << "\" ";
  of << indent << "TclTaskFilename=\"" 
     << (this->TclTaskFilename ? this->TclTaskFilename  : "NULL") << "\" ";
  of << indent << "TaskPreprocessingSetting=\"" 
     << (this->TaskPreprocessingSetting ? this->TaskPreprocessingSetting : "NULL") << "\" ";

}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSNode::
UpdateReferenceID(const char* oldID, const char* newID)
{
  if (this->SegmenterNodeID && !strcmp(oldID, this->SegmenterNodeID))
    {
    this->SetSegmenterNodeID(newID);
    }
}

//-----------------------------------------------------------------------------
void 
vtkMRMLEMSNode::
UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->SegmenterNodeID != NULL && 
      this->Scene->GetNodeByID(this->SegmenterNodeID) == NULL)
    {
    this->SetSegmenterNodeID(NULL);
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSNode::ReadXMLAttributes(const char** attrs)
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
    
    if (!strcmp(key, "SegmenterNodeID"))
      {
      
      this->SetSegmenterNodeID(val);
      cout << "WARNING: vtkMRMLEMSNode::ReadXMLAttributes:EMSNode is outdated since Slicer 3.6.3  - please update your MRML file" << endl;
      }
    else if (!strcmp(key, "TemplateFilename"))
      {
      this->SetTemplateFilename(val);
      cout << "WARNING: vtkMRMLEMSNode::ReadXMLAttributes:EMSNode is outdated since Slicer 3.6.3  - please update your MRML file" << endl;
      }
    else if (!strcmp(key, "SaveTemplateAfterSegmentation"))
      {
      cout << "WARNING: vtkMRMLEMSNode::ReadXMLAttributes:EMSNode is outdated since Slicer 3.6.3  - please update your MRML file" << endl;
      vtksys_stl::stringstream ss;
      ss << val;
      ss >> this->SaveTemplateAfterSegmentation;
      }
    else if (!strcmp(key, "TclTaskFilename"))
      {
      cout << "WARNING: vtkMRMLEMSNode::ReadXMLAttributes:EMSNode is outdated since Slicer 3.6.3  - please update your MRML file" << endl;
      this->SetTclTaskFilename(val);
      }
    else if (!strcmp(key, "TaskPreprocessingSetting"))
      {
      cout << "WARNING: vtkMRMLEMSNode::ReadXMLAttributes:EMSNode is outdated since Slicer 3.6.3  - please update your MRML file" << endl;
      this->SetTaskPreprocessingSetting(val);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
  vtkMRMLEMSNode* node = (vtkMRMLEMSNode*) rhs;

  this->SetSegmenterNodeID(node->SegmenterNodeID);
  this->SetTemplateFilename(node->TemplateFilename);
  this->SetSaveTemplateAfterSegmentation(node->SaveTemplateAfterSegmentation);
  this->SetTclTaskFilename(node->TclTaskFilename);
  this->SetTaskPreprocessingSetting(node->TaskPreprocessingSetting);
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  
  os << indent << "SegmenterNodeID: " <<
    (this->SegmenterNodeID ? this->SegmenterNodeID : "(none)") << "\n";
  os << indent << "TemplateFilename: " <<
    (this->TemplateFilename ? this->TemplateFilename : "(none)") << "\n";
  os << indent << "SaveTemplateAfterSegmentation: " 
     << this->SaveTemplateAfterSegmentation << "\n";
  os << indent << "TclTaskFilename: " <<
    (this->TclTaskFilename ? this->TclTaskFilename : "(none)") << "\n";
  os << indent << "TaskPreprocessingSetting: " <<
    (this->TaskPreprocessingSetting ? this->TaskPreprocessingSetting : "(none)") << "\n";

}

//-----------------------------------------------------------------------------
vtkMRMLEMSSegmenterNode*
vtkMRMLEMSNode::
GetSegmenterNode()
{
  vtkMRMLEMSSegmenterNode* node = NULL;
  if (this->GetScene() && this->GetSegmenterNodeID() )
    {
    vtkMRMLNode* snode = 
      this->GetScene()->GetNodeByID(this->SegmenterNodeID);
    node = vtkMRMLEMSSegmenterNode::SafeDownCast(snode);
    } 
  return node;
}

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkMRMLLayoutNode.h"
#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkMRMLLayoutNode* vtkMRMLLayoutNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLLayoutNode");
  if(ret)
    {
    return (vtkMRMLLayoutNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLLayoutNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLLayoutNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLLayoutNode");
  if(ret)
    {
    return (vtkMRMLLayoutNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLLayoutNode;
}

//----------------------------------------------------------------------------
vtkMRMLLayoutNode::vtkMRMLLayoutNode()
{

  this->SingletonTag = "vtkMRMLLayoutNode";
  this->HideFromEditors = 1;
  this->GUIPanelVisibility = 1;
  this->BottomPanelVisibility = 1;
  this->GUIPanelLR = 0;
//  this->ViewArrangement = this->SlicerLayoutInitialView;

  return;

}

//----------------------------------------------------------------------------
vtkMRMLLayoutNode::~vtkMRMLLayoutNode()
{
}


//----------------------------------------------------------------------------
void vtkMRMLLayoutNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes, since the parsing of the string is dependent on the
  // order here
  
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << " currentViewArrangement=\"" << this->ViewArrangement << "\"";
  of << indent << " guiPanelVisibility=\"" << this->GUIPanelVisibility << "\"";
  of << indent << " bottomPanelVisiblity =\"" << this->BottomPanelVisibility << "\"";
  of << indent << " guiPanelLR =\"" << this->GUIPanelLR << "\"";    
}


//----------------------------------------------------------------------------
void vtkMRMLLayoutNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);
  
  const char* attName;
  const char* attValue;
  
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "currentViewArrangement")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->ViewArrangement;
      }
    else if (!strcmp (attName, "guiPanelVisibility"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->GUIPanelVisibility;
      }
    else if ( !strcmp ( attName, "bottomPanelVisibility" ))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->BottomPanelVisibility;
      }
    else if ( !strcmp (attName, "guiPanelLR" ))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->GUIPanelLR;
      }
    }
}
    


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, LabelText, ID
void vtkMRMLLayoutNode::Copy(vtkMRMLNode *anode)
{
//  vtkObject::Copy(anode);
  vtkMRMLLayoutNode *node = (vtkMRMLLayoutNode *) anode;
  this->SetViewArrangement (node->ViewArrangement);
  this->SetGUIPanelVisibility(node->GUIPanelVisibility) ;
  this->SetBottomPanelVisibility (node->BottomPanelVisibility);
  this->SetGUIPanelLR ( node->GUIPanelLR);
}

//----------------------------------------------------------------------------
void vtkMRMLLayoutNode::PrintSelf(ostream& os, vtkIndent indent)
{  

  Superclass::PrintSelf(os,indent);
  
  // Layout:
  os << indent << "ViewArrangement:" << this->ViewArrangement  << "\n";
  os << indent << "GUIPanelVisibility:" << this->GUIPanelVisibility  << "\n";
  os << indent << "GUIPanelLR:" << this->GUIPanelLR  << "\n";
  os << indent << "BottomPanelVisibility:" << this->BottomPanelVisibility  << "\n";
}



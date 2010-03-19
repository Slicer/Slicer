#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLSlicerWelcomeNode.h"


//------------------------------------------------------------------------------
vtkMRMLSlicerWelcomeNode* vtkMRMLSlicerWelcomeNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLSlicerWelcomeNode");
  if(ret)
    {
      return (vtkMRMLSlicerWelcomeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLSlicerWelcomeNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLSlicerWelcomeNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLSlicerWelcomeNode");
  if(ret)
    {
      return (vtkMRMLSlicerWelcomeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLSlicerWelcomeNode;
}

//----------------------------------------------------------------------------
vtkMRMLSlicerWelcomeNode::vtkMRMLSlicerWelcomeNode()
{
  this->GUIWidth = 0;
  this->WelcomeGUIWidth = 0;
}

//----------------------------------------------------------------------------
vtkMRMLSlicerWelcomeNode::~vtkMRMLSlicerWelcomeNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLSlicerWelcomeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream

  vtkIndent indent(nIndent);
  of << indent << " LastGUIWidth =\"" << this->GetGUIWidth() << "\"";
  of << indent << " WelcomeGUIWidth =\"" << this->GetWelcomeGUIWidth() << "\"";

}


//----------------------------------------------------------------------------
void vtkMRMLSlicerWelcomeNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "LastGUIWidth"))
      {
      this->SetGUIWidth( atoi (attValue ));
      }    
    else if (!strcmp(attName, "WelcomeGUIWidth"))
      {
      this->SetWelcomeGUIWidth( atoi (attValue ));
      }    

    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLSlicerWelcomeNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLSlicerWelcomeNode *node = (vtkMRMLSlicerWelcomeNode *) anode;

  this->SetGUIWidth ( node->GetGUIWidth() );
  this->SetGUIWidth ( node->GetWelcomeGUIWidth() );
}



//----------------------------------------------------------------------------
void vtkMRMLSlicerWelcomeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLNode::PrintSelf(os,indent);
  os << indent << "GUIWidth: " << this->GetGUIWidth() << "\n";
  os << indent << "WelcomeGUIWidth: " << this->GetWelcomeGUIWidth() << "\n";
}


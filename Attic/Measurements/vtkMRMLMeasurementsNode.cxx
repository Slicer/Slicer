#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkMRMLMeasurementsNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkCxxRevisionMacro ( vtkMRMLMeasurementsNode, "$Revision: 1.0 $");

//----------------------------------------------------------------------------
//--- a word about language:
//--- Methods and vars in this module assume that:
//--- "Tag" means a metadata element comprised of an "attribute" (or "keyword") and "value".
//--- Tags may have an attribute with many possible values.
//--- Sometimes "Tag" is used to mean "attribute".
//--- we'll change this eventually to be "Tagname"
//----------------------------------------------------------------------------


//------------------------------------------------------------------------------
vtkMRMLMeasurementsNode* vtkMRMLMeasurementsNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLMeasurementsNode");
  if(ret)
    {
      return (vtkMRMLMeasurementsNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLMeasurementsNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLMeasurementsNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLMeasurementsNode");
  if(ret)
    {
      return (vtkMRMLMeasurementsNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLMeasurementsNode;
}

//----------------------------------------------------------------------------
vtkMRMLMeasurementsNode::vtkMRMLMeasurementsNode()
{
  this->HideFromEditors = false;

  this->Visibility = 1;
}



//----------------------------------------------------------------------------
vtkMRMLMeasurementsNode::~vtkMRMLMeasurementsNode()
{
 
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurementsNode::WriteXML(ostream& of, int nIndent)
{

  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkIndent indent(nIndent);

  of << " visible=\"" << this->GetVisibility() << "\"";

}


//----------------------------------------------------------------------------
void vtkMRMLMeasurementsNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "visible"))
      {
      this->SetVisibility(atoi(attValue));
      }
    }
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLMeasurementsNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLMeasurementsNode *node = (vtkMRMLMeasurementsNode *) anode;

  this->SetVisibility ( node->GetVisibility() );
}


//----------------------------------------------------------------------------
void vtkMRMLMeasurementsNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "Visibility: " << this->GetVisibility() << "\n";
}




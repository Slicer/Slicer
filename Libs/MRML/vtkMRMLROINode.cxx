#include <string>
#include <iostream>
#include <sstream>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLROINode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLROINode* vtkMRMLROINode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLROINode");
  if(ret)
    {
    return (vtkMRMLROINode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLROINode;
}
 
//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLROINode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLROINode");
  if(ret)
    {
    return (vtkMRMLROINode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLROINode;
}

//----------------------------------------------------------------------------
void vtkMRMLROINode::PrintSelf(ostream& os, vtkIndent indent)
{  
  vtkObject::PrintSelf(os,indent);
  //Fill in ......
  return;
}

//----------------------------------------------------------------------------
vtkMRMLROINode::vtkMRMLROINode()
{
  this->XYZ[0] = this->XYZ[1] = this->XYZ[2] = 0.0;
  this->DeltaXYZ[0] = this->DeltaXYZ[1] = this->DeltaXYZ[2] = 4.0;

  this->IJK[0] = this->IJK[1] = this->IJK[2] = 0;
  this->DeltaIJK[0] = this->DeltaIJK[1] = this->DeltaIJK[2] = 0;
  // so that the SetLabelText macro won't try to free memory
  this->LabelText = NULL;
  this->SetLabelText(""); 
  this->ID = NULL;
  this->SetID("");
  this->Selected = false;
  this->VolumeNodeID = NULL;
  return;
}

//----------------------------------------------------------------------------
vtkMRMLROINode::~vtkMRMLROINode()
{
  if (this->LabelText)
    {
    delete [] this->LabelText;
    this->LabelText = NULL;
    }
  if (this->ID)
    {
    delete [] this->ID;
    this->ID = NULL;
    }
  if (this->VolumeNodeID)
    {
    delete [] this->VolumeNodeID;
    this->VolumeNodeID = NULL;
    }
  return;
}

//----------------------------------------------------------------------------
void vtkMRMLROINode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  //Superclass::WriteXML(of, nIndent);

  //vtkIndent indent(nIndent);

  if (this->ID != NULL) 
    {
    of <<  " ROINodeID " << this->ID;
    }
  if (this->VolumeNodeID != NULL)
    {
    of << " VolumeNodeID " << this->VolumeNodeID;
    }
  if (this->LabelText != NULL)
    {
    of << " Labeltext " << this->LabelText;
    }

  of <<  " XYZ " 
    << this->XYZ[0] << " " << this->XYZ[1] << " " << this->XYZ[2];

  of <<  " DeltaXYZ " 
    << this->DeltaXYZ[0] << " " << this->DeltaXYZ[1] << " " << this->DeltaXYZ[2];
  of << " Selected " << this->Selected;

  return;
}

//----------------------------------------------------------------------------
void vtkMRMLROINode::ReadXMLAttributes( const char** atts)
{
  const char* attName;
  const char* attValue;

  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "XYZ")) 
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++) 
        {
        ss >> val;
        this->XYZ[i] = val;
        }
      }
    if (!strcmp(attName, "DeltaXYZ")) 
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++) 
        {
        ss >> val;
        this->DeltaXYZ[i] = val;
        }
      }
    if (!strcmp(attName, "Selected")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Selected;
      }
    else if (!strcmp(attName, "ROINodeID")) 
      {
      this->SetID(attValue);
      }
    else if (!strcmp(attName, "VolumeNodeID")) 
      {
      this->SetVolumeNodeID(attValue);
      }
    else if (!strcmp(attName, "LabelText"))
      {
      this->SetLabelText(attValue);
      }
    }
  return;
}

void vtkMRMLROINode::ReadXMLString(const char *keyValuePairs)
{
  // used because the ROI list gloms together the point's key and
  // values into one long string, VERY dependent on the order it's written
  // out in when WriteXML is used

  // insert the string into a stream
  std::stringstream ss;
  ss << keyValuePairs;

  char keyName[1024];

  // get out the id
  ss >> keyName;
  ss >> this->ID;
  vtkDebugMacro("ReadXMLString: got id " << this->ID);

  // get out the volume id
  ss >> keyName;
  if (!strcmp(keyName, "VolumeNodeID"))
    {
    char* IDValue = new char[1024];
    ss >> IDValue;
    this->SetVolumeNodeID(IDValue);
    delete [] IDValue;
    vtkDebugMacro("ReadXMLString: got VolumeNodeID " << this->VolumeNodeID);
    }
  else
    {
    // now get the label text value
    ss >> this->LabelText;
    vtkDebugMacro("ReadXMLString: got label text " << this->LabelText);
    }
  
  // get the xyz key
  ss >> keyName;
  // now get the x, y, z values
  ss >> this->XYZ[0];
  ss >> this->XYZ[1];
  ss >> this->XYZ[2];

  // get the Deltaxyz key
  ss >> keyName;
  // now get the x, y, z values
  ss >> this->DeltaXYZ[0];
  ss >> this->DeltaXYZ[1];
  ss >> this->DeltaXYZ[2];

  // get the selected flag
  ss >> keyName;
  ss >> this->Selected;
  return;
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLROINode::Copy(vtkMRMLNode *anode)
{
  //  vtkObject::Copy(anode);
  vtkMRMLROINode *node = (vtkMRMLROINode *) anode;
  this->SetXYZ(node->XYZ);
  this->SetDeltaXYZ(node->DeltaXYZ);
  this->SetLabelText(node->GetLabelText());
  this->SetID(node->ID);
  this->SetSelected(node->GetSelected());

  this->Modified();
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::UpdateReferences()
{
  Superclass::UpdateReferences();
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::ProcessMRMLEvents ( vtkObject *caller,
                                        unsigned long event, 
                                        void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::SetXYZ(float X, float Y, float Z)
{
  this->XYZ[0] = X;
  this->XYZ[1] = Y;
  this->XYZ[2] = Z;

  this->Modified();
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::SetXYZ(float* XYZ)
{
  this->SetXYZ(XYZ[0], XYZ[1], XYZ[2]);
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::SetDeltaXYZ(float DeltaX, float DeltaY, float DeltaZ)
{ 
  this->DeltaXYZ[0] = DeltaX;
  this->DeltaXYZ[1] = DeltaY;
  this->DeltaXYZ[2] = DeltaZ;

  this->Modified();
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::SetDeltaXYZ(float* DeltaXYZ)
{
  this->SetDeltaXYZ(DeltaXYZ[0], DeltaXYZ[1], DeltaXYZ[2]);
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::SetIJK(float I, float J, float K)
{
  this->IJK[0] = I;
  this->IJK[1] = J;
  this->IJK[2] = K;

  //Update  

  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::SetIJK(float* IJK)
{
  this->SetIJK(IJK[0], IJK[1], IJK[2]);
  this->Modified();
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::SetDeltaIJK(float DeltaI, float DeltaJ, float DeltaK)
{
  this->DeltaIJK[0] = DeltaI;
  this->DeltaIJK[1] = DeltaJ;
  this->DeltaIJK[2] = DeltaK;

  //Update

  this->Modified();
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::SetDeltaIJK(float* DeltaIJK)
{
  this->SetDeltaIJK(DeltaIJK[0], DeltaIJK[1], DeltaIJK[2]);
  return;
}

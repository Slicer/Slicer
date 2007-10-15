/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeMathNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLVolumeMathNode.h"
#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkMRMLVolumeMathNode* vtkMRMLVolumeMathNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeMathNode");
  if(ret)
    {
      return (vtkMRMLVolumeMathNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVolumeMathNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLVolumeMathNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeMathNode");
  if(ret)
    {
      return (vtkMRMLVolumeMathNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVolumeMathNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeMathNode::vtkMRMLVolumeMathNode()
{
   this->InputGrayscaleRef = NULL;
   this->InputLabelmapRef = NULL;
   this->ResultText = NULL;
   this->HideFromEditors = true;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeMathNode::~vtkMRMLVolumeMathNode()
{
   this->SetInputGrayscaleRef( NULL );
   this->SetInputLabelmapRef( NULL );
   this->SetResultText( NULL );
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeMathNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream

  vtkIndent indent(nIndent);

  {
    std::stringstream ss;
    if ( this->InputGrayscaleRef )
      {
      ss << this->InputGrayscaleRef;
      of << indent << " InputGrayscaleRef=\"" << ss.str() << "\"";
     }
  }
  {
    std::stringstream ss;
    if ( this->InputLabelmapRef )
      {
      ss << this->InputLabelmapRef;
      of << indent << " InputLabelmapRef=\"" << ss.str() << "\"";
      }
  }
  {
    std::stringstream ss;
    if ( this->ResultText )
      {
      ss << this->ResultText;
      of << indent << " ResultText=\"" << ss.str() << "\"";
      }
  }
  //TODO for labelStats
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeMathNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
  
    if (!strcmp(attName, "InputGrayscaleRef"))
      {
      this->SetInputGrayscaleRef(attValue);
      this->Scene->AddReferencedNodeID(this->InputGrayscaleRef, this);
      }
    else if (!strcmp(attName, "InputLabelmapRef"))
      {
      this->SetInputLabelmapRef(attValue);
      this->Scene->AddReferencedNodeID(this->InputLabelmapRef, this);
      }
    else if (!strcmp(attName, "ResultText"))
      {
      this->SetResultText(attValue);
      this->Scene->AddReferencedNodeID(this->ResultText, this);
      }
    }
  //TODO for labelStats
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLVolumeMathNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLVolumeMathNode *node = (vtkMRMLVolumeMathNode *) anode;

  this->SetInputGrayscaleRef(node->InputGrayscaleRef);
  this->SetInputLabelmapRef(node->InputLabelmapRef);
  this->SetResultText(node->ResultText);
  //TODO for labelStats
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeMathNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "InputGrayscaleRef:   " << 
   (this->InputGrayscaleRef ? this->InputGrayscaleRef : "(none)") << "\n";
  os << indent << "InputLabelmapRef:   " << 
   (this->InputLabelmapRef ? this->InputLabelmapRef : "(none)") << "\n";
  os << indent << "ResultText:   " << 
    (this->ResultText ? this->ResultText : "(none)") << "\n";
  //TODO for labelStats
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeMathNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (!strcmp(oldID, this->InputGrayscaleRef))
    {
    this->SetInputGrayscaleRef(newID);
    }
  if (!strcmp(oldID, this->InputLabelmapRef))
    {
    this->SetInputLabelmapRef(newID);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeMathNode::SaveResultToTextFile( const char *fileName )
{
  std::ofstream myfile;
  myfile.open (fileName);
  myfile << this->GetResultText();
  myfile.close();

}

/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLCameraNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLCameraNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLCameraNode* vtkMRMLCameraNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLCameraNode");
  if(ret)
    {
    return (vtkMRMLCameraNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLCameraNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLCameraNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLCameraNode");
  if(ret)
    {
    return (vtkMRMLCameraNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLCameraNode;
}


//----------------------------------------------------------------------------
vtkMRMLCameraNode::vtkMRMLCameraNode()
{
  this->Position[0] = 0.0;
  this->Position[1] = 0.0;
  this->Position[2] = 0.0;

  this->FocalPoint[0] = 0.0;
  this->FocalPoint[1] = 0.0;
  this->FocalPoint[2] = 0.0;

  this->ViewUp[0] = 0.0;
  this->ViewUp[1] = 1.0;
  this->ViewUp[2] = 0.0;

 }

//----------------------------------------------------------------------------
vtkMRMLCameraNode::~vtkMRMLCameraNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->Position)
    {
    of << indent << " position=\"" << this->Position[0] << " "
      << this->Position[1] << " "
      << this->Position[2] << "\"";
    }
  if (this->FocalPoint)
    {
    of << indent << " focalPoint=\"" << this->FocalPoint[0] << " "
      << this->FocalPoint[1] << " "
      << this->FocalPoint[2] << "\"";
    }
  if (this->ViewUp)
    {
    of << indent << " viewUp=\"" << this->ViewUp[0] << " "
      << this->ViewUp[1] << " "
      << this->ViewUp[2] << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "position")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> Position[0];
      ss >> Position[1];
      ss >> Position[2];
      }
    else if (!strcmp(attName, "focalPoint")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> FocalPoint[0];
      ss >> FocalPoint[1];
      ss >> FocalPoint[2];
      }
    else if (!strcmp(attName, "viewUp")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> ViewUp[0];
      ss >> ViewUp[1];
      ss >> ViewUp[2];
      }

    }  
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLCameraNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLCameraNode *node = (vtkMRMLCameraNode *) anode;


  this->SetPosition(node->Position);
  this->SetFocalPoint(node->FocalPoint);
  this->SetViewUp(node->ViewUp);
}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  Superclass::PrintSelf(os,indent);

  os << "Position:\n";
  for (idx = 0; idx < 2; ++idx)
    {
    os << indent << ", " << this->Position[idx];
    }
  os << "FocalPoint:\n";
  for (idx = 0; idx < 2; ++idx)
    {
    os << indent << ", " << this->FocalPoint[idx];
    }
  os << "ViewUp:\n";
  for (idx = 0; idx < 2; ++idx)
    {
    os << indent << ", " << this->ViewUp[idx];
    }
}

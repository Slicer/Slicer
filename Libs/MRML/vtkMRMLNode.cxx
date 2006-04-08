/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.11 $

=========================================================================auto=*/
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"

#include "vtkObjectFactory.h"

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLNode");
  if(ret)
    {
    return (vtkMRMLNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return NULL;
}

//------------------------------------------------------------------------------
vtkMRMLNode::vtkMRMLNode()
{
  this->ID = NULL;
  
  // By default nodes have no effect on indentation
  this->Indent = 0;
  
  // Strings
  this->Description = NULL;

  // By default all MRML nodes have a blank name
  // Must set name to NULL first so that the SetName
  // macro will not free memory.
  this->Name = NULL;
  this->SetName("");

  this->SceneRootDir = NULL;
  this->Scene = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLNode::~vtkMRMLNode()
{
  this->SetDescription(NULL);
  this->SetName(NULL);
  this->SetID(NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLNode::Copy(vtkMRMLNode *node)
{
  this->SetDescription(node->GetDescription());
  this->SetName(node->GetName());
  this->SetID( node->GetID() );
  
  this->SetScene(node->GetScene());
}

//----------------------------------------------------------------------------
void vtkMRMLNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os,indent);

  os << indent << "ID: " <<
    (this->ID ? this->ID : "(none)") << "\n";
  
  os << indent << "Indent:      " << this->Indent << "\n";
  
  os << indent << "Name: " <<
    (this->Name ? this->Name : "(none)") << "\n";
  
  os << indent << "Description: " <<
    (this->Description ? this->Description : "(none)") << "\n";

}

//----------------------------------------------------------------------------
void vtkMRMLNode::WriteXML(ostream& of, int nIndent)
{
  vtkIndent indent(nIndent);
  if (this->ID != NULL) 
    {
    of << indent << "id=\"" << this->ID << "\" ";
    }
  if (this->Name != NULL) 
    {
    of << indent << "name=\"" << this->Name << "\" ";
    }
  if (this->Description != NULL) 
    {
    of << indent << "description=\"" << this->Description << "\" ";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLNode::ReadXMLAttributes(const char** atts)
{
  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "id")) 
      {
      this->SetID(attValue);
      }
    else if (!strcmp(attName, "name")) 
      {
      this->SetName(attValue);
      }
    else if (!strcmp(attName, "description")) 
      {
      this->SetDescription(attValue);
      }
    } 
  return;
}


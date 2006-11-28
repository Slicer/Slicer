/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLViewNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLViewNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLViewNode* vtkMRMLViewNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLViewNode");
  if(ret)
    {
    return (vtkMRMLViewNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLViewNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLViewNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLViewNode");
  if(ret)
    {
    return (vtkMRMLViewNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLViewNode;
}


//----------------------------------------------------------------------------
vtkMRMLViewNode::vtkMRMLViewNode()
{
  this->BoxVisible = 1;
  this->AxisLabelsVisible = 1;
  this->FieldOfView = 200;
  this->LetterSize = 0.05;
 }

//----------------------------------------------------------------------------
vtkMRMLViewNode::~vtkMRMLViewNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLViewNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " fieldOfView=\"" << this->GetFieldOfView() << "\"";
  of << indent << " letterSize=\"" << this->GetLetterSize() << "\"";
  of << indent << " boxVisible=\"" << (this->BoxVisible ? "true" : "false") << "\"";
  of << indent << " axisLabelsVisible=\"" << (this->AxisLabelsVisible ? "true" : "false") << "\"";

}

//----------------------------------------------------------------------------
void vtkMRMLViewNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "fieldOfView")) 
      {
      std::stringstream ss;
      ss << attValue;
      double fov;
      ss >> fov;
      this->FieldOfView = fov;
      }
    else if (!strcmp(attName, "letterSize")) 
      {
      std::stringstream ss;
      ss << attValue;
      double fov;
      ss >> fov;
      this->LetterSize = fov;
      }

    else if (!strcmp(attName, "boxVisible")) 
      {
      if (!strcmp(attValue,"true")) 
        {
        this->BoxVisible = 1;
        }
      else
        {
        this->BoxVisible = 0;
        }
      }
    else if (!strcmp(attName, "axisLabelsVisible")) 
      {
      if (!strcmp(attValue,"true")) 
        {
        this->AxisLabelsVisible = 1;
        }
      else
        {
        this->AxisLabelsVisible = 0;
        }
      }
    }
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLViewNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLViewNode *node = (vtkMRMLViewNode *) anode;


  this->SetBoxVisible(node->GetBoxVisible());
  this->SetAxisLabelsVisible(node->GetAxisLabelsVisible());
  this->SetFieldOfView(node->GetFieldOfView());
  this->SetLetterSize(node->GetLetterSize());

}

//----------------------------------------------------------------------------
void vtkMRMLViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  Superclass::PrintSelf(os,indent);

  os << indent << "BoxVisible:        " << this->BoxVisible << "\n";
  os << indent << "AxisLabelsVisible: " << this->AxisLabelsVisible << "\n";
  os << indent << "FieldOfView:       " << this->FieldOfView << "\n";
  os << indent << "LetterSize:       " << this->LetterSize << "\n";

}


/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiducialListNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLFiducialListNode* vtkMRMLFiducialListNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiducialListNode");
  if(ret)
    {
    return (vtkMRMLFiducialListNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiducialListNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLFiducialListNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiducialListNode");
  if(ret)
    {
    return (vtkMRMLFiducialListNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiducialListNode;
}


//----------------------------------------------------------------------------
vtkMRMLFiducialListNode::vtkMRMLFiducialListNode()
{

  this->FiducialList = vtkCollection::New();
  this->Indent = 1;
  this->SymbolSize = 6.0;
  this->TextSize = 4.5;
  this->Visibility = 1;
  this->Color[0]=0.4; this->Color[1]=1.0; this->Color[2]=1.0;
  this->Type = NULL;
  this->SetType("default");
}

//----------------------------------------------------------------------------
vtkMRMLFiducialListNode::~vtkMRMLFiducialListNode()
{
  this->FiducialList->Delete();

  if (this->Type) {
    delete [] this->Type;
    this->Type = NULL;
  }
}

//----------------------------------------------------------------------------
void vtkMRMLFiducialListNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);

  of << " type=\"" << this->Type << "\"";
  of << " symbolSize=\"" << this->SymbolSize << "\"";
  of << " textSize=\"" << this->TextSize << "\"";
  of << " visibility=\"" << this->Visibility << "\"";
  
  of << " color=\"" << this->Color[0] << " " << 
                    this->Color[1] << " " <<
                    this->Color[2] << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLFiducialListNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "color")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Color[0];
      ss >> this->Color[1];
      ss >> this->Color[2];
      }
    else if (!strcmp(attName, "type")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Type;
      }
    else if (!strcmp(attName, "symbolSize")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->SymbolSize;
      }
    else if (!strcmp(attName, "textSize")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->TextSize;
      }
    else if (!strcmp(attName, "visibility")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Visibility;
      }
    }  
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLFiducialListNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLNode::Copy(anode);
  vtkMRMLFiducialListNode *node = (vtkMRMLFiducialListNode *) anode;

  this->SetColor(node->Color);
  this->SetSymbolSize(node->SymbolSize);
  this->SetTextSize(node->TextSize);
  this->SetVisibility(node->Visibility);
  this->SetType(node->Type);
}

//----------------------------------------------------------------------------
void vtkMRMLFiducialListNode::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  vtkMRMLNode::PrintSelf(os,indent);

 os << indent << "Symbol size: (";
  os << indent << this->SymbolSize << ") \n ";

  os << indent << "Text size: (";
  os << indent << this->TextSize << ") \n ";

  os << indent << "Visibility:        " << this->Visibility << "\n";
  os << indent << "Type:              " << this->Type << "\n";

  os << "Color:\n";
  for (idx = 0; idx < 3; ++idx)
    {
    os << indent << ", " << this->Color[idx];
    }
}

//-----------------------------------------------------------

void vtkMRMLFiducialListNode::UpdateScene(vtkMRMLScene *scene)
{
}

//-----------------------------------------------------------
vtkMRMLFiducialNode* vtkMRMLFiducialListNode::GetNthFiducialNode(int n)
{

  if(n < 0 || n >= this->FiducialList->GetNumberOfItems()) 
    {
    return NULL;
    }
  else 
    {
    return (vtkMRMLFiducialNode*)this->FiducialList->GetItemAsObject(n);
    }
}

/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFESurfaceNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLFESurfaceNode.h"
#include "vtkMRMLScene.h"
//***
#include "vtkMimxSurfacePolyDataActor.h"

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLFESurfaceNode);

//----------------------------------------------------------------------------
vtkMRMLFESurfaceNode::vtkMRMLFESurfaceNode()
{

}

//----------------------------------------------------------------------------
vtkMRMLFESurfaceNode::~vtkMRMLFESurfaceNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLFESurfaceNode::WriteXML(ostream& of, int nIndent)
{
Superclass::WriteXML(of, nIndent);

  // write a space between previous attribs and these new ones
  of << " ";

  vtkIndent indent(nIndent);
  {
    std::stringstream ss;
    ss << this->actor->GetDataType();
    of << indent << " DataType=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->actor->GetFileName();
    of << indent << " fileName=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->actor->GetFilePath();
    of << indent << " FilePath=\"" << ss.str() << "\"";
  }
}

//----------------------------------------------------------------------------
void vtkMRMLFESurfaceNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  int intAttribute;
  char StringAttribute[1024];
  
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);

     if (!strcmp(attName, "DataType")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> intAttribute;
      this->actor->SetDataType(intAttribute);
      }
    else if (!strcmp(attName, "fileName"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> StringAttribute;
      //***this->SetFileName(StringAttribute);
      //this->fileName(StringAttribute);
      }
    else if (!strcmp(attName, "FilePath"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> StringAttribute;
      this->SetFilePath(StringAttribute);
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLFESurfaceNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLFESurfaceNode *node = (vtkMRMLFESurfaceNode *) anode;

  this->actor->SetDataType(node->GetDataType());
  //***this->actor->SetFileName(node->GetFileName());
  this->actor->SetFilePath(node->GetFilePath());
}

//----------------------------------------------------------------------------
void vtkMRMLFESurfaceNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
    Superclass::PrintSelf(os,indent);

  os << indent << "DataType:   " << this->GetDataType() << "\n";
  os << indent << "FileName:   " << this->GetFileName() << "\n";
  os << indent << "FilePath:   " << this->GetFilePath() << "\n";
}


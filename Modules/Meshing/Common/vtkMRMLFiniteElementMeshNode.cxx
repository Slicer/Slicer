/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiniteElementMeshNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLFiniteElementMeshNode.h"
#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLFiniteElementMeshNode);

//----------------------------------------------------------------------------
vtkMRMLFiniteElementMeshNode::vtkMRMLFiniteElementMeshNode()
{
  this->MimxMeshActor = vtkMimxMeshActor::New();
}

//----------------------------------------------------------------------------
vtkMRMLFiniteElementMeshNode::~vtkMRMLFiniteElementMeshNode()
{
  if (this->MimxMeshActor)
    {
    this->MimxMeshActor->Delete();
    this->MimxMeshActor = NULL;
    }

}

//----------------------------------------------------------------------------
void vtkMRMLFiniteElementMeshNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  
  of << " ";
  vtkIndent indent(nIndent);
  {
    std::stringstream ss;
    ss << this->MimxMeshActor->GetDataType();
    of << indent << " DataType=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->MimxMeshActor->GetFileName();
    of << indent << " fileName=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->MimxMeshActor->GetFilePath();
    of << indent << " FilePath=\"" << ss.str() << "\"";
  }
  {
    std::stringstream ss;
    ss << this->GetSavedVisibilityState();
    of << indent << " savedVisibilityState =\"" << this->savedVisibilityState << "\"";
  }
}

//----------------------------------------------------------------------------
void vtkMRMLFiniteElementMeshNode::ReadXMLAttributes(const char** atts)
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
       this->MimxMeshActor->SetDataType(intAttribute);
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
     else if (!strcmp(attName, "savedVisibilityState")) 
       {
       std::stringstream ss;
       ss << attValue;
       ss >> intAttribute;
       this->SetSavedVisibilityState((intAttribute)?true:false);
       }
     }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLFiniteElementMeshNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLFiniteElementMeshNode *node = (vtkMRMLFiniteElementMeshNode *) anode;

  this->MimxMeshActor->SetDataType(node->GetDataType());
  this->MimxMeshActor->SetFilePath(node->GetFilePath());
}

//----------------------------------------------------------------------------
void vtkMRMLFiniteElementMeshNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
    Superclass::PrintSelf(os,indent);

  os << indent << "DataType:   " << this->GetDataType() << "\n";
  os << indent << "FileName:   " << this->GetFileName() << "\n";
  os << indent << "FilePath:   " << this->GetFilePath() << "\n";


}


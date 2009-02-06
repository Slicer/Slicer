/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
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
vtkMRMLFiniteElementMeshNode* vtkMRMLFiniteElementMeshNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiniteElementMeshNode");
  if(ret)
    {
      return (vtkMRMLFiniteElementMeshNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  vtkMimxMeshActor* newactor =  vtkMimxMeshActor::New();
  vtkMRMLFiniteElementMeshNode* newnode = new vtkMRMLFiniteElementMeshNode;
  newnode->SetMimxMeshActor(newactor);
  return newnode;
}

//----------------------------------------------------------------------------

vtkMRMLFiniteElementMeshNode* vtkMRMLFiniteElementMeshNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiniteElementMeshNode");
  if(ret)
    {
      return (vtkMRMLFiniteElementMeshNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  vtkMimxMeshActor* newactor =  vtkMimxMeshActor::New();
   vtkMRMLFiniteElementMeshNode* newnode = new vtkMRMLFiniteElementMeshNode;
   newnode->SetMimxMeshActor(newactor);
   return newnode;
}

//----------------------------------------------------------------------------
vtkMRMLFiniteElementMeshNode::vtkMRMLFiniteElementMeshNode()
{

}

//----------------------------------------------------------------------------
vtkMRMLFiniteElementMeshNode::~vtkMRMLFiniteElementMeshNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLFiniteElementMeshNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  
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
     else if (!strcmp(attName, "savedVisibilityState")) 
       {
       std::stringstream ss;
       ss << attValue;
       ss >> intAttribute;
       this->SetSavedVisibilityState(intAttribute);
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

  this->actor->SetDataType(node->GetDataType());
  //***this->actor->SetFileName(node->GetFileName());
  this->actor->SetFilePath(node->GetFilePath());
}

//----------------------------------------------------------------------------
void vtkMRMLFiniteElementMeshNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
    Superclass::PrintSelf(os,indent);

  os << indent << "DataType:   " << this->GetDataType() << "\n";
  os << indent << "FileName:   " << this->GetFileName() << "\n";
  os << indent << "FilePath:   " << this->GetFilePath() << "\n";


}


/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiniteElementBuildingBlockNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLFiniteElementBuildingBlockNode.h"
#include "vtkMRMLScene.h"



//------------------------------------------------------------------------------
vtkMRMLFiniteElementBuildingBlockNode* vtkMRMLFiniteElementBuildingBlockNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiniteElementBuildingBlockNode");
  if(ret)
    {
      return (vtkMRMLFiniteElementBuildingBlockNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.  
  vtkMRMLFiniteElementBuildingBlockNode* newnode = new vtkMRMLFiniteElementBuildingBlockNode;
  return newnode;
}

//----------------------------------------------------------------------------

vtkMRMLFiniteElementBuildingBlockNode* vtkMRMLFiniteElementBuildingBlockNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiniteElementBuildingBlockNode");
  if(ret)
    {
      return (vtkMRMLFiniteElementBuildingBlockNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
   vtkMRMLFiniteElementBuildingBlockNode* newnode = new vtkMRMLFiniteElementBuildingBlockNode;
   return newnode;
}

//----------------------------------------------------------------------------
vtkMRMLFiniteElementBuildingBlockNode::vtkMRMLFiniteElementBuildingBlockNode()
{
  this->MimxUnstructuredGridActor = vtkMimxUnstructuredGridActor::New();
}

//----------------------------------------------------------------------------
vtkMRMLFiniteElementBuildingBlockNode::~vtkMRMLFiniteElementBuildingBlockNode()
{
  if (this->MimxUnstructuredGridActor)
    {
    this->MimxUnstructuredGridActor->Delete();
    this->MimxUnstructuredGridActor = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFiniteElementBuildingBlockNode::WriteXML(ostream& of, int nIndent)
{
Superclass::WriteXML(of, nIndent);
    
    of << " ";
    vtkIndent indent(nIndent);
    {
      std::stringstream ss;
      ss << this->MimxUnstructuredGridActor->GetDataType();
      of << indent << " DataType=\"" << ss.str() << "\"";
    }
    {
      std::stringstream ss;
      ss << this->MimxUnstructuredGridActor->GetFileName();
      of << indent << " fileName=\"" << ss.str() << "\"";
    }
//    {
//      std::stringstream ss;
//      ss << this->MimxUnstructuredGridActor->GetFilePath();
//      of << indent << " FilePath=\"" << ss.str() << "\"";
//    }
}

//----------------------------------------------------------------------------
void vtkMRMLFiniteElementBuildingBlockNode::ReadXMLAttributes(const char** atts)
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
      this->MimxUnstructuredGridActor->SetDataType(intAttribute);
      }
    else if (!strcmp(attName, "fileName"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> StringAttribute;
      //***this->SetFileName(StringAttribute);
      //this->fileName(StringAttribute);
      }
//    else if (!strcmp(attName, "FilePath"))
//      {
//      std::stringstream ss;
//      ss << attValue;
//      ss >> StringAttribute;
//      this->SetFilePath(StringAttribute);
//      }
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLFiniteElementBuildingBlockNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLFiniteElementBuildingBlockNode *node = (vtkMRMLFiniteElementBuildingBlockNode *) anode;

  this->MimxUnstructuredGridActor->SetDataType(node->GetDataType());
  //***this->MimxUnstructuredGridActor->SetFileName(node->GetFileName());
  //this->MimxUnstructuredGridActor->SetFilePath(node->GetFilePath());
}

//----------------------------------------------------------------------------
void vtkMRMLFiniteElementBuildingBlockNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

   os << indent << "DataType:   " << this->GetDataType() << "\n";
   os << indent << "FileName:   " << this->GetFileName() << "\n";
  // os << indent << "FilePath:   " << this->GetFilePath() << "\n";

}


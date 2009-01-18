/*=Auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumePropertyStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkStringArray.h"

#include "vtkMRMLVolumePropertyNode.h"
#include "vtkMRMLVolumePropertyStorageNode.h"
#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkMRMLVolumePropertyStorageNode* vtkMRMLVolumePropertyStorageNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumePropertyStorageNode");
  if(ret)
    {
    return (vtkMRMLVolumePropertyStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVolumePropertyStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLVolumePropertyStorageNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumePropertyStorageNode");
  if(ret)
    {
    return (vtkMRMLVolumePropertyStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVolumePropertyStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyStorageNode::vtkMRMLVolumePropertyStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyStorageNode::~vtkMRMLVolumePropertyStorageNode()
{
}

void vtkMRMLVolumePropertyStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyStorageNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLStorageNode::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLVolumePropertyStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLStorageNode::PrintSelf(os,indent);

  os << indent << "FileName: " <<
    (this->FileName ? this->FileName : "(none)") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

//----------------------------------------------------------------------------
int vtkMRMLVolumePropertyStorageNode::ReadData(vtkMRMLNode *refNode)
{
  // do not read if if we are not in the scene (for example inside snapshot)
  if (  !refNode->GetAddToScene() )
    {
    return 1;
    }

  if (!refNode->IsA("vtkMRMLVolumePropertyNode") ) 
    {
    //vtkErrorMacro("Reference node is not a vtkMRMLVolumePropertyNode");
    return 0;
    }

  Superclass::StageReadData(refNode);
  if ( this->GetReadState() != this->TransferDone )
    {
    // remote file download hasn't finished
    return 0;
    }
  
  vtkMRMLVolumePropertyNode *transformNode = dynamic_cast <vtkMRMLVolumePropertyNode *> (refNode);

  std::string fullName = this->GetFullNameFromFileName(); 
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  int result = 1;

  this->SetReadStateIdle();
   
  return result;
}

//----------------------------------------------------------------------------
int vtkMRMLVolumePropertyStorageNode::WriteData(vtkMRMLNode *refNode)
{
  // test whether refNode is a valid node to hold a transform
  if (!refNode->IsA("vtkMRMLTransformNode") ) 
    {
    vtkErrorMacro("Reference node is not a vtkMRMLTransformNode");
    return 0;
    }
  
  //vtkMRMLTransformNode *transformNode = vtkMRMLTransformNode::SafeDownCast(refNode);
  
  std::string fullName =  this->GetFullNameFromFileName();
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("vtkMRMLTransformNode: File name not specified");
    return 0;
    }

  int result =1;
  return result;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("VolumePropperty (.vp)");
  this->SupportedWriteFileTypes->InsertNextValue("Text (.txt)");
  this->SupportedWriteFileTypes->InsertNextValue("VolumePropperty (.*)");
}

//----------------------------------------------------------------------------
int vtkMRMLVolumePropertyStorageNode::SupportedFileType(const char *fileName)
{
  // check to see which file name we need to check
  std::string name;
  if (fileName)
  {
    name = std::string(fileName);
  }
  else if (this->FileName != NULL)
  {
    name = std::string(this->FileName);
  }
  else if (this->URI != NULL)
  {
    name = std::string(this->URI);
  }
  else
  {
    vtkWarningMacro("SupportedFileType: no file name to check");
    return 0;
  }

  return 1;
}

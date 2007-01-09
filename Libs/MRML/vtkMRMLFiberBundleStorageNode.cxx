/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiberBundleStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLFiberBundleStorageNode.h"
#include "vtkMRMLScene.h"

#include "vtkPolyDataReader.h"
#include "vtkPolyDataWriter.h"


//------------------------------------------------------------------------------
vtkMRMLFiberBundleStorageNode* vtkMRMLFiberBundleStorageNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiberBundleStorageNode");
  if(ret)
    {
    return (vtkMRMLFiberBundleStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiberBundleStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLFiberBundleStorageNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiberBundleStorageNode");
  if(ret)
    {
    return (vtkMRMLFiberBundleStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiberBundleStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLFiberBundleStorageNode::vtkMRMLFiberBundleStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLFiberBundleStorageNode::~vtkMRMLFiberBundleStorageNode()
{
}

void vtkMRMLFiberBundleStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleStorageNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLStorageNode::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLFiberBundleStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLStorageNode::PrintSelf(os,indent);

  os << indent << "FileName: " <<
    (this->FileName ? this->FileName : "(none)") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

//----------------------------------------------------------------------------
int vtkMRMLFiberBundleStorageNode::ReadData(vtkMRMLNode *refNode)
{
  if (!refNode->IsA("vtkMRMLFiberBundleNode") ) 
    {
    vtkErrorMacro("Reference node is not a vtkMRMLFiberBundleNode");
    return 0;
    }

  vtkMRMLFiberBundleNode *fiberBundleNode = dynamic_cast <vtkMRMLFiberBundleNode *> (refNode);

  std::string fullName;
  if (this->SceneRootDir != NULL && this->Scene->IsFilePathRelative(this->GetFileName())) 
    {
    fullName = std::string(this->SceneRootDir) + std::string(this->GetFileName());
    }
  else 
    {
    fullName = std::string(this->GetFileName());
    }
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("vtkMRMLFiberBundleNode: File name not specified");
    return 0;
    }

  // compute file prefix
  std::string name(fullName);
  std::string::size_type loc = name.find(".");
  if( loc == std::string::npos ) 
    {
    vtkErrorMacro("vtkMRMLFiberBundleNode: no file extention specified");
    }
  std::string extension = name.substr(loc);

  if (fiberBundleNode->GetPolyData()) 
    {
    fiberBundleNode->GetPolyData()->Delete();
    fiberBundleNode->SetAndObservePolyData (NULL);
    }

  int result = 1;
  try
    {
    if (extension == std::string(".vtk")) 
      {
      vtkPolyDataReader *reader = vtkPolyDataReader::New();
      reader->SetFileName(fullName.c_str());
      reader->Update();
      fiberBundleNode->SetAndObservePolyData(reader->GetOutput());
      reader->Delete();
      }  
    else 
      {
      vtkErrorMacro("Cannot read fiberBundle file '" << name.c_str() << "'");
      return 0;
      }
    }
  catch (vtkstd::exception &e)
    {
    result = 0;
    }

  if (fiberBundleNode->GetPolyData() != NULL) 
    {
    fiberBundleNode->GetPolyData()->Modified();
    }
  fiberBundleNode->SetModifiedSinceRead(0);
  return result;
}

//----------------------------------------------------------------------------
int vtkMRMLFiberBundleStorageNode::WriteData(vtkMRMLNode *refNode)
{
  // test whether refNode is a valid node to hold a fiberBundle
  if (!refNode->IsA("vtkMRMLFiberBundleNode") ) 
    {
    vtkErrorMacro("Reference node is not a vtkMRMLFiberBundleNode");
    return 0;
    }
  
  vtkMRMLFiberBundleNode *fiberBundleNode = vtkMRMLFiberBundleNode::SafeDownCast(refNode);
  
  std::string fullName;
  if (this->SceneRootDir != NULL && this->Scene->IsFilePathRelative(this->GetFileName())) 
    {
    fullName = std::string(this->SceneRootDir) + std::string(this->GetFileName());
    }
  else 
    {
    fullName = std::string(this->GetFileName());
    }  
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("vtkMRMLFiberBundleNode: File name not specified");
    return 0;
    }

  vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
  writer->SetFileName(fullName.c_str());
  writer->SetInput( fiberBundleNode->GetPolyData() );

  int result = 1;
  try
    {
    writer->Write();
    }
  catch (vtkstd::exception &e)
    {
    result = 0;
    }
  writer->Delete();    
  
  return 1;
}

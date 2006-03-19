/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLModelStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <ostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLModelStorageNode.h"

#include "vtkMatrix4x4.h"

#include "vtkBYUReader.h" 
#include "vtkPolyDataReader.h"
#include "vtkSTLReader.h"
//TODO: read in a free surfer file
//#include "vtkFSSurfaceReader.h"



// Initialize static member that controls resampling -- 
// old comment: "This offset will be changed to 0.5 from 0.0 per 2/8/2002 Slicer 
// development meeting, to move ijk coordinates to voxel centers."


//------------------------------------------------------------------------------
vtkMRMLModelStorageNode* vtkMRMLModelStorageNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLModelStorageNode");
  if(ret)
    {
      return (vtkMRMLModelStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLModelStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLModelStorageNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLModelStorageNode");
  if(ret)
    {
      return (vtkMRMLModelStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLModelStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLModelStorageNode::vtkMRMLModelStorageNode()
{
  this->FileName = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLModelStorageNode::~vtkMRMLModelStorageNode()
{
  if (this->FileName) {
    delete [] this->FileName;
    this->FileName = NULL;
  }
}

void vtkMRMLModelStorageNode::WriteXML(ostream& of, int nIndent)
{
  vtkErrorMacro("NOT IMPLEMENTED YET");
  (void)of; (void)nIndent;
}

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLStorageNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "FileName")) {
      this->SetFileName(attValue);
    }
  }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLModelStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLModelStorageNode *node = (vtkMRMLModelStorageNode *) anode;

  this->SetFileName(node->FileName);
}

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLStorageNode::PrintSelf(os,indent);

  os << indent << "FileName: " <<
    (this->FileName ? this->FileName : "(none)") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::ReadData(vtkMRMLNode *refNode)
{
  if (!refNode->IsA("vtkMRMLModelNode") ) {
    vtkErrorMacro("Reference node is not a vtkMRMLModelNode");
    return;
  }

  vtkMRMLModelNode *modelNode = dynamic_cast <vtkMRMLModelNode *> (refNode);

  if (modelNode->GetPolyData()) {
    modelNode->GetPolyData()->Delete();
    modelNode->SetPolyData (NULL);
  }

  std::string fullName;
  if (this->SceneRootDir != NULL) {
    fullName = std::string(this->SceneRootDir) + std::string(this->GetFileName());
  }
  else {
    fullName = std::string(this->GetFileName());
  }

  if (fullName == std::string("")) {
    vtkErrorMacro("vtkMRMLModelNode: File name not specified");
  }

  // compute file prefix
  std::string name(fullName);
  std::string::size_type loc = name.find(".");
  if( loc == std::string::npos ) {
    vtkErrorMacro("vtkMRMLModelNode: no file extention specified");
  }
  std::string extention = name.substr(loc);
  
  if ( extention == std::string(".g")) {
    vtkBYUReader *reader = vtkBYUReader::New();
    reader->SetGeometryFileName(fullName.c_str());
    reader->Update();
    modelNode->SetPolyData(reader->GetOutput());
  }
  else if (extention == std::string(".vtk")) {
    vtkPolyDataReader *reader = vtkPolyDataReader::New();
    reader->SetFileName(fullName.c_str());
    reader->Update();
    modelNode->SetPolyData(reader->GetOutput());
  }  
  else if ( extention == std::string(".orig") ||
            extention == std::string(".inflated") || 
            extention == std::string(".pial") ) {
    //TODO: read in a free surfer file
    //vtkFSSurfaceReader *reader = vtkFSSurfaceReader::New();
    //reader->SetFileName(fullName.c_str());
    //reader->Update();
    //modelNode->SetPolyData(reader->GetOutput());
  }  
  else if (extention == std::string(".stl")) {
    vtkSTLReader *reader = vtkSTLReader::New();
    reader->SetFileName(fullName.c_str());
    modelNode->SetPolyData(reader->GetOutput());
    reader->Update();
  }
}

void vtkMRMLModelStorageNode::WriteData(vtkMRMLNode *refNode)
{
  vtkErrorMacro("NOT IMPLEMENTED YET");
}

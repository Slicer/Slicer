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
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLScene.h"

#include "vtkMatrix4x4.h"
#include "vtkPolyDataNormals.h"
#include "vtkStripper.h"

#include "vtkBYUReader.h" 
#include "vtkPolyDataReader.h"
#include "vtkSTLReader.h"
//TODO: read in a free surfer file
#include "vtkFSSurfaceReader.h"
#include "vtkPolyDataWriter.h"


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
}

//----------------------------------------------------------------------------
vtkMRMLModelStorageNode::~vtkMRMLModelStorageNode()
{
}

void vtkMRMLModelStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLStorageNode::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLModelStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
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
int vtkMRMLModelStorageNode::ReadData(vtkMRMLNode *refNode)
{
  if (!refNode->IsA("vtkMRMLModelNode") ) 
    {
    vtkErrorMacro("Reference node is not a vtkMRMLModelNode");
    return 0;
    }

  vtkMRMLModelNode *modelNode = dynamic_cast <vtkMRMLModelNode *> (refNode);

  if (modelNode->GetPolyData()) 
    {
    modelNode->GetPolyData()->Delete();
    modelNode->SetAndObservePolyData (NULL);
    }

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
    vtkErrorMacro("vtkMRMLModelNode: File name not specified");
    return 0;
    }

  // compute file prefix
  std::string name(fullName);
  std::string::size_type loc = name.find(".");
  if( loc == std::string::npos ) 
    {
    vtkErrorMacro("vtkMRMLModelNode: no file extention specified");
    }
  std::string extention = name.substr(loc);
  
  if ( extention == std::string(".g")) 
    {
    vtkBYUReader *reader = vtkBYUReader::New();
    reader->SetGeometryFileName(fullName.c_str());
    reader->Update();
    modelNode->SetAndObservePolyData(reader->GetOutput());
    reader->Delete();
    }
  else if (extention == std::string(".vtk")) 
    {
    vtkPolyDataReader *reader = vtkPolyDataReader::New();
    reader->SetFileName(fullName.c_str());
    reader->Update();
    modelNode->SetAndObservePolyData(reader->GetOutput());
    reader->Delete();
    }  
  else if ( extention == std::string(".orig") ||
            extention == std::string(".inflated") ||
            extention == std::string(".sphere") ||
            extention == std::string(".white") ||
            extention == std::string(".smoothwm") ||
            extention == std::string(".pial") ) 
    {
    //read in a free surfer file
    // -- create normals and triangle strips also
    vtkFSSurfaceReader *reader = vtkFSSurfaceReader::New();
    vtkPolyDataNormals *normals = vtkPolyDataNormals::New();
    vtkStripper *stripper = vtkStripper::New();

    reader->SetFileName(fullName.c_str());
    normals->SetSplitting(0);
    normals->SetInput( reader->GetOutput() );
    stripper->SetInput( normals->GetOutput() );
    stripper->Update();
    modelNode->SetAndObservePolyData(stripper->GetOutput());

    reader->Delete();
    normals->Delete();
    stripper->Delete();
    }  
  else if (extention == std::string(".stl")) 
    {
    vtkSTLReader *reader = vtkSTLReader::New();
    reader->SetFileName(fullName.c_str());
    modelNode->SetAndObservePolyData(reader->GetOutput());
    reader->Update();
    reader->Delete();
    }
  else 
    {
    vtkErrorMacro("Cannot read model file '" << name.c_str() << "'");
    return 0;
    }
    
  if (modelNode->GetPolyData() != NULL) 
    {
    modelNode->GetPolyData()->Modified();
    }
  modelNode->SetModifiedSinceRead(0);
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLModelStorageNode::WriteData(vtkMRMLNode *refNode)
{
  // test whether refNode is a valid node to hold a model
  if (!refNode->IsA("vtkMRMLModelNode") ) 
    {
    vtkErrorMacro("Reference node is not a vtkMRMLModelNode");
    return 0;
    }
  
  vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(refNode);
  
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
    vtkErrorMacro("vtkMRMLModelNode: File name not specified");
    return 0;
    }

  vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
  writer->SetFileName(fullName.c_str());
  writer->SetInput( modelNode->GetPolyData() );

  writer->Write();

  writer->Delete();    
  
  return 1;
}

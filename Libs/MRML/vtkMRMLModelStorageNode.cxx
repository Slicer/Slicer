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
#include "vtkXMLPolyDataReader.h"
#include "vtkSTLReader.h"
#include "vtkMRMLColorTableNode.h"

#include "vtkPolyDataWriter.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkSTLWriter.h"

#include "vtkPointData.h"

#include "itksys/SystemTools.hxx"

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
  // do not read if if we are not in the scene (for example inside snapshot)
  if ( !this->GetAddToScene() || !refNode->GetAddToScene() )
    {
    return 1;
    }
  
  if (!refNode->IsA("vtkMRMLModelNode") ) 
    {
    //vtkErrorMacro("Reference node is not a vtkMRMLModelNode");
    return 0;
    }

  Superclass::StageReadData(refNode);
  if ( this->GetReadState() != this->TransferDone )
    {
    // remote file download hasn't finished
    return 0;
    }
  
  vtkMRMLModelNode *modelNode = dynamic_cast <vtkMRMLModelNode *> (refNode);

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  // compute file prefix
  std::string name(fullName);
  std::string::size_type loc = name.find_last_of(".");
  if( loc == std::string::npos ) 
    {
    vtkErrorMacro("ReadData: no file extension specified: " << name.c_str());
    return 0;
    }
  std::string extension = name.substr(loc);

  vtkDebugMacro("ReadData: extension = " << extension.c_str());

  int result = 1;
  try
    {
    if ( extension == std::string(".g")) 
      {
      vtkBYUReader *reader = vtkBYUReader::New();
      reader->SetGeometryFileName(fullName.c_str());
      reader->Update();
      modelNode->SetAndObservePolyData(reader->GetOutput());
      reader->Delete();
      }
    else if (extension == std::string(".vtk")) 
      {
      vtkPolyDataReader *reader = vtkPolyDataReader::New();
      reader->SetFileName(fullName.c_str());
      if (!reader->IsFilePolyData())
        {
        vtkErrorMacro("File " << fullName.c_str() << " is not polydata, cannot be read with this reader");
        result = 0;
        }
      else
        {
        reader->Update();
        if (reader->GetOutput() == NULL)
          {
          vtkErrorMacro("Unable to read file " << fullName.c_str());
          result = 0;
          }
        else
          {
          modelNode->SetAndObservePolyData(reader->GetOutput());
          }
        }
      reader->Delete();
      }  
    else if (extension == std::string(".vtp")) 
      {
      vtkXMLPolyDataReader *reader = vtkXMLPolyDataReader::New();
      reader->SetFileName(fullName.c_str());
      reader->Update();
      modelNode->SetAndObservePolyData(reader->GetOutput());
      reader->Delete();
      }  
    else if (extension == std::string(".stl")) 
      {
      vtkSTLReader *reader = vtkSTLReader::New();
      reader->SetFileName(fullName.c_str());
      modelNode->SetAndObservePolyData(reader->GetOutput());
      reader->Update();
      reader->Delete();
      }
    else 
      {
      vtkDebugMacro("Cannot read model file '" << name.c_str() << "' (extension = " << extension.c_str() << ")");
      return 0;
      }
    }
  catch (...)
    {
    result = 0;
    }

  this->SetReadStateIdle();
  if (modelNode->GetPolyData() != NULL) 
    {
    modelNode->GetPolyData()->Modified();
    }
  modelNode->SetModifiedSinceRead(0);
  return result;
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
  
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("vtkMRMLModelNode: File name not specified");
    return 0;
    }

  std::string extension = itksys::SystemTools::GetFilenameLastExtension(fullName);

  int result = 1;
  if (extension == ".vtk")
    {
    vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
    writer->SetFileName(fullName.c_str());
    writer->SetInput( modelNode->GetPolyData() );
    try
      {
      writer->Write();
      }
    catch (...)
      {
      result = 0;
      }
    writer->Delete();    
    }
  else if (extension == ".vtp")
    {
    vtkXMLPolyDataWriter *writer = vtkXMLPolyDataWriter::New();
    writer->SetFileName(fullName.c_str());
    writer->SetInput( modelNode->GetPolyData() );
    try
      {
      writer->Write();
      }
    catch (...)
      {
      result = 0;
      }
    writer->Delete();    
    }
  else if (extension == ".stl")
    {
    vtkSTLWriter *writer = vtkSTLWriter::New();
    writer->SetFileName(fullName.c_str());
    writer->SetInput( modelNode->GetPolyData() );
    try
      {
      writer->Write();
      }
    catch (...)
      {
      result = 0;
      }
    writer->Delete();    
    }
  else
    {
    result = 0;
    vtkErrorMacro( << "No file extension recognized: " << fullName.c_str() );
    }

  if (result != 0)
    {
    this->StageWriteData(refNode);
    }
  
  return result;
}

//----------------------------------------------------------------------------
int vtkMRMLModelStorageNode::SupportedFileType(const char *fileName)
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
  
  std::string::size_type loc = name.find_last_of(".");
  if( loc == std::string::npos ) 
    {
    vtkErrorMacro("SupportedFileType: no file extension specified");
    return 0;
    }
  std::string extension = name.substr(loc);

  vtkDebugMacro("SupportedFileType: extension = " << extension.c_str());
  if (extension.compare(".vtk") == 0 ||
      extension.compare(".vtp") == 0 ||
      extension.compare(".g") == 0 ||
      extension.compare(".stl") == 0 )
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

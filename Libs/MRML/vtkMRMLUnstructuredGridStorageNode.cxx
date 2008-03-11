/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLUnstructuredGridStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "itksys/SystemTools.hxx"

#include "vtkObjectFactory.h"
#include "vtkMRMLUnstructuredGridStorageNode.h"
#include "vtkMRMLScene.h"

#include "vtkUnstructuredGridReader.h"
#include "vtkUnstructuredGridWriter.h"


//------------------------------------------------------------------------------
vtkMRMLUnstructuredGridStorageNode* vtkMRMLUnstructuredGridStorageNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLUnstructuredGridStorageNode");
  if(ret)
    {
    return (vtkMRMLUnstructuredGridStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLUnstructuredGridStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLUnstructuredGridStorageNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLUnstructuredGridStorageNode");
  if(ret)
    {
    return (vtkMRMLUnstructuredGridStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLUnstructuredGridStorageNode;
}


//----------------------------------------------------------------------------
void vtkMRMLUnstructuredGridStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

//----------------------------------------------------------------------------
int vtkMRMLUnstructuredGridStorageNode::ReadData(vtkMRMLNode *refNode)
{
  if (!refNode->IsA("vtkMRMLUnstructuredGridNode") ) 
    {
    vtkErrorMacro("Reference node is not a vtkMRMLUnstructuredGridNode");
    return 0;
    }

  Superclass::StageReadData(refNode);
  if ( this->GetReadState() == this->Pending )
    {
    // remote file download hasn't finished
    return 0;
    }
  
  vtkMRMLUnstructuredGridNode *modelNode = vtkMRMLUnstructuredGridNode::SafeDownCast (refNode);

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  // compute file prefix
  std::string name(fullName);
  std::string::size_type loc = name.find(".");
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
    if (extension == std::string(".vtk")) 
      {
      vtkUnstructuredGridReader *reader = vtkUnstructuredGridReader::New();
      reader->SetFileName(fullName.c_str());
      reader->Update();
      if (reader->GetOutput() == NULL)
        {
        vtkErrorMacro("Unable to read file " << fullName.c_str());
        result = 0;
        }
      else
        {
        modelNode->SetAndObserveUnstructuredGrid(reader->GetOutput());
        }
      reader->Delete();
      }  
    else 
      {
      vtkWarningMacro("Cannot read model file '" << name.c_str() << "' (extension = " << extension.c_str() << ")");
      return 0;
      }
    }
  catch (...)
    {
    result = 0;
    }

  if (modelNode->GetUnstructuredGrid() != NULL) 
    {
    modelNode->GetUnstructuredGrid()->Modified();
    }
  modelNode->SetModifiedSinceRead(0);
  return result;
}


//----------------------------------------------------------------------------
int vtkMRMLUnstructuredGridStorageNode::WriteData(vtkMRMLNode *refNode)
{
  // test whether refNode is a valid node to hold a model
  if (!refNode->IsA("vtkMRMLUnstructuredGridNode") ) 
    {
    vtkErrorMacro("Reference node is not a vtkMRMLUnstructuredGridNode");
    return 0;
    }
  
  vtkMRMLUnstructuredGridNode *modelNode = vtkMRMLUnstructuredGridNode::SafeDownCast(refNode);
  
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
    vtkUnstructuredGridWriter *writer = vtkUnstructuredGridWriter::New();
    writer->SetFileName(fullName.c_str());
    writer->SetInput( modelNode->GetUnstructuredGrid() );
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
    Superclass::StageWriteData(refNode);
    }
  
  return result;
}

//----------------------------------------------------------------------------
int vtkMRMLUnstructuredGridStorageNode::SupportedFileType(const char *fileName)
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
  if (extension.compare(".vtk") == 0)
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

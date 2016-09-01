/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLUnstructuredGridStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkMRMLUnstructuredGridNode.h"
#include "vtkMRMLUnstructuredGridStorageNode.h"
#include "vtkMRMLScene.h"

#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkUnstructuredGridWriter.h"
#include <vtkVersion.h>

#include "itksys/SystemTools.hxx"


//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLUnstructuredGridStorageNode);

//----------------------------------------------------------------------------
vtkMRMLUnstructuredGridStorageNode::vtkMRMLUnstructuredGridStorageNode()
{
  this->DefaultWriteFileExtension = "vtk";
}

//----------------------------------------------------------------------------
void vtkMRMLUnstructuredGridStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLUnstructuredGridStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLUnstructuredGridNode");
}

//----------------------------------------------------------------------------
int vtkMRMLUnstructuredGridStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  int result = 0;
  vtkMRMLUnstructuredGridNode *modelNode = vtkMRMLUnstructuredGridNode::SafeDownCast (refNode);

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("ReadData: File name not specified");
    return result;
    }

  // compute file prefix
  std::string extension=vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  if( extension.empty() )
    {
    vtkErrorMacro("ReadData: no file extension specified: " << fullName.c_str());
    return result;
    }

  vtkDebugMacro("ReadData: extension = " << extension.c_str());

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
        }
      else
        {
        modelNode->SetAndObserveUnstructuredGrid(reader->GetOutput());
        result = 1;
        }
      reader->Delete();
      }
    else
      {
      vtkWarningMacro("Cannot read model file '" << fullName.c_str() << "' (extension = " << extension.c_str() << ")");
      }
    }
  catch (...)
    {
    }

  if (modelNode->GetUnstructuredGrid() != NULL)
    {
    //modelNode->GetUnstructuredGrid()->Modified();
    }
  return result;
}


//----------------------------------------------------------------------------
int vtkMRMLUnstructuredGridStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLUnstructuredGridNode *modelNode = vtkMRMLUnstructuredGridNode::SafeDownCast(refNode);

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLModelNode: File name not specified");
    return 0;
    }

  std::string extension=vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);

  int result = 1;
  if (extension == ".vtk")
    {
    vtkUnstructuredGridWriter *writer = vtkUnstructuredGridWriter::New();
    writer->SetFileName(fullName.c_str());
    writer->SetInputData( modelNode->GetUnstructuredGrid() );
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
void vtkMRMLUnstructuredGridStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue(
    "Unstructured Grid (.vtk)");
}

//----------------------------------------------------------------------------
void vtkMRMLUnstructuredGridStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue(
    "Unstructured Grid (.vtk)");
}

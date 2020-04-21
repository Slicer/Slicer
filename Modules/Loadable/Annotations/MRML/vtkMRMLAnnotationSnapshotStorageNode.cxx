/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAnnotationSnapshotStorageNode.cxx,v $
  Date:      $Date: 2006/03/17 15:10:09 $
  Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkMRMLAnnotationSnapshotNode.h"
#include "vtkMRMLAnnotationSnapshotStorageNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkBMPReader.h>
#include <vtkBMPWriter.h>
#include <vtkImageData.h>
#include <vtkJPEGReader.h>
#include <vtkJPEGWriter.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPNGReader.h>
#include <vtkPNGWriter.h>
#include <vtkStringArray.h>
#include <vtkTIFFReader.h>
#include <vtkTIFFWriter.h>
#include <vtkVersion.h>

// ITKsys includes
#include <itksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnnotationSnapshotStorageNode);

//----------------------------------------------------------------------------
vtkMRMLAnnotationSnapshotStorageNode::vtkMRMLAnnotationSnapshotStorageNode()
{
  this->DefaultWriteFileExtension = "png";
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationSnapshotStorageNode::~vtkMRMLAnnotationSnapshotStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLAnnotationSnapshotStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLAnnotationSnapshotStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLAnnotationSnapshotNode");
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationSnapshotStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLAnnotationSnapshotNode *sceneViewNode =
    vtkMRMLAnnotationSnapshotNode::SafeDownCast(refNode);

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  // compute file prefix
  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  if( extension.empty() )
    {
    vtkErrorMacro("ReadData: no file extension specified: " << fullName.c_str());
    return 0;
    }

  vtkDebugMacro("ReadData: extension = " << extension.c_str());

  int result = 1;
  vtkNew<vtkImageData> imageData;

  try
    {
    if ( extension == std::string(".png") )
      {
      vtkNew<vtkPNGReader> reader;
      reader->SetFileName(fullName.c_str());
      reader->Update();
      if (reader->GetOutput())
        {
        vtkDebugMacro("ReadData: read file, copying output to image data");
        imageData->DeepCopy(reader->GetOutput());
        }
      }
    else if (extension == std::string(".jpg") ||
             extension == std::string(".jpeg"))
      {
      vtkNew<vtkJPEGReader> reader;
      reader->SetFileName(fullName.c_str());
      reader->Update();
      if (reader->GetOutput())
        {
        imageData->DeepCopy(reader->GetOutput());
        }
      }
    else if (extension == std::string(".tiff"))
      {
      vtkNew<vtkTIFFReader> reader;
      reader->SetFileName(fullName.c_str());
      reader->Update();
      if (reader->GetOutput())
        {
        imageData->DeepCopy(reader->GetOutput());
        }
      }
    else if (extension == std::string(".bmp"))
      {
      vtkNew<vtkBMPReader> reader;
      reader->SetFileName(fullName.c_str());
      reader->Update();
      if (reader->GetOutput())
        {
        imageData->DeepCopy(reader->GetOutput());
        }
      }
    else
      {
      vtkDebugMacro("Cannot read scene view file '" << fullName.c_str() << "' (extension = " << extension.c_str() << ")");
      return 0;
      }
    }
  catch (...)
    {
    vtkWarningMacro("ReadData: error in read, setting result to 0");
    result = 0;
    }

  sceneViewNode->SetScreenShot(imageData.GetPointer());
  sceneViewNode->GetScreenShot()->SetSpacing(1.0, 1.0, 1.0);
  sceneViewNode->GetScreenShot()->SetOrigin(0.0, 0.0, 0.0);

  return result;
}

//----------------------------------------------------------------------------
int vtkMRMLAnnotationSnapshotStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLAnnotationSnapshotNode *sceneViewNode = vtkMRMLAnnotationSnapshotNode::SafeDownCast(refNode);

  if (sceneViewNode->GetScreenShot() == nullptr)
    {
    // nothing to write
    return 1;
    }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
  {
    vtkErrorMacro("vtkMRMLAnnotationSnapshotNode: File name not specified");
    return 0;
  }

  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);

  int result = 1;
  if (extension == ".png")
    {
    vtkNew<vtkPNGWriter> writer;
    writer->SetFileName(fullName.c_str());
    writer->SetInputData( sceneViewNode->GetScreenShot() );
    try
      {
      writer->Write();
      }
    catch (...)
      {
      result = 0;
      }
    }
  else if (extension == ".jpg" || extension == ".jpeg")
    {
    vtkNew<vtkJPEGWriter> writer;
    writer->SetFileName(fullName.c_str());
    writer->SetInputData( sceneViewNode->GetScreenShot() );
    try
      {
      writer->Write();
      }
    catch (...)
      {
      result = 0;
      }
    }
  else if (extension == ".tiff")
    {
    vtkNew<vtkTIFFWriter> writer;
    writer->SetFileName(fullName.c_str());
    writer->SetInputData( sceneViewNode->GetScreenShot() );
    try
      {
      writer->Write();
      }
    catch (...)
      {
      result = 0;
      }
    }
  else if (extension == ".bmp")
    {
    vtkNew<vtkBMPWriter> writer;
    writer->SetFileName(fullName.c_str());
    writer->SetInputData( sceneViewNode->GetScreenShot() );
    try
      {
      writer->Write();
      }
    catch (...)
      {
      result = 0;
      }
    }
  else
    {
    result = 0;
    vtkErrorMacro( << "No file extension recognized: " << fullName.c_str() );
    }

  return result;
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationSnapshotStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("PNG (.png)");
  this->SupportedReadFileTypes->InsertNextValue("JPG (.jpg)");
  this->SupportedReadFileTypes->InsertNextValue("JPEG (.jpeg)");
  this->SupportedReadFileTypes->InsertNextValue("TIFF (.tiff)");
  this->SupportedReadFileTypes->InsertNextValue("BMP (.bmp)");
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationSnapshotStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("PNG (.png)");
  this->SupportedWriteFileTypes->InsertNextValue("JPG (.jpg)");
  this->SupportedWriteFileTypes->InsertNextValue("JPEG (.jpeg)");
  this->SupportedWriteFileTypes->InsertNextValue("TIFF (.tiff)");
  this->SupportedWriteFileTypes->InsertNextValue("BMP (.bmp)");
}

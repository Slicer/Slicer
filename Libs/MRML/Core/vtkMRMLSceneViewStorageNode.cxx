/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSceneViewStorageNode.cxx,v $
  Date:      $Date: 2006/03/17 15:10:09 $
  Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLSceneViewNode.h"
#include "vtkMRMLSceneViewStorageNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkBMPReader.h>
#include <vtkBMPWriter.h>
#include <vtkErrorCode.h>
#include <vtkImageData.h>
#include <vtkJPEGReader.h>
#include <vtkJPEGWriter.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPNGReader.h>
#include <vtkPNGWriter.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTIFFReader.h>
#include <vtkTIFFWriter.h>
#include <vtkVersion.h>

// ITK includes
#include <itksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSceneViewStorageNode);

//----------------------------------------------------------------------------
vtkMRMLSceneViewStorageNode::vtkMRMLSceneViewStorageNode()
{
  this->DefaultWriteFileExtension = "png";
}

//----------------------------------------------------------------------------
vtkMRMLSceneViewStorageNode::~vtkMRMLSceneViewStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLSceneViewStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  return this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLSceneViewStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLSceneViewNode");
}

//----------------------------------------------------------------------------
int vtkMRMLSceneViewStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  // don't read from disk if restoring
  if (this->GetScene() && this->GetScene()->IsRestoring())
    {
    return 1;
    }

  vtkMRMLSceneViewNode *sceneViewNode = dynamic_cast <vtkMRMLSceneViewNode *> (refNode);

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  if (itksys::SystemTools::FileExists(fullName.c_str(), true) == false)
    {
    vtkErrorMacro("ReadDataInternal: file does not exist: " << fullName.c_str());
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
  vtkSmartPointer<vtkImageReader2> reader;

  if ( extension == std::string(".png") )
    {
      reader=vtkSmartPointer<vtkPNGReader>::New();
    }
  else if (extension == std::string(".jpg") ||
           extension == std::string(".jpeg"))
    {
    reader=vtkSmartPointer<vtkJPEGReader>::New();
    }
  else if (extension == std::string(".tiff"))
    {
    reader=vtkSmartPointer<vtkTIFFReader>::New();
    }
  else if (extension == std::string(".bmp"))
    {
    reader=vtkSmartPointer<vtkBMPReader>::New();
    }
  else
    {
    vtkDebugMacro("Cannot read scene view file '" << fullName.c_str() << "' (extension = " << extension.c_str() << ")");
    return 0;
    }

  try
    {
    reader->SetFileName(fullName.c_str());
    reader->Update();
    if (reader->GetOutput())
      {
      vtkDebugMacro("ReadData: read file, copying output to image data");
      imageData->DeepCopy(reader->GetOutput());
      }
    if (reader->GetErrorCode() != vtkErrorCode::NoError)
      {
      vtkDebugMacro("Cannot read scene view file '" << fullName.c_str() << "' ("
        << vtkErrorCode::GetStringFromErrorCode(reader->GetErrorCode()) << ")");
      result = 0;
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
int vtkMRMLSceneViewStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLSceneViewNode *sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(refNode);

  if (sceneViewNode->GetScreenShot() == nullptr)
    {
    // nothing to write
    return 1;
    }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLSceneViewNode: File name not specified");
    return 0;
    }

  std::string extension=vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);

  vtkSmartPointer<vtkImageWriter> writer;
  if (extension == ".png")
    {
    writer = vtkSmartPointer<vtkPNGWriter>::New();
    }
  else if (extension == ".jpg" || extension == ".jpeg")
    {
    writer = vtkSmartPointer<vtkJPEGWriter>::New();
    }
  else if (extension == ".tiff")
    {
    writer = vtkSmartPointer<vtkTIFFWriter>::New();
    }
  else if (extension == ".bmp")
    {
    writer = vtkSmartPointer<vtkBMPWriter>::New();
    }
  else
    {
    vtkErrorMacro( << "No file extension recognized: " << fullName.c_str() );
    return 0;
    }

  int result = 1; // success by default

  writer->SetFileName(fullName.c_str());
  writer->SetInputData( sceneViewNode->GetScreenShot() );
  try
    {
    writer->Write();
    }
  catch (...)
    {
    vtkDebugMacro("Cannot write scene view file '" << fullName.c_str() << "' unknown exception occurred");
    result = 0;
    }
  if (writer->GetErrorCode() != vtkErrorCode::NoError)
    {
    vtkDebugMacro("Cannot write scene view file '" << fullName.c_str() << "' ("
      << vtkErrorCode::GetStringFromErrorCode(writer->GetErrorCode()) << ")");
    result = 0;
    }

  if (result != 0)
    {
    this->StageWriteData(refNode);
    }

  return result;
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("PNG (.png)");
  this->SupportedReadFileTypes->InsertNextValue("JPG (.jpg)");
  this->SupportedReadFileTypes->InsertNextValue("JPEG (.jpeg)");
  this->SupportedReadFileTypes->InsertNextValue("TIFF (.tiff)");
  this->SupportedReadFileTypes->InsertNextValue("BMP (.bmp)");
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("PNG (.png)");
  this->SupportedWriteFileTypes->InsertNextValue("JPG (.jpg)");
  this->SupportedWriteFileTypes->InsertNextValue("JPEG (.jpeg)");
  this->SupportedWriteFileTypes->InsertNextValue("TIFF (.tiff)");
  this->SupportedWriteFileTypes->InsertNextValue("BMP (.bmp)");
}

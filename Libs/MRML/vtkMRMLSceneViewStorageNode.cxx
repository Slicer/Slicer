/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.
  
  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.
  
  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSceneViewStorageNode.cxx,v $
  Date:      $Date: 2006/03/17 15:10:09 $
  Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLSceneViewStorageNode.h"
#include "vtkMRMLScene.h"

#include "vtkJPEGReader.h" 
#include "vtkPNGReader.h"
#include "vtkTIFFReader.h"
#include "vtkBMPReader.h"

#include "vtkJPEGWriter.h" 
#include "vtkPNGWriter.h"
#include "vtkTIFFWriter.h"
#include "vtkBMPWriter.h"

#include "itksys/SystemTools.hxx"

#include "vtkSmartPointer.h"
#include "vtkImageData.h"
#include "vtkStringArray.h"

//------------------------------------------------------------------------------
vtkMRMLSceneViewStorageNode* vtkMRMLSceneViewStorageNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLSceneViewStorageNode");
  if(ret)
    {
    return (vtkMRMLSceneViewStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLSceneViewStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLSceneViewStorageNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLSceneViewStorageNode");
  if(ret)
    {
    return (vtkMRMLSceneViewStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLSceneViewStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLSceneViewStorageNode::vtkMRMLSceneViewStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLSceneViewStorageNode::~vtkMRMLSceneViewStorageNode()
{
}

void vtkMRMLSceneViewStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewStorageNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLStorageNode::ReadXMLAttributes(atts);
  vtkDebugMacro("ReadXMLAttributes: file name = " << this->GetFileName());
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLSceneViewStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{

  vtkMRMLStorageNode::PrintSelf(os,indent);

  os << indent << "FileName: " <<
    (this->FileName ? this->FileName : "(none)") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

//----------------------------------------------------------------------------
int vtkMRMLSceneViewStorageNode::ReadData(vtkMRMLNode *refNode)
{
  if (refNode == NULL)
    {
    vtkErrorMacro("ReadData: can't read into a null node");
    return 0;
    }

  // do not read if if we are not in the scene (for example inside snapshot)
  if ( !refNode->GetAddToScene() )
    {
    vtkDebugMacro("ReadData: reference node not to be added to the scene");
    return 1;
    }

  if (this->GetScene() && this->GetScene()->GetReadDataOnLoad() == 0)
    {
    vtkDebugMacro("ReadData: get read data on load is zero");
    return 1;
    }

  if (!refNode->IsA("vtkMRMLSceneViewNode") ) 
    {
    vtkErrorMacro("Reference node is not a vtkMRMLSceneViewNode");
    return 0;
    }

  Superclass::StageReadData(refNode);
  if ( this->GetReadState() != this->TransferDone )
    {
    // remote file download hasn't finished
    vtkWarningMacro("ReadData: remote file download hasn't finished");
    return 0;
    }

  vtkMRMLSceneViewNode *sceneViewNode = dynamic_cast <vtkMRMLSceneViewNode *> (refNode);

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
  vtkImageData *imageData = vtkImageData::New();

  try
    {
    if ( extension == std::string(".png") )
      {
      vtkSmartPointer<vtkPNGReader> reader = vtkSmartPointer<vtkPNGReader>::New();
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
      vtkSmartPointer<vtkJPEGReader> reader = vtkSmartPointer<vtkJPEGReader>::New();
      reader->SetFileName(fullName.c_str());
      reader->Update();
      if (reader->GetOutput())
        {
        imageData->DeepCopy(reader->GetOutput());
        }
      }
    else if (extension == std::string(".tiff")) 
      {
      vtkSmartPointer<vtkTIFFReader> reader = vtkSmartPointer<vtkTIFFReader>::New();
      reader->SetFileName(fullName.c_str());
      reader->Update();
      if (reader->GetOutput())
        {
        imageData->DeepCopy(reader->GetOutput());
        }
      }  
    else if (extension == std::string(".bmp")) 
      {
      vtkSmartPointer<vtkBMPReader> reader = vtkSmartPointer<vtkBMPReader>::New();
      reader->SetFileName(fullName.c_str());
      reader->Update();
      if (reader->GetOutput())
        {
        imageData->DeepCopy(reader->GetOutput());
        }
      }
    else 
      {
      vtkDebugMacro("Cannot read scene view file '" << name.c_str() << "' (extension = " << extension.c_str() << ")");
      return 0;
      }
    }
  catch (...)
    {
    vtkWarningMacro("ReadData: error in read, setting result to 0");
    result = 0;
    }
  
  sceneViewNode->SetScreenshot(imageData);
  sceneViewNode->GetScreenshot()->SetSpacing(1.0, 1.0, 1.0);
  sceneViewNode->GetScreenshot()->SetOrigin(0.0, 0.0, 0.0);
  sceneViewNode->GetScreenshot()->SetScalarType(VTK_UNSIGNED_CHAR);
  imageData->Delete();

  this->SetReadStateIdle();
  
  sceneViewNode->SetModifiedSinceRead(0);

  return result;
}

//----------------------------------------------------------------------------
int vtkMRMLSceneViewStorageNode::WriteData(vtkMRMLNode *refNode)
{
  if (refNode == NULL)
    {
    vtkErrorMacro("WriteData: can't write, input node is null");
    return 0;
    }

  // test whether refNode is a valid node to hold a scene view
  if (!refNode->IsA("vtkMRMLSceneViewNode") ) 
  {
    vtkErrorMacro("Reference node is not a vtkMRMLSceneViewNode");
    return 0;
  }

  vtkMRMLSceneViewNode *sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(refNode);

  if (sceneViewNode->GetScreenshot() == NULL)
    {
    // nothing to write
    return 1;
    }
  
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string("")) 
  {
    vtkErrorMacro("vtkMRMLSceneViewNode: File name not specified");
    return 0;
  }

  std::string extension = itksys::SystemTools::GetFilenameLastExtension(fullName);

  int result = 1;
  if (extension == ".png")
    {
    vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
    writer->SetFileName(fullName.c_str());
    writer->SetInput( sceneViewNode->GetScreenshot() );
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
    vtkSmartPointer<vtkJPEGWriter> writer = vtkSmartPointer<vtkJPEGWriter>::New();
    writer->SetFileName(fullName.c_str());
    writer->SetInput( sceneViewNode->GetScreenshot() );
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
    vtkSmartPointer<vtkTIFFWriter> writer = vtkSmartPointer<vtkTIFFWriter>::New();
        writer->SetFileName(fullName.c_str());
    writer->SetInput( sceneViewNode->GetScreenshot() );
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
    vtkSmartPointer<vtkBMPWriter> writer = vtkSmartPointer<vtkBMPWriter>::New();
        writer->SetFileName(fullName.c_str());
    writer->SetInput( sceneViewNode->GetScreenshot() );
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

  if (result != 0)
    {
    this->StageWriteData(refNode);
    }

  return result;
}


//----------------------------------------------------------------------------
int vtkMRMLSceneViewStorageNode::SupportedFileType(const char *fileName)
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
  if (extension.compare(".png") == 0 ||
        extension.compare(".jpeg") == 0 ||
        extension.compare(".jpg") == 0 ||
        extension.compare(".tiff") == 0 ||
        extension.compare(".bmp") == 0)
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewStorageNode::InitializeSupportedWriteFileTypes()
{
  // Look at WriteData(), .g and .meta are not being written even though 
  // SupportedFileType() says they are supported
  this->SupportedWriteFileTypes->InsertNextValue("PNG (.png)");
  this->SupportedWriteFileTypes->InsertNextValue("JPG (.jpg)");
  this->SupportedWriteFileTypes->InsertNextValue("JPEG (.jpeg)");
  this->SupportedWriteFileTypes->InsertNextValue("TIFF (.tiff)");
  this->SupportedWriteFileTypes->InsertNextValue("BMP (.bmp)");
}

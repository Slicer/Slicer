/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeHeaderlessStorageNode.cxx,v $
Date:      $Date: 2006/03/18 12:24:37 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// MRML includes
#include "vtkDataIOManager.h"
#include "vtkDataFileFormatHelper.h"
#include "vtkMRMLScene.h"
#ifdef MRML_USE_vtkTeem
#include "vtkMRMLVectorVolumeNode.h"
#endif
#include "vtkMRMLVolumeHeaderlessStorageNode.h"

// ITK includes
#include "vtkITKImageWriter.h"
#include <itkArchetypeSeriesFileNames.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkImageChangeInformation.h>
#include <vtkStringArray.h>
#include <vtkImageReader2.h>
#include <vtkImageAppend.h>
#include <vtkImageFlip.h>
#include <vtkNew.h>
#include <vtkVersion.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLVolumeHeaderlessStorageNode);

//----------------------------------------------------------------------------
vtkMRMLVolumeHeaderlessStorageNode::vtkMRMLVolumeHeaderlessStorageNode()
{
  this->FileScanOrder = nullptr;
  this->FileScalarType = VTK_SHORT;
  this->FileNumberOfScalarComponents = 0;
  this->FileLittleEndian = 0;
  memset(this->FileDimensions,0,3*sizeof(int));
  memset(this->FileSpacing,0,3*sizeof(double));
  this->CenterImage = 0;

  // ScanOrder can never be nullptr
  this->FileScanOrder = new char[3];
  strcpy(this->FileScanOrder, "");

  // Initialize
  this->SetFileDimensions(0, 0, 0);
  this->SetFileSpacing(0, 0, 0);

}

//----------------------------------------------------------------------------
vtkMRMLVolumeHeaderlessStorageNode::~vtkMRMLVolumeHeaderlessStorageNode()
{
  if (this->FileScanOrder)
    {
    delete [] this->FileScanOrder;
    this->FileScanOrder = nullptr;
    }
}

//----------------------------------------------------------------------------
const char* vtkMRMLVolumeHeaderlessStorageNode::GetFileScalarTypeAsString()
{
  switch (this->FileScalarType)
    {
    case VTK_VOID:           return "Void"; break;
    case VTK_BIT:            return "Bit"; break;
    case VTK_CHAR:           return "Char"; break;
    case VTK_UNSIGNED_CHAR:  return "UnsignedChar"; break;
    case VTK_SHORT:          return "Short"; break;
    case VTK_UNSIGNED_SHORT: return "UnsignedShort"; break;
    case VTK_INT:            return "Int"; break;
    case VTK_UNSIGNED_INT:   return "UnsignedInt"; break;
    case VTK_LONG:           return "Long"; break;
    case VTK_UNSIGNED_LONG:  return "UnsignedLong"; break;
    case VTK_FLOAT:          return "Float"; break;
    case VTK_DOUBLE:         return "Double"; break;
    }
  return "Short";
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeHeaderlessStorageNode::SetFileScalarTypeAsString(const char* type)
{
  if (!strcmp(type, "Bit"))
    {
    this->FileScalarType = VTK_BIT;
    }
  else if (!strcmp(type, "Char"))
    {
    this->FileScalarType = VTK_CHAR;
    }
  else if (!strcmp(type, "UnsignedChar"))
    {
    this->FileScalarType = VTK_UNSIGNED_CHAR;
    }
  else if (!strcmp(type, "Short"))
    {
    this->FileScalarType = VTK_SHORT;
    }
  else if (!strcmp(type, "UnsignedShort"))
    {
    this->FileScalarType = VTK_UNSIGNED_SHORT;
    }
  else if (!strcmp(type, "Int"))
    {
    this->FileScalarType = VTK_INT;
    }
  else if (!strcmp(type, "UnsignedInt"))
    {
    this->FileScalarType = VTK_UNSIGNED_INT;
    }
  else if (!strcmp(type, "Long"))
    {
    this->FileScalarType = VTK_LONG;
    }
  else if (!strcmp(type, "UnsignedLong"))
    {
    this->FileScalarType = VTK_UNSIGNED_LONG;
    }
  else if (!strcmp(type, "Float"))
    {
    this->FileScalarType = VTK_FLOAT;
    }
  else if (!strcmp(type, "Double"))
    {
    this->FileScalarType = VTK_DOUBLE;
    }
}


void vtkMRMLVolumeHeaderlessStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  {
  std::stringstream ss;
  ss << this->CenterImage;
  of << " centerImage=\"" << ss.str() << "\"";
  }
  {
  of << " fileDimensions=\"" << this->FileDimensions[0] << " "
    << this->FileDimensions[1] << " "
    << this->FileDimensions[2] << "\"";
  }
  {
  of << " fileSpacing=\"" << this->FileSpacing[0] << " "
    << this->FileSpacing[1] << " "
    << this->FileSpacing[2] << "\"";
  }
  {
  std::stringstream ss;
  ss << this->FileLittleEndian;
  of << " fileLittleEndian=\"" << ss.str() << "\"";
  }
  {
  std::stringstream ss;
  ss << this->FileScalarType;
  of << " fileScalarType=\"" << ss.str() << "\"";
  }
  {
  std::stringstream ss;
  ss << this->FileScanOrder;
  of << " fileScanOrder=\"" << ss.str() << "\"";
  }
  {
  std::stringstream ss;
  ss << this->FileNumberOfScalarComponents;
  of << " fileNumberOfScalarComponents=\"" << ss.str() << "\"";
  }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeHeaderlessStorageNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  vtkMRMLStorageNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "fileScanOrder"))
      {
      this->SetFileScanOrder(attValue);
      }
    else if (!strcmp(attName, "fileDimensions"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> FileDimensions[0];
      ss >> FileDimensions[1];
      ss >> FileDimensions[2];
      }
    else if (!strcmp(attName, "fileSpacing"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> FileSpacing[0];
      ss >> FileSpacing[1];
      ss >> FileSpacing[2];
      }
    else if (!strcmp(attName, "fileNumberOfScalarComponents"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> FileNumberOfScalarComponents;
      }
    else if (!strcmp(attName, "fileScalarType"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> FileScalarType;
      }
    else if (!strcmp(attName, "fileLittleEndian"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> FileLittleEndian;
      }
    else if (!strcmp(attName, "centerImage"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> CenterImage;
      }
    }

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLVolumeHeaderlessStorageNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLVolumeHeaderlessStorageNode *node = (vtkMRMLVolumeHeaderlessStorageNode *) anode;

  this->SetFileScanOrder(node->FileScanOrder);
  this->SetFileSpacing(node->FileSpacing);
  this->SetFileDimensions(node->FileDimensions);
  this->SetFileLittleEndian(node->FileLittleEndian);
  this->SetFileScalarType(node->FileScalarType);
  this->SetFileNumberOfScalarComponents(node->FileNumberOfScalarComponents);
  this->SetCenterImage(node->CenterImage);

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkMRMLVolumeHeaderlessStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);
  int idx;

  os << indent << "FileScanOrder: " <<
    (this->FileScanOrder ? this->FileScanOrder : "(none)") << "\n";
  os << indent << "FileLittleEndian:  " << this->FileLittleEndian << "\n";
  os << indent << "FileScalarType:    " << this->FileScalarType << "\n";
  os << indent << "FileNumberOfScalarComponents:  " << this->FileNumberOfScalarComponents << "\n";
  os << "FileSpacing:\n";
  for (idx = 0; idx < 3; ++idx)
    {
    os << indent << ", " << this->FileSpacing[idx];
    }
  os << ")\n";

  os << "FileDimensions:\n";
  for (idx = 0; idx < 3; ++idx)
    {
    os << indent << ", " << this->FileDimensions[idx];
    }
  os << ")\n";

}

//----------------------------------------------------------------------------
bool vtkMRMLVolumeHeaderlessStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLScalarVolumeNode") ||
         refNode->IsA("vtkMRMLVectorVolumeNode");
}

//----------------------------------------------------------------------------
int vtkMRMLVolumeHeaderlessStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  // Skip file loading for empty volume, for which no file was saved
  if (this->GetWriteState() == SkippedNoData)
    {
    vtkDebugMacro("ReadDataInternal: Empty volume file was not saved, ignore loading");
    return 1;
    }

  vtkMRMLVolumeNode *volNode = nullptr;

  if ( refNode->IsA("vtkMRMLScalarVolumeNode") )
    {
    volNode = dynamic_cast <vtkMRMLScalarVolumeNode *> (refNode);
    }
#ifdef MRML_USE_vtkTeem
  else if ( refNode->IsA("vtkMRMLVectorVolumeNode") )
    {
    volNode = dynamic_cast <vtkMRMLVectorVolumeNode *> (refNode);
    }
#endif
  if (volNode->GetImageData())
    {
    volNode->SetAndObserveImageData(nullptr);
    }

  std::string fullName = this->GetFullNameFromFileName();

  if (fullName.empty())
    {
    vtkErrorMacro("ReadDataInternal: Volume file name not specified");
    return 0;
    }

  vtkNew<vtkStringArray> archNames;
  itk::ArchetypeSeriesFileNames::Pointer archtypeNames = itk::ArchetypeSeriesFileNames::New();

  archtypeNames->SetArchetype(fullName);
  itk::ArchetypeSeriesFileNames::StringVectorType names = archtypeNames->GetFileNames();

  vtkNew<vtkImageReader2> reader;
  reader->SetNumberOfScalarComponents(this->GetFileNumberOfScalarComponents());
  reader->SetDataScalarType(this->GetFileScalarType());
  reader->SetDataByteOrder(this->GetFileLittleEndian());

  vtkNew<vtkImageFlip> flip;
  flip->SetInputConnection(reader->GetOutputPort());
  flip->SetFilteredAxes(1);

  int dims[3];
  this->GetFileDimensions(dims[0], dims[1], dims[2]);
  dims[2] = names.size();
  reader->SetDataExtent(0, dims[0]-1, 0, dims[1]-1, 0, 0);

  double spacing[3];
  this->GetFileSpacing(spacing);

  vtkNew<vtkImageAppend> appender;
  appender->SetAppendAxis(2);

  vtkNew<vtkImageData> image;

  int result = 1;

  for (unsigned int i=0; i<names.size(); i++)
    {
    archNames->InsertNextValue(names[i].c_str());
    reader->SetFileName(names[i].c_str());
    try
      {
      reader->Update();
      flip->Update();
      }
      catch (...)
      {
      vtkErrorMacro("ReadDataInternal: Cannot read file");
      reader->RemoveObservers( vtkCommand::ProgressEvent,  this->MRMLCallbackCommand);
      return 0;
      }
    if (reader->GetOutput() == nullptr)
      {
      vtkErrorMacro("ReadDataInternal: Cannot read file");
      return 0;
      }
    if (i==0)
      {
      image->DeepCopy(flip->GetOutput());
      }
    else
      {
      appender->SetInputData(0, image.GetPointer());
      appender->SetInputConnection(1, flip->GetOutputPort());
      appender->Update();
      image ->DeepCopy(appender->GetOutput());
      }
    }

  vtkNew<vtkImageChangeInformation> ici;
  ici->SetInputData(image.GetPointer());
  ici->SetOutputSpacing( 1, 1, 1 );
  ici->SetOutputOrigin( 0, 0, 0 );
  ici->Update();

  if (ici->GetOutput() == nullptr)
    {
    vtkErrorMacro("ReadDataInternal: Cannot read file");
    reader->RemoveObservers( vtkCommand::ProgressEvent,  this->MRMLCallbackCommand);
    return 0;
    }
  else
    {
    volNode->SetAndObserveImageData(ici->GetOutput());
    }

  vtkNew<vtkMatrix4x4> mat;
  mat->Identity();
  volNode->ComputeIJKToRASFromScanOrder(this->GetFileScanOrder(),
                                        spacing, dims,
                                        this->GetCenterImage(),
                                        mat.GetPointer());

  volNode->SetIJKToRASMatrix(mat.GetPointer());

  reader->RemoveObservers(vtkCommand::ProgressEvent, this->MRMLCallbackCommand);

  return result;
}

//----------------------------------------------------------------------------
bool vtkMRMLVolumeHeaderlessStorageNode::CanWriteFromReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLScalarVolumeNode");
}

//----------------------------------------------------------------------------
int vtkMRMLVolumeHeaderlessStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLVolumeNode *volNode = vtkMRMLScalarVolumeNode::SafeDownCast(refNode);

  if (volNode->GetImageData() == nullptr)
    {
    this->SetWriteStateSkippedNoData();
    return 1;
    }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLVolumeNode: File name not specified");
    return 0;
    }
  vtkNew<vtkITKImageWriter> writer;
  writer->SetFileName(fullName.c_str());

  writer->SetInputData( volNode->GetImageData() );
  if(this->WriteFileFormat)
    {
    writer->SetImageIOClassName(
      this->GetScene()->GetDataIOManager()->GetFileFormatHelper()->
      GetClassNameFromFormatString(this->WriteFileFormat));
    }

  // set volume attributes
  vtkNew<vtkMatrix4x4> mat;
  volNode->GetRASToIJKMatrix(mat.GetPointer());
  writer->SetRasToIJKMatrix(mat.GetPointer());

  int result = 1;
  try
    {
    writer->Write();
    }
    catch (...)
    {
    result = 0;
    }

  return result;
}


//----------------------------------------------------------------------------
void vtkMRMLVolumeHeaderlessStorageNode::InitializeSupportedWriteFileTypes()
{
  Superclass::InitializeSupportedWriteFileTypes();

  if (this->GetScene() &&
      this->GetScene()->GetDataIOManager() &&
      this->GetScene()->GetDataIOManager()->GetFileFormatHelper())
    {
    vtkStringArray* supportedFormats = this->GetScene()->GetDataIOManager()->
      GetFileFormatHelper()->GetITKSupportedWriteFileFormats();
    for(int i=0; i<supportedFormats->GetNumberOfTuples(); i++)
      {
      this->SupportedWriteFileTypes->InsertNextValue(
              supportedFormats->GetValue(i));
      }
    }
}

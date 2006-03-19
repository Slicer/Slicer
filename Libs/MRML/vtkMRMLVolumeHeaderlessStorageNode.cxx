/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeHeaderlessStorageNode.cxx,v $
Date:      $Date: 2006/03/18 12:24:37 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

#include <string>
#include <ostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLVolumeHeaderlessStorageNode.h"
#include "vtkMRMLVolumeNode.h"

#include "vtkMatrix4x4.h"
#include "vtkImageData.h"

//------------------------------------------------------------------------------
vtkMRMLVolumeHeaderlessStorageNode* vtkMRMLVolumeHeaderlessStorageNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeHeaderlessStorageNode");
  if(ret)
    {
      return (vtkMRMLVolumeHeaderlessStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVolumeHeaderlessStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLVolumeHeaderlessStorageNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeHeaderlessStorageNode");
  if(ret)
    {
      return (vtkMRMLVolumeHeaderlessStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVolumeHeaderlessStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeHeaderlessStorageNode::vtkMRMLVolumeHeaderlessStorageNode()
{
  this->Filename = NULL;
  this->FileScanOrder = NULL;
  this->FileScalarType = VTK_SHORT;
  this->FileNumberOfScalarComponents = 0;
  this->FileLittleEndian = 0;
  memset(this->FileDimensions,0,2*sizeof(int));
  memset(this->FileSpacing,0,3*sizeof(vtkFloatingPointType));

  // ScanOrder can never be NULL
  this->FileScanOrder = new char[3];
  strcpy(this->FileScanOrder, "");

  // Initialize 
  this->SetFileDimensions(0, 0, 0);
  this->SetFileSpacing(0, 0, 0);

}

//----------------------------------------------------------------------------
vtkMRMLVolumeHeaderlessStorageNode::~vtkMRMLVolumeHeaderlessStorageNode()
{
  if (this->Filename) {
    delete [] this->Filename;
    this->Filename = NULL;
  }
  if (this->FileScanOrder) {
    delete [] this->FileScanOrder;
    this->FileScanOrder = NULL;
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



void vtkMRMLVolumeHeaderlessStorageNode::WriteXML(ostream& of, int nIndent)
{
  vtkErrorMacro("NOT IMPLEMENTED YET");
  (void)of; (void)nIndent;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeHeaderlessStorageNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLStorageNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "Filename")) {
      this->SetFilename(attValue);
    }
    if (!strcmp(attName, "FileScanOrder")) {
      this->SetFileScanOrder(attValue);
    }
    else if (!strcmp(attName, "FileDimensions")) {
      std::stringstream ss;
      ss << attValue;
      ss >> FileDimensions[0];
      ss >> FileDimensions[1];
      ss >> FileDimensions[2];
    }
    else if (!strcmp(attName, "FileSpacing")) {
      std::stringstream ss;
      ss << attValue;
      ss >> FileSpacing[0];
      ss >> FileSpacing[1];
      ss >> FileSpacing[2];
    }
    else if (!strcmp(attName, "FileNumberOfScalarComponents")) {
      std::stringstream ss;
      ss << attValue;
      ss >> FileNumberOfScalarComponents;
    }
    else if (!strcmp(attName, "FileScalarType")) {
      std::stringstream ss;
      ss << attValue;
      ss >> FileScalarType;
    }
    else if (!strcmp(attName, "FileLittleEndian")) {
      std::stringstream ss;
      ss << attValue;
      ss >> FileLittleEndian;
    }

  }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLVolumeHeaderlessStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLVolumeHeaderlessStorageNode *node = (vtkMRMLVolumeHeaderlessStorageNode *) anode;

  this->SetFilename(node->Filename);
  this->SetFileScanOrder(node->FileScanOrder);
  this->SetFileSpacing(node->FileSpacing);
  this->SetFileDimensions(node->FileDimensions);
  this->SetFileLittleEndian(node->FileLittleEndian);
  this->SetFileScalarType(node->FileScalarType);
  this->SetFileNumberOfScalarComponents(node->FileNumberOfScalarComponents);


}

//----------------------------------------------------------------------------
void vtkMRMLVolumeHeaderlessStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{  
  vtkMRMLStorageNode::PrintSelf(os,indent);
  int idx;

  os << indent << "Filename: " <<
    (this->Filename ? this->Filename : "(none)") << "\n";
  os << indent << "FileScanOrder: " <<
    (this->FileScanOrder ? this->FileScanOrder : "(none)") << "\n";
  os << indent << "FileLittleEndian:  " << this->FileLittleEndian << "\n";
  os << indent << "FileScalarType:    " << this->FileScalarType << "\n";
  os << indent << "FileNumberOfScalarComponents:  " << this->FileNumberOfScalarComponents << "\n";
  os << "FileSpacing:\n";
  for (idx = 0; idx < 3; ++idx) {
    os << indent << ", " << this->FileSpacing[idx];
  }
  os << ")\n";
  
  os << "FileDimensions:\n";
  for (idx = 0; idx < 3; ++idx) {
    os << indent << ", " << this->FileDimensions[idx];
  }
  os << ")\n";

}

//----------------------------------------------------------------------------
void vtkMRMLVolumeHeaderlessStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

//----------------------------------------------------------------------------

void vtkMRMLVolumeHeaderlessStorageNode::ReadData(vtkMRMLNode *refNode)
{
  vtkErrorMacro("NOT IMPLEMENTED YET");


  if (!refNode->IsA("vtkMRMLVolumeNode") ) {
    vtkErrorMacro("Reference node is not a vtkMRMLVolumeNode");
    return;
  }

  vtkMRMLVolumeNode *volNode = dynamic_cast <vtkMRMLVolumeNode *> (refNode);

  if (volNode->GetImageData()) {
    volNode->GetImageData()->Delete();
    volNode->SetImageData (NULL);
  }

  std::string fullName;
  if (this->SceneRootDir != NULL) {
    fullName = std::string(this->SceneRootDir) + std::string(this->GetFilename());
  }
  else {
    fullName = std::string(this->GetFilename());
  }

  if (fullName == std::string("")) {
    vtkErrorMacro("vtkMRMLVolumeNode: File name not specified");
  }
  //volNode->SetImageData (reader->GetOutput());

  //volNode->SetIjkToRasMatrix(mat);
  volNode->SetStorageNode(this);
  //TODO update scene to send Modified event
}

void vtkMRMLVolumeHeaderlessStorageNode::WriteData(vtkMRMLNode *refNode)
{
  vtkErrorMacro("NOT IMPLEMENTED YET");

  if (!refNode->IsA("vtkMRMLVolumeNode") ) {
    vtkErrorMacro("Reference node is not a vtkMRMLVolumeNode");
    return;
  }
  
  vtkMRMLVolumeNode *volNode = dynamic_cast <vtkMRMLVolumeNode *> (refNode);
  
  if (volNode->GetImageData() == NULL) {
    vtkErrorMacro("cannot write ImageData, it's NULL");
  }
  
  std::string fullName;
  if (this->SceneRootDir != NULL) {
    fullName = std::string(this->SceneRootDir) + std::string(this->GetFilename());
  }
  else {
    fullName = std::string(this->GetFilename());
  }
  
  if (fullName == std::string("")) {
    vtkErrorMacro("vtkMRMLVolumeNode: File name not specified");
  }
}

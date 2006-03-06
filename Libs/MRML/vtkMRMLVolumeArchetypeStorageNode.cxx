/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeArchetypeStorageNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:41 $
Version:   $Revision: 1.1 $

=========================================================================auto=*/

#include <string>
#include <ostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLVolumeNode.h"

#include "vtkMatrix4x4.h"
#include "vtkImageData.h"
#include "vtkITKArchetypeImageSeriesReader.h"
#include "vtkITKArchetypeImageSeriesScalarReader.h"

// Initialize static member that controls resampling -- 
// old comment: "This offset will be changed to 0.5 from 0.0 per 2/8/2002 Slicer 
// development meeting, to move ijk coordinates to voxel centers."


//------------------------------------------------------------------------------
vtkMRMLVolumeArchetypeStorageNode* vtkMRMLVolumeArchetypeStorageNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeArchetypeStorageNode");
  if(ret)
    {
      return (vtkMRMLVolumeArchetypeStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVolumeArchetypeStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLVolumeArchetypeStorageNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeArchetypeStorageNode");
  if(ret)
    {
      return (vtkMRMLVolumeArchetypeStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVolumeArchetypeStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeArchetypeStorageNode::vtkMRMLVolumeArchetypeStorageNode()
{
  this->FileArcheType = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeArchetypeStorageNode::~vtkMRMLVolumeArchetypeStorageNode()
{
  if (this->FileArcheType) {
    delete [] this->FileArcheType;
    this->FileArcheType = NULL;
  }
}

void vtkMRMLVolumeArchetypeStorageNode::WriteXML(ostream& of, int nIndent)
{
  vtkErrorMacro("NOT IMPLEMENTED YET");
  (void)of; (void)nIndent;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeArchetypeStorageNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLStorageNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "FileArcheType")) {
      this->SetFileArcheType(attValue);
    }
  }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLVolumeArchetypeStorageNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLStorageNode::Copy(anode);
  vtkMRMLVolumeArchetypeStorageNode *node = (vtkMRMLVolumeArchetypeStorageNode *) anode;

  // Strings
  this->SetFileArcheType(node->FileArcheType);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeArchetypeStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{  
  vtkMRMLStorageNode::PrintSelf(os,indent);

  os << indent << "FileArcheType: " <<
    (this->FileArcheType ? this->FileArcheType : "(none)") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeArchetypeStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

//----------------------------------------------------------------------------

void vtkMRMLVolumeArchetypeStorageNode::ReadData(vtkMRMLNode *refNode)
{
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
    fullName = std::string(this->SceneRootDir) + std::string(this->GetFileArcheType());
  }
  else {
    fullName = std::string(this->GetFileArcheType());
  }

  if (fullName == std::string("")) {
    vtkErrorMacro("vtkMRMLVolumeNode: File name not specified");
  }

  //TODO: handle both scalars and vectors
  vtkITKArchetypeImageSeriesScalarReader* reader = vtkITKArchetypeImageSeriesScalarReader::New();

  reader->SetArchetype(fullName.c_str());
  reader->Update();
  volNode->SetImageData (reader->GetOutput());

  // set volume attributes
  vtkMatrix4x4* mat = reader->GetRasToIjkMatrix();
  mat->Invert();

  // normalize direction vectors
  for (int row=0; row<3; row++) {
    double len =0;
    int col;
    for (col=0; col<3; col++) {
      len += mat->GetElement(row, col) * mat->GetElement(row, col);
    }
    len = sqrt(len);
    for (col=0; col<3; col++) {
      mat->SetElement(row, col,  mat->GetElement(row, col)/len);
    }
  }
  volNode->SetIjkToRasMatrix(mat);
}

void vtkMRMLVolumeArchetypeStorageNode::WriteData(vtkMRMLNode *refNode)
{
  vtkErrorMacro("NOT IMPLEMENTED YET");
}

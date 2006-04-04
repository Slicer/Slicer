/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeArchetypeStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

#include <string>
#include <ostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLVectorVolumeNode.h"

#include "vtkMatrix4x4.h"
#include "vtkImageData.h"
#include "vtkITKArchetypeImageSeriesReader.h"
#include "vtkITKArchetypeImageSeriesScalarReader.h"
#include "vtkITKArchetypeImageSeriesVectorReader.h"
#include "vtkITKImageWriter.h"

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
  this->FileArchetype = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeArchetypeStorageNode::~vtkMRMLVolumeArchetypeStorageNode()
{
  if (this->FileArchetype) 
    {
    delete [] this->FileArchetype;
    this->FileArchetype = NULL;
    }
}

void vtkMRMLVolumeArchetypeStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);
  if (this->FileArchetype != NULL) 
    {
    of << indent << "FileArchetype='" << this->FileArchetype << "' ";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeArchetypeStorageNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLStorageNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "FileArchetype")) 
      {
      this->SetFileArchetype(attValue);
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLVolumeArchetypeStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLVolumeArchetypeStorageNode *node = (vtkMRMLVolumeArchetypeStorageNode *) anode;

  this->SetFileArchetype(node->FileArchetype);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeArchetypeStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{  
  vtkMRMLStorageNode::PrintSelf(os,indent);

  os << indent << "FileArchetype: " <<
    (this->FileArchetype ? this->FileArchetype : "(none)") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeArchetypeStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

//----------------------------------------------------------------------------

void vtkMRMLVolumeArchetypeStorageNode::ReadData(vtkMRMLNode *refNode)
{

  // test whether refNode is a valid node to hold a volume
  if ( !refNode->IsA("vtkMRMLScalarVolumeNode") || !refNode->IsA("vtkMRMLVolumeVolumeNode")) 
    {
    vtkErrorMacro("Reference node is not a vtkMRMLVolumeNode");
    return;         
    }

  
  vtkMRMLVolumeNode *volNode;

  vtkITKArchetypeImageSeriesReader* reader;
  
  if ( refNode->IsA("vtkMRMLScalarVolumeNode") ) 
    {
    volNode = dynamic_cast <vtkMRMLScalarVolumeNode *> (refNode);
    reader = vtkITKArchetypeImageSeriesScalarReader::New();  
    }
  else if ( refNode->IsA("vtkMRMLVectorVolumeNode") ) 
    {
    volNode = dynamic_cast <vtkMRMLVectorVolumeNode *> (refNode);
    reader = vtkITKArchetypeImageSeriesVectorReader::New();
    }
  if (volNode->GetImageData()) 
    {
    volNode->GetImageData()->Delete();
    volNode->SetImageData (NULL);
    }

  std::string fullName;
  if (this->SceneRootDir != NULL) 
    {
    fullName = std::string(this->SceneRootDir) + std::string(this->GetFileArchetype());
    }
  else 
    {
    fullName = std::string(this->GetFileArchetype());
    }
  
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("vtkMRMLVolumeNode: File name not specified");
    }

  //TODO: handle both scalars and vectors

  reader->SetArchetype(fullName.c_str());
  reader->SetOutputScalarTypeToNative();
  reader->SetDesiredCoordinateOrientationToNative();
  reader->SetUseNativeOriginOff();
  reader->Update();


  // set volume attributes
  vtkMatrix4x4* mat = reader->GetRasToIjkMatrix();
  mat->Invert();

  // normalize direction vectors
  double spacing[3];
  int row;
  for (row=0; row<3; row++) 
    {
    double len =0;
    int col;
    for (col=0; col<3; col++) 
      {
      len += mat->GetElement(row, col) * mat->GetElement(row, col);
      }
    len = sqrt(len);
    spacing[row] = len;
    for (col=0; col<3; col++) 
      {
      mat->SetElement(row, col,  mat->GetElement(row, col)/len);
      }
    }
  volNode->SetIjkToRasMatrix(mat);
  volNode->SetSpacing(spacing);
  volNode->SetOrigin(reader->GetOutput()->GetOrigin());
  volNode->SetStorageNode(this);
  //TODO update scene to send Modified event
 
  reader->GetOutput()->SetSpacing(1.0, 1.0, 1.0);
  reader->GetOutput()->SetOrigin(0.0, 0.0, 0.0);
  volNode->SetImageData (reader->GetOutput());
}

void vtkMRMLVolumeArchetypeStorageNode::WriteData(vtkMRMLNode *refNode)
{
  // test whether refNode is a valid node to hold a volume
  if (!refNode->IsA("vtkMRMLScalarVolumeNode") ) 
    {
    vtkErrorMacro("Reference node is not a vtkMRMLVolumeNode");
    return;
    }
  
  vtkMRMLVolumeNode *volNode;
  
  if ( refNode->IsA("vtkMRMLScalarVolumeNode") ) 
    {
    volNode = dynamic_cast <vtkMRMLScalarVolumeNode *> (refNode);
    }
  
  if (volNode->GetImageData() == NULL) 
    {
    vtkErrorMacro("cannot write ImageData, it's NULL");
    }
  
  std::string fullName;
  if (this->SceneRootDir != NULL) 
    {
    fullName = std::string(this->SceneRootDir) + std::string(this->GetFileArchetype());
    }
  else 
    {
    fullName = std::string(this->GetFileArchetype());
    }
  
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("vtkMRMLVolumeNode: File name not specified");
    }
  vtkITKImageWriter *writer = vtkITKImageWriter::New();
  writer->SetFileName(fullName.c_str());
  
  writer->SetInput( volNode->GetImageData() );

  // set volume attributes
  vtkMatrix4x4* mat = vtkMatrix4x4::New();
  volNode->GetIjkToRasMatrix(mat);
  mat->Invert();

  writer->SetRasToIJKMatrix(mat);

  writer->Write();

  mat->Delete();
  writer->Delete();
}

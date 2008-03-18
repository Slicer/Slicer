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
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkImageChangeInformation.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLVectorVolumeNode.h"

#include "vtkMatrix4x4.h"
#include "vtkImageData.h"
#include "vtkDataArray.h"
#include "vtkPointData.h"
#include "vtkITKArchetypeImageSeriesReader.h"
#include "vtkITKArchetypeImageSeriesScalarReader.h"
#include "vtkITKArchetypeImageSeriesVectorReaderFile.h"
#include "vtkITKArchetypeImageSeriesVectorReaderSeries.h"
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
  this->CenterImage = 0;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeArchetypeStorageNode::~vtkMRMLVolumeArchetypeStorageNode()
{
}

void vtkMRMLVolumeArchetypeStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);
  {
  std::stringstream ss;
  ss << this->CenterImage;
  of << indent << " centerImage=\"" << ss.str() << "\"";
  }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeArchetypeStorageNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "centerImage")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->CenterImage;
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

  this->SetCenterImage(node->CenterImage);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeArchetypeStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{  
  vtkMRMLStorageNode::PrintSelf(os,indent);
  os << indent << "CenterImage:   " << this->CenterImage << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeArchetypeStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

//----------------------------------------------------------------------------
int vtkMRMLVolumeArchetypeStorageNode::ReadData(vtkMRMLNode *refNode)
{

  // test whether refNode is a valid node to hold a volume
  if ( !( refNode->IsA("vtkMRMLScalarVolumeNode") || refNode->IsA("vtkMRMLVectorVolumeNode" ) ) )
    {
    vtkErrorMacro("Reference node is not a vtkMRMLVolumeNode");
    return 0;         
    }
  if (this->GetFileName() == NULL && this->GetURI() == NULL) 
    {
    vtkErrorMacro("ReadData: both filename and uri are null.");
    return 0;
    }
  
  Superclass::StageReadData(refNode);
  if ( this->GetReadState() != this->Ready )
    {
    // remote file download hasn't finished
    vtkWarningMacro("ReadData: read state is pending, remote download hasn't finished yet");
    return 0;
    }
  else
    {
    vtkDebugMacro("ReadData: read state is ready, URI = " << (this->GetURI() == NULL ? "null" : this->GetURI()) << ", filename = " << (this->GetFileName() == NULL ? "null" : this->GetFileName()));
    }
  
  std::string fullName = this->GetFullNameFromFileName();

  if (fullName == std::string("")) 
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }
  
  vtkMRMLVolumeNode *volNode;
  vtkITKArchetypeImageSeriesReader* reader;
  
  if ( refNode->IsA("vtkMRMLScalarVolumeNode") ) 
    {
    volNode = dynamic_cast <vtkMRMLScalarVolumeNode *> (refNode);
    reader = vtkITKArchetypeImageSeriesScalarReader::New();  
    reader->SetSingleFile( this->GetSingleFile() );
    }
  else if ( refNode->IsA("vtkMRMLVectorVolumeNode") ) 
    {
    volNode = dynamic_cast <vtkMRMLVectorVolumeNode *> (refNode);
    // 
    // decide if we want to use a vector file reader (e.g. for multi-component nrrd)
    // or a vector series reader (e.g. for a sequence of rgb image files)
    // - note these are different classes because they are too big to compile
    //   as a single class on some systems
    //
    vtkITKArchetypeImageSeriesVectorReaderFile *readerFile = vtkITKArchetypeImageSeriesVectorReaderFile::New();
    vtkITKArchetypeImageSeriesVectorReaderSeries *readerSeries = vtkITKArchetypeImageSeriesVectorReaderSeries::New();

    readerFile->SetSingleFile( this->GetSingleFile() );
    readerSeries->SetSingleFile( this->GetSingleFile() );

    readerFile->SetArchetype(fullName.c_str());
    try 
      {
      readerFile->UpdateInformation();
      }
    catch ( ... )
      {
      readerFile->Delete();
      readerSeries->Delete();
      return 0;
      }
    if ( readerFile->GetNumberOfFileNames() == 1 )
      {
      reader = readerFile;
      readerSeries->Delete();
      }
    else
      {
      reader = readerSeries;
      readerFile->Delete();
      }
    if (reader->GetNumberOfComponents() < 3)
      {
      reader->Delete();
      return 0;
      }

    }

  reader->AddObserver( vtkCommand::ProgressEvent,  this->MRMLCallbackCommand);

  if (volNode->GetImageData()) 
    {
    volNode->SetAndObserveImageData (NULL);
    }


  reader->SetArchetype(fullName.c_str());
  reader->SetOutputScalarTypeToNative();
  reader->SetDesiredCoordinateOrientationToNative();
  if (this->CenterImage) 
    {
    reader->SetUseNativeOriginOff();
    }
  else
    {
    reader->SetUseNativeOriginOn();
    }

  int result = 1;
  try
    {
    reader->Update();
    }
    catch (...)
    {
    vtkErrorMacro("vtkMRMLVolumeArchetypeStorageNode: Cannot read file: " << fullName.c_str() );
    reader->RemoveObservers( vtkCommand::ProgressEvent,  this->MRMLCallbackCommand);
    reader->Delete();
    return 0;
    }
  if (reader->GetOutput() == NULL 
      || reader->GetOutput()->GetPointData()->GetScalars()->GetNumberOfTuples() == 0) 
    {
    vtkErrorMacro("vtkMRMLVolumeArchetypeStorageNode: Cannot read file: " << fullName.c_str() );
    reader->Delete();
    return 0;
    }
  // set volume attributes
  volNode->SetAndObserveStorageNodeID(this->GetID());
  volNode->SetMetaDataDictionary( reader->GetMetaDataDictionary() );
  //TODO update scene to send Modified event
 
  vtkImageChangeInformation *ici = vtkImageChangeInformation::New();
  ici->SetInput (reader->GetOutput());
  ici->SetOutputSpacing( 1, 1, 1 );
  ici->SetOutputOrigin( 0, 0, 0 );
  ici->Update();

  if (ici->GetOutput() == NULL)
    {
    vtkErrorMacro("vtkMRMLVolumeArchetypeStorageNode: Cannot read file: " << fullName.c_str() );
    reader->RemoveObservers( vtkCommand::ProgressEvent,  this->MRMLCallbackCommand);
    reader->Delete();
    ici->Delete();
    return 0;
    }
  else
    {
    volNode->SetAndObserveImageData (ici->GetOutput());
    }

  vtkMatrix4x4* mat = reader->GetRasToIjkMatrix();
  if ( mat == NULL )
    {
    vtkErrorMacro ("Reader returned NULL RasToIjkMatrix");
    }
  volNode->SetRASToIJKMatrix(mat);

  reader->RemoveObservers( vtkCommand::ProgressEvent,  this->MRMLCallbackCommand);
  reader->Delete();
  ici->Delete();

  return result;
}

//----------------------------------------------------------------------------
int vtkMRMLVolumeArchetypeStorageNode::WriteData(vtkMRMLNode *refNode)
{
  // test whether refNode is a valid node to hold a volume
  if (!refNode->IsA("vtkMRMLScalarVolumeNode") ) 
    {
    vtkErrorMacro("Reference node is not a vtkMRMLVolumeNode");
    return 0;
    }
  
  vtkMRMLVolumeNode *volNode;
  
  if ( refNode->IsA("vtkMRMLScalarVolumeNode") ) 
    {
    volNode = vtkMRMLScalarVolumeNode::SafeDownCast(refNode);
    }
  
  if (volNode->GetImageData() == NULL) 
    {
    vtkErrorMacro("cannot write ImageData, it's NULL");
    return 0;
    }
  
  std::string fullName = this->GetFullNameFromFileName();  
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("WriteData: File name not specified");
    return 0;
    }
  vtkITKImageWriter *writer = vtkITKImageWriter::New();
  writer->SetFileName(fullName.c_str());
  
  writer->SetInput( volNode->GetImageData() );
  writer->SetUseCompression(this->GetUseCompression());

  // set volume attributes
  vtkMatrix4x4* mat = vtkMatrix4x4::New();
  volNode->GetRASToIJKMatrix(mat);
  writer->SetRasToIJKMatrix(mat);

  int result = 1;
  try
    {
    writer->Write();
    }
    catch (...)
    {
    result = 0;
    }
  mat->Delete();
  writer->Delete();    

  Superclass::StageWriteData(refNode);
  
  return result;

}

//----------------------------------------------------------------------------
int vtkMRMLVolumeArchetypeStorageNode::SupportedFileType(const char *fileName)
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

  // for now, return 1
  return 1;
}

/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFreeSurferModelStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLFreeSurferModelStorageNode.h"
#include "vtkMRMLScene.h"

#include "vtkMatrix4x4.h"
#include "vtkPolyDataNormals.h"
#include "vtkStripper.h"

#include "vtkFSSurfaceReader.h"
#include "vtkMRMLFreeSurferProceduralColorNode.h"
#include "vtkMRMLColorTableNode.h"

#include "vtkPolyDataWriter.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkPolyDataReader.h"
#include "vtkXMLPolyDataReader.h"

#include "vtkMRMLFreeSurferProceduralColorNode.h"
#include "vtkPointData.h"

#include "vtkITKArchetypeImageSeriesScalarReader.h"

#include "itksys/SystemTools.hxx"

#include "vtkCollection.h"
#include "vtkStdString.h"
#include "vtkStringArray.h"

// Initialize static member that controls resampling -- 
// old comment: "This offset will be changed to 0.5 from 0.0 per 2/8/2002 Slicer 
// development meeting, to move ijk coordinates to voxel centers."


//------------------------------------------------------------------------------
vtkMRMLFreeSurferModelStorageNode* vtkMRMLFreeSurferModelStorageNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFreeSurferModelStorageNode");
  if(ret)
    {
    return (vtkMRMLFreeSurferModelStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFreeSurferModelStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLFreeSurferModelStorageNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFreeSurferModelStorageNode");
  if(ret)
    {
    return (vtkMRMLFreeSurferModelStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFreeSurferModelStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLFreeSurferModelStorageNode::vtkMRMLFreeSurferModelStorageNode()
{
  this->UseStripper = 1;
  // set up the list of known surface file extensions
  this->AddFileExtension(std::string(".orig"));
  this->AddFileExtension(std::string(".inflated"));
  this->AddFileExtension(std::string(".sphere"));
  this->AddFileExtension(std::string(".white"));
  this->AddFileExtension(std::string(".smoothwm"));
  this->AddFileExtension(std::string(".pial"));

  // this is for reading in files that have been saved in vtk format
  this->AddFileExtension(std::string(".vtk"));
  this->AddFileExtension(std::string(".vtp"));
}

//----------------------------------------------------------------------------
vtkMRMLFreeSurferModelStorageNode::~vtkMRMLFreeSurferModelStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelStorageNode::WriteXML(ostream& of, int indent)
{
  Superclass::WriteXML(of, indent);

  of << " useStripper=\"" << this->UseStripper << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelStorageNode::ReadXMLAttributes(const char** atts)
{
  vtkDebugMacro("ReadXMLAttributes called... calling superclass");
  
  vtkMRMLStorageNode::ReadXMLAttributes(atts);
  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "useStripper")) 
      {
      vtkDebugMacro("Got use stripper " << attValue);
      this->SetUseStripper(atoi(attValue));
      }
    if (!strcmp(attName, "surfaceFileName") || !strcmp(attName, "overlays"))
      {
      vtkErrorMacro("ReadXMLAttributes: surfaceFileName and overlays are attributes of old style freesurfer model storage nodes, they are now handled by freesurfer model overlay storage nodes.");
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLFreeSurferModelStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);

  vtkMRMLFreeSurferModelStorageNode *node = (vtkMRMLFreeSurferModelStorageNode*)anode;
  this->SetUseStripper(node->GetUseStripper());
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLStorageNode::PrintSelf(os,indent);

  os << indent << "Use Triangle Stripper: " << this->UseStripper << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

//----------------------------------------------------------------------------
int vtkMRMLFreeSurferModelStorageNode::ReadData(vtkMRMLNode *refNode)
{
  if (this->GetScene() && this->GetScene()->GetReadDataOnLoad() == 0)
    {
    return 1;
    }

  if (refNode == NULL)
    {
    vtkErrorMacro("vtkMRMLFreeSurferModelStorageNode::ReadData: Reference node is null.");
    return 0;
    }
    
  // do not read if if we are not in the scene (for example inside snapshot)
  if (  !refNode->GetAddToScene() )
    {
    return 1;
    }

  if (!refNode->IsA("vtkMRMLModelNode") ) 
    {
    vtkErrorMacro("Reference node is not a vtkMRMLModelNode");
    return 0;
    }

  Superclass::StageReadData(refNode);
  if ( this->GetReadState() != this->TransferDone )
    {
    // remote file download hasn't finished
    return 0;
    }
  
  vtkMRMLModelNode *modelNode = dynamic_cast <vtkMRMLModelNode *> (refNode);
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  vtkDebugMacro("ReadData: reading " << fullName.c_str());
  
  // compute file prefix
  std::string name(fullName);
  std::string::size_type loc = name.find_last_of(".");
  if( loc == std::string::npos ) 
    {
    vtkErrorMacro("ReadData: no file extension specified");
    }
  std::string extension = name.substr(loc);

  vtkDebugMacro("ReadData: extension = " << extension.c_str());
  
  int result = 1;
  try
    {
    if ( this->IsKnownFileExtension(extension))
      {
      vtkDebugMacro("Reading in a freesurfer surface file, extension = " << extension.c_str());

      if ( extension == std::string(".vtk"))
        {
        // read in a binary vtk surface file
        vtkPolyDataReader *reader = vtkPolyDataReader::New();
        reader->SetFileName(fullName.c_str());
        if (!reader->IsFilePolyData())
          {
          vtkErrorMacro("File " << fullName.c_str() << " is not polydata, cannot be read with this reader");
          result = 0;
          }
        else
          {
          reader->Update();
          if (reader->GetOutput() == NULL)
            {
            vtkErrorMacro("Unable to read file " << fullName.c_str());
            result = 0;
            }
          else
            {
            modelNode->SetAndObservePolyData(reader->GetOutput());
            }
          }
        reader->Delete();
        }
      else if (extension == std::string(".vtp"))
        {
        // read in an ascii vtk surface file
        vtkXMLPolyDataReader *reader = vtkXMLPolyDataReader::New();
        reader->SetFileName(fullName.c_str());
        reader->Update();
        modelNode->SetAndObservePolyData(reader->GetOutput());
        reader->Delete();
        }
      else
        {
        //read in a free surfer file
        // -- create normals and triangle strips also
        vtkFSSurfaceReader *reader = vtkFSSurfaceReader::New();
        vtkPolyDataNormals *normals = vtkPolyDataNormals::New();
        vtkStripper *stripper = vtkStripper::New();
        
        reader->SetFileName(fullName.c_str());
        normals->SetSplitting(0);
        normals->SetInput( reader->GetOutput() );
        if ( this->GetUseStripper() )
          {
          stripper->SetInput( normals->GetOutput() );
          stripper->Update();
          if (stripper->GetOutput() == NULL ||
              stripper->GetOutput()->GetNumberOfCells() == 0)
            {
            vtkDebugMacro("Surface file error: no output from triangle stripper.");
            result = 0;
            }
          else
            {
            modelNode->SetAndObservePolyData(stripper->GetOutput());
            }
          }
        else
          {
          normals->Update();
          if (normals->GetOutput() == NULL ||
              normals->GetOutput()->GetNumberOfCells() == 0)
            {
            vtkDebugMacro("Surface file error: no output from the normals");
            result = 0;
            }
          else
            {
            modelNode->SetAndObservePolyData(normals->GetOutput());
            }
          }
        
        reader->Delete();
        normals->Delete();
        stripper->Delete();    
        }
      }
    else 
      {
      vtkErrorMacro("MRML FreeSurfer ModelStorage Node: Cannot read model file '" << name.c_str() << "' (extension = " << extension.c_str() << ")");
      return 0;
      }
    }
  catch (...)
    {
    result = 0;
    }
  
  if (modelNode->GetPolyData() != NULL) 
    {
    modelNode->GetPolyData()->Modified();
    }

  this->SetReadStateIdle();
  
  modelNode->SetModifiedSinceRead(0);
  return result;
}


//----------------------------------------------------------------------------
int vtkMRMLFreeSurferModelStorageNode::WriteData(vtkMRMLNode *refNode)
{
  if (refNode == NULL)
    {
    vtkErrorMacro("WriteData: Reference node is null!");
    return 0;
    }
  // test whether refNode is a valid node to hold a model
  if (!refNode->IsA("vtkMRMLModelNode") ) 
    {
    vtkErrorMacro("WriteData: Reference node is not a vtkMRMLModelNode");
    return 0;
    }
  
  vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(refNode);
  
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("WriteData: File name not specified");
    return 0;
    }

  std::string extension = itksys::SystemTools::GetFilenameLastExtension(fullName);

  int result = 1;
  if (extension == std::string(".vtk"))
    {
    vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
    writer->SetFileName(fullName.c_str());
    writer->SetInput( modelNode->GetPolyData() );
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
  else if (extension == std::string(".vtp"))
    {
    vtkXMLPolyDataWriter *writer = vtkXMLPolyDataWriter::New();
    writer->SetFileName(fullName.c_str());
    writer->SetInput( modelNode->GetPolyData() );
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
    vtkErrorMacro("WriteData: No Writer for file extension: '" << extension.c_str() << "', use VTK model extensions .vtk or .vtp" );
    }
  
  if (result != 0)
    {
    this->StageWriteData(refNode);
    }
  
  return result;
}

//----------------------------------------------------------------------------
int vtkMRMLFreeSurferModelStorageNode::CopyData(vtkMRMLNode *refNode,
                                                const char *newFileName)
{
  
  bool copyOK;
  
  // test whether refNode is a valid node to hold a model
  if (!refNode->IsA("vtkMRMLModelNode") ) 
    {
    vtkErrorMacro("Reference node is not a vtkMRMLModelNode");
    return 0;
    }
  
  //vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(refNode);
  std::string newName = newFileName;
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("vtkMRMLFreeSurferModelNode: File name not specified");
    return 0;
    }
  if ( newName == std::string(""))
    {
    vtkErrorMacro("vtkMRMLFreeSurferModelNode: Copy-to file name not specified");
    return 0;
    }
  if (fullName == newName )
    {
    vtkWarningMacro("vtkMRMLFreeSurferModelNode: Copy-to file name and Copy-from file names are identical");
//    return 1;
    }

  //--- try copying to destination always
  copyOK = itksys::SystemTools::CopyAFile ( fullName.c_str(), newName.c_str(), 1 );
  //--- try copying to destination if different
  //tst = itksys::SystemTools::CopyAFile ( fullName.c_str(), newName.c_str(), 0 );
  
  if ( !copyOK )
    {
    return ( 0 );
    }
  
  //--- if copy worked, change filename, then upload.
  this->SetFileName ( newName.c_str() );
  this->StageWriteData(refNode);
  return 1;

}




//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelStorageNode::AddFileExtension(std::string ext)
{
  if (!IsKnownFileExtension(ext))
    {
    this->KnownFileExtensions.push_back(ext);
    }
}

//----------------------------------------------------------------------------
bool vtkMRMLFreeSurferModelStorageNode::IsKnownFileExtension(std::string ext)
{
  std::vector< std::string >::iterator iter;
  for (iter = this->KnownFileExtensions.begin(); iter != this->KnownFileExtensions.end(); ++iter)
    {
    if ((*iter) == ext)
      {
      return true;
      }
    }
  return false;  
}

//----------------------------------------------------------------------------
int vtkMRMLFreeSurferModelStorageNode::SupportedFileType(const char *fileName)
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
  if (this->IsKnownFileExtension(extension))
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelStorageNode::InitializeSupportedWriteFileTypes()
{
  // Look at WriteData()
  // support saving in vtk format
  this->SupportedWriteFileTypes->InsertNextValue("Poly Data (.vtk)");
  this->SupportedWriteFileTypes->InsertNextValue("XML Poly Data (.vtp)");  
}



/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFreeSurferModelStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/


#include "vtkObjectFactory.h"
#include "vtkMRMLFreeSurferModelStorageNode.h"
#include "vtkMRMLScene.h"

#include "vtkPolyDataNormals.h"
#include "vtkStripper.h"

#include "vtkFSSurfaceReader.h"
#include "vtkMRMLModelNode.h"

#include "vtkPolyDataWriter.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkPolyDataReader.h"
#include "vtkXMLPolyDataReader.h"



#include "itksys/SystemTools.hxx"

#include "vtkStringArray.h"

// Initialize static member that controls resampling --
// old comment: "This offset will be changed to 0.5 from 0.0 per 2/8/2002 Slicer
// development meeting, to move ijk coordinates to voxel centers."

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLFreeSurferModelStorageNode);

//----------------------------------------------------------------------------
vtkMRMLFreeSurferModelStorageNode::vtkMRMLFreeSurferModelStorageNode()
{
  this->UseStripper = 1;
}

//----------------------------------------------------------------------------
vtkMRMLFreeSurferModelStorageNode::~vtkMRMLFreeSurferModelStorageNode() = default;

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
  while (*atts != nullptr)
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
  this->Superclass::Copy(anode);

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
int vtkMRMLFreeSurferModelStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLModelNode *modelNode = dynamic_cast <vtkMRMLModelNode *> (refNode);
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("ReadDataInternal: File name not specified");
    return 0;
    }

  vtkDebugMacro("ReadDataInternal: reading " << fullName.c_str());

  // compute file prefix
  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  vtkDebugMacro("ReadDataInternal: extension = " << extension.c_str());

  // Node was saved with vtk extension.
  // Should be handled by vtkMRMLModelStorageNode.
  if (extension == ".vtk")
    {
    return Superclass::ReadDataInternal(refNode);
    }

  int result = 1;
  try
    {
    vtkDebugMacro("ReadDataInternal: Reading in a freesurfer surface file, extension = " << extension.c_str());

    //read in a free surfer file
    // -- create normals and triangle strips also
    vtkFSSurfaceReader *reader = vtkFSSurfaceReader::New();
    vtkPolyDataNormals *normals = vtkPolyDataNormals::New();
    vtkStripper *stripper = vtkStripper::New();

    reader->SetFileName(fullName.c_str());
    normals->SetSplitting(0);
    normals->SetInputConnection( reader->GetOutputPort() );
    if ( this->GetUseStripper() )
      {
      stripper->SetInputConnection( normals->GetOutputPort() );
      stripper->Update();
      if (stripper->GetOutput() == nullptr ||
          stripper->GetOutput()->GetNumberOfCells() == 0)
        {
        vtkDebugMacro("Surface file error: no output from triangle stripper.");
        result = 0;
        }
      else
        {
        modelNode->SetPolyDataConnection(stripper->GetOutputPort());
        }
      }
    else
      {
      normals->Update();
      if (normals->GetOutput() == nullptr ||
          normals->GetOutput()->GetNumberOfCells() == 0)
        {
        vtkDebugMacro("Surface file error: no output from the normals");
        result = 0;
        }
      else
        {
        modelNode->SetPolyDataConnection(normals->GetOutputPort());
        }
      }

    reader->Delete();
    normals->Delete();
    stripper->Delete();
    }
  catch (...)
    {
    result = 0;
    }

  if (modelNode->GetPolyData() != nullptr)
    {
    //modelNode->GetPolyData()->Modified();
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
  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLFreeSurferModelNode: File name not specified");
    return 0;
    }
  if ( newName.empty())
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
  copyOK = itksys::SystemTools::CopyAFile ( fullName.c_str(), newName.c_str(), true );
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
void vtkMRMLFreeSurferModelStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer Original (.orig)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer Inflated (.inflated)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer Sphere (.sphere)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer WhiteMatter (.white)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer SmoothWM (.smoothwm)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer Pial (.pial)");
}

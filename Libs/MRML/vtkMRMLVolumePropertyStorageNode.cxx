/*=Auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumePropertyStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkMRMLVolumePropertyNode.h"
#include "vtkMRMLVolumePropertyStorageNode.h"
#include "vtkMRMLScene.h"

#include <vtkColorTransferFunction.h>
#include <vtkObjectFactory.h>
#include <vtkPiecewiseFunction.h>
#include <vtkStringArray.h>
#include <vtkVolumeProperty.h>

#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLVolumePropertyStorageNode);

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyStorageNode::vtkMRMLVolumePropertyStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyStorageNode::~vtkMRMLVolumePropertyStorageNode()
{
}

void vtkMRMLVolumePropertyStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyStorageNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLStorageNode::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLVolumePropertyStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLStorageNode::PrintSelf(os,indent);

  os << indent << "FileName: " <<
    (this->FileName ? this->FileName : "(none)") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

//----------------------------------------------------------------------------
int vtkMRMLVolumePropertyStorageNode::ReadData(vtkMRMLNode *refNode)
{
  if (refNode == NULL)
    {
    vtkErrorMacro("ReadData: can't read into a null node");
    return 0;
    }

  // do not read if if we are not in the scene (for example inside snapshot)
  if (  !refNode->GetAddToScene() )
    {
    return 1;
    }

  if (this->GetScene() && this->GetScene()->GetReadDataOnLoad() == 0)
    {
    return 1;
    }

  if (!refNode->IsA("vtkMRMLVolumePropertyNode") ) 
    {
    //vtkErrorMacro("Reference node is not a vtkMRMLVolumePropertyNode");
    return 0;
    }

  Superclass::StageReadData(refNode);
  if ( this->GetReadState() != this->TransferDone )
    {
    // remote file download hasn't finished
    return 0;
    }
  
  vtkMRMLVolumePropertyNode *vpNode = dynamic_cast <vtkMRMLVolumePropertyNode *> (refNode);

  std::string fullName = this->GetFullNameFromFileName(); 
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  int result = 1;

  std::ifstream ifs;
#ifdef _WIN32
  ifs.open(fullName.c_str(), ios::binary | ios::in);
#else
  ifs.open(fullName.c_str(), ios::in);
#endif
  if ( !ifs )
    {
    vtkErrorMacro("Cannot open volume property file: " << fullName);
    return 0;
    }
  char line[1024];
  std::string sline;

  ifs.getline(line, 1024);
  sline = line;
  if (!sline.empty()) 
    {
    int value;
    std::stringstream ss;
    ss << sline;
    ss >> value;
    vpNode->GetVolumeProperty()->SetInterpolationType(value);
    }
  ifs.getline(line, 1024);
  sline = line;
  if (!sline.empty()) 
    {
    int value;
    std::stringstream ss;
    ss << sline;
    ss >> value;
    vpNode->GetVolumeProperty()->SetShade(value);
    }
  ifs.getline(line, 1024);
  sline = line;
  if (!sline.empty()) 
    {
    double value;
    std::stringstream ss;
    ss << sline;
    ss >> value;
    vpNode->GetVolumeProperty()->SetDiffuse(value);
    }
  ifs.getline(line, 1024);
  sline = line;
  if (!sline.empty()) 
    {
    double value;
    std::stringstream ss;
    ss << sline;
    ss >> value;
    vpNode->GetVolumeProperty()->SetAmbient(value);
    }
  ifs.getline(line, 1024);
  sline = line;
  if (!sline.empty()) 
    {
    double value;
    std::stringstream ss;
    ss << sline;
    ss >> value;
    vpNode->GetVolumeProperty()->SetSpecular(value);
    }
  ifs.getline(line, 1024);
  sline = line;
  if (!sline.empty()) 
    {
    double value;
    std::stringstream ss;
    ss << sline;
    ss >> value;
    vpNode->GetVolumeProperty()->SetSpecularPower(value);
    }

  ifs.getline(line, 1024);
  sline = line;
  if (!sline.empty()) 
    {
    vtkPiecewiseFunction *scalarOpacity=vtkPiecewiseFunction::New();
    vpNode->GetPiecewiseFunctionFromString(sline, scalarOpacity), 
    vpNode->SetScalarOpacity(scalarOpacity);
    scalarOpacity->Delete();
    }

  ifs.getline(line, 1024);
  sline = line;
  if (!sline.empty()) 
    {
    vtkPiecewiseFunction *gradientOpacity=vtkPiecewiseFunction::New();
    vpNode->GetPiecewiseFunctionFromString(sline, gradientOpacity);
    vpNode->SetGradientOpacity(gradientOpacity);
    gradientOpacity->Delete();
    }

  ifs.getline(line, 1024);
  sline = line;
  if (!sline.empty()) 
    {
    vtkColorTransferFunction *colorTransfer=vtkColorTransferFunction::New();
    vpNode->GetColorTransferFunctionFromString(sline, colorTransfer);
    vpNode->SetColor(colorTransfer);
    colorTransfer->Delete();
    }
  ifs.close();

  this->SetReadStateIdle();
   
  return result;
}

//----------------------------------------------------------------------------
int vtkMRMLVolumePropertyStorageNode::WriteData(vtkMRMLNode *refNode)
{
  if (refNode == NULL)
    {
    vtkErrorMacro("WriteData: can't write, input node is null");
    return 0;
    }

  if (!refNode->IsA("vtkMRMLVolumePropertyNode") ) 
    {
    //vtkErrorMacro("Reference node is not a vtkMRMLVolumePropertyNode");
    return 0;
    }

  vtkMRMLVolumePropertyNode *vpNode = vtkMRMLVolumePropertyNode::SafeDownCast(refNode);
  

  std::string fullName =  this->GetFullNameFromFileName();
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("vtkMRMLVolumePropertyStorageNode: File name not specified");
    return 0;
    }

  std::ofstream ofs;
#ifdef _WIN32
  ofs.open(fullName.c_str(), ios::binary | ios::out);
#else
  ofs.open(fullName.c_str(), ios::out);
#endif

  if ( !ofs )
    {
    vtkErrorMacro("Cannot open volume property file: " << fullName);
    return 0;
    }
  ofs << vpNode->GetVolumeProperty()->GetInterpolationType()  << std::endl;
  ofs << vpNode->GetVolumeProperty()->GetShade()  << std::endl;
  ofs << vpNode->GetVolumeProperty()->GetDiffuse()  << std::endl;
  ofs << vpNode->GetVolumeProperty()->GetAmbient()  << std::endl;
  ofs << vpNode->GetVolumeProperty()->GetSpecular()  << std::endl;
  ofs << vpNode->GetVolumeProperty()->GetSpecularPower()  << std::endl;
  ofs << vpNode->GetPiecewiseFunctionString(vpNode->GetVolumeProperty()->GetScalarOpacity())  << std::endl;
  ofs << vpNode->GetPiecewiseFunctionString(vpNode->GetVolumeProperty()->GetGradientOpacity())<< std::endl;
  ofs << vpNode->GetColorTransferFunctionString(vpNode->GetVolumeProperty()->GetRGBTransferFunction())<< std::endl;

  ofs.close();

  int result =1;
  return result;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("VolumePropperty (.vp)");
  this->SupportedWriteFileTypes->InsertNextValue("Text (.txt)");
  this->SupportedWriteFileTypes->InsertNextValue("VolumePropperty (.*)");
}

//----------------------------------------------------------------------------
int vtkMRMLVolumePropertyStorageNode::SupportedFileType(const char *fileName)
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

  return 1;
}

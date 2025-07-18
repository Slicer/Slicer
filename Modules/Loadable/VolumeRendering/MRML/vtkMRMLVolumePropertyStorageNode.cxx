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
#include "vtkMRMLI18N.h"
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
  this->DefaultWriteFileExtension = "vp";
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyStorageNode::~vtkMRMLVolumePropertyStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLVolumePropertyStorageNode::CanReadInReferenceNode(vtkMRMLNode* refNode)
{
  return refNode->IsA("vtkMRMLVolumePropertyNode");
}

//----------------------------------------------------------------------------
int vtkMRMLVolumePropertyStorageNode::ReadDataInternal(vtkMRMLNode* refNode)
{
  vtkMRMLVolumePropertyNode* vpNode = vtkMRMLVolumePropertyNode::SafeDownCast(refNode);

  const std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
  {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
  }

  const int result = 1;

  std::ifstream ifs;
#ifdef _WIN32
  ifs.open(fullName.c_str(), ios::binary | ios::in);
#else
  ifs.open(fullName.c_str(), ios::in);
#endif
  if (!ifs)
  {
    vtkErrorMacro("Cannot open volume property file: " << fullName);
    return 0;
  }
  std::string line;

  std::getline(ifs, line);
  if (!line.empty())
  {
    int value;
    std::stringstream ss;
    ss << line;
    ss >> value;
    vpNode->GetVolumeProperty()->SetInterpolationType(value);
  }
  std::getline(ifs, line);
  if (!line.empty())
  {
    int value;
    std::stringstream ss;
    ss << line;
    ss >> value;
    vpNode->GetVolumeProperty()->SetShade(value);
  }
  std::getline(ifs, line);
  if (!line.empty())
  {
    double value;
    std::stringstream ss;
    ss << line;
    ss >> value;
    vpNode->GetVolumeProperty()->SetDiffuse(value);
  }
  std::getline(ifs, line);
  if (!line.empty())
  {
    double value;
    std::stringstream ss;
    ss << line;
    ss >> value;
    vpNode->GetVolumeProperty()->SetAmbient(value);
  }
  std::getline(ifs, line);
  if (!line.empty())
  {
    double value;
    std::stringstream ss;
    ss << line;
    ss >> value;
    vpNode->GetVolumeProperty()->SetSpecular(value);
  }
  std::getline(ifs, line);
  if (!line.empty())
  {
    double value;
    std::stringstream ss;
    ss << line;
    ss >> value;
    vpNode->GetVolumeProperty()->SetSpecularPower(value);
  }

  std::getline(ifs, line);
  if (!line.empty())
  {
    const vtkNew<vtkPiecewiseFunction> scalarOpacity;
    vpNode->GetPiecewiseFunctionFromString(line, scalarOpacity), vpNode->SetScalarOpacity(scalarOpacity);
  }

  std::getline(ifs, line);
  if (!line.empty())
  {
    const vtkNew<vtkPiecewiseFunction> gradientOpacity;
    vpNode->GetPiecewiseFunctionFromString(line, gradientOpacity);
    vpNode->SetGradientOpacity(gradientOpacity);
  }

  std::getline(ifs, line);
  if (!line.empty())
  {
    const vtkNew<vtkColorTransferFunction> colorTransfer;
    vpNode->GetColorTransferFunctionFromString(line, colorTransfer);
    vpNode->SetColor(colorTransfer);
  }
  ifs.close();

  return result;
}

//----------------------------------------------------------------------------
int vtkMRMLVolumePropertyStorageNode::WriteDataInternal(vtkMRMLNode* refNode)
{
  vtkMRMLVolumePropertyNode* vpNode = vtkMRMLVolumePropertyNode::SafeDownCast(refNode);

  const std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
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

  if (!ofs)
  {
    vtkErrorMacro("Cannot open volume property file: " << fullName);
    return 0;
  }
  ofs << vpNode->GetVolumeProperty()->GetInterpolationType() << std::endl;
  ofs << vpNode->GetVolumeProperty()->GetShade() << std::endl;
  ofs << vpNode->GetVolumeProperty()->GetDiffuse() << std::endl;
  ofs << vpNode->GetVolumeProperty()->GetAmbient() << std::endl;
  ofs << vpNode->GetVolumeProperty()->GetSpecular() << std::endl;
  ofs << vpNode->GetVolumeProperty()->GetSpecularPower() << std::endl;
  ofs << vpNode->GetPiecewiseFunctionString(vpNode->GetVolumeProperty()->GetScalarOpacity()) << std::endl;
  ofs << vpNode->GetPiecewiseFunctionString(vpNode->GetVolumeProperty()->GetGradientOpacity()) << std::endl;
  ofs << vpNode->GetColorTransferFunctionString(vpNode->GetVolumeProperty()->GetRGBTransferFunction()) << std::endl;

  ofs.close();

  const int result = 1;
  return result;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyStorageNode::InitializeSupportedReadFileTypes()
{
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLVolumePropertyStorageNode", "MRML Volume Property") + " (.vp)");
}

//----------------------------------------------------------------------------
void vtkMRMLVolumePropertyStorageNode::InitializeSupportedWriteFileTypes()
{
  //: File format name
  this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLVolumePropertyStorageNode", "MRML Volume Property") + " (.vp)");
  //: File format name
  this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLVolumePropertyStorageNode", "MRML Volume Property") + " (.txt)");
}

/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLProceduralColorStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

// MRML include
#include "vtkMRMLProceduralColorStorageNode.h"
#include "vtkMRMLProceduralColorNode.h"
#include "vtkMRMLScene.h"

// VTK include
#include <vtkColorTransferFunction.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>

// STD include
#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLProceduralColorStorageNode);

//----------------------------------------------------------------------------
vtkMRMLProceduralColorStorageNode::vtkMRMLProceduralColorStorageNode()
{
  this->DefaultWriteFileExtension = "txt";
}

//----------------------------------------------------------------------------
vtkMRMLProceduralColorStorageNode::~vtkMRMLProceduralColorStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLProceduralColorStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLProceduralColorStorageNode::CanReadInReferenceNode(vtkMRMLNode* refNode)
{
  // FreeSurfer color nodes are special cases and are treated like
  // color table nodes
  return (refNode->IsA("vtkMRMLProceduralColorNode") &&
          !refNode->IsA("vtkMRMLFreeSurferProceduralColorNode"));
}

//----------------------------------------------------------------------------
int vtkMRMLProceduralColorStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();

  // cast the input node
  vtkMRMLProceduralColorNode *colorNode =
    vtkMRMLProceduralColorNode::SafeDownCast(refNode);

  if (colorNode == nullptr)
    {
    vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID()
                  << " to a known procedural color node");
    return 0;
    }

  vtkColorTransferFunction *ctf = colorNode->GetColorTransferFunction();
  if (!ctf)
    {
    vtkErrorMacro("ReadDataInternal: no color transfer function!");
    return 0;
    }

  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  if (extension == std::string(".txt"))
    {
    // open the file for reading input
    fstream fstr;

    fstr.open(fullName.c_str(), fstream::in);

    if (!fstr.is_open())
      {
      vtkErrorMacro("ERROR opening procedural colour file " << this->FileName << endl);
      return 0;
      }

    // clear out the node
    int wasModifying = colorNode->StartModify();
    colorNode->SetTypeToFile();
    colorNode->NamesInitialisedOff();
    ctf->RemoveAllPoints();

    char line[1024];

    while (fstr.good())
      {
      fstr.getline(line, 1024);

      // does it start with a #?
      if (line[0] == '#')
        {
        vtkDebugMacro("Comment line, skipping:\n\"" << line << "\"");
        }
      else
        {
        // is it empty?
        if (line[0] == '\0')
          {
          vtkDebugMacro("Empty line, skipping:\n\"" << line << "\"");
          }
        else
          {
          vtkDebugMacro("got a line: \n\"" << line << "\"");
          std::stringstream ss;
          ss << line;
          double x = 0.0, r = 0.0, g = 0.0, b = 0.0;
          // TBD: check that it's not a color table file!
          ss >> x;
          ss >> r;
          ss >> g;
          ss >> b;
          ctf->AddRGBPoint(x, r, g, b);
          }
        }
      }
    fstr.close();
    colorNode->EndModify(wasModifying);
    }
  else
    {
    vtkErrorMacro("ReadDataInternal: other extensions than .txt not supported yet! Can't read " << extension);
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLProceduralColorStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLProceduralColorStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkMRMLProceduralColorNode *colorNode = nullptr;
  if ( refNode->IsA("vtkMRMLProceduralColorNode") )
    {
    colorNode = dynamic_cast <vtkMRMLProceduralColorNode *> (refNode);
    }

  if (colorNode == nullptr)
    {
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known color table node");
    return 0;
    }

  vtkColorTransferFunction *ctf = colorNode->GetColorTransferFunction();
  if (!ctf)
    {
    vtkErrorMacro("WriteDataInternal: no color transfer function!");
    return 0;
    }

  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  if (extension == std::string(".txt"))
    {
    // open the file for writing
    fstream of;

    of.open(fullName.c_str(), fstream::out);

    if (!of.is_open())
      {
      vtkErrorMacro("WriteDataInternal: unable to open file " << fullName.c_str() << " for writing");
      return 0;
      }

    // put down a header
    of << "# Color procedural file " << (this->GetFileName() != nullptr ? this->GetFileName() : "null") << endl;
    int numPoints = ctf->GetSize();
    of << "# " << numPoints << " points" << endl;
    of << "# position R G B" << endl;
    for (int i = 0; i < numPoints; ++i)
      {
      double val[6];
      ctf->GetNodeValue(i, val);
      // val holds location, r, g, b, midpoint, sharpness
      of << val[0];
      of << " ";
      of << val[1];
      of << " ";
      of << val[2];
      of << " ";
      of << val[3];
      of << endl;
      }
    of.close();
    }
  else
    {
    vtkErrorMacro("WriteDataInternal: only .txt supported");
    return 0;
    }


  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLProceduralColorStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Color Function (.cxml)");
  this->SupportedReadFileTypes->InsertNextValue("Text (.txt)");
}

//----------------------------------------------------------------------------
void vtkMRMLProceduralColorStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Color Function (.cxml)");
  this->SupportedWriteFileTypes->InsertNextValue("Text (.txt)");
}

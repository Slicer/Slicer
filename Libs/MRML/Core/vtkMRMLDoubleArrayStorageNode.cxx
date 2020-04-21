/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiducialListStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLDoubleArrayStorageNode.h"
#include "vtkMRMLDoubleArrayNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLDoubleArrayStorageNode);

//----------------------------------------------------------------------------
vtkMRMLDoubleArrayStorageNode::vtkMRMLDoubleArrayStorageNode()
{
  this->DefaultWriteFileExtension = "mcsv";
}

//----------------------------------------------------------------------------
vtkMRMLDoubleArrayStorageNode::~vtkMRMLDoubleArrayStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLDoubleArrayStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLDoubleArrayStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLDoubleArrayNode");
}

//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();

  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLDoubleArrayStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkMRMLDoubleArrayNode *doubleArrayNode =
    vtkMRMLDoubleArrayNode::SafeDownCast(refNode);

  if (doubleArrayNode == nullptr)
    {
    vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID()
                  << " to a double array (measurement) node");
    return 0;
    }

  // open the file for reading input
  fstream fstr;
  fstr.open(fullName.c_str(), fstream::in);

  if (!fstr.is_open() || !fstr.good() || fstr.eof())
    {
    vtkErrorMacro("ERROR opening measurement file \"" << this->FileName << "\"");
    return 0;
    }

  char line[1024];
  // default column ordering for measurement info
  // independent_variable dependent_veariable err
  int xColumn = 0;
  int yColumn = 1;
  int zColumn = 2;

  int numColumns = 3;
  std::vector<std::string> labels;
  bool haslabels = true;

  while (fstr.good())
    {
    fstr.getline(line, 1024);

    // does it start with a #?
    if (line[0] == '#')
      {
      vtkDebugMacro("Comment line, checking:\n\"" << line << "\"");

      char *ptr;
      if (strncmp(line, " ", 1) != 0)
        {
        ptr = strtok(line, " ");
        ptr = strtok(nullptr, " ");
        if (strcmp(ptr, "nolabels") == 0)
          {
          haslabels = false;
          vtkDebugMacro("No labels in this file");
          }
        }
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
        char *ptr;
        bool reTokenise = false;
        if (strncmp(line, ",", 1) == 0)
          {
          ptr = nullptr;
          reTokenise = true;
          }
        else
          {
          ptr = strtok(line, ",");
          }
        double x = 0.0, y = 0.0, yerr = 0.0;
        int columnNumber = 0;

        while (columnNumber < numColumns)
          {
          if (ptr != nullptr)
            {
            if (columnNumber == xColumn)
              {
              if (haslabels)
                {
                labels.emplace_back(ptr);
                }
              else
                {
                x = atof(ptr);
                }
              }
            else if (columnNumber == yColumn)
              {
              if (haslabels)
                {
                labels.emplace_back(ptr);
                }
              else
                {
                y = atof(ptr);
                }
              }
            else if (columnNumber == zColumn)
              {
              if (haslabels)
                {
                labels.emplace_back(ptr);
                }
              else
                {
                yerr = atof(ptr);
                }
              }
            }
          if (reTokenise == false)
            {
            ptr = strtok(nullptr, ",");
            }
          else
            {
            ptr = strtok(line, ",");
            // turn it off
            reTokenise = false;
            }
          columnNumber++;
          } // end while over columns
        int   fidIndex;
        if (haslabels)
          {
          doubleArrayNode->vtkMRMLDoubleArrayNode::SetLabels(labels);
          haslabels = false;
          }
        else
          {
          fidIndex = doubleArrayNode->vtkMRMLDoubleArrayNode::AddXYValue(x, y, yerr);
          if (fidIndex == 0)
            {
            vtkErrorMacro("Error adding a measurement to the list");
            }
          }
        } // point line
      }
    }
  fstr.close();

  return doubleArrayNode->GetSize() > 0;
}

//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
    if (this->GetFileName() == nullptr)
    {
        vtkErrorMacro("WriteData: file name is not set");
        return 0;
    }

    std::string fullName = this->GetFullNameFromFileName();
    if (fullName.empty())
    {
        vtkErrorMacro("vtkMRMLDoubleArrayStorageNode: File name not specified");
        return 0;
    }

    // cast the input node
    vtkMRMLDoubleArrayNode *doubleArrayNode =
      vtkMRMLDoubleArrayNode::SafeDownCast(refNode);

    if (doubleArrayNode == nullptr)
    {
      vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known double array node");
      return 0;
    }
    if (doubleArrayNode->GetSize() == 0)
    {
      vtkErrorMacro("WriteData: " << refNode->GetID() << " is empty, no data to write in " << fullName.c_str());
      return 0;
    }

    // open the file for writing
    fstream of;
    of.open(fullName.c_str(), fstream::out);
    if (!of.is_open())
    {
        vtkErrorMacro("WriteData: unable to open file " << fullName.c_str() << " for writing");
        return 0;
    }

    // put down a header
    of << "# measurement file " << (this->GetFileName() != nullptr ? this->GetFileName() : "null") << endl;

    // put labels
    std::vector< std::string > labels = doubleArrayNode->GetLabels();
    if (labels.size())
    {
        for(unsigned int l = 0; l<labels.size()-1; l++)
        {
            of << labels.at(l) << ",";
        }
        of << labels.at(labels.size()-1) << endl;
    }
    else
    {
        of << "# nolabels"<< endl;
    }

    // put values
    for (unsigned int i = 0; i < doubleArrayNode->GetSize(); i++)
    {
        double x,y,yerr;
        if (doubleArrayNode->GetXYValue(i, &x, &y, &yerr))
        {

            of << x << "," << y << "," << yerr;
            of << endl;
        }
        else if (doubleArrayNode->GetXYValue(i, &x, &y))
        {

            of << x << "," << y ;
            of << endl;
        }
    }
    of.close();

    return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLDoubleArrayStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Measurement CSV (.mcsv)");
  this->SupportedReadFileTypes->InsertNextValue("Text (.txt)");
}

//----------------------------------------------------------------------------
void vtkMRMLDoubleArrayStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Measurement CSV (.mcsv)");
  this->SupportedWriteFileTypes->InsertNextValue("Text (.txt)");
}

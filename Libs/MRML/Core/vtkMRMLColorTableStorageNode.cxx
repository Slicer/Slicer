/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLColorTableStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

// MRML include
#include "vtkMRMLColorTableStorageNode.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLScene.h"

// VTK include
#include <vtkLookupTable.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>

// STD include
#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLColorTableStorageNode);

//----------------------------------------------------------------------------
vtkMRMLColorTableStorageNode::vtkMRMLColorTableStorageNode()
{
  // use 32K as a maximum color id for now
  this->MaximumColorID = 32768;
  this->DefaultWriteFileExtension = "ctbl";
}

//----------------------------------------------------------------------------
vtkMRMLColorTableStorageNode::~vtkMRMLColorTableStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLColorTableStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLColorTableStorageNode::CanReadInReferenceNode(vtkMRMLNode* refNode)
{
  return refNode->IsA("vtkMRMLColorTableNode");
}

//----------------------------------------------------------------------------
int vtkMRMLColorTableStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();

  // cast the input node
  vtkMRMLColorTableNode *colorNode =
    vtkMRMLColorTableNode::SafeDownCast(refNode);

  if (colorNode == nullptr)
    {
    vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID() << " to a known color table node");
    return 0;
    }
  // open the file for reading input
  fstream fstr;

  fstr.open(fullName.c_str(), fstream::in);

  if (fstr.is_open())
    {
    // clear out the table
    int wasModifying = colorNode->StartModify();

    // Set type to "File" by default if it has not been set yet.
    // It is important to only change type if it has not been set already
    // because otherwise "User" color node types would be always reverted to
    // read-only "File" type when the scene is saved and reloaded.
    if (colorNode->GetType()<colorNode->GetFirstType()
      || colorNode->GetType()>colorNode->GetLastType())
      {
      // no valid type has been set, set it to File
      colorNode->SetTypeToFile();
      }

    colorNode->NamesInitialisedOff();

    char line[1024];
    // save the valid lines in a vector, parse them once know the max id
    std::vector<std::string>lines;
    int maxID = 0;
    while (fstr.good())
      {
      fstr.getline(line, 1024);

      // does it start with a #?
      if (line[0] == '#')
        {
        vtkDebugMacro("Comment line, skipping:\n\"" << line << "\"");
        // sanity check: does the procedural header match?
        if (strncmp(line, "# Color procedural file", 23) == 0)
          {
          vtkErrorMacro("ReadDataInternal:\nfound a comment that this file "
                        << " is a procedural color file, returning:\n"
                        << line);
          return 0;
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
          lines.emplace_back(line);
          std::stringstream ss;
          ss << line;
          int id;
          ss >> id;
          if (id > maxID)
            {
            maxID = id;
            }
          }
        }
      }
    fstr.close();
    // now parse out the valid lines and set up the colour lookup table
    vtkDebugMacro("The largest id is " << maxID);
    if (maxID > this->MaximumColorID)
      {
      vtkErrorMacro("ReadData: maximum color id " << maxID << " is > "
                    << this->MaximumColorID << ", invalid color file: "
                    << this->GetFileName());
      colorNode->SetNumberOfColors(0);
      colorNode->EndModify(wasModifying);
      return 0;
      }
    // extra one for zero, also resizes the names array
    colorNode->SetNumberOfColors(maxID + 1);
    if (colorNode->GetLookupTable())
      {
      colorNode->GetLookupTable()->SetTableRange(0, maxID);
      }
    // init the table to black/opacity 0 with no name, just in case we're missing values
    const char *noName = colorNode->GetNoName();
    for (int i = 0; i < maxID+1; i++)
      {
      colorNode->SetColor(i, noName, 0.0, 0.0, 0.0, 0.0);
      }
    // We are sure that all the names are initialized here, flag it as such
    // to prevent unnecessary recomputation
    colorNode->NamesInitialisedOn();
    // do a little sanity check, if never get an rgb bigger than 1.0, report
    // it as a possibly miswritten file
    bool biggerThanOne = false;
    for (unsigned int i = 0; i < lines.size(); i++)
      {
      std::stringstream ss;
      ss << lines[i];
      int id = 0;
      std::string name;
      double r = 0.0, g = 0.0, b = 0.0, a = 0.0;
      ss >> id;
      ss >> name;
      ss >> r;
      ss >> g;
      ss >> b;
      // check that there's still something left in the stream
      if (ss.str().length() == 0)
        {
        std::cout << "Error in parsing line " << i << ", no alpha information!" << std::endl;
        }
      else
        {
        ss >> a;
        }
      if (!biggerThanOne &&
          (r > 1.0 || g > 1.0 || b > 1.0))
        {
        biggerThanOne = true;
        }
      // the file values are 0-255, colour look up table needs 0-1
      // clamp the colors just in case
      r = r > 255.0 ? 255.0 : r;
      r = r < 0.0 ? 0.0 : r;
      g = g > 255.0 ? 255.0 : g;
      g = g < 0.0 ? 0.0 : g;
      b = b > 255.0 ? 255.0 : b;
      b = b < 0.0 ? 0.0 : b;
      a = a > 255.0 ? 255.0 : a;
      a = a < 0.0 ? 0.0 : a;
      // now shift to 0-1
      r = r / 255.0;
      g = g / 255.0;
      b = b / 255.0;
      a = a / 255.0;
      // if the name has ticks around it, from copying from a mrml file, trim
      // them off the string
      if (name.find("'") != std::string::npos)
        {
        size_t firstnottick = name.find_first_not_of("'");
        size_t lastnottick = name.find_last_not_of("'");
        std::string withoutTicks = name.substr(firstnottick, (lastnottick-firstnottick) + 1);
        vtkDebugMacro("ReadDataInternal: Found ticks around name \"" << name << "\", using name without ticks instead:  \"" << withoutTicks << "\"");
        name = withoutTicks;
        }
      if (i < 10)
        {
        vtkDebugMacro("(first ten) Adding colour at id " << id << ", name = " << name.c_str() << ", r = " << r << ", g = " << g << ", b = " << b << ", a = " << a);
        }
      if (colorNode->SetColor(id, name.c_str(), r, g, b, a) == 0)
        {
        vtkWarningMacro("ReadData: unable to set color " << id << " with name " << name.c_str() << ", breaking the loop over " << lines.size() << " lines in the file " << this->FileName);
        colorNode->EndModify(wasModifying);
        return 0;
        }
      colorNode->SetColorNameWithSpaces(id, name.c_str(), "_");
      }
    if (lines.size() > 0 && !biggerThanOne)
      {
      vtkWarningMacro("ReadDataInternal: possibly malformed colour table file:\n" << this->FileName << ".\n\tNo RGB values are greater than 1. Valid values are 0-255");
      }
    colorNode->EndModify(wasModifying);
    }
  else
    {
    vtkErrorMacro("ERROR opening colour file " << this->FileName << endl);
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLColorTableStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLColorTableStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkMRMLColorTableNode *colorNode = nullptr;
  if ( refNode->IsA("vtkMRMLColorTableNode") )
    {
    colorNode = dynamic_cast <vtkMRMLColorTableNode *> (refNode);
    }

  if (colorNode == nullptr)
    {
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known color table node");
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
  of << "# Color table file " << (this->GetFileName() != nullptr ? this->GetFileName() : "null") << endl;
  if (colorNode->GetLookupTable() != nullptr)
    {
    of << "# " << colorNode->GetLookupTable()->GetNumberOfTableValues() << " values" << endl;
    for (int i = 0; i < colorNode->GetLookupTable()->GetNumberOfTableValues(); i++)
      {
      // is it an unnamed color?
      if (colorNode->GetNoName() &&
          colorNode->GetColorName(i) &&
          strcmp(colorNode->GetNoName(), colorNode->GetColorName(i)) == 0)
        {
        continue;
        }

      double *rgba;
      rgba = colorNode->GetLookupTable()->GetTableValue(i);
      // the colour look up table uses 0-1, file values are 0-255,
      double r = rgba[0] * 255.0;
      double g = rgba[1] * 255.0;
      double b = rgba[2] * 255.0;
      double a = rgba[3] * 255.0;
      of << i;
      of << " ";
      of << colorNode->GetColorNameWithoutSpaces(i, "_");
      of << " ";
      of << r;
      of << " ";
      of << g;
      of << " ";
      of << b;
      of << " ";
      of << a;
      of << endl;
      }
    }
  of.close();

  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Color Table (.ctbl)");
  this->SupportedReadFileTypes->InsertNextValue("Text (.txt)");
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Color Table (.ctbl)");
  this->SupportedWriteFileTypes->InsertNextValue("Text (.txt)");
}

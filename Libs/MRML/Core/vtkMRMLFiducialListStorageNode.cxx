/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiducialListStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

#include "vtkMRMLFiducialListStorageNode.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLScene.h"

#include "vtkObjectFactory.h"
#include "vtkStringArray.h"

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLFiducialListStorageNode);

//----------------------------------------------------------------------------
vtkMRMLFiducialListStorageNode::vtkMRMLFiducialListStorageNode()
{
  // version 2 has the new glyph symbol numbering, which starts at 1
  this->Version = 2;
  this->DefaultWriteFileExtension = "fcsv";
}

//----------------------------------------------------------------------------
vtkMRMLFiducialListStorageNode::~vtkMRMLFiducialListStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLFiducialListStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLFiducialListStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLFiducialListNode");
}

//----------------------------------------------------------------------------
int vtkMRMLFiducialListStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();

  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLFiducialListStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkMRMLFiducialListNode *fiducialListNode =
    vtkMRMLFiducialListNode::SafeDownCast(refNode);

  // open the file for reading input
  fstream fstr;

  fstr.open(fullName.c_str(), fstream::in);

  if (fstr.is_open())
    {
    // with the change to VTK 5.6, the glyph type is off by one for old files,
    // make sure that we don't have a new MRML scene with and old .fcsv file.
    int mrmlGlyphType = -1;
    // set the version number to undefined
    this->SetVersion(-1);
    if (fiducialListNode->GetNumberOfFiducials() > 0)
      {
      mrmlGlyphType = fiducialListNode->GetGlyphType();
      // clear out the list
      fiducialListNode->RemoveAllFiducials();
      }

    // turn off modified events
//    int modFlag = fiducialListNode->GetDisableModifiedEvent();
//    fiducialListNode->DisableModifiedEventOn();
    char line[1024];
    // default column ordering for fiducial info
    // first pass: line will have label,x,y,z,selected,visible
    int labelColumn = 0;
    int xColumn = 1;
    int yColumn = 2;
    int zColumn = 3;
    int selColumn = 4;
    int visColumn = 5;
    int numColumns = 6;
    // save the valid lines in a vector, parse them once know the max id
    std::vector<std::string>lines;
    while (fstr.good())
      {
      fstr.getline(line, 1024);

      // does it start with a #?
      if (line[0] == '#')
        {
        vtkDebugMacro("Comment line, checking:\n\"" << line << "\"");
        // TODO: parse out the display node settings
        // if there's a space after the hash, try to find options
        if (line[1] == ' ')
          {
          vtkDebugMacro("Have a possible option in line " << line);
          std::string lineString = std::string(line);
          if (lineString.find("# numPoints = ") != std::string::npos)
            {
            vtkDebugMacro("Skipping numPoints");
            }
          else if (lineString.find("# name = ") != std::string::npos)
            {
            std::string str = lineString.substr(9,std::string::npos);
            vtkDebugMacro("Getting name, substr = " << str);
            fiducialListNode->SetName(str.c_str());
            }
          else if (lineString.find("# symbolScale = ") != std::string::npos)
            {
            std::string str = lineString.substr(16,std::string::npos);
            vtkDebugMacro("Getting symbolScale, substr = " << str);
            float scale = atof(str.c_str());
            fiducialListNode->SetSymbolScale(scale);
            }
          else if (lineString.find("# version = ") != std::string::npos)
            {
            std::string str = lineString.substr(12,std::string::npos);
            vtkDebugMacro("Getting version, substr = " << str);
            int ver = atoi(str.c_str());
            this->SetVersion(ver);
            }
          else if (lineString.find("# symbolType = ") != std::string::npos)
            {
            std::string str = lineString.substr(15,std::string::npos);
            vtkDebugMacro("Getting symbolType, substr = " << str);
            int t = atoi(str.c_str());
            fiducialListNode->SetGlyphType(t);
            }
          else if (lineString.find("# visibility = ") != std::string::npos)
            {
            std::string str = lineString.substr(15,std::string::npos);
            vtkDebugMacro("Getting visibility, substr = " << str);
            int vis = atoi(str.c_str());
            fiducialListNode->SetVisibility(vis);
            }
          else if (lineString.find("# textScale = ") != std::string::npos)
            {
            std::string str = lineString.substr(14, std::string::npos);
            vtkDebugMacro("Getting textScale, substr = " << str.c_str());
            float scale = atof(str.c_str());
            fiducialListNode->SetTextScale(scale);
            }
          else if (lineString.find("# color = ") != std::string::npos ||
                   lineString.find("# selectedColor = ") != std::string::npos)
            {
            std::string str;
            if (lineString.find("# color = ") != std::string::npos)
              {
              str = lineString.substr(10, std::string::npos);
              }
            else
              {
              str = lineString.substr(18, std::string::npos);
              }
            vtkDebugMacro("Getting colors, substr = " << str.c_str());
            // the rgb values are separated by commas
            float r = 0.0, g = 0.0, b = 0.0;
            char *ptr;
            char *colours = (char *)(str.c_str());
            ptr = strtok(colours, ",");
            if (ptr != nullptr)
              {
              r = atof(ptr);
              }
            ptr = strtok(nullptr, ",");
            if (ptr != nullptr)
              {
              g = atof(ptr);
              }
            ptr = strtok(nullptr, ",");
            if (ptr != nullptr)
              {
              b = atof(ptr);
              }
            // now set the correct value
            if (lineString.find("# color = ") != std::string::npos)
              {
              fiducialListNode->SetColor(r,g,b);
              }
            else
              {
              fiducialListNode->SetSelectedColor(r,g,b);
              }
            }
          else if (lineString.find("# opacity = ") != std::string::npos)
            {
            std::string str = lineString.substr(12, std::string::npos);
            vtkDebugMacro("Getting opacity, substr = " << str.c_str());
            float val = atof(str.c_str());
            fiducialListNode->SetOpacity(val);
            }
          else if (lineString.find("# ambient = ") != std::string::npos)
            {
            std::string str = lineString.substr(12, std::string::npos);
            vtkDebugMacro("Getting ambient, substr = " << str.c_str());
            float val = atof(str.c_str());
            fiducialListNode->SetAmbient(val);
            }
          else if (lineString.find("# diffuse = ") != std::string::npos)
            {
            std::string str = lineString.substr(12, std::string::npos);
            vtkDebugMacro("Getting diffuse, substr = " << str.c_str());
            float val = atof(str.c_str());
            fiducialListNode->SetDiffuse(val);
            }
          else if (lineString.find("# specular = ") != std::string::npos)
            {
            std::string str = lineString.substr(13, std::string::npos);
            vtkDebugMacro("Getting specular, substr = " << str.c_str());
            float val = atof(str.c_str());
            fiducialListNode->SetSpecular(val);
            }
          else if (lineString.find("# power = ") != std::string::npos)
            {
            std::string str = lineString.substr(10, std::string::npos);
            vtkDebugMacro("Getting power, substr = " << str.c_str());
            float val = atof(str.c_str());
            fiducialListNode->SetPower(val);
            }
          else if (lineString.find("# opacity = ") != std::string::npos)
            {
            std::string str = lineString.substr(12, std::string::npos);
            vtkDebugMacro("Getting opacity, substr = " << str.c_str());
            float val = atof(str.c_str());
            fiducialListNode->SetOpacity(val);
            }
          else if (lineString.find("# locked = ") != std::string::npos)
            {
            std::string str = lineString.substr(10, std::string::npos);
            vtkDebugMacro("Getting locked, substr = " << str.c_str());
            int val = atoi(str.c_str());
            fiducialListNode->SetLocked(val);
            }
          else if (lineString.find("# numberingScheme = ") != std::string::npos)
            {
            std::string str = lineString.substr(10, std::string::npos);
            vtkDebugMacro("Getting numberingScheme, substr = " << str.c_str());
            int val = atoi(str.c_str());
            fiducialListNode->SetNumberingScheme(val);
            }
          else if (lineString.find("# columns = ") != std::string::npos)
            {
            std::string str = lineString.substr(12, std::string::npos);
            vtkDebugMacro("Getting column order for the fids, substr = " << str.c_str());
            // reset all of them
            labelColumn = -1, xColumn = -1, yColumn = -1, zColumn = -1, selColumn = -1, visColumn = -1;
            int columnNumber = 0;
            char *columns = (char *)str.c_str();
            char *ptr = strtok(columns, ",");
            while (ptr != nullptr)
              {
              if (strcmp(ptr, "label") == 0)
                {
                labelColumn = columnNumber;
                }
              else if (strcmp(ptr, "x") == 0)
                {
                xColumn = columnNumber;
                }
              else if (strcmp(ptr, "y") == 0)
                {
                yColumn = columnNumber;
                }
              else if (strcmp(ptr, "z") == 0)
                {
                zColumn = columnNumber;
                }
              else if (strcmp(ptr, "sel") == 0)
                {
                selColumn = columnNumber;
                }
              else if (strcmp(ptr, "vis" ) == 0)
                {
                visColumn = columnNumber;
                }
              ptr = strtok(nullptr, ",");
              columnNumber++;
              }
            // set the total number of columns
            numColumns = columnNumber;
            vtkDebugMacro("Got " << numColumns << " columns, label = " << labelColumn << ", x = " << xColumn << ", y = " << yColumn << ", z = " << zColumn << ", sel = " <<  selColumn << ", vis = " << visColumn);
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
          // if the label text is missing, strtok will return a pointer to the
          // x value, will need to call strtok with the line on the next try
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
          std::string label = std::string("");
          double x = 0.0, y = 0.0, z = 0.0;
          int sel = 1, vis = 1;
          int columnNumber = 0;
          while (columnNumber < numColumns)
            {
            if (ptr != nullptr)
              {
              if (columnNumber == labelColumn)
                {
                label = std::string(ptr);
                }
              else if (columnNumber == xColumn)
                {
                x = atof(ptr);
                }
              else if (columnNumber == yColumn)
                {
                y = atof(ptr);
                }
              else if (columnNumber == zColumn)
                {
                z = atof(ptr);
                }
              else if (columnNumber == selColumn)
                {
                sel = atoi(ptr);
                }
              else if (columnNumber == visColumn)
                {
                vis = atoi(ptr);
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
          int fidIndex = fiducialListNode->AddFiducialWithLabelXYZSelectedVisibility(label.c_str(), x, y, z, sel, vis);
          if (fidIndex == -1)
            {
            vtkErrorMacro("Error adding fiducial to list, label = " << label.c_str());
            }
          } // point line
        }
      }
//    fiducialListNode->SetDisableModifiedEvent(modFlag);
//    fiducialListNode->InvokeEvent(vtkMRMLScene::NodeAddedEvent, fiducialListNode);//vtkMRMLFiducialListNode::DisplayModifiedEvent);
    fstr.close();

    // check on version number
    // at svn version 12553, the symbol type changed by one. There was a bit of
    // a time gap and then added a fcsv file version to also check against
    int glyphType = fiducialListNode->GetGlyphType();
    if (this->GetScene() && this->GetScene()->GetLastLoadedVersion())
      {
      if (this->GetScene()->GetLastLoadedVersion() != std::string(""))
        {
        if (atoi(this->GetScene()->GetLastLoadedVersion()) < 12553)
          {
          vtkDebugMacro("ReadData: last loaded version '" << this->GetScene()->GetLastLoadedVersion() << "' is less than 12553, incrementing glyph type from " << glyphType);
          glyphType++;
          }
        else
          {
          // could also be off by one if it's an old .fcsv file in a new
          // mrml scene
          if (mrmlGlyphType != -1 &&
              mrmlGlyphType - glyphType == 1 &&
              this->GetVersion() == -1)
            {
            vtkDebugMacro("ReadData: the glyph type in the mrml version is " << mrmlGlyphType << ", but in the file it's " << glyphType << ", using the mrml version");
            glyphType = mrmlGlyphType;
            }
          }
        }
      else
        {
        vtkDebugMacro("ReadData: no last loaded version number on scene, glyph type = " << glyphType);
        if (this->GetVersion() == -1)
          {
          vtkDebugMacro("ReadData: no storage node version found in the file, assuming it's an old one and incrementing the glyph type");
          glyphType++;
          }
        }
      }
    if (glyphType != fiducialListNode->GetGlyphType())
      {
      vtkWarningMacro("ReadData: updating glyph type from " <<  fiducialListNode->GetGlyphType() << " to " << glyphType << " for Slicer3.6");
      fiducialListNode->SetGlyphType(glyphType);
      // now it's set to be the current version
      this->SetVersion(2);
      }
    }
  else
    {
    vtkErrorMacro("ERROR opening fiducials file " << this->FileName << endl);
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLFiducialListStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLFiducialListStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkMRMLFiducialListNode *fiducialListNode = nullptr;
  if ( refNode->IsA("vtkMRMLFiducialListNode") )
    {
    fiducialListNode = dynamic_cast <vtkMRMLFiducialListNode *> (refNode);
    }

  if (fiducialListNode == nullptr)
    {
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known fiducial list node");
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
  of << "# Fiducial List file " << (this->GetFileName() != nullptr ? this->GetFileName() : "null") << endl;
  of << "# version = " << this->GetVersion() << endl;
  of << "# name = " << fiducialListNode->GetName() << endl;
  of << "# numPoints = " << fiducialListNode->GetNumberOfFiducials() << endl;
  of << "# symbolScale = " << fiducialListNode->GetSymbolScale() << endl;
  of << "# symbolType = " << fiducialListNode->GetGlyphType() << endl;
  of << "# visibility = " << fiducialListNode->GetVisibility() << endl;
  of << "# textScale = " << fiducialListNode->GetTextScale() << endl;
  double *colour = fiducialListNode->GetColor();
  of << "# color = " << colour[0] << "," << colour[1] << "," << colour[2] << endl;
  colour = fiducialListNode->GetSelectedColor();
  of << "# selectedColor = " << colour[0] << "," << colour[1] << "," << colour[2] << endl;
  of << "# opacity = " << fiducialListNode->GetOpacity() << endl;
  of << "# ambient = " << fiducialListNode->GetAmbient() << endl;
  of << "# diffuse = " << fiducialListNode->GetDiffuse() << endl;
  of << "# specular = " << fiducialListNode->GetSpecular() << endl;
  of << "# power = " << fiducialListNode->GetPower() << endl;
  of << "# locked = " << fiducialListNode->GetLocked() << endl;
  of << "# numberingScheme = " << fiducialListNode->GetNumberingScheme() << endl;

  // if change the ones being included, make sure to update the parsing in ReadData
  of << "# columns = label,x,y,z,sel,vis" << endl;
  for (int i = 0; i < fiducialListNode->GetNumberOfFiducials(); i++)
    {
    float *xyz = fiducialListNode->GetNthFiducialXYZ(i);
    // for now, skip orientation
    //float *xyzw = fiducialListNode->GetNthFiducialOrientation(i);
    const char *label = fiducialListNode->GetNthFiducialLabelText(i);
    int sel = fiducialListNode->GetNthFiducialSelected(i);
    int vis = fiducialListNode->GetNthFiducialVisibility(i);
    of << label;
    of << "," << xyz[0] << "," << xyz[1] << "," << xyz[2];
    of << "," << sel << "," << vis;
    of << endl;
    }
  of.close();

  return 1;

}

//----------------------------------------------------------------------------
void vtkMRMLFiducialListStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Text (.txt)");
}

//----------------------------------------------------------------------------
void vtkMRMLFiducialListStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Fiducial List CSV (.fcsv)");
}

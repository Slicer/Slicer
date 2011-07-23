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
#include <vtkImageChangeInformation.h>
#include <vtkLookupTable.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>

// STD include
#include <string>
#include <iostream>
#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLColorTableStorageNode* vtkMRMLColorTableStorageNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLColorTableStorageNode");
  if(ret)
    {
    return (vtkMRMLColorTableStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLColorTableStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLColorTableStorageNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLColorTableStorageNode");
  if(ret)
    {
    return (vtkMRMLColorTableStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLColorTableStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLColorTableStorageNode::vtkMRMLColorTableStorageNode()
{
  // use 32K as a maximum color id for now 
  this->MaximumColorID = 32768;
  this->InitializeSupportedWriteFileTypes();
}

//----------------------------------------------------------------------------
vtkMRMLColorTableStorageNode::~vtkMRMLColorTableStorageNode()
{
}

void vtkMRMLColorTableStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  std::stringstream ss;
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableStorageNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

/*
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
*/
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLColorTableStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  //vtkMRMLColorTableStorageNode *node = (vtkMRMLColorTableStorageNode *) anode;
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{  
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
int vtkMRMLColorTableStorageNode::ReadData(vtkMRMLNode *refNode)
{
  if (refNode == NULL)
    {
    vtkErrorMacro("ReadData: can't read into a null node");
    return 0;
    }

  // do not read if if we are not in the scene (for example inside snapshot)
  if ( !refNode->GetAddToScene() )
    {
    return 1;
    }

  if (this->GetScene() && this->GetScene()->GetReadDataOnLoad() == 0)
    {
    return 1;
    }

  vtkDebugMacro("Reading ColorTable data");
  // test whether refNode is a valid node to hold a color table
  if ( !( refNode->IsA("vtkMRMLColorTableNode"))
     ) 
    {
    vtkErrorMacro("Reference node is not a proper vtkMRMLColorNode");
    return 0;         
    }

  if (this->GetFileName() == NULL && this->GetURI() == NULL) 
    {
    vtkErrorMacro("ReadData: file name and uri not set");
    return 0;
    }

  Superclass::StageReadData(refNode);
  if ( this->GetReadState() != this->TransferDone )
    {
    // remote file download hasn't finished
    vtkWarningMacro("ReadData: Read state is pending, returning.");
    return 0;
    }
  
  std::string fullName = this->GetFullNameFromFileName(); 

  if (fullName == std::string("")) 
    {
    vtkErrorMacro("vtkMRMLColorTableStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkMRMLColorTableNode *colorNode = NULL;
  if ( refNode->IsA("vtkMRMLColorTableNode") )
    {
    colorNode = dynamic_cast <vtkMRMLColorTableNode *> (refNode);
    }

  if (colorNode == NULL)
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
    colorNode->SetTypeToFile();
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
          lines.push_back(std::string(line));
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
    colorNode->GetLookupTable()->SetTableRange(0, maxID);
    // init the table to black/opactity 0, just in case we're missing values
    for (int i = 0; i < maxID+1; i++)
      {
      colorNode->SetColor(i, 0.0, 0.0, 0.0, 0.0);
      }
    // We are sure that all the names are initialized here, flag it as such
    // to prevent unnecessary recomputation
    colorNode->NamesInitialisedOn();
    for (unsigned int i = 0; i < lines.size(); i++)
      {
      std::stringstream ss;
      ss << lines[i];
      int id;
      std::string name;
      double r, g, b, a;
      ss >> id;
      ss >> name;
      ss >> r;
      ss >> g;
      ss >> b;
      ss >> a;
      
      // the file values are 0-255, colour look up table needs 0-1
      r = r / 255.0;
      g = g / 255.0;
      b = b / 255.0;
      a = a / 255.0;
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
      }
    colorNode->EndModify(wasModifying);
    }
  else
    {
    vtkErrorMacro("ERROR opening colour file " << this->FileName << endl);
    return 0;
    }
  
  this->SetReadStateIdle();
  
  // make sure that the color node points to this storage node
  colorNode->SetAndObserveStorageNodeID(this->GetID());
  
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLColorTableStorageNode::WriteData(vtkMRMLNode *refNode)
{
  if (refNode == NULL)
    {
    vtkErrorMacro("WriteData: can't write, input node is null");
    return 0;
    }

  // test whether refNode is a valid node to hold a volume
  if ( !( refNode->IsA("vtkMRMLColorTableNode") ) )
    {
    vtkErrorMacro("Reference node is not a proper vtkMRMLColorTableNode");
    return 0;         
    }

  if (this->GetFileName() == NULL) 
    {
    vtkErrorMacro("ReadData: file name is not set");
    return 0;
    }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("vtkMRMLColorTableStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkMRMLColorTableNode *colorNode = NULL;
  if ( refNode->IsA("vtkMRMLColorTableNode") )
    {
    colorNode = dynamic_cast <vtkMRMLColorTableNode *> (refNode);
    }

  if (colorNode == NULL)
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
  of << "# Color table file " << (this->GetFileName() != NULL ? this->GetFileName() : "null") << endl;
  of << "# " << colorNode->GetLookupTable()->GetNumberOfTableValues() << " values" << endl;
  for (int i = 0; i < colorNode->GetLookupTable()->GetNumberOfTableValues(); i++)
    {
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
  of.close();

  Superclass::StageWriteData(refNode);
  
  return 1;
  
}

//----------------------------------------------------------------------------
int vtkMRMLColorTableStorageNode::SupportedFileType(const char *fileName)
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
  if (extension.compare(".ctbl") == 0 ||
      extension.compare(".txt") == 0)
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableStorageNode::InitializeSupportedWriteFileTypes()
{
  Superclass::InitializeSupportedWriteFileTypes();
  this->SupportedWriteFileTypes->InsertNextValue("Color Table (.ctbl)");
  this->SupportedWriteFileTypes->InsertNextValue("Text (.txt)");
}

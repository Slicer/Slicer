/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLColorTableStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkImageChangeInformation.h"
#include "vtkMRMLColorTableStorageNode.h"
#include "vtkMRMLColorTableNode.h"

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
  vtkMRMLColorTableStorageNode *node = (vtkMRMLColorTableStorageNode *) anode;
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{  
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

//----------------------------------------------------------------------------

int vtkMRMLColorTableStorageNode::ReadData(vtkMRMLNode *refNode)
{
  vtkDebugMacro("Reading ColorTable data");
  // test whether refNode is a valid node to hold a color table
  if ( !( refNode->IsA("vtkMRMLColorTableNode"))
     ) 
    {
    vtkErrorMacro("Reference node is not a proper vtkMRMLColorNode");
    return 0;         
    }

  if (this->GetFileName() == NULL) 
    {
    vtkErrorMacro("ReadData: file name is not set");
    return 0;
    }

  std::string fullName;
  if (this->SceneRootDir != NULL && this->Scene->IsFilePathRelative(this->GetFileName())) 
    {
    fullName = std::string(this->SceneRootDir) + std::string(this->GetFileName());
    }
  else 
    {
    fullName = std::string(this->GetFileName());
    }

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
    colorNode->SetTypeToFile();
    colorNode->GetLookupTable()->SetNumberOfTableValues(0);
    colorNode->ClearNames();
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
    colorNode->SetNumberOfColors(maxID + 1); // extra one for zero, also
                                             // resizes the names array
    colorNode->GetLookupTable()->SetNumberOfColors(maxID + 1);
    colorNode->GetLookupTable()->SetTableRange(0, maxID);
    // init the table to black/opactity 0, just in case we're missing values
    for (int i = 0; i < maxID+1; i++)
      {
      colorNode->GetLookupTable()->SetTableValue(i, 0.0, 0.0, 0.0, 0.0);
      } 
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
      vtkDebugMacro("id " << id << ", name = " << name.c_str() << ", r = " << r << ", g = " << g << ", b = " << b << ", a = " << a);
      // the file values are 0-255, colour look up table needs 0-1
      r = r / 255.0;
      g = g / 255.0;
      b = b / 255.0;
      a = a / 255.0;
      vtkDebugMacro("Adding colour at id " << id << " and then pushing name " << name.c_str());
      colorNode->GetLookupTable()->SetTableValue(id, r, g, b, a);
      colorNode->SetColorName(id, name.c_str());
      }
    colorNode->NamesInitialisedOn();
    }
  else
    {
    vtkErrorMacro("ERROR opening colour file " << this->FileName << endl);
    return 0;
    }

  // make sure that the color node points to this storage node
  colorNode->SetStorageNodeID(this->GetID());
  
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLColorTableStorageNode::WriteData(vtkMRMLNode *refNode)
{

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

  std::string fullName;
  if (this->SceneRootDir != NULL && this->Scene->IsFilePathRelative(this->GetFileName())) 
    {
    fullName = std::string(this->SceneRootDir) + std::string(this->GetFileName());
    }
  else 
    {
    fullName = std::string(this->GetFileName());
    }

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
  of << "# Color table file " << this->GetFileName() << endl;
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

  return 1;
  
}

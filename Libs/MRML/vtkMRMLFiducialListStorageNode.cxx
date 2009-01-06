/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiducialListStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkImageChangeInformation.h"
#include "vtkMRMLFiducialListStorageNode.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkStringArray.h"

//------------------------------------------------------------------------------
vtkMRMLFiducialListStorageNode* vtkMRMLFiducialListStorageNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiducialListStorageNode");
  if(ret)
    {
    return (vtkMRMLFiducialListStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiducialListStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLFiducialListStorageNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiducialListStorageNode");
  if(ret)
    {
    return (vtkMRMLFiducialListStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiducialListStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLFiducialListStorageNode::vtkMRMLFiducialListStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLFiducialListStorageNode::~vtkMRMLFiducialListStorageNode()
{
}

void vtkMRMLFiducialListStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  std::stringstream ss;
}

//----------------------------------------------------------------------------
void vtkMRMLFiducialListStorageNode::ReadXMLAttributes(const char** atts)
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
void vtkMRMLFiducialListStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  //vtkMRMLFiducialListStorageNode *node = (vtkMRMLFiducialListStorageNode *) anode;
}

//----------------------------------------------------------------------------
void vtkMRMLFiducialListStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{  
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLFiducialListStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

//----------------------------------------------------------------------------
int vtkMRMLFiducialListStorageNode::ReadData(vtkMRMLNode *refNode)
{
  // do not read if if we are not in the scene (for example inside snapshot)
  if ( !this->GetAddToScene() || !refNode->GetAddToScene() )
    {
    return 1;
    }

  vtkDebugMacro("Reading Fiducial list data");
  // test whether refNode is a valid node to hold a color table
  if ( !( refNode->IsA("vtkMRMLFiducialListNode"))
     ) 
    {
    vtkErrorMacro("Reference node is not a proper vtkMRMLFiducialListNode");
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
    vtkErrorMacro("vtkMRMLFiducialListStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkMRMLFiducialListNode *fiducialListNode = NULL;
  if ( refNode->IsA("vtkMRMLFiducialListNode") )
    {
    fiducialListNode = dynamic_cast <vtkMRMLFiducialListNode *> (refNode);
    }

  if (fiducialListNode == NULL)
    {
    vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID() << " to a fiducial list node");
    return 0;
    }
  // open the file for reading input
  fstream fstr;

  fstr.open(fullName.c_str(), fstream::in);

  if (fstr.is_open())
    {
    // clear out the list
    fiducialListNode->RemoveAllFiducials();
    char line[1024];
    // save the valid lines in a vector, parse them once know the max id
    std::vector<std::string>lines;
    while (fstr.good())
      {
      fstr.getline(line, 1024);
      
      // does it start with a #?
      if (line[0] == '#')
        {
        vtkDebugMacro("Comment line, skipping:\n\"" << line << "\"");
        // TODO: parse out the display node settings
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
          // first pass: line will have label,x,y,z,selected,visible
          // TODO: parse out the header line
          char *ptr;
          ptr = strtok(line, ",");
          std::string label = std::string("");
          double x = 0.0, y = 0.0, z = 0.0;
          int sel = 1, vis = 1;
          if (ptr != NULL)
            {
            label = std::string(ptr);
            ptr = strtok(NULL, ",");
            }
          if (ptr != NULL)
            {
            x = atof(ptr);
            ptr = strtok(NULL, ",");
            }
          if (ptr != NULL)
            {
            y = atof(ptr);
            ptr = strtok(NULL, ",");
            }
          if (ptr != NULL)
            {
            z = atof(ptr);
            ptr = strtok(NULL, ",");
            }
          if (ptr != NULL)
            {
            sel = atoi(ptr);
            ptr = strtok(NULL, ",");
            }
          if (ptr != NULL)
            {
            vis = atoi(ptr);
            ptr = strtok(NULL, ",");
            }
          int fidIndex = fiducialListNode->AddFiducialWithLabelXYZSelectedVisibility(label.c_str(), x, y, z, sel, vis);
          if (fidIndex == -1)
            {
            vtkErrorMacro("Error adding fiducial to list, label = " << label.c_str());
            }
             }
        }
      }
    fstr.close();
    }
  else
    {
    vtkErrorMacro("ERROR opening colour file " << this->FileName << endl);
    return 0;
    }
  
  this->SetReadStateIdle();
  
  // make sure that the color node points to this storage node
  fiducialListNode->SetAndObserveStorageNodeID(this->GetID());
  
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLFiducialListStorageNode::WriteData(vtkMRMLNode *refNode)
{

  // test whether refNode is a valid node to hold a volume
  if ( !( refNode->IsA("vtkMRMLFiducialListNode") ) )
    {
    vtkErrorMacro("Reference node is not a proper vtkMRMLFiducialListNode");
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
    vtkErrorMacro("vtkMRMLFiducialListStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkMRMLFiducialListNode *fiducialListNode = NULL;
  if ( refNode->IsA("vtkMRMLFiducialListNode") )
    {
    fiducialListNode = dynamic_cast <vtkMRMLFiducialListNode *> (refNode);
    }

  if (fiducialListNode == NULL)
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
  of << "# Fiducial List file " << (this->GetFileName() != NULL ? this->GetFileName() : "null") << endl;
  of << "# numPoints = " << fiducialListNode->GetNumberOfFiducials() << endl;
  of << "# symbolScale = " << fiducialListNode->GetSymbolScale() << endl;
  of << "# visibility = " << fiducialListNode->GetVisibility() << endl;
  of << "# textScale = " << fiducialListNode->GetTextScale() << endl;
  double *colour = fiducialListNode->GetColor();
  of << "# color = " << colour[0] << "," << colour[1] << "," << colour[2] << endl;
  colour = fiducialListNode->GetSelectedColor();
  of << "# selectedColor = " << colour[0] << "," << colour[1] << "," << colour[2] << endl;
  
  of << "#label,x,y,z,sel,vis" << endl;
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

  Superclass::StageWriteData(refNode);
  
  return 1;
  
}

//----------------------------------------------------------------------------
int vtkMRMLFiducialListStorageNode::SupportedFileType(const char *fileName)
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
  if (extension.compare(".fcsv") == 0 ||
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
void vtkMRMLFiducialListStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Fiducial List CSV (.fcsv)");
  this->SupportedWriteFileTypes->InsertNextValue("Text (.txt)");
}

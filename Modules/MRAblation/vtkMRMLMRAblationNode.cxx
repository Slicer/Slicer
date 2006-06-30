/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLMRAblationNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLMRAblationNode.h"
#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkMRMLMRAblationNode* vtkMRMLMRAblationNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLMRAblationNode");
  if(ret)
    {
      return (vtkMRMLMRAblationNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLMRAblationNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLMRAblationNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLMRAblationNode");
  if(ret)
    {
      return (vtkMRMLMRAblationNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLMRAblationNode;
}

//----------------------------------------------------------------------------
vtkMRMLMRAblationNode::vtkMRMLMRAblationNode()
{
   this->ImageDirectory = NULL;
   this->WorkingDirectory = NULL;
   this->Timepoints = 30;
   this->Slices = 10;
   this->TE = 0.020;
   this->w0 = 21.3;
   this->TC = 0.01076;
   this->OutputVolumeRef = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLMRAblationNode::~vtkMRMLMRAblationNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLMRAblationNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream

  vtkIndent indent(nIndent);

  {
    std::stringstream ss;
    ss << this->Timepoints;
    of << indent << "Timepoints='" << ss.str() << "' ";
  }
  {
    std::stringstream ss;
    ss << this->Slices;
    of << indent << "Slices='" << ss.str() << "' ";
  }
  {
    std::stringstream ss;
    ss << this->TE;
    of << indent << "TE='" << ss.str() << "' ";
  }
  {
    std::stringstream ss;
    ss << this->w0;
    of << indent << "w0='" << ss.str() << "' ";
  }
  {
    std::stringstream ss;
    ss << this->TC;
    of << indent << "TC='" << ss.str() << "' ";
  }
  {
    std::stringstream ss;
    ss << this->OutputVolumeRef;
    of << indent << "OutputVolumeRef='" << ss.str() << "' ";
  }
}

//----------------------------------------------------------------------------
void vtkMRMLMRAblationNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "Timepoints")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Timepoints;
      }
    else if (!strcmp(attName, "Slices")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Slices;
      }
    else if (!strcmp(attName, "TE")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->TE;
      }
    else if (!strcmp(attName, "w0")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->w0;
      }
    else if (!strcmp(attName, "TC")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->TC;
      }
    else if (!strcmp(attName, "OutputVolumeRef"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->OutputVolumeRef;
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLMRAblationNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLMRAblationNode *node = (vtkMRMLMRAblationNode *) anode;

  this->SetTimepoints(node->Timepoints);
  this->SetSlices(node->Slices);
  this->SetTE(node->TE);
  this->Setw0(node->w0);
  this->SetTC(node->TC);
  this->SetOutputVolumeRef(node->OutputVolumeRef);
}

//----------------------------------------------------------------------------
void vtkMRMLMRAblationNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "Timepoints:   " << this->Timepoints << "\n";
  os << indent << "Slices:   " << this->Slices << "\n";
  os << indent << "TE:   " << this->TE << "\n";
  os << indent << "w0:   " << this->w0 << "\n";
  os << indent << "TC:   " << this->TC << "\n";
  os << indent << "OutputVolumeRef:   " << this->OutputVolumeRef << "\n";
}

const char *vtkMRMLMRAblationNode::GetImageDirectory() 
{
  return ImageDirectory;
}

void vtkMRMLMRAblationNode::SetImageDirectory(const char *imgDir) 
{
  std::string tmp(imgDir);
  if (ImageDirectory != NULL)
  {
    delete [] ImageDirectory;
    ImageDirectory = NULL;
  }
  if (tmp.size() > 0)
  {
    ImageDirectory = new char [tmp.size()];
    strcpy(ImageDirectory, tmp.c_str());
  }
}


const char *vtkMRMLMRAblationNode::GetWorkingDirectory() 
{
  return WorkingDirectory;
}

void vtkMRMLMRAblationNode::SetWorkingDirectory(const char *workingDir) 
{
  std::string tmp(workingDir);
  if (WorkingDirectory != NULL)
  {
    delete [] WorkingDirectory;
    WorkingDirectory = NULL;
  }
  if (tmp.size() > 0)
  {
    WorkingDirectory = new char [tmp.size()];
    strcpy(WorkingDirectory, tmp.c_str());
  }
}


/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <ostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLScene.h"

// Initialize static member that controls resampling -- 
// old comment: "This offset will be changed to 0.5 from 0.0 per 2/8/2002 Slicer 
// development meeting, to move ijk coordinates to voxel centers."


//------------------------------------------------------------------------------
vtkMRMLVolumeDisplayNode* vtkMRMLVolumeDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeDisplayNode");
  if(ret)
    {
      return (vtkMRMLVolumeDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVolumeDisplayNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLVolumeDisplayNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeDisplayNode");
  if(ret)
    {
      return (vtkMRMLVolumeDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVolumeDisplayNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeDisplayNode::vtkMRMLVolumeDisplayNode()
{
  // Strings
  this->LUTName = NULL;
  this->Interpolate = 1;
  this->AutoWindowLevel = 1;
  this->Window = 256;
  this->Level = 128;
  this->AutoThreshold = 0;
  this->ApplyThreshold = 0;
  this->LowerThreshold = VTK_SHORT_MIN;
  this->UpperThreshold = VTK_SHORT_MAX;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeDisplayNode::~vtkMRMLVolumeDisplayNode()
{
  if (this->LUTName)
    {
      delete [] this->LUTName;
      this->LUTName = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::WriteXML(ostream& of, int nIndent)
{
Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->LUTName != NULL) {
    of << indent << "LUTName='" << this->LUTName << "' ";
  }

  std::stringstream ss;

  ss << this->Window;
  of << indent << "Window='" << ss.str() << "' ";
  ss.clear();

  ss << this->Level;
  of << indent << "Level='" << ss.str() << "' ";
  ss.clear();

  ss << this->UpperThreshold;
  of << indent << "UpperThreshold='" << ss.str() << "' ";
  ss.clear();

  ss << this->LowerThreshold;
  of << indent << "LowerThreshold='" << ss.str() << "' ";
  ss.clear();

  ss << this->Interpolate;
  of << indent << "Interpolate='" << ss.str() << "' ";
  ss.clear();

  ss << this->AutoWindowLevel;
  of << indent << "AutoWindowLevel='" << ss.str() << "' ";
  ss.clear();

  ss << this->ApplyThreshold;
  of << indent << "ApplyThreshold='" << ss.str() << "' ";
  ss.clear();

  ss << this->AutoThreshold;
  of << indent << "AutoThreshold='" << ss.str() << "' ";
  ss.clear();

}

//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "Window")) {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Window;
    }
    else if (!strcmp(attName, "Level")) {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Level;
    }
    else if (!strcmp(attName, "UpperThreshold")) {
      std::stringstream ss;
      ss << attValue;
      ss >> this->UpperThreshold;
    }
    else if (!strcmp(attName, "LowerThreshold")) {
      std::stringstream ss;
      ss << attValue;
      ss >> this->LowerThreshold;
    }
    else if (!strcmp(attName, "Interpolate")) {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Interpolate;
    }
    else if (!strcmp(attName, "AutoWindowLevel")) {
      std::stringstream ss;
      ss << attValue;
      ss >> this->AutoWindowLevel;
    }
    else if (!strcmp(attName, "ApplyThreshold")) {
      std::stringstream ss;
      ss << attValue;
      ss >> this->ApplyThreshold;
    }
    else if (!strcmp(attName, "AutoThreshold")) {
      std::stringstream ss;
      ss << attValue;
      ss >> this->AutoThreshold;
    }
  }  
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLVolumeDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLVolumeDisplayNode *node = (vtkMRMLVolumeDisplayNode *) anode;

  this->SetLUTName(node->LUTName);
  this->SetAutoWindowLevel(node->AutoWindowLevel);
  this->SetWindow(node->Window);
  this->SetLevel(node->Level);
  this->SetAutoThreshold(node->AutoThreshold);
  this->SetApplyThreshold(node->ApplyThreshold);
  this->SetUpperThreshold(node->UpperThreshold);
  this->SetLowerThreshold(node->LowerThreshold);
  this->SetInterpolate(node->Interpolate);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "LUTName: " <<
    (this->LUTName ? this->LUTName : "(none)") << "\n";

  os << indent << "AutoWindowLevel:   " << this->AutoWindowLevel << "\n";
  os << indent << "Window:            " << this->Window << "\n";
  os << indent << "Level:             " << this->Level << "\n";
  os << indent << "AutoThreshold:     " << this->AutoThreshold << "\n";
  os << indent << "ApplyThreshold:    " << this->ApplyThreshold << "\n";
  os << indent << "UpperThreshold:    " << this->UpperThreshold << "\n";
  os << indent << "LowerThreshold:    " << this->LowerThreshold << "\n";
  os << indent << "Interpolate:       " << this->Interpolate << "\n";
}


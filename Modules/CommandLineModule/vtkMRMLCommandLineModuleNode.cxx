/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLGradientAnisotropicDiffusionFilterNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLCommandLineModuleNode.h"
#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkMRMLCommandLineModuleNode* vtkMRMLCommandLineModuleNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLCommandLineModuleNode");
  if(ret)
    {
      return (vtkMRMLCommandLineModuleNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLCommandLineModuleNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLCommandLineModuleNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLCommandLineModuleNode");
  if(ret)
    {
      return (vtkMRMLCommandLineModuleNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLCommandLineModuleNode;
}

//----------------------------------------------------------------------------
vtkMRMLCommandLineModuleNode::vtkMRMLCommandLineModuleNode()
{
    this->HideFromEditors = false;
}

//----------------------------------------------------------------------------
vtkMRMLCommandLineModuleNode::~vtkMRMLCommandLineModuleNode()
{
}


//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream

  vtkIndent indent(nIndent);

//   {
//     std::stringstream ss;
//     ss << this->Conductance;
//     of << indent << "Conductance='" << ss.str() << "' ";
//   }
//   {
//     std::stringstream ss;
//     ss << this->NumberOfIterations;
//     of << indent << "NumberOfIterations='" << ss.str() << "' ";
//   }
//   {
//     std::stringstream ss;
//     ss << this->TimeStep;
//     of << indent << "TimeStep='" << ss.str() << "' ";
//   }
//   {
//     std::stringstream ss;
//     ss << this->InputVolumeRef;
//     of << indent << "InputVolumeRef='" << ss.str() << "' ";
//   }
//   {
//     std::stringstream ss;
//     ss << this->OutputVolumeRef;
//     of << indent << "OutputVolumeRef='" << ss.str() << "' ";
//   }
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
//   const char* attName;
//   const char* attValue;
//   while (*atts != NULL) 
//     {
//     attName = *(atts++);
//     attValue = *(atts++);
//     if (!strcmp(attName, "Conductance")) 
//       {
//       std::stringstream ss;
//       ss << attValue;
//       ss >> this->Conductance;
//       }
//     else if (!strcmp(attName, "NumberOfIterations")) 
//       {
//       std::stringstream ss;
//       ss << attValue;
//       ss >> this->NumberOfIterations;
//       }
//     else if (!strcmp(attName, "TimeStep")) 
//       {
//       std::stringstream ss;
//       ss << attValue;
//       ss >> this->TimeStep;
//       }
//     else if (!strcmp(attName, "InputVolumeRef"))
//       {
//       std::stringstream ss;
//       ss << attValue;
//       ss >> this->InputVolumeRef;
//       }
//     else if (!strcmp(attName, "OutputVolumeRef"))
//       {
//       std::stringstream ss;
//       ss << attValue;
//       ss >> this->OutputVolumeRef;
//       }
//     }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLCommandLineModuleNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLCommandLineModuleNode *node = (vtkMRMLCommandLineModuleNode *) anode;

  this->SetModuleDescription(node->ModuleDescriptionObject);
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "Module description:   " << this->ModuleDescriptionObject;
}

//----------------------------------------------------------------------------
void
vtkMRMLCommandLineModuleNode::SetModuleDescription(const ModuleDescription& description)
{
  // Copy the module description
  ModuleDescriptionObject = description;

  // Set an attribute on the node so that we can select nodes that
  // have the same command line module (program)
  this->SetAttribute("CommandLineModule", description.GetTitle().c_str());

  
  this->Modified();
}


//----------------------------------------------------------------------------
void
vtkMRMLCommandLineModuleNode
::SetParameterAsString(const std::string& name, const std::string& value)
{
  // Set the default value of the named parameter with the value
  // specified
  if (value != this->GetParameterAsString(name))
    {
    this->ModuleDescriptionObject.SetParameterDefaultValue(name, value);
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void
vtkMRMLCommandLineModuleNode
::SetParameterAsDouble(const std::string& name, double value)
{
  std::ostrstream strvalue;

  strvalue << value;
  strvalue << ends;
  
  // Set the default value of the named parameter with the value
  // specified
  if (strvalue.str() != this->GetParameterAsString(name))
    {
    this->ModuleDescriptionObject
      .SetParameterDefaultValue(name, strvalue.str());
    this->Modified();
    }

  strvalue.rdbuf()->freeze(0);
}

//----------------------------------------------------------------------------
void
vtkMRMLCommandLineModuleNode
::SetParameterAsFloat(const std::string& name, float value)
{
  std::ostrstream strvalue;

  strvalue << value;
  strvalue << ends;
  
  // Set the default value of the named parameter with the value
  // specified
  if (strvalue.str() != this->GetParameterAsString(name))
    {
    this->ModuleDescriptionObject
      .SetParameterDefaultValue(name, strvalue.str());
    this->Modified();
    }

  strvalue.rdbuf()->freeze(0);
}


//----------------------------------------------------------------------------
void
vtkMRMLCommandLineModuleNode
::SetParameterAsInt(const std::string& name, int value)
{
  std::ostrstream strvalue;

  strvalue << value;
  strvalue << ends;
  
  // Set the default value of the named parameter with the value
  // specified
  if (strvalue.str() != this->GetParameterAsString(name))
    {
    this->ModuleDescriptionObject
      .SetParameterDefaultValue(name, strvalue.str());
    this->Modified();
    }

  strvalue.rdbuf()->freeze(0);
}

//----------------------------------------------------------------------------
void
vtkMRMLCommandLineModuleNode
::SetParameterAsBool(const std::string& name, bool value)
{
  // Set the default value of the named parameter with the value
  // specified
  if (this->GetParameterAsString(name) != (value ? "true" : "false"))
    {
    this->ModuleDescriptionObject
      .SetParameterDefaultValue(name, value ? "true" : "false");
    this->Modified();
    }
}


std::string
vtkMRMLCommandLineModuleNode
::GetParameterAsString(const std::string& name) const
{
  return this->ModuleDescriptionObject.GetParameterDefaultValue(name);
}





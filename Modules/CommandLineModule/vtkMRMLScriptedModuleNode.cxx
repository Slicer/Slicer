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

#include "vtkMRMLScriptedModuleNode.h"
#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkMRMLScriptedModuleNode* vtkMRMLScriptedModuleNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLScriptedModuleNode");
  if(ret)
    {
      return (vtkMRMLScriptedModuleNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLScriptedModuleNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLScriptedModuleNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLScriptedModuleNode");
  if(ret)
    {
      return (vtkMRMLScriptedModuleNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLScriptedModuleNode;
}

//----------------------------------------------------------------------------
vtkMRMLScriptedModuleNode::vtkMRMLScriptedModuleNode()
{
  this->Value = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLScriptedModuleNode::~vtkMRMLScriptedModuleNode()
{
  if (this->Value)
    {
    delete [] this->Value;
    this->Value = NULL;
    }
}


//----------------------------------------------------------------------------
void vtkMRMLScriptedModuleNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream

  vtkIndent indent(nIndent);

  std::map<std::string, std::string>::iterator iter;

  for (iter=this->Parameters.begin(); iter != this->Parameters.end(); iter++)
    {
    of << iter->first << "= '" << iter->second << "' ";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLScriptedModuleNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    std::string sname(attName);
    std::string svalue(attValue);
    this->SetParameter(sname, svalue);
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLScriptedModuleNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLScriptedModuleNode *node = (vtkMRMLScriptedModuleNode *) anode;

  this->Parameters = node->Parameters;
}

//----------------------------------------------------------------------------
void vtkMRMLScriptedModuleNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);

  std::map<std::string, std::string>::iterator iter;

  for (iter=this->Parameters.begin(); iter != this->Parameters.end(); iter++)
    {
    os << indent << iter->first << ": " << iter->second << "\n";
    }
}

//----------------------------------------------------------------------------
void
vtkMRMLScriptedModuleNode
::SetParameter(const std::string& name, const std::string& value)
{
  // Set the default value of the named parameter with the value
  // specified
  if (value != this->GetParameter(name))
    {
    this->Parameters[name] = value;
    this->Modified();
    }
}


//----------------------------------------------------------------------------
std::string
vtkMRMLScriptedModuleNode
::GetParameter(const std::string& name) const
{
  if ( this->Parameters.find(name) == this->Parameters.end() )
    {
    return (NULL);
    }
  return this->Parameters.find(name)->second;

  // return this->Parameters[name];
}


//----------------------------------------------------------------------------
void
vtkMRMLScriptedModuleNode
::SetParameter(const char *name, const char *value)
{
  std::string sname(name);
  std::string svalue(value);
  this->SetParameter(sname, svalue);
}

//----------------------------------------------------------------------------
void
vtkMRMLScriptedModuleNode
::RequestParameter(const char *name)
{
  std::string sname(name);
  std::string svalue = this->GetParameter(sname);
  this->SetValue (svalue.c_str());
}



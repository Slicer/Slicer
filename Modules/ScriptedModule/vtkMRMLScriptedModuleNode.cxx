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
  this->ParameterList = NULL;
  this->ModuleName = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLScriptedModuleNode::~vtkMRMLScriptedModuleNode()
{
  this->SetValue(NULL);
  this->SetParameterList(NULL);

  if (this->Value)
    {
    delete [] this->Value;
    this->Value = NULL;
    }
  if (this->ModuleName)
    {
    delete [] this->ModuleName;
    this->ModuleName = NULL;
    }
}


//----------------------------------------------------------------------------
void vtkMRMLScriptedModuleNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream

  vtkIndent indent(nIndent);

  if (this->ModuleName != NULL)
    {
    of << " ModuleName =\"" << this->ModuleName << "\""; 
    }

  std::map<std::string, std::string>::iterator iter;

  int i = 0;
  for (iter=this->Parameters.begin(); iter != this->Parameters.end(); iter++)
    {
    of << " parameter" << i << "= \"" << iter->first << " " << iter->second << "\"";
    i++;
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

    if ( !strcmp(attName, "ModuleName") )
      {
      this->SetModuleName( attValue );
      }
    else if ( !strncmp(attName, "parameter", strlen("parameter") ) )
      {
      std::string satt(attValue);
      int space = satt.find(" ", 0);
      std::string sname = satt.substr(0,space);
      std::string svalue = satt.substr(space+1,satt.length()-space-1);
      this->SetParameter(sname, svalue);
      }
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

  this->SetModuleName( this->GetModuleName() );
}

//----------------------------------------------------------------------------
void vtkMRMLScriptedModuleNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "ModuleName: " << (this->GetModuleName() ? this->GetModuleName() : "(none)") << "\n";

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
  const std::string *currentValue = this->GetParameter(name);
  if (currentValue == NULL || (currentValue != NULL && value != *currentValue ) )
    {
    this->Parameters[name] = value;
    this->Modified();
    }
}


//----------------------------------------------------------------------------
const std::string *
vtkMRMLScriptedModuleNode
::GetParameter(const std::string& name) const
{
  if ( this->Parameters.find(name) == this->Parameters.end() )
    {
    return (NULL);
    }
  return &(this->Parameters.find(name)->second);
}

//----------------------------------------------------------------------------
const char *
vtkMRMLScriptedModuleNode
::GetParameter(const char *name)
{
  this->RequestParameter(name);
  return (this->GetValue());
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
  const std::string *svaluep = this->GetParameter(sname);
  this->SetDisableModifiedEvent(1);
  if ( svaluep )
    {
    this->SetValue (svaluep->c_str());
    }
  else
    {
    this->SetValue ("");
    }
  this->SetDisableModifiedEvent(0);
}


//----------------------------------------------------------------------------
void
vtkMRMLScriptedModuleNode
::RequestParameterList()
{
  std::string list("");

  std::map<std::string, std::string>::iterator iter;

  for (iter=this->Parameters.begin(); iter != this->Parameters.end(); iter++)
    {
    list += "\"" + iter->first + "\" \"" + iter->second + "\" ";
    }
  this->SetParameterList (list.c_str());
}

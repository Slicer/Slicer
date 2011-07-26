/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLGradientAnisotropicDiffusionFilterNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRMLCLI includes
#include "vtkMRMLCommandLineModuleNode.h"

// MRML includes

// VTK includes
#include "vtkObjectFactory.h"

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLCommandLineModuleNode);

//------------------------------------------------------------------------------
// Private implementaton of an std::map
class ModuleDescriptionMap : public std::map<std::string, ModuleDescription> {};

ModuleDescriptionMap* vtkMRMLCommandLineModuleNode::RegisteredModules = new ModuleDescriptionMap;

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLCommandLineModuleNode::CreateNodeInstance()
{
  return vtkMRMLCommandLineModuleNode::New();
}

//----------------------------------------------------------------------------
vtkMRMLCommandLineModuleNode::vtkMRMLCommandLineModuleNode()
{
  this->HideFromEditors = true;
  this->m_Status = Idle;
}

//----------------------------------------------------------------------------
vtkMRMLCommandLineModuleNode::~vtkMRMLCommandLineModuleNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::WriteXML(ostream& of, int nIndent)
{
  // Serialize a CommandLineModule node.
  //
  // Only need to write out enough information from the
  // ModuleDescription such that we can recognize the node type.  When
  // we reconstitute a node, we will start with a copy of the
  // prototype node for that module and then overwrite individual
  // parameter values using the parameter values indicated here.


  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  const ModuleDescription& module = this->GetModuleDescription();

  // Need to write out module description and parameters as
  // attributes.  Only need to write out the module title and version
  // in order to be able recognize the node type.  Then we just need
  // to write out each parameter name and default.  Note that any
  // references to other nodes are already stored as IDs. So we write
  // out those IDs.
  //
  of << " title=\"" << this->URLEncodeString ( module.GetTitle().c_str() ) << "\"";
  of << " version=\"" << this->URLEncodeString ( module.GetVersion().c_str() ) << "\"";
  
  // Loop over the parameter groups, writing each parameter.  Note
  // that the parameter names are unique.
  std::vector<ModuleParameterGroup>::const_iterator pgbeginit
    = module.GetParameterGroups().begin();
  std::vector<ModuleParameterGroup>::const_iterator pgendit
    = module.GetParameterGroups().end();
  std::vector<ModuleParameterGroup>::const_iterator pgit;

  
  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::const_iterator pbeginit
      = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::const_iterator pendit
      = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::const_iterator pit;

    for (pit = pbeginit; pit != pendit; ++pit)
      {
      // two calls, as the mrml node method saves the new string in a member
      // variable and it was getting over written when used twice before the
      // buffer was flushed.
      of << " " << this->URLEncodeString ( (*pit).GetName().c_str() );
      of  << "=\"" << this->URLEncodeString ( (*pit).GetDefault().c_str() ) << "\"";
      }
    }
  
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  // To reconstitute a CommandLineModule node:
  //
  // 1. Find the prototype node from the "title" and "version".
  // 2. Copy the prototype node into the current node.
  // 3. Override parameter values with the attributes (attributes not
  // consumed by the superclass or known attributes from the prototype
  // node).
  //
  // Referenced nodes are stored as IDs.  Do we need to remap them at all?

  // first look for the title which we need to find the prototype node
  std::string moduleTitle;
  std::string moduleVersion;

  const char **tatts = atts;
  const char *attName = NULL;
  const char *attValue;
  while (*tatts)
    {
    attName = *(tatts++);
    attValue = *(tatts++);

    if (!strcmp(attName, "title"))
      {
      moduleTitle = this->URLDecodeString(attValue);
      }
    else if (!strcmp(attName, "version"))
      {
      moduleVersion = this->URLDecodeString(attValue);
      }
    }

  // Set an attribute on the node based on the module title so that
  // the node selectors can filter on it.
  this->SetAttribute("CommandLineModule", moduleTitle.c_str());
  
  // look up the module description from the library
  if (vtkMRMLCommandLineModuleNode::HasRegisteredModule( moduleTitle ))
    {
    this->ModuleDescriptionObject =
     vtkMRMLCommandLineModuleNode::GetRegisteredModuleDescription(moduleTitle);
    }
  else
    {
    // can't locate the module, return;
    return;
    }

  // Verify the version
  if (moduleVersion != this->ModuleDescriptionObject.GetVersion())
    {
    std::string msg = "Command line module " + moduleTitle + " is version \""
      + this->ModuleDescriptionObject.GetVersion()
      + "\" but parameter set from MRML file is version \""
      + moduleVersion
      + "\". Parameter set may not load properly,";
      
    vtkWarningMacro(<< msg.c_str());
    }
  
  // run through the attributes and pull out any attributes for this
  // module
  tatts = atts;
  while (*tatts)
    {
    std::string sattName = std::string(this->URLDecodeString(*(tatts++)));
    std::string sattValue = std::string(this->URLDecodeString(*(tatts++)));

    if (this->ModuleDescriptionObject.HasParameter(attName))
      {
      this->ModuleDescriptionObject.SetParameterDefaultValue(sattName.c_str(),sattValue.c_str());
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLCommandLineModuleNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLCommandLineModuleNode *node = (vtkMRMLCommandLineModuleNode *) anode;

  this->SetModuleDescription(node->ModuleDescriptionObject);
  this->SetStatus(node->m_Status);
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "Module description:   "
     << std::endl
     << "   " << this->ModuleDescriptionObject;
  os << indent << "Status: " << this->m_Status;
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::SetModuleDescription(const ModuleDescription& description)
{
  // Copy the module description
  ModuleDescriptionObject = description;

  // Set an attribute on the node so that we can select nodes that
  // have the same command line module (program)
  this->SetAttribute("CommandLineModule", description.GetTitle().c_str());

  
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode
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
void vtkMRMLCommandLineModuleNode
::SetParameterAsDouble(const std::string& name, double value)
{
  std::ostringstream strvalue;

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
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode
::SetParameterAsFloat(const std::string& name, float value)
{
  std::ostringstream strvalue;

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
}


//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode
::SetParameterAsInt(const std::string& name, int value)
{
  std::ostringstream strvalue;

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
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode
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

//----------------------------------------------------------------------------
std::string vtkMRMLCommandLineModuleNode::GetParameterAsString(const std::string& name) const
{
  return this->ModuleDescriptionObject.GetParameterDefaultValue(name);
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode
::SetStatus(vtkMRMLCommandLineModuleNode::StatusType status, bool modify)
{
  if (this->m_Status != status)
    {
    this->m_Status = status;
    if (this->m_Status == vtkMRMLCommandLineModuleNode::Cancelled)
      {
      this->AbortProcess();
      }
    if (modify)
      {
      this->Modified();
      }
    }
}

//----------------------------------------------------------------------------
vtkMRMLCommandLineModuleNode::StatusType vtkMRMLCommandLineModuleNode::GetStatus()
{
  return this->m_Status;
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::AbortProcess()
{
  this->GetModuleDescription().GetProcessInformation()->Abort = 1;
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode::HasRegisteredModule(const std::string& name)
{
  ModuleDescriptionMap::iterator mit;

  mit = (*vtkMRMLCommandLineModuleNode::RegisteredModules).find(name);

  return mit != (*vtkMRMLCommandLineModuleNode::RegisteredModules).end();
}

//----------------------------------------------------------------------------
ModuleDescription vtkMRMLCommandLineModuleNode
::GetRegisteredModuleDescription(const std::string& name)
{
  ModuleDescriptionMap::iterator mit;

  mit = (*vtkMRMLCommandLineModuleNode::RegisteredModules).find(name);

  if (mit != (*vtkMRMLCommandLineModuleNode::RegisteredModules).end())
    {
    return (*mit).second;
    }

  return ModuleDescription();
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode
::RegisterModuleDescription(ModuleDescription md)
{
  (*vtkMRMLCommandLineModuleNode::RegisteredModules)[md.GetTitle()] = md;
}

//----------------------------------------------------------------------------
void vtkMRMLCommandLineModuleNode::ClearRegisteredModules()
{
  (*vtkMRMLCommandLineModuleNode::RegisteredModules).clear();
}

//----------------------------------------------------------------------------
int vtkMRMLCommandLineModuleNode::GetNumberOfRegisteredModules ()
{ 
  return (int)RegisteredModules->size(); 
}

//----------------------------------------------------------------------------
const char* vtkMRMLCommandLineModuleNode::GetRegisteredModuleNameByIndex ( int idx )
{
  ModuleDescriptionMap::iterator mit = RegisteredModules->begin();
  int count = 0;
  while ( mit != RegisteredModules->end() ) 
    {
    if ( count == idx ) { return (*mit).first.c_str(); }
    ++mit;
    ++count;
    }
  return "";
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode::ReadParameterFile(const std::string& filename)
{
  return this->ReadParameterFile(filename.c_str());
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode::ReadParameterFile(const char* filename)
{
  bool modified = this->ModuleDescriptionObject.ReadParameterFile(filename);
  
  if (modified)
    {
    this->Modified();
    }

  return modified;
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode
::WriteParameterFile(const std::string& filename, bool withHandlesToBulkParameters)
{
  return this->WriteParameterFile(filename.c_str(), withHandlesToBulkParameters);
}

//----------------------------------------------------------------------------
bool vtkMRMLCommandLineModuleNode
::WriteParameterFile(const char* filename, bool withHandlesToBulkParameters)
{
  bool modified 
    = this->ModuleDescriptionObject.WriteParameterFile(filename, withHandlesToBulkParameters);
  
  if (modified)
    {
    this->Modified();
    }

  return modified;
}

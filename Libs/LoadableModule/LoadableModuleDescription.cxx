/*=========================================================================

  Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Loadable Module
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#include "LoadableModuleDescription.h"


LoadableModuleDescription::LoadableModuleDescription()
{
  this->Type = "Unknown";
}


LoadableModuleDescription::LoadableModuleDescription(const LoadableModuleDescription &md)
{
  this->Name = md.Name;
  this->ShortName = md.ShortName;
  this->GUIName = md.GUIName;
  this->TclInitName = md.TclInitName;

  this->Message = md.Message;

  this->GUIPtr = md.GUIPtr;
  this->LogicPtr = md.LogicPtr;
 
  this->TclInitFunction = md.TclInitFunction;

  this->Dependencies = md.Dependencies;

  this->Type = md.Type;
  this->Target = md.Target;
  this->Location = md.Location;

  this->AlternativeType = md.AlternativeType;
  this->AlternativeTarget = md.AlternativeTarget;
  this->AlternativeLocation = md.AlternativeLocation;
}

void
LoadableModuleDescription::operator=(const LoadableModuleDescription &md)
{
  this->Name = md.Name;
  this->ShortName = md.ShortName;
  this->GUIName = md.GUIName;
  this->TclInitName = md.TclInitName;

  this->Message = md.Message;

  this->GUIPtr = md.GUIPtr;
  this->LogicPtr = md.LogicPtr;

  this->TclInitFunction = md.TclInitFunction;

  this->Dependencies = md.Dependencies;

  this->Type = md.Type;
  this->Target = md.Target;
  this->Location = md.Location;

  this->AlternativeType = md.AlternativeType;
  this->AlternativeTarget = md.AlternativeTarget;
  this->AlternativeLocation = md.AlternativeLocation;
}

std::ostream & operator<<(std::ostream &os, const LoadableModuleDescription &module)
{
  os << "Name: " << module.GetName() << std::endl;
  os << "ShortName: " << module.GetShortName() << std::endl;
  os << "GUIName: " << module.GetGUIName() << std::endl;
  os << "TclInitName: " << module.GetTclInitName() << std::endl;

  os << "Message: " << module.GetMessage() << std::endl;

  std::vector<std::string>::iterator iter = module.GetDependencies().begin();
  while (iter != module.GetDependencies().end()) {
    os << "Dependency: " << (*iter) << std::endl;
  }

  os << "Type: " <<  module.GetType() << std::endl;
  os << "Target: " << module.GetTarget() << std::endl;
  os << "Location: " << module.GetLocation() << std::endl;

  os << "AlternativeType: " << module.GetAlternativeType() << std::endl;
  os << "AlternativeTarget: " << module.GetAlternativeTarget() << std::endl;
  os << "AlternativeLocation: " << module.GetAlternativeLocation() << std::endl;

  return os;
}



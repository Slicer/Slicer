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
  this->Description = "No description provided";
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

  this->Category = md.Category;
  this->Description = md.Description;
  this->Version = md.Version;
  this->DocumentationURL = md.DocumentationURL;
  this->License = md.License;
  this->Acknowledgements = md.Acknowledgements;
  this->Contributor = md.Contributor;
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

  this->Category = md.Category;
  this->Description = md.Description;
  this->Version = md.Version;
  this->DocumentationURL = md.DocumentationURL;
  this->License = md.License;
  this->Acknowledgements = md.Acknowledgements;
  this->Contributor = md.Contributor;
  this->Type= md.Type;
  this->Target = md.Target;
  this->Location = md.Location;
  this->AlternativeType= md.AlternativeType;
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

  os << "Category: " << module.GetCategory() << std::endl;
  os << "Description: " << module.GetDescription() << std::endl;
  os << "Version: " << module.GetVersion() << std::endl;
  os << "DocumentationURL: " << module.GetDocumentationURL() << std::endl;
  os << "License: " << module.GetLicense() << std::endl;
  os << "Contributor: " << module.GetContributor() << std::endl;
  os << "Acknowledgements: " << module.GetAcknowledgements() << std::endl;
  os << "Type: " << module.GetType() << std::endl;
  os << "Target: " << module.GetTarget() << std::endl;
  os << "Location: " << module.GetLocation() << std::endl;
  os << "Alternative Type: " << module.GetAlternativeType() << std::endl;
  os << "Alternative Target: " << module.GetAlternativeTarget() << std::endl;
  os << "Alternative Location: " << module.GetAlternativeLocation() << std::endl;

  return os;
}



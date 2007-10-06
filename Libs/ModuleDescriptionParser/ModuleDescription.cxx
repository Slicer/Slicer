/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Module Description Parser
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#include "ModuleDescription.h"


ModuleDescription::ModuleDescription()
{
  this->Type = "Unknown";
  this->Description = "No description provided";
}


ModuleDescription::ModuleDescription(const ModuleDescription &md)
{
  this->Title = md.Title;
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
  this->ParameterGroups = md.ParameterGroups;
  this->Logo = md.Logo;
  
  this->ProcessInformation.Initialize();
}

void
ModuleDescription::operator=(const ModuleDescription &md)
{
  this->Title = md.Title;
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
  this->ParameterGroups = md.ParameterGroups;
  this->ProcessInformation = md.ProcessInformation;
  this->Logo = md.Logo;
}

std::ostream & operator<<(std::ostream &os, const ModuleDescription &module)
{
  os << "Title: " << module.GetTitle() << std::endl;
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
  //os << "Logo: " << module.GetLogo() << std::endl;

  os << "ProcessInformation: " << std::endl
     << *(module.GetProcessInformation());
  
  os << "ParameterGroups: " << std::endl;
  std::vector<ModuleParameterGroup>::const_iterator it = module.GetParameterGroups().begin();
  while (it != module.GetParameterGroups().end())
    {
    os << *it;
    ++it;
    }
  return os;
}


bool
ModuleDescription
::HasParameter(const std::string& name) const
{
  // iterate over each parameter group
  std::vector<ModuleParameterGroup>::const_iterator pgbeginit
    = this->ParameterGroups.begin();
  std::vector<ModuleParameterGroup>::const_iterator pgendit
    = this->ParameterGroups.end();
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
      if ((*pit).GetName() == name)
        {
        return true;
        }
      }    
    }

  return false;
}

bool
ModuleDescription
::SetParameterDefaultValue(const std::string& name, const std::string& value)
{
  // iterate over each parameter group
  std::vector<ModuleParameterGroup>::iterator pgbeginit
    = this->ParameterGroups.begin();
  std::vector<ModuleParameterGroup>::iterator pgendit
    = this->ParameterGroups.end();
  std::vector<ModuleParameterGroup>::iterator pgit;
  
  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::iterator pbeginit
      = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::iterator pendit
      = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::iterator pit;

    for (pit = pbeginit; pit != pendit; ++pit)
      {
      if ((*pit).GetName() == name)
        {
        (*pit).SetDefault(value);
        return true;
        }
      }    
    }

  return false;
}


std::string
ModuleDescription
::GetParameterDefaultValue(const std::string& name) const
{
  // iterate over each parameter group
  std::vector<ModuleParameterGroup>::const_iterator pgbeginit
    = this->ParameterGroups.begin();
  std::vector<ModuleParameterGroup>::const_iterator pgendit
    = this->ParameterGroups.end();
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
      if ((*pit).GetName() == name)
        {
        return (*pit).GetDefault();
        }
      }    
    }

  return "";
}

void
ModuleDescription
::SetLogo(const ModuleLogo& logo)
{
  this->Logo = logo;
}

const ModuleLogo&
ModuleDescription
::GetLogo() const
{
  return this->Logo;
}

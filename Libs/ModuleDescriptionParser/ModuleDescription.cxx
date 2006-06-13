#include "ModuleDescription.h"


ModuleDescription::ModuleDescription()
{
}


ModuleDescription::ModuleDescription(const ModuleDescription &md)
{
  this->Category = md.Category;
  this->Title = md.Title;
  this->Description = md.Description;
  this->Version = md.Version;
  this->DocumentationURL = md.DocumentationURL;
  this->License = md.License;
  this->Contributor = md.Contributor;
  this->ParameterGroups = md.ParameterGroups;
}

void
ModuleDescription::operator=(const ModuleDescription &md)
{
  this->Category = md.Category;
  this->Title = md.Title;
  this->Description = md.Description;
  this->Version = md.Version;
  this->DocumentationURL = md.DocumentationURL;
  this->License = md.License;
  this->Contributor = md.Contributor;
  this->ParameterGroups = md.ParameterGroups;
}

std::ostream & operator<<(std::ostream &os, const ModuleDescription &module)
{ 
  os << "Category: " << module.GetCategory() << std::endl;
  os << "Title: " << module.GetTitle() << std::endl;
  os << "Description: " << module.GetDescription() << std::endl;
  os << "Version: " << module.GetVersion() << std::endl;
  os << "DocumentationURL: " << module.GetDocumentationURL() << std::endl;
  os << "License: " << module.GetLicense() << std::endl;
  os << "Contributor: " << module.GetContributor() << std::endl;
  os << "ParameterGroups: " << std::endl;
  std::vector<ModuleParameterGroup>::const_iterator it = module.GetParameterGroups().begin();
  while (it != module.GetParameterGroups().end())
    {
    os << *it;
    ++it;
    }
  return os;
}


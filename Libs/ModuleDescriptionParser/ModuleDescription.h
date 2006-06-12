#ifndef __ModuleDescription_h
#define __ModuleDescription_h

#include "ModuleParameterGroup.h"

#include <string>
#include <vector>

class ModuleDescription
{
public:
  ModuleDescription() {};
  ModuleDescription(const ModuleDescription &md)
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

  void operator=(const ModuleDescription &md)
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

  void SetCategory(const std::string &cat)
  {
    this->Category = cat;
  }

  const std::string&  GetCategory() const
  {
    return this->Category;
  }
  
  void SetTitle(const std::string &title)
  {
    this->Title = title;
  }

  const std::string& GetTitle() const
  {
    return this->Title;
  }

  void SetDescription(const std::string &description)
  {
    this->Description = description;
  }

  const std::string& GetDescription() const
  {
    return this->Description;
  }

  void SetVersion(const std::string &version)
  {
    this->Version = version;
  }

  const std::string& GetVersion() const
  {
    return this->Version;
  }

  void SetDocumentationURL(const std::string &documentationURL)
  {
    this->DocumentationURL = documentationURL;
  }

  const std::string& GetDocumentationURL() const
  {
    return this->DocumentationURL;
  }

  void SetLicense(const std::string &license)
  {
    this->License = license;
  }

  const std::string& GetLicense() const
  {
    return this->License;
  }

  void SetContributor(const std::string &contributor)
  {
    this->Contributor = contributor;
  }

  const std::string& GetContributor() const
  {
    return this->Contributor;
  }

  void AddParameterGroup(const ModuleParameterGroup &group)
  {
    this->ParameterGroups.push_back(group);
  }

  const std::vector<ModuleParameterGroup>& GetParameterGroups() const
  {
    return this->ParameterGroups;
  }

private:
  std::string Category;
  std::string Title;
  std::string Description;
  std::string Version;
  std::string DocumentationURL;
  std::string License;
  std::string Contributor;
  std::vector<ModuleParameterGroup> ParameterGroups;  
};

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


#endif

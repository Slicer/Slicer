#ifndef __ModuleDescription_h
#define __ModuleDescription_h

#include "ModuleDescriptionParserWin32Header.h"

#include "ModuleParameterGroup.h"

#include <string>
#include <vector>

class ModuleDescriptionParser_EXPORT ModuleDescription
{
public:
  ModuleDescription();
  ModuleDescription(const ModuleDescription &md);

  void operator=(const ModuleDescription &md);

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

  void SetParameterGroups(const std::vector<ModuleParameterGroup>& groups)
  {
      this->ParameterGroups = groups;
  }

private:
  std::string Category;
  std::string Title;
  std::string Description;
  std::string Version;
  std::string DocumentationURL;
  std::string License;
  std::string Contributor;

#if defined(_MSC_VER) && (_MSC_VER > 1300) 
  template class ModuleDescriptionParser_EXPORT std::allocator<ModuleParameterGroup>;
  template class ModuleDescriptionParser_EXPORT std::vector<ModuleParameterGroup>;
#endif
  std::vector<ModuleParameterGroup> ParameterGroups;  
};

ModuleDescriptionParser_EXPORT std::ostream & operator<<(std::ostream &os, const ModuleDescription &module);

#endif

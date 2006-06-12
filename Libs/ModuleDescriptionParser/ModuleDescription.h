#ifndef __ModuleDescription_h
#define __ModuleDescription_h

#include "ModuleParameterGroup.h"

#include <string>
#include <vector>

class ModuleDescription
{
public:
  ModuleDescription() {};
  ModuleDescription(const ModuleDescription &md) {
    this->Category = md.Category;
    this->Title = md.Title;
    this->Description = md.Description;
    this->ParameterGroups = md.ParameterGroups;
  }

  void operator=(const ModuleDescription &md) {
    this->Category = md.Category;
    this->Title = md.Title;
    this->Description = md.Description;
    this->ParameterGroups = md.ParameterGroups;
  }

  void SetCategory(const std::string &cat) {
    this->Category = cat;
  }

  const std::string&  GetCategory() const {
    return this->Category;
  }
  
  void SetTitle(const std::string &title) {
    this->Title = title;
  }

  const std::string& GetTitle() const {
    return this->Title;
  }

  void SetDescription(const std::string &description) {
    this->Description = description;
  }

  const std::string& GetDescription() const {
    return this->Description;
  }

  void AddParameterGroup(const ModuleParameterGroup &group) {
    this->ParameterGroups.push_back(group);
  }

  const std::vector<ModuleParameterGroup>& GetParameterGroups() const {
    return this->ParameterGroups;
  }

private:
  std::string Category;
  std::string Title;
  std::string Description;
  std::vector<ModuleParameterGroup> ParameterGroups;  
};

std::ostream & operator<<(std::ostream &os, const ModuleDescription &module)
  { 
    os << "Category: " << module.GetCategory() << std::endl;
    os << "Title: " << module.GetTitle() << std::endl;
    os << "Description: " << module.GetDescription() << std::endl;
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

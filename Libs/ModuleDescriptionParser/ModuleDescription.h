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
    this->Title = md.Category;
    this->Description = md.Description;
    this->ParameterGroups = md.ParameterGroups;
  }

  void operator=(const ModuleDescription &md) {
    this->Category = md.Category;
    this->Title = md.Category;
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

protected:

private:
  std::string Category;
  std::string Title;
  std::string Description;
  std::vector<ModuleParameterGroup> ParameterGroups;  
};


#endif

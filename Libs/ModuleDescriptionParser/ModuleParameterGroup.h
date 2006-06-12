#ifndef __ModuleParameterGroup_h
#define __ModuleParameterGroup_h

#include "ModuleParameter.h"

#include <vector>
#include <string>

class ModuleParameterGroup
{
public:
  ModuleParameterGroup() {};
  ModuleParameterGroup(const ModuleParameterGroup &parameters)
  {
    this->Label = parameters.Label;
    this->Description = parameters.Description;
    this->Parameters = parameters.Parameters;
  }

  void operator=(const ModuleParameterGroup &parameters)
  {
    this->Label = parameters.Label;
    this->Description = parameters.Description;
    this->Parameters = parameters.Parameters;
  }
  
  void SetLabel(const std::string &label) {
    this->Label = label;
  }

  const std::string& GetLabel() const {
    return this->Label;
  }

  void SetDescription(const std::string &label) {
    this->Description = label;
  }

  const std::string& GetDescription() const {
    return this->Description;
  }

  void AddParameter(const ModuleParameter &parameter) {
    this->Parameters.push_back(parameter);
  }

  const std::vector<ModuleParameter>& GetParameters() const {
    return this->Parameters;
  }

private:
  std::string Label;
  std::string Description;
  std::vector<ModuleParameter> Parameters;
};

std::ostream & operator<<(std::ostream &os, const ModuleParameterGroup &group)
  { 
    os << "  Label: " << group.GetLabel() << std::endl;
    os << "  Description: " << group.GetDescription() << std::endl;
    os << "  Parameters: " << std::endl;
    std::vector<ModuleParameter>::const_iterator it = group.GetParameters().begin();
    while (it != group.GetParameters().end())
      {
      os << *it;
      ++it;
      }
    return os;
  }

#endif

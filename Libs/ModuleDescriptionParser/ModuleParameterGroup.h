#ifndef __ModuleParameterGroup_h
#define __ModuleParameterGroup_h

#include "ModuleParameter.h"

#include <vector>
#include <string>

class ModuleParameterGroup
{
public:
  ModuleParameterGroup() {};
  ModuleParameterGroup(const ModuleParameterGroup &parameters) {
    this->Label = parameters.Label;
    this->Parameters = parameters.Parameters;
  }

  void operator=(const ModuleParameterGroup &parameters) {
    this->Label = parameters.Label;
    this->Parameters = parameters.Parameters;
  }
  
  void SetLabel(const std::string &label) {
    this->Label = label;
  }

  const std::string& GetLabel() const {
    return this->Label;
  }

  void AddParameter(const ModuleParameter &parameter) {
    this->Parameters.push_back(parameter);
  }

  const std::vector<ModuleParameter>& GetParameters() const {
    return this->Parameters;
  }

private:
  std::string Label;
  std::vector<ModuleParameter> Parameters;
};

#endif

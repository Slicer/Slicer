#ifndef __ModuleParameterGroup_h
#define __ModuleParameterGroup_h

#include "ModuleDescriptionParserWin32Header.h"
#include "ModuleParameter.h"

#include <vector>
#include <string>

class ModuleDescriptionParser_EXPORT ModuleParameterGroup
{
public:
  ModuleParameterGroup() {};
  ModuleParameterGroup(const ModuleParameterGroup &parameters);

  void operator=(const ModuleParameterGroup &parameters);

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

  template class ModuleDescriptionParser_EXPORT std::allocator<ModuleParameter>;
  template class ModuleDescriptionParser_EXPORT std::vector<ModuleParameter>;
  std::vector<ModuleParameter> Parameters;
};

ModuleDescriptionParser_EXPORT std::ostream & operator<<(std::ostream &os, const ModuleParameterGroup &group);

#endif

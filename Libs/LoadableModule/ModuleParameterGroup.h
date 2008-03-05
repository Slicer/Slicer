/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Module Description Parser
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

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

  void SetDescription(const std::string &description) {
    this->Description = description;
  }

  const std::string& GetDescription() const {
    return this->Description;
  }

  void SetAdvanced(const std::string &advanced) {
    this->Advanced = advanced;
  }

  const std::string& GetAdvanced() const {
    return this->Advanced;
  }

  void AddParameter(const ModuleParameter &parameter) {
    this->Parameters.push_back(parameter);
  }

  const std::vector<ModuleParameter>& GetParameters() const {
    return this->Parameters;
  }

  std::vector<ModuleParameter>& GetParameters() {
    return this->Parameters;
  }
  
private:
  std::string Label;
  std::string Description;
  std::string Advanced;
  std::vector<ModuleParameter> Parameters;
};

ModuleDescriptionParser_EXPORT std::ostream & operator<<(std::ostream &os, const ModuleParameterGroup &group);

#endif

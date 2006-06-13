#ifndef __ModuleDescriptionParser_h
#define __ModuleDescriptionParser_h

#include <string>
#include <vector>

#include "ModuleDescriptionParserWin32Header.h"

class ModuleDescription;
class ModuleParameterGroup;


class ModuleDescriptionParser_EXPORT ModuleDescriptionParser
{
public:
  ModuleDescriptionParser() {};
  ~ModuleDescriptionParser() {};

  int Parse( const std::string& xml, ModuleDescription& description);
};

#endif

#ifndef __CommandLineModuleDescription_h
#define __CommandLineModuleDescription_h

#include "ModuleDescription.h"

class CommandLineModuleDescription : public ModuleDescription
{
public:
  CommandLineModuleDescription() {};
  CommandLineModuleDescription(const CommandLineModuleDescription &md)
    : ModuleDescription(md)
    {
      this->CommandString = md.CommandString;
    }

  void SetCommandString(const std::string& c) {
    this->CommandString = c;
  }

  const std::string& GetCommandString() const {
    return this->CommandString;
  }

private:
  std::string CommandString;    // path to command line executable and
                               // the name of executable
};


#endif

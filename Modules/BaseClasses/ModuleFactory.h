#ifndef __ModuleFactory_h
#define __ModuleFactory_h

#include "ModuleDescription.h"
#include <map>

class ModuleFactory
{
public:
  ModuleFactory() {};
  ~ModuleFactory() {};
  
  virtual void Scan();

  const std::map<std::string, ModuleDescription>& GetModules() const {
    return this->Modules;
  }

protected:
  virtual void ScanForSharedObjectModules();
  virtual void ScanForCommandLineModules();
  
private:
  std::map<std::string, ModuleDescription> Modules;
};


#endif

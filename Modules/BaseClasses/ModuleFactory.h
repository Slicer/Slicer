#ifndef __ModuleFactory_h
#define __ModuleFactory_h

#include "vtkModulesBaseClasses.h"

#include "ModuleDescription.h"

#include <vector>

class ModuleDescriptionMap;

class VTK_MODULES_BASE_CLASSES_EXPORT ModuleFactory
{
public:
  ModuleFactory();
  virtual ~ModuleFactory();

  // Set the search path for modules (both command line modules and
  // shared object modules).
  void SetSearchPath(const std::string& path) { SearchPath = path; }

  // Get the module search path.
  const std::string& GetSearchPath() const { return SearchPath; }
  
  // Scan for modules in the module search path.  This will locate
  // command line modules as well as shared object modules.
  virtual void Scan();

  // Get the names of all the modules.
  std::vector<std::string> GetModuleNames() const;

  // Get a module description by name.
  ModuleDescription GetModuleDescription(const std::string&) const;
  
protected:
  // Scan for shared object modules (i.e. DLL) in the module search
  // path. Returns the number of modules found.
  virtual long ScanForSharedObjectModules();

  // Scan for command line modules in the module search path. Command
  // line modules are executables that respond to a --xml
  // argument. Returns the number of modules found.
  virtual long ScanForCommandLineModules();
  
private:
  ModuleDescriptionMap *InternalMap;

  std::string SearchPath;
};


#endif

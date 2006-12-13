#ifndef __ModuleFactory_h
#define __ModuleFactory_h

#include "ModuleDescriptionParserWin32Header.h"

#include "ModuleDescription.h"

#include <vector>

class ModuleDescriptionMap;

class ModuleDescriptionParser_EXPORT ModuleFactory
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

  // Typedef of callback function
  typedef void (*CallbackFunctionType)(const char *);
    
  // Set/Get a function to call to report back warnings 
  void SetWarningMessageCallback( CallbackFunctionType );
  CallbackFunctionType GetWarningMessageCallback();

  // Set/Get a function to call to report back errors
  void SetErrorMessageCallback( CallbackFunctionType );
  CallbackFunctionType GetErrorMessageCallback();

  // Set/Get a function to call to report back information
  void SetInformationMessageCallback( CallbackFunctionType );
  CallbackFunctionType GetInformationMessageCallback();

  // Set/Get a function to call to report back a discovered module
  void SetModuleDiscoveryMessageCallback( CallbackFunctionType );
  CallbackFunctionType GetModuleDiscoveryMessageCallback();
  
  
protected:
  // Scan for shared object modules (i.e. DLL) in the module search
  // path. Returns the number of modules found.
  virtual long ScanForSharedObjectModules();

  // Scan for command line modules in the module search path. Command
  // line modules are executables that respond to a --xml
  // argument. Returns the number of modules found.
  virtual long ScanForCommandLineModules();

  void WarningMessage( const char *);
  void ErrorMessage( const char *);
  void InformationMessage( const char *);
  void ModuleDiscoveryMessage( const char *);
  
  
private:
  ModuleDescriptionMap *InternalMap;

  std::string SearchPath;

  CallbackFunctionType WarningMessageCallback;
  CallbackFunctionType ErrorMessageCallback;
  CallbackFunctionType InformationMessageCallback;
  CallbackFunctionType ModuleDiscoveryMessageCallback;
};


#endif

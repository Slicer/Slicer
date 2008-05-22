/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Module Description Parser
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/
#ifndef __ModuleFactory_h
#define __ModuleFactory_h

#include "ModuleDescriptionParserWin32Header.h"

#include "ModuleDescription.h"

#include <vector>

class ModuleDescriptionMap;
class ModuleFileMap;
class ModuleCache;

class ModuleDescriptionParser_EXPORT ModuleFactory
{
public:
  ModuleFactory();
  virtual ~ModuleFactory();

  // Set the name of the module factory.  The name is used in error
  // messages (to refer to the application using the module factory)
  // and in encoding entry points.
  void SetName( const std::string& name) { Name = name; }

  // Get the name of the module factory.  The name is used in error
  // messages (to refer to the application using the module factory)
  // and in encoding entry points.
  const std::string& GetName() const { return Name; }
  
  // Set the search path for modules (both command line modules and
  // shared object modules). A list of ':' or ';' separated paths.
  void SetSearchPaths(const std::string& paths) { SearchPaths = paths; }

  // Get the module search path.
  const std::string& GetSearchPaths() const { return SearchPaths; }

  // Set the cache path for modules (both command line modules and
  // shared object modules). The cache is an area where information on
  // a mocule is cached between sessions.
  void SetCachePath(const std::string& path) { CachePath = path; }

  // Get the module cache path.
  const std::string& GetCachePath() const { return CachePath; }
  
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
  // Load the module cache.
  virtual void LoadModuleCache();

  // Save the module cache.
  virtual void SaveModuleCache();

  // Get the module description from the cache. Returns 0 if module
  // was not found in the cache.  Returns 1 if module found in the
  // cache and used.  Returns 2 if module found in the cache but not
  // used because it was not a module. commandName is the full path to
  // the module file being tested.
  virtual int GetModuleFromCache(const std::string &commandName,
                                 long int commandModifiedTime,
                                 const std::string & type,
                                 std::stringstream &stream);
  
  // Scan for shared object modules (i.e. DLL) in the module search
  // path. Modules can either have global symbols or entry points to
  // describe the module and logos. Returns the number of modules
  // found (that have not already been discovered by another method).
  virtual long ScanForSharedObjectModules();

  // Scan for command line modules in the module search path. Command
  // line modules are executables that respond to a --xml
  // argument. Returns the number of modules found (that have not
  // already been discovered by another method).
  virtual long ScanForCommandLineModulesByExecuting();

  // Scan for command line module in the module search path. Command
  // line modules are executables with global symbols that can be
  // queried without executing the program. Returns the number of
  // modules found (that have not already been discovered by another
  // method). 
  virtual long ScanForCommandLineModulesByPeeking();

  // Scan for Python modules in the module search path. 
  virtual long ScanForPythonModulesByLoading();

  // Scan for files which are known (by exclusion) to not be modules
  virtual long ScanForNotAModuleFiles();

  // Get the logo for a command line module
  virtual void GetLogoForCommandLineModuleByExecuting(ModuleDescription&);
  
  void WarningMessage( const char *);
  void ErrorMessage( const char *);
  void InformationMessage( const char *);
  void ModuleDiscoveryMessage( const char *);
  
  ModuleCache *InternalCache;
  ModuleDescriptionMap *InternalMap;
  ModuleFileMap *InternalFileMap;

  bool CacheModified;
  
private:

  std::string Name;
  std::string SearchPaths;
  std::string CachePath;

  CallbackFunctionType WarningMessageCallback;
  CallbackFunctionType ErrorMessageCallback;
  CallbackFunctionType InformationMessageCallback;
  CallbackFunctionType ModuleDiscoveryMessageCallback;
};


#endif

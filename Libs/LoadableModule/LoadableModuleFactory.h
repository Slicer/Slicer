/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Dynamic Module
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/
#ifndef __LoadableModuleFactory_h
#define __LoadableModuleFactory_h

#include "LoadableModuleWin32Header.h"

#include "LoadableModuleDescription.h"

#include <vector>

class LoadableModuleDescriptionMap;
class LoadableModuleFileMap;
class LoadableModuleCache;

class LoadableModule_EXPORT LoadableModuleFactory
{
public:
  LoadableModuleFactory();
  virtual ~LoadableModuleFactory();

  // Set the name of the module factory.  The name is used in error
  // messages (to refer to the application using the module factory)
  // and in encoding entry points.
  void SetName( const std::string& name) { Name = name; }

  // Get the name of the module factory.  The name is used in error
  // messages (to refer to the application using the module factory)
  // and in encoding entry points.
  const std::string& GetName() const { return Name; }
  
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
  LoadableModuleDescription GetModuleDescription(const std::string&) const;

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
  // path. Modules can either have global symbols or entry points to
  // describe the module and logos. Returns the number of modules
  // found (that have not already been discovered by another method).
  virtual long ScanForSharedObjectModules();

  void WarningMessage( const char *);
  void ErrorMessage( const char *);
  void InformationMessage( const char *);
  void ModuleDiscoveryMessage( const char *);
  
  LoadableModuleCache *InternalCache;
  LoadableModuleDescriptionMap *InternalMap;
  LoadableModuleFileMap *InternalFileMap;

  bool CacheModified;
  
private:

  std::string Name;
  std::string SearchPath;
  std::string CachePath;

  CallbackFunctionType WarningMessageCallback;
  CallbackFunctionType ErrorMessageCallback;
  CallbackFunctionType InformationMessageCallback;
  CallbackFunctionType ModuleDiscoveryMessageCallback;
};


#endif

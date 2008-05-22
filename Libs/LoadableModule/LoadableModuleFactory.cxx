/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Loadable Module
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/
#include "itksys/DynamicLoader.hxx" 
#include "itksys/Directory.hxx"
#include "itksys/SystemTools.hxx"
#include "itksys/Process.h"
#include "itksys/Base64.h"

#include "LoadableModuleFactory.h"
#include "LoadableModuleDescriptionParser.h"
#include "LoadableModuleDescription.h"

#include <set>
#include <map>
#include <sstream>
#include <fstream>
#include <cctype>
#include <algorithm>
#include <deque>

#if !defined(WIN32) && defined(HAVE_BFD)
#include "BinaryFileDescriptor.h"
#endif

#ifdef LoadableModule_USE_PYTHON
// If debug, Python wants pythonxx_d.lib, so fake it out
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif
#endif

static void
splitString (std::string &text,
             std::string &separators,
             std::vector<std::string> &words)
{
  int n = text.length();
  int start, stop;
  start = text.find_first_not_of(separators);
  while ((start >= 0) && (start < n))
    {
    stop = text.find_first_of(separators, start);
    if ((stop < 0) || (stop > n)) stop = n;
    words.push_back(text.substr(start, stop - start));
    start = text.find_first_not_of(separators, stop+1);
    }
}

/**
 * A file scoped function to determine if a file has
 * the shared library extension in its name, this converts name to lower
 * case before the compare, DynamicLoader always uses
 * lower case for LibExtension values.
 */
inline bool 
NameIsSharedLibrary(const char* name)
{
  std::string extension = itksys::DynamicLoader::LibExtension();

#ifdef __APPLE__
  // possible bug: CMake generated build file on the Mac makes
  // libraries with a .dylib extension.  kwsys guesses the extension
  // should be ".so"
  extension = ".dylib";
#endif
  
  std::string sname = name;
  if ( sname.rfind(extension) == sname.size() - extension.size() )
    {
    return true;
    }
  return false;
}


/**
 * A file scope typedef to make the cast code to the load
 * function cleaner to read.
 */
typedef char * (*ModuleDescriptionFunction)();
typedef void * (*ModuleGUIFunction)();
typedef void * (*ModuleLogicFunction)();


// Private implementaton of an std::map
class LoadableModuleDescriptionMap : public std::map<std::string, LoadableModuleDescription> {};
class LoadableModuleFileMap : public std::set<std::string> {};

// Private datastructures for module cache.
//
//

// cache entry for a module
struct LoadableModuleCacheEntry
{
  std::string Location;        // complete path to a file
  unsigned int ModifiedTime;    // file's modified time
  std::string Type;            // SharedObjectModule, CommandLineModule, PythonModule, NotAModule
//  std::string Title;           // name of the module
  std::string XMLDescription;  // Module description
  int LogoWidth;
  int LogoHeight;
  int LogoPixelSize;
  unsigned long LogoLength;
  std::string Logo;
};

// map from a filename to cache entry
class LoadableModuleCache : public std::map<std::string, LoadableModuleCacheEntry> {};


// ---

LoadableModuleFactory::LoadableModuleFactory()
{
  this->Name = "Application";
  this->InternalCache = new LoadableModuleCache;
  this->InternalMap = new LoadableModuleDescriptionMap;
  this->InternalFileMap = new LoadableModuleFileMap;
  this->WarningMessageCallback = 0;
  this->ErrorMessageCallback = 0;
  this->InformationMessageCallback = 0;
  this->ModuleDiscoveryMessageCallback = 0;
  this->CacheModified = false;
}

LoadableModuleFactory::~LoadableModuleFactory()
{
  delete this->InternalCache;
  delete this->InternalMap;
  delete this->InternalFileMap;
}


void
LoadableModuleFactory::WarningMessage(const char *msg)
{
  if (this->WarningMessageCallback && msg)
    {
    (*this->WarningMessageCallback)(msg);
    }
}

void
LoadableModuleFactory::ErrorMessage(const char *msg)
{
  if (this->ErrorMessageCallback && msg)
    {
    (*this->ErrorMessageCallback)(msg);
    }
}

void
LoadableModuleFactory::InformationMessage(const char *msg)
{
  if (this->InformationMessageCallback && msg)
    {
    (*this->InformationMessageCallback)(msg);
    }
}

void
LoadableModuleFactory::ModuleDiscoveryMessage(const char *msg)
{
  if (this->ModuleDiscoveryMessageCallback && msg)
    {
    (*this->ModuleDiscoveryMessageCallback)(msg);
    }
}


void
LoadableModuleFactory::SetWarningMessageCallback( CallbackFunctionType f )
{
  this->WarningMessageCallback = f;
}

LoadableModuleFactory::CallbackFunctionType
LoadableModuleFactory::GetWarningMessageCallback()
{
  return this->WarningMessageCallback;
}

void
LoadableModuleFactory::SetErrorMessageCallback( CallbackFunctionType f )
{
  this->ErrorMessageCallback = f;
}

LoadableModuleFactory::CallbackFunctionType
LoadableModuleFactory::GetErrorMessageCallback()
{
  return this->ErrorMessageCallback;
}

void
LoadableModuleFactory::SetInformationMessageCallback( CallbackFunctionType f )
{
  this->InformationMessageCallback = f;
}

LoadableModuleFactory::CallbackFunctionType
LoadableModuleFactory::GetInformationMessageCallback()
{
  return this->InformationMessageCallback;
}

void
LoadableModuleFactory::SetModuleDiscoveryMessageCallback( CallbackFunctionType f )
{
  this->ModuleDiscoveryMessageCallback = f;
}

LoadableModuleFactory::CallbackFunctionType
LoadableModuleFactory::GetModuleDiscoveryMessageCallback()
{
  return this->ModuleDiscoveryMessageCallback;
}


std::vector<std::string>
LoadableModuleFactory
::GetModuleNames() const
{
  std::vector<std::string> names;

  std::map<std::string, LoadableModuleDescription>::const_iterator mit;

  for (mit = this->InternalMap->begin(); mit != this->InternalMap->end(); ++mit)
    {
    names.push_back( (*mit).first );
    }

  return names;
}

LoadableModuleDescription
LoadableModuleFactory
::GetModuleDescription(const std::string& name) const
{
  std::map<std::string, LoadableModuleDescription>::const_iterator mit;

  mit = this->InternalMap->find(name);

  if (mit != this->InternalMap->end())
    {
    return (*mit).second;
    }

  return LoadableModuleDescription();
}



void
LoadableModuleFactory
::Scan()
{  
  int numberOfShared = this->ScanForSharedObjectModules();

  if (numberOfShared == 0)
    {
    this->WarningMessage( ("No loadable modules found. Check your module search path and your " + this->Name + " installation.").c_str() );
    }
  else
    {
      // check Dependencies of each module, remove from list if
      // dependencies not met

      std::map<std::string, LoadableModuleDescription>::iterator mit = this->InternalMap->begin();
      while (mit != this->InternalMap->end())
        {
          std::vector<std::string> deps = (*mit).second.GetDependencies();
          if (0 == deps.size())
            {
              // no dependencies, continue
              ++mit;
              continue;
            }

          bool found_all = true;
          std::vector<std::string>::iterator iter;
          for (iter = deps.begin();
               iter != deps.end();
               ++iter)
            {
              LoadableModuleDescription desc = this->GetModuleDescription(*iter);
              if (desc.GetName().empty()) {
                found_all = false;
                break;
              }
            }

          if (!found_all)
            {
              this->WarningMessage( ("Dependency (" + (*iter) + ") for module " + (*mit).second.GetName()  + " not met, will not load.").c_str() );
              this->InternalMap->erase(mit++);
            }
          else
            {
              ++mit;
            }
        }// mit != this->InternalMap.end()

      // reorder so that modules without dependencies loaded first

    }// numberShared == 0


}

long
LoadableModuleFactory
::ScanForSharedObjectModules()
{
  // add any of the self-describing shared object modules available
  //
  // self-describing shared object modules live in a prescribed path
  // and have a prescribed symbol.
  if (this->SearchPaths == "")
    {
    this->WarningMessage( "Empty module search paths." );
    return 0;
    }
  
  std::vector<std::string> modulePaths;
#ifdef _WIN32
  std::string delim(";");
#else
  std::string delim(":");
#endif
  splitString(this->SearchPaths, delim, modulePaths);

  std::vector<std::string>::const_iterator pit;
  long numberTested = 0;
  long numberFound = 0;
  double t0, t1;
  
  t0 = itksys::SystemTools::GetTime();  
  for (pit = modulePaths.begin(); pit != modulePaths.end(); ++pit)
    {
    std::stringstream information;
  
    information << "Searching " << *pit
                << " for loadable module libraries." << std::endl;
    
    itksys::Directory directory;
    directory.Load( (*pit).c_str() );

    for ( unsigned int ii=0; ii < directory.GetNumberOfFiles(); ++ii)
      {
      bool isAPlugin = true;
      const char *filename = directory.GetFile(ii);
      
      // skip any directories
      if (!itksys::SystemTools::FileIsDirectory(filename))
        {
        // make sure the file has a shared library extension
        if ( NameIsSharedLibrary(filename) )
          {
          numberTested++;
          
          // library name 
          std::string fullLibraryPath = std::string(directory.GetPath())
            + "/" + filename;
          //std::cout << "Checking " << fullLibraryPath << std::endl;

          // early exit if we have already tested this file and succeeded
          LoadableModuleFileMap::iterator fit
            = this->InternalFileMap->find(fullLibraryPath);
          if (fit != this->InternalFileMap->end())
            {
            // file was already discovered as a module
            information << "Module already discovered at " << fullLibraryPath
                        << std::endl;
            continue;
            }


          // determine the modified time of the module
          long int libraryModifiedTime
            = itksys::SystemTools::ModifiedTime(fullLibraryPath.c_str());

          itksys::DynamicLoader::LibraryHandle lib
            = itksys::DynamicLoader::OpenLibrary(fullLibraryPath.c_str());
          if ( lib )
            {
            // Look for the entry points and symbols to get an XML
            // description of the module.  Symbols (constants) are
            // used if they exist, otherwise entry points are used.
            char *xmlSymbol = 0;
            ModuleDescriptionFunction descFunction = 0;
            ModuleGUIFunction guiFunction = 0;
            ModuleLogicFunction logicFunction = 0;

            xmlSymbol = (char*)itksys::DynamicLoader::GetSymbolAddress(lib, "LoadableModuleDescription");

            if (!xmlSymbol)
              {
              descFunction = (ModuleDescriptionFunction)itksys::DynamicLoader::GetSymbolAddress(lib, "GetLoadableModuleDescription");
              }
            
            if (xmlSymbol || descFunction)
              {
              logicFunction = (ModuleLogicFunction)itksys::DynamicLoader::GetSymbolAddress(lib, "GetLoadableModuleLogic");

              if (logicFunction)
                {
                guiFunction = (ModuleGUIFunction)itksys::DynamicLoader::GetSymbolAddress(lib, "GetLoadableModuleGUI");
                }
              }

            // if the symbols are found, then get the XML descriptions
            // and cache the gui function to run the module
            if ( (xmlSymbol || descFunction) && guiFunction )
              {
              std::string xml;
              if (xmlSymbol)
                {
                xml = xmlSymbol; // make a string out of the symbol
                }
              else
                {
                xml = (*descFunction)(); // call the function
                }

              // check if the module generated a valid xml description
              if (xml.compare(0, 5, "<?xml") == 0)
                {
                this->InternalFileMap->insert(fullLibraryPath);
                
                // Construct and configure the module object
                LoadableModuleDescription module;
                module.SetType("SharedObjectModule");

                module.SetLogicPtr( reinterpret_cast<vtkSlicerModuleLogic*> ((*logicFunction)()) );
                module.SetGUIPtr( reinterpret_cast<vtkSlicerModuleGUI*> ((*guiFunction)()) );

                // Set the target as the entry point to call
                char entryPointAsText[256];
                std::string entryPointAsString;
                std::string lowerName = this->Name;
                std::transform(lowerName.begin(), lowerName.end(),
                               lowerName.begin(),
                               (int (*)(int))std::tolower);
                
                sprintf(entryPointAsText, "%p", guiFunction);
                entryPointAsString = lowerName + ":" + entryPointAsText;
                module.SetTarget( entryPointAsString );
                module.SetLocation( fullLibraryPath );

                // Parse the xml to build the description of the module
                // and the parameters
                LoadableModuleDescriptionParser parser;
                parser.Parse(xml, module);

                // :KLUGE: 20080225 tgl: The name of the TCL
                // initializer function is in the description.

                module.SetTclInitFunction( (TclInit)itksys::DynamicLoader::GetSymbolAddress(lib, module.GetTclInitName().c_str()) );

                // Check to make sure the module is not already in the
                // lists
                LoadableModuleDescriptionMap::iterator mit
                  = this->InternalMap->find(module.GetName());

                std::string splash_msg("Discovered ");
                splash_msg +=  module.GetShortName();
                splash_msg += " Loadable Module (adding to cache)...";
                this->ModuleDiscoveryMessage(splash_msg.c_str());
                
                if (mit == this->InternalMap->end())
                  {

                  // Store the module in the list
                  (*this->InternalMap)[module.GetName()] =  module ;
                  
                  information << "A loadable module named \"" << module.GetName()
                              << "\" has been discovered at "
                              << module.GetLocation() << "("
                              << module.GetTarget() << ")" << std::endl;
                  numberFound++;
                  }
                else
                  {
                  // module already exists, set the alternative type,
                  // location, and target if not already set
                  if ((*mit).second.GetAlternativeType().empty()
                      && (*mit).second.GetType() != module.GetType())
                    {
                    (*mit).second.SetAlternativeType( module.GetType() );
                    (*mit).second.SetAlternativeLocation(module.GetLocation());
                    (*mit).second.SetAlternativeTarget( module.GetTarget() );
                    }
                  
                  information << "A loadable module named \"" << module.GetName()
                            << "\" has already been discovered." << std::endl
                            << "    First discovered at "
                            << (*mit).second.GetLocation()
                            << "(" << (*mit).second.GetTarget() << ")"
                            << std::endl
                            << "    Then discovered at "
                            << module.GetLocation()
                            << "(" << module.GetTarget() << ")"
                            << std::endl
                            << "    Keeping first loadable module." << std::endl;
                  }

                // Put the module in the cache
                LoadableModuleCacheEntry entry;
                entry.Location = fullLibraryPath;
                entry.ModifiedTime = libraryModifiedTime;
                entry.Type = "SharedObjectModule";
                entry.XMLDescription = xml;
                
                (*this->InternalCache)[entry.Location] = entry;
                this->CacheModified = true;
                }
              else
                {
                // not a plugin, no xml description, close the library
                itksys::DynamicLoader::CloseLibrary(lib);

                isAPlugin = false;
                information << filename
                            << " is not a loadable module (no XML description)."
                            << std::endl;
                }
              }
            else
              {
//                std::cout << "Symbols not found." << std::endl;
//                std::cout << "descFunction: " << (void*)descFunction << std::endl;
//                std::cout << "guiFunction: " << (void*)guiFunction << std::endl;
//                std::cout << "logicFunction: " << (void*)logicFunction <<
//                std::endl;

              // not a plugin, doesn't have the symbols, close the library
              itksys::DynamicLoader::CloseLibrary(lib);

              isAPlugin = false;
              information << filename
                          << " is not a loadable module (no entry points)."
                          << std::endl;
              }
            }
          }
        }
      }

    this->InformationMessage( information.str().c_str() );
    }
  t1 = itksys::SystemTools::GetTime();
  
  std::stringstream information;
  information << "Tested " << numberTested << " files as loadable module libraries. Found "
              << numberFound << " new modules in " << t1 - t0
              << " seconds." << std::endl;
  
  this->InformationMessage( information.str().c_str() );

  return numberFound;
}


/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Module Description Parser
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/
#include "itksys/DynamicLoader.hxx" 
#include "itksys/Directory.hxx"
#include "itksys/SystemTools.hxx"
#include "itksys/Process.h"

#include "ModuleFactory.h"
#include "ModuleDescriptionParser.h"
#include "ModuleDescription.h"

#include <set>
#include <map>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <deque>
#include <dlfcn.h>

#if !defined(WIN32)
#include "BinaryFileDescriptor.h"
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
 * A file scoped function to determine if a file has
 * the executable extension in its name
 */
inline bool 
NameIsExecutable(const char* name)
{
  static const char* standardExtensions[] = {".bat", ".com", ".sh", ".csh", ".tcsh", ".pl", ".tcl", ".py", ".m"};
  std::deque<std::string> extensions(standardExtensions, standardExtensions+9);


  bool hasDefaultExtension = false;
  std::string extension = itksys::SystemTools::GetExecutableExtension();
  if (extension != "")
    {
    hasDefaultExtension = true;
    extensions.push_front( extension );
    }

  bool foundIt = false;
  std::string sname = name;
  std::transform(sname.begin(), sname.end(), sname.begin(),
                 (int (*)(int))std::tolower);
  
  for (std::deque<std::string>::iterator it=extensions.begin();
       !foundIt && it != extensions.end(); ++it)
    {
    if ( sname.rfind(*it) == sname.size() - (*it).size())
      {
      foundIt = true;
      }
    }

  if (hasDefaultExtension)
    {
    // if we have a default extension return whether we found any
    // of the extensions
    return foundIt;
    }
  else if (!foundIt)
    {
    // if there is not default extension and we have not found any of
    // the other possible extensions for an executable, then check
    // whether we can exclude the file because it has a shared object
    // extension. 
    return !NameIsSharedLibrary(name);
    }
  else
    {
    // no default extension but we found another extension that can be
    // an executable
    return true;
    }

  return true;
}



/**
 * A file scope typedef to make the cast code to the load
 * function cleaner to read.
 */
typedef char * (*XMLModuleDescriptionFunction)();
typedef int (*ModuleEntryPoint)(int argc, char* argv[]);
typedef unsigned char * (*ModuleLogoFunction)(int *width, int *height, int *pixel_size, unsigned long *bufferLength);


// Private implementaton of an std::map
class ModuleDescriptionMap : public std::map<std::string, ModuleDescription> {};
class ModuleFileMap : public std::set<std::string> {};

// ---

ModuleFactory::ModuleFactory()
{
  this->Name = "Application";
  this->InternalMap = new ModuleDescriptionMap;
  this->InternalFileMap = new ModuleFileMap;
  this->WarningMessageCallback = 0;
  this->ErrorMessageCallback = 0;
  this->InformationMessageCallback = 0;
  this->ModuleDiscoveryMessageCallback = 0;
}

ModuleFactory::~ModuleFactory()
{
  delete this->InternalMap;
  delete this->InternalFileMap;
}


void
ModuleFactory::WarningMessage(const char *msg)
{
  if (this->WarningMessageCallback && msg)
    {
    (*this->WarningMessageCallback)(msg);
    }
}

void
ModuleFactory::ErrorMessage(const char *msg)
{
  if (this->ErrorMessageCallback && msg)
    {
    (*this->ErrorMessageCallback)(msg);
    }
}

void
ModuleFactory::InformationMessage(const char *msg)
{
  if (this->InformationMessageCallback && msg)
    {
    (*this->InformationMessageCallback)(msg);
    }
}

void
ModuleFactory::ModuleDiscoveryMessage(const char *msg)
{
  if (this->ModuleDiscoveryMessageCallback && msg)
    {
    (*this->ModuleDiscoveryMessageCallback)(msg);
    }
}


void
ModuleFactory::SetWarningMessageCallback( CallbackFunctionType f )
{
  this->WarningMessageCallback = f;
}

ModuleFactory::CallbackFunctionType
ModuleFactory::GetWarningMessageCallback()
{
  return this->WarningMessageCallback;
}

void
ModuleFactory::SetErrorMessageCallback( CallbackFunctionType f )
{
  this->ErrorMessageCallback = f;
}

ModuleFactory::CallbackFunctionType
ModuleFactory::GetErrorMessageCallback()
{
  return this->ErrorMessageCallback;
}

void
ModuleFactory::SetInformationMessageCallback( CallbackFunctionType f )
{
  this->InformationMessageCallback = f;
}

ModuleFactory::CallbackFunctionType
ModuleFactory::GetInformationMessageCallback()
{
  return this->InformationMessageCallback;
}

void
ModuleFactory::SetModuleDiscoveryMessageCallback( CallbackFunctionType f )
{
  this->ModuleDiscoveryMessageCallback = f;
}

ModuleFactory::CallbackFunctionType
ModuleFactory::GetModuleDiscoveryMessageCallback()
{
  return this->ModuleDiscoveryMessageCallback;
}


std::vector<std::string>
ModuleFactory
::GetModuleNames() const
{
  std::vector<std::string> names;

  std::map<std::string, ModuleDescription>::const_iterator mit;

  for (mit = this->InternalMap->begin(); mit != this->InternalMap->end(); ++mit)
    {
    names.push_back( (*mit).first );
    }

  return names;
}

ModuleDescription
ModuleFactory
::GetModuleDescription(const std::string& name) const
{
  std::map<std::string, ModuleDescription>::const_iterator mit;

  mit = this->InternalMap->find(name);

  if (mit != this->InternalMap->end())
    {
    return (*mit).second;
    }

  return ModuleDescription();
}



void
ModuleFactory
::Scan()
{
  // Scan for shared object modules first since they will be higher
  // performance than command line module
  int numberOfShared, numberOfExecutables, numberOfPeekedExecutables;

  numberOfShared = this->ScanForSharedObjectModules();
  numberOfPeekedExecutables = this->ScanForCommandLineModulesByPeeking();
  numberOfExecutables = this->ScanForCommandLineModulesByExecuting();

  if (numberOfShared + numberOfExecutables + numberOfPeekedExecutables == 0)
    {
    this->WarningMessage( ("No plugin modules found. Check your module search path and your " + this->Name + " installation.").c_str() );
    }
}

long
ModuleFactory
::ScanForSharedObjectModules()
{
  // add any of the self-describing shared object modules available
  //
  // self-describing shared object modules live in a prescribed path
  // and have a prescribed symbol.
  if (this->SearchPath == "")
    {
    this->WarningMessage( "Empty module search path." );
    return 0;
    }
  
  std::vector<std::string> modulePaths;
#ifdef _WIN32
  std::string delim(";");
#else
  std::string delim(":");
#endif
  splitString(this->SearchPath, delim, modulePaths);

  std::vector<std::string>::const_iterator pit;
  long numberTested = 0;
  long numberFound = 0;
  double t0, t1;
  
  t0 = itksys::SystemTools::GetTime();  
  for (pit = modulePaths.begin(); pit != modulePaths.end(); ++pit)
    {
    std::stringstream information;
  
    information << "Searching " << *pit
                << " for shared object plugins." << std::endl;
    
    itksys::Directory directory;
    directory.Load( (*pit).c_str() );

    for ( unsigned int ii=0; ii < directory.GetNumberOfFiles(); ++ii)
      {
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
          ModuleFileMap::iterator fit
            = this->InternalFileMap->find(fullLibraryPath);
          if (fit != this->InternalFileMap->end())
            {
            // file was already discovered as a module
            information << "Module already discovered at " << fullLibraryPath
                        << std::endl;
            continue;
            }


          itksys::DynamicLoader::LibraryHandle lib
            = itksys::DynamicLoader::OpenLibrary(fullLibraryPath.c_str());
          if ( lib )
            {
            // Look for the entry points and symbols to get an XML
            // description of the module, execute the module, and
            // define a logo.  Symbols (constants) are used if they
            // exist, otherwise entry points are used.
            char *xmlSymbol = 0;
            XMLModuleDescriptionFunction xmlFunction = 0;
            ModuleEntryPoint entryPoint = 0;

            ModuleLogoFunction logoFunction = 0;
            unsigned char *logoImage = 0;
            int *logoWidth=0, *logoHeight=0, *logoPixelSize=0;
            unsigned long *logoLength=0;
            
            xmlSymbol = (char*)itksys::DynamicLoader::GetSymbolAddress(lib, "XMLModuleDescription");

            if (!xmlSymbol)
              {
              xmlFunction = (XMLModuleDescriptionFunction)itksys::DynamicLoader::GetSymbolAddress(lib, "GetXMLModuleDescription");
              }
            
            if (xmlSymbol || xmlFunction)
              {
              entryPoint = (ModuleEntryPoint)itksys::DynamicLoader::GetSymbolAddress(lib, "ModuleEntryPoint");

              if (entryPoint)
                {
                // look for logo variables
                logoImage = (unsigned char *)itksys::DynamicLoader::GetSymbolAddress(lib, "ModuleLogoImage");
                
                if (logoImage)
                  {
                  logoWidth = (int *)itksys::DynamicLoader::GetSymbolAddress(lib, "ModuleLogoWidth");
                  logoHeight = (int *)itksys::DynamicLoader::GetSymbolAddress(lib, "ModuleLogoHeight");
                  logoPixelSize = (int *)itksys::DynamicLoader::GetSymbolAddress(lib, "ModuleLogoPixelSize");
                  logoLength = (unsigned long *)itksys::DynamicLoader::GetSymbolAddress(lib, "ModuleLogoLength");                  
                  }
                else
                  {
                  // look for logo function
                  logoFunction
                    = (ModuleLogoFunction)itksys::DynamicLoader::GetSymbolAddress(lib, "GetModuleLogo");
                  }
                }
              }

            // if the symbols are found, then get the XML descriptions
            // and cache the entry point to run the module
            if ( (xmlSymbol || xmlFunction) && entryPoint )
              {
              std::string xml;
              if (xmlSymbol)
                {
                xml = xmlSymbol; // make a string out of the symbol
                }
              else
                {
                xml = (*xmlFunction)(); // call the function
                }

              // check if the module generated a valid xml description
              if (xml.compare(0, 5, "<?xml") == 0)
                {
                this->InternalFileMap->insert(fullLibraryPath);
                
                // Construct and configure the module object
                ModuleDescription module;

                // Set the target as the entry point to call
                char entryPointAsText[256];
                std::string entryPointAsString;
                std::string lowerName = this->Name;
                std::transform(lowerName.begin(), lowerName.end(),
                               lowerName.begin(),
                               (int (*)(int))std::tolower);
                
                sprintf(entryPointAsText, "%p", entryPoint);
                entryPointAsString = lowerName + ":" + entryPointAsText;
                module.SetTarget( entryPointAsString );
                module.SetLocation( fullLibraryPath );

                // Parse the xml to build the description of the module
                // and the parameters
                ModuleDescriptionParser parser;
                parser.Parse(xml, module);

                // Check to make sure the module is not already in the
                // list
                ModuleDescriptionMap::iterator mit
                  = this->InternalMap->find(module.GetTitle());

                std::string splash_msg("Discovered ");
                splash_msg +=  module.GetTitle();
                splash_msg += " Module...";
                this->ModuleDiscoveryMessage(splash_msg.c_str());
                
                if (mit == this->InternalMap->end())
                  {
                  // Store the module in the list
                  (*this->InternalMap)[module.GetTitle()] =  module ;

                  information << "A module named \"" << module.GetTitle()
                              << "\" has been discovered at "
                              << module.GetLocation() << "("
                              << module.GetTarget() << ")" << std::endl;
                  numberFound++;
                  }
                else
                  {
                  information << "A module named \"" << module.GetTitle()
                            << "\" has already been discovered." << std::endl
                            << "    First discovered at "
                            << (*mit).second.GetLocation()
                            << "(" << (*mit).second.GetTarget() << ")"
                            << std::endl
                            << "    Then discovered at "
                            << module.GetLocation()
                            << "(" << module.GetTarget() << ")"
                            << std::endl
                            << "    Keeping first module." << std::endl;
                  }

                if (logoImage)
                  {
                  // construct a module logo and set it on the module
                  ModuleLogo mLogo;
                  mLogo.SetLogo( logoImage, *logoWidth, *logoHeight,
                                 *logoPixelSize, *logoLength, 0);
                  (*this->InternalMap)[module.GetTitle()].SetLogo(mLogo);
                  }
                else if (logoFunction)
                  {                  
                  int width, height, pixelSize;
                  unsigned long bufferLength;
                  
                  // call the logo function to get the pixels and information
                  unsigned char *logo = (*logoFunction)(&width, &height,
                                                        &pixelSize,
                                                        &bufferLength);
                  
                  // construct a module logo and set it on the module
                  ModuleLogo mLogo;
                  mLogo.SetLogo( logo, width, height, pixelSize,
                                 bufferLength, 0);
                  (*this->InternalMap)[module.GetTitle()].SetLogo(mLogo);
                  }
                }
              else
                {
                // not a plugin, no xml description, close the library
                itksys::DynamicLoader::CloseLibrary(lib);
                }
              }
            else
              {
//               std::cout << "Symbols not found." << std::endl;
//               std::cout << "xmlFunction: " << (void*)xmlFunction << std::endl;
//               std::cout << "entryPoint: " << (void*)entryPoint <<
//               std::endl;

              // not a plugin, doesn't have the symbols, close the library
              itksys::DynamicLoader::CloseLibrary(lib);
              }
            }
          }
        }
      }

    this->InformationMessage( information.str().c_str() );
    }
  t1 = itksys::SystemTools::GetTime();
  
  std::stringstream information;
  information << "Tested " << numberTested << " files as shared object plugins. Found "
              << numberFound << " new plugins in " << t1 - t0
              << " seconds." << std::endl;
  
  this->InformationMessage( information.str().c_str() );

  return numberFound;
}

long
ModuleFactory
::ScanForCommandLineModulesByExecuting()
{
  // add any of the self-describing command-line modules available
  //
  // self-describing command-line modules live in a prescribed
  // path and respond to a command line argument "--xml"
  //
  if (this->SearchPath == "")
    {
    this->WarningMessage( "Empty module search path." ); 
    return 0;
    }
  
  std::vector<std::string> modulePaths;
#ifdef _WIN32
  std::string delim(";");
#else
  std::string delim(":");
#endif
  splitString(this->SearchPath, delim, modulePaths);

  std::vector<std::string>::const_iterator pit;
  long numberTested = 0;
  long numberFound = 0;
  double t0, t1;

  t0 = itksys::SystemTools::GetTime();
  for (pit = modulePaths.begin(); pit != modulePaths.end(); ++pit)
    {
    std::stringstream information;
    
    information << "Searching " << *pit
                << " for command line executable plugins by executing." << std::endl;
    
    itksys::Directory directory;
    directory.Load( (*pit).c_str() );

    for ( unsigned int ii=0; ii < directory.GetNumberOfFiles(); ++ii)
      {
      const char *filename = directory.GetFile(ii);
      
      // skip any directories
      if (!itksys::SystemTools::FileIsDirectory(filename))
        {
        // try to focus only on executables
        if ( NameIsExecutable(filename) )
          {
          numberTested++;
          //std::cout << "Testing " << filename << " as a plugin:" << std::endl;
          std::string commandName = std::string(directory.GetPath())
            + "/" + filename;

          // early exit if we have already tested this file and succeeded
          ModuleFileMap::iterator fit
            = this->InternalFileMap->find(commandName);
          if (fit != this->InternalFileMap->end())
            {
            // file was already discovered as a module
            information << "Module already discovered at " << commandName
                        << std::endl;
            continue;
            }

          // command, process and argument to probe the executable
          char *command[3];
          itksysProcess *process = itksysProcess_New();
          std::string arg("--xml");

          // build the command/parameter array.
          command[0] = const_cast<char*>(commandName.c_str());
          command[1] = const_cast<char*>(arg.c_str());
          command[2] = 0;

          // setup the command
          itksysProcess_SetCommand(process, command);
          itksysProcess_SetOption(process,
                                  itksysProcess_Option_Detach, 0);
          itksysProcess_SetOption(process,
                                  itksysProcess_Option_HideWindow, 1);
          itksysProcess_SetTimeout(process, 5.0); // 5 seconds

          // execute the command
          itksysProcess_Execute(process);

          // Wait for the command to finish
          char *tbuffer;
          int length;
          int pipe;
          std::string stdoutbuffer;
          std::string stderrbuffer;
          while ((pipe = itksysProcess_WaitForData(process ,&tbuffer,
                                                   &length, 0)) != 0)
            {
            if (length != 0 && tbuffer != 0)
              {
              if (pipe == itksysProcess_Pipe_STDOUT)
                {
                stdoutbuffer = stdoutbuffer.append(tbuffer, length);
                }
              else if (pipe == itksysProcess_Pipe_STDERR)
                {
                stderrbuffer = stderrbuffer.append(tbuffer, length);
                }
              }
            }
          itksysProcess_WaitForExit(process, 0);

          // check the exit state / error state of the process
          int result = itksysProcess_GetState(process);
          if (result == itksysProcess_State_Exited)
            {
            // executable exited cleanly and must of done
            // "something" when presented with a "--xml" argument
            // (note that it may have just printed out that it did
            // not understand --xml)
            if (itksysProcess_GetExitValue(process) == 0)
              {
              // executable exited without errors, check if it
              // generated a valid xml description
              if (stdoutbuffer.compare(0, 5, "<?xml") == 0)
                {
                //std::cout << "\t" << filename << " is a plugin." << std::endl;
                this->InternalFileMap->insert( commandName );
              
                // Construct and configure the module object
                ModuleDescription module;
                module.SetTarget( commandName );
                module.SetLocation( commandName );

                // Parse the xml to build the description of the module
                // and the parameters
                ModuleDescriptionParser parser;
                parser.Parse(stdoutbuffer, module);

                // Check to make sure the module is not already in the
                // list
                ModuleDescriptionMap::iterator mit
                  = this->InternalMap->find(module.GetTitle());

                std::string splash_msg("Discovered ");
                splash_msg +=  module.GetTitle();
                splash_msg += " Module...";
                this->ModuleDiscoveryMessage(splash_msg.c_str());
              
                if (mit == this->InternalMap->end())
                  {
                  // See if the module has a logo, if so, store it in
                  // the module description
                  this->GetLogoForCommandLineModuleByExecuting(module);

                  // Store the module in the list
                  (*this->InternalMap)[module.GetTitle()] =  module ;

                  information << "A module named \"" << module.GetTitle()
                              << "\" has been discovered at "
                              << module.GetLocation() 
                              << "(" << module.GetTarget() << ")"
                              << std::endl;
                    
                  numberFound++;
                  }
                else
                  {
                  information << "A module named \"" << module.GetTitle()
                              << "\" has already been discovered." << std::endl
                              << "    First discovered at "
                              << (*mit).second.GetLocation()
                              << "(" << (*mit).second.GetTarget() << ")"
                              << std::endl
                              << "    Then discovered at "
                              << module.GetLocation()
                              << "(" << module.GetTarget() << ")"
                              << std::endl
                              << "    Keeping first module." << std::endl;
                  }
                }
              else
                {
//               std::cout << "\t" << filename << " is not a plugin." << std::endl
//                         << "\t" << filename << " did not generate an xml description." << std::endl;
                }
              }
            else
              {
//             std::cout << "\t" << filename << " is not a plugin." << std::endl
//                       << "\t" << filename << " exited with errors." << std::endl;
              }
            }
          else if (result == itksysProcess_State_Expired)
            {
//           std::cout << "\t" << filename << " is not a plugin." << std::endl
//                     << "\t" << filename << " timeout exceeded." << std::endl;
            }
          else
            {
//           std::cout << "\t" << filename << " is not a plugin." << std::endl
//                     << "\t" << filename << " did not exit cleanly." << std::endl;
            }

          // clean up
          itksysProcess_Delete(process);
          }
        }
      }
    this->InformationMessage( information.str().c_str() );    
    }
  t1 = itksys::SystemTools::GetTime();

  std::stringstream information;
  information << "Tested " << numberTested << " files as command line executable plugins by executing. Found "
              << numberFound << " new plugins in " << t1 - t0
              << " seconds." << std::endl;
  this->InformationMessage( information.str().c_str() );

  return numberFound;
}

#if WIN32
// Implementation of ScanForCommandLineModulesByPeeking() for Windows.
// On Windows, executables can be opened and queried like libraries
// for global symbols.
//
long
ModuleFactory
::ScanForCommandLineModulesByPeeking()
{
  // add any of the self-describing command-line modules available
  //
  // self-describing command-line modules live in a prescribed
  // path and respond to a command line argument "--xml"
  //
  if (this->SearchPath == "")
    {
    this->WarningMessage( "Empty module search path." ); 
    return 0;
    }
  
  std::vector<std::string> modulePaths;
#ifdef _WIN32
  std::string delim(";");
#else
  std::string delim(":");
#endif
  splitString(this->SearchPath, delim, modulePaths);

  std::vector<std::string>::const_iterator pit;
  long numberTested = 0;
  long numberFound = 0;
  double t0, t1;

  t0 = itksys::SystemTools::GetTime();
  for (pit = modulePaths.begin(); pit != modulePaths.end(); ++pit)
    {
    std::stringstream information;
    
    information << "Searching " << *pit
                << " for command line executable plugins by peeking." << std::endl;
    
    itksys::Directory directory;
    directory.Load( (*pit).c_str() );

    for ( unsigned int ii=0; ii < directory.GetNumberOfFiles(); ++ii)
      {
      const char *filename = directory.GetFile(ii);
      
      // skip any directories
      if (!itksys::SystemTools::FileIsDirectory(filename))
        {
        // try to focus only on executables
        if ( NameIsExecutable(filename) )
          {
          numberTested++;
          //std::cout << "Testing " << filename << " as a plugin:" << std::endl;
          // executable path
          std::string fullExecutablePath = std::string(directory.GetPath())
            + "/" + filename;

          // early exit if we have already tested this file and succeeded
          ModuleFileMap::iterator fit
            = this->InternalFileMap->find(fullExecutablePath);
          if (fit != this->InternalFileMap->end())
            {
            // file was already discovered as a module
            information << "Module already discovered at "
                        << fullExecutablePath << std::endl;
            continue;
            }

          itksys::DynamicLoader::LibraryHandle lib
            = itksys::DynamicLoader::OpenLibrary(fullExecutablePath.c_str());

          if ( lib )
            {
            char *xmlSymbol = 0;
            unsigned char *logoImage = 0;
            int *logoWidth=0, *logoHeight=0, *logoPixelSize=0;
            unsigned long *logoLength=0;
            
            xmlSymbol
              = (char*)itksys::DynamicLoader::GetSymbolAddress(lib, "XMLModuleDescription");
            if (!xmlSymbol)
              {
               information << "Did not find xml in " + fullExecutablePath
                           << std::endl;
              }

            if (xmlSymbol)
              {
              // look for logo variables
              logoImage = (unsigned char *)itksys::DynamicLoader::GetSymbolAddress(lib, "ModuleLogoImage");
                
              if (logoImage)
                {
                logoWidth = (int *)itksys::DynamicLoader::GetSymbolAddress(lib, "ModuleLogoWidth");
                logoHeight = (int *)itksys::DynamicLoader::GetSymbolAddress(lib, "ModuleLogoHeight");
                logoPixelSize = (int *)itksys::DynamicLoader::GetSymbolAddress(lib, "ModuleLogoPixelSize");
                logoLength = (unsigned long *)itksys::DynamicLoader::GetSymbolAddress(lib, "ModuleLogoLength");                  
                }
              }

            // Build a module description
            if (xmlSymbol)
              {
              std::string xml(xmlSymbol);

              // check if the module description is valid
              if (xml.compare(0, 5, "<?xml") == 0)
                {
                this->InternalFileMap->insert(fullExecutablePath);

                // Construct and configure the module object
                ModuleDescription module;
                module.SetTarget( fullExecutablePath );
                module.SetLocation( fullExecutablePath );
                
                // Parse the xml to build the description of the module
                // and the parameters
                ModuleDescriptionParser parser;
                parser.Parse(xml, module);

                // Check to make sure the module is not already in the
                // list
                ModuleDescriptionMap::iterator mit
                  = this->InternalMap->find(module.GetTitle());

                std::string splash_msg("Discovered ");
                splash_msg +=  module.GetTitle();
                splash_msg += " Module...";
                this->ModuleDiscoveryMessage(splash_msg.c_str());
                
                if (mit == this->InternalMap->end())
                  {
                  // Store the module in the list
                  (*this->InternalMap)[module.GetTitle()] =  module ;
                  
                  information << "A module named \"" << module.GetTitle()
                              << "\" has been discovered at "
                              << module.GetLocation()
                              << "(" << module.GetTarget() << ")"
                              << std::endl;
                  numberFound++;
                  }
                else
                  {
                  information << "A module named \"" << module.GetTitle()
                            << "\" has already been discovered." << std::endl
                            << "    First discovered at "
                            << (*mit).second.GetLocation()
                            << "(" << (*mit).second.GetTarget() << ")"
                            << std::endl
                            << "    Then discovered at "
                            << module.GetLocation()
                            << "(" << module.GetTarget() << ")"
                            << std::endl
                            << "    Keeping first module." << std::endl;
                  }

                if (logoImage)
                  {
                  // construct a module logo and set it on the module
                  ModuleLogo mLogo;
                  mLogo.SetLogo( logoImage, *logoWidth, *logoHeight,
                                 *logoPixelSize, *logoLength, 0);
                  (*this->InternalMap)[module.GetTitle()].SetLogo(mLogo);
                  }
                }
              }
            itksys::DynamicLoader::CloseLibrary(lib);
            }
          }
        }
      }
    this->InformationMessage( information.str().c_str() );
    }
  t1 = itksys::SystemTools::GetTime();
  
  std::stringstream information;
  information << "Tested " << numberTested << " files as command line executable plugins by peeking. Found "
              << numberFound << " new plugins in " << t1 - t0
              << " seconds." << std::endl;
  
  this->InformationMessage( information.str().c_str() );

  return numberFound;
}
#else
// Implementation of ScanForCommandLineModulesByPeeking() for variants
// of unix.  On Linux, executables cannot be opened and queried like
// libraries because the loader tries to load "main" at a fixed
// address which is already occupied by the current program. This
// implementation, therefore, operates on the object file format
// directly, using the Binary File Descriptor (BFD) library to find
// global symbols.
//
long
ModuleFactory
::ScanForCommandLineModulesByPeeking()
{
  // add any of the self-describing command-line modules available
  //
  // self-describing command-line modules live in a prescribed
  // path and respond to a command line argument "--xml"
  //
  if (this->SearchPath == "")
    {
    this->WarningMessage( "Empty module search path." ); 
    return 0;
    }
  
  std::vector<std::string> modulePaths;
#ifdef _WIN32
  std::string delim(";");
#else
  std::string delim(":");
#endif
  splitString(this->SearchPath, delim, modulePaths);

  std::vector<std::string>::const_iterator pit;
  long numberTested = 0;
  long numberFound = 0;
  double t0, t1;

  t0 = itksys::SystemTools::GetTime();
  for (pit = modulePaths.begin(); pit != modulePaths.end(); ++pit)
    {
    std::stringstream information;
    
    information << "Searching " << *pit
                << " for command line executable plugins by peeking." << std::endl;
    
    itksys::Directory directory;
    directory.Load( (*pit).c_str() );

    for ( unsigned int ii=0; ii < directory.GetNumberOfFiles(); ++ii)
      {
      const char *filename = directory.GetFile(ii);
      
      // skip any directories
      if (!itksys::SystemTools::FileIsDirectory(filename))
        {
        // try to focus only on executables
        if ( NameIsExecutable(filename) )
          {
          numberTested++;
          //std::cout << "Testing " << filename << " as a plugin:" << std::endl;
          // executable path
          std::string fullExecutablePath = std::string(directory.GetPath())
            + "/" + filename;

          // early exit if we have already tested this file and succeeded
          ModuleFileMap::iterator fit
            = this->InternalFileMap->find(fullExecutablePath);
          if (fit != this->InternalFileMap->end())
            {
            // file was already discovered as a module
            information << "Module already discovered at "
                        << fullExecutablePath << std::endl;
            continue;
            }

          BinaryFileDescriptor myBFD;
          bool opened = myBFD.Open(fullExecutablePath.c_str());

          if ( opened )
            {
            char *xmlSymbol = 0;
            unsigned char *logoImage = 0;
            int *logoWidth=0, *logoHeight=0, *logoPixelSize=0;
            unsigned long *logoLength=0;
            
            xmlSymbol
              = (char*) myBFD.GetSymbolAddress("XMLModuleDescription");
            if (!xmlSymbol)
              {
               information << "Did not find xml in " + fullExecutablePath
                           << std::endl;
              }

            if (xmlSymbol)
              {
              // look for logo variables
              logoImage = (unsigned char *) myBFD.GetSymbolAddress("ModuleLogoImage");
                
              if (logoImage)
                {
                logoWidth = (int *) myBFD.GetSymbolAddress("ModuleLogoWidth");
                logoHeight = (int *) myBFD.GetSymbolAddress("ModuleLogoHeight");
                logoPixelSize = (int *) myBFD.GetSymbolAddress("ModuleLogoPixelSize");
                logoLength = (unsigned long *) myBFD.GetSymbolAddress("ModuleLogoLength");                  
                }
              }

            // Build a module description
            if (xmlSymbol)
              {
              std::string xml(xmlSymbol);

              // check if the module description is valid
              if (xml.compare(0, 5, "<?xml") == 0)
                {
                this->InternalFileMap->insert(fullExecutablePath);

                // Construct and configure the module object
                ModuleDescription module;
                module.SetTarget( fullExecutablePath );
                module.SetLocation( fullExecutablePath );
                
                // Parse the xml to build the description of the module
                // and the parameters
                ModuleDescriptionParser parser;
                parser.Parse(xml, module);

                // Check to make sure the module is not already in the
                // list
                ModuleDescriptionMap::iterator mit
                  = this->InternalMap->find(module.GetTitle());

                std::string splash_msg("Discovered ");
                splash_msg +=  module.GetTitle();
                splash_msg += " Module...";
                this->ModuleDiscoveryMessage(splash_msg.c_str());
                
                if (mit == this->InternalMap->end())
                  {
                  // Store the module in the list
                  (*this->InternalMap)[module.GetTitle()] =  module ;
                  
                  information << "A module named \"" << module.GetTitle()
                              << "\" has been discovered at "
                              << module.GetLocation()
                              << "(" << module.GetTarget() << ")"
                              << std::endl;
                  numberFound++;
                  }
                else
                  {
                  information << "A module named \"" << module.GetTitle()
                            << "\" has already been discovered." << std::endl
                            << "    First discovered at "
                            << (*mit).second.GetLocation()
                            << "(" << (*mit).second.GetTarget() << ")"
                            << std::endl
                            << "    Then discovered at "
                            << module.GetLocation()
                            << "(" << module.GetTarget() << ")"
                            << std::endl
                            << "    Keeping first module." << std::endl;
                  }

                if (logoImage)
                  {
                  // construct a module logo and set it on the module
                  ModuleLogo mLogo;
                  mLogo.SetLogo( logoImage, *logoWidth, *logoHeight,
                                 *logoPixelSize, *logoLength, 0);
                  (*this->InternalMap)[module.GetTitle()].SetLogo(mLogo);
                  }
                }
              }
            myBFD.Close();
            }
          }
        }
      }
    this->InformationMessage( information.str().c_str() );
    }
  t1 = itksys::SystemTools::GetTime();
  
  std::stringstream information;
  information << "Tested " << numberTested << " files as command line executable plugins by peeking. Found "
              << numberFound << " new plugins in " << t1 - t0
              << " seconds." << std::endl;
  
  this->InformationMessage( information.str().c_str() );

  return numberFound;
}
#endif
          
void
ModuleFactory
::GetLogoForCommandLineModuleByExecuting(ModuleDescription& module)
{
  itksysProcess *process = itksysProcess_New();
  char *command[3];
  std::string arg("--logo");

  command[0] = const_cast<char*>(module.GetTarget().c_str());
  command[1] = const_cast<char*>(arg.c_str());
  command[2] = 0;

  // setup the command
  itksysProcess_SetCommand(process, command);
  itksysProcess_SetOption(process,
                          itksysProcess_Option_Detach, 0);
  itksysProcess_SetOption(process,
                          itksysProcess_Option_HideWindow, 1);
  itksysProcess_SetTimeout(process, 5.0); // 5 seconds
  
  // execute the command
  itksysProcess_Execute(process);

  // Wait for the command to finish
  char *tbuffer;
  int length;
  int pipe;
  std::string stdoutbuffer;
  std::string stderrbuffer;
  while ((pipe = itksysProcess_WaitForData(process ,&tbuffer,
                                           &length, 0)) != 0)
    {
    if (length != 0 && tbuffer != 0)
      {
      if (pipe == itksysProcess_Pipe_STDOUT)
        {
        stdoutbuffer = stdoutbuffer.append(tbuffer, length);
        }
      else if (pipe == itksysProcess_Pipe_STDERR)
        {
        stderrbuffer = stderrbuffer.append(tbuffer, length);
        }
      }
    }
  itksysProcess_WaitForExit(process, 0);
  
  // check the exit state / error state of the process
  int result = itksysProcess_GetState(process);
  if (result == itksysProcess_State_Exited)
    {
    // executable exited cleanly and must of done
    // "something" when presented with a "--xml" argument
    // (note that it may have just printed out that it did
    // not understand --xml)
    if (itksysProcess_GetExitValue(process) == 0)
      {
      // executable exited without errors, check if it
      // generated a valid xml description
      if (stdoutbuffer.compare(0, 4, "LOGO") == 0)
        {
        std::string prefix;
        int width;
        int height;
        int pixelSize;
        unsigned long bufferLength;

        // make a string stream of the buffer
        std::stringstream buffer;
        buffer << stdoutbuffer << std::ends;

        // read the header
        buffer >> prefix;
        buffer >> width;
        buffer >> height;
        buffer >> pixelSize;
        buffer >> bufferLength;

        // read the newline after the options
        char whitespace[10];
        buffer.getline(whitespace, 10);

        // now read the data for the icon
        //
        // read twice what we think we need because KWWidgets has a
        // bug in the buffer length
        std::string logo(bufferLength + 1, 0);
        buffer.read(const_cast<char*>(logo.c_str()), bufferLength);

        // make a ModuleLogo and configure it
        ModuleLogo mLogo;
        mLogo.SetLogo( (unsigned char *)logo.c_str(), width, height, pixelSize, bufferLength, 0);

        // set the log on the module
        module.SetLogo(mLogo);
        }
      else
        {
        // no logo response
        }
      }
    else
      {
      // exited with errors
      }
    }
  else if (result == itksysProcess_State_Expired)
    {
    // timeout
    }
  else
    {
    // did not exit cleanly
    }
  
  // clean up
  itksysProcess_Delete(process);
}

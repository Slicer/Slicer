#include "itksys/DynamicLoader.hxx" 
#include "itksys/Directory.hxx"
#include "itksys/SystemTools.hxx"
#include "itksys/Process.h"

#include "ModuleFactory.h"
#include "ModuleDescriptionParser.h"
#include "ModuleDescription.h"

#include <map>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <deque>

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



// Private implementaton of an std::map
class ModuleDescriptionMap : public std::map<std::string, ModuleDescription> {};


// ---

ModuleFactory::ModuleFactory()
{
  this->InternalMap = new ModuleDescriptionMap;
  this->WarningMessageCallback = 0;
  this->ErrorMessageCallback = 0;
  this->InformationMessageCallback = 0;
  this->ModuleDiscoveryMessageCallback = 0;
}

ModuleFactory::~ModuleFactory()
{
  delete this->InternalMap;
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
  // performance faster than command line module
  int numberOfShared, numberOfExecutables;

  numberOfShared = this->ScanForSharedObjectModules();
  numberOfExecutables = this->ScanForCommandLineModules();
  
  if (numberOfShared + numberOfExecutables == 0)
    {
    this->WarningMessage( "No plugin modules found. Check your module search path and your Slicer installation." );
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
          
          itksys::DynamicLoader::LibraryHandle lib
            = itksys::DynamicLoader::OpenLibrary(fullLibraryPath.c_str());
          if ( lib )
            {
            // Look for the symbol to get an XML description of the
            // module and a symbol to execute the module
            XMLModuleDescriptionFunction xmlFunction
              = (XMLModuleDescriptionFunction)itksys::DynamicLoader::GetSymbolAddress(lib, "GetXMLModuleDescription");

            ModuleEntryPoint entryPoint
              = (ModuleEntryPoint)itksys::DynamicLoader::GetSymbolAddress(lib, "SlicerModuleEntryPoint");


            // if the symbols are found, then get the XML descriptions
            // and cache the entry point to run the module
            if ( xmlFunction && entryPoint )
              {
              std::string xml = (*xmlFunction)();

              // check if the module generated a valid xml description
              if (xml.compare(0, 5, "<?xml") == 0)
                {
                // Construct and configure the module object
                ModuleDescription module;

                // Set the target as the entry point to call
                char entryPointAsText[256];
                sprintf(entryPointAsText, "slicer:%p", entryPoint);
                module.SetTarget( entryPointAsText );

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
                              << "\" has been discovered at " << module.GetTarget() << std::endl;
                  numberFound++;
                  }
                else
                  {
                  information << "A module named \"" << module.GetTitle()
                            << "\" has already been discovered." << std::endl
                            << "    First discovered at "
                            << (*mit).second.GetTarget() << std::endl
                            << "    Then discovered at "
                            << module.GetTarget() << std::endl
                            << "    Keeping first module." << std::endl;
                  }
                }
              }
            else
              {
//               std::cout << "Symbols not found." << std::endl;
//               std::cout << "xmlFunction: " << (void*)xmlFunction << std::endl;
//               std::cout << "entryPoint: " << (void*)entryPoint << std::endl;
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
              << numberFound << " valid plugins in " << t1 - t0
              << " seconds." << std::endl;
  
  this->InformationMessage( information.str().c_str() );

  return numberFound;
}

long
ModuleFactory
::ScanForCommandLineModules()
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
                << " for command line executable plugins." << std::endl;
    
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
        
          char *command[3];
          itksysProcess *process = itksysProcess_New();

          // fullcommand name and the argument to probe the executable
          std::string commandName = std::string(directory.GetPath())
            + "/" + filename;
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
              
                // Construct and configure the module object
                ModuleDescription module;
                module.SetTarget( commandName );

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
                  // Store the module in the list
                  (*this->InternalMap)[module.GetTitle()] =  module ;

                  information << "A module named \"" << module.GetTitle()
                              << "\" has been discovered at " << module.GetTarget() << std::endl;
                  numberFound++;
                  }
                else
                  {
                  information << "A module named \"" << module.GetTitle()
                              << "\" has already been discovered." << std::endl
                              << "    First discovered at "
                              << (*mit).second.GetTarget() << std::endl
                              << "    Then discovered at "
                              << module.GetTarget() << std::endl
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
  information << "Tested " << numberTested << " files as command line executable plugins. Found "
              << numberFound << " valid plugins in " << t1 - t0
              << " seconds." << std::endl;
  this->InformationMessage( information.str().c_str() );

  return numberFound;
}

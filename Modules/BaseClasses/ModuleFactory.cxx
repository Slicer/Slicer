#include "itkDynamicLoader.h"  // include itk files first 

#include "ModuleFactory.h"

#include "ModuleDescriptionParser.h"
#include "ModuleDescription.h"

#include "itksys/Directory.hxx"
#include "itksys/SystemTools.hxx"
#include "itksys/Process.h"

#include <map>

void
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
  std::string sname = name;
  if ( sname.find(itk::DynamicLoader::LibExtension()) != std::string::npos )
    {
    return true;
    }
  return false;
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
}

ModuleFactory::~ModuleFactory()
{
  delete this->InternalMap;
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
  this->ScanForSharedObjectModules();
  this->ScanForCommandLineModules();
}

void
ModuleFactory
::ScanForSharedObjectModules()
{
  // add any of the self-describing shared object modules available
  //
  // self-describing shared object modules live in a prescribed path
  // and have a prescribed symbol.
  if (this->SearchPath == "")
    {
    std::cout << "Empty module search path." << std::endl;
    return;
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
  
  for (pit = modulePaths.begin(); pit != modulePaths.end(); ++pit)
    {
    std::cout << "Searching " << *pit << std::endl;
    
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
          std::cout << "Checking " << fullLibraryPath << std::endl;
          
          itk::LibHandle lib
            = itk::DynamicLoader::OpenLibrary(fullLibraryPath.c_str());
          if ( lib )
            {
            // Look for the symbol to get an XML description of the
            // module and a symbol to execute the module
            XMLModuleDescriptionFunction xmlFunction
              = (XMLModuleDescriptionFunction)itk::DynamicLoader::GetSymbolAddress(lib, "GetXMLModuleDescription");

            ModuleEntryPoint entryPoint
              = (ModuleEntryPoint)itk::DynamicLoader::GetSymbolAddress(lib, "SlicerModuleEntryPoint");


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
                char entryPointAsText[16];
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
                
                if (mit == this->InternalMap->end())
                  {
                  // Store the module in the list
                  (*this->InternalMap)[module.GetTitle()] =  module ;
                  }
                else
                  {
                  std::cout << "  A module named \"" << module.GetTitle()
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
              std::cout << "Symbols not found." << std::endl;
              std::cout << "xmlFunction: " << (void*)xmlFunction << std::endl;
              std::cout << "entryPoint: " << (void*)entryPoint << std::endl;
              }
            }
          }
        }
      }
    }

  std::cout << "Tested " << numberTested << " files as shared object plugins. Found "
            << this->InternalMap->size() << " valid plugins." << std::endl;
}

void
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
    std::cout << "Empty module search path." << std::endl;
    return;
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
  
  for (pit = modulePaths.begin(); pit != modulePaths.end(); ++pit)
    {
    std::cout << "Searching " << *pit << std::endl;
    
    itksys::Directory directory;
    directory.Load( (*pit).c_str() );

    for ( unsigned int ii=0; ii < directory.GetNumberOfFiles(); ++ii)
      {
      const char *filename = directory.GetFile(ii);
      
      // skip any directories
      if (!itksys::SystemTools::FileIsDirectory(filename))
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

              if (mit == this->InternalMap->end())
                {
                // Store the module in the list
                (*this->InternalMap)[module.GetTitle()] =  module ;
                }
              else
                {
                std::cout << "  A module named \"" << module.GetTitle()
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

  std::cout << "Tested " << numberTested << " files as plugins. Found "
            << this->InternalMap->size() << " valid plugins." << std::endl;
}

#include "ModuleFactory.h"

#include "ModuleDescriptionParser.h"
#include "CommandLineModuleDescription.h"

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
  this->ScanForSharedObjectModules();
  this->ScanForCommandLineModules();
}

void
ModuleFactory
::ScanForSharedObjectModules()
{
  // todo
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
      numberTested++;
      
      // skip any directories
      if (!itksys::SystemTools::FileIsDirectory(filename))
        {
        std::cout << "Testing " << filename << " as a plugin:" << std::endl;
        
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
              std::cout << "\t" << filename << " is a plugin." << std::endl;
              
              // Construct and configure the module object
              CommandLineModuleDescription module;
              module.SetCommandString( filename );

              // Parse the xml to build the description of the module
              // and the parameters
              ModuleDescriptionParser parser;
              parser.Parse(stdoutbuffer, module);
            
              // Store the module in the list
              (*this->InternalMap)[module.GetTitle()] =  module ;
              }
            else
              {
              std::cout << "\t" << filename << " is not a plugin." << std::endl
                        << "\t" << filename << " did not generate an xml description." << std::endl;
              }
            }
          else
            {
            std::cout << "\t" << filename << " is not a plugin." << std::endl
                      << "\t" << filename << " exited with errors." << std::endl;
            }
          }
        else
          {
          std::cout << "\t" << filename << " is not a plugin." << std::endl
                    << "\t" << filename << " did not exit cleanly." << std::endl;
          }

        // clean up
        itksysProcess_Delete(process);
        }
      }
    }

  std::cout << "Tested " << numberTested << " files as plugins. Found "
            << this->InternalMap->size() << " valid plugins." << std::endl;
}

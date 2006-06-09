#include "ModuleFactory.h"
#include "CommandLineModuleDescription.h"

#include "vtksys/Directory.hxx"
#include "vtksys/SystemTools.hxx"
#include "vtksys/Process.h"

#include "vtk_expat.h"

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
  // should pull the directory or path from some KWWidget resource
  vtksys::Directory directory;
  directory.Load("c:/projects/Slicer3-net2003/Plugins");

  for ( unsigned int ii=0; ii < directory.GetNumberOfFiles(); ++ii)
    {
    const char *filename = directory.GetFile(ii);
    // skip any directories
    if (!vtksys::SystemTools::FileIsDirectory(filename))
      {
      char *command[3];
      vtksysProcess *process = vtksysProcess_New();

      // fullcommand name and the argument to probe the executable
      std::string commandName = std::string(directory.GetPath())
        + "/" + filename;
      std::string arg("--xml");

      // build the command/parameter array.
      command[0] = const_cast<char*>(commandName.c_str());
      command[1] = const_cast<char*>(arg.c_str());
      command[2] = 0;

      // setup the command
      vtksysProcess_SetCommand(process, command);
      vtksysProcess_SetOption(process,
                              vtksysProcess_Option_Detach, 0);
      vtksysProcess_SetOption(process,
                              vtksysProcess_Option_HideWindow, 1);

      // execute the command
      vtksysProcess_Execute(process);

      // Wait for the command to finish
      char *tbuffer;
      int length;
      int pipe;
      std::string stdoutbuffer;
      std::string stderrbuffer;
      while ((pipe = vtksysProcess_WaitForData(process ,&tbuffer,
                                               &length, 0)) != 0)
        {
        if (length != 0 && tbuffer != 0)
          {
          if (pipe == vtksysProcess_Pipe_STDOUT)
            {
            stdoutbuffer = stdoutbuffer.append(tbuffer, length);
            }
          else if (pipe == vtksysProcess_Pipe_STDERR)
            {
            stderrbuffer = stderrbuffer.append(tbuffer, length);
            }
          }
        }
      vtksysProcess_WaitForExit(process, 0);

      // check the exit state / error state of the process
      int result = vtksysProcess_GetState(process);
      if (result == vtksysProcess_State_Exited)
        {
        // executable exited cleanly and must of done
        // "something" when presented with a "--xml" argument
        // (note that it may have just printed out that it did
        // not understand --xml)
        if (vtksysProcess_GetExitValue(process) == 0)
          {
          // executable exited without errors, check if it
          // generated a valid xml description
          if (stdoutbuffer.compare(0, 5, "<?xml") == 0)
            {
            // Construct and configure the module object
            CommandLineModuleDescription module;
            module.SetCommandString( filename );

            // Parse the xml to build the description of the module
            // and the parameters

            
            // Fill in rest of module with results of XML parsing

            
            // for now use the filename without an extension as the
            // module, later we'll use the name of the module
            // specified in the xml description
            std::string moduleName
              = vtksys::SystemTools::GetFilenameWithoutExtension(std::string(filename));

            // Store the module in the list
            this->Modules[moduleName] =  module ;
            }
          }
        }
      }
    }
}

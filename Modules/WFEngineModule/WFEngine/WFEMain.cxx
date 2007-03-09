#include <iostream>
#include <fstream>

#include <string>
#include <tclap/CmdLine.h>

#include "WFBaseEngine.h"

using namespace std;
using namespace TCLAP;

namespace WFEngine
{
 
int my_main(int argc, char *argv[])
{
 //Initialize tcl
// Tcl_Interp *interp = vtkKWApplication::InitializeTcl(1, argv, &cerr);
//
// if (!interp)
// {
//   cerr << "Error: InitializeTcl failed" << endl ;
//   return 1;
// }
 
 //parse Commandline Arguments
 
// char* wfConfigFile;
// vtksys::CommandLineArguments args;
// args.Initialize(argc, argv);
// args.AddArgument(
//   "--config", vtksys::CommandLineArguments::SPACE_ARGUMENT, &wfConfigFile, "");
// args.Parse();
 
 string wfConfigFile;
 bool showEditGUI;
 
 //Wrap everything in a try block.  Do this every time,
 // because exceptions will be thrown for problems. 
 try {  

   // Define the command line object.
   CmdLine cmd("Command description message", ' ', "0.9");

   // Define a value argument and add it to the command line.
   ValueArg<string> configArg("c","config","Configuration file to be used",false,"","string");
   cmd.add( configArg );
   
   //Define a switch and add it to the command line.
   SwitchArg showEditGUISwitch("e","editor","show Configuration Editor", false);
   cmd.add( showEditGUISwitch );
   
   // Parse the args.
   cmd.parse( argc, argv );

   // Get the value parsed by each arg. 
   wfConfigFile = configArg.getValue();
   
   showEditGUI = showEditGUISwitch.getValue();
   
   cout << "My config file is: " << wfConfigFile << endl;

 } catch (ArgException &e)  // catch any exceptions
 { cerr << "error: " << e.error() << " for arg " << e.argId() << endl; }
 
 WFBaseEngine *wfeBE = WFBaseEngine::New();
 wfeBE->InitializeWFEngine(wfConfigFile);
 wfeBE->RunNetworkInterface();
 
 //Initialize die Engine
 
 //Initialize den xmlManager
 
 //Initialize das WFInterfaces
 
 
 return 0;
}

}

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <windows.h>
int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int)
{
  int argc;
  char **argv;
  vtksys::SystemTools::ConvertWindowsCommandLineToUnixArguments(
    lpCmdLine, &argc, &argv);
  int ret = my_main(argc, argv);
  for (int i = 0; i < argc; i++) { delete [] argv[i]; }
  delete [] argv;
  return ret;
}
#else
int main(int argc, char *argv[])
{
  return WFEngine::my_main(argc, argv);
}
#endif

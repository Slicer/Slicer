
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerGUI.h"

#include <vtksys/SystemTools.hxx>

extern "C" int Slicer3lib_Init(Tcl_Interp *interp);
int Slicer3_main(int argc, char *argv[])
{
  // Initialize Tcl

  Tcl_Interp *interp = vtkKWApplication::InitializeTcl(argc, argv, &cerr);
  if (!interp)
    {
    cerr << "Error: InitializeTcl failed" << endl ;
    return 1;
    }

  // Initialize our Tcl library (i.e. our classes wrapped in Tcl)

  Slicer3lib_Init(interp);

  // Create the application Logic object, 
  // then create the GUI object and have it observe the Logic
  // then let the application gui run

  vtkSlicerApplicationLogic *appLogic = vtkSlicerApplicationLogic::New();

  vtkSlicerApplicationGUI *appGUI = vtkSlicerApplicationGUI::New();
  
  appGUI->SetLogic(appLogic);

  int res = appGUI->Run(argc, argv);

  appGUI->Delete();
  appLogic->Delete();

  return res;
}

#ifdef _WIN32
#include <windows.h>
int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int)
{
  int argc;
  char **argv;
  vtksys::SystemTools::ConvertWindowsCommandLineToUnixArguments(
    lpCmdLine, &argc, &argv);
  int ret = Slicer3_main(argc, argv);
  for (int i = 0; i < argc; i++) { delete [] argv[i]; }
  delete [] argv;
  return ret;
}
#else
int main(int argc, char *argv[])
{
  return Slicer3_main(argc, argv);
}
#endif

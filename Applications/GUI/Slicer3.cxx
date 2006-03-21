
#include "vtkKWApplication.h"
#include "vtkSlicerGUI.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerApplicationGUI.h"

#include <vtksys/SystemTools.hxx>

extern "C" int Slicerbasegui_Init(Tcl_Interp *interp);
extern "C" int Mrml_Init(Tcl_Interp *interp);

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

  Slicerbasegui_Init(interp);
  Mrml_Init(interp);

  
  // Create SlicerGUI application, style, and main window 
  vtkSlicerGUI *slicerGUI = vtkSlicerGUI::New ( );

  // Create the application Logic object, 
  // Create the application GUI object
  // and have it observe the Logic
  vtkSlicerApplicationLogic *appLogic = vtkSlicerApplicationLogic::New( );
  vtkSlicerApplicationGUI *appGUI = vtkSlicerApplicationGUI::New ( );
  appGUI->SetApplication ( slicerGUI );
  appGUI->SetLogic ( appLogic );
  
  appGUI->SetParent ( slicerGUI->GetMainSlicerWin( )->GetViewFrame ( ));
  appGUI->BuildGUI ( );
  appGUI->AddGUIObservers ( );
  appGUI->AddLogicObservers ( );
  // after everything is packed 
  slicerGUI->DisplayMainSlicerWindow ( );

  // add to collection of component GUIs
  slicerGUI->AddGUI ( appGUI );
  
  // TODO: where should args get parsed?
  //int res = appGUI->StartApplication(argc, argv);
  int res = slicerGUI->StartApplication();

  appGUI->Delete();
  appLogic->Delete();
  slicerGUI->Delete();
  
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

int main(int argc, char *argv[])
{
  return Slicer3_main(argc, argv);
}

#else
int main(int argc, char *argv[])
{
  return Slicer3_main(argc, argv);
}
#endif

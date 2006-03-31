
#include "vtkRenderWindow.h"

#include "vtkKWApplication.h"
#include "vtkKWNotebook.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkMRMLScene.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerSliceGUI.h"

#include <vtksys/SystemTools.hxx>

extern "C" int Slicerbasegui_Init(Tcl_Interp *interp);
extern "C" int Slicerbaselogic_Init(Tcl_Interp *interp);
extern "C" int Mrml_Init(Tcl_Interp *interp);
extern "C" int Vtkitk_Init(Tcl_Interp *interp);

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
  Slicerbaselogic_Init(interp);
  Mrml_Init(interp);
  Vtkitk_Init(interp);

  
  // Create SlicerGUI application, style, and main window 
  vtkSlicerApplication *slicerApp = vtkSlicerApplication::New ( );
  slicerApp->GetSlicerStyle()->ApplyPresentation ( );

  // Create the application Logic object, 
  // Create the application GUI object
  // and have it observe the Logic
  vtkSlicerApplicationLogic *appLogic = vtkSlicerApplicationLogic::New( );
  vtkSlicerApplicationGUI *appGUI = vtkSlicerApplicationGUI::New ( );
  appGUI->SetApplication ( slicerApp );
  appGUI->SetLogic ( appLogic );
  appGUI->SetMrml ( appGUI->GetLogic()->GetMRMLScene( ) );
  appGUI->BuildGUI ( );
  appGUI->AddGUIObservers ( );
  appGUI->AddLogicObservers ( );
  appGUI->AddMrmlObservers ( );

  vtkSlicerSliceGUI *sliceGUI = vtkSlicerSliceGUI::New ();
  // ---
  // SLICE GUI
  sliceGUI = vtkSlicerSliceGUI::New ( );
  sliceGUI->SetMrml (appGUI->GetMrml ( ) );
  sliceGUI->SetApplication ( slicerApp);
  sliceGUI->BuildGUI ( appGUI->GetDefaultSlice0Frame(),
                       appGUI->GetDefaultSlice1Frame(),
                       appGUI->GetDefaultSlice2Frame() );

  // set slice viewer size
  sliceGUI->GetMainSlice0()->GetRenderWidget()->GetRenderWindow()->SetSize( appGUI->GetDefaultSliceGUIFrameWidth(), appGUI->GetDefaultSliceGUIFrameWidth());
  
  sliceGUI->GetMainSlice1()->GetRenderWidget()->GetRenderWindow()->SetSize( appGUI->GetDefaultSliceGUIFrameWidth(), appGUI->GetDefaultSliceGUIFrameWidth());
  
  sliceGUI->GetMainSlice2()->GetRenderWidget()->GetRenderWindow()->SetSize( appGUI->GetDefaultSliceGUIFrameWidth(), appGUI->GetDefaultSliceGUIFrameWidth());
  sliceGUI->AddGUIObservers();
  sliceGUI->AddLogicObservers();
  sliceGUI->AddMrmlObservers();
  
  appGUI->DisplayMainSlicerWindow ( );

  // add to collection of component GUIs
  slicerApp->AddGUI ( appGUI );


  const char *name = slicerApp->GetTclName();

  // TODO: where should args get parsed?
  //int res = appGUI->StartApplication(argc, argv);
  int res = slicerApp->StartApplication();

  appGUI->Delete();
  appLogic->Delete();
  slicerApp->Delete();
  
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

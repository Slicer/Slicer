
#include "vtkRenderWindow.h"

#include "vtkKWApplication.h"
#include "vtkKWNotebook.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkMRMLScene.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkKWUserInterfacePanel.h"
#include "vtkKWUserInterfaceManager.h"
#include "vtkKWUserInterfaceManagerNotebook.h"
#include "vtkSlicerGUICollection.h"
#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerDataGUI.h"


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
    appGUI->BuildGUI ( );
    appGUI->AddGUIObservers ( );
    appGUI->AddLogicObservers ( );


    vtkSlicerSliceGUI *sliceGUI = vtkSlicerSliceGUI::New ();
    // ---
    // SLICE GUI
    sliceGUI->SetApplication ( slicerApp);
    sliceGUI->BuildGUI ( appGUI->GetDefaultSlice0Frame(),
                         appGUI->GetDefaultSlice1Frame(),
                         appGUI->GetDefaultSlice2Frame() );
    sliceGUI->AddGUIObservers();
    sliceGUI->AddLogicObservers();

    // ---
    // Note on vtkSlicerApplication's ModuleGUICollection:
    // right now the vtkSlicerApplication's ModuleGUICollection
    // collects ONLY module GUIs which populate the shared
    // ui panel in the main user interface. Other GUIs, like
    // the vtkSlicerSliceGUI which manages the slice widgets
    // the vtkSlicerApplicationGUI which manages the entire
    // main user interface and viewer, any future GUI that
    // creates toplevel widgets are not added to this collection.
    // If we need to collect them at some point, we should define 
    // other collections in the vtkSlicerApplication class.

    // ---
    // MODULE GUIs:
    // for now list them here but later autodetect;
    // Also add code to handle the raising of their
    // GUI panel in vtkSlicerApplicationGUI.cxx

    // Create the volumes module GUI.
    vtkSlicerVolumesGUI *VolumesGUI = vtkSlicerVolumesGUI::New ( );
    VolumesGUI->SetApplication ( slicerApp );
    VolumesGUI->SetGUIName( "VolumesGUI" );
    VolumesGUI->GetUIPanel()->SetName ("VolumesGUI");
    VolumesGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWin()->GetMainUserInterfaceManager ( ) );
    VolumesGUI->GetUIPanel()->Create ( );
    VolumesGUI->BuildGUI ( );
    VolumesGUI->AddGUIObservers ( );
    VolumesGUI->AddLogicObservers ( );
    slicerApp->AddModuleGUI ( VolumesGUI );
    
    // Create the models module GUI.
    vtkSlicerModelsGUI *ModelsGUI = vtkSlicerModelsGUI::New ( );
    ModelsGUI->SetApplication ( slicerApp );
    ModelsGUI->SetGUIName( "ModelsGUI" );
    ModelsGUI->GetUIPanel()->SetName ("ModelsGUI");
    ModelsGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWin()->GetMainUserInterfaceManager ( ) );
    ModelsGUI->GetUIPanel()->Create ( );
    ModelsGUI->BuildGUI ( );
    ModelsGUI->AddGUIObservers ( );
    ModelsGUI->AddLogicObservers ( );
    slicerApp->AddModuleGUI ( ModelsGUI );


    // Create the data module GUI.
    vtkSlicerDataGUI *DataGUI = vtkSlicerDataGUI::New ( );
    DataGUI->SetApplication ( slicerApp );
    DataGUI->SetGUIName( "DataGUI" );
    DataGUI->GetUIPanel()->SetName ("DataGUI");
    DataGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWin()->GetMainUserInterfaceManager ( ) );
    DataGUI->GetUIPanel()->Create ( );
    DataGUI->BuildGUI ( );
    DataGUI->AddGUIObservers ( );
    DataGUI->AddLogicObservers ( );
    slicerApp->AddModuleGUI ( DataGUI );
    
    // Additional Modules GUI panel configuration.
    vtkKWUserInterfaceManagerNotebook *mnb = vtkKWUserInterfaceManagerNotebook::SafeDownCast (appGUI->GetMainSlicerWin()->GetMainUserInterfaceManager());
    mnb->GetNotebook()->AlwaysShowTabsOff();
    mnb->GetNotebook()->ShowOnlyPagesWithSameTagOn();    

    const char *name = slicerApp->GetTclName();

    // DISPLAY WINDOW AND RUN
    appGUI->DisplayMainSlicerWindow ( );
    int res = slicerApp->StartApplication();

    // REMOVE OBSERVERS
    VolumesGUI->RemoveGUIObservers ( );
    VolumesGUI->RemoveLogicObservers ( );
    ModelsGUI->RemoveGUIObservers ( );
    ModelsGUI->RemoveLogicObservers ( );
    DataGUI->RemoveGUIObservers ( );
    DataGUI->RemoveLogicObservers ( );
    sliceGUI->RemoveGUIObservers();
    sliceGUI->RemoveLogicObservers();
  
    // REMOVE ALL COLLECTED GUI OBJECTS OR DELETE WON'T WORK
    sliceGUI->GetSliceWidgets()->RemoveAllItems(); 
    slicerApp->GetModuleGUICollection ( )->RemoveAllItems ( );

    // EXIT THE APPLICATION
    slicerApp->Exit();

    // DELETE ALL GUI OBJECTS IN GOOD ORDER
    VolumesGUI->Delete ( );
    ModelsGUI->Delete ( );
    DataGUI->Delete ( );
    sliceGUI->Delete ();
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

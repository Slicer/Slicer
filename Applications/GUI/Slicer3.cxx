
#include "vtkRenderWindow.h"

#include "vtkKWApplication.h"
#include "vtkKWNotebook.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerSliceLogic.h"
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
    appGUI->SetApplicationLogic ( appLogic );
    appGUI->BuildGUI ( );
    appGUI->AddGUIObservers ( );


    vtkSlicerSliceGUI *sliceGUI = vtkSlicerSliceGUI::New ();
    // assume something is creating a slicelogic for each of the
    // three default slice widgets in the sliceGUI...
    vtkSlicerSliceLogic *sliceLogic0 = vtkSlicerSliceLogic::New ( );
    vtkSlicerSliceLogic *sliceLogic1 = vtkSlicerSliceLogic::New ( );
    vtkSlicerSliceLogic *sliceLogic2 = vtkSlicerSliceLogic::New ( );
    // ---
    // SLICE GUI
    sliceGUI->SetApplication ( slicerApp);
    // creates 3 default slice widgets
    sliceGUI->BuildGUI ( appGUI->GetDefaultSlice0Frame(),
                         appGUI->GetDefaultSlice1Frame(),
                         appGUI->GetDefaultSlice2Frame() );
    sliceGUI->AddGUIObservers();
    sliceGUI->SetApplicationLogic ( appLogic );
    // Set mrml scene pointer for the slice GUI
    // and add observers on mrml.
    sliceGUI->SetMRMLScene (  appLogic->GetMRMLScene( ) );
    // Set logic pointers for three slice widgets (0,1,2)
    // in the GUI, and add observers on that logic.
    sliceGUI->SetSliceLogic ( sliceLogic0, 0 );
    sliceGUI->SetSliceLogic ( sliceLogic1, 1 );
    sliceGUI->SetSliceLogic ( sliceLogic2, 2 );

    
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
    //    vtkSlicerVolumesLogic *volumesLogic = vtkSlicerVolumesLogic::New ( );
    VolumesGUI->SetApplication ( slicerApp );
    VolumesGUI->SetGUIName( "VolumesGUI" );
    VolumesGUI->GetUIPanel()->SetName ("VolumesGUI");
    VolumesGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWin()->GetMainUserInterfaceManager ( ) );
    VolumesGUI->GetUIPanel()->Create ( );
    VolumesGUI->BuildGUI ( );
    VolumesGUI->AddGUIObservers ( );
    VolumesGUI->SetApplicationLogic ( appLogic );
    //    VolumesGUI->SetModuleLogic ( volumesLogic );
    slicerApp->AddModuleGUI ( VolumesGUI );
    
    // Create the models module GUI.
    vtkSlicerModelsGUI *ModelsGUI = vtkSlicerModelsGUI::New ( );
    //    vtkSlicerModelsLogic *modelsLogic = vtkSlicerModelsLogic::New ( );
    ModelsGUI->SetApplication ( slicerApp );
    ModelsGUI->SetGUIName( "ModelsGUI" );
    ModelsGUI->GetUIPanel()->SetName ("ModelsGUI");
    ModelsGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWin()->GetMainUserInterfaceManager ( ) );
    ModelsGUI->GetUIPanel()->Create ( );
    ModelsGUI->BuildGUI ( );
    ModelsGUI->AddGUIObservers ( );
    ModelsGUI->SetApplicationLogic ( appLogic );
    //    ModelsGUI->SetModuleLogic ( modelsLogic );
    slicerApp->AddModuleGUI ( ModelsGUI );


    // Create the data module GUI.
    vtkSlicerDataGUI *DataGUI = vtkSlicerDataGUI::New ( );
    //    vtkSlicerDataLogic *dataLogic = vtkSlicerDataLogic::New ( );
    DataGUI->SetApplication ( slicerApp );
    DataGUI->SetGUIName( "DataGUI" );
    DataGUI->GetUIPanel()->SetName ("DataGUI");
    DataGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWin()->GetMainUserInterfaceManager ( ) );
    DataGUI->GetUIPanel()->Create ( );
    DataGUI->BuildGUI ( );
    DataGUI->AddGUIObservers ( );
    DataGUI->SetApplicationLogic ( appLogic );
    //    DataGUI->SetModuleLogic ( dataLogic );
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
    VolumesGUI->RemoveApplicationLogicObservers ( );
    VolumesGUI->RemoveLogicObservers ( );
    VolumesGUI->RemoveMRMLObservers ( );
    ModelsGUI->RemoveGUIObservers ( );
    ModelsGUI->RemoveApplicationLogicObservers ( );
    ModelsGUI->RemoveLogicObservers ( );
    ModelsGUI->RemoveMRMLObservers ( );
    DataGUI->RemoveGUIObservers ( );
    DataGUI->RemoveApplicationLogicObservers ( );
    DataGUI->RemoveLogicObservers ( );
    DataGUI->RemoveMRMLObservers ( );

    sliceGUI->RemoveGUIObservers ( );
    sliceGUI->RemoveApplicationLogicObservers();
    sliceGUI->RemoveLogicObservers ( );
    sliceGUI->RemoveMRMLObservers ( );

    
    // REMOVE ALL COLLECTED GUI OBJECTS OR DELETE WON'T WORK
    //    sliceGUI->GetSliceWidgetCollection()->RemoveAllItems();
    //    sliceGUI->GetSliceLogicCollection()->RemoveAllItems();

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


#include "vtkRenderWindow.h"

#include "vtkKWApplication.h"
#include "vtkKWNotebook.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkSlicerVolumesLogic.h"
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

#include "vtkGradientAnisotropicDiffusionFilterLogic.h"
#include "vtkGradientAnisotropicDiffusionFilterGUI.h"

#include <vtksys/SystemTools.hxx>

extern "C" int Slicerbasegui_Init(Tcl_Interp *interp);
extern "C" int Slicerbaselogic_Init(Tcl_Interp *interp);
extern "C" int Mrml_Init(Tcl_Interp *interp);
extern "C" int Vtkitk_Init(Tcl_Interp *interp);
//TODO remove temporary
extern "C" int Gradientanisotropicdiffusionfilter_Init(Tcl_Interp *interp);

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
    //TODO remove temporary
    Gradientanisotropicdiffusionfilter_Init(interp);

    // Create SlicerGUI application, style, and main window 
    vtkSlicerApplication *slicerApp = vtkSlicerApplication::New ( );
    slicerApp->GetSlicerStyle()->ApplyPresentation ( );
 
    // Create MRML scene
    vtkMRMLScene *scene = vtkMRMLScene::New();

    // Create the application Logic object, 
    // Create the application GUI object
    // and have it observe the Logic
    vtkSlicerApplicationLogic *appLogic = vtkSlicerApplicationLogic::New( );
    appLogic->SetMRMLScene(scene);
    vtkSlicerApplicationGUI *appGUI = vtkSlicerApplicationGUI::New ( );
    appGUI->SetApplication ( slicerApp );
    appGUI->SetApplicationLogic ( appLogic );

    // CREATE MODULE LOGICS & GUIS; add to GUI collection
    // (presumably these will be auto-detected, not listed out as below...)
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
    vtkSlicerVolumesGUI *volumesGUI = vtkSlicerVolumesGUI::New ( );
    vtkSlicerVolumesLogic *volumesLogic = vtkSlicerVolumesLogic::New ( );
    volumesLogic->SetMRMLScene(scene);
    volumesGUI->SetApplication ( slicerApp );
    volumesGUI->SetApplicationLogic ( appLogic );
    volumesGUI->SetLogic(volumesLogic);
    volumesGUI->SetGUIName( "Volumes" );
    volumesGUI->GetUIPanel()->SetName ( volumesGUI->GetGUIName ( ) );
    volumesGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWin()->GetMainUserInterfaceManager ( ) );
    volumesGUI->GetUIPanel()->Create ( );
    slicerApp->AddModuleGUI ( volumesGUI );
    
    vtkSlicerModelsGUI *modelsGUI = vtkSlicerModelsGUI::New ( );

    //vtkSlicerModelsLogic *modelsLogic = vtkSlicerModelsLogic::New ( );
    modelsGUI->SetApplication ( slicerApp );
    modelsGUI->SetApplicationLogic ( appLogic );
    modelsGUI->SetGUIName( "Models" );
    modelsGUI->GetUIPanel()->SetName ( modelsGUI->GetGUIName ( ) );
    modelsGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWin()->GetMainUserInterfaceManager ( ) );
    modelsGUI->GetUIPanel()->Create ( );
    slicerApp->AddModuleGUI ( modelsGUI );
    //---
    vtkSlicerDataGUI *dataGUI = vtkSlicerDataGUI::New ( );
    //vtkSlicerDataLogic *dataLogic = vtkSlicerDataLogic::New ( );
    dataGUI->SetApplication ( slicerApp );
    dataGUI->SetApplicationLogic ( appLogic );
    dataGUI->SetGUIName( "Data" );
    dataGUI->GetUIPanel()->SetName ( dataGUI->GetGUIName ( ) );
    dataGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWin()->GetMainUserInterfaceManager ( ) );
    dataGUI->GetUIPanel()->Create ( );    
    slicerApp->AddModuleGUI ( dataGUI );
    // ---
    vtkGradientAnisotropicDiffusionFilterGUI *gradientAnisotropicDiffusionFilterGUI = vtkGradientAnisotropicDiffusionFilterGUI::New ( );
    vtkGradientAnisotropicDiffusionFilterLogic *gradientAnisotropicDiffusionFilterLogic  = vtkGradientAnisotropicDiffusionFilterLogic::New ( );
    gradientAnisotropicDiffusionFilterLogic->SetMRMLScene(scene);
    gradientAnisotropicDiffusionFilterGUI->SetLogic ( gradientAnisotropicDiffusionFilterLogic );
    gradientAnisotropicDiffusionFilterGUI->SetApplication ( slicerApp );
    gradientAnisotropicDiffusionFilterGUI->SetApplicationLogic ( appLogic );
    gradientAnisotropicDiffusionFilterGUI->SetGUIName( "GradientAnisotropicDiffusionFilter" );
    gradientAnisotropicDiffusionFilterGUI->GetUIPanel()->SetName ( gradientAnisotropicDiffusionFilterGUI->GetGUIName ( ) );
    gradientAnisotropicDiffusionFilterGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWin()->GetMainUserInterfaceManager ( ) );
    gradientAnisotropicDiffusionFilterGUI->GetUIPanel()->Create ( );
    slicerApp->AddModuleGUI ( gradientAnisotropicDiffusionFilterGUI );
    // ---    
    vtkSlicerSliceGUI *sliceGUI = vtkSlicerSliceGUI::New ();
    vtkSlicerSliceLogic *sliceLogic0 = vtkSlicerSliceLogic::New ( );
    vtkSlicerSliceLogic *sliceLogic1 = vtkSlicerSliceLogic::New ( );
    vtkSlicerSliceLogic *sliceLogic2 = vtkSlicerSliceLogic::New ( );
    sliceGUI->SetApplication ( slicerApp);
    sliceGUI->SetApplicationLogic ( appLogic );
    sliceGUI->SetGUIName( "Slices" );
    sliceGUI->GetUIPanel()->SetName ( sliceGUI->GetGUIName ( ) );
    sliceGUI->GetUIPanel()->SetUserInterfaceManager ( appGUI->GetMainSlicerWin( )->GetMainUserInterfaceManager( ) );
    sliceGUI->GetUIPanel( )->Create( );
    slicerApp->AddModuleGUI ( sliceGUI );


    // BUILD APPLICATION GUI (this requires collection of module GUIs)
    appGUI->BuildGUI ( );
    appGUI->AddGUIObservers ( );


    // ---
    // BUILD MODULE GUIs (these require appGUI to be built):
    volumesGUI->BuildGUI ( );
    volumesGUI->AddGUIObservers ( );
    // ---
    modelsGUI->BuildGUI ( );
    modelsGUI->AddGUIObservers ( );
    // ---
    dataGUI->BuildGUI ( );
    dataGUI->AddGUIObservers ( );
    // ---
    gradientAnisotropicDiffusionFilterGUI->BuildGUI ( );
    gradientAnisotropicDiffusionFilterGUI->AddGUIObservers ( );
    // ---
    sliceGUI->BuildGUI ( appGUI->GetDefaultSlice0Frame(),
                         appGUI->GetDefaultSlice1Frame(),
                         appGUI->GetDefaultSlice2Frame() );
    sliceGUI->AddGUIObservers();
    sliceGUI->SetMRMLScene (  appLogic->GetMRMLScene( ) );
    sliceGUI->SetSliceLogic ( sliceLogic0, 0 );
    sliceGUI->SetSliceLogic ( sliceLogic1, 1 );
    sliceGUI->SetSliceLogic ( sliceLogic2, 2 );
    
    
    // Additional Modules GUI panel configuration.
    vtkKWUserInterfaceManagerNotebook *mnb = vtkKWUserInterfaceManagerNotebook::SafeDownCast (appGUI->GetMainSlicerWin()->GetMainUserInterfaceManager());
    mnb->GetNotebook()->AlwaysShowTabsOff();
    mnb->GetNotebook()->ShowOnlyPagesWithSameTagOn();    

    const char *name = slicerApp->GetTclName();

    // DISPLAY WINDOW AND RUN
    appGUI->DisplayMainSlicerWindow ( );
    int res = slicerApp->StartApplication();

    // REMOVE OBSERVERS
    //---
    volumesGUI->RemoveGUIObservers ( );
    volumesGUI->RemoveApplicationLogicObservers ( );
    volumesGUI->RemoveLogicObservers ( );
    volumesGUI->RemoveMRMLObservers ( );
    //---
    modelsGUI->RemoveGUIObservers ( );
    modelsGUI->RemoveApplicationLogicObservers ( );
    modelsGUI->RemoveLogicObservers ( );
    modelsGUI->RemoveMRMLObservers ( );
    //---
    dataGUI->RemoveGUIObservers ( );
    dataGUI->RemoveApplicationLogicObservers ( );
    dataGUI->RemoveLogicObservers ( );
    dataGUI->RemoveMRMLObservers ( );
    //---
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
    volumesGUI->Delete ( );
    modelsGUI->Delete ( );
    dataGUI->Delete ( );
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

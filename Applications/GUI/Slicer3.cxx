
#include "vtkRenderWindow.h"

#include "vtkKWApplication.h"
#include "vtkKWNotebook.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkSlicerVolumesLogic.h"
#include "vtkMRMLScene.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerSlicesGUI.h"
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

//TODO added temporary
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
    //TODO added temporary
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
    appLogic->SetAndObserveMRMLScene ( scene );

    vtkSlicerApplicationGUI *appGUI = vtkSlicerApplicationGUI::New ( );
    appGUI->SetApplication ( slicerApp );
    appGUI->SetAndObserveApplicationLogic ( appLogic );


    // ------------------------------
    // CREATE MODULE LOGICS & GUIS; add to GUI collection
    // (presumably these will be auto-detected, not listed out as below...)
    // ---
    // Note on vtkSlicerApplication's ModuleGUICollection:
    // right now the vtkSlicerApplication's ModuleGUICollection
    // collects ONLY module GUIs which populate the shared
    // ui panel in the main user interface. Other GUIs, like
    // the vtkSlicerSliceGUI which contains one SliceWidget
    // the vtkSlicerApplicationGUI which manages the entire
    // main user interface and viewer, any future GUI that
    // creates toplevel widgets are not added to this collection.
    // If we need to collect them at some point, we should define 
    // other collections in the vtkSlicerApplication class.

    // ADD INDIVIDUAL MODULES
    // --- Volumes module
    vtkSlicerVolumesLogic *volumesLogic = vtkSlicerVolumesLogic::New ( );
    volumesLogic->SetAndObserveMRMLScene ( scene );
    vtkSlicerVolumesGUI *volumesGUI = vtkSlicerVolumesGUI::New ( );
    volumesGUI->SetApplication ( slicerApp );
    volumesGUI->SetAndObserveApplicationLogic ( appLogic );
    volumesGUI->SetAndObserveMRMLScene ( scene );
    volumesGUI->SetModuleLogic ( volumesLogic );
    volumesGUI->SetGUIName( "Volumes" );
    volumesGUI->GetUIPanel()->SetName ( volumesGUI->GetGUIName ( ) );
    volumesGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWin()->GetMainUserInterfaceManager ( ) );
    volumesGUI->GetUIPanel()->Create ( );
    slicerApp->AddModuleGUI ( volumesGUI );

    // --- Models module    
    //vtkSlicerModelLogic *modelLogic = vtkSlicerModelLogic::New ( );
    //modelLogic->SetAndObserveMRMLScene ( scene );
    vtkSlicerModelsGUI *modelsGUI = vtkSlicerModelsGUI::New ( );
    modelsGUI->SetApplication ( slicerApp );
    modelsGUI->SetAndObserveApplicationLogic ( appLogic );
    modelsGUI->SetAndObserveMRMLScene ( scene );
    //modelsGUI->SetModuleLogic ( modelLogic );
    modelsGUI->SetGUIName( "Models" );
    modelsGUI->GetUIPanel()->SetName ( modelsGUI->GetGUIName ( ) );
    modelsGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWin()->GetMainUserInterfaceManager ( ) );
    modelsGUI->GetUIPanel()->Create ( );
    slicerApp->AddModuleGUI ( modelsGUI );

    //--- Data module
    //vtkSlicerDataLogic *dataLogic = vtkSlicerDataLogic::New ( );
    //dataLogic->SetAndObserveMRMLScene ( scene );
    vtkSlicerDataGUI *dataGUI = vtkSlicerDataGUI::New ( );
    dataGUI->SetApplication ( slicerApp );
    dataGUI->SetAndObserveApplicationLogic ( appLogic );
    dataGUI->SetAndObserveMRMLScene ( scene );
    //dataGUI->SetModuleLogic ( dataLogic );
    dataGUI->SetGUIName( "Data" );
    dataGUI->GetUIPanel()->SetName ( dataGUI->GetGUIName ( ) );
    dataGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWin()->GetMainUserInterfaceManager ( ) );
    dataGUI->GetUIPanel()->Create ( );    
    slicerApp->AddModuleGUI ( dataGUI );

    // --- Slices module
    vtkSlicerSliceLogic *sliceLogic0 = vtkSlicerSliceLogic::New ( );
    vtkSlicerSliceLogic *sliceLogic1 = vtkSlicerSliceLogic::New ( );
    vtkSlicerSliceLogic *sliceLogic2 = vtkSlicerSliceLogic::New ( );
    sliceLogic0->SetAndObserveMRMLScene ( scene );
    sliceLogic0->ProcessMRMLEvents ();
    sliceLogic0->ProcessLogicEvents ();
    sliceLogic1->SetAndObserveMRMLScene ( scene );
    sliceLogic1->ProcessMRMLEvents ();
    sliceLogic1->ProcessLogicEvents ();
    sliceLogic2->SetAndObserveMRMLScene ( scene );
    sliceLogic2->ProcessMRMLEvents ();
    sliceLogic2->ProcessLogicEvents ();
    vtkSlicerSlicesGUI *slicesGUI = vtkSlicerSlicesGUI::New ();
    slicesGUI->SetApplication ( slicerApp );
    slicesGUI->SetAndObserveApplicationLogic ( appLogic );
    slicesGUI->SetAndObserveMRMLScene ( scene );
    slicesGUI->SetGUIName( "Slices" );
    slicesGUI->GetUIPanel()->SetName ( slicesGUI->GetGUIName ( ) );
    slicesGUI->GetUIPanel()->SetUserInterfaceManager ( appGUI->GetMainSlicerWin( )->GetMainUserInterfaceManager( ) );
    slicesGUI->GetUIPanel( )->Create( );
    slicerApp->AddModuleGUI ( slicesGUI );
    
    // --- Gradient anisotropic diffusion filter module
    
    vtkGradientAnisotropicDiffusionFilterGUI *gradientAnisotropicDiffusionFilterGUI = vtkGradientAnisotropicDiffusionFilterGUI::New ( );
    vtkGradientAnisotropicDiffusionFilterLogic *gradientAnisotropicDiffusionFilterLogic  = vtkGradientAnisotropicDiffusionFilterLogic::New ( );
    gradientAnisotropicDiffusionFilterLogic->SetAndObserveMRMLScene ( scene );
    //    gradientAnisotropicDiffusionFilterLogic->SetMRMLScene(scene);
    gradientAnisotropicDiffusionFilterGUI->SetLogic ( gradientAnisotropicDiffusionFilterLogic );
    gradientAnisotropicDiffusionFilterGUI->SetApplication ( slicerApp );
    gradientAnisotropicDiffusionFilterGUI->SetApplicationLogic ( appLogic );
    gradientAnisotropicDiffusionFilterGUI->SetGUIName( "GradientAnisotropicDiffusionFilter" );
    gradientAnisotropicDiffusionFilterGUI->GetUIPanel()->SetName ( gradientAnisotropicDiffusionFilterGUI->GetGUIName ( ) );
    gradientAnisotropicDiffusionFilterGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWin()->GetMainUserInterfaceManager ( ) );
    gradientAnisotropicDiffusionFilterGUI->GetUIPanel()->Create ( );
    slicerApp->AddModuleGUI ( gradientAnisotropicDiffusionFilterGUI );

    
    // ------------------------------
    // BUILD APPLICATION GUI
    // (this requires collection of module GUIs)
    appGUI->BuildGUI ( );
    appGUI->AddGUIObservers ( );

    // ------------------------------
    // BUILD MODULE GUIs
    // (these require appGUI to be built):
    volumesGUI->BuildGUI ( );
    volumesGUI->AddGUIObservers ( );

    modelsGUI->BuildGUI ( );
    modelsGUI->AddGUIObservers ( );

    dataGUI->BuildGUI ( );
    dataGUI->AddGUIObservers ( );

    slicesGUI->BuildGUI ( appGUI->GetDefaultSlice0Frame(),
                         appGUI->GetDefaultSlice1Frame(),
                         appGUI->GetDefaultSlice2Frame() );
    slicesGUI->AddGUIObservers();
    slicesGUI->SetAndObserveModuleLogic ( 0, sliceLogic0);
    slicesGUI->SetAndObserveModuleLogic ( 1, sliceLogic1);
    slicesGUI->SetAndObserveModuleLogic ( 2, sliceLogic2);

    // ---
    gradientAnisotropicDiffusionFilterGUI->BuildGUI ( );
    gradientAnisotropicDiffusionFilterGUI->AddGUIObservers ( );
    
    // ------------------------------
    // CONFIGURE SlICER'S SHARED GUI PANEL
    // Additional Modules GUI panel configuration.
    vtkKWUserInterfaceManagerNotebook *mnb = vtkKWUserInterfaceManagerNotebook::SafeDownCast (appGUI->GetMainSlicerWin()->GetMainUserInterfaceManager());
    mnb->GetNotebook()->AlwaysShowTabsOff();
    mnb->GetNotebook()->ShowOnlyPagesWithSameTagOn();    

    //
    // get the Tcl name so the vtk class will be registered in the interpreter as a byproduct
    // - set some handy variables so it will be easy to access these classes from
    //   the tkcon
    // - all the variables are put in the slicer3 tcl namespace for easy access
    //
    const char *name;
    name = slicerApp->GetTclName();
    slicerApp->Script ("namespace eval slicer3 set SlicerApplication %s", name);
    name = appGUI->GetTclName();
    slicerApp->Script ("namespace eval slicer3 set SlicerApplicationGUI %s", name);
    name = slicesGUI->GetTclName();
    slicerApp->Script ("namespace eval slicer3 set SlicerSlicesGUI %s", name);

    slicerApp->Script ("namespace eval slicer3 set MRMLScene [$::slicer3::SlicerApplication GetMRMLScene]");

    // TODO
    //name = sliceLogic0->GetTclName();
    //slicerApp->Script ("namespace eval slicer3 set SliceLogic0 %s", name);
    //name = sliceLogic1->GetTclName();
    //slicerApp->Script ("namespace eval slicer3 set SliceLogic1 %s", name);
    //name = sliceLogic2->GetTclName();
    //slicerApp->Script ("namespace eval slicer3 set SliceLogic2 %s", name);


    // ------------------------------
    // DISPLAY WINDOW AND RUN
    appGUI->DisplayMainSlicerWindow ( );
    int res = slicerApp->StartApplication();

    // ------------------------------
    // REMOVE OBSERVERS
    gradientAnisotropicDiffusionFilterGUI->RemoveGUIObservers ( );
    gradientAnisotropicDiffusionFilterGUI->SetMRMLScene ( NULL );    
    gradientAnisotropicDiffusionFilterGUI->SetApplicationLogic ( NULL );
    //gradientAnisotropicDiffusionFilterGUI->SetMRMLNode ( NULL );
    //gradientAnisotropicDiffusionFilterGUI->SetModuleLogic ( NULL );

    volumesGUI->RemoveGUIObservers ( );
    volumesGUI->SetModuleLogic ( NULL );
    volumesGUI->SetApplicationLogic ( NULL );
    volumesGUI->SetMRMLNode ( NULL );
    volumesGUI->SetMRMLScene ( NULL );

    modelsGUI->RemoveGUIObservers ( );
    //modelsGUI->SetModuleLogic ( NULL );
    modelsGUI->SetApplicationLogic ( NULL );
    //modelsGUI->SetMRMLNode ( NULL );
    modelsGUI->SetMRMLScene ( NULL );

    dataGUI->RemoveGUIObservers ( );
    //dataGUI->SetModuleLogic ( NULL );
    dataGUI->SetApplicationLogic ( NULL );
    //dataGUI->SetMRMLNode ( NULL );
    dataGUI->SetMRMLScene ( NULL );

    slicesGUI->RemoveGUIObservers ( );
    slicesGUI->SetModuleLogic ( 0, NULL);
    slicesGUI->SetModuleLogic ( 1, NULL);
    slicesGUI->SetModuleLogic ( 2, NULL);
    slicesGUI->SetMRMLScene ( NULL );
    slicesGUI->SetApplicationLogic ( NULL );

    // ------------------------------
    // REMOVE ALL COLLECTED GUI OBJECTS OR DELETE WON'T WORK
    slicesGUI->GetSliceGUICollection()->RemoveAllItems();
    slicerApp->GetModuleGUICollection ( )->RemoveAllItems ( );

    // ------------------------------
    // EXIT THE APPLICATION
    slicerApp->Exit();

    // ------------------------------
    // DELETE 
    //--- scene
    scene->Delete ();

    //--- logic
    appLogic->Delete ();
    gradientAnisotropicDiffusionFilterLogic->Delete ();
    volumesLogic->Delete();
    sliceLogic0->Delete ();
    sliceLogic1->Delete ();
    sliceLogic2->Delete ();
    
    //--- gui
    gradientAnisotropicDiffusionFilterGUI->Delete ();
    volumesGUI->Delete ();
    modelsGUI->Delete ();
    dataGUI->Delete ();
    slicesGUI->Delete ();
    appGUI->Delete ();
    
    slicerApp->Delete ();

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

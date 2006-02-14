#include "vtkKWApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerMainDesktopGUI.h"
#include "vtkSlicerLogic.h"
#include "vtkSlicerApplicationLogic.h"

#include <vtksys/SystemTools.hxx>
#include <vtksys/CommandLineArguments.hxx>

// If I change the name of this function and library
// in the CMakeLists.txt file, the linking throws an
// undefined symbol error. So, changing the library
// name to KWSlicer3Lib, and the function to
// Kwslicer3lib_Init won't work. What am i missing?
extern "C" int Slicerbasegui_Init (Tcl_Interp *interp );

int slicer3_main( int argc, char *argv[] )
{
    int ret;
    // Try including this tcl interpreter in the
    // in the vtkSlicerApplicationGUI class.
    Tcl_Interp *interp = vtkKWApplication::InitializeTcl (argc, argv, &cerr);
    if (!interp ) {
        cerr << "Error: InitializeTcl failed" << endl ;
        return 1;
    }
    // Initialize our classses wrapped in Tcl.
    // Tcl requires a call to wrap them.
    Slicerbasegui_Init (interp);
    
    vtkSlicerApplicationGUI *app;
    vtkSlicerMainDesktopGUI *gui;
    vtkSlicerApplicationLogic *logic;

    // normally we'd not allocate here, but that
    // we'd point to already instantiated object.
    logic = vtkSlicerApplicationLogic::New ( );

    app = vtkSlicerApplicationGUI::New ( );
    app->ConfigureApplication ( );

    gui = vtkSlicerMainDesktopGUI::New ( );
    gui->SetLogic ( logic );
    gui->BuildGUI ( app );
    gui->AddGUIObservers ( );
    gui->AddLogicObservers ( );

    ret = app->StartApplication ( );

    logic->Delete ( );
    gui->Delete ( );
    // something causes unhappiness in app->Delete
    app->Delete ( );
    return ret;    
}

#ifdef _WIN32
#include "windows.h"
int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int)
{
    int argc;
    char **argv;
    vtksys::SystemTools::ConvertWindowsCommandLineToUnixArguments( lpCmdLine, &argc, &argv );
    int ret = slicer3_main ( argc, argv );
    for (int i = 0; i < argc; i++) { delete [] argv[i]; }
    delete [] argv;
    return ret;
}
#else
int main(int argc, char *argv[])
{
    return slicer3_main ( argc, argv );
}
#endif

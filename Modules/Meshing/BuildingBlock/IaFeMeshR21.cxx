/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: IaFeMeshR21.cxx,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.16.4.1 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <vtksys/SystemTools.hxx>
#include <vtksys/CommandLineArguments.hxx>

//Mimx files
#include "vtkKWMimxApplication.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWSplashScreen.h"

//#include "vtkKWMimxMenuWindow.h"
//#include "vtkKWMimxViewWindow.h"

extern "C" int Iafemeshr21lib_Init(Tcl_Interp *interp);

int my_main(int argc, char *argv[])
{
    // Initialize Tcl

    Tcl_Interp *interp = vtkKWApplication::InitializeTcl(argc, argv, &cerr);
    if (!interp)
    {
        cerr << "Error: InitializeTcl failed" << endl ;
        return 1;
    }

    // Process some command-line arguments
    // The --test option here is used to run this example as a non-interactive 
    // test for software quality purposes. You can ignore it.
    Iafemeshr21lib_Init(interp);
    int option_test = 0;
    vtksys::CommandLineArguments args;
    args.Initialize(argc, argv);
    args.AddArgument(
        "--test", vtksys::CommandLineArguments::NO_ARGUMENT, &option_test, "");
    args.Parse();

    // Create the application
    // If --test was provided, ignore all registry settings, and exit silently
    // Restore the settings that have been saved to the registry, like
    // the geometry of the user interface so far.

    vtkKWMimxApplication *app = vtkKWMimxApplication::New();
    app->SetName("IA-FEMesh");
    if (option_test)
    {
        app->SetRegistryLevel(0);
        app->PromptBeforeExitOff();
    }
    
    /* Setup the Splash Screen */
    app->RestoreApplicationSettingsFromRegistry();
  app->SupportSplashScreenOn();
  app->SplashScreenVisibilityOn();
  
  std::string exePath;
  std::string errorMsg;
  vtksys::SystemTools::FindProgramPath(argv[0], exePath, errorMsg);
  std::string exeDir = vtksys::SystemTools::GetProgramPath( exePath.c_str() );           
  char res_path[2048];
  sprintf(res_path, 
            "%s/IA-FEMesh_splash.png",
            exeDir.c_str() );
  //std::cout << res_path << std::endl;
  app->GetSplashScreen()->ReadImage(res_path);
  app->GetSplashScreen()->SetProgressMessage("Starting IA-FEMesh ...");

    
  // May be used to set the Application Icon
  //app->Script("wm iconbitmap . @/tmp/mimx.xbm");
  
    // Set a help link. Can be a remote link (URL), or a local file

    // vtksys::SystemTools::GetFilenamePath(__FILE__) + "/help.html";
    app->SetHelpDialogStartingPage("http://www.ccad.uiowa.edu/mimx");

    // Add a window
    // Set 'SupportHelp' to automatically add a menu entry for the help link

    vtkKWMimxMainWindow *mainwin = vtkKWMimxMainWindow::New();
    mainwin->SetTitle("IA-FEMesh");
    app->AddWindow(mainwin);
    mainwin->SupportHelpOn();
    mainwin->SecondaryPanelVisibilityOff();
    mainwin->InstallDefaultTheme( );
    mainwin->LoadRegistryApplicationSettings();
    mainwin->Create();
    mainwin->Display();
    
    int ret;
    if (!option_test)
    {
        app->Start(argc, argv);
        ret = app->GetExitStatus();
    }

    mainwin->Close();
    mainwin->Delete();

    app->Delete();

    return ret;
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
    return my_main(argc, argv);
}
#endif


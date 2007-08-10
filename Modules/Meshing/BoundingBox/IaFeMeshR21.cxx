#include <vtksys/SystemTools.hxx>
#include <vtksys/CommandLineArguments.hxx>

//Mimx files
#include "vtkKWMimxApplication.h"
#include "vtkKWMimxMenuWindow.h"
#include "vtkKWMimxViewWindow.h"

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

  vtkKWApplication *app = vtkKWApplication::New();
  app->SetName("Iafemeshr21");
  if (option_test)
  {
    app->SetRegistryLevel(0);
    app->PromptBeforeExitOff();
  }
  app->RestoreApplicationSettingsFromRegistry();

  // Set a help link. Can be a remote link (URL), or a local file

  // vtksys::SystemTools::GetFilenamePath(__FILE__) + "/help.html";
  app->SetHelpDialogStartingPage("http://www.kwwidgets.org");

  // Add a window
  // Set 'SupportHelp' to automatically add a menu entry for the help link

  vtkKWMimxViewWindow *viewwin = vtkKWMimxViewWindow::New();
  vtkKWMimxMenuWindow *menuwin = vtkKWMimxMenuWindow::New();
  menuwin->SetMimxViewWindow(viewwin);
  app->AddWindow(viewwin);
  app->AddWindow(menuwin);
  viewwin->Create();
  menuwin->Create();

  menuwin->Display();
  viewwin->Display();
  int ret;
  if (!option_test)
  {
    app->Start(argc, argv);
    ret = app->GetExitStatus();
  }

  viewwin->Close();
  menuwin->Close();
  viewwin->Delete();
  menuwin->Delete();

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


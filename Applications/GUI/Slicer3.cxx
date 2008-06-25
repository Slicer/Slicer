#include "vtkOpenGLRenderWindow.h"
#include "vtkRenderWindow.h"

#include "vtkKWApplication.h"
#include "vtkKWWindow.h"
#include "vtkKWNotebook.h"
#include "vtkKWRegistryHelper.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWSplashScreen.h"
#include "vtkSlicerLogoIcons.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkSlicerVolumesLogic.h"
#include "vtkSlicerModelsLogic.h"
#include "vtkSlicerModelHierarchyLogic.h"
#include "vtkSlicerFiducialsLogic.h"
#include "vtkSlicerColorLogic.h"
#include "vtkMRMLScene.h"
#include "vtkCacheManager.h"
#include "vtkDataIOManager.h"
#include "vtkDataIOManagerLogic.h"
#include "vtkSlicerCacheAndDataIOManagerGUI.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerSlicesGUI.h"
#include "vtkKWUserInterfacePanel.h"
#include "vtkKWUserInterfaceManager.h"
#include "vtkKWUserInterfaceManagerNotebook.h"
#include "vtkKWLogDialog.h"
#include "vtkSlicerGUICollection.h"
#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerFiducialsGUI.h"
#include "vtkSlicerColorGUI.h"
#include "vtkSlicerDataGUI.h"
#include "vtkSlicerTransformsGUI.h"
#include "vtkSlicerCamerasGUI.h"
#include "vtkSlicerTheme.h"
#include "vtkSlicerWindow.h"
#include "vtkSlicerApplicationSettingsInterface.h"


#include "vtkSlicerConfigure.h" /* Slicer3_USE_* */

#include "ModuleFactory.h"

#include "vtkSlicerROILogic.h"
#include "vtkSlicerROIGUI.h"

#ifdef Slicer3_USE_PYTHON
// If debug, Python wants pythonxx_d.lib, so fake it out
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

extern "C" {
  void init_mytkinter( Tcl_Interp* );
  void init_slicer(void );
}
#include "vtkTclUtil.h"

#endif

#include <vtksys/SystemTools.hxx>
#include <vtksys/Directory.hxx>
#include <vtksys/stl/string>

// for data prov
#include <time.h>
#include <stdlib.h>

#include <vtksys/stl/vector>

#include "Resources/vtkSlicerSplashScreen_ImageData.h"

// uncomment these lines to disable a particular module (handy for debugging)
//#define CLIMODULES_DEBUG
//#define COLORS_DEBUG
//#define COMMANDLINE_DEBUG
//#define DAEMON_DEBUG
//#define FIDUCIALS_DEBUG
//#define MODELS_DEBUG
//#define REMOTEIO_DEBUG
//#define SLICES_DEBUG
//#define TCLMODULES_DEBUG
//#define TRACTOGRAPHY_DEBUG
//#define VOLUMES_DEBUG

#define CAMERA_DEBUG

#include <LoadableModuleFactory.h>

#if !defined(TRACTOGRAPHY_DEBUG) && defined(Slicer3_BUILD_MODULES)
#include "vtkSlicerFiberBundleLogic.h"
#include "vtkSlicerTractographyDisplayLogic.h"
#include "vtkSlicerTractographyDisplayGUI.h"
#include "vtkSlicerTractographyFiducialSeedingGUI.h"
#endif

#if !defined(SCRIPTEDMODULE_DEBUG) && defined(Slicer3_BUILD_MODULES)
#include "vtkScriptedModuleLogic.h"
#include "vtkScriptedModuleGUI.h"
#endif

#if !defined(COMMANDLINE_DEBUG) && (defined(Slicer3_BUILD_CLI) || defined(Slicer3_BUILD_MODULES))
#include "vtkCommandLineModuleLogic.h"
#include "vtkCommandLineModuleGUI.h"
#endif

#if !defined(VOLUMES_DEBUG) && defined(Slicer3_BUILD_MODULES)
#include "vtkSlicerVolumesGUI.h"
#endif

#if !defined(REMOTEIO_DEBUG)
#include "vtkHTTPHandler.h"
#include "vtkSRBHandler.h"
#include "vtkXNATHandler.h"
#include "vtkSlicerPermissionPrompterWidget.h"
#include "vtkSlicerXNATPermissionPrompterWidget.h"
#endif

//
// note: always write to cout rather than cerr so log messages will
// appear in the correct order when running tests.  Normally cerr preempts cout
// when output is buffered going to a terminal, but in the case of the launcher
// cerr is collected in a separate buffer and printed at the end.   Either
// way, it's better for all logging to go the the same channel so it shows
// up in the correct order.
//
#ifdef _WIN32
#  define slicerCerr(x)                                                 \
  do {                                                                  \
  cout << x;                                                            \
  vtkstd::ostringstream str;                                            \
  str << x;                                                             \
  MessageBox(NULL, str.str().c_str(), "Slicer Error", MB_OK);           \
  } while (0)
#else
#  define slicerCerr(x)                                                 \
  cout << x
#endif


// Get the automated arg parsing code
#include "Slicer3CLP.h"

extern "C" int Slicerbasegui_Init(Tcl_Interp *interp);
extern "C" int Slicerbaselogic_Init(Tcl_Interp *interp);
extern "C" int Mrml_Init(Tcl_Interp *interp);
extern "C" int Vtkitk_Init(Tcl_Interp *interp);
extern "C" int Freesurfer_Init(Tcl_Interp *interp);
extern "C" int Igt_Init(Tcl_Interp *interp);
extern "C" int Vtkteem_Init(Tcl_Interp *interp);


//TODO added temporary
#if !defined(TRACTOGRAPHY_DEBUG) && defined(Slicer3_BUILD_MODULES)
extern "C" int Slicertractographydisplay_Init(Tcl_Interp *interp);
extern "C" int Slicertractographyfiducialseeding_Init(Tcl_Interp *interp);
#endif
#if !defined(DAEMON_DEBUG) && defined(Slicer3_BUILD_MODULES)
extern "C" int Slicerdaemon_Init(Tcl_Interp *interp);
#endif
#if !defined(COMMANDLINE_DEBUG) && (defined(Slicer3_BUILD_CLI) || defined(Slicer3_BUILD_MODULES))
extern "C" int Commandlinemodule_Init(Tcl_Interp *interp);
#endif
#if !defined(SCRIPTEDMODULE_DEBUG) && defined(Slicer3_BUILD_MODULES)
extern "C" int Scriptedmodule_Init(Tcl_Interp *interp);
#endif
#if !defined(VOLUMES_DEBUG) && defined(Slicer3_BUILD_MODULES)
extern "C" int Volumes_Init(Tcl_Interp *interp);
#endif

struct SpacesToUnderscores
{
  char operator() (char in)
    {
      if (in == ' ' )
        {
        return '_';
        }

      return in;
    }
};


int Slicer3_Tcl_Eval ( Tcl_Interp *interp, const char *script )
{
  if ( Tcl_Eval (interp, script) != TCL_OK )
    {
    // TODO: need to figure out how to turn on the stdio channels
    // so puts will work from scripts in windows...
    slicerCerr("Error: " << Tcl_GetStringResult( interp ) << "\n" 
               << "while executing:\n" << script << "\n\n" 
               << Tcl_GetVar2( interp, "errorInfo", NULL, TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG ) );
    return 1;
    }
  return 0;
}

void Slicer3_BrokerScriptHandler ( const char *script )
{
  vtkSlicerApplication::GetInstance()->Script( script );
}

void printAllInfo(int argc, char **argv)
{
  int i;
  struct tm * timeInfo;
  time_t rawtime;
  // yyyy/mm/dd-hh-mm-ss-ms-TZ
  // plus one for 3 char time zone
  char timeStr[27];

  fprintf(stdout, "<ProcessStep>\n");
  fprintf(stdout, "<ProgramName version=\"$Revision$\">%s</ProgramName>\n", argv[0]);
  fprintf(stdout, "<ProgramArguments>");
  for (i = 1; i < argc; i++)
    {
    fprintf(stdout, " %s", argv[i]);
    }
  fprintf(stdout, "</ProgramArguments>\n");
  fprintf(stdout, "<CVS>$Id$</CVS> <TimeStamp>");
  time ( &rawtime );
  timeInfo = localtime (&rawtime);
  strftime (timeStr, 27, "%Y/%m/%d-%H-%M-%S-00-%Z", timeInfo);
  fprintf(stdout, "%s</TimeStamp>\n", timeStr);
  fprintf(stdout, "<User>%s</User>\n", getenv("USER"));
  
  fprintf(stdout, "<HostName>");
  if (getenv("HOSTNAME") == NULL)
    {
    if (getenv("HOST") == NULL)
      {
      fprintf(stdout, "(unknown)");
      }
    else
      {
      fprintf(stdout, "%s", getenv("HOST"));
      }
    }
  else
    {
    fprintf(stdout, "%s", getenv("HOSTNAME"));
    }
  fprintf(stdout, "</HostName><platform version=\"");
#if defined(sun) || defined(__sun)
#if defined(__SunOS_5_7)
  fprintf(stdout, "2.7");
#endif
#if defined(__SunOS_5_8)
  fprintf(stdout, "8");
#endif
#endif
#if defined(linux) || defined(__linux)
  fprintf(stdout, "unknown");
#endif
  
  fprintf(stdout, "\">");
  // now the platform name
#if defined(linux) || defined(__linux)
  fprintf(stdout, "Linux");
#endif 
#if defined(macintosh) || defined(Macintosh)
  fprintf(stdout, "MAC OS 9");
#endif
#ifdef __MACOSX__
  fprintf(stdout, "MAC OS X");
#endif
#if defined(sun) || defined(__sun)
# if defined(__SVR4) || defined(__svr4__)
  fprintf(stdout, "Solaris");
# else
  fprintf(stdout, "SunOS");
# endif
#endif
#if defined(_WIN32) || defined(__WIN32__)
  fprintf(stdout, "Windows");
#endif
  fprintf(stdout, "</platform>\n");

  if (getenv("MACHTYPE") != NULL)
    {
    fprintf(stdout, "<Architecture>%s</Architecture>\n", getenv("MACHTYPE"));
    }
  
  fprintf(stdout, "<Compiler version=\"");
#if defined(__GNUC__)
#if defined(__GNU_PATCHLEVEL__)
  fprintf(stdout, "%d", (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__));
#else
  fprintf(stdout, "%d", (__GNUC__ * 10000 + __GNUC_MINOR__ * 100));
#endif
#endif
#if defined(_MSC_VER)
  fprintf(stdout, "%d", (_MSC_VER));
#endif
  // now the compiler name
  fprintf(stdout, ">");
#if defined(__GNUC__)
  fprintf(stdout, "GCC");
#else
#if defined(_MSC_VER)
  fprintf(stdout, "MSC");
#else
  fprintf(stdout, "UKNOWN");
#endif
#endif
  fprintf(stdout, "</Compiler>\n");

  
  fprintf(stdout, "<library version=\"%s\">VTK</librarary><library version=\"unknown\">ITK</library><library version=\"%s\">KWWidgets</library>\n", VTK_VERSION, KWWidgets_VERSION);
  int major, minor, patchLevel;
  Tcl_GetVersion(&major, &minor, &patchLevel, NULL);
  fprintf(stdout, "<library version=\"%d.%d.%d\">TCL</library>\n", major, minor, patchLevel);
  //fprintf(stdout, "<library version=\"%d.%d.%d\">TK</library>\n", major,
  //minor, patchLevel);
#ifdef Slicer3_USE_PYTHON
  fprintf(stdout, "<library version=\"%s\">Python</library>\n", PY_VERSION);
#endif
  fprintf(stdout, "<Repository>$HeadURL$</Repository>\n");
  fprintf(stdout, "<ProcessStep>\n");
  fprintf(stdout, "\n");
}



static void WarningMessage(const char *msg)
{
  vtkSlicerApplication::GetInstance()->WarningMessage(msg);
}

static void ErrorMessage(const char *msg)
{
  vtkSlicerApplication::GetInstance()->ErrorMessage(msg);
}

static void InformationMessage(const char *msg)
{
  vtkSlicerApplication::GetInstance()->InformationMessage(msg);
}

static void DebugMessage(const char *msg)
{
  vtkSlicerApplication::GetInstance()->DebugMessage(msg);
}

static void SplashMessage(const char *msg)
{
  vtkSlicerApplication::GetInstance()->SplashMessage(msg);
}


int Slicer3_main(int argc, char *argv[])
{
#if WIN32
#define PathSep ";"
#else
#define PathSep ":"
#endif

  // Append the path to Slicer specific ITK factories. These are kept
  // in a separate directory from other libraries to speed the search
  // and launching of plugins. Also, set up the TCL_LIBRARY
  // environment variable.
  std::string itkAutoLoadPath;
  vtksys::SystemTools::GetEnv("ITK_AUTOLOAD_PATH", itkAutoLoadPath);

  std::string programPath;
  std::string errorMessage;
  if ( !vtksys::SystemTools::FindProgramPath(argv[0],
                                             programPath, errorMessage) )
    {
    slicerCerr("Error: Cannot find Slicer3 executable" << endl);
    return 1;
    }
  
  std::string slicerBinDir
    = vtksys::SystemTools::GetFilenamePath(programPath.c_str());
  bool hasIntDir = false;
  std::string intDir = "";
  
  std::string tmpName = slicerBinDir + "/../" + Slicer3_INSTALL_LIB_DIR + "/SlicerBaseGUI/Tcl/Loader.tcl";
  if ( !vtksys::SystemTools::FileExists(tmpName.c_str()) )
    {
    // Handle Visual Studio IntDir
    std::vector<std::string> pathComponents;
    vtksys::SystemTools::SplitPath(slicerBinDir.c_str(), pathComponents);

    slicerBinDir = slicerBinDir + "/..";
    tmpName = slicerBinDir + "/../" + Slicer3_INSTALL_LIB_DIR + "/SlicerBaseGUI/Tcl/Loader.tcl";
    if ( !vtksys::SystemTools::FileExists(tmpName.c_str()) )
      {
      slicerCerr("Error: Cannot find Slicer3 libraries" << endl);
      return 1;
      }

    if (pathComponents.size() > 0)
      {
      hasIntDir = true;
      intDir = pathComponents[pathComponents.size()-1];
      }
    }

  slicerBinDir = vtksys::SystemTools::CollapseFullPath(slicerBinDir.c_str());

  // set the Slicer3_HOME variable if it doesn't already exist from the launcher
  vtksys_stl::string slicerHome;
  if ( !vtksys::SystemTools::GetEnv("Slicer3_HOME", slicerHome) )
    {
    slicerHome = slicerBinDir + "/..";
    slicerHome = vtksys::SystemTools::CollapseFullPath(slicerHome.c_str());
    std::string homeEnv = "Slicer3_HOME=" + slicerHome;
    cout << "Set environment: " << homeEnv.c_str() << endl;
    vtkKWApplication::PutEnv(const_cast <char *> (homeEnv.c_str()));
    }

  std::string tclEnv = "TCL_LIBRARY=";
  tclEnv += slicerHome + "/lib/TclTk/lib/tcl" + Slicer3_TCL_TK_MAJOR_VERSION + "." + Slicer3_TCL_TK_MINOR_VERSION;
  vtkKWApplication::PutEnv(const_cast <char *> (tclEnv.c_str()));

  // ITK factories dir

  std::string slicerITKFactoriesDir;
  slicerITKFactoriesDir = slicerHome + "/" + Slicer3_INSTALL_ITKFACTORIES_DIR;
  if (hasIntDir)
    {
    slicerITKFactoriesDir += "/" + intDir;
    }
  if ( itkAutoLoadPath.size() == 0 )
    {
    itkAutoLoadPath = slicerITKFactoriesDir; 
    }
  else 
    {
    itkAutoLoadPath = slicerITKFactoriesDir + PathSep + itkAutoLoadPath; 
    }
  itkAutoLoadPath = "ITK_AUTOLOAD_PATH=" + itkAutoLoadPath;
  int putSuccess = 
    vtkKWApplication::PutEnv(const_cast <char *> (itkAutoLoadPath.c_str()));
  if (!putSuccess)
    {
    cout << "Unable to set ITK_AUTOLOAD_PATH. " << itkAutoLoadPath << endl;
    }

  
  // Initialize Tcl
  // -- create the interp
  // -- set up initial global variables
  // -- later in this function tcl scripts are executed 
  //    and some additional global variables are defined
  Tcl_Interp *interp = NULL;
  interp = vtkKWApplication::InitializeTcl(argc, argv, &cout);
  if (!interp)
    {
    slicerCerr("Error: InitializeTcl failed" << endl );
    return 1;
    }

  //
  // turn off hardware antialiasing by default
  // - the QueryAtlas picker relies on this
  //   
  vtkOpenGLRenderWindow::SetGlobalMaximumNumberOfMultiSamples(0);

#ifdef Slicer3_USE_PYTHON
  // Initialize Python
    
  // Set up the search path
  std::string pythonEnv = "PYTHONPATH=";
    
  const char* existingPythonEnv = vtksys::SystemTools::GetEnv("PYTHONPATH");
  if ( existingPythonEnv )
    {
    pythonEnv += std::string ( existingPythonEnv ) + PathSep;
    }

  pythonEnv += slicerHome + "/" + Slicer3_INSTALL_LIB_DIR + "/SlicerBaseGUI/Python" + PathSep;
  pythonEnv += slicerHome + "/" + Slicer3_INSTALL_PLUGINS_BIN_DIR + PathSep;
  vtkKWApplication::PutEnv(const_cast <char *> (pythonEnv.c_str()));
  
  Py_Initialize();
  PySys_SetArgv(argc, argv);
  PyObject* PythonModule = PyImport_AddModule("__main__");
  if (PythonModule == NULL)
    {
    std::cout << "Warning: Failed to initialize python" << std::endl;
    }
  PyObject* PythonDictionary = PyModule_GetDict(PythonModule);

  // Intercept _tkinter, and use ours...
  init_mytkinter(interp);
  init_slicer();
  PyObject* v;

  std::string TkinitString = "import Tkinter, sys;"
    "tk = Tkinter.Tk();"
    "sys.path.append ( \""
    + slicerHome + "/" + Slicer3_INSTALL_LIB_DIR + "/SlicerBaseGUI/Python"
    + "\" );\n"
    "sys.path.append ( \""
    + slicerHome + "/" + Slicer3_INSTALL_PLUGINS_BIN_DIR
    + "\" );\n";
  
  v = PyRun_String( TkinitString.c_str(),
                    Py_file_input,
                    PythonDictionary,
                    PythonDictionary );
  if (v == NULL)
    {
    PyErr_Print();
    }
  else
    {
    if (Py_FlushLine())
      {
      PyErr_Clear();
      }
    }
#endif

  // TODO: get rid of fixed size buffer
  char cmd[2048];

  // Make sure Slicer3_HOME is available

  sprintf(cmd, "set ::env(Slicer3_HOME) {%s};", slicerHome.c_str());
  Slicer3_Tcl_Eval(interp, cmd);
  
  sprintf(cmd, "set ::Slicer3_HOME {%s};", slicerHome.c_str());
  Slicer3_Tcl_Eval(interp, cmd);
  
  // Tell KWWidgets to make names like .vtkKWPushButton10 instead of .10 
  vtkKWWidget::UseClassNameInWidgetNameOn();

  //
  // Set a global variable so modules that use tcl can find the 
  // binary dir (where Slicer3.exe is) and the build dir (where tcl scripts are stored)
  //
  std::string config_types(VTKSLICER_CONFIGURATION_TYPES);
  std::string one_up;
  if (config_types.size())
    {
    one_up = "/..";
    }
  sprintf(cmd, "                                                   \
      set ::Slicer3_BIN [file dirname [info nameofexecutable]];       \
      if { $::tcl_platform(platform) == \"windows\"} {               \
        set ::Slicer3_BUILD [file normalize $Slicer3_BIN/..%s]         \
      } else {                                                       \
        set ::Slicer3_BUILD [file normalize $Slicer3_BIN/..]           \
      };                                                             \
    ", one_up.c_str());

  Slicer3_Tcl_Eval( interp, cmd);

  PARSE_ARGS;  // relies on the header file auto generated by Slicer3.xml

  if (VTKDebugOn)
    {
    vtkObject::GlobalWarningDisplayOn();
    }

  //
  // load itcl package (needed for interactive widgets)
  //
  {    
  std::string cmd;
  int returnCode;

  // Pass arguments to the Tcl script
  cmd =  "package require Itcl;";
  returnCode = Slicer3_Tcl_Eval( interp, cmd.c_str() );
  if ( returnCode )
    {
    slicerCerr("Error: slicer requires the Itcl package (" << cmd.c_str() << ")" << endl);
    return ( returnCode );
    }
  }

  //
  // load the gui tcl package (for the interactive widgets)
  //
  {    
  std::string cmd;
  int returnCode;

  // Pass arguments to the Tcl script
  cmd =  "lappend auto_path \"" + slicerHome + "/"
    Slicer3_INSTALL_LIB_DIR + "/SlicerBaseGUI/Tcl\"; ";
  //cmd += "puts $auto_path; ";
  cmd += "package require SlicerBaseGUITcl; ";
  returnCode = Slicer3_Tcl_Eval( interp, cmd.c_str() );
  if ( returnCode )
    {
    slicerCerr("Load SlicerBaseGUITcl: " << cmd.c_str() << endl);
    return ( returnCode );
    }
  }


  //
  // load the custom icons
  //
#ifdef _WIN32
  {    
  std::string cmd;
  int returnCode;

  cmd =  "wm iconbitmap . -default \""+ slicerHome + "/" + Slicer3_INSTALL_SHARE_DIR + "/slicer3.ico\"";
  returnCode = Slicer3_Tcl_Eval( interp, cmd.c_str() );
  if ( returnCode )
    {
    slicerCerr("Load Custom Icons: " << cmd.c_str() << endl);
    }
  }
#endif


  //
  // Initialize our Tcl library (i.e. our classes wrapped in Tcl)
  //
  Slicerbasegui_Init(interp);
  Slicerbaselogic_Init(interp);
  Mrml_Init(interp);
  Vtkitk_Init(interp);
  Freesurfer_Init(interp);
  Igt_Init(interp);
  Vtkteem_Init(interp);

#if !defined(TRACTOGRAPHY_DEBUG) && defined(Slicer3_BUILD_MODULES)
  Slicertractographydisplay_Init(interp);
  Slicertractographyfiducialseeding_Init(interp);
#endif
#if !defined(DAEMON_DEBUG) && defined(Slicer3_BUILD_MODULES)
  Slicerdaemon_Init(interp);
#endif
#if !defined(COMMANDLINE_DEBUG) && (defined(Slicer3_BUILD_CLI) || defined(Slicer3_BUILD_MODULES))
  Commandlinemodule_Init(interp);
#endif
#if !defined(SCRIPTEDMODULE_DEBUG) && defined(Slicer3_BUILD_MODULES)
  Scriptedmodule_Init(interp);
#endif
#if !defined(VOLUMES_DEBUG) && defined(Slicer3_BUILD_MODULES)
  Volumes_Init(interp);
#endif

  // first call to GetInstance will create the Application
  // 
  // make a substitute tcl proc for the 'exit' built in that
  // goes through the vtkKWApplication methodology for exiting 
  // cleanly
  //
  vtkSlicerApplication *slicerApp = vtkSlicerApplication::GetInstance ( );

  if (TestMode)
    {
    slicerApp->SetRegistryLevel(0);
    slicerApp->PromptBeforeExitOff();
    NoSplash = true;
    slicerApp->Script ("namespace eval slicer3 set TEST_MODE 1");
    }
  else
    {
    slicerApp->Script ("namespace eval slicer3 set TEST_MODE 0");
    }

  {
  std::string cmd, slicerAppName;

  slicerAppName = slicerApp->GetTclName();

  slicerApp->Script ("namespace eval slicer3 set Application %s", slicerAppName.c_str());

  cmd = "rename exit tcl_exit; ";

  cmd += 
    "proc exit {args} { \
      if { $args != {} && [string is integer $args] == \"1\" } { \
        " + slicerAppName + " SetExitStatus $args \
      } else { \
        " + slicerAppName + " SetExitStatus 0 \
      } ;\
      after idle {" + slicerAppName + " Exit}; \
    }";
  Slicer3_Tcl_Eval( interp, cmd.c_str() );
  }

  //
  // use the startup script passed on command line if it exists
  //
  if ( File != "" )
    {    

    std::string cmd;
    
    // Pass arguments to the Tcl script
    cmd = "set args \"\"; ";
    std::vector<std::string>::const_iterator argit = Args.begin();
    while (argit != Args.end())
      {
      cmd += " lappend args \"" + *argit + "\"; ";
      ++argit;
      }
    Slicer3_Tcl_Eval( interp, cmd.c_str() );

    cmd = "source " + File;
    int returnCode = Slicer3_Tcl_Eval( interp, cmd.c_str() );
    Slicer3_Tcl_Eval( interp, "update" );
    slicerApp->Delete();
    return ( returnCode );
    }
    
  //
  // use the startup code passed on command line if it exists
  //
  if ( Eval != "" )
    {    
    std::string cmd = "set ::SLICER(eval) \"" + Eval + "\" ; ";
    cmd += "regsub -all {\\.,} $::SLICER(eval) \";\" ::SLICER(eval); ";
    cmd += "regsub -all {,\\.} $::SLICER(eval) \";\" ::SLICER(eval); ";
    cmd += "eval $::SLICER(eval);";
    int returnCode = Slicer3_Tcl_Eval( interp, cmd.c_str() );
    Slicer3_Tcl_Eval( interp, "update" );
    slicerApp->Delete();
    return ( returnCode );
    }

  // Create SlicerGUI application, style, and main window 
  //  - note: use the singleton application 
  slicerApp->InstallTheme( slicerApp->GetSlicerTheme() );

  // copy the image from the header file into memory

  unsigned char *buffer = 
    new unsigned char [image_S3SplashScreen_length];

  unsigned int i;
  unsigned char *curPos = buffer;
  for (i = 0; i < image_S3SplashScreen_nb_sections; i++)
    {
    size_t len = strlen((const char*)image_S3SplashScreen_sections[i]);
    memcpy(curPos, image_S3SplashScreen_sections[i], len);
    curPos += len;
    }

  vtkKWTkUtilities::UpdatePhoto(
    slicerApp->GetMainInterp(),
    "S3SplashScreen", 
    buffer, 
    image_S3SplashScreen_width, 
    image_S3SplashScreen_height,
    image_S3SplashScreen_pixel_size,
    image_S3SplashScreen_length);
  delete [] buffer; 

  slicerApp->GetSplashScreen()->SetImageName("S3SplashScreen");
  if ( NoSplash )
    {
    slicerApp->SetUseSplashScreen(0);
    slicerApp->SupportSplashScreenOff();
    slicerApp->SplashScreenVisibilityOff();
    }
  else
    {
    slicerApp->SetUseSplashScreen(1);
    slicerApp->SupportSplashScreenOn();
    slicerApp->SplashScreenVisibilityOn();
    }
  slicerApp->GetSplashScreen()->SetProgressMessageVerticalOffset(-25);
  slicerApp->SplashMessage("Initializing Window...");

  cout << "Starting Slicer: " << slicerHome.c_str() << endl;

  if ( Stereo )
    {
    slicerApp->SetStereoEnabled(1);
    } 
  else 
    {
    slicerApp->SetStereoEnabled(0);
    }
    
  // -- event broker
  // - script handler to pass callback strings to the tcl interp
  // - synchronous mode so that redunant events do not get collapsed
  //   (this is compatible with standard vtk event behavior, but adds
  //   the ability to trace event execution).
  vtkEventBroker::GetInstance()->SetScriptHandler( Slicer3_BrokerScriptHandler ); 
  vtkEventBroker::GetInstance()->SetEventModeToSynchronous(); 


  // Create MRML scene
  vtkMRMLScene *scene = vtkMRMLScene::New();
  vtksys_stl::string root = vtksys::SystemTools::GetCurrentWorkingDirectory();
  scene->SetRootDirectory(root.c_str());
  vtkMRMLScene::SetActiveScene( scene );
    
  // Create the application Logic object, 
  // Create the application GUI object
  // and have it observe the Logic
  vtkSlicerApplicationLogic *appLogic = vtkSlicerApplicationLogic::New( );
  appLogic->SetMRMLScene ( scene );
  // pass through event handling once without observing the scene
  // -- allows any dependent nodes to be created
  appLogic->ProcessMRMLEvents (scene, vtkCommand::ModifiedEvent, NULL);  
  appLogic->SetAndObserveMRMLScene ( scene );
  appLogic->CreateProcessingThread();

  slicerApp->SplashMessage("Creating Application GUI...");

  // CREATE APPLICATION GUI, including the main window 
  vtkSlicerApplicationGUI *appGUI = vtkSlicerApplicationGUI::New ( );
  appGUI->SetApplication ( slicerApp );
  appGUI->SetAndObserveApplicationLogic ( appLogic );
  vtkIntArray *appGUIEvents = vtkIntArray::New();
  appGUIEvents->InsertNextValue( vtkCommand::ModifiedEvent );
  appGUIEvents->InsertNextValue( vtkMRMLScene::NodeAddedEvent );
  appGUIEvents->InsertNextValue( vtkMRMLScene::SceneCloseEvent );
  appGUI->SetAndObserveMRMLSceneEvents ( scene, appGUIEvents );
  appGUIEvents->Delete();

  // set fonts from registry before building GUI...
  /*
    if ( appGUI->GetMainSlicerWindow()->GetApplicationSettingsInterface() )
    {
    slicerApp->GetSlicerTheme()->InstallFonts();
    }
  */
  slicerApp->SaveUserInterfaceGeometryOn();


  // Create Remote I/O and Cache handling mechanisms
  // and configure them using Application registry values
  vtkCacheManager *cacheManager = vtkCacheManager::New();
  cacheManager->SetRemoteCacheLimit ( slicerApp->GetRemoteCacheLimit() );
  cacheManager->SetRemoteCacheFreeBufferSize ( slicerApp->GetRemoteCacheFreeBufferSize() );
  cacheManager->SetEnableForceRedownload ( slicerApp->GetEnableForceRedownload() );
  cacheManager->SetMRMLScene ( scene );
  //cacheManager->SetEnableRemoteCacheOverwriting ( slicerApp->GetEnableRemoteCacheOverwriting() );
  //--- MRML collection of data transfers with access to cache manager
  vtkDataIOManager *dataIOManager = vtkDataIOManager::New();
  dataIOManager->SetCacheManager ( cacheManager );
  dataIOManager->SetEnableAsynchronousIO ( slicerApp->GetEnableAsynchronousIO () );
  //--- Data transfer logic
  vtkDataIOManagerLogic *dataIOManagerLogic = vtkDataIOManagerLogic::New();
  dataIOManagerLogic->SetMRMLScene ( scene );
  dataIOManagerLogic->SetApplicationLogic ( appLogic );
  dataIOManagerLogic->SetAndObserveDataIOManager ( dataIOManager );


  scene->SetDataIOManager ( dataIOManager );
  scene->SetCacheManager( cacheManager );
  vtkCollection *URIHandlerCollection = vtkCollection::New();
  // add some new handlers
    
    
  scene->SetURIHandlerCollection( URIHandlerCollection );
#if !defined(REMOTEIO_DEBUG)
  // register all existing uri handlers (add to collection)
  vtkHTTPHandler *httpHandler = vtkHTTPHandler::New();
  httpHandler->SetPrefix ( "http://" );
  scene->AddURIHandler(httpHandler);
  httpHandler->Delete();

  vtkSRBHandler *srbHandler = vtkSRBHandler::New();
  srbHandler->SetPrefix ( "srb://" );
  scene->AddURIHandler(srbHandler);
  srbHandler->Delete();

  vtkXNATHandler *xnatHandler = vtkXNATHandler::New();
  vtkSlicerXNATPermissionPrompterWidget *xnatPermissionPrompter = vtkSlicerXNATPermissionPrompterWidget::New();
  xnatPermissionPrompter->SetApplication ( slicerApp );
  xnatPermissionPrompter->SetPromptTitle ("Permission Prompt");
  xnatHandler->SetPrefix ( "xnat://" );
  xnatHandler->SetRequiresPermission (1);
  xnatHandler->SetPermissionPrompter ( xnatPermissionPrompter );
  scene->AddURIHandler(xnatHandler);
  xnatPermissionPrompter->Delete();
  xnatHandler->Delete();
#endif


#ifndef SLICES_DEBUG
  vtkSlicerSlicesGUI *slicesGUI = vtkSlicerSlicesGUI::New ();
    
  // build the application GUI
  appGUI->SetSlicesGUI(slicesGUI);
#endif

  appGUI->BuildGUI ( );
  appGUI->AddGUIObservers ( );
  slicerApp->SetApplicationGUI ( appGUI );
  slicerApp->ConfigureRemoteIOSettingsFromRegistry();

#ifndef SLICES_DEBUG
  // set a pointer to vtkSlicerSlicesGUI in vtkSlicerApplicationGUI
  slicesGUI->SetApplication ( slicerApp );
  slicesGUI->SetApplicationGUI ( appGUI );
  slicesGUI->SetAndObserveApplicationLogic ( appLogic );
  slicesGUI->SetAndObserveMRMLScene ( scene );
  slicesGUI->SetGUIName( "Slices" );
  slicesGUI->GetUIPanel()->SetName ( slicesGUI->GetGUIName ( ) );
  slicesGUI->GetUIPanel()->SetUserInterfaceManager ( appGUI->GetMainSlicerWindow( )->GetMainUserInterfaceManager( ) );
  slicesGUI->GetUIPanel( )->Create( );
  slicerApp->AddModuleGUI ( slicesGUI );
  slicesGUI->BuildGUI ();
  slicesGUI->AddGUIObservers ( );
#endif

  // get the module paths that the user has configured
  std::string userModulePaths;
  if (slicerApp->GetModulePaths())
    {
    userModulePaths = slicerApp->GetModulePaths();
    }

  // define module paths based on the slicer installation
  // or build tree.

  std::string modulePaths;
  std::string defaultModulePaths;

  // On Win32, *both* paths have to be there, since scripts are installed
  // in the install location, and exec/libs are *automatically* installed
  // in intDir.
  defaultModulePaths = slicerHome + "/" + Slicer3_INSTALL_MODULES_LIB_DIR;
  if (hasIntDir)
    {
    defaultModulePaths = defaultModulePaths + PathSep + 
      slicerHome + "/" + Slicer3_INSTALL_MODULES_LIB_DIR + "/" + intDir;
    }

  // add the default modules directory (based on the slicer
  // installation or build tree) to the user paths
  modulePaths = userModulePaths + PathSep + defaultModulePaths;

  vtksys_stl::vector<vtksys_stl::string> modulePathsList;
  vtksys::SystemTools::Split(modulePaths.c_str(), modulePathsList, PathSep[0]);
  vtksys_stl::vector<vtksys_stl::string>::iterator module_paths_it;
  vtksys_stl::vector<vtksys_stl::string>::iterator module_paths_end =
    modulePathsList.end();

  // define plugins paths based on the slicer installation
  // or build tree.

  std::string pluginsPaths;
  std::string defaultPluginsPaths;

  // On Win32, *both* paths have to be there, since scripts are installed
  // in the install location, and exec/libs are *automatically* installed
  // in intDir.
  defaultPluginsPaths = slicerHome + "/" + Slicer3_INSTALL_PLUGINS_BIN_DIR;
  if (hasIntDir)
    {
    defaultPluginsPaths = defaultPluginsPaths + PathSep + 
      slicerHome + "/" + Slicer3_INSTALL_PLUGINS_BIN_DIR + "/" + intDir;
    }
    
  // add the default plugins directory (based on the slicer
  // installation or build tree) to the user paths
  pluginsPaths = userModulePaths + PathSep + defaultPluginsPaths;


  vtksys_stl::vector<vtksys_stl::string> pluginsPathsList;
  vtksys::SystemTools::Split(pluginsPaths.c_str(), pluginsPathsList, PathSep[0]);
  vtksys_stl::vector<vtksys_stl::string>::iterator plugins_paths_it;
  vtksys_stl::vector<vtksys_stl::string>::iterator plugins_paths_end =
    pluginsPathsList.end();

  // Set cache paths for modules

  std::string cachePath;
  std::string defaultCachePath;
  std::string userCachePath;

  // define a default cache for module information
  defaultCachePath = slicerHome + "/" + Slicer3_INSTALL_PLUGINS_CACHE_DIR;
  if (hasIntDir)
    {
    defaultCachePath += "/" + intDir;
    }
      
  // get the cache path that the user has configured
  if (slicerApp->GetModuleCachePath())
    {
    userCachePath = slicerApp->GetModuleCachePath();
    }

  // if user cache path is set and we can write to it, use it.
  // if user cache path is not set or we cannot write to it, try
  // the default cache path.
  // if we cannot write to the default cache path, then warn and
  // don't use a cache.
  vtksys::Directory directory;
  if (userCachePath != "")
    {
    if (!vtksys::SystemTools::FileExists(userCachePath.c_str()))
      {
      vtksys::SystemTools::MakeDirectory(userCachePath.c_str());
      }
    if (vtksys::SystemTools::FileExists(userCachePath.c_str())
        && vtksys::SystemTools::FileIsDirectory(userCachePath.c_str()))
      {
      std::ofstream tst((userCachePath + "/tstCache.txt").c_str());
      if (tst)
        {
        cachePath = userCachePath;
        }
      }
    }
  if (cachePath == "")
    {
    if (!vtksys::SystemTools::FileExists(defaultCachePath.c_str()))
      {
      vtksys::SystemTools::MakeDirectory(defaultCachePath.c_str());
      }
    if (vtksys::SystemTools::FileExists(defaultCachePath.c_str())
        && vtksys::SystemTools::FileIsDirectory(defaultCachePath.c_str()))
      {
      std::ofstream tst((defaultCachePath + "/tstCache.txt").c_str());
      if (tst)
        {
        cachePath = defaultCachePath;
        }
      }
    }
  if (ClearModuleCache)
    {
    std::string cacheFile = cachePath + "/ModuleCache.csv";
    vtksys::SystemTools::RemoveFile(cacheFile.c_str());
    }
  if (NoModuleCache)
    {
    cachePath = "";
    }
  if (cachePath == "")
    {
    if (NoModuleCache)
      {
      std::cout << "Module cache disabled by command line argument."
                << std::endl;
      }
    else
      {
      std::cout << "Module cache disabled. Slicer application directory may not be writable, a user level cache directory may not be set, or the user level cache directory may not be writable." << std::endl;
      }
    }

    
  // ------------------------------
  // CREATE MODULE LOGICS & GUIS; add to GUI collection
  // (those modules not made using the LM library must
  // must be listed individually, as below...)
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


  // LOADABLE MODULES
  LoadableModuleFactory loadableModuleFactory;
  loadableModuleFactory.SetName("Slicer");
  loadableModuleFactory.SetSearchPaths( modulePaths );

  if (VerboseModuleDiscovery)
    {
    loadableModuleFactory.SetWarningMessageCallback( WarningMessage );
    loadableModuleFactory.SetErrorMessageCallback( ErrorMessage );
    loadableModuleFactory.SetInformationMessageCallback( InformationMessage);
    }
  if (!NoSplash)
    {
    loadableModuleFactory.SetModuleDiscoveryMessageCallback( SplashMessage );
    }

  // obey the NoModules command line argument
  // without scanning, downstream iterators will be empty

  if (slicerApp->GetLoadModules() && !NoModules)
    {
    loadableModuleFactory.Scan();
    }

  std::vector<std::string> loadableModuleNames = 
    loadableModuleFactory.GetModuleNames();
  std::vector<std::string>::const_iterator lmit =
    loadableModuleNames.begin();
    
  while (lmit != loadableModuleNames.end())
    {
    LoadableModuleDescription desc = loadableModuleFactory.GetModuleDescription(*lmit);
    slicerApp->SplashMessage(desc.GetMessage().c_str());

    // Initialize TCL

    TclInit Tcl_Init = desc.GetTclInitFunction();

    if (Tcl_Init != 0)
      {
      (*Tcl_Init)(interp);
      }
    else
      {
      std::string warning("Cannot get tcl init function for: ");
      warning += desc.GetName(); 
      warning += std::string("\n");
      WarningMessage( warning.c_str() );
      }

    vtkSlicerModuleGUI* gui = desc.GetGUIPtr();
    vtkSlicerModuleLogic* logic = desc.GetLogicPtr();
    logic->SetAndObserveMRMLScene( scene );
    logic->SetModuleName(desc.GetShortName().c_str());

    vtkIntArray* events = logic->NewObservableEvents();
    logic->SetAndObserveMRMLSceneEvents(scene, events);
    events->Delete();
        
    events = gui->NewObservableEvents();
    gui->SetAndObserveMRMLSceneEvents(scene, events);
    events->Delete();
        
    logic->SetApplicationLogic(appLogic);
    logic->SetModuleLocation(desc.GetLocation().c_str());

    gui->SetApplication( slicerApp );
    gui->SetApplicationGUI( appGUI );
    gui->SetAndObserveApplicationLogic( appLogic );
    gui->SetAndObserveMRMLScene( scene );
    gui->SetModuleLogic(logic);
    gui->SetGUIName( desc.GetGUIName().c_str() );
    gui->GetUIPanel()->SetName( gui->GetGUIName ( ) );
    gui->GetUIPanel()->SetUserInterfaceManager( appGUI->GetMainSlicerWindow()->GetMainUserInterfaceManager ( ) );
    gui->GetUIPanel()->Create( );
    slicerApp->AddModuleGUI( gui );
    gui->BuildGUI ( );
    gui->AddGUIObservers ( );
    gui->Init();

    lmit++;
    }



  // ADD INDIVIDUAL MODULES
  // (these require appGUI to be built):
  // --- Volumes module

#if !defined(VOLUMES_DEBUG) && defined(Slicer3_BUILD_MODULES)
  slicerApp->SplashMessage("Initializing Volumes Module...");

  vtkSlicerVolumesLogic *volumesLogic = vtkSlicerVolumesLogic::New ( );
  volumesLogic->SetAndObserveMRMLScene ( scene );
  vtkSlicerVolumesGUI *volumesGUI = vtkSlicerVolumesGUI::New ( );
  volumesGUI->SetApplication ( slicerApp );
  volumesGUI->SetApplicationGUI ( appGUI );
  volumesGUI->SetAndObserveApplicationLogic ( appLogic );
  volumesGUI->SetAndObserveMRMLScene ( scene );
  volumesGUI->SetModuleLogic ( volumesLogic );
  volumesGUI->SetGUIName( "Volumes" );
  volumesGUI->GetUIPanel()->SetName ( volumesGUI->GetGUIName ( ) );
  volumesGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWindow()->GetMainUserInterfaceManager ( ) );
  volumesGUI->GetUIPanel()->Create ( );
  slicerApp->AddModuleGUI ( volumesGUI );
#endif


#ifndef MODELS_DEBUG
  slicerApp->SplashMessage("Initializing Models Module...");

  // --- Models module    
  vtkSlicerModelsLogic *modelsLogic = vtkSlicerModelsLogic::New ( );
  vtkSlicerModelHierarchyLogic *modelHierarchyLogic = vtkSlicerModelHierarchyLogic::New ( );
  modelsLogic->SetAndObserveMRMLScene ( scene );
  vtkSlicerModelsGUI *modelsGUI = vtkSlicerModelsGUI::New ( );
  modelsGUI->SetApplication ( slicerApp );
  modelsGUI->SetApplicationGUI ( appGUI );
  modelsGUI->SetAndObserveApplicationLogic ( appLogic );
  modelsGUI->SetAndObserveMRMLScene ( scene );
  modelsGUI->SetModuleLogic ( modelsLogic );
  modelsGUI->SetModelHierarchyLogic( modelHierarchyLogic );
  appGUI->GetViewerWidget()->SetModelHierarchyLogic(modelHierarchyLogic);
  modelsGUI->SetGUIName( "Models" );
  modelsGUI->GetUIPanel()->SetName ( modelsGUI->GetGUIName ( ) );
  modelsGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWindow()->GetMainUserInterfaceManager ( ) );
  modelsGUI->GetUIPanel()->Create ( );
  slicerApp->AddModuleGUI ( modelsGUI );
#endif


#ifndef FIDUCIALS_DEBUG
  slicerApp->SplashMessage("Initializing Fiducials Module...");

  // --- Fiducials module    
  vtkSlicerFiducialsLogic *fiducialsLogic = vtkSlicerFiducialsLogic::New ( );
  fiducialsLogic->SetAndObserveMRMLScene ( scene );
  vtkSlicerFiducialsGUI *fiducialsGUI = vtkSlicerFiducialsGUI::New ( );
  fiducialsGUI->SetApplication ( slicerApp );
  fiducialsGUI->SetApplicationGUI ( appGUI );
  fiducialsGUI->SetAndObserveApplicationLogic ( appLogic );
  fiducialsGUI->SetAndObserveMRMLScene ( scene );
  fiducialsGUI->SetModuleLogic ( fiducialsLogic );
  fiducialsGUI->SetGUIName( "Fiducials" );
  fiducialsGUI->GetUIPanel()->SetName ( fiducialsGUI->GetGUIName ( ) );
  fiducialsGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWindow()->GetMainUserInterfaceManager ( ) );
  fiducialsGUI->GetUIPanel()->Create ( );
  slicerApp->AddModuleGUI ( fiducialsGUI );
#endif

  slicerApp->SplashMessage("Initializing ROI Module...");

  // --- Fiducials module    
  vtkSlicerROILogic *ROILogic = vtkSlicerROILogic::New ( );
  ROILogic->SetAndObserveMRMLScene ( scene );
  vtkSlicerROIGUI *ROIGUI = vtkSlicerROIGUI::New ( );
  ROIGUI->SetApplication ( slicerApp );
  ROIGUI->SetApplicationGUI ( appGUI );
  ROIGUI->SetAndObserveApplicationLogic ( appLogic );
  ROIGUI->SetAndObserveMRMLScene ( scene );
  ROIGUI->SetModuleLogic ( ROILogic );
  ROIGUI->SetGUIName( "ROI" );
  ROIGUI->GetUIPanel()->SetName ( ROIGUI->GetGUIName ( ) );
  ROIGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWindow()->GetMainUserInterfaceManager ( ) );
  ROIGUI->GetUIPanel()->Create ( );
  slicerApp->AddModuleGUI ( ROIGUI );

#ifndef COLORS_DEBUG
  slicerApp->SplashMessage("Initializing Colors Module...");

  // -- Color module
  vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New ( );
  // observe the scene's new scene event
  vtkIntArray *colorEvents = vtkIntArray::New();
  colorEvents->InsertNextValue( vtkMRMLScene::NewSceneEvent );
  colorEvents->InsertNextValue( vtkMRMLScene::SceneCloseEvent );
  colorLogic->SetAndObserveMRMLSceneEvents ( scene,  colorEvents);
  colorEvents->Delete();
  // this should be triggered somehow by a new scene event, but for now,
  // call it explicitly
  colorLogic->AddDefaultColorNodes();
  vtkSlicerColorGUI *colorGUI = vtkSlicerColorGUI::New ( );
  colorGUI->SetApplication ( slicerApp );
  colorGUI->SetApplicationGUI ( appGUI );
  colorGUI->SetAndObserveApplicationLogic ( appLogic );
  colorGUI->SetAndObserveMRMLScene ( scene );
  colorGUI->SetModuleLogic ( colorLogic );
  colorGUI->SetGUIName( "Color" );
  colorGUI->GetUIPanel()->SetName ( colorGUI->GetGUIName ( ) );
  colorGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWindow()->GetMainUserInterfaceManager ( ) );
  colorGUI->GetUIPanel()->Create ( );
  slicerApp->AddModuleGUI ( colorGUI );
#endif

  // --- Transforms module
  slicerApp->SplashMessage("Initializing Transforms Module...");

  vtkSlicerTransformsGUI *transformsGUI = vtkSlicerTransformsGUI::New ( );
  transformsGUI->SetApplication ( slicerApp );
  transformsGUI->SetApplicationGUI ( appGUI );
  transformsGUI->SetAndObserveApplicationLogic ( appLogic );
  transformsGUI->SetAndObserveMRMLScene ( scene );
  transformsGUI->SetGUIName( "Transforms" );
  transformsGUI->GetUIPanel()->SetName ( transformsGUI->GetGUIName ( ) );
  transformsGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWindow()->GetMainUserInterfaceManager ( ) );
  transformsGUI->GetUIPanel()->Create ( );
  slicerApp->AddModuleGUI ( transformsGUI );

  //--- Data module
  slicerApp->SplashMessage( "Initializing Data Module...");

  //vtkSlicerDataLogic *dataLogic = vtkSlicerDataLogic::New ( );
  //dataLogic->SetAndObserveMRMLScene ( scene );
  //dataLogic->SetApplicationLogic ( appLogic );
  vtkSlicerDataGUI *dataGUI = vtkSlicerDataGUI::New ( );
  dataGUI->SetApplication ( slicerApp );
  dataGUI->SetApplicationGUI ( appGUI );
  dataGUI->SetAndObserveApplicationLogic ( appLogic );
  dataGUI->SetAndObserveMRMLScene ( scene );
  //dataGUI->SetModuleLogic ( dataLogic );
  dataGUI->SetGUIName( "Data" );
  dataGUI->GetUIPanel()->SetName ( dataGUI->GetGUIName ( ) );
  dataGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWindow()->GetMainUserInterfaceManager ( ) );
  dataGUI->GetUIPanel()->Create ( );    
  slicerApp->AddModuleGUI ( dataGUI );
    
#ifndef CAMERA_DEBUG
  slicerApp->SplashMessage("Initializing Camera Module...");

  // --- Camera module
  vtkSlicerCamerasGUI *cameraGUI = vtkSlicerCamerasGUI::New ( );
  cameraGUI->SetApplication ( slicerApp );
  cameraGUI->SetApplicationGUI ( appGUI );
  cameraGUI->SetAndObserveApplicationLogic ( appLogic );
  cameraGUI->SetAndObserveMRMLScene ( scene );
  cameraGUI->SetGUIName( "Cameras" );
  cameraGUI->GetUIPanel()->SetName ( cameraGUI->GetGUIName ( ) );
  cameraGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWindow()->GetMainUserInterfaceManager ( ) );
  cameraGUI->GetUIPanel()->Create ( );
  slicerApp->AddModuleGUI ( cameraGUI );
  cameraGUI->BuildGUI ( );
  cameraGUI->AddGUIObservers ( );
  cameraGUI->UpdateCameraSelector();
#endif

  // --- Slices module
  slicerApp->SplashMessage("Initializing Slices Module...");
  appLogic->CreateSliceLogics();
  appGUI->InitializeSlicesControlGUI();
  appGUI->InitializeViewControlGUI();
  //    appGUI->InitializeNavigationWidget();


  
#if !defined(TRACTOGRAPHY_DEBUG) && defined(Slicer3_BUILD_MODULES)
  // --- Tractography Display module
  slicerApp->SplashMessage("Initializing Tractography Display Module...");
  vtkSlicerTractographyDisplayGUI *slicerTractographyDisplayGUI = vtkSlicerTractographyDisplayGUI::New ( );
  vtkSlicerTractographyDisplayLogic *slicerTractographyDisplayLogic  = vtkSlicerTractographyDisplayLogic::New ( );
  //slicerTractographyDisplayLogic->DebugOn ( );

  // Observe scene events to handle display logic for new nodes or new scenes
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
  //events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  slicerTractographyDisplayLogic->SetAndObserveMRMLSceneEvents ( scene , events );
  events->Delete();

  slicerTractographyDisplayLogic->SetApplicationLogic ( appLogic );
  slicerTractographyDisplayGUI->SetLogic ( slicerTractographyDisplayLogic );
  slicerTractographyDisplayGUI->SetApplication ( slicerApp );
  slicerTractographyDisplayGUI->SetApplicationLogic ( appLogic );
  slicerTractographyDisplayGUI->SetApplicationGUI ( appGUI );
  slicerTractographyDisplayGUI->SetGUIName( "DisplayLoadSave" );
  slicerTractographyDisplayGUI->GetUIPanel()->SetName ( slicerTractographyDisplayGUI->GetGUIName ( ) );
  slicerTractographyDisplayGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWindow()->GetMainUserInterfaceManager ( ) );
  slicerTractographyDisplayGUI->GetUIPanel()->Create ( );
  slicerApp->AddModuleGUI ( slicerTractographyDisplayGUI );
  slicerTractographyDisplayGUI->BuildGUI ( );
  slicerTractographyDisplayGUI->AddGUIObservers ( );
#endif

#if !defined(TRACTOGRAPHY_DEBUG) && defined(Slicer3_BUILD_MODULES)
  // --- Tractography Fiducial Seeding module
  slicerApp->SplashMessage("Initializing Tractography Fiducial Seeding Module...");
  vtkSlicerTractographyFiducialSeedingGUI *slicerTractographyFiducialSeedingGUI = vtkSlicerTractographyFiducialSeedingGUI::New ( );


  slicerTractographyDisplayLogic->SetApplicationLogic ( appLogic );
  slicerTractographyFiducialSeedingGUI->SetApplication ( slicerApp );
  slicerTractographyFiducialSeedingGUI->SetApplicationLogic ( appLogic );
  slicerTractographyFiducialSeedingGUI->SetApplicationGUI ( appGUI );
  slicerTractographyFiducialSeedingGUI->SetGUIName( "FiducialSeeding" );
  slicerTractographyFiducialSeedingGUI->GetUIPanel()->SetName ( slicerTractographyFiducialSeedingGUI->GetGUIName ( ) );
  slicerTractographyFiducialSeedingGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWindow()->GetMainUserInterfaceManager ( ) );
  slicerTractographyFiducialSeedingGUI->GetUIPanel()->Create ( );
  slicerApp->AddModuleGUI ( slicerTractographyFiducialSeedingGUI );
  slicerTractographyFiducialSeedingGUI->BuildGUI ( );
  slicerTractographyFiducialSeedingGUI->AddGUIObservers ( );
#endif


  //
  //--- Cache and RemoteIO ManagerGUI
  //
#if !defined (REMOTEIO_DEBUG)
  vtkSlicerCacheAndDataIOManagerGUI *remoteIOGUI = vtkSlicerCacheAndDataIOManagerGUI::New();
  remoteIOGUI->SetApplication ( slicerApp);
  remoteIOGUI->SetAndObserveCacheManager ( scene->GetCacheManager() );
  remoteIOGUI->SetAndObserveDataIOManager ( scene->GetDataIOManager() );
  remoteIOGUI->Enter();
#endif

#if !defined(DAEMON_DEBUG) && defined(Slicer3_BUILD_MODULES)
  //
  // --- SlicerDaemon Module
  // need to source the slicerd.tcl script here
  // - only start if selected on command line or enabled in registry, since 
  //   windows firewall will complain. 
  //

  if ( Daemon || slicerApp->GetEnableDaemon() )
    {
    slicerApp->SplashMessage("Initializing Slicer Daemon...");
    std::string cmd;
    cmd =  "source \"" + slicerHome + "/"
      Slicer3_INSTALL_MODULES_LIB_DIR "/SlicerDaemon/Tcl/slicerd.tcl\"; slicerd_start; ";
    Slicer3_Tcl_Eval(interp, cmd.c_str());
    }
#endif

#if !defined(COMMANDLINE_DEBUG) && (defined(Slicer3_BUILD_CLI) || defined(Slicer3_BUILD_MODULES))
  std::vector<std::string> moduleNames;
  std::vector<std::string>::const_iterator mit;
  if ( slicerApp->GetLoadCommandLineModules() && !NoModules )
    {
    // --- Scan for command line and shared object modules
    //
    // - set the module path the the default if there isn't something set
    // already
    //
    // NB: don't want to use the registry value of the module path, because 
    // there is only one value even if you have multiple versions of 
    // slicer installed, you may get conflicts.
    //
    //

    // Search for modules
    ModuleFactory moduleFactory;
    moduleFactory.SetName("Slicer");
    moduleFactory.SetSearchPaths( pluginsPaths );
    moduleFactory.SetCachePath( cachePath );
    if (VerboseModuleDiscovery)
      {
      moduleFactory.SetWarningMessageCallback( WarningMessage );
      moduleFactory.SetErrorMessageCallback( ErrorMessage );
      moduleFactory.SetInformationMessageCallback( InformationMessage );
      }
    if (!NoSplash)
      {
      moduleFactory.SetModuleDiscoveryMessageCallback( SplashMessage );
      }      
    moduleFactory.Scan();

    // Register the node type for the command line modules
    vtkMRMLCommandLineModuleNode* clmNode = vtkMRMLCommandLineModuleNode::New();
    scene->RegisterNodeClass(clmNode);
    clmNode->Delete();
      
    // add the modules to the available modules
    moduleNames = moduleFactory.GetModuleNames();
    mit = moduleNames.begin();
    while (mit != moduleNames.end())
      {
      // slicerCerr(moduleFactory.GetModuleDescription(*mit) << endl);

      vtkCommandLineModuleGUI *commandLineModuleGUI
        = vtkCommandLineModuleGUI::New();
      vtkCommandLineModuleLogic *commandLineModuleLogic
        = vtkCommandLineModuleLogic::New ( );
      
      // set up stream redirection
      if (NoModuleStreamsRedirect)
        {
        commandLineModuleLogic->RedirectModuleStreamsOff();
        }

      // Set the ModuleDescripton on the gui
      commandLineModuleGUI
        ->SetModuleDescription( moduleFactory.GetModuleDescription(*mit) );

      // Register the module description in the master list
      vtkMRMLCommandLineModuleNode::RegisterModuleDescription( moduleFactory.GetModuleDescription(*mit) );
        
      // Configure the Logic, GUI, and add to app
      commandLineModuleLogic->SetAndObserveMRMLScene ( scene );
      commandLineModuleLogic->SetApplicationLogic (appLogic);
      commandLineModuleGUI->SetLogic ( commandLineModuleLogic );
      commandLineModuleGUI->SetApplication ( slicerApp );
      commandLineModuleGUI->SetApplicationLogic ( appLogic );
      commandLineModuleGUI->SetApplicationGUI ( appGUI );
      commandLineModuleGUI->SetGUIName( moduleFactory.GetModuleDescription(*mit).GetTitle().c_str() );
      commandLineModuleGUI->GetUIPanel()->SetName ( commandLineModuleGUI->GetGUIName ( ) );
      commandLineModuleGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWindow()->GetMainUserInterfaceManager ( ) );
      commandLineModuleGUI->GetUIPanel()->Create ( );
      slicerApp->AddModuleGUI ( commandLineModuleGUI );

      ++mit;
      }
    }
#endif // CLIMODULES_DEBUG


  //
  // get the Tcl name so the vtk class will be registered in the interpreter
  // as a byproduct
  // - set some handy variables so it will be easy to access these classes
  ///  from   the tkcon
  // - all the variables are put in the slicer3 tcl namespace for easy access
  // - note, slicerApp is loaded as $::slicer3::Application above, so it
  //   can be used from scripts that run without the GUI
  //
  const char *name;
  name = appGUI->GetTclName();
  slicerApp->Script ("namespace eval slicer3 set ApplicationGUI %s", name);

  lmit = loadableModuleNames.begin();
     
  while (lmit != loadableModuleNames.end())
    {
    LoadableModuleDescription desc = loadableModuleFactory.GetModuleDescription(*lmit);

    vtkSlicerModuleGUI* gui = desc.GetGUIPtr();

    name = gui->GetTclName();
    std::string format("namespace eval slicer3 set ");
    format += desc.GetShortName();
    format += "GUI %s";

    slicerApp->Script (format.c_str(), name);        

    lmit++;
    }


#ifndef REMOTEIO_DEBUG
  name = remoteIOGUI->GetTclName();
  slicerApp->Script ("namespace eval slicer3 set RemoteIOGUI %s", name);
#endif
    
#ifndef SLICES_DEBUG
  name = slicesGUI->GetTclName();
  slicerApp->Script ("namespace eval slicer3 set SlicesGUI %s", name);
#endif

#if !defined(VOLUMES_DEBUG) && defined(Slicer3_BUILD_MODULES)
  name = volumesGUI->GetTclName();
  slicerApp->Script ("namespace eval slicer3 set VolumesGUI %s", name);
#endif
#ifndef MODELS_DEBUG
  name = modelsGUI->GetTclName();
  slicerApp->Script ("namespace eval slicer3 set ModelsGUI %s", name);
#endif
#ifndef FIDUCIALS_DEBUG
  name = fiducialsGUI->GetTclName();
  slicerApp->Script ("namespace eval slicer3 set FiducialsGUI %s", name);
#endif

  name = ROIGUI->GetTclName();
  slicerApp->Script ("namespace eval slicer3 set ROIGUI %s", name);
 

#ifndef COLORS_DEBUG
  name = colorGUI->GetTclName();
  slicerApp->Script ("namespace eval slicer3 set ColorGUI %s", name);
#endif

  name = transformsGUI->GetTclName();
  slicerApp->Script ("namespace eval slicer3 set TransformsGUI %s", name);

  if ( appGUI->GetViewerWidget() )
    {
    name = appGUI->GetViewerWidget()->GetTclName();
    slicerApp->Script ("namespace eval slicer3 set ViewerWidget %s", name);
    }

  slicerApp->Script ("namespace eval slicer3 set ApplicationLogic [$::slicer3::ApplicationGUI GetApplicationLogic]");
  slicerApp->Script ("namespace eval slicer3 set MRMLScene [$::slicer3::ApplicationLogic GetMRMLScene]");

  // Get the global event broker (same as returned by vtkEventBroker::GetInstance()
  // - since the singleton method is not exposed to tcl, access it this way
  //   and then delete it at the end.
  slicerApp->Script ("namespace eval slicer3 set Broker [vtkEventBroker New]");

#if !defined(COMMANDLINE_DEBUG) && (defined(Slicer3_BUILD_CLI) || defined(Slicer3_BUILD_MODULES))
  mit = moduleNames.begin();
  while ( mit != moduleNames.end() )
    {
    vtkSlicerModuleGUI *module;
    module = slicerApp->GetModuleGUIByName( (*mit).c_str() );
    if (module) 
      {
      name = module->GetTclName();
      }
    std::string title = *mit;
    std::transform(
      title.begin(), title.end(), title.begin(), SpacesToUnderscores());

    slicerApp->Script ("namespace eval slicer3 set CommandLineModuleGUI_%s %s", title.c_str(), name);
      
    ++mit;
    }
#endif

#if !defined(SCRIPTEDMODULE_DEBUG) && defined(Slicer3_BUILD_MODULES)
  //
  // process any ScriptedModules
  // - scan for pkgIndex.tcl files 
  // - load the corresponding packages
  // - create scripted logic and scripted gui instances for the module
  // - have the GUI construct itself
  //

  slicerApp->SplashMessage("Initializing Scripted Modules...");
  std::string tclCommand = "set ::Slicer3_PACKAGES(list) {};";

  module_paths_it = modulePathsList.begin();
  for (; module_paths_it != module_paths_end; module_paths_it++)
    {
    vtksys_stl::string module_path(*module_paths_it);
    vtksys::SystemTools::ConvertToUnixSlashes(module_path);
    if (*module_path.c_str() &&
        vtksys::SystemTools::FileExists(module_path.c_str()))
      {
      tclCommand += "set dirs [glob \"" + module_path + "/*\"]; ";
      tclCommand += "foreach d $dirs { ";
      tclCommand += "  foreach subdir {\".\" \"Tcl\"} {";
      tclCommand += "    set location [file join [file join $d $subdir] pkgIndex.tcl];";
      tclCommand += "    if { [file exists $location] } {";
      tclCommand += "      set package [file tail $d];";
      tclCommand += "      if { [lsearch -exact $::Slicer3_PACKAGES(list) $package] == -1 } {";
      tclCommand += "        set ::Slicer3_PACKAGES($package,location) $location;";
      tclCommand += "        lappend ::Slicer3_PACKAGES(list) $package;";
      tclCommand += "        lappend ::auto_path $d;";
      tclCommand += "      }";
      tclCommand += "    }";
      tclCommand += "  }";
      tclCommand += "}; ";
      }
    }
  Slicer3_Tcl_Eval( interp, tclCommand.c_str());

  tclCommand = "";
  tclCommand += "foreach package $::Slicer3_PACKAGES(list) { ";
  tclCommand += "  package require $package;";
  tclCommand += "  set ::Slicer3_PACKAGES($package,logic) [vtkScriptedModuleLogic New];";
  tclCommand += "  set logic $::Slicer3_PACKAGES($package,logic);";
  tclCommand += "  $logic SetModuleName $package;";
  tclCommand += "  $logic SetModuleLocation $::Slicer3_PACKAGES($package,location);";
  tclCommand += "  $logic SetAndObserveMRMLScene $::slicer3::MRMLScene;";
  tclCommand += "  $logic SetApplicationLogic $::slicer3::ApplicationLogic;";
  tclCommand += "  set ::Slicer3_PACKAGES($package,gui) [vtkScriptedModuleGUI New];";
  tclCommand += "  set gui $::Slicer3_PACKAGES($package,gui);";
  tclCommand += "  $gui SetModuleName $package;";
  tclCommand += "  $gui SetLogic $logic;";
  tclCommand += "  $gui SetApplicationGUI $::slicer3::ApplicationGUI;";
  tclCommand += "  $gui SetApplication $::slicer3::Application;";
  tclCommand += "  $gui SetGUIName $package;";
  tclCommand += "  [$gui GetUIPanel] SetName $package;";
  tclCommand += "  [$gui GetUIPanel] SetUserInterfaceManager [[$::slicer3::ApplicationGUI GetMainSlicerWindow] GetMainUserInterfaceManager];";
  tclCommand += "  [$gui GetUIPanel] Create;";
  tclCommand += "  $::slicer3::Application AddModuleGUI $gui;" ;
  tclCommand += "  $gui BuildGUI;";
  tclCommand += "  $gui AddGUIObservers;";
  tclCommand += "";
  tclCommand += "}";
  Slicer3_Tcl_Eval( interp, tclCommand.c_str() );

#ifdef Slicer3_USE_PYTHON
  slicerApp->SplashMessage("Initializing Python Scripted Modules...");
  std::string pythonCommand = "";
  pythonCommand += "import sys\n";
  pythonCommand += "import os\n";
  pythonCommand += "packageNames = []\n";

  module_paths_it = modulePathsList.begin();
  for (; module_paths_it != module_paths_end; module_paths_it++)
    {
    vtksys_stl::string module_path(*module_paths_it);
    vtksys::SystemTools::ConvertToUnixSlashes(module_path);
    if (*module_path.c_str() &&
        vtksys::SystemTools::FileExists(module_path.c_str()))
      {
      pythonCommand += "modulePath = '" + module_path + "'\n";
      pythonCommand += "sys.path.append(modulePath)\n";
      pythonCommand += "for packageName in os.listdir(modulePath):\n";
      pythonCommand += "    if os.path.isfile(os.path.join(modulePath,packageName,'__init__.py')):\n";
      pythonCommand += "        packageNames.append(packageName)\n";
      }
    }
  pythonCommand += "import Slicer\n";
  pythonCommand += "slicer = Slicer.Slicer()\n";
  pythonCommand += "PythonScriptedModuleDict = {}\n";
  pythonCommand += "for packageName in packageNames:\n";
  pythonCommand += "    PythonScriptedModuleDict[packageName] = __import__(packageName)\n";
  pythonCommand += "    logic = slicer.vtkScriptedModuleLogic.New()\n";
  pythonCommand += "    logic.SetModuleName(packageName)\n";
  pythonCommand += "    logic.SetAndObserveMRMLScene(slicer.MRMLScene)\n";
  pythonCommand += "    logic.SetApplicationLogic(slicer.ApplicationLogic)\n";
  pythonCommand += "    gui = slicer.vtkScriptedModuleGUI.New()\n";
  pythonCommand += "    gui.SetModuleName(packageName)\n";
  pythonCommand += "    gui.SetLanguageToPython()\n";
  pythonCommand += "    gui.SetLogic(logic)\n";
  pythonCommand += "    gui.SetApplicationGUI(slicer.ApplicationGUI)\n";
  pythonCommand += "    gui.SetApplication(slicer.Application)\n";
  pythonCommand += "    gui.SetGUIName(packageName)\n";
  pythonCommand += "    gui.GetUIPanel().SetName(packageName)\n";
  pythonCommand += "    gui.GetUIPanel().SetUserInterfaceManager(slicer.ApplicationGUI.GetMainSlicerWindow().GetMainUserInterfaceManager())\n";
  pythonCommand += "    gui.GetUIPanel().Create()\n";
  pythonCommand += "    slicer.Application.AddModuleGUI(gui)\n";
  pythonCommand += "    gui.BuildGUI()\n";
  pythonCommand += "    gui.AddGUIObservers()\n";
  v = PyRun_String( pythonCommand.c_str(),
                    Py_file_input,
                    PythonDictionary,PythonDictionary);
  if (v == NULL)
    {
    PyErr_Print();
    }
#endif
#endif

  //
  // create the three main slice viewers after slicesGUI is created
  //
  slicerApp->SplashMessage("Configuring Slices and Modules...");
  appGUI->PopulateModuleChooseList ( );
#ifndef SLICES_DEBUG
  appGUI->SetAndObserveMainSliceLogic ( appLogic->GetSliceLogic("Red"), appLogic->GetSliceLogic("Yellow"), appLogic->GetSliceLogic("Green") );
  appGUI->ConfigureMainSliceViewers ( );
#endif
    
  // ------------------------------
  // CONFIGURE SlICER'S SHARED GUI PANEL
  // Additional Modules GUI panel configuration.
  vtkKWUserInterfaceManagerNotebook *mnb = vtkKWUserInterfaceManagerNotebook::SafeDownCast (appGUI->GetMainSlicerWindow()->GetMainUserInterfaceManager());
  mnb->GetNotebook()->AlwaysShowTabsOff();
  mnb->GetNotebook()->ShowOnlyPagesWithSameTagOn();    

  // ------------------------------
  // DISPLAY WINDOW AND RUN
  slicerApp->SplashMessage("Finalizing Startup...");
  appGUI->DisplayMainSlicerWindow ( );

  Slicer3_Tcl_Eval( interp, "update" ) ;
    
  // More command line arguments:
  // use the startup script passed on command line if it exists
  if ( Script != "" )
    {    
    std::string cmd = "after idle source " + Script;
    Slicer3_Tcl_Eval( interp, cmd.c_str() ) ;
    }

  int res; // return code (exit code)

  // use the startup code passed on command line if it exists
  // Example for csh to exit after startup:
  //
  // ./Slicer3 --exec exit' 
  //
  if ( Exec != "" )
    {    
    std::string cmd = "set ::SLICER(exec) \"" + Exec + "\" ; ";
    cmd += "regsub -all {\\.,} $::SLICER(exec) \";\" ::SLICER(exec); ";
    cmd += "regsub -all {,\\.} $::SLICER(exec) \";\" ::SLICER(exec); ";
    cmd += "after idle eval $::SLICER(exec);";
    res = Slicer3_Tcl_Eval( interp, cmd.c_str() );
    }

  //
  // if there was no script file, the Args should hold a mrml file
  // 
  //
  if ( File == ""  && Args.size() != 0)
    {
    std::vector<std::string>::const_iterator argit = Args.begin();
    while (argit != Args.end())
      {
      std::string fileName;
      // strip backslashes
      char buffer[2]; buffer[1] = '\0';
      for (unsigned int i = 0; i < (*argit).size(); i++)
        {
        buffer[0] = (*argit)[i];
        if ( buffer[0] != '\\' )
          {
          fileName.append(std::string(buffer));
          }
        }
      // is it a MRML or XML file?
      if (fileName.find(".mrml",0) != std::string::npos
          || fileName.find(".MRML",0) != std::string::npos)
        {
        appLogic->GetMRMLScene()->SetURL(fileName.c_str());
        // and then load it
        slicerApp->SplashMessage("Set scene url, connecting...");
        std::string cmd = "after idle {update; $::slicer3::MRMLScene Connect}";
        res = Slicer3_Tcl_Eval( interp, cmd.c_str() );
        }                    
      else if (fileName.find(".xml",0) != std::string::npos
               || fileName.find(".XML",0) != std::string::npos)
        {
        // if it's an xml file, load it
        std::string cmd = "after idle {update; ImportSlicer2Scene \"" + *argit + "\"}";
        slicerApp->SplashMessage("Importing Slicer2 scene...");
        res = Slicer3_Tcl_Eval( interp, cmd.c_str() );
        slicerApp->SplashMessage("Imported scene.");
        }
      else if (fileName.find(".xcat",0) != std::string::npos
               || fileName.find(".XCAT",0) != std::string::npos)
        {
        // if it's an xcede file, load it
        slicerApp->SplashMessage("Importing Xcede catalog ...");
        std::string cmd = "after idle {update; XcatalogImport \"" + *argit + "\"}";
        res = Slicer3_Tcl_Eval( interp, cmd.c_str() );
        slicerApp->SplashMessage("Imported catalog.");
        }
      else if (fileName.find(".tcl",0) != std::string::npos)
        {
        // if it's a tcl file source it after the app starts
        std::string cmd = "after idle {update; source " + *argit + "}";
        res = Slicer3_Tcl_Eval( interp, cmd.c_str() );
        }
      else
        {
        // if we're not sure, assume it is data to load...
        std::string cmd = "after idle {update; ::Loader::ShowDialog {" + *argit + "}}";
        res = Slicer3_Tcl_Eval( interp, cmd.c_str() );
        }
      ++argit;
      }
    }

  //
  // --load_dicom <directory | filename>
  //
  if ( LoadDicomDir != "" )
    {    
    // load either a directory or an archetype
    std::string cmd = "after idle {update; ::Loader::LoadArchetype {" + LoadDicomDir + "}}";
    res = Slicer3_Tcl_Eval( interp, cmd.c_str() );
    }


  //--- set home module based on registry settings
  const char *homeModule = slicerApp->GetHomeModule();
  if ( homeModule && *homeModule )
    {
    appGUI->SelectModule ( homeModule );
    }
  else
    {
    appGUI->SelectModule("Data");
    }
#ifdef Slicer3_USE_PYTHON
  vtkSlicerApplication::GetInstance()->InitializePython(
    (void*)PythonModule, (void*)PythonDictionary);
#endif    

  //
  // Run!  - this will return when the user exits
  //
  res = slicerApp->StartApplication();

  appGUI->GetMainSlicerWindow()->PrepareForDelete();
  appGUI->GetMainSlicerWindow()->Close();

  // ------------------------------
  // REMOVE OBSERVERS and references to MRML and Logic

  lmit = loadableModuleNames.begin();
  while (lmit != loadableModuleNames.end()) {
  LoadableModuleDescription desc = loadableModuleFactory.GetModuleDescription(*lmit);

  desc.GetGUIPtr()->TearDownGUI();
  desc.GetGUIPtr()->RemoveGUIObservers();

  lmit++;
  }

#if !defined(TRACTOGRAPHY_DEBUG) && defined(Slicer3_BUILD_MODULES)
  slicerTractographyDisplayGUI->RemoveGUIObservers ( );
  slicerTractographyFiducialSeedingGUI->RemoveGUIObservers ( );
#endif
#if !defined(VOLUMES_DEBUG) && defined(Slicer3_BUILD_MODULES)
  //    volumesGUI->RemoveGUIObservers ( );
  volumesGUI->TearDownGUI ( );
#endif
#ifndef MODELS_DEBUG
  modelsGUI->TearDownGUI ( );
#endif
#ifndef FIDUCIALS_DEBUG
  fiducialsGUI->TearDownGUI ( );
#endif

  ROIGUI->TearDownGUI ( );

#ifndef COLORS_DEBUG
  colorGUI->TearDownGUI ( );
#endif

  transformsGUI->TearDownGUI ( );
#ifndef CAMERA_DEBUG
  cameraGUI->RemoveGUIObservers ( );
#endif

  dataGUI->TearDownGUI ( );
    
#ifndef SLICES_DEBUG
  //slicesGUI->RemoveGUIObservers ( );
#endif
  appGUI->RemoveGUIObservers ( );
#ifndef SLICES_DEBUG
  appGUI->SetAndObserveMainSliceLogic ( NULL, NULL, NULL );
#endif

#if !defined(COMMANDLINE_DEBUG) && (defined(Slicer3_BUILD_CLI) || defined(Slicer3_BUILD_MODULES))
  // remove the observers from the factory discovered modules
  // (as we remove the observers, cache the GUIs in a vector so we
  // can delete them later).
  mit = moduleNames.begin();
  std::vector<vtkSlicerModuleGUI*> moduleGUIs;
  while ( mit != moduleNames.end() )
    {
    vtkSlicerModuleGUI *module;
    module = slicerApp->GetModuleGUIByName( (*mit).c_str() );

    /*      module->RemoveGUIObservers();*/
    module->TearDownGUI ( );

    moduleGUIs.push_back(module);
      
    ++mit;
    }
#endif    

#if !defined(SCRIPTEDMODULE_DEBUG) && defined(Slicer3_BUILD_MODULES)
  // remove the observers from the scripted modules
  tclCommand = "";
  tclCommand += "foreach package $::Slicer3_PACKAGES(list) { ";
  tclCommand += "  $::Slicer3_PACKAGES($package,gui) RemoveGUIObservers;";
  tclCommand += "  $::Slicer3_PACKAGES($package,gui) TearDownGUI;";
  tclCommand += "}";
  Slicer3_Tcl_Eval( interp, tclCommand.c_str() );
#endif 

  // ------------------------------
  // Remove References to Module GUIs
  slicerApp->GetModuleGUICollection ( )->RemoveAllItems ( );
  slicerApp->GetTheme()->SetApplication( NULL );
  slicerApp->SetTheme( NULL );
    
  // ------------------------------
  // EXIT 
  slicerApp->Exit();

  // ------------------------------
  // DELETE 
    
  //
  //--- Cache and RemoteIO ManagerGUI
  //
#if !defined (REMOTEIO_DEBUG)
  remoteIOGUI->SetAndObserveDataIOManager ( NULL );
  remoteIOGUI->SetAndObserveCacheManager ( NULL );
  remoteIOGUI->TearDownGUI();
  remoteIOGUI->Delete();
#endif

  //--- Remote data handling mechanisms
  if ( dataIOManagerLogic != NULL )
    {
    dataIOManagerLogic->SetAndObserveDataIOManager ( NULL );
    dataIOManagerLogic->SetMRMLScene ( NULL );
    dataIOManagerLogic->Delete();
    dataIOManagerLogic = NULL;
    }
  if ( dataIOManager != NULL )
    {
    dataIOManager->Delete();
    dataIOManager = NULL;
    }
  if ( cacheManager != NULL )
    {
    cacheManager->SetMRMLScene ( NULL );
    cacheManager->Delete();
    cacheManager = NULL;
    }
  if (URIHandlerCollection != NULL )
    {
    URIHandlerCollection->Delete();
    URIHandlerCollection = NULL;
    }

    
  //--- delete gui first, removing Refs to Logic and MRML
    
  lmit = loadableModuleNames.begin();
  while (lmit != loadableModuleNames.end()) {
  LoadableModuleDescription desc = loadableModuleFactory.GetModuleDescription(*lmit);

  desc.GetGUIPtr()->Delete();

  lmit++;
  }

#if !defined(TRACTOGRAPHY_DEBUG) && defined(Slicer3_BUILD_MODULES)
  slicerTractographyDisplayGUI->Delete ();
  slicerTractographyFiducialSeedingGUI->Delete ();
#endif

#if !defined(VOLUMES_DEBUG) && defined(Slicer3_BUILD_MODULES)
  volumesGUI->Delete ();
#endif
#ifndef MODELS_DEBUG
  modelsGUI->Delete ();
#endif
#ifndef FIDUCIALS_DEBUG
  fiducialsGUI->Delete ();
#endif

  ROIGUI->Delete ();

#ifndef COLORS_DEBUG
  colorGUI->Delete();
#endif


  transformsGUI->Delete ();

#ifndef CAMERA_DEBUG
  cameraGUI->Delete ();
#endif
  dataGUI->Delete ();
#ifndef SLICES_DEBUG
  slicesGUI->Delete ();
#endif

#if !defined(SCRIPTEDMODULE_DEBUG) && defined(Slicer3_BUILD_MODULES)
  tclCommand = "";
  tclCommand += "foreach package $::Slicer3_PACKAGES(list) { ";
  tclCommand += "  $::Slicer3_PACKAGES($package,gui) Delete;";
  tclCommand += "}";
  Slicer3_Tcl_Eval( interp, tclCommand.c_str() );
#endif

  // Release reference to applicaiton GUI
  // and delete it.
  slicerApp->SetApplicationGUI ( NULL );
  appGUI->DeleteComponentGUIs();
  //cout << "vtkSlicerApplicationGUI deleting app GUI\n";
  //   appGUI->Delete ();

#if !defined(COMMANDLINE_DEBUG) && (defined(Slicer3_BUILD_CLI) || defined(Slicer3_BUILD_MODULES))
  // delete the factory discovered module GUIs (as we delete the
  // GUIs, cache the associated logic instances so we can delete
  std::vector<vtkSlicerModuleGUI*>::iterator git;
  std::vector<vtkSlicerModuleLogic*> moduleLogics;
  for (git = moduleGUIs.begin(); git != moduleGUIs.end(); ++git)
    {
    moduleLogics.push_back(
      dynamic_cast<vtkCommandLineModuleGUI*>((*git))->GetLogic());

    (*git)->Delete();
    }
  moduleGUIs.clear();
#endif

  // Release reference to applicaiton GUI
  // and delete it.
  slicerApp->SetApplicationGUI ( NULL );
  appGUI->DeleteComponentGUIs();
  appGUI->Delete ();
    
  //--- delete logic next, removing Refs to MRML
  appLogic->ClearCollections ( );

  lmit = loadableModuleNames.begin();
  while (lmit != loadableModuleNames.end()) {
  LoadableModuleDescription desc = loadableModuleFactory.GetModuleDescription(*lmit);

  vtkSlicerModuleLogic* logic = desc.GetLogicPtr();

  logic->SetAndObserveMRMLScene( NULL );
  logic->Delete();

  lmit++;
  }

#if !defined(TRACTOGRAPHY_DEBUG) && defined(Slicer3_BUILD_MODULES)
  slicerTractographyDisplayLogic->SetAndObserveMRMLScene ( NULL );
  slicerTractographyDisplayLogic->Delete ();
#endif
        

#if !defined(VOLUMES_DEBUG) && defined(Slicer3_BUILD_MODULES)
  volumesLogic->SetAndObserveMRMLScene ( NULL );
  volumesLogic->Delete();
#endif
#ifndef MODELS_DEBUG
  modelsLogic->SetAndObserveMRMLScene ( NULL );
  modelsLogic->Delete();
    
  modelHierarchyLogic->SetAndObserveMRMLScene ( NULL );
  modelHierarchyLogic->Delete();
#endif
#ifndef FIDUCIALS_DEBUG
  fiducialsLogic->SetAndObserveMRMLScene ( NULL );
  fiducialsLogic->Delete();
#endif

  ROILogic->SetAndObserveMRMLScene ( NULL );
  ROILogic->Delete();

#ifndef COLORS_DEBUG
  colorLogic->SetAndObserveMRMLScene ( NULL );
  colorLogic->Delete();
#endif

  appLogic->DeleteSliceLogics();
  appLogic->SetAndObserveMRMLScene ( NULL );
  appLogic->TerminateProcessingThread();
  appLogic->Delete ();

#if !defined(COMMANDLINE_DEBUG) && (defined(Slicer3_BUILD_CLI) || defined(Slicer3_BUILD_MODULES))
  // delete the factory discovered module Logics
  std::vector<vtkSlicerModuleLogic*>::iterator lit;
  for (lit = moduleLogics.begin(); lit != moduleLogics.end(); ++lit)
    {
    (*lit)->SetAndObserveMRMLScene ( NULL );
    (*lit)->Delete();
    }
  moduleLogics.clear();
#endif


#if !defined(SCRIPTEDMODULE_DEBUG) && defined(Slicer3_BUILD_MODULES)
  // delete the scripted logics
  tclCommand = "";
  tclCommand += "foreach package $::Slicer3_PACKAGES(list) { ";
  tclCommand += "  $::Slicer3_PACKAGES($package,logic) SetAndObserveMRMLScene {};";
  tclCommand += "  $::Slicer3_PACKAGES($package,logic) Delete;";
  tclCommand += "}";
  Slicer3_Tcl_Eval( interp, tclCommand.c_str() );
#endif

  //--- application last
  slicerApp->Delete ();

  //--- scene next;
  scene->Clear(1);
  scene->Delete ();

  // -- event broker
  // - free up the reference from the interpeter
  // - free the actual singleton instance
  slicerApp->Script ("$::slicer3::Broker Delete");
  vtkEventBroker::GetInstance()->Delete(); 

  //--- application last
  slicerApp->Delete ();

#ifdef Slicer3_USE_PYTHON
  // Shutdown python interpreter
  Py_Finalize();
#endif


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

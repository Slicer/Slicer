/*=========================================================================

  Program:   Slicer
  Language:  C++
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

#include "vtkSlicerConfigure.h" /* Slicer_USE_* */

//---------------------------------------------------------------------------
// Slicer_USE_QT
//
#ifdef Slicer_USE_QT

// SlicerQT includes
#include "qSlicerApplication.h"
#include "qSlicerCLILoadableModuleFactory.h"
#include "qSlicerCLIExecutableModuleFactory.h"
#include "qSlicerCommandOptions.h"
#include "qSlicerCoreModuleFactory.h"
#include "qSlicerLoadableModuleFactory.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"
#include "qSlicerModulePanel.h"

// QT includes
#include <QStringList>
#include <QDebug>

// #include "vtkSlicerEmptyModuleGUI.h"
#endif

#include "vtkOpenGLRenderWindow.h"
#include "vtkRenderWindow.h"
#include "vtkDebugLeaks.h"

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
#include "vtkSlicerModelsLogic.h"
#include "vtkSlicerModelHierarchyLogic.h"
#include "vtkSlicerFiducialsLogic.h"
#include "vtkSlicerColorLogic.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLCrosshairNode.h"

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

#include "ModuleFactory.h"

#include "vtkSlicerROILogic.h"
#include "vtkSlicerROIGUI.h"

//---------------------------------------------------------------------------
// Slicer_USE_PYTHON
//
#ifdef Slicer_USE_PYTHON
#include "slicerPython.h"

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
//#define SCRIPTEDMODULE_DEBUG
//#define SLICESMODULE_DEBUG

//#define CAMERA_DEBUG

#include <LoadableModuleFactory.h>

#if !defined(SCRIPTEDMODULE_DEBUG) && defined(Slicer_BUILD_MODULES)
#include "vtkScriptedModuleLogic.h"
#include "vtkScriptedModuleGUI.h"
#endif

#if !defined(COMMANDLINE_DEBUG) && (defined(Slicer_BUILD_CLI) || defined(Slicer_BUILD_MODULES))
#include "vtkCommandLineModuleLogic.h"
#include "vtkCommandLineModuleGUI.h"
#endif

#if !defined(REMOTEIO_DEBUG)
#include "vtkHTTPHandler.h"
#include "vtkSRBHandler.h"
#include "vtkXNATHandler.h"
#include "vtkHIDHandler.h"
#include "vtkXNDHandler.h"
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
extern "C" int Mrmllogic_Init(Tcl_Interp *interp);
extern "C" int Mrmldisplayablemanager_Init(Tcl_Interp *interp);
extern "C" int Mrmlcli_Init(Tcl_Interp *interp);
extern "C" int Vtkitk_Init(Tcl_Interp *interp);
extern "C" int Freesurfer_Init(Tcl_Interp *interp);
extern "C" int Igt_Init(Tcl_Interp *interp);
extern "C" int Vtkteem_Init(Tcl_Interp *interp);


//TODO added temporary
#if !defined(DAEMON_DEBUG) && defined(Slicer_BUILD_MODULES)
extern "C" int Slicerdaemon_Init(Tcl_Interp *interp);
#endif
#if !defined(COMMANDLINE_DEBUG) && (defined(Slicer_BUILD_CLI) || defined(Slicer_BUILD_MODULES))
extern "C" int Commandlinemodule_Init(Tcl_Interp *interp);
#endif
#if !defined(SCRIPTEDMODULE_DEBUG) && defined(Slicer_BUILD_MODULES)
extern "C" int Scriptedmodule_Init(Tcl_Interp *interp);
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


int Slicer_Tcl_Eval ( Tcl_Interp *interp, const char *script )
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

void Slicer_BrokerScriptHandler ( const char *script, void * vtkNotUsed(dummy) )
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
#ifdef Slicer_USE_PYTHON
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

#if 0
static void DebugMessage(const char *msg)
{
  vtkSlicerApplication::GetInstance()->DebugMessage(msg);
}
#endif

static void SplashMessage(const char *msg)
{
  vtkSlicerApplication::GetInstance()->SplashMessage(msg);
}


int Slicer_main(int& argc, char *argv[])
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

  vtkDebugLeaks::SetExitError(true);

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

  std::string tmpName = slicerBinDir + "/../" + Slicer_LIB_DIR + "/SlicerBaseGUI/Tcl/Loader.tcl";
  if ( !vtksys::SystemTools::FileExists(tmpName.c_str()) )
    {
    // Handle Visual Studio IntDir
    std::vector<std::string> pathComponents;
    vtksys::SystemTools::SplitPath(slicerBinDir.c_str(), pathComponents);

    slicerBinDir = slicerBinDir + "/..";
    tmpName = slicerBinDir + "/../" + Slicer_LIB_DIR + "/SlicerBaseGUI/Tcl/Loader.tcl";
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

  // set the SLICER_HOME variable if it doesn't already exist from the launcher
  vtksys_stl::string slicerHome;
  if ( !vtksys::SystemTools::GetEnv("SLICER_HOME", slicerHome) )
    {
    slicerHome = slicerBinDir + "/..";
    slicerHome = vtksys::SystemTools::CollapseFullPath(slicerHome.c_str());
    std::string homeEnv = "SLICER_HOME=" + slicerHome;
    cout << "Set environment: " << homeEnv.c_str() << endl;
    vtkKWApplication::PutEnv(const_cast <char *> (homeEnv.c_str()));
    }

  std::string tclEnv = "TCL_LIBRARY=";
  tclEnv += slicerHome + "/lib/TclTk/lib/tcl" + Slicer_TCL_TK_MAJOR_VERSION + "." + Slicer_TCL_TK_MINOR_VERSION;
  vtkKWApplication::PutEnv(const_cast <char *> (tclEnv.c_str()));

  // ITK factories dir

  std::string slicerITKFactoriesDir;
  slicerITKFactoriesDir = slicerHome + "/" + Slicer_ITKFACTORIES_DIR;
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

  // explicitly request full numerical precision so floating
  // point numbers will go to/from VTK correctly for tcl and python
  // http://www.na-mic.org/Bug/view.php?id=668
  // http://wiki.tcl.tk/1650
  Slicer_Tcl_Eval(interp, "set ::tcl_precision 17");

  //
  // turn off hardware antialiasing by default
  // - the QueryAtlas picker relies on this
  //
  vtkOpenGLRenderWindow::SetGlobalMaximumNumberOfMultiSamples(0);

#ifdef Slicer_USE_PYTHON
  // Initialize Python

  // Set up the search path
  std::string pythonEnv = "PYTHONPATH=";

  const char* existingPythonEnv = vtksys::SystemTools::GetEnv("PYTHONPATH");
  if ( existingPythonEnv )
    {
    pythonEnv += std::string ( existingPythonEnv ) + PathSep;
    }

  pythonEnv += slicerHome + "/" + Slicer_LIB_DIR + "/SlicerBaseGUI/Python" + PathSep;
  pythonEnv += slicerHome + "/" + Slicer_CLIMODULES_BIN_DIR + PathSep;
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

  std::vector<std::string> pythonInitStrings;

  pythonInitStrings.push_back(std::string("import _tkinter;"));  
  pythonInitStrings.push_back(std::string("import Tkinter;"));
  pythonInitStrings.push_back(std::string("import sys;"));
  pythonInitStrings.push_back(std::string("from os.path import join as j;"));
  pythonInitStrings.push_back(std::string("tk = Tkinter.Tk();"));
  pythonInitStrings.push_back(std::string("sys.path.append ( j('" + slicerHome + "','" + Slicer_LIB_DIR + "', 'SlicerBaseGUI', 'Python')" + " );"));
  pythonInitStrings.push_back(std::string("sys.path.append ( j('" + slicerHome + "','" + Slicer_CLIMODULES_BIN_DIR + "') );"));

  /*
  std::string TkinitString = "import Tkinter, sys;"
    "from os.path import join as j;"
    "tk = Tkinter.Tk();"
    "sys.path.append ( j('"
    + slicerHome + "','" + Slicer_LIB_DIR + "', 'SlicerBaseGUI', 'Python')"
    + " );\n"
    "sys.path.append ( j('"
    + slicerHome + "','" + Slicer_CLIMODULES_BIN_DIR
    + "') );\n";
    */
  
  std::vector<std::string>::iterator strIt;
  strIt = pythonInitStrings.begin();
  for (; strIt != pythonInitStrings.end(); strIt++)
    {
    v = PyRun_String( (*strIt).c_str(),
                      Py_file_input,
                      PythonDictionary,
                      PythonDictionary );
    if (v == NULL)
      {
      PyObject *exception, *v, *tb;
      PyObject *exception_s, *v_s, *tb_s;

      PyErr_Fetch(&exception, &v, &tb);
      if (exception != NULL)
        {
        PyErr_NormalizeException(&exception, &v, &tb);
        if (exception != NULL)
          {
          exception_s = PyObject_Str(exception);
          v_s = PyObject_Str(v);
          tb_s = PyObject_Str(tb);
          const char *e_string, *v_string, *tb_string;
          cout << "Running: " << (*strIt).c_str() << endl;
          e_string = PyString_AS_STRING(exception_s);
          cout << "Exception: " << e_string << endl;
          v_string = PyString_AS_STRING(v_s);
          cout << "V: " << v_string << endl;
          tb_string = PyString_AS_STRING(PyObject_Str(tb_s));
          cout << "TB: " << tb_string << endl;
          Py_DECREF ( exception_s );
          Py_DECREF ( v_s );
          Py_DECREF ( tb_s );
          Py_DECREF ( exception );
          Py_DECREF ( v );
          if ( tb )
            {
            Py_DECREF ( tb );
            }
          }
        }

      PyErr_Print();
      }
    else
      {
      if (Py_FlushLine())
        {
        PyErr_Clear();
        }
      }
    }
#endif

  // TODO: get rid of fixed size buffer
  char cmd[2048];

  // Make sure SLICER_HOME is available

  sprintf(cmd, "set ::env(SLICER_HOME) {%s};", slicerHome.c_str());
  Slicer_Tcl_Eval(interp, cmd);

  sprintf(cmd, "set ::SlicerHome {%s};", slicerHome.c_str());
  Slicer_Tcl_Eval(interp, cmd);

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
      set ::Slicer_BIN [file dirname [info nameofexecutable]];       \
      if { $::tcl_platform(platform) == \"windows\"} {               \
        set ::Slicer_BUILD [file normalize $Slicer_BIN/..%s]         \
      } else {                                                       \
        set ::Slicer_BUILD [file normalize $Slicer_BIN/..]           \
      };                                                             \
    ", one_up.c_str());

  Slicer_Tcl_Eval( interp, cmd);

  PARSE_ARGS;  // relies on the header file auto generated by Slicer3.xml

  // shared library paths for dependent libraries must be added to the
  // environment before the program is launched.  This flag allows
  // the launcher to query the module paths before actually launching
  // the executable to run
  if (PrintModulePaths)
    {
    vtkSlicerApplication *slicerApp = vtkSlicerApplication::GetInstance ( );
    slicerApp->SetRegistryLevel(0);
    slicerApp->PromptBeforeExitOff();
    std::cout << slicerApp->GetModulePaths() << std::endl;
    slicerApp->SetExitStatus(0);
    slicerApp->Exit();
    slicerApp->Delete ();
    exit(0);
    }

  if (VTKDebugOn)
    {
    vtkObject::GlobalWarningDisplayOn();
    }

  //
  // load itcl package (needed for interactive widgets)
  //
  {
  std::string tclCmd;
  int returnCode;

  // Pass arguments to the Tcl script
  tclCmd =  "package require Itcl;";
  returnCode = Slicer_Tcl_Eval( interp, tclCmd.c_str() );
  if ( returnCode )
    {
    slicerCerr("Error: slicer requires the Itcl package (" << tclCmd.c_str() << ")" << endl);
    return ( returnCode );
    }
  }

  //
  // load the gui tcl package (for the interactive widgets)
  //
  {
  std::string tclCmd;
  int returnCode;

  // Pass arguments to the Tcl script
  tclCmd =  "lappend auto_path \"" + slicerHome + "/"
    Slicer_LIB_DIR + "/SlicerBaseGUI/Tcl\"; ";
  //cmd += "puts $auto_path; ";
  tclCmd += "package require SlicerBaseGUITcl; ";
  returnCode = Slicer_Tcl_Eval( interp, tclCmd.c_str() );
  if ( returnCode )
    {
    slicerCerr("Load SlicerBaseGUITcl: " << tclCmd.c_str() << endl);
    return ( returnCode );
    }
  }


  //
  // load the custom icons
  //
#ifdef _WIN32
  {
  std::string tclCmd;
  int returnCode;

  tclCmd =  "wm iconbitmap . -default \""+ slicerHome + "/" + Slicer_SHARE_DIR + "/Resources/slicer3.ico\"";
  returnCode = Slicer_Tcl_Eval( interp, tclCmd.c_str() );
  if ( returnCode )
    {
    slicerCerr("Load Custom Icons: " << tclCmd.c_str() << endl);
    }
  }
#endif


  //
  // Initialize our Tcl library (i.e. our classes wrapped in Tcl)
  //
  Slicerbasegui_Init(interp);
  Slicerbaselogic_Init(interp);
  Mrml_Init(interp);
  Mrmlcli_Init(interp);
  Mrmllogic_Init(interp);
  Mrmldisplayablemanager_Init(interp);
  Vtkitk_Init(interp);
  Freesurfer_Init(interp);
  Igt_Init(interp);
  Vtkteem_Init(interp);

#if !defined(DAEMON_DEBUG) && defined(Slicer_BUILD_MODULES)
  Slicerdaemon_Init(interp);
#endif
#if !defined(COMMANDLINE_DEBUG) && (defined(Slicer_BUILD_CLI) || defined(Slicer_BUILD_MODULES))
  Commandlinemodule_Init(interp);
#endif
#if !defined(SCRIPTEDMODULE_DEBUG) && defined(Slicer_BUILD_MODULES)
  Scriptedmodule_Init(interp);
#endif


  // first call to GetInstance will create the Application
  //
  // make a substitute tcl proc for the 'exit' built in that
  // goes through the vtkKWApplication methodology for exiting
  // cleanly
  //
  vtkSlicerApplication *slicerApp = vtkSlicerApplication::GetInstance ( );

  slicerApp->SetBinDir(slicerBinDir.c_str());

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
  std::string tclCmd, slicerAppName;

  slicerAppName = slicerApp->GetTclName();

  slicerApp->Script ("namespace eval slicer3 set Application %s", slicerAppName.c_str());

  tclCmd = "rename exit tcl_exit; ";

  tclCmd +=
    "proc exit {args} { \
      if { $args != {} && [string is integer $args] == \"1\" } { \
        " + slicerAppName + " SetExitStatus $args \
      } else { \
        " + slicerAppName + " SetExitStatus 0 \
      } ;\
      after idle {" + slicerAppName + " Exit}; \
    }";
  Slicer_Tcl_Eval( interp, tclCmd.c_str() );
  }

#ifdef Slicer_USE_QT
  static int slicerAppArgc = 1;
  qSlicerApplication* qApplication = new qSlicerApplication(slicerAppArgc, argv);
  Q_CHECK_PTR(qApplication);

  qApplication->setCoreCommandOptions(
    new qSlicerCommandOptions(qApplication->settings()));

  // Set window flags used to display top level widgets
#ifdef Q_WS_MAC
  qApplication->setDefaultWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool);
#elif defined(Q_WS_WIN)
  qApplication->setDefaultWindowFlags(
    Qt::WindowStaysOnTopHint | Qt::Tool | Qt::FramelessWindowHint | Qt::WindowTitleHint);
#else
  qApplication->setDefaultWindowFlags(
    Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
#endif
  slicerApp->SetModulePanel(new qSlicerModulePanel(0, qApplication->defaultWindowFlags()));
 #ifdef Slicer_USE_PYTHONQT
  PythonQt::init(PythonQt::DoNotInitializePython);
  PythonQt_QtAll::init();
 #endif
#endif

  //
  // ignore any modules specified on the command line
  //
  vtkSmartPointer<vtkStringArray> ignoreModules = vtkSmartPointer<vtkStringArray>::New();
  ignoreModules->DeepCopy(slicerApp->GetIgnoreModules());
  std::vector<std::string>::const_iterator ignoreit = IgnoreModule.begin();
  while (ignoreit != IgnoreModule.end())
    {
    ignoreModules->InsertNextValue((*ignoreit).c_str());
    ++ignoreit;
    }


  //
  // use the startup script passed on command line if it exists
  //
  if ( File != "" )
    {

    std::string tclCmd;

    // Pass arguments to the Tcl script
    tclCmd = "set args \"\"; ";
    std::vector<std::string>::const_iterator argit = Args.begin();
    while (argit != Args.end())
      {
      tclCmd += " lappend args \"" + *argit + "\"; ";
      ++argit;
      }
    Slicer_Tcl_Eval( interp, tclCmd.c_str() );

    tclCmd = "source " + File;
    int returnCode = Slicer_Tcl_Eval( interp, tclCmd.c_str() );
    Slicer_Tcl_Eval( interp, "update" );
    slicerApp->Delete();
    return ( returnCode );
    }

  //
  // use the startup code passed on command line if it exists
  //
  if ( Eval != "" )
    {
    std::string tclCmd = "set ::SLICER(eval) \"" + Eval + "\" ; ";
    tclCmd += "regsub -all {\\.,} $::SLICER(eval) \";\" ::SLICER(eval); ";
    tclCmd += "regsub -all {,\\.} $::SLICER(eval) \";\" ::SLICER(eval); ";
    tclCmd += "eval $::SLICER(eval);";
    int returnCode = Slicer_Tcl_Eval( interp, tclCmd.c_str() );
    Slicer_Tcl_Eval( interp, "update" );
    slicerApp->Delete();
    return ( returnCode );
    }

  // Create SlicerGUI application, style, and main window
  //  - note: use the singleton application
  slicerApp->InstallTheme( slicerApp->GetSlicerTheme() );

  // copy the image from the header file into memory

  unsigned char *buffer =
    new unsigned char [image_SlicerSplashScreen_length];

  unsigned int i;
  unsigned char *curPos = buffer;
  for (i = 0; i < image_SlicerSplashScreen_nb_sections; i++)
    {
    size_t len = strlen((const char*)image_SlicerSplashScreen_sections[i]);
    memcpy(curPos, image_SlicerSplashScreen_sections[i], len);
    curPos += len;
    }

  vtkKWTkUtilities::UpdatePhoto(
    slicerApp->GetMainInterp(),
    "SlicerSplashScreen",
    buffer,
    image_SlicerSplashScreen_width,
    image_SlicerSplashScreen_height,
    image_SlicerSplashScreen_pixel_size,
    image_SlicerSplashScreen_length);
  delete [] buffer;

  slicerApp->GetSplashScreen()->SetImageName("SlicerSplashScreen");
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

  if (!TestMode)
    {
    cout << "Starting Slicer: " << slicerHome.c_str() << endl;
    }

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
  vtkEventBroker::GetInstance()->SetScriptHandler( Slicer_BrokerScriptHandler, NULL );
  vtkEventBroker::GetInstance()->SetEventModeToSynchronous();

  if ( EventLog != "" )
    {
    vtkEventBroker::GetInstance()->SetLogFileName(EventLog.c_str());
    vtkEventBroker::GetInstance()->SetEventLogging(1);
    }


  // Create MRML scene
  vtkMRMLScene *scene = vtkMRMLScene::New();
  vtksys_stl::string root = vtksys::SystemTools::GetCurrentWorkingDirectory();
  scene->SetRootDirectory(root.c_str());
  vtkMRMLScene::SetActiveScene( scene );
  slicerApp->SetMRMLScene( scene );
#ifdef Slicer_USE_QT
  qSlicerApplication::application()->setMRMLScene( scene );
#endif

  // Create the application Logic object,
  vtkSlicerApplicationLogic *appLogic = vtkSlicerApplicationLogic::New( );
  appLogic->SetMRMLScene ( scene );
  // pass through event handling once without observing the scene
  // -- allows any dependent nodes to be created
  appLogic->ProcessMRMLEvents (scene, vtkCommand::ModifiedEvent, NULL);
  appLogic->SetAndObserveMRMLScene ( scene );
  appLogic->CreateProcessingThread();

#ifdef Slicer_USE_QT
  qSlicerApplication::application()->setAppLogic( appLogic );
  qSlicerApplication::application()->setInitialized(true);

  // Instanciate module manager
  qSlicerApplication::application()->setModuleManager(new qSlicerModuleManager);
  qSlicerModuleFactoryManager * moduleFactoryManager =
      qSlicerApplication::application()->moduleManager()->factoryManager();

  // Register module factories
  moduleFactoryManager->registerFactory("qSlicerCoreModuleFactory",
                                        new qSlicerCoreModuleFactory());
  moduleFactoryManager->registerFactory("qSlicerLoadableModuleFactory",
                                        new qSlicerLoadableModuleFactory());
  moduleFactoryManager->registerFactory("qSlicerCLILoadableModuleFactory",
                                        new qSlicerCLILoadableModuleFactory());
  moduleFactoryManager->registerFactory("qSlicerCLIExecutableModuleFactory",
                                        new qSlicerCLIExecutableModuleFactory());
#endif

  slicerApp->SplashMessage("Creating Application GUI...");

  // Create the application GUI object and have it observe the Logic
  // CREATE APPLICATION GUI, including the main window
  vtkSlicerApplicationGUI *appGUI = vtkSlicerApplicationGUI::New ( );
  appGUI->SetApplication ( slicerApp );
  appGUI->SetAndObserveApplicationLogic ( appLogic );
  appGUI->SetAndObserveMRMLScene( scene ); // observers added in appGUI->BuildGUI

  slicerApp->Script ("update");
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
  httpHandler->SetName ( "HTTPHandler");
  scene->AddURIHandler(httpHandler);
  httpHandler->Delete();

  vtkSRBHandler *srbHandler = vtkSRBHandler::New();
  srbHandler->SetPrefix ( "srb://" );
  srbHandler->SetName ( "SRBHandler" );
  scene->AddURIHandler(srbHandler);
  srbHandler->Delete();

  vtkXNATHandler *xnatHandler = vtkXNATHandler::New();
  vtkSlicerXNATPermissionPrompterWidget *xnatPermissionPrompter = vtkSlicerXNATPermissionPrompterWidget::New();
  xnatPermissionPrompter->SetApplication ( slicerApp );
  xnatPermissionPrompter->SetPromptTitle ("Permission Prompt");
  xnatHandler->SetPrefix ( "xnat://" );
  xnatHandler->SetName ( "XNATHandler" );
  xnatHandler->SetRequiresPermission (1);
  xnatHandler->SetPermissionPrompter ( xnatPermissionPrompter );
  scene->AddURIHandler(xnatHandler);
  xnatPermissionPrompter->Delete();
  xnatHandler->Delete();

  vtkHIDHandler *hidHandler = vtkHIDHandler::New();
  hidHandler->SetPrefix ( "hid://" );
  hidHandler->SetName ( "HIDHandler" );
  scene->AddURIHandler( hidHandler);
  hidHandler->Delete();

  vtkXNDHandler *xndHandler = vtkXNDHandler::New();
  xndHandler->SetPrefix ( "xnd://" );
  xndHandler->SetName ( "XNDHandler" );
  scene->AddURIHandler( xndHandler);
  xndHandler->Delete();

  //add something to hold user tags
  vtkTagTable *userTagTable = vtkTagTable::New();
  scene->SetUserTagTable( userTagTable );
  userTagTable->Delete();

#endif

#if !defined(SLICESMODULE_DEBUG)
  vtkSlicerSlicesGUI *slicesGUI = vtkSlicerSlicesGUI::New ();

  // build the application GUI
  appGUI->SetSlicesGUI(slicesGUI);

  appGUI->BuildGUI ( );
  appGUI->AddGUIObservers ( );
  slicerApp->SetApplicationGUI ( appGUI );
  slicerApp->ConfigureRemoteIOSettingsFromRegistry();


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

  bool FullDisk = false;
  //--- perform a check for a
  if ( slicerApp->FullFileSystemCheck() )
    {
    FullDisk = true;
    //--- if there's no room on disk,
    //--- ditch building anything that adds to cache.
    NoModules = true;
    }

  // get the module paths that the user has configured
  std::string userModulePaths;
  if (slicerApp->GetModulePaths())
    {
    userModulePaths = slicerApp->GetModulePaths();

    std::string token("@SVN@");
    std::string::size_type pos = userModulePaths.find(token);
    while (std::string::npos != pos)
      {
      userModulePaths.erase(pos, token.length());
      userModulePaths.insert(pos, slicerApp->GetSvnRevision());

      pos = userModulePaths.find(token, pos);
      }
    }

  // define module paths based on the slicer installation
  // or build tree.

  std::string modulePaths;
  std::string defaultModulePaths;

  // On Win32, *both* paths have to be there, since scripts are installed
  // in the install location, and exec/libs are *automatically* installed
  // in intDir.
  defaultModulePaths = slicerHome + "/" + Slicer_MODULES_LIB_DIR;
  if (hasIntDir)
    {
    defaultModulePaths = defaultModulePaths + PathSep +
      slicerHome + "/" + Slicer_MODULES_LIB_DIR + "/" + intDir;
    }

  // add the default modules directory (based on the slicer
  // installation or build tree) to the user paths
  modulePaths = userModulePaths + PathSep + defaultModulePaths;

  // =================== vvv
  // TODO: this is moved to the launcher since setting it at run
  // time has no effect on dlopen
  //
  //
  // add module path to the shared library load path so that modules can
  // be executed when they depend on their on libs
#ifdef __APPLE__
  const char *pathVar = "DYLD_LIBRARY_PATH";
#elif _WIN32
  const char *pathVar = "PATH";
#else
  const char *pathVar = "LD_LIBRARY_PATH";
#endif

  std::string sharedLibPath;
  vtksys::SystemTools::GetEnv(pathVar, sharedLibPath);
  sharedLibPath = std::string(pathVar) + "=" + sharedLibPath + PathSep + modulePaths;
  vtkKWApplication::PutEnv(const_cast <char *> (sharedLibPath.c_str()));

  // =================== ^^^

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
  defaultPluginsPaths = slicerHome + "/" + Slicer_CLIMODULES_BIN_DIR;
  if (hasIntDir)
    {
    defaultPluginsPaths = defaultPluginsPaths + PathSep +
      slicerHome + "/" + Slicer_CLIMODULES_BIN_DIR + "/" + intDir;
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
  defaultCachePath = std::string(slicerApp->GetTemporaryDirectory()) + "/" + Slicer_PLUGINS_CACHE_DIR;
  if (hasIntDir)
    {
    defaultCachePath += "/" + intDir;
    }

  // get the ext. install path that the user has configured
  if (slicerApp->GetExtensionsInstallPath())
    {
    userCachePath = slicerApp->GetExtensionsInstallPath();
    }
  slicerApp->SetExtensionsInstallPath(userCachePath.c_str());

  if ( FullDisk == false)
    {
    // if user cache path is set and we can write to it, use it.
    // if user cache path is not set or we cannot write to it, try
    // the default cache path.
    // if we cannot write to the default cache path, then warn and
    // don't use a cache.
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
#ifdef Slicer_USE_QT
    moduleFactoryManager->registerModules("qSlicerLoadableModuleFactory");
    moduleFactoryManager->instantiateModules("qSlicerLoadableModuleFactory");
#endif
    }

  std::vector<std::string> loadableModuleNames =
    loadableModuleFactory.GetModuleNames();
  std::vector<std::string>::const_iterator lmit =
    loadableModuleNames.begin();

  vtkSmartPointer<vtkStringArray> loadableModules =
    vtkSmartPointer<vtkStringArray>::New();

  while (lmit != loadableModuleNames.end())
    {
    LoadableModuleDescription desc = loadableModuleFactory.GetModuleDescription(*lmit);
    loadableModules->InsertNextValue(desc.GetName());
#ifdef Slicer_USE_QT
    qDebug() << "Initialize loadable module:" << desc.GetName().c_str();
#endif
    if (ignoreModules->LookupValue(desc.GetName().c_str()) < 0)
      {
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

      if ( logic )
        {
        logic->SetAndObserveMRMLScene( scene );
        logic->SetModuleName(desc.GetShortName().c_str());

        vtkIntArray* events = logic->NewObservableEvents();
        if ( events != NULL )
          {
          logic->SetAndObserveMRMLSceneEvents(scene, events);
          events->Delete();
          }

        logic->SetApplicationLogic(appLogic);
        logic->SetModuleLocation(desc.GetLocation().c_str());

        if ( gui )
          {
          events = gui->NewObservableEvents();
          if (events != NULL)
            {
            gui->SetAndObserveMRMLSceneEvents(scene, events);
            events->Delete();
            }

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
          }
        }
      }
    lmit++;
    }

  slicerApp->SetLoadableModules(loadableModules);

  // ADD INDIVIDUAL MODULES
  // (these require appGUI to be built):

#ifndef MODELS_DEBUG

#ifdef Slicer_USE_QT
  moduleFactoryManager->registerModules("qSlicerCoreModuleFactory");
  moduleFactoryManager->instantiateModules("qSlicerCoreModuleFactory");
#endif

  SlicerQDebug("Initializing Models Module");
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
  if (  appGUI->GetActiveViewerWidget() )
    {
    appGUI->GetActiveViewerWidget()->SetModelHierarchyLogic(modelHierarchyLogic);
    }
  modelsGUI->SetGUIName( "Models" );
  modelsGUI->GetUIPanel()->SetName ( modelsGUI->GetGUIName ( ) );
  modelsGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWindow()->GetMainUserInterfaceManager ( ) );
  modelsGUI->GetUIPanel()->Create ( );
  slicerApp->AddModuleGUI ( modelsGUI );
#endif


#ifndef FIDUCIALS_DEBUG
  SlicerQDebug("Initializing Fiducials Module");
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
  SlicerQDebug("Initializing ROI Module");
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
  SlicerQDebug("Initializing Colors Module");

  // -- Color module
  vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New ( );
  // observe the scene's new scene event
  vtkIntArray *colorEvents = vtkIntArray::New();
  colorEvents->InsertNextValue( vtkMRMLScene::NewSceneEvent );
  colorEvents->InsertNextValue( vtkMRMLScene::SceneClosedEvent );
  colorLogic->SetAndObserveMRMLSceneEvents ( scene,  colorEvents);
  colorEvents->Delete();
  // set the user defined color paths, it will be used by the add default
  // color nodes call
  colorLogic->SetUserColorFilePaths(slicerApp->GetColorFilePaths());
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
  SlicerQDebug("Initializing Transforms Module");

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
  SlicerQDebug("Initializing Data Module");

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
  // Set up observers here so appGUI will be notified from menu
  // events in the tree widget
  dataGUI->AddObserver(vtkSlicerModuleGUI::ModuleSelectedEvent, (vtkCommand *)appGUI->GetGUICallbackCommand());

#ifndef CAMERA_DEBUG
  slicerApp->SplashMessage("Initializing Camera Module...");
  SlicerQDebug("Initializing Camera Module");

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
  SlicerQDebug("Initializing Slices Module");
  appLogic->CreateSliceLogics();
  appGUI->InitializeSlicesControlGUI();
  appGUI->InitializeViewControlGUI();
  //    appGUI->InitializeNavigationWidget();

#ifdef Slicer_USE_QT
//   // --- Empty module
//   slicerApp->SplashMessage("Initializing Empty Module...");
//
//   vtkSlicerEmptyModuleGUI *emptyModuleGUI = vtkSlicerEmptyModuleGUI::New ( );
//   emptyModuleGUI->SetApplication ( slicerApp );
//   emptyModuleGUI->SetApplicationGUI ( appGUI );
//   emptyModuleGUI->SetAndObserveApplicationLogic ( appLogic );
//   emptyModuleGUI->SetAndObserveMRMLScene ( scene );
//   emptyModuleGUI->SetGUIName( "EmptyModule" );
//   emptyModuleGUI->GetUIPanel()->SetName ( emptyModuleGUI->GetGUIName ( ) );
//   emptyModuleGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWindow()->GetMainUserInterfaceManager ( ) );
//   //emptyModuleGUI->GetUIPanel()->Create ( );
//   slicerApp->AddModuleGUI ( emptyModuleGUI );

  // --- Initialize Qt modules
  slicerApp->InitializeQtCoreModules();
  slicerApp->InitializeQtLoadableModules();
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

#if !defined(DAEMON_DEBUG) && defined(Slicer_BUILD_MODULES)
  //
  // --- SlicerDaemon Module
  // need to source the slicerd.tcl script here
  // - only start if selected on command line or enabled in registry, since
  //   windows firewall will complain.
  //

  if ( Daemon || slicerApp->GetEnableDaemon() )
    {
    slicerApp->SplashMessage("Initializing Slicer Daemon...");
    std::string tclCmd;
    tclCmd =  "source \"" + slicerHome + "/"
      Slicer_MODULES_LIB_DIR "/SlicerDaemon/Tcl/slicerd.tcl\"; slicerd_start; ";
    Slicer_Tcl_Eval(interp, tclCmd.c_str());
    }
#endif

#if !defined(COMMANDLINE_DEBUG) && (defined(Slicer_BUILD_CLI) || defined(Slicer_BUILD_MODULES))
  std::vector<std::string> moduleNames;
  std::vector<std::string>::const_iterator mit;
  if ( slicerApp->GetLoadCommandLineModules()
        && !NoModules
        && (ignoreModules->LookupValue("CLI") < 0) )
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
    // register a program that can execute tcl scripts (used first to wrap
    // java)
    // find the tcl bin dir first
    vtksys_stl::string tclBuild;
    vtksys::SystemTools::GetEnv("TCL_DIR", tclBuild);
    tclBuild += std::string("/bin");
    std::string tclBinDir = vtksys::SystemTools::CollapseFullPath(tclBuild.c_str());
#ifdef _WIN32
    moduleFactory.RegisterFileExtension(".tcl", "%s/tclsh.exe", tclBinDir.c_str());
#else
    std::string tclexec = std::string("%s/tclsh") + Slicer_TCL_TK_MAJOR_VERSION + "." + Slicer_TCL_TK_MINOR_VERSION;
    moduleFactory.RegisterFileExtension(".tcl", tclexec.c_str(), tclBinDir.c_str());
#endif
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
#ifdef Slicer_USE_QT
    moduleFactoryManager->registerModules("qSlicerCLILoadableModuleFactory");
    moduleFactoryManager->instantiateModules("qSlicerCLILoadableModuleFactory");

    moduleFactoryManager->registerModules("qSlicerCLIExecutableModuleFactory");
    moduleFactoryManager->instantiateModules("qSlicerCLIExecutableModuleFactory");
#endif

    // Register the node type for the command line modules
    vtkMRMLCommandLineModuleNode* clmNode = vtkMRMLCommandLineModuleNode::New();
    scene->RegisterNodeClass(clmNode);
    clmNode->Delete();

    // add the modules to the available modules
    moduleNames = moduleFactory.GetModuleNames();
    mit = moduleNames.begin();
    while (mit != moduleNames.end())
      {
      ModuleDescription desc = moduleFactory.GetModuleDescription(*mit);
      // slicerCerr(desc << endl);
#ifdef Slicer_USE_QT
      //qDebug() << "Initialize command line module:" << mit->c_str()
      //         << "/" << desc.GetTitle().c_str()
      //         << "/" <<desc.GetLocation().c_str() ;
#endif

      vtkCommandLineModuleGUI *commandLineModuleGUI
        = vtkCommandLineModuleGUI::New();
      vtkCommandLineModuleLogic *commandLineModuleLogic
        = vtkCommandLineModuleLogic::New ( );

      // set up stream redirection
      if (slicerApp->GetRedirectModuleStreams() == 0 ||
          NoModuleStreamsRedirect)
        {
        commandLineModuleLogic->RedirectModuleStreamsOff();
        }
      else
        {
        commandLineModuleLogic->RedirectModuleStreamsOn();
        }

      // set up deleting temporary files
      if (slicerApp->GetDeleteTemporaryFiles() == 0 ||
          NoDeleteTemporaryFiles)
        {
        commandLineModuleLogic->DeleteTemporaryFilesOff();
        }
      else
        {
        commandLineModuleLogic->DeleteTemporaryFilesOn();
        }
      // Set the ModuleDescripton on the gui
      commandLineModuleGUI->SetModuleDescription( desc );

      // Register the module description in the master list
      vtkMRMLCommandLineModuleNode::RegisterModuleDescription( desc );

      // Configure the Logic, GUI, and add to app
      commandLineModuleLogic->SetAndObserveMRMLScene ( scene );
      commandLineModuleLogic->SetApplicationLogic (appLogic);
      commandLineModuleGUI->SetLogic ( commandLineModuleLogic );
      commandLineModuleGUI->SetApplication ( slicerApp );
      commandLineModuleGUI->SetApplicationLogic ( appLogic );
      commandLineModuleGUI->SetApplicationGUI ( appGUI );
      commandLineModuleGUI->SetGUIName( desc.GetTitle().c_str() );
      commandLineModuleGUI->GetUIPanel()->SetName ( commandLineModuleGUI->GetGUIName ( ) );
      commandLineModuleGUI->GetUIPanel()->SetUserInterfaceManager (appGUI->GetMainSlicerWindow()->GetMainUserInterfaceManager ( ) );
      commandLineModuleGUI->GetUIPanel()->Create ( );
      slicerApp->AddModuleGUI ( commandLineModuleGUI );

      ++mit;
      }

    }

#ifdef Slicer_USE_QT
  slicerApp->InitializeQtCommandLineModules();
#endif

#endif // CLIMODULES_DEBUG

  // Load any default parameter sets
  //
  //
  vtkSlicerModuleLogic::LoadDefaultParameterSets(scene);

  //
  // get the Tcl name so the vtk class will be registered in the interpreter
  // as a byproduct
  // - set some handy variables so it will be easy to access these classes
  //  from the tkcon
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

    if (ignoreModules->LookupValue(desc.GetName().c_str()) < 0)
      {

      vtkSlicerModuleGUI* gui = desc.GetGUIPtr();

      if ( gui )
        {
        name = gui->GetTclName();
        std::string format("namespace eval slicer3 set ");
        format += desc.GetShortName();
        format += "GUI %s";

        slicerApp->Script (format.c_str(), name);
        }

      }
    lmit++;
    }


#ifndef REMOTEIO_DEBUG
  name = remoteIOGUI->GetTclName();
  slicerApp->Script ("namespace eval slicer3 set RemoteIOGUI %s", name);
#endif

#if !defined(SLICESMODULE_DEBUG)
  name = slicesGUI->GetTclName();
  slicerApp->Script ("namespace eval slicer3 set SlicesGUI %s", name);
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

  if ( appGUI->GetActiveViewerWidget() )
    {
    name = appGUI->GetActiveViewerWidget()->GetTclName();
    slicerApp->Script ("namespace eval slicer3 set ViewerWidget %s", name);
    }

  slicerApp->Script ("namespace eval slicer3 set ApplicationLogic [$::slicer3::ApplicationGUI GetApplicationLogic]");
  slicerApp->Script ("namespace eval slicer3 set MRMLScene [$::slicer3::ApplicationLogic GetMRMLScene]");

  // Get the global event broker (same as returned by vtkEventBroker::GetInstance()
  // - since the singleton method is not exposed to tcl, access it this way
  //   and then delete it at the end.
  slicerApp->Script ("namespace eval slicer3 set Broker [vtkEventBroker New]");
  //slicerApp->Script ("namespace eval slicer3 set Broker [vtkEventBroker GetInstance]");

#if !defined(COMMANDLINE_DEBUG) && (defined(Slicer_BUILD_CLI) || defined(Slicer_BUILD_MODULES))
  mit = moduleNames.begin();
  while ( mit != moduleNames.end() )
    {
    vtkSlicerModuleGUI *module;
    module = slicerApp->GetModuleGUIByName( (*mit).c_str() );
    if (module)
      {
      name = module->GetTclName();
      std::string title = *mit;
      std::transform(
        title.begin(), title.end(), title.begin(), SpacesToUnderscores());

      slicerApp->Script ("namespace eval slicer3 set CommandLineModuleGUI_%s %s", title.c_str(), name);
      }

    ++mit;
    }
#endif

#if !defined(SCRIPTEDMODULE_DEBUG) && defined(Slicer_BUILD_MODULES)
  //
  // process any ScriptedModules
  // - scan for pkgIndex.tcl files
  // - load the corresponding packages
  // - create scripted logic and scripted gui instances for the module
  // - have the GUI construct itself
  //

  std::string tclCommand;
  if ( !NoModules && (ignoreModules->LookupValue("Scripted") < 0) )
    {
    slicerApp->SplashMessage("Initializing Scripted Modules...");
    tclCommand = "set ::Slicer_PACKAGES(list) {};";

    module_paths_it = modulePathsList.begin();
    for (; module_paths_it != module_paths_end; module_paths_it++)
      {
      vtksys_stl::string module_path(*module_paths_it);
      vtksys::SystemTools::ConvertToUnixSlashes(module_path);
      if (*module_path.c_str() &&
          vtksys::SystemTools::FileExists(module_path.c_str()))
        {
        tclCommand += "set dirs [glob -nocomplain \"" + module_path + "/*\"]; ";
        tclCommand += "foreach d $dirs { ";
        tclCommand += "  foreach subdir {\".\" \"Tcl\"} {";
        tclCommand += "    set location [file join [file join $d $subdir] pkgIndex.tcl];";
        tclCommand += "    if { [file exists $location] } {";
        tclCommand += "      set package [file tail $d];";
        tclCommand += "      if { [lsearch -exact $::Slicer_PACKAGES(list) $package] == -1 } {";
        tclCommand += "        set ::Slicer_PACKAGES($package,location) $location;";
        tclCommand += "        lappend ::Slicer_PACKAGES(list) $package;";
        tclCommand += "        lappend ::auto_path $d;";
        tclCommand += "      }";
        tclCommand += "    }";
        tclCommand += "  }";
        tclCommand += "}; ";
        }
      }
    Slicer_Tcl_Eval( interp, tclCommand.c_str());

    tclCommand = "";
    tclCommand += "foreach package $::Slicer_PACKAGES(list) { ";
    tclCommand += "  set ret [catch \"package require $package\" res];";
    tclCommand += "  if { $ret } { ";
    tclCommand += "    puts $res;";
    tclCommand += "  } else {";
    tclCommand += "    set ::Slicer_PACKAGES($package,logic) [vtkScriptedModuleLogic New];";
    tclCommand += "    set logic $::Slicer_PACKAGES($package,logic);";
    tclCommand += "    $logic SetModuleName $package;";
    tclCommand += "    $logic SetModuleLocation $::Slicer_PACKAGES($package,location);";
    tclCommand += "    $logic SetAndObserveMRMLScene $::slicer3::MRMLScene;";
    tclCommand += "    $logic SetApplicationLogic $::slicer3::ApplicationLogic;";
    tclCommand += "    set ::Slicer_PACKAGES($package,gui) [vtkScriptedModuleGUI New];";
    tclCommand += "    set gui $::Slicer_PACKAGES($package,gui);";
    tclCommand += "    $gui SetModuleName $package;";
    tclCommand += "    $gui SetLogic $logic;";
    tclCommand += "    $gui SetApplicationGUI $::slicer3::ApplicationGUI;";
    tclCommand += "    $gui SetApplication $::slicer3::Application;";
    tclCommand += "    $gui SetGUIName $package;";
    tclCommand += "    [$gui GetUIPanel] SetName $package;";
    tclCommand += "    [$gui GetUIPanel] SetUserInterfaceManager [[$::slicer3::ApplicationGUI GetMainSlicerWindow] GetMainUserInterfaceManager];";
    tclCommand += "    [$gui GetUIPanel] Create;";
    tclCommand += "    $::slicer3::Application AddModuleGUI $gui;" ;
    tclCommand += "    $gui BuildGUI;";
    tclCommand += "    $gui AddGUIObservers;";
    tclCommand += "  };";
    tclCommand += "";
    tclCommand += "}";
    Slicer_Tcl_Eval( interp, tclCommand.c_str() );
    }

#ifdef Slicer_USE_PYTHON

  vtkSlicerApplication::GetInstance()->InitializePython(
    (void*)PythonModule, (void*)PythonDictionary);

  if ( !NoModules )
    {
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
    pythonCommand += "import SlicerScriptedModule\n";
    pythonCommand += "SlicerScriptedModuleInfo = SlicerScriptedModule.SlicerScriptedModuleImporter(packageNames)\n";
    pythonCommand += "SlicerScriptedModuleInfo.ScanAndInitModules()\n";
    pythonCommand += "Slicer.ScriptedModuleInfo = SlicerScriptedModuleInfo\n";

    v = PyRun_String( pythonCommand.c_str(),
                      Py_file_input,
                      PythonDictionary,PythonDictionary);
    if (v == NULL)
      {
      PyErr_Print();
      }
    }
#endif
#endif

  if ( EvalPython != "" )
    {
#if defined(Slicer_USE_PYTHON)
    if ( !NoModules )
      {
      std::string pythonCmd = EvalPython + "\n";
      v = PyRun_String( EvalPython.c_str(),
                        Py_file_input,
                        PythonDictionary,PythonDictionary);
      if (v == NULL)
        {
        PyErr_Print();
        }
      slicerApp->SetExitStatus(v == NULL ? EXIT_FAILURE : EXIT_SUCCESS);
      slicerApp->Exit();
      slicerApp->Delete();
      exit(v == NULL ? EXIT_FAILURE : EXIT_SUCCESS);
      }
#endif
    }

  //
  // create the three main slice viewers after slicesGUI is created
  //
  slicerApp->SplashMessage("Configuring Slices and Modules...");
  appGUI->PopulateModuleChooseList ( );
  appGUI->SetAndObserveMainSliceLogic ( appLogic->GetSliceLogic("Red"), appLogic->GetSliceLogic("Yellow"), appLogic->GetSliceLogic("Green") );
  appGUI->ConfigureMainSliceViewers ( );

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


  //
  //--- set home module based on registry settings
  // - show welcome unless disabled or not there
  // - show user's home if set
  // - show Data if nothing else
  // - don't set if in test mode or --no-modules
  //
  const char *homeModule = slicerApp->GetHomeModule();
  if ( ( slicerApp->GetUseWelcomeModuleAtStartup() ) &&
       ( slicerApp->GetModuleGUIByName ( "SlicerWelcome" ) != NULL) )
    {
    homeModule = "SlicerWelcome";
    }
  if ( homeModule == NULL || *homeModule == 0 )
    {
    homeModule = "Data";
    }

  if ( !NoModules && !TestMode )
    {
    std::string tclCmd = "update; $::slicer3::ApplicationGUI SelectModule \"" + std::string(homeModule) + "\"";
    Slicer_Tcl_Eval( interp, tclCmd.c_str() );
    }


  // More command line arguments:
  // use the startup script passed on command line if it exists
  if ( Script != "" )
    {
    std::string tclCmd = "";
    // Pass arguments to the Tcl script
    tclCmd = "set args \"\"; ";
    std::vector<std::string>::const_iterator argit = Args.begin();
    while (argit != Args.end())
      {
      tclCmd += " lappend args \"" + *argit + "\"; ";
      ++argit;
      }

    if (Script.find(".tcl",0) != std::string::npos)
      {
      tclCmd += " after idle {update; source " + Script +"}";
      }
    else if (Script.find(".py",0) != std::string::npos)
      {
      tclCmd += "after idle {update; $::slicer3::ApplicationGUI PythonCommand \"execfile('" + Script + "')\" }";
      }
    Slicer_Tcl_Eval( interp, tclCmd.c_str() ) ;
    }


  int res; // return code (exit code)

  // use the startup code passed on command line if it exists
  // Example for csh to exit after startup:
  //
  // ./Slicer3 --exec exit'
  //
  if ( Exec != "" )
    {
    std::string tclCmd = "set ::SLICER(exec) \"" + Exec + "\" ; ";
    tclCmd += "regsub -all {\\.,} $::SLICER(exec) \";\" ::SLICER(exec); ";
    tclCmd += "regsub -all {,\\.} $::SLICER(exec) \";\" ::SLICER(exec); ";
    tclCmd += "after idle eval $::SLICER(exec);";
    res = Slicer_Tcl_Eval( interp, tclCmd.c_str() );
    }

  if ( ExecPython != "" )
    {
#if defined(Slicer_USE_PYTHON)
    if ( !NoModules )
      {
      std::string pythonCmd = ExecPython + "\n";
      v = PyRun_String( ExecPython.c_str(),
                        Py_file_input,
                        PythonDictionary,PythonDictionary);
      if (v == NULL)
        {
        PyErr_Print();
        }
      }
#endif
    }

  //
  // if there was no script file, the Args should hold a mrml file
  //
  //
  if ( File == ""  && Script == "" && Args.size() != 0)
    {
    std::vector<std::string>::const_iterator argit = Args.begin();
    while (argit != Args.end())
      {
      std::string fileName;
      // strip backslashes
      char argBuffer[2]; argBuffer[1] = '\0';
      for (i = 0; i < (*argit).size(); i++)
        {
        argBuffer[0] = (*argit)[i];
        if ( argBuffer[0] != '\\' )
          {
          fileName.append(std::string(argBuffer));
          }
        }
      // is it a MRML or XML file?
      if (fileName.find(".mrml",0) != std::string::npos
          || fileName.find(".MRML",0) != std::string::npos)
        {
        appLogic->GetMRMLScene()->SetURL(fileName.c_str());
        // and then load it
        slicerApp->SplashMessage("Set scene url, connecting...");
        std::string tclCmd = "after idle {update; $::slicer3::MRMLScene Connect}";
        res = Slicer_Tcl_Eval( interp, tclCmd.c_str() );
        }
      else if (fileName.find(".xml",0) != std::string::npos
               || fileName.find(".XML",0) != std::string::npos)
        {
        // if it's an xml file, load it
        std::string tclCmd = "after idle {update; ImportSlicer2Scene \"" + *argit + "\"}";
        slicerApp->SplashMessage("Importing Slicer2 scene...");
        res = Slicer_Tcl_Eval( interp, tclCmd.c_str() );
        slicerApp->SplashMessage("Imported scene.");
        }
      else if (fileName.find(".xcat",0) != std::string::npos
               || fileName.find(".XCAT",0) != std::string::npos)
        {
        // if it's an xcede file, load it
        slicerApp->SplashMessage("Importing Xcede catalog ...");
        std::string tclCmd = "after idle {update; XcatalogImport \"" + *argit + "\"}";
        res = Slicer_Tcl_Eval( interp, tclCmd.c_str() );
        slicerApp->SplashMessage("Imported catalog.");
        }
      else if (fileName.find(".tcl",0) != std::string::npos)
        {
        // if it's a tcl file source it after the app starts
        std::string tclCmd = "after idle {update; source " + *argit + "}";
        res = Slicer_Tcl_Eval( interp, tclCmd.c_str() );
        }
      else if (fileName.find(".py",0) != std::string::npos)
        {
        // if it's a python file source it after the app starts
        std::string tclCmd = 
          "after idle {update; $::slicer3::ApplicationGUI PythonCommand \"execfile('" + *argit + "')\" }";
        res = Slicer_Tcl_Eval( interp, tclCmd.c_str() );
        }
      else
        {
        // if we're not sure, assume it is data to load...
        std::string tclCmd = "after idle {update; ::Loader::ShowDialog {" + *argit + "}}";
        res = Slicer_Tcl_Eval( interp, tclCmd.c_str() );
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
    std::string tclCmd = "after idle {update; ::Loader::LoadArchetype {" + LoadDicomDir + "}}";
    res = Slicer_Tcl_Eval( interp, tclCmd.c_str() );
    }


  scene->ClearUndoStack();
  scene->ClearRedoStack();

  //
  // Run!  - this will return when the user exits
  //
  res = slicerApp->StartApplication();

  SlicerQDebug("Shutting down ...");

  appGUI->GetMainSlicerWindow()->PrepareForDelete();
  appGUI->GetMainSlicerWindow()->Close();

  // ------------------------------
  // REMOVE OBSERVERS and references to MRML and Logic

  lmit = loadableModuleNames.begin();
  while (lmit != loadableModuleNames.end())
    {
    LoadableModuleDescription desc = loadableModuleFactory.GetModuleDescription(*lmit);
    if (ignoreModules->LookupValue(desc.GetName().c_str()) < 0)
      {
      vtkSlicerModuleGUI* gui = desc.GetGUIPtr();
      if (gui)
        {
        gui->TearDownGUI();
        gui->RemoveGUIObservers();
        }
      }
    lmit++;
    }

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

  dataGUI->RemoveObservers(vtkSlicerModuleGUI::ModuleSelectedEvent, (vtkCommand *)appGUI->GetGUICallbackCommand());
  dataGUI->TearDownGUI ( );

  appGUI->RemoveGUIObservers ( );
  appGUI->SetAndObserveMainSliceLogic ( NULL, NULL, NULL );

#if !defined(COMMANDLINE_DEBUG) && (defined(Slicer_BUILD_CLI) || defined(Slicer_BUILD_MODULES))
  // remove the observers from the factory discovered modules
  // (as we remove the observers, cache the GUIs in a vector so we
  // can delete them later).
  mit = moduleNames.begin();
  std::vector<vtkSlicerModuleGUI*> moduleGUIs;
  while ( mit != moduleNames.end() )
    {
    vtkSlicerModuleGUI *module;
    module = slicerApp->GetModuleGUIByName( (*mit).c_str() );

    if ( module )
      {

      /*      module->RemoveGUIObservers();*/
      module->TearDownGUI ( );

      moduleGUIs.push_back(module);
      }

    ++mit;
    }
#endif

#if !defined(SCRIPTEDMODULE_DEBUG) && defined(Slicer_BUILD_MODULES)
  if ( !NoModules && (ignoreModules->LookupValue("Scripted") < 0) )
    {
    // remove the observers from the scripted modules
    tclCommand = "";
    tclCommand += "foreach package $::Slicer_PACKAGES(list) { ";
    tclCommand += "  if {[info exists ::Slicer_PACKAGES($package,gui)] == 1} { ";
    tclCommand += "    $::Slicer_PACKAGES($package,gui) RemoveGUIObservers;";
    tclCommand += "    $::Slicer_PACKAGES($package,gui) TearDownGUI;";
    tclCommand += "  } ";
    tclCommand += "}";
    Slicer_Tcl_Eval( interp, tclCommand.c_str() );
    }

#if defined(Slicer_USE_PYTHON)
  if ( !NoModules )
    {
    v = PyRun_String( "SlicerScriptedModuleInfo.TearDownAllGUI();\n",
                      Py_file_input,
                      PythonDictionary,PythonDictionary);
    if (v == NULL)
      {
      PyErr_Print();
      }
    }
#endif
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
    scene->SetDataIOManager(NULL);
    dataIOManager->SetCacheManager(NULL);
    dataIOManager->Delete();
    dataIOManager = NULL;
    }
  if ( cacheManager != NULL )
    {
    scene->SetCacheManager(NULL);
    cacheManager->SetMRMLScene ( NULL );
    cacheManager->Delete();
    cacheManager = NULL;
    }
  if (URIHandlerCollection != NULL )
    {
    scene->SetURIHandlerCollection(NULL);
    URIHandlerCollection->Delete();
    URIHandlerCollection = NULL;
    }
  scene->SetUserTagTable( NULL );



  //--- delete gui first, removing Refs to Logic and MRML

  lmit = loadableModuleNames.begin();
  while (lmit != loadableModuleNames.end())
    {
    LoadableModuleDescription desc = loadableModuleFactory.GetModuleDescription(*lmit);
    vtkSlicerModuleGUI* gui = desc.GetGUIPtr();
    if (gui)
      {
      gui->Delete();
      }
    lmit++;
    }

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

#if !defined(SCRIPTEDMODULE_DEBUG) && defined(Slicer_BUILD_MODULES)
  if ( !NoModules && (ignoreModules->LookupValue("Scripted") < 0) )
    {
    tclCommand = "";
    tclCommand += "foreach package $::Slicer_PACKAGES(list) { ";
    tclCommand += "  if {[info exists ::Slicer_PACKAGES($package,gui)] == 1} { ";
    tclCommand += "    $::Slicer_PACKAGES($package,gui) Delete;";
    tclCommand += "  }";
    tclCommand += "}";
    Slicer_Tcl_Eval( interp, tclCommand.c_str() );
    }

#if defined(Slicer_USE_PYTHON)
  if ( !NoModules )
    {
    v = PyRun_String( "SlicerScriptedModuleInfo.DeleteAllGUI();\n",
                      Py_file_input,
                      PythonDictionary,PythonDictionary);
    if (v == NULL)
      {
      PyErr_Print();
      }
    }
#endif
#endif

  // Release reference to applicaiton GUI
  // and delete it.
  // slicerApp->SetApplicationGUI ( NULL );
  // appGUI->DeleteComponentGUIs();
  // cout << "vtkSlicerApplicationGUI deleting app GUI\n";
  // appGUI->Delete ();

#if !defined(COMMANDLINE_DEBUG) && (defined(Slicer_BUILD_CLI) || defined(Slicer_BUILD_MODULES))
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

  // Release reference to application GUI
  // and delete it.
  slicerApp->SetApplicationGUI ( NULL );
  appGUI->DeleteComponentGUIs();
  appGUI->TearDownViewers();
  appGUI->SetSlicesGUI ( NULL );
#if !defined(SLICESMODULE_DEBUG)
  slicesGUI->Delete();
#endif
  appGUI->Delete ();

  //--- delete logic next, removing Refs to MRML
  appLogic->ClearCollections ( );

  lmit = loadableModuleNames.begin();
  while (lmit != loadableModuleNames.end())
    {
    LoadableModuleDescription desc = loadableModuleFactory.GetModuleDescription(*lmit);

    vtkSlicerModuleLogic* logic = desc.GetLogicPtr();
    if ( logic )
      {
      logic->SetAndObserveMRMLScene( NULL );
      logic->Delete();
      }
    lmit++;
    }

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

#if !defined(COMMANDLINE_DEBUG) && (defined(Slicer_BUILD_CLI) || defined(Slicer_BUILD_MODULES))
  // delete the factory discovered module Logics
  std::vector<vtkSlicerModuleLogic*>::iterator lit;
  for (lit = moduleLogics.begin(); lit != moduleLogics.end(); ++lit)
    {
    (*lit)->SetAndObserveMRMLScene ( NULL );
    (*lit)->Delete();
    }
  moduleLogics.clear();
#endif


#if !defined(SCRIPTEDMODULE_DEBUG) && defined(Slicer_BUILD_MODULES)
  if ( !NoModules && (ignoreModules->LookupValue("Scripted") < 0) )
    {
    // delete the scripted logics
    tclCommand = "";
    tclCommand += "foreach package $::Slicer_PACKAGES(list) { ";
    tclCommand += "  if {[info exists ::Slicer_PACKAGES($package,logic)] == 1} { ";
    tclCommand += "    $::Slicer_PACKAGES($package,logic) SetAndObserveMRMLScene {};";
    tclCommand += "    $::Slicer_PACKAGES($package,logic) Delete;";
    tclCommand += "  }";
    tclCommand += "}";
    Slicer_Tcl_Eval( interp, tclCommand.c_str() );
    }

#if defined(Slicer_USE_PYTHON)
  if ( !NoModules )
    {
    v = PyRun_String( "SlicerScriptedModuleInfo.DeleteAllLogic();\n",
                      Py_file_input,
                      PythonDictionary,PythonDictionary);
    if (v == NULL)
      {
      PyErr_Print();
      }
    }
#endif
#endif

#ifdef Slicer_USE_PYTHON
  // Shutdown python interpreter
  v = PyRun_String( "import Slicer;\nSlicer.slicer.deleteInstances()\n",
                    Py_file_input,
                    PythonDictionary,PythonDictionary);
  if (v == NULL)
    {
    PyErr_Print();
    }
  Py_Finalize();
#endif

  //--- scene next;
  scene->Clear(1);
  scene->Delete ();

  // -- event broker
  // - free up the reference from the interpeter
  slicerApp->Script ("$::slicer3::Broker Delete");

  //--- application last
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
  int ret = Slicer_main(argc, argv);
  for (int i = 0; i < argc; i++) { delete [] argv[i]; }
  delete [] argv;
  return ret;
}

int main(int argc, char *argv[])
{
  return Slicer_main(argc, argv);
}

#else
int main(int argc, char *argv[])
{
  return Slicer_main(argc, argv);
}
#endif

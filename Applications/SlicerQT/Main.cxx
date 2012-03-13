/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QSettings>
#include <QSplashScreen>
#include <QTimer>

// Slicer includes
#include "vtkSlicerConfigure.h" // For Slicer_USE_PYTHONQT

// CTK includes
#include <ctkAbstractLibraryFactory.h>
#ifdef Slicer_USE_PYTHONQT
# include <ctkPythonConsole.h>
#endif

// MRMLWidgets includes
#include <qMRMLEventLoggerWidget.h>

// Slicer includes
#include "vtkSlicerVersionConfigure.h" // For Slicer_VERSION_FULL, Slicer_BUILD_CLI_SUPPORT

// SlicerQt includes
#include "qSlicerApplication.h"
#ifdef Slicer_BUILD_CLI_SUPPORT
# include "qSlicerCLIExecutableModuleFactory.h"
# include "qSlicerCLILoadableModuleFactory.h"
#endif
#include "qSlicerCommandOptions.h"
#include "qSlicerCoreModuleFactory.h"
#include "qSlicerLoadableModuleFactory.h"
#include "qSlicerMainWindow.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"
#include "qSlicerStyle.h"

// VTK includes
//#include <vtkObject.h>

#ifdef Slicer_USE_PYTHONQT
# include "qSlicerScriptedLoadableModuleFactory.h"
#endif

#ifdef Slicer_USE_PYTHONQT
# include <PythonQtObjectPtr.h>
# include "qSlicerPythonManager.h"
# include "qSlicerSettingsPythonPanel.h"
# include <dPython.h>
#endif

#if defined (_WIN32) && !defined (Slicer_BUILD_WIN32_CONSOLE)
# include <windows.h>
# include <vtksys/SystemTools.hxx>
#endif

namespace
{

#ifdef Slicer_USE_PYTHONQT

//----------------------------------------------------------------------------
void initializePython()
{
  qSlicerApplication * app = qSlicerApplication::application();

  // If first unparsed argument is python script, enable 'shebang' mode
  QStringList unparsedArguments = app->commandOptions()->unparsedArguments();
  if (unparsedArguments.size() > 0 && unparsedArguments.at(0).endsWith(".py"))
    {
    if(!app->commandOptions()->pythonScript().isEmpty())
      {
      qWarning() << "Ignore script specified using '--python-script'";
      }
    app->commandOptions()->setExtraPythonScript(unparsedArguments.at(0));
    app->commandOptions()->setRunPythonAndExit(true);
    }
  if (!app->commandOptions()->pythonCode().isEmpty())
    {
    app->commandOptions()->setRunPythonAndExit(true);
    }
}

//----------------------------------------------------------------------------
void initializePythonConsole(ctkPythonConsole& pythonConsole)
{
  // Create python console
  Q_ASSERT(qSlicerApplication::application()->pythonManager());
  pythonConsole.initialize(qSlicerApplication::application()->pythonManager());

  QStringList autocompletePreferenceList;
  autocompletePreferenceList
      << "slicer" << "slicer.mrmlScene"
      << "qt.QPushButton";
  pythonConsole.completer()->setAutocompletePreferenceList(autocompletePreferenceList);

  //pythonConsole.setAttribute(Qt::WA_QuitOnClose, false);
  pythonConsole.resize(600, 280);

  qSlicerApplication::application()->settingsDialog()->addPanel(
    "Python settings", new qSlicerSettingsPythonPanel);

  // Show pythonConsole if required
  qSlicerCommandOptions * options = qSlicerApplication::application()->commandOptions();
  if(options->showPythonInteractor() && !options->runPythonAndExit())
    {
    pythonConsole.show();
    pythonConsole.activateWindow();
    pythonConsole.raise();
    }
}
#endif

//----------------------------------------------------------------------------
void setupModuleFactoryManager(qSlicerModuleFactoryManager * moduleFactoryManager)
{
  qSlicerApplication* app = qSlicerApplication::application();
  // Register module factories
  moduleFactoryManager->registerFactory(new qSlicerCoreModuleFactory);

// \todo Move the registration somewhere else for reuse.
  qSlicerCommandOptions* options = qSlicerApplication::application()->commandOptions();
  if (!options->disableLoadableModules() && !options->runPythonAndExit())
    {
    moduleFactoryManager->registerFactory(new qSlicerLoadableModuleFactory);
    QString loadablePath = app->slicerHome() + "/" + Slicer_QTLOADABLEMODULES_LIB_DIR + "/";
    moduleFactoryManager->addSearchPath(loadablePath);
    // On Win32, *both* paths have to be there, since scripts are installed
    // in the install location, and exec/libs are *automatically* installed
    // in intDir.
    moduleFactoryManager->addSearchPath(loadablePath + app->intDir());
    }

#ifdef Slicer_USE_PYTHONQT
  if (!options->disableScriptedLoadableModules() &&
      !qSlicerApplication::testAttribute(qSlicerApplication::AA_DisablePython) &&
      !options->runPythonAndExit())
    {
    moduleFactoryManager->registerFactory(
      new qSlicerScriptedLoadableModuleFactory);
    QString scriptedPath = app->slicerHome() + "/" + Slicer_QTSCRIPTEDMODULES_LIB_DIR + "/";
    moduleFactoryManager->addSearchPath(scriptedPath);
    // On Win32, *both* paths have to be there, since scripts are installed
    // in the install location, and exec/libs are *automatically* installed
    // in intDir.
    moduleFactoryManager->addSearchPath(scriptedPath + app->intDir());
    }
#endif

  QSettings settings;
#ifdef Slicer_BUILD_CLI_SUPPORT
  if (!options->disableCLIModules() && !options->runPythonAndExit())
    {
    QString tempDirectory =
      qSlicerCoreApplication::application()->coreCommandOptions()->tempDirectory();
    bool preferExecutableCLIs =
      settings.value("Modules/PreferExecutableCLI", false).toBool();
    moduleFactoryManager->registerFactory(
      new qSlicerCLILoadableModuleFactory(tempDirectory), preferExecutableCLIs ? 0 : 1);
    // Option to prefer executable CLIs to limit memory consumption.
    moduleFactoryManager->registerFactory(
      new qSlicerCLIExecutableModuleFactory(tempDirectory), preferExecutableCLIs ? 1 : 0);
    QString cliPath = app->slicerHome() + "/" + Slicer_CLIMODULES_LIB_DIR + "/";
    moduleFactoryManager->addSearchPath(cliPath);
    // On Win32, *both* paths have to be there, since scripts are installed
    // in the install location, and exec/libs are *automatically* installed
    // in intDir.
    moduleFactoryManager->addSearchPath(cliPath + app->intDir());
#ifdef Q_OS_MAC
    moduleFactoryManager->addSearchPath(app->slicerHome() + "/" + Slicer_CLIMODULES_SUBDIR);
#endif
    }
#endif
  moduleFactoryManager->addSearchPaths(
    settings.value("Modules/AdditionalPaths").toStringList());
  moduleFactoryManager->setModulesToIgnore(
    settings.value("Modules/IgnoreModules").toStringList());
  moduleFactoryManager->setVerboseModuleDiscovery(app->commandOptions()->verboseModuleDiscovery());
}

//----------------------------------------------------------------------------
void showMRMLEventLoggerWidget()
{
  qMRMLEventLoggerWidget* logger = new qMRMLEventLoggerWidget(0);
  logger->setAttribute(Qt::WA_DeleteOnClose);
  logger->setConsoleOutputEnabled(false);
  logger->setMRMLScene(qSlicerApplication::application()->mrmlScene());

  QObject::connect(qSlicerApplication::application(),
                   SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   logger,
                   SLOT(setMRMLScene(vtkMRMLScene*)));

  logger->show();
}

//----------------------------------------------------------------------------
void splashMessage(QScopedPointer<QSplashScreen>& splashScreen, const QString& message)
{
  if (splashScreen.isNull())
    {
    return;
    }
  splashScreen->showMessage(message, Qt::AlignBottom | Qt::AlignHCenter);
  //splashScreen->repaint();
}

//----------------------------------------------------------------------------
int slicerQtMain(int argc, char* argv[])
{
  QCoreApplication::setApplicationName("Slicer");
  QCoreApplication::setApplicationVersion(Slicer_VERSION_FULL);
  //vtkObject::SetGlobalWarningDisplay(false);
  QApplication::setDesktopSettingsAware(false);
  QApplication::setStyle(new qSlicerStyle);

  qSlicerApplication app(argc, argv);
  if (app.returnCode() != -1)
    {
    return app.returnCode();
    }

#ifdef Slicer_USE_PYTHONQT
  ctkPythonConsole pythonConsole;
  if (!qSlicerApplication::testAttribute(qSlicerApplication::AA_DisablePython))
    {
    initializePython();
    initializePythonConsole(pythonConsole);
    }
#endif

  bool enableMainWindow = !app.commandOptions()->noMainWindow();
  enableMainWindow = enableMainWindow && !app.commandOptions()->runPythonAndExit();
  bool showSplashScreen = !app.commandOptions()->noSplash() && enableMainWindow;

  QScopedPointer<QSplashScreen> splashScreen;
  if (showSplashScreen)
    {
    QPixmap pixmap(":Images/SlicerSplashScreen.png");
    splashScreen.reset(new QSplashScreen(pixmap));
    splashMessage(splashScreen, "Initializing...");
    splashScreen->show();
    }

  qSlicerModuleManager * moduleManager = qSlicerApplication::application()->moduleManager();
  qSlicerModuleFactoryManager * moduleFactoryManager = moduleManager->factoryManager();
  setupModuleFactoryManager(moduleFactoryManager);

  // Register and instantiate modules
  splashMessage(splashScreen, "Registering modules...");
  moduleFactoryManager->registerModules();
  qDebug() << "Number of registered modules:"
           << moduleFactoryManager->registeredModuleNames().count();
  splashMessage(splashScreen, "Instantiating modules...");
  moduleFactoryManager->instantiateModules();
  qDebug() << "Number of instantiated modules:"
           << moduleFactoryManager->instantiatedModuleNames().count();
  // Create main window
  splashMessage(splashScreen, "Initializing user interface...");
  QScopedPointer<qSlicerMainWindow> window;
  if (enableMainWindow)
    {
    window.reset(new qSlicerMainWindow);
    window->setWindowTitle(window->windowTitle()+ " " + Slicer_VERSION_FULL);
    }

  // Load all available modules
  foreach(const QString& name, moduleFactoryManager->instantiatedModuleNames())
    {
    Q_ASSERT(!name.isNull());
    qDebug() << "Loading module" << name;
    splashMessage(splashScreen, "Loading module \"" + name + "\"...");
    moduleFactoryManager->loadModule(name);
    }
  qDebug() << "Number of loaded modules:" << moduleManager->modulesNames().count();

  splashMessage(splashScreen, QString());

  if (window)
    {
    window->setHomeModuleCurrent();
    window->show();
    }

  if (splashScreen && window)
    {
    splashScreen->finish(window.data());
    }

  // Process command line argument after the event loop is started
  QTimer::singleShot(0, &app, SLOT(handleCommandLineArguments()));

  // showMRMLEventLoggerWidget();

  // Look at QApplication::exec() documentation, it is recommended to connect
  // clean up code to the aboutToQuit() signal
  return app.exec();
}

} // end of anonymous namespace

#if defined (_WIN32) && !defined (Slicer_BUILD_WIN32_CONSOLE)
int __stdcall WinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPSTR lpCmdLine, int nShowCmd)
{
  Q_UNUSED(hInstance);
  Q_UNUSED(hPrevInstance);
  Q_UNUSED(nShowCmd);

  int argc;
  char **argv;
  vtksys::SystemTools::ConvertWindowsCommandLineToUnixArguments(
    lpCmdLine, &argc, &argv);

  int ret = slicerQtMain(argc, argv);

  for (int i = 0; i < argc; i++)
    {
    delete [] argv[i];
    }
  delete [] argv;

  return ret;
}
#else
int main(int argc, char *argv[])
{
  return slicerQtMain(argc, argv);
}
#endif

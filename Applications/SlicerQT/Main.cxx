/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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
#include <QMessageBox>
#include <QSplashScreen>
#include <QTimer>

// Slicer includes
#include "vtkSlicerConfigure.h" // For Slicer_USE_PYTHONQT

// CTK includes
#include <ctkCallback.h>
#include <ctkLogger.h>
#ifdef Slicer_USE_PYTHONQT
# include <ctkPythonConsole.h>
#endif

// qMRMLWidgets includes
#include <qMRMLEventLoggerWidget.h>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerCLIExecutableModuleFactory.h"
#include "qSlicerCLILoadableModuleFactory.h"
#include "qSlicerCommandOptions.h"
#include "qSlicerCoreModuleFactory.h"
#include "qSlicerLoadableModuleFactory.h"
#include "qSlicerMainWindow.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"
#include "qSlicerStyle.h"

// Slicer includes
#include "vtkSlicerVersionConfigure.h" // For Slicer_VERSION_FULL

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

// PythonQt wrapper initialization methods
void PythonQt_init_org_slicer_base_qSlicerBaseQTCore(PyObject*);
void PythonQt_init_org_slicer_base_qSlicerBaseQTGUI(PyObject*);

//---------------------------------------------------------------------------
void PythonPreInitialization()
{
  // Initialize wrappers
  PythonQt_init_org_slicer_base_qSlicerBaseQTCore(0);
  PythonQt_init_org_slicer_base_qSlicerBaseQTGUI(0);
}
#endif

namespace
{
//----------------------------------------------------------------------------
void popupDisclaimerDialog(void * data)
{
  if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_EnableTesting))
    {
    QString message = QString("Thank you for trying %1!\n\n"
                              "Please be aware that this software is under active "
                              "development and has not been tested for accuracy. "
                              "Many important features are still missing.\n\n"
                              "This software is not intended for clinical use.")
      .arg(QString("3D Slicer ") + Slicer_VERSION_FULL);
    QMessageBox::information(reinterpret_cast<qSlicerMainWindow*>(data), "3D Slicer", message);
    }
}


#ifdef Slicer_USE_PYTHONQT
//----------------------------------------------------------------------------
void setApplicationDisablePythonAttribute(int argc, char* argv[])
{
  for (int i = 0; i < argc; ++i)
    {
    if (qstrcmp(argv[i], "--disable-python") == 0)
      {
      qSlicerCoreApplication::setAttribute(qSlicerCoreApplication::AA_DisablePython);
      break;
      }
    }
}

//----------------------------------------------------------------------------
void initializePython()
{
  qSlicerApplication * app = qSlicerApplication::application();
  app->pythonManager()->setInitializationFunction(PythonPreInitialization);
  app->corePythonManager()->mainContext(); // Initialize python
#ifdef Q_WS_WIN
  // HACK - Since on windows setting an environment variable using putenv doesn't propagate
  // to the environment initialized in python, let's make sure 'os.environ' is updated.
  app->updatePythonOsEnviron();
#endif

  // If first unparsed argument is python script, enable 'shebang' mode
  QStringList unparsedArguments = app->commandOptions()->unparsedArguments();
  if (unparsedArguments.size() > 0 && unparsedArguments.at(0).endsWith(".py"))
    {
    if(!app->commandOptions()->pythonScript().isEmpty())
      {
      qWarning() << "Ignore script specified using '--python-script'";
      }
    app->commandOptions()->setExtraPythonScript(unparsedArguments.at(0));
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
  if(qSlicerApplication::application()->commandOptions()->showPythonInteractor())
    {
    pythonConsole.show();
    pythonConsole.activateWindow();
    pythonConsole.raise();
    }
}
#endif

//----------------------------------------------------------------------------
void registerLoadableModuleFactory(
  qSlicerModuleFactoryManager * moduleFactoryManager,
  const QSharedPointer<ctkAbstractLibraryFactory<qSlicerAbstractCoreModule>::HashType>& moduleFactorySharedRegisteredItemKeys)
{
  qSlicerLoadableModuleFactory* loadableModuleFactory = new qSlicerLoadableModuleFactory();
  loadableModuleFactory->setSharedItems(moduleFactorySharedRegisteredItemKeys);
  moduleFactoryManager->registerFactory("qSlicerLoadableModuleFactory", loadableModuleFactory);
}

//----------------------------------------------------------------------------
void registerScriptedLoadableModuleFactory(
  qSlicerModuleFactoryManager * moduleFactoryManager,
  const QSharedPointer<ctkAbstractLibraryFactory<qSlicerAbstractCoreModule>::HashType>& moduleFactorySharedRegisteredItemKeys)
{
#ifdef Slicer_USE_PYTHONQT
  if (!qSlicerApplication::testAttribute(qSlicerApplication::AA_DisablePython))
    {
    qSlicerScriptedLoadableModuleFactory* scriptedLoadableModuleFactory =
      new qSlicerScriptedLoadableModuleFactory();
    scriptedLoadableModuleFactory->setSharedItems(moduleFactorySharedRegisteredItemKeys);
    moduleFactoryManager->registerFactory("qSlicerScriptedLoadableModuleFactory",
                                          scriptedLoadableModuleFactory);
    }
#else
  Q_UNUSED(moduleFactoryManager);
  Q_UNUSED(moduleFactorySharedRegisteredItemKeys);
#endif
}

//----------------------------------------------------------------------------
void registerCLIModuleFactory(
  qSlicerModuleFactoryManager * moduleFactoryManager, const QString& tempDirectory,
  const QSharedPointer<ctkAbstractLibraryFactory<qSlicerAbstractCoreModule>::HashType>& moduleFactorySharedRegisteredItemKeys)
{
  qSlicerCLILoadableModuleFactory* cliLoadableModuleFactory =
    new qSlicerCLILoadableModuleFactory();
  cliLoadableModuleFactory->setTempDirectory(tempDirectory);
  cliLoadableModuleFactory->setSharedItems(moduleFactorySharedRegisteredItemKeys);
  moduleFactoryManager->registerFactory("qSlicerCLILoadableModuleFactory",
                                        cliLoadableModuleFactory);

  qSlicerCLIExecutableModuleFactory* cliExecutableModuleFactory =
    new qSlicerCLIExecutableModuleFactory();
  cliExecutableModuleFactory->setTempDirectory(tempDirectory);
  cliExecutableModuleFactory->setSharedItems(moduleFactorySharedRegisteredItemKeys);
  moduleFactoryManager->registerFactory("qSlicerCLIExecutableModuleFactory",
                                        cliExecutableModuleFactory);
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

} // end of anonymous namespace

//----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  QCoreApplication::setApplicationName("Slicer");
  QCoreApplication::setApplicationVersion(Slicer_VERSION_FULL);
  //vtkObject::SetGlobalWarningDisplay(false);
  QApplication::setDesktopSettingsAware(false);
  QApplication::setStyle(new qSlicerStyle);
  ctkLogger::configure();

#ifdef Slicer_USE_PYTHONQT
  // Since the attribute AA_DisablePython has to be set before the application
  // is instantiated, the following function will check if --disable-python argument
  // has been passed.
  setApplicationDisablePythonAttribute(argc, argv);
#endif

  qSlicerApplication app(argc, argv);

  bool exitWhenDone = false;
  app.parseArguments(exitWhenDone);
  if (exitWhenDone)
    {
    return EXIT_SUCCESS;
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
  enableMainWindow = enableMainWindow && app.commandOptions()->extraPythonScript().isEmpty();
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

  // Register module factories
  qSlicerCoreModuleFactory* coreModuleFactory = new qSlicerCoreModuleFactory();
  moduleFactoryManager->registerFactory("qSlicerCoreModuleFactory", coreModuleFactory);

  if (!app.commandOptions()->disableLoadableModules())
    {
    registerLoadableModuleFactory(moduleFactoryManager, coreModuleFactory->sharedItems());
    }
  if (!app.commandOptions()->disableScriptedLoadableModules())
    {
    registerScriptedLoadableModuleFactory(moduleFactoryManager, coreModuleFactory->sharedItems());
    }
  if (!app.commandOptions()->disableCLIModules())
    {
    registerCLIModuleFactory(
          moduleFactoryManager,
          qSlicerCoreApplication::application()->coreCommandOptions()->tempDirectory(),
          coreModuleFactory->sharedItems());
    }

  moduleFactoryManager->setVerboseModuleDiscovery(app.commandOptions()->verboseModuleDiscovery());
  
  // Register and instantiate modules
  splashMessage(splashScreen, "Registering modules...");
  moduleFactoryManager->registerAllModules();
  qDebug() << "Number of registered modules:" << moduleManager->moduleList().count();

  splashMessage(splashScreen, "Instantiating modules...");
  moduleFactoryManager->instantiateAllModules();

  // Create main window
  splashMessage(splashScreen, "Initializing user interface...");
  QScopedPointer<qSlicerMainWindow> window;
  if (enableMainWindow)
    {
    window.reset(new qSlicerMainWindow);
    window->setWindowTitle(window->windowTitle()+ " " + Slicer_VERSION_FULL);
    }

  // Load all available modules
  QStringList moduleNames = moduleFactoryManager->moduleNames();
  foreach(const QString& name, moduleNames)
    {
    if (name.isNull())
      {
      qWarning() << "Encountered null module name";
      continue;
      }
    qDebug() << "Loading module" << name;
    splashMessage(splashScreen, "Loading module \"" + name + "\"...");
    moduleManager->loadModule(name);
    }
  qDebug() << "Number of loaded modules:" << moduleManager->loadedModules().count();

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

  // Popup disclaimer
  ctkCallback popupDisclaimerDialogCallback;
  if (window)
    {
    popupDisclaimerDialogCallback.setCallback(popupDisclaimerDialog);
    popupDisclaimerDialogCallback.setCallbackData(window.data());
    QTimer::singleShot(0, &popupDisclaimerDialogCallback, SLOT(invoke()));
    }

  // showMRMLEventLoggerWidget();

  // Look at QApplication::exec() documentation, it is recommended to connect
  // clean up code to the aboutToQuit() signal
  return app.exec();
}

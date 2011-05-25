/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QSplashScreen>
#include <QTimer>

// Slicer includes
#include "vtkSlicerConfigure.h" // For Slicer_USE_PYTHONQT

// CTK includes
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
#include "qSlicerModulePanel.h"
#include "qSlicerModuleSelectorToolBar.h"
#include "qSlicerStyle.h"

// Slicer includes
#include "vtkSlicerVersionConfigure.h" // For Slicer_VERSION_FULL

// VTK includes
//#include <vtkObject.h>

#ifdef Slicer_USE_PYTHONQT
# include "qSlicerPythonManager.h"
# include "qSlicerScriptedLoadableModuleFactory.h"
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

//----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  QCoreApplication::setApplicationName("Slicer");
  QCoreApplication::setApplicationVersion(Slicer_VERSION_FULL);

  //vtkObject::SetGlobalWarningDisplay(false);
  QApplication::setDesktopSettingsAware(false);
  QApplication::setStyle(new qSlicerStyle);
  ctkLogger::configure();

  qSlicerApplication app(argc, argv);

#ifdef Slicer_USE_PYTHONQT
  if (!qSlicerApplication::testAttribute(qSlicerApplication::AA_DisablePython))
    {
    app.pythonManager()->setInitializationFunction(PythonPreInitialization);
    }
#endif

  //app.setApplicationVersion();
  //app.setWindowIcon(QIcon(":Icons/..."));
  bool exitWhenDone = false;
  app.initialize(exitWhenDone);
  if  (exitWhenDone)
    {
    return EXIT_SUCCESS;
    }

  // If first unparsed argument is python script, enable 'shebang' mode
  QStringList unparsedArguments =
      qSlicerApplication::application()->commandOptions()->unparsedArguments();
  if (unparsedArguments.size() > 0 && unparsedArguments.at(0).endsWith(".py"))
    {
    if(!app.commandOptions()->pythonScript().isEmpty())
      {
      qWarning() << "Ignore script specified using '--python-script'";
      }
    app.commandOptions()->setExtraPythonScript(unparsedArguments.at(0));
    }

  bool enableMainWindow = !app.commandOptions()->noMainWindow();
  enableMainWindow = enableMainWindow && app.commandOptions()->extraPythonScript().isEmpty();

  QPixmap pixmap(":Images/SlicerSplashScreen.png");
  QSplashScreen splash(pixmap/*, Qt::WindowStaysOnTopHint*/);
  bool enableSplash = !app.commandOptions()->noSplash();
  enableSplash = enableSplash && enableMainWindow;
  if (enableSplash)
    {
    splash.show();
    }

  qSlicerModuleManager * moduleManager = qSlicerApplication::application()->moduleManager();
  Q_ASSERT(moduleManager);

  qSlicerModuleFactoryManager * moduleFactoryManager = moduleManager->factoryManager();

  // Register module factories
  qSlicerCoreModuleFactory* coreModuleFactory = new qSlicerCoreModuleFactory();
  moduleFactoryManager->registerFactory("qSlicerCoreModuleFactory", coreModuleFactory);

  if (!app.commandOptions()->disableLoadableModule())
    {
    qSlicerLoadableModuleFactory* loadableModuleFactory = new qSlicerLoadableModuleFactory();
    loadableModuleFactory->setRegisteredItems(coreModuleFactory->registeredItems());
    moduleFactoryManager->registerFactory("qSlicerLoadableModuleFactory", loadableModuleFactory);

#ifdef Slicer_USE_PYTHONQT
    if (!qSlicerApplication::testAttribute(qSlicerApplication::AA_DisablePython))
      {
      qSlicerScriptedLoadableModuleFactory* scriptedLoadableModuleFactory =
        new qSlicerScriptedLoadableModuleFactory();
      scriptedLoadableModuleFactory->setRegisteredItems(coreModuleFactory->registeredItems());
      moduleFactoryManager->registerFactory("qSlicerScriptedLoadableModuleFactory",
                                            scriptedLoadableModuleFactory);
      }
#endif
    }

  if (!app.commandOptions()->disableCLIModule())
    {
    qSlicerCLILoadableModuleFactory* cliLoadableModuleFactory =
      new qSlicerCLILoadableModuleFactory();
    cliLoadableModuleFactory->setRegisteredItems(coreModuleFactory->registeredItems());
    moduleFactoryManager->registerFactory("qSlicerCLILoadableModuleFactory",
                                          cliLoadableModuleFactory);
    qSlicerCLIExecutableModuleFactory* cliExecutableModuleFactory =
      new qSlicerCLIExecutableModuleFactory();
    cliExecutableModuleFactory->setRegisteredItems(coreModuleFactory->registeredItems());
    moduleFactoryManager->registerFactory("qSlicerCLIExecutableModuleFactory",
                                          cliExecutableModuleFactory);
    }

  moduleFactoryManager->setVerboseModuleDiscovery(app.commandOptions()->verboseModuleDiscovery());
  
  // Register and instantiate modules
  moduleFactoryManager->registerAllModules();
  moduleFactoryManager->instantiateAllModules();

#ifdef Slicer_USE_PYTHONQT
  ctkPythonConsole pythonConsole;
  if (!qSlicerApplication::testAttribute(qSlicerApplication::AA_DisablePython))
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
    }
#endif

  // Create main window
  QScopedPointer<qSlicerMainWindow> window;
  if (enableMainWindow)
    {
    window.reset(new qSlicerMainWindow);
    window->setWindowTitle(window->windowTitle()+ " " + Slicer_VERSION_FULL);
    }

  // Load all available modules
  QStringList moduleNames = moduleManager->factoryManager()->moduleNames();
  foreach(const QString& name, moduleNames)
    {
    if ( name.isNull() )
      {
      qWarning() << "Encountered null module name";
      continue;
      }
    qWarning() << "checking module " << name;
    moduleManager->loadModule(name);
    if (enableSplash)
      {
      splash.showMessage("Loading module " + name, Qt::AlignBottom | Qt::AlignHCenter);
      splash.repaint();
      }
    }

  if (enableSplash)
    {
    splash.clearMessage();
    }

  if (window)
    {
    window->setHomeModuleCurrent();

    // Show main window
    window->show();
    }

  if (enableSplash && window)
    {
    splash.finish(window.data());
    }

  // Process command line argument after the event loop is started
  QTimer::singleShot(0, &app, SLOT(handleCommandLineArguments()));

  if (window)
    {
    QString message = QString("Thank you for trying %1!\n\n"
                              "Please be aware that this software is under active "
                              "development and has not been tested for accuracy. "
                              "Many important features are still missing.\n\n"
                              "This software is not intended for clinical use.")
      .arg(QString("3D Slicer ") + Slicer_VERSION_FULL);
    QMessageBox::information(window.data(), "3D Slicer", message);
    }

#ifdef Slicer_USE_PYTHONQT
  if (!qSlicerApplication::testAttribute(qSlicerApplication::AA_DisablePython))
    {
    // Show pythonConsole if required
    if(app.commandOptions()->showPythonInteractor())
      {
      pythonConsole.show();
      pythonConsole.activateWindow();
      pythonConsole.raise();
      }
    }
#endif

//  qMRMLEventLoggerWidget logger;
//  logger.setConsoleOutputEnabled(false);
//  logger.setMRMLScene(qSlicerApplication::application()->mrmlScene());
//
//  QObject::connect(qSlicerApplication::application(),
//                   SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
//                   &logger,
//                   SLOT(setMRMLScene(vtkMRMLScene*)));
//
//  logger.show();

  // Look at QApplication::exec() documentation, it is recommended to connect
  // clean up code to the aboutToQuit() signal
  return app.exec();
}

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

// CTK includes
#include <ctkLogger.h>

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
  //vtkObject::SetGlobalWarningDisplay(false);
  QApplication::setDesktopSettingsAware(false);
  QApplication::setStyle(new qSlicerStyle);
  ctkLogger::configure();

  qSlicerApplication app(argc, argv);

#ifdef Slicer_USE_PYTHONQT
  app.pythonManager()->setInitializationFunction(PythonPreInitialization);
#endif

  //app.setApplicationVersion();
  //app.setWindowIcon(QIcon(":Icons/..."));
  bool exitWhenDone = false;
  app.initialize(exitWhenDone);
  if  (exitWhenDone)
    {
    return EXIT_SUCCESS;
    }

  QPixmap pixmap(":Images/SlicerSplashScreen.png");
  QSplashScreen splash(pixmap/*, Qt::WindowStaysOnTopHint*/);
  bool enableSplash = !app.commandOptions()->noSplash(); 
  if (enableSplash)
    {
    splash.show();
    }

  qSlicerModuleManager * moduleManager = qSlicerApplication::application()->moduleManager();
  Q_ASSERT(moduleManager);

  qSlicerModuleFactoryManager * moduleFactoryManager = moduleManager->factoryManager();

  // Register module factories
  moduleFactoryManager->registerFactory("qSlicerCoreModuleFactory",
                                        new qSlicerCoreModuleFactory());

  if (!app.commandOptions()->disableLoadableModule())
    {
    moduleFactoryManager->registerFactory("qSlicerLoadableModuleFactory",
                                          new qSlicerLoadableModuleFactory());

#ifdef Slicer_USE_PYTHONQT
    moduleFactoryManager->registerFactory("qSlicerScriptedLoadableModuleFactory",
                                          new qSlicerScriptedLoadableModuleFactory());
#endif
    }

  if (!app.commandOptions()->disableCLIModule())
    {
    moduleFactoryManager->registerFactory("qSlicerCLILoadableModuleFactory",
                                          new qSlicerCLILoadableModuleFactory());
    moduleFactoryManager->registerFactory("qSlicerCLIExecutableModuleFactory",
                                          new qSlicerCLIExecutableModuleFactory());
    }

  moduleFactoryManager->setVerboseModuleDiscovery(app.commandOptions()->verboseModuleDiscovery());
  
  // Register and instanciate modules
  moduleFactoryManager->registerAllModules();
  moduleFactoryManager->instantiateAllModules();

  // Create main window
  qSlicerMainWindow window;
  window.setWindowTitle(window.windowTitle()+ " " + Slicer_VERSION_FULL);
  
  // Load all available modules
  QStringList moduleNames = moduleManager->factoryManager()->moduleNames();
  foreach(const QString& name, moduleNames)
    {
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

  // TODO: load home module (check in Settings)
  //QSettings settings;
  //QString defaultModule = settings.value("Modules/HomeModule").toString();
  //window.moduleSelector()->selectModule(defaultModule);
  window.setHomeModuleCurrent();

  // Show main window
  window.show();
  if (!app.commandOptions()->noSplash())
    {
    splash.finish(&window);
    }

  // Process command line argument after the event loop is started
  QTimer::singleShot(0, &app, SLOT(handleCommandLineArguments()));
  QString message = QString("Thank you for trying %1!\n\n"
                            "Please be aware that this software is under active "
                            "development and has not been tested for accuracy. "
                            "Many important features are still missing.\n\n"
                            "This software is not intended for clinical use.")
    .arg(QString("3D Slicer ") + Slicer_VERSION_FULL);
  QMessageBox::information(&window, "3D Slicer", message);
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

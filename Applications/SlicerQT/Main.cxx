
// Qt includes
#include <QSplashScreen>
#include <QDebug>

// CTK includes
#include <ctkLogger.h>

// qMRMLWidgets includes
#include <qMRMLEventLoggerWidget.h>

// SlicerQt includes
#include "qSlicerCommandOptions.h"
#include "qSlicerModulePanel.h"
#include "qSlicerMainWindow.h"
#include "qSlicerModuleSelectorToolBar.h"
#include "qSlicerModuleManager.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerCoreModuleFactory.h"
#include "qSlicerLoadableModuleFactory.h"
#include "qSlicerCLILoadableModuleFactory.h"
#include "qSlicerCLIExecutableModuleFactory.h"
#include "qSlicerApplication.h"

// QT includes
#include <QSplashScreen>
#include <QDebug>

// VTK includes
//#include <vtkObject.h>

//----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  //vtkObject::SetGlobalWarningDisplay(false);
  ctkLogger::configure();

  qSlicerApplication app(argc, argv);
  app.setApplicationName("Slicer");
  //app.setApplicationVersion();
  //app.setWindowIcon(QIcon(":Icons/..."));

  bool exitWhenDone = false;
  app.initialize(exitWhenDone);
  if  (exitWhenDone)
    {
    return EXIT_SUCCESS;
    }

  QPixmap pixmap(":Images/SlicerSplashScreen.png");
  QSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);
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
  window.modulePanel()->setModule( moduleManager->moduleName("SlicerWelcome") );

  // Show main window
  window.show();
  if (!app.commandOptions()->noSplash())
    {
    splash.finish(&window);
    }

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
  
  return app.exec();
}

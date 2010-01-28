#include "qSlicerApplication.h"

// SlicerQT includes
#include "qSlicerModulePanel.h"
#include "qSlicerMainWindow.h"
#include "qSlicerModuleSelectorWidget.h"
#include "qSlicerModuleManager.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerCoreModuleFactory.h"
#include "qSlicerLoadableModuleFactory.h"
#include "qSlicerCLILoadableModuleFactory.h"
#include "qSlicerCLIExecutableModuleFactory.h"

// qMRML includes
// #include <qMRMLEventLoggerWidget.h>

// QT includes
#include <QSplashScreen>
#include <QDebug>

//----------------------------------------------------------------------------
// namespace
// {
// QString getSlicerHome()
// {
//   QString home = QString(getenv("Slicer3_HOME"));
//   if (home.isEmpty())
//     {
//     home = slicerBinDir + "/..";
//     //slicerHome = vtksys::SystemTools::CollapseFullPath(home.c_str());
//     QString homeEnv = "Slicer3_HOME=%1";
//     qDebug() << "Set environment: " << homeEnv.arg(home);
//     //vtkKWApplication::PutEnv(const_cast <char *> (homeEnv.c_str()));
//     }
//   return 
// }
// }

//----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  qSlicerApplication app(argc, argv);

  // Only need to call Q_INIT_RESOURCE with static libs
#if defined(WIN32) && defined(VTKSLICER_STATIC)
  //Q_INIT_RESOURCE(qSlicerQT);
  //Q_INIT_RESOURCE(qSlicerBaseGUIQT);
  //Q_INIT_RESOURCE(qCTKWidgets);
  //Q_INIT_RESOURCE(qMRMLWidgets);
#endif

  QPixmap pixmap(":Images/SlicerSplashScreen.png");
  QSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);
  splash.show();
  
  //app.setOrganizationName("");
  //app.setApplicationName("");
  //app.setApplicationVersion();
  //app.setWindowIcon(QIcon(":Icons/..."));

  app.initialize();

  qSlicerModuleManager * moduleManager = qSlicerApplication::application()->moduleManager();
  Q_ASSERT(moduleManager);

  qSlicerModuleFactoryManager * moduleFactoryManager = moduleManager->factoryManager();
  
  // Register module factories
  moduleFactoryManager->registerFactory("qSlicerCoreModuleFactory",
                                        new qSlicerCoreModuleFactory());
  moduleFactoryManager->registerFactory("qSlicerLoadableModuleFactory",
                                        new qSlicerLoadableModuleFactory());
  moduleFactoryManager->registerFactory("qSlicerCLILoadableModuleFactory",
                                        new qSlicerCLILoadableModuleFactory());
  moduleFactoryManager->registerFactory("qSlicerCLIExecutableModuleFactory",
                                        new qSlicerCLIExecutableModuleFactory());

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
    splash.showMessage("Loading module " + name, Qt::AlignBottom | Qt::AlignHCenter);
    splash.repaint();
    }

  splash.clearMessage();
  
  // Show main window
  window.show();
  splash.finish(&window);
  
  // Add modules to the selector
  window.moduleSelector()->addModules(moduleNames);

//   qMRMLEventLoggerWidget logger;
//   logger.setMRMLScene(qSlicerApplication::application()->mrmlScene());
//   
//   QObject::connect(qSlicerApplication::application(),
//                    SIGNAL(currentMRMLSceneChanged(vtkMRMLScene*)),
//                    &logger,
//                    SLOT(setMRMLScene(vtkMRMLScene*)));
// 
//   logger.show();
  
  return app.exec();
}

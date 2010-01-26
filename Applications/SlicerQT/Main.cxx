#include "qSlicerApplication.h"

// SlicerQT includes
#include "qSlicerModuleManager.h"
#include "qSlicerModulePanel.h"
#include "qSlicerModuleFactory.h"
#include "qSlicerMainWindow.h"
#include "qSlicerModuleSelectorWidget.h"

// qMRML includes
#include <qMRMLEventLoggerWidget.h>

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

  app.initializeLoadableModulesPaths();
  app.initializeCmdLineModulesPaths();

  qSlicerModuleManager * moduleManager = qSlicerApplication::application()->moduleManager();
  Q_ASSERT(moduleManager);
  
  moduleManager->factory()->registerCoreModules();
  moduleManager->factory()->registerLoadableModules();
  moduleManager->factory()->registerCmdLineModules();

  moduleManager->factory()->instantiateCoreModules();
  moduleManager->factory()->instantiateLoadableModules();
  moduleManager->factory()->instantiateCmdLineModules();
  
  // Create main window
  qSlicerMainWindow window;
  
  // Load all available modules
  QStringList moduleNames = moduleManager->factory()->moduleNames();
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

  qMRMLEventLoggerWidget logger;
  logger.setMRMLScene(qSlicerApplication::application()->mrmlScene());
  
  QObject::connect(qSlicerApplication::application(),
                   SIGNAL(currentMRMLSceneChanged(vtkMRMLScene*)),
                   &logger,
                   SLOT(setMRMLScene(vtkMRMLScene*)));

  logger.show();
  
  return app.exec();
}

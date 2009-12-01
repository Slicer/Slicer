#include "qSlicerApplication.h"

// SlicerQT includes
#include "qSlicerModuleManager.h"
#include "qSlicerModuleFactory.h"
#include "qSlicerMainWindow.h"

// QT includes
#include <QWidget>

int main(int argc, char* argv[])
{
  qSlicerApplication app(argc, argv);

  //app.setOrganizationName("");
  //app.setApplicationName("");
  //app.setApplicationVersion();
  //app.setWindowIcon(QIcon(":Icons/..."));

  app.initialize();

  app.setSlicerHome("/home/jchris/Projects/Slicer3-build/");

  app.initializeLoadableModulesPaths();
  app.initializeCmdLineModulesPaths();

  // Only need to call Q_INIT_RESOURCE with static libs
#if defined(WIN32) && defined(VTKSLICER_STATIC)
  //Q_INIT_RESOURCE(qSlicerQT);
  //Q_INIT_RESOURCE(qSlicerBaseGUIQT);
  //Q_INIT_RESOURCE(qCTKWidgets);
  //Q_INIT_RESOURCE(qMRMLWidgets);
#endif

  qSlicerModuleManager::instance()->factory()->registerCoreModules();

  qSlicerModuleManager::instance()->factory()->registerLoadableModules();

  qSlicerModuleManager::instance()->factory()->registerCmdLineModules();

  // Create and show main window
  qSlicerMainWindow window;
  window.show();

  // Show module panel - The module panel is a container for a module
  qSlicerModuleManager::instance()->setModulePanelVisible(true);

  QStringList modules;
  modules << "Volumes" << "Measurements" << "Cameras"
          << "Add Images" << "SlicerWelcome";

  foreach(const QString& module, modules)
    {
    qSlicerModuleManager::instance()->loadModule(module);
    }

  qSlicerModuleManager::instance()->showModule("Add Images");

  return app.exec();
}

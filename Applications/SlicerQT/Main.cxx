#include "qSlicerApplication.h"

#include "qSlicerModuleManager.h"
#include "qSlicerModuleFactory.h"
#include "qSlicerMainWindow.h"

#include <QWidget>

int main(int argc, char* argv[])
{
  qSlicerApplication app(argc, argv); 
  
  //app.setOrganizationName("");
  //app.setApplicationName("");
  //app.setApplicationVersion();
  //app.setWindowIcon(QIcon(":Icons/..."));
  
  app.setSlicerHome("/home/jchris/Projects/Slicer3-build/");
  
  app.initializeLoadableModulesPaths();
  
  // Only need to call Q_INIT_RESOURCE with static libs
#if defined(WIN32) && defined(VTKSLICER_STATIC) 
  //Q_INIT_RESOURCE(qSlicerQT);
  //Q_INIT_RESOURCE(qSlicerBaseGUIQT);
  //Q_INIT_RESOURCE(qCTKWidgets);
  //Q_INIT_RESOURCE(qMRMLWidgets);
#endif
  
  qSlicerModuleManager::instance()->factory()->registerLoadableModules();
  
  // Create and show main window
  qSlicerMainWindow window;
  window.show();
  
  QWidget * w = qSlicerModuleManager::instance()->loadModule("Volumes");
  if (w) { w->show(); }
  
  w = qSlicerModuleManager::instance()->loadModule("Measurements");
  if (w) { w->show(); }
  
  
    
  return app.exec();
}

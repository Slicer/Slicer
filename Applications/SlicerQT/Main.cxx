#include "qSlicerApplication.h"

// SlicerQT includes

// CTK includes
#include <ctkLogger.h>

#include "qSlicerCommandOptions.h"
#include "qSlicerModulePanel.h"
#include "qSlicerMainWindow.h"
#include "qSlicerModuleSelectorWidget.h"
#include "qSlicerModuleManager.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerCoreModuleFactory.h"
#include "qSlicerLoadableModuleFactory.h"
#include "qSlicerCLILoadableModuleFactory.h"
#include "qSlicerCLIExecutableModuleFactory.h"

#include "vtkRendererCollection.h"

// qMRML includes
// #include <qMRMLEventLoggerWidget.h>

#include "vtkRenderWindow.h"
#include "ctkVTKRenderView.h"

#include "vtkMRMLModelDisplayableManager.h"
#include "vtkMRMLDisplayableManagerFactory.h"

// QT includes
#include <QSplashScreen>
#include <QDebug>

//----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
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
  
  // Show main window
  window.show();
  if (!app.commandOptions()->noSplash())
    {
    splash.finish(&window);
    }
  
  // Add modules to the selector
  window.moduleSelector()->addModules(moduleNames);

  // This needs to be moved to a different widjet by JC
  ctkVTKRenderView* renderView = window.renderView();

  vtkRenderWindow *renderWindow = renderView->renderWindow();

  vtkMRMLDisplayableManagerFactory *displayableManagerFactory = vtkMRMLDisplayableManagerFactory::New();

  displayableManagerFactory->SetMRMLScene(app.mrmlScene());
  displayableManagerFactory->SetRenderer(renderWindow->GetRenderers()->GetFirstRenderer());
  displayableManagerFactory->SetInteractor(renderWindow->GetInteractor() );

  vtkMRMLModelDisplayableManager *modelManager = vtkMRMLModelDisplayableManager::New();
  displayableManagerFactory->RegisterDisplayableManager(modelManager);
  modelManager->Delete();


//   qMRMLEventLoggerWidget logger;
//   logger.setMRMLScene(qSlicerApplication::application()->mrmlScene());
//   
//   QObject::connect(qSlicerApplication::application(),
//                    SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
//                    &logger,
//                    SLOT(setMRMLScene(vtkMRMLScene*)));
// 
//   logger.show();
  
  return app.exec();
}

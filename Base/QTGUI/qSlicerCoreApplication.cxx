#include "qSlicerCoreApplication.h"

// SlicerQT includes
#include "qSlicerModuleManager.h"
#include "qSlicerModuleFactory.h"

// SlicerLogic includes
#include "vtkSlicerApplicationLogic.h"

// QT includes
#include <QDebug>

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLCrosshairNode.h"
#include "vtkEventBroker.h"

// VTK includes
#include "vtkSmartPointer.h"

// VTKSYS includes
#include <vtksys/SystemTools.hxx>
#include <vtksys/stl/string>

#include "vtkSlicerConfigure.h"

//-----------------------------------------------------------------------------
struct qSlicerCoreApplicationPrivate: public qCTKPrivate<qSlicerCoreApplication>
{
  QCTK_DECLARE_PUBLIC(qSlicerCoreApplication);
  qSlicerCoreApplicationPrivate()
    {
    this->AppLogic = 0;
    this->MRMLScene = 0;
    }

  ~qSlicerCoreApplicationPrivate()
    {
     if (this->AppLogic) { this->AppLogic->Delete(); }
     if (this->MRMLScene) { this->MRMLScene->Delete(); }
    }

  // Description:
  // MRMLScene and AppLogic pointers
  // Note: Since the logic and the scene should be deleted before the EventBroker,
  // they are not SmartPointer.
  vtkMRMLScene*                        MRMLScene;
  vtkSlicerApplicationLogic*           AppLogic;
  
  vtkSmartPointer<vtkEventBroker>      EventBroker;

  QString                       SlicerHome;
};

//-----------------------------------------------------------------------------
qSlicerCoreApplication::qSlicerCoreApplication(int &argc, char **argv)
  : Superclass(argc, argv)
{
  QCTK_INIT_PRIVATE(qSlicerCoreApplication);
}

//-----------------------------------------------------------------------------
qSlicerCoreApplication::~qSlicerCoreApplication()
{
  // Uninstantiate modules
  qSlicerModuleManager::instance()->factory()->uninstantiateAll();
}

//-----------------------------------------------------------------------------
qSlicerCoreApplication* qSlicerCoreApplication::application()
{
  qSlicerCoreApplication* app = qobject_cast<qSlicerCoreApplication*>(QApplication::instance());
  return app;
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::initialize()
{
  
  //this->setSlicerHome(); 
  // Take ownership of the vtkEventBroker instance.
  // Note: Since EventBroker is a SmartPointer, the object will be deleted when
  // qSlicerCoreApplicationPrivate will be deleted
  qctk_d()->EventBroker.TakeReference(vtkEventBroker::GetInstance());
  
  // Create MRML scene
  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtksys_stl::string root = vtksys::SystemTools::GetCurrentWorkingDirectory();
  scene->SetRootDirectory(root.c_str());
  vtkMRMLScene::SetActiveScene( scene );

  // Create the application Logic object,
  vtkSmartPointer<vtkSlicerApplicationLogic> appLogic =
    vtkSmartPointer<vtkSlicerApplicationLogic>::New();
  appLogic->SetMRMLScene(scene);

  // pass through event handling once without observing the scene
  // -- allows any dependent nodes to be created
  appLogic->ProcessMRMLEvents(scene, vtkCommand::ModifiedEvent, NULL);
  appLogic->SetAndObserveMRMLScene(scene);
  appLogic->CreateProcessingThread();

  // --- First scene needs a crosshair to be added manually
  vtkSmartPointer<vtkMRMLCrosshairNode> crosshair = vtkSmartPointer<vtkMRMLCrosshairNode>::New();
  crosshair->SetCrosshairName("default");
  scene->AddNode( crosshair );

  this->setMRMLScene(scene);
  this->setAppLogic(appLogic);
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setMRMLScene(vtkMRMLScene* mrmlScene)
{
  QCTK_D(qSlicerCoreApplication);
  if (d->MRMLScene == mrmlScene)
    {
    return;
    }
  d->MRMLScene = mrmlScene;
  d->MRMLScene->Register(NULL);
  emit this->currentMRMLSceneChanged(mrmlScene);
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerCoreApplication, vtkMRMLScene*, mrmlScene, MRMLScene);

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setAppLogic(vtkSlicerApplicationLogic* appLogic)
{
  QCTK_D(qSlicerCoreApplication);
  d->AppLogic = appLogic;
  d->AppLogic->Register(NULL);
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerCoreApplication, vtkSlicerApplicationLogic*, appLogic, AppLogic);

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::slicerHome()
{
  // TODO Use QCoreApplication::applicationDirPath
  return qctk_d()->SlicerHome;
}

//-----------------------------------------------------------------------------
QCTK_SET_CXX(qSlicerCoreApplication, const QString&, setSlicerHome, SlicerHome);

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::initializeLoadableModulesPaths()
{
  // On Win32, *both* paths have to be there, since scripts are installed
  // in the install location, and exec/libs are *automatically* installed
  // in intDir.
  QString defaultQTModulePaths = this->slicerHome() + Slicer3_INSTALL_QTLOADABLEMODULES_LIB_DIR;

//   if (hasIntDir)
//     {
//     defaultQTModulePaths = defaultQTModulePaths + PathSep +
//       slicerHome + "/" + Slicer3_INSTALL_QTLOADABLEMODULES_LIB_DIR + "/" + intDir;
//     }

  // add the default modules directory (based on the slicer
  // installation or build tree) to the user paths
  QString qtModulePaths = /*userModulePaths + PathSep + */defaultQTModulePaths;
  this->addLibraryPath(defaultQTModulePaths);

//   foreach (QString path, this->libraryPaths())
//     {
//     qDebug() << "libraryPath:" << path;
//     }

  QStringList paths;
  paths << qtModulePaths;
  qSlicerModuleManager::instance()->factory()->setLoadableModuleSearchPaths(paths);
  //qDebug() << "initializeLoadableModulesPaths - qtModulePaths:" << qtModulePaths;
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::initializeCmdLineModulesPaths()
{
  QString defaultCmdLineModulePaths;

  // On Win32, *both* paths have to be there, since scripts are installed
  // in the install location, and exec/libs are *automatically* installed
  // in intDir.
  defaultCmdLineModulePaths = this->slicerHome() + "/" + Slicer3_INSTALL_PLUGINS_BIN_DIR;
//   if (hasIntDir)
//     {
//     defaultQTModulePaths = defaultCmdLineModulePaths + PathSep +
//       slicerHome + "/" + Slicer3_INSTALL_PLUGINS_BIN_DIR + "/" + intDir;
//     }

  // add the default modules directory (based on the slicer
  // installation or build tree) to the user paths
  QString cmdLineModulePaths = /*userModulePaths + PathSep +*/ defaultCmdLineModulePaths;
  this->addLibraryPath(defaultCmdLineModulePaths);

  QStringList paths;
  paths << cmdLineModulePaths;
  qSlicerModuleManager::instance()->factory()->setCmdLineModuleSearchPaths(paths);
  //cout << "cmdLineModulePaths:" << cmdLineModulePaths << endl;
}

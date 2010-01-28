/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerCoreApplication.h"

// SlicerQT includes
#include "qSlicerModuleManager.h"
#include "qSlicerCoreIOManager.h"

// SlicerLogic includes
#include "vtkSlicerApplicationLogic.h"

// QT includes
#include <QVector>
#include <QStringList>
#include <QDir>
#include <QDebug>

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLCrosshairNode.h"
#include "vtkMRMLCommandLineModuleNode.h"

// VTK includes
#include "vtkSmartPointer.h"

// VTKSYS includes
#include <vtksys/SystemTools.hxx>
#include <vtksys/stl/string>

// For:
//  - Slicer3_INSTALL_QTLOADABLEMODULES_LIB_DIR
//  - Slicer3_INSTALL_PLUGINS_BIN_DIR
//  - Slicer3_INSTALL_LIB_DIR
#include "vtkSlicerConfigure.h"

//-----------------------------------------------------------------------------
class qSlicerCoreApplicationPrivate: public qCTKPrivate<qSlicerCoreApplication>
{
public:
  QCTK_DECLARE_PUBLIC(qSlicerCoreApplication);
  typedef qSlicerCoreApplicationPrivate Self; 
  qSlicerCoreApplicationPrivate()
    {
    this->AppLogic = 0;
    this->MRMLScene = 0;
    this->ModuleManager = 0;
    this->CoreIOManager = 0;
    this->Initialized = false; 
    }

  ~qSlicerCoreApplicationPrivate()
    {
    if (this->ModuleManager)
      {
      delete this->ModuleManager; 
      }
    if (this->CoreIOManager) { delete this->CoreIOManager; }
    }

  // Description:
  // Given the program name, should return Slicer Home Directory
  void discoverSlicerHomeDirectory(const QString& programName);

  // Description:
  // Given the program name, attempt to return the corresponding binary directory
  QString discoverSlicerBinDirectory(const QString& programName);

  // Description:
  // Accept argument of the form "FOO=BAR" and update the process environment
  int putEnv(const QString& value);

  // Description:
  // MRMLScene and AppLogic pointers
  vtkSmartPointer< vtkMRMLScene >               MRMLScene;
  vtkSmartPointer< vtkSlicerApplicationLogic >  AppLogic;

  QString                              SlicerHome;

  // Description:
  // ModuleManager - It should exist only one instance of the factory
  qSlicerModuleManager*                ModuleManager;

  // Description:
  // IOManager - It should exist only one instance of the factory
  qSlicerCoreIOManager*                CoreIOManager;

  // For ::PutEnv
  // See http://groups.google.com/group/comp.unix.wizards/msg/f0915a043bf259fa?dmode=source
  struct DeletingCharVector : public QVector<char*>
  {
    ~DeletingCharVector()
      {
      for (int i = 0; i < this->size(); ++i)
        {
        delete []this->at(i); 
        }
      }
  };

  // On windows platform, after the method 'discoverSlicerBinDirectory' has been called,
  // IntDir should be set to either Debug,
  // Release, RelWithDebInfo, MinSizeRel or any other custom build type.
  QString            IntDir;

  // Indicate if initialize() method has been called.
  bool               Initialized; 
  
};

//-----------------------------------------------------------------------------
qSlicerCoreApplication::qSlicerCoreApplication(int &_argc, char **_argv):Superclass(_argc, _argv)
{
  QCTK_INIT_PRIVATE(qSlicerCoreApplication);
}

//-----------------------------------------------------------------------------
qSlicerCoreApplication::~qSlicerCoreApplication()
{
}

//-----------------------------------------------------------------------------
qSlicerCoreApplication* qSlicerCoreApplication::application()
{
  qSlicerCoreApplication* app = qobject_cast<qSlicerCoreApplication*>(QApplication::instance());
  return app;
}

//-----------------------------------------------------------------------------
QCTK_SET_CXX(qSlicerCoreApplication, bool, setInitialized, Initialized);
QCTK_GET_CXX(qSlicerCoreApplication, bool, initialized, Initialized);

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::initialize()
{
  QCTK_D(qSlicerCoreApplication);
  d->discoverSlicerHomeDirectory(this->arguments().at(0));
  
  // Create MRML scene
  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtksys_stl::string root = vtksys::SystemTools::GetCurrentWorkingDirectory();
  scene->SetRootDirectory(root.c_str());
  vtkMRMLScene::SetActiveScene( scene );


  // Register the node type for the command line modules
  vtkSmartPointer<vtkMRMLCommandLineModuleNode> clmNode =
    vtkSmartPointer<vtkMRMLCommandLineModuleNode>::New();
  scene->RegisterNodeClass(clmNode);

  // Create the application Logic object,
  vtkSmartPointer<vtkSlicerApplicationLogic> _appLogic =
    vtkSmartPointer<vtkSlicerApplicationLogic>::New();
  _appLogic->SetMRMLScene(scene);

  // pass through event handling once without observing the scene
  // -- allows any dependent nodes to be created
  _appLogic->ProcessMRMLEvents(scene, vtkCommand::ModifiedEvent, NULL);
  _appLogic->SetAndObserveMRMLScene(scene);
  _appLogic->CreateProcessingThread();

  // --- First scene needs a crosshair to be added manually
  vtkSmartPointer<vtkMRMLCrosshairNode> crosshair = vtkSmartPointer<vtkMRMLCrosshairNode>::New();
  crosshair->SetCrosshairName("default");
  scene->AddNode( crosshair );

  this->setMRMLScene(scene);
  this->setAppLogic(_appLogic);

  // Initialization done !
  d->Initialized = true;

  qSlicerModuleManager * _moduleManager = new qSlicerModuleManager;
  Q_ASSERT(_moduleManager);
  d->ModuleManager = _moduleManager;
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::initializePaths(const QString& programPath)
{
  QCTK_D(qSlicerCoreApplication);
  // we can't use this->arguments().at(0) here as argc/argv are incorrect.
  d->discoverSlicerHomeDirectory(programPath);
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerCoreApplication, QString, intDir, IntDir);

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setMRMLScene(vtkMRMLScene* _mrmlScene)
{
  QCTK_D(qSlicerCoreApplication);
  if (d->MRMLScene == _mrmlScene)
    {
    return;
    }

  d->MRMLScene = _mrmlScene;
  
  emit this->currentMRMLSceneChanged(_mrmlScene);
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerCoreApplication, vtkMRMLScene*, mrmlScene, MRMLScene);

//-----------------------------------------------------------------------------
QCTK_SET_CXX(qSlicerCoreApplication, vtkSlicerApplicationLogic*, setAppLogic, AppLogic);
QCTK_GET_CXX(qSlicerCoreApplication, vtkSlicerApplicationLogic*, appLogic, AppLogic);

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::slicerHome() const
{
  // TODO Use QCoreApplication::applicationDirPath
  return qctk_d()->SlicerHome;
}

//-----------------------------------------------------------------------------
QCTK_SET_CXX(qSlicerCoreApplication, const QString&, setSlicerHome, SlicerHome);

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::tempDirectory() const
{
  return QDir::tempPath();
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setModuleManager(qSlicerModuleManager* _moduleManager)
{
  QCTK_D(qSlicerCoreApplication);
  if (_moduleManager == d->ModuleManager)
    {
    return; 
    }
  if (d->ModuleManager)
    {
    delete d->ModuleManager;
    d->ModuleManager = 0; 
    }
  if (_moduleManager)
    {
    d->ModuleManager = _moduleManager;
    }
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerCoreApplication, qSlicerModuleManager*, moduleManager, ModuleManager);

//-----------------------------------------------------------------------------
QCTK_SET_CXX(qSlicerCoreApplication, qSlicerCoreIOManager*, setCoreIOManager, CoreIOManager);
QCTK_GET_CXX(qSlicerCoreApplication, qSlicerCoreIOManager*, coreIOManager, CoreIOManager);

//-----------------------------------------------------------------------------
// qSlicerCoreApplicationPrivate methods

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::discoverSlicerHomeDirectory(const QString& programName)
{
  QString slicerBinDir = this->discoverSlicerBinDirectory(programName);
  if (slicerBinDir.isEmpty())
    {
    qWarning() << "Failed to retrieve Slicer binary directory.";
    return;
    }
    
  this->SlicerHome = QString::fromStdString(
    vtksys::SystemTools::CollapseFullPath((slicerBinDir + "/..").toLatin1()));
    
  // set the Slicer3_HOME variable if it doesn't already exist from the launcher 
  if (QString::fromLatin1(getenv("Slicer3_HOME")) != this->SlicerHome)
    {
    // Update env
    QString homeEnv = "Slicer3_HOME=%1";
    qDebug() << "Set environment: " << homeEnv.arg(this->SlicerHome);
    this->putEnv(homeEnv.arg(this->SlicerHome));
    }
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplicationPrivate::discoverSlicerBinDirectory(const QString& programName)
{
  std::string programPath;
  std::string errorMessage;
  if ( !vtksys::SystemTools::FindProgramPath(programName.toLatin1(), programPath, errorMessage) )
    {
    qCritical() << "Cannot find Slicer3 executable - " << errorMessage.c_str();
    return "";
    }

  std::string slicerBinDir = vtksys::SystemTools::GetFilenamePath(programPath.c_str());
  
  // If the path: [slicerBinDir + Slicer3_INSTALL_LIB_DIR] isn't valid, try to
  // discover the appropriate one
  std::string tmpName = slicerBinDir + "/../" + Slicer3_INSTALL_LIB_DIR;
  if ( !vtksys::SystemTools::FileExists(tmpName.c_str()) )
    {
    // Handle Visual Studio IntDir
    std::vector<std::string> pathComponents;
    vtksys::SystemTools::SplitPath(slicerBinDir.c_str(), pathComponents);

    slicerBinDir = slicerBinDir + "/..";
    tmpName = slicerBinDir + "/../" + Slicer3_INSTALL_LIB_DIR;
    if ( !vtksys::SystemTools::FileExists(tmpName.c_str()) )
      {
      qCritical() << "Cannot find Slicer3 libraries";
      return "";
      }

    if (pathComponents.size() > 0)
      {
      this->IntDir = QString::fromStdString(pathComponents[pathComponents.size()-1]);
      }
    }

  slicerBinDir = vtksys::SystemTools::CollapseFullPath(slicerBinDir.c_str());
  return QString::fromStdString(slicerBinDir);
}

//-----------------------------------------------------------------------------
int qSlicerCoreApplicationPrivate::putEnv(const QString& value)
{ 
  static Self::DeletingCharVector local_environment;
  char *env_var = new char[value.size() + 1];
  strcpy(env_var, value.toLatin1());
  int ret = putenv(env_var);
  // Save the pointer in the static vector so that it can be deleted on exit
  // See http://groups.google.com/group/comp.unix.wizards/msg/f0915a043bf259fa?dmode=source
  local_environment << env_var;
  return ret == 0;
}

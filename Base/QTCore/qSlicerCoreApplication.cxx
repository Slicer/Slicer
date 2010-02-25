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
#include "qSlicerCoreCommandOptions.h"

// SlicerLogic includes
#include "vtkSlicerApplicationLogic.h"

// Slicer includes
#include "vtkSlicerVersionConfigure.h" // For Slicer3_VERSION_{MINOR, MAJOR}, Slicer3_VERSION_FULL

// qCTK includes
#include <qCTKSettings.h>

// QT includes
#include <QVector>
#include <QStringList>
#include <QDir>
#include <QTimer>
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

// STL includes
#include <memory> // auto_ptr

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
  qSlicerCoreApplicationPrivate();
  ~qSlicerCoreApplicationPrivate();

  ///
  /// Instanciate settings object
  qCTKSettings* instantiateSettings(const QString& suffix, bool useTmp);
  
  ///
  /// Given the program name, should return Slicer Home Directory
  void discoverSlicerHomeDirectory(const QString& programName);

  ///
  /// Given the program name, attempt to return the corresponding binary directory
  QString discoverSlicerBinDirectory(const QString& programName);

  ///
  /// Parse arguments
  bool parseArguments(int argc, char** argv);
  
  ///
  /// See the ExitWhenDone flag to True
  void terminate();

  ///
  /// Accept argument of the form "FOO=BAR" and update the process environment
  int putEnv(const QString& value);

  ///
  /// MRMLScene and AppLogic pointers
  vtkSmartPointer< vtkMRMLScene >               MRMLScene;
  vtkSmartPointer< vtkSlicerApplicationLogic >  AppLogic;

  QString                              SlicerHome;
  qCTKSettings*                        Settings;

  ///
  /// ModuleManager - It should exist only one instance of the factory
  std::auto_ptr<qSlicerModuleManager>       ModuleManager;

  ///
  /// IOManager - It should exist only one instance of the factory
  std::auto_ptr<qSlicerCoreIOManager>       CoreIOManager;

  ///
  /// CoreOptions - It should exist only one instance of the coreOptions
  std::auto_ptr<qSlicerCoreCommandOptions>  CoreCommandOptions;

  /// ExitWhenDone flag
  bool                                 ExitWhenDone; 

  /// For ::PutEnv
  /// See http://groups.google.com/group/comp.unix.wizards/msg/f0915a043bf259fa?dmode=source
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

  /// On windows platform, after the method 'discoverSlicerBinDirectory' has been called,
  /// IntDir should be set to either Debug,
  /// Release, RelWithDebInfo, MinSizeRel or any other custom build type.
  QString            IntDir;

  /// Indicate if initialize() method has been called.
  bool               Initialized;

  /// Local copy of the arguments
  int                Argc;
  char**             Argv;
  
};

//-----------------------------------------------------------------------------
// qSlicerCoreApplicationPrivate methods

//-----------------------------------------------------------------------------
qSlicerCoreApplicationPrivate::qSlicerCoreApplicationPrivate()
{
  this->AppLogic = 0;
  this->MRMLScene = 0;
  this->Settings = 0;
  this->Initialized = false;
  this->Argc = 0;
  this->Argv = 0;
  this->ExitWhenDone = false;
}

//-----------------------------------------------------------------------------
qSlicerCoreApplicationPrivate::~qSlicerCoreApplicationPrivate()
{
  if (this->Argc && this->Argv)
    {
    for (int i = 0; i < this->Argc; ++i)
      {
      delete [] this->Argv[i];
      }
    delete [] this->Argv;
    }
}

//-----------------------------------------------------------------------------
qCTKSettings* qSlicerCoreApplicationPrivate::instantiateSettings(const QString& suffix,
                                                                 bool useTmp)
{
  QCTK_P(qSlicerCoreApplication);

  QString settingsFileName = QString("%1-%2.%3%4").
    arg(qSlicerCoreApplication::applicationName().replace(":", "")).
    arg(QString::number(Slicer3_VERSION_MAJOR)).
    arg(QString::number(Slicer3_VERSION_MINOR)).
    arg(suffix);

  if (useTmp)
    {
    settingsFileName += "-tmp";
    }

  qCTKSettings* settings = new qCTKSettings(p->organizationName(), settingsFileName, p);

  if (useTmp)
    {
    settings->clear();
    }
  return settings;
}

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
    //qDebug() << "Set environment: " << homeEnv.arg(this->SlicerHome);
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
bool qSlicerCoreApplicationPrivate::parseArguments(int _argc, char** _argv)
{
  QCTK_P(qSlicerCoreApplication);
  
  qSlicerCoreCommandOptions* options = this->CoreCommandOptions.get();
  if (!options)
    {
    qWarning() << "Failed to parse arguments - "
                  "it seems you forgot to call setCoreCommandOptions()";
    this->terminate();
    return false;
    }
  if (!options->parse(_argc, _argv))
    {
    qCritical("Problem parsing command line arguments.  Try with --help.");
    this->terminate();
    return false;
    }

  p->handlePreApplicationCommandLineArguments();
  QTimer::singleShot(0, p, SLOT(handleCommandLineArguments()));
  
  return true; 
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::terminate()
{
  this->ExitWhenDone = true;
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

//-----------------------------------------------------------------------------
// qSlicerCoreApplication methods

//-----------------------------------------------------------------------------
qSlicerCoreApplication::qSlicerCoreApplication(int &_argc, char **_argv):Superclass(_argc, _argv)
{
  QCTK_INIT_PRIVATE(qSlicerCoreApplication);
  QCTK_D(qSlicerCoreApplication);

  this->setOrganizationName("NAMIC");
  
  // Keep a local copy of the original arguments
  char **myArgv = new char*[_argc];
  for (int i = 0; i < _argc; ++i)
    {
    int length = static_cast<int>(strlen(_argv[i])) + 1;
    myArgv[i] = new char[length];
    strncpy(myArgv[i], _argv[i], length);
    }
  d->Argc = _argc;
  d->Argv = myArgv;
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
QCTK_GET_CXX(qSlicerCoreApplication, bool, initialized, Initialized);

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::initialize(bool& exitWhenDone)
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
  d->AppLogic = _appLogic;
  
  // Initialization done !
  d->Initialized = true;

  // Instanciate moduleManager
  d->ModuleManager = std::auto_ptr<qSlicerModuleManager>(new qSlicerModuleManager);

  // Parse command line arguments
  d->parseArguments(d->Argc, d->Argv);
    
  exitWhenDone = d->ExitWhenDone;
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::handlePreApplicationCommandLineArguments()
{
  QCTK_D(qSlicerCoreApplication);
  
  qSlicerCoreCommandOptions* options = this->coreCommandOptions();
  Q_ASSERT(options);

  if (options->helpSelected())
    {
    std::cout << options->help().toStdString() << std::endl;
    d->terminate();
    return;
    }
    
  if (options->displayVersionAndExit())
    {
    std::cout << this->applicationName().toStdString() << " " << Slicer3_VERSION_FULL << std::endl;
    d->terminate();
    return;
    }

  if (options->displayProgramPathAndExit())
    {
    std::cout << this->arguments().at(0).toStdString() << std::endl;
    d->terminate();
    return;
    }

  if (options->displayHomePathAndExit())
    {
    std::cout << this->slicerHome().toStdString() << std::endl;
    d->terminate();
    return;
    }

  if (options->ignoreRest())
    {
    qDebug() << "Ignored arguments:" << options->ignoredArguments();
    return;
    }
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::handleCommandLineArguments()
{
  qSlicerCoreCommandOptions* options = this->coreCommandOptions();
  Q_ASSERT(options);
  Q_UNUSED(options);
  
}

//-----------------------------------------------------------------------------
qCTKSettings* qSlicerCoreApplication::settings()
{
  QCTK_D(qSlicerCoreApplication);

  // If required, instanciate Settings
  if(!d->Settings)
    {
    d->Settings = d->instantiateSettings("", false);
    }
  return d->Settings;
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::disableSettings()
{
  QCTK_D(qSlicerCoreApplication);
  Q_ASSERT(!d->Settings);
  
  // Instanciate empty Settings
  d->Settings = d->instantiateSettings("", true);
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::clearSettings()
{
  QCTK_D(qSlicerCoreApplication);
  Q_ASSERT(!d->Settings);
  d->Settings->clear();
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerCoreApplication, QString, intDir, IntDir);

//-----------------------------------------------------------------------------
bool qSlicerCoreApplication::isInstalled()
{
#ifdef _WIN32
  return QFile::exists(this->slicerHome()+"/bin/vtk.exe");
#else
  return QFile::exists(this->slicerHome()+"/bin/vtk");
#endif
}

#ifdef Slicer3_USE_KWWIDGETS

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::initializePaths(const QString& programPath)
{
  QCTK_D(qSlicerCoreApplication);
  // we can't use this->arguments().at(0) here as argc/argv are incorrect.
  d->discoverSlicerHomeDirectory(programPath);
}

//-----------------------------------------------------------------------------
QCTK_SET_CXX(qSlicerCoreApplication, bool, setInitialized, Initialized);

//-----------------------------------------------------------------------------
QCTK_SET_CXX(qSlicerCoreApplication, vtkSlicerApplicationLogic*, setAppLogic, AppLogic);

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setModuleManager(qSlicerModuleManager* manager)
{
  qctk_d()->ModuleManager = std::auto_ptr<qSlicerModuleManager>(manager);
}

#endif //Slicer3_USE_KWWIDGETS

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
qSlicerModuleManager* qSlicerCoreApplication::moduleManager()const
{
  return qctk_d()->ModuleManager.get();
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setCoreIOManager(qSlicerCoreIOManager* manager)
{
  qctk_d()->CoreIOManager = std::auto_ptr<qSlicerCoreIOManager>(manager);
}

//-----------------------------------------------------------------------------
qSlicerCoreIOManager* qSlicerCoreApplication::coreIOManager()const
{
  return qctk_d()->CoreIOManager.get();
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setCoreCommandOptions(qSlicerCoreCommandOptions* options)
{
  qctk_d()->CoreCommandOptions = std::auto_ptr<qSlicerCoreCommandOptions>(options);
}

//-----------------------------------------------------------------------------
qSlicerCoreCommandOptions* qSlicerCoreApplication::coreCommandOptions()const
{
  return qctk_d()->CoreCommandOptions.get();
}

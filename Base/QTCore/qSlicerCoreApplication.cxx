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
#include <QDir>
#include <QProcessEnvironment>
#include <QSettings>
#include <QSharedPointer>
#include <QStringList>
#include <QTimer>
#include <QVector>

// CTK includes
#include <ctkErrorLogModel.h>
#include <ctkErrorLogFDMessageHandler.h>
#include <ctkErrorLogQtMessageHandler.h>
#include <ctkErrorLogStreamMessageHandler.h>
#include <ctkITKErrorLogMessageHandler.h>
#include <ctkVTKErrorLogMessageHandler.h>

// For:
//  - Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR
//  - Slicer_INSTALL_PLUGINS_BIN_DIR
//  - Slicer_INSTALL_LIB_DIR
//  - Slicer_USE_PYTHONQT
#include "vtkSlicerConfigure.h"

#ifdef Slicer_USE_PYTHONQT
// PythonQt includes
#include <PythonQt.h>
#endif

// SlicerQt includes
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerCoreCommandOptions.h"
#ifdef Slicer_USE_PYTHONQT
# include "qSlicerCorePythonManager.h"
#endif

// SlicerLogic includes
#include "vtkSlicerApplicationLogic.h"
#include "vtkDataIOManagerLogic.h"

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLRemoteIOLogic.h>

// MRML includes
#include "vtkCacheManager.h"
#include "vtkDataIOManager.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLCrosshairNode.h"
#include "vtkMRMLCommandLineModuleNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtksys/SystemTools.hxx>

// Slicer includes
#include "vtkSlicerVersionConfigure.h" // For Slicer_VERSION_{MINOR, MAJOR}, Slicer_VERSION_FULL

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()
  
//-----------------------------------------------------------------------------
class qSlicerCoreApplicationPrivate
{
  Q_DECLARE_PUBLIC(qSlicerCoreApplication);
protected:
  qSlicerCoreApplication* q_ptr;
public:
  qSlicerCoreApplicationPrivate(qSlicerCoreApplication& object);
  ~qSlicerCoreApplicationPrivate();
  
  typedef qSlicerCoreApplicationPrivate Self;

  void initialize();
  
  /// Instanciate settings object
  QSettings* instantiateSettings(const QString& suffix, bool useTmp);

  /// Given the program name, should return Slicer Home Directory
  void discoverSlicerHomeDirectory();

  /// If it successfully obtains 'applicationDirPath()', sets the variable 'SlicerBin'
  /// \sa QCoreApplication::applicationDirPath
  void discoverSlicerBinDirectory();

  /// Set 'ITKFactoriesDir' variable using 'ITK_AUTOLOAD_PATH' environment variable
  void discoverITKFactoriesDirectory();

  /// Set variable 'Repository{Url, Revision, Url}' and Platform extracting information
  /// from SlicerVersion.txt
  void discoverRepository();

  /// Parse arguments
  void parseArguments();

  /// Set the ExitWhenDone flag to True
  void terminate();

  /// MRMLScene and AppLogic pointers
  vtkSmartPointer< vtkMRMLScene >               MRMLScene;
  vtkSmartPointer< vtkSlicerApplicationLogic >  AppLogic;
  vtkSmartPointer< vtkMRMLApplicationLogic >    MRMLApplicationLogic;
  vtkSmartPointer< vtkMRMLRemoteIOLogic >       MRMLRemoteIOLogic;

  /// Data manager
  vtkSmartPointer< vtkDataIOManagerLogic>       DataIOManagerLogic;
  vtkSmartPointer< vtkCacheManager>             CacheManager;

  /// SlicerBin doesn't contain Debug/Release/... (see IntDir)
  QString                                       SlicerBin;
  QString                                       SlicerHome;
  QString                                       ITKFactoriesDir;
  /// On windows platform, after the method 'discoverSlicerBinDirectory' has been called,
  /// IntDir should be set to either Debug,
  /// Release, RelWithDebInfo, MinSizeRel or any other custom build type.
  QString                                       IntDir;
  
  /// Variable set extracting information from SlicerVersion.txt
  /// \sa discoverRepository
  QString                                       RepositoryUrl;
  QString                                       RepositoryBranch;
  QString                                       RepositoryRevision;
  QString                                       Platform;
  
  QSettings*                                    Settings;

  /// ModuleManager - It should exist only one instance of the factory
  QSharedPointer<qSlicerModuleManager>       ModuleManager;

  /// CoreIOManager - It should exist only one instance of the IOManager
  QSharedPointer<qSlicerCoreIOManager>       CoreIOManager;

  /// CoreCommandOptions - It should exist only one instance of the CoreCommandOptions
  QSharedPointer<qSlicerCoreCommandOptions>  CoreCommandOptions;

  /// CoreCommandOptions - It should exist only one instance of the CoreCommandOptions
  QSharedPointer<ctkErrorLogModel>           ErrorLogModel;

  /// ExitWhenDone flag
  bool                                 ExitWhenDone;

#ifdef Slicer_USE_PYTHONQT
  /// CorePythonManager - It should exist only one instance of the CorePythonManager
  QSharedPointer<qSlicerCorePythonManager> CorePythonManager;
#endif
};

//-----------------------------------------------------------------------------
// qSlicerCoreApplicationPrivate methods

//-----------------------------------------------------------------------------
qSlicerCoreApplicationPrivate::qSlicerCoreApplicationPrivate(qSlicerCoreApplication& object)
  : q_ptr(&object)
{
  this->AppLogic = 0;
  this->MRMLScene = 0;
  this->Settings = 0;
  this->ExitWhenDone = false;
}

//-----------------------------------------------------------------------------
qSlicerCoreApplicationPrivate::~qSlicerCoreApplicationPrivate()
{
  // - The ModuleManager deals with scripted module which internally work with
  // python references. (I.e calling Py_DECREF, ...)
  // - The PythonManager takes care of initializing and terminating the
  // python embedded interpreter
  // => Di facto, it's important to make sure PythonManager is destructed
  // after the ModuleManager.
  // To do so, the associated SharedPointer are cleared in the appropriate order
  this->ModuleManager.clear();
#ifdef Slicer_USE_PYTHONQT
  this->CorePythonManager.clear();
#endif
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::initialize()
{
  Q_Q(qSlicerCoreApplication);

  QCoreApplication::setOrganizationDomain("www.na-mic.org");
  QCoreApplication::setOrganizationName("NA-MIC");

  QSettings::setDefaultFormat(QSettings::IniFormat);

  // Note: qSlicerCoreApplication class takes ownership of the ioManager and
  // will be responsible to delete it
  q->setCoreIOManager(new qSlicerCoreIOManager);
  if (q->arguments().isEmpty())
    {
    qDebug() << "qSlicerCoreApplication must be given the True argc/argv";
    }
  this->discoverSlicerBinDirectory();
  // Slicer Home Directory must be set in the constructor of qSlicerCoreApplication
  // in order to be used in the constructor of qSlicerApplication (to initialize the
  // QCoreApplication::addLibraryPath (to handle the iconengines plugin) )
  this->discoverSlicerHomeDirectory();
  this->discoverITKFactoriesDirectory();
  this->discoverRepository();

  // Qt can't set environment variables for child processes that are not QProcess.
  // As the command line modules are not QProcess and need ITK_AUTOLOAD_PATH to
  // be able to read Slicer volumes, we need to change the current process env.
  QString setEnv = QString("Slicer_HOME=") + this->SlicerHome;
  vtksys::SystemTools::PutEnv(setEnv.toLatin1());
  setEnv = QString("ITK_AUTOLOAD_PATH=") + this->ITKFactoriesDir;
  vtksys::SystemTools::PutEnv(setEnv.toLatin1());

  // Instantiate ErrorLogModel
  this->ErrorLogModel = QSharedPointer<ctkErrorLogModel>(new ctkErrorLogModel);
  this->ErrorLogModel->setLogEntryGrouping(true);
  this->ErrorLogModel->setTerminalOutputEnabled(true);

  this->ErrorLogModel->registerMsgHandler(new ctkErrorLogFDMessageHandler);
  this->ErrorLogModel->registerMsgHandler(new ctkErrorLogQtMessageHandler);
  this->ErrorLogModel->registerMsgHandler(new ctkErrorLogStreamMessageHandler);
  this->ErrorLogModel->registerMsgHandler(new ctkITKErrorLogMessageHandler);
  this->ErrorLogModel->registerMsgHandler(new ctkVTKErrorLogMessageHandler);

  this->ErrorLogModel->setAllMsgHandlerEnabled(true);

  // Create the application Logic object,
  VTK_CREATE(vtkSlicerApplicationLogic, _appLogic);
  this->AppLogic = _appLogic;
  q->qvtkConnect(this->AppLogic, vtkSlicerApplicationLogic::RequestModifiedEvent,
              q, SLOT(onSlicerApplicationLogicRequest(vtkObject*, void* , unsigned long)));
  q->qvtkConnect(this->AppLogic, vtkSlicerApplicationLogic::RequestReadDataEvent,
              q, SLOT(onSlicerApplicationLogicRequest(vtkObject*, void* , unsigned long)));
  q->qvtkConnect(this->AppLogic, vtkSlicerApplicationLogic::RequestWriteDataEvent,
              q, SLOT(onSlicerApplicationLogicRequest(vtkObject*, void* , unsigned long)));

  // pass through event handling once without observing the scene
  // -- allows any dependent nodes to be created
  // Note that Interaction and Selection Node are now created
  // in MRMLApplicationLogic.
  //_appLogic->ProcessMRMLEvents(scene, vtkCommand::ModifiedEvent, NULL);
  //_appLogic->SetAndObserveMRMLScene(scene);
  _appLogic->CreateProcessingThread();

  // Create MRMLApplicationLogic
  this->MRMLApplicationLogic = vtkSmartPointer<vtkMRMLApplicationLogic>::New();

  // Create MRMLRemoteIOLogic
  this->MRMLRemoteIOLogic = vtkSmartPointer<vtkMRMLRemoteIOLogic>::New();

  this->CacheManager = vtkSmartPointer<vtkCacheManager>::New();
  vtkNew<vtkDataIOManager> dataIOManager;
  dataIOManager->SetCacheManager(this->CacheManager);
  this->DataIOManagerLogic = vtkSmartPointer<vtkDataIOManagerLogic>::New();
  this->DataIOManagerLogic->SetApplicationLogic(this->AppLogic);
  this->DataIOManagerLogic->SetAndObserveDataIOManager(dataIOManager.GetPointer());

  // Create MRML scene
  vtkNew<vtkMRMLScene> scene;
  q->setMRMLScene(scene.GetPointer());

  // Instantiate moduleManager
  this->ModuleManager = QSharedPointer<qSlicerModuleManager>(new qSlicerModuleManager);
}

//-----------------------------------------------------------------------------
QSettings* qSlicerCoreApplicationPrivate::instantiateSettings(const QString& suffix,
                                                                bool useTmp)
{
  Q_Q(qSlicerCoreApplication);

  QString settingsFileName;
  if (useTmp)
    {
    settingsFileName = QString("%1-%2.%3%4").
      arg(qSlicerCoreApplication::applicationName().replace(":", "")).
      arg(QString::number(Slicer_VERSION_MAJOR)).
      arg(QString::number(Slicer_VERSION_MINOR)).
      arg(suffix);
    settingsFileName += "-tmp";
    }

  QSettings* settings = q->newSettings(settingsFileName);

  if (useTmp)
    {
    settings->clear();
    }
  return settings;
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::discoverSlicerHomeDirectory()
{
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  this->SlicerHome = env.value("Slicer_HOME");
  if (!this->SlicerHome.isEmpty())
    {
    return;
    }
  QDir slicerBinDir(this->SlicerBin);
  bool cdUpRes = slicerBinDir.cdUp();
  if (!cdUpRes)
    {
    qDebug() << "Warning, can't cdUp in " << slicerBinDir;
    }
  this->SlicerHome = slicerBinDir.absolutePath();
  env.insert("Slicer_HOME", this->SlicerHome);
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::discoverSlicerBinDirectory()
{
  Q_Q(qSlicerCoreApplication);
  this->SlicerBin = QString();
  this->IntDir = QString();
  QDir slicerBinDir(q->applicationDirPath());
  if ( !slicerBinDir.exists() )
    {
    qCritical() << "Cannot find Slicer3 executable" << q->applicationDirPath();
    return ;
    }
  QDir slicerLibDir = slicerBinDir;
  if (slicerLibDir.cd( QString("../") + Slicer_INSTALL_LIB_DIR))
    {
    this->SlicerBin = slicerBinDir.absolutePath();
    return ;
    }
  if (slicerLibDir.cd( QString("../../") + Slicer_INSTALL_LIB_DIR))
    {
    this->IntDir = slicerBinDir.dirName();
    slicerBinDir.cdUp();
    this->SlicerBin = slicerBinDir.absolutePath();
    return;
    }
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::discoverITKFactoriesDirectory()
{
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

  QDir itkFactoriesDir (this->SlicerHome);
  itkFactoriesDir.cd(Slicer_INSTALL_ITKFACTORIES_DIR);
  if (!this->IntDir.isEmpty())
    {
    itkFactoriesDir.cd(this->IntDir);
    }
  QString relativeAutoLoadPath = env.value("ITK_AUTOLOAD_PATH");
  if (relativeAutoLoadPath.isEmpty())
    {
    itkFactoriesDir.cd(relativeAutoLoadPath);
    }
  this->ITKFactoriesDir = itkFactoriesDir.absolutePath();
  if (!itkFactoriesDir.exists())
    {
    qWarning() << "ITK_AUTOLOAD_PATH doesn't exists:"<< this->ITKFactoriesDir;
    }
  env.insert("ITK_AUTOLOAD_PATH", this->ITKFactoriesDir);
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::discoverRepository()
{
  QDir libSlicerDir(this->SlicerHome);
  libSlicerDir.cd(Slicer_INSTALL_LIB_DIR);
  QFileInfo slicerVersion(libSlicerDir, "SlicerVersion.txt");
  QFile slicerVersionFile(slicerVersion.absoluteFilePath());
  if (!slicerVersionFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    qWarning() << "Can't find SlicerVersion.txt at address:"
               << slicerVersionFile.fileName();
    return;
    }

  QTextStream slicerVersionStream(&slicerVersionFile);
  QString build, buildDate, repositoryUrl, repositoryRevision;
  slicerVersionStream >> build >> this->Platform;
  slicerVersionStream >> buildDate >> buildDate;
  slicerVersionStream >> repositoryUrl >> this->RepositoryUrl;
  slicerVersionStream >> repositoryRevision >> this->RepositoryRevision;

  this->RepositoryBranch = QFileInfo(this->RepositoryUrl).fileName();
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::parseArguments()
{
  Q_Q(qSlicerCoreApplication);

  qSlicerCoreCommandOptions* options = this->CoreCommandOptions.data();
  if (!options)
    {
    qWarning() << "Failed to parse arguments - "
                  "it seems you forgot to call setCoreCommandOptions()";
    this->terminate();
    return;
    }
  if (!options->parse(q->arguments()))
    {
    qCritical("Problem parsing command line arguments.  Try with --help.");
    this->terminate();
    return;
    }

  q->handlePreApplicationCommandLineArguments();
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::terminate()
{
  this->ExitWhenDone = true;
}

//-----------------------------------------------------------------------------
// qSlicerCoreApplication methods

//-----------------------------------------------------------------------------
qSlicerCoreApplication::qSlicerCoreApplication(int &_argc, char **_argv):Superclass(_argc, _argv)
  , d_ptr(new qSlicerCoreApplicationPrivate(*this))
{
  Q_D(qSlicerCoreApplication);
  d->initialize();
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
void qSlicerCoreApplication::setAttribute(qSlicerCoreApplication::ApplicationAttribute attribute, bool on)
{
  QCoreApplication::setAttribute(static_cast<Qt::ApplicationAttribute>(attribute), on);
}

//-----------------------------------------------------------------------------
bool qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::ApplicationAttribute attribute)
{
  return QCoreApplication::testAttribute(static_cast<Qt::ApplicationAttribute>(attribute));
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::parseArguments(bool& exitWhenDone)
{
  Q_D(qSlicerCoreApplication);
  d->parseArguments();
  exitWhenDone = d->ExitWhenDone;
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::handlePreApplicationCommandLineArguments()
{
  Q_D(qSlicerCoreApplication);
  
  qSlicerCoreCommandOptions* options = this->coreCommandOptions();
  Q_ASSERT(options);

  if (options->displayHelpAndExit())
    {
    std::cout << qPrintable(options->helpText()) << std::endl;
    d->terminate();
    return;
    }
    
  if (options->displayVersionAndExit())
    {
    std::cout << qPrintable(this->applicationName() + " " +
                            this->applicationVersion()) << std::endl;
    d->terminate();
    return;
    }

  if (options->displayProgramPathAndExit())
    {
    std::cout << qPrintable(this->arguments().at(0)) << std::endl;
    d->terminate();
    return;
    }

  if (options->displayHomePathAndExit())
    {
    std::cout << qPrintable(this->slicerHome()) << std::endl;
    d->terminate();
    return;
    }

  if (options->displaySettingsPathAndExit())
    {
    std::cout << qPrintable(this->settings()->fileName()) << std::endl;
    d->terminate();
    return;
    }

  if (options->ignoreRest())
    {
    qDebug() << "Ignored arguments:" << options->unparsedArguments();
    return;
    }
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::handleCommandLineArguments()
{
  qSlicerCoreCommandOptions* options = this->coreCommandOptions();
  Q_ASSERT(options);

#ifdef Slicer_USE_PYTHONQT

  if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
    {
    // Note that 'pythonScript' is ignored if 'extraPythonScript' is specified
    QString pythonScript = options->pythonScript();
    QString extraPythonScript = options->extraPythonScript();
    if(!extraPythonScript.isEmpty())
      {
      pythonScript = extraPythonScript;
      }

    // Set 'argv' so that python script can retrieve its associated arguments
    int pythonArgc = 1 /*scriptname*/ + options->unparsedArguments().count();
    char** pythonArgv = new char*[pythonArgc];
    pythonArgv[0] = new char[pythonScript.size() + 1];
    strcpy(pythonArgv[0], pythonScript.toAscii().data());
    for(int i = 0; i < options->unparsedArguments().count(); ++i)
      {
      pythonArgv[i + 1] = new char[options->unparsedArguments().at(i).size() + 1];
      strcpy(pythonArgv[i + 1], options->unparsedArguments().at(i).toAscii().data());
      }

    // See http://docs.python.org/c-api/init.html
    PySys_SetArgvEx(pythonArgc, pythonArgv, /*updatepath=*/false);

    // Clean memory
    for(int i = 0; i < pythonArgc; ++i){ delete[] pythonArgv[i];}
    delete[] pythonArgv;

    // Attempt to load Slicer RC file only if 'display...AndExit' options are not True
    if (!(options->displayHelpAndExit() ||
        options->displayHomePathAndExit() ||
        options->displayProgramPathAndExit() ||
        options->displaySettingsPathAndExit() ||
        options->displayVersionAndExit() ||
        options->ignoreSlicerRC()))
      {
      this->corePythonManager()->executeString("loadSlicerRCFile()");
      }

    // Execute python script
    bool exitStatus = EXIT_FAILURE;
    if(!pythonScript.isEmpty())
      {
      if (QFile::exists(pythonScript))
        {
        this->corePythonManager()->executeFile(pythonScript);
        exitStatus = this->corePythonManager()->getVariable("slicer.testing._status").toInt();
        }
      else
        {
        qWarning() << "Specified python script doesn't exist:" << pythonScript;
        }
      }
    QString pythonCode = options->pythonCode();
    if(!pythonCode.isEmpty())
      {
      this->corePythonManager()->executeString(pythonCode);
      exitStatus = this->corePythonManager()->getVariable("slicer.testing._status").toInt();
      }
    if (this->testAttribute(AA_EnableTesting))
      {
      qSlicerCoreApplication::exit(exitStatus);
      }
    }
#endif
}

//-----------------------------------------------------------------------------
QSettings* qSlicerCoreApplication::settings()const
{
  Q_D(const qSlicerCoreApplication);
  qSlicerCoreApplicationPrivate* mutable_d =
    const_cast<qSlicerCoreApplicationPrivate*>(d);
  // If required, instantiate Settings
  if(!mutable_d->Settings)
    {
    mutable_d->Settings = mutable_d->instantiateSettings("", false);
    }
  return mutable_d->Settings;
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::disableSettings()
{
  Q_D(qSlicerCoreApplication);
  Q_ASSERT(d->Settings);

  // Instanciate empty Settings
  d->Settings = d->instantiateSettings("", true);
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::clearSettings()
{
  Q_D(qSlicerCoreApplication);
  Q_ASSERT(!d->Settings);
  d->Settings->clear();
}

//-----------------------------------------------------------------------------
QSettings* qSlicerCoreApplication::newSettings(const QString& fileName)
{
  if (!fileName.isEmpty())
    {
    // Special case for tmp settings
    return new QSettings(fileName, QSettings::defaultFormat(), this);
    }
  return new QSettings(this);
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerCoreApplication, QString, intDir, IntDir);

//-----------------------------------------------------------------------------
bool qSlicerCoreApplication::isInstalled()const
{
  Q_D(const qSlicerCoreApplication);
  // TODO: make the check more robust (using an environment variable?)
  // If you change here, change also in launch.tcl.in
#ifdef _WIN32
  return QFile::exists(d->SlicerBin + "/unu.exe");
#else
  return QFile::exists(d->SlicerBin + "/unu");
#endif
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setMRMLScene(vtkMRMLScene* newMRMLScene)
{
  Q_D(qSlicerCoreApplication);
  if (d->MRMLScene == newMRMLScene)
    {
    return;
    }

  QString workingDirectory = QDir::currentPath();
  newMRMLScene->SetRootDirectory(workingDirectory.toLatin1());

  vtkMRMLScene::SetActiveScene( newMRMLScene );

  // Register the node type for the command line modules
  // TODO: should probably done in the command line logic
  vtkNew<vtkMRMLCommandLineModuleNode> clmNode;
  newMRMLScene->RegisterNodeClass(clmNode.GetPointer());

  // First scene needs a crosshair to be added manually
  vtkNew<vtkMRMLCrosshairNode> crosshair;
  crosshair->SetCrosshairName("default");
  newMRMLScene->AddNode(crosshair.GetPointer());

  if (d->AppLogic.GetPointer())
    {
    d->AppLogic->SetMRMLScene(newMRMLScene);
    }
  if (d->MRMLApplicationLogic.GetPointer())
    {
    d->MRMLApplicationLogic->SetMRMLScene(newMRMLScene);
    }
  if (d->MRMLRemoteIOLogic.GetPointer())
    {
    if (d->MRMLScene)
      {
      d->MRMLRemoteIOLogic->RemoveDataIOFromScene();
      }
    d->MRMLRemoteIOLogic->SetMRMLScene(newMRMLScene);
    }
  if (d->CacheManager.GetPointer())
    {
    d->CacheManager->SetMRMLScene(newMRMLScene);
    }
  if (d->DataIOManagerLogic.GetPointer())
    {
    d->DataIOManagerLogic->SetMRMLScene(newMRMLScene);
    }

  d->MRMLScene = newMRMLScene;

  if (d->MRMLScene)
    {
    if (d->DataIOManagerLogic.GetPointer())
      {
      d->MRMLScene->SetDataIOManager(d->DataIOManagerLogic->GetDataIOManager());
      }
    if (d->CacheManager.GetPointer())
      {
      d->MRMLScene->SetCacheManager(d->CacheManager);
      }
    if (d->MRMLRemoteIOLogic.GetPointer())
      {
      d->MRMLRemoteIOLogic->AddDataIOToScene();
      }
    }

  emit this->mrmlSceneChanged(newMRMLScene);
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerCoreApplication, vtkMRMLScene*, mrmlScene, MRMLScene);

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerCoreApplication, vtkSlicerApplicationLogic*, appLogic, AppLogic);

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerCoreApplication, vtkMRMLApplicationLogic*,
            mrmlApplicationLogic, MRMLApplicationLogic);

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::slicerHome() const
{
  Q_D(const qSlicerCoreApplication);
  // TODO Use QCoreApplication::applicationDirPath
  return d->SlicerHome;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::temporaryPath() const
{
  QSettings* appSettings = this->settings();
  Q_ASSERT(appSettings);
  QFileInfo defaultTemporaryPath(QDir::tempPath(), this->applicationName());
  QString temp = appSettings->value("TemporaryPath", defaultTemporaryPath.absoluteFilePath()).toString();
  // make sure the path exists
  if (!QDir(temp).exists())
    {
    QDir::root().mkpath(temp);
    }
  return temp;
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setTemporaryPath(const QString& path)
{
  QSettings* appSettings = this->settings();
  Q_ASSERT(appSettings);
  appSettings->setValue("TemporaryPath", path);
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::extensionsPath() const
{
  QSettings* appSettings = this->settings();
  Q_ASSERT(appSettings);
  QFileInfo defaultExtensionsPath(appSettings->fileName());
  defaultExtensionsPath = defaultExtensionsPath.absolutePath() + "/Extensions";
  QString extensions = appSettings->value("ExtensionsPath", defaultExtensionsPath.absoluteFilePath()).toString();
  // make sure the path exists
  if (!QDir(extensions).exists())
    {
    QDir::root().mkpath(extensions);
    }
  return extensions;
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setExtensionsPath(const QString& path)
{
  QSettings* appSettings = this->settings();
  Q_ASSERT(appSettings);
  appSettings->setValue("ExtensionsPath", path);
  // TODO: rescan for new extensions
}

//-----------------------------------------------------------------------------
#ifdef Slicer_USE_PYTHONQT

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setCorePythonManager(qSlicerCorePythonManager* manager)
{
  Q_D(qSlicerCoreApplication);
  d->CorePythonManager = QSharedPointer<qSlicerCorePythonManager>(manager);
}

//-----------------------------------------------------------------------------
qSlicerCorePythonManager* qSlicerCoreApplication::corePythonManager()const
{
  Q_D(const qSlicerCoreApplication);
  return d->CorePythonManager.data();
}

#endif

//-----------------------------------------------------------------------------
qSlicerModuleManager* qSlicerCoreApplication::moduleManager()const
{
  Q_D(const qSlicerCoreApplication);
  return d->ModuleManager.data();
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setCoreIOManager(qSlicerCoreIOManager* manager)
{
  Q_D(qSlicerCoreApplication);
  d->CoreIOManager = QSharedPointer<qSlicerCoreIOManager>(manager);
}

//-----------------------------------------------------------------------------
qSlicerCoreIOManager* qSlicerCoreApplication::coreIOManager()const
{
  Q_D(const qSlicerCoreApplication);
  return d->CoreIOManager.data();
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setCoreCommandOptions(qSlicerCoreCommandOptions* options)
{
  Q_D(qSlicerCoreApplication);
  d->CoreCommandOptions = QSharedPointer<qSlicerCoreCommandOptions>(options);
}

//-----------------------------------------------------------------------------
qSlicerCoreCommandOptions* qSlicerCoreApplication::coreCommandOptions()const
{
  Q_D(const qSlicerCoreApplication);
  return d->CoreCommandOptions.data();
}

//-----------------------------------------------------------------------------
ctkErrorLogModel* qSlicerCoreApplication::errorLogModel()const
{
  Q_D(const qSlicerCoreApplication);
  return d->ErrorLogModel.data();
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::copyrights()const
{
  QString copyrightsText(
    "Slicer is NOT an FDA approved medical device.\n\n"
    "See www.slicer.org for license details.\n\n"
    "See http://www.na-mic.org/Wiki/index.php/Slicer3:Acknowledgements\n\n"
    "VTK http://www.vtk.org/copyright.php\n\n"
    "ITK http://www.itk.org/HTML/Copyright.htm\n\n"
    "Tcl/Tk http://www.tcl.tk\n\n"
    "Teem:  http://teem.sf.ne\n\nt"
    "Supported by: NA-MIC, NAC, BIRN, NCIGT and the Slicer Community.\n\n"
    "Special thanks to the NIH and our other supporters.\n\n"
    "This work is part of the National Alliance for Medical Image Computing "
    "(NA-MIC), funded by the National Institutes of Health through the NIH "
    "Roadmap for Medical Research, Grant U54 EB005149. Information on the "
    "National Centers for Biomedical Computing can be obtained from "
    "http://nihroadmap.nih.gov/bioinformatics.\n\n");
  return copyrightsText;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::repositoryUrl()const
{
  Q_D(const qSlicerCoreApplication);
  return d->RepositoryUrl;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::repositoryBranch()const
{
  Q_D(const qSlicerCoreApplication);
  return d->RepositoryBranch;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::repositoryRevision()const
{
  Q_D(const qSlicerCoreApplication);
  return d->RepositoryRevision;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::platform()const
{
  Q_D(const qSlicerCoreApplication);
  return d->Platform;
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::restart()
{
  QStringList args = qSlicerCoreApplication::instance()->arguments();
  QProcess::startDetached(qSlicerCoreApplication::instance()->applicationFilePath(), args);
  QCoreApplication::quit();
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication
::onSlicerApplicationLogicRequest(vtkObject* appLogic, void* delay, unsigned long event)
{
  Q_D(qSlicerCoreApplication);
  Q_ASSERT(d->AppLogic.GetPointer() == vtkSlicerApplicationLogic::SafeDownCast(appLogic));
  Q_UNUSED(appLogic);
  Q_UNUSED(d);
  int delayInMs = *reinterpret_cast<int *>(delay);
  switch(event)
    {
    case vtkSlicerApplicationLogic::RequestModifiedEvent:
      QTimer::singleShot(delayInMs,
                         this, SLOT(processAppLogicModified()));
      break;
    case vtkSlicerApplicationLogic::RequestReadDataEvent:
      QTimer::singleShot(delayInMs,
                         this, SLOT(processAppLogicReadData()));
      break;
    case vtkSlicerApplicationLogic::RequestWriteDataEvent:
      QTimer::singleShot(delayInMs,
                         this, SLOT(processAppLogicWriteData()));
      break;
    default:
      break;
    }
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::processAppLogicModified()
{
  Q_D(qSlicerCoreApplication);
  d->AppLogic->ProcessModified();
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::processAppLogicReadData()
{
  Q_D(qSlicerCoreApplication);
  d->AppLogic->ProcessReadData();
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::processAppLogicWriteData()
{
  Q_D(qSlicerCoreApplication);
  d->AppLogic->ProcessWriteData();
}

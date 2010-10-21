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

// MRML includes
#include "vtkCacheManager.h"
#include "vtkDataIOManager.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLCrosshairNode.h"
#include "vtkMRMLCommandLineModuleNode.h"

// VTK includes
#include "vtkSmartPointer.h"
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
  
  ///
  /// Instanciate settings object
  QSettings* instantiateSettings(const QString& suffix, bool useTmp);

  ///
  /// Given the program name, should return Slicer Home Directory
  void discoverSlicerHomeDirectory();

  ///
  /// Given the program name, attempt to return the corresponding binary directory
  void discoverSlicerBinDirectory();

  void discoverITKFactoriesDirectory();

  ///
  /// Parse arguments
  bool parseArguments();

  ///
  /// Set the ExitWhenDone flag to True
  void terminate();

  ///
  /// MRMLScene and AppLogic pointers
  vtkSmartPointer< vtkMRMLScene >               MRMLScene;
  vtkSmartPointer< vtkSlicerApplicationLogic >  AppLogic;
  vtkSmartPointer< vtkMRMLApplicationLogic >    MRMLApplicationLogic;

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
  QSettings*                                    Settings;

  ///
  /// ModuleManager - It should exist only one instance of the factory
  QSharedPointer<qSlicerModuleManager>       ModuleManager;

  ///
  /// CoreIOManager - It should exist only one instance of the IOManager
  QSharedPointer<qSlicerCoreIOManager>       CoreIOManager;

  ///
  /// CoreCommandOptions - It should exist only one instance of the CoreCommandOptions
  QSharedPointer<qSlicerCoreCommandOptions>  CoreCommandOptions;

  /// ExitWhenDone flag
  bool                                 ExitWhenDone;

  /// Indicate if initialize() method has been called.
  bool               Initialized;

#ifdef Slicer_USE_PYTHONQT
  ///
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
  this->Initialized = false;
  this->ExitWhenDone = false;
}

//-----------------------------------------------------------------------------
qSlicerCoreApplicationPrivate::~qSlicerCoreApplicationPrivate()
{
}

//-----------------------------------------------------------------------------
QSettings* qSlicerCoreApplicationPrivate::instantiateSettings(const QString& suffix,
                                                                bool useTmp)
{
  Q_Q(qSlicerCoreApplication);

  QString settingsFileName = QString("%1-%2.%3%4").
    arg(qSlicerCoreApplication::applicationName().replace(":", "")).
    arg(QString::number(Slicer_VERSION_MAJOR)).
    arg(QString::number(Slicer_VERSION_MINOR)).
    arg(suffix);

  if (useTmp)
    {
    settingsFileName += "-tmp";
    }

  QSettings* settings = q->newSettings(q->organizationName(), settingsFileName);

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
bool qSlicerCoreApplicationPrivate::parseArguments()
{
  Q_Q(qSlicerCoreApplication);
  
  qSlicerCoreCommandOptions* options = this->CoreCommandOptions.data();
  if (!options)
    {
    qWarning() << "Failed to parse arguments - "
                  "it seems you forgot to call setCoreCommandOptions()";
    this->terminate();
    return false;
    }
  if (!options->parse(q->arguments()))
    {
    qCritical("Problem parsing command line arguments.  Try with --help.");
    this->terminate();
    return false;
    }

  q->handlePreApplicationCommandLineArguments();
  QTimer::singleShot(0, q, SLOT(handleCommandLineArguments()));
  
  return true; 
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

  this->setOrganizationName("NAMIC");
  this->setApplicationVersion(Slicer_VERSION_FULL);

  // Note: qSlicerCoreApplication class takes ownership of the ioManager and
  // will be responsible to delete it
  this->setCoreIOManager(new qSlicerCoreIOManager);
  if (this->arguments().isEmpty())
    {
    qDebug() << "qSlicerCoreApplication must be given the True argc/argv";
    }
  d->discoverSlicerBinDirectory();
  // Slicer Home Directory must be set in the constructor of qSlicerCoreApplication
  // in order to be used in the constructor of qSlicerApplication (to initialize the
  // QCoreApplication::addLibraryPath (to handle the iconengines plugin) )
  d->discoverSlicerHomeDirectory();
  d->discoverITKFactoriesDirectory();

  // Qt can't set environment variables for child processes that are not QProcess.
  // As the command line modules are not QProcess and need ITK_AUTOLOAD_PATH to
  // be able to read Slicer volumes, we need to change the current process env.
  QString setEnv = QString("Slicer_HOME=") + d->SlicerHome;
  vtksys::SystemTools::PutEnv(setEnv.toLatin1());
  setEnv = QString("ITK_AUTOLOAD_PATH=") + d->ITKFactoriesDir;
  vtksys::SystemTools::PutEnv(setEnv.toLatin1());
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
CTK_GET_CPP(qSlicerCoreApplication, bool, initialized, Initialized);

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::initialize(bool& exitWhenDone)
{
  Q_D(qSlicerCoreApplication);
  
  // Create MRML scene
  VTK_CREATE(vtkMRMLScene, scene);
  QString workingDirectory = QDir::currentPath();
  scene->SetRootDirectory(workingDirectory.toLatin1());
  vtkMRMLScene::SetActiveScene( scene );

  // Register the node type for the command line modules
  VTK_CREATE(vtkMRMLCommandLineModuleNode, clmNode);
  scene->RegisterNodeClass(clmNode);

  // Create the application Logic object,
  VTK_CREATE(vtkSlicerApplicationLogic, _appLogic);
  d->AppLogic = _appLogic;

  // pass through event handling once without observing the scene
  // -- allows any dependent nodes to be created
  // Note that Interaction and Selection Node are now created
  // in MRMLApplicationLogic.
  //_appLogic->ProcessMRMLEvents(scene, vtkCommand::ModifiedEvent, NULL);
  //_appLogic->SetAndObserveMRMLScene(scene);
  _appLogic->CreateProcessingThread();

  // Create MRMLApplicationLogic
  d->MRMLApplicationLogic = vtkSmartPointer<vtkMRMLApplicationLogic>::New();

  // --- First scene needs a crosshair to be added manually
  VTK_CREATE(vtkMRMLCrosshairNode, crosshair);
  crosshair->SetCrosshairName("default");
  scene->AddNode(crosshair);

  d->CacheManager = vtkSmartPointer<vtkCacheManager>::New();
  VTK_CREATE(vtkDataIOManager, dataIOManager);
  dataIOManager->SetCacheManager(d->CacheManager);
  d->DataIOManagerLogic = vtkSmartPointer<vtkDataIOManagerLogic>::New();
  d->DataIOManagerLogic->SetApplicationLogic(d->AppLogic);
  d->DataIOManagerLogic->SetAndObserveDataIOManager(dataIOManager);

  this->setMRMLScene(scene);

  // Initialization done !
  d->Initialized = true;

  // Instanciate moduleManager
  d->ModuleManager = QSharedPointer<qSlicerModuleManager>(new qSlicerModuleManager);

#ifdef Slicer_USE_PYTHONQT
  // Initialize Python
  if (this->corePythonManager())
    {
    this->corePythonManager()->mainContext();
    }
#endif

  // Parse command line arguments
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
  Q_UNUSED(options);
}

//-----------------------------------------------------------------------------
QSettings* qSlicerCoreApplication::settings()
{
  Q_D(qSlicerCoreApplication);

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
QSettings* qSlicerCoreApplication::newSettings(const QString& organization,
                                               const QString& application)
{
  return new QSettings(organization, application, this);
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerCoreApplication, QString, intDir, IntDir);

//-----------------------------------------------------------------------------
bool qSlicerCoreApplication::isInstalled()const
{
  Q_D(const qSlicerCoreApplication);
  // TODO: make the check more robust (using an environment variable?)
#ifdef _WIN32
  return QFile::exists(d->SlicerBin+"/vtkEncodeString.exe");
#else
  return QFile::exists(d->SlicerBin+"/vtkEncodeString");
#endif
}

#ifdef Slicer_USE_KWWIDGETS

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerCoreApplication, bool, setInitialized, Initialized);

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerCoreApplication, vtkSlicerApplicationLogic*, setAppLogic, AppLogic);

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setModuleManager(qSlicerModuleManager* manager)
{
  Q_D(qSlicerCoreApplication);
  d->ModuleManager = QSharedPointer<qSlicerModuleManager>(manager);
}

#endif //Slicer_USE_KWWIDGETS

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setMRMLScene(vtkMRMLScene* newMRMLScene)
{
  Q_D(qSlicerCoreApplication);
  if (d->MRMLScene == newMRMLScene)
    {
    return;
    }

  if (d->AppLogic.GetPointer())
    {
    d->AppLogic->SetMRMLScene(newMRMLScene);
    }
  if (d->MRMLApplicationLogic.GetPointer())
    {
    d->MRMLApplicationLogic->SetMRMLScene(newMRMLScene);
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
QString qSlicerCoreApplication::copyrights()const
{
  QString copyrightsText(
    "Slicer is NOT an FDA approved medical device.\n\n"
    "See www.slicer.org for license details.\n\n"
    "See http://www.na-mic.org/Wiki/index.php/Slicer3:Acknowledgements\n\n"
    "VTK http://www.vtk.org/copyright.php\n\n"
    "ITK http://www.itk.org/HTML/Copyright.htm\n\n"
    "KWWidgets http://www.kitware.com/Copyright.htm\n\n"
    "Tcl/Tk http://www.tcl.tk\n\n"
    "Teem:  http://teem.sf.ne\n\nt"
    "Supported by: NA-MIC, NAC, BIRN, NCIGT and the Slicer Community.\n\n"
    "Special thanks to the NIH and our other supporters.\n\n"
    "This work is part of the National Alliance for Medical Image Computing "
    "(NAMIC), funded by the National Institutes of Health through the NIH "
    "Roadmap for Medical Research, Grant U54 EB005149. Information on the "
    "National Centers for Biomedical Computing can be obtained from "
    "http://nihroadmap.nih.gov/bioinformatics.\n\n");
  return copyrightsText;
}

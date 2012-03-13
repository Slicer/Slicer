/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
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
#include <QMessageBox>
#include <QTimer>

// CTK includes
#include <ctkErrorLogFDMessageHandler.h>
#include <ctkErrorLogQtMessageHandler.h>
#include <ctkErrorLogStreamMessageHandler.h>
#include <ctkITKErrorLogMessageHandler.h>
#include <ctkVTKErrorLogMessageHandler.h>

// For:
//  - Slicer_QTLOADABLEMODULES_LIB_DIR
//  - Slicer_CLIMODULES_BIN_DIR
//  - Slicer_LIB_DIR
//  - Slicer_SHARE_DIR
//  - Slicer_QtPlugins_DIR
//  - Slicer_USE_PYTHONQT
//  - Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
//  - Slicer_BUILD_WIN32_CONSOLE
//  - Slicer_BUILD_CLI_SUPPORT
#include "vtkSlicerConfigure.h"

#ifdef Slicer_USE_PYTHONQT
// PythonQt includes
#include <PythonQt.h>
#endif

// SlicerQt includes
#include "qSlicerCoreApplication_p.h"
#include "qSlicerCoreCommandOptions.h"
#include "qSlicerCoreIOManager.h"
#ifdef Slicer_USE_PYTHONQT
# include "qSlicerCorePythonManager.h"
#endif
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
# include "qSlicerExtensionsManagerModel.h"
#endif
#include "qSlicerLoadableModuleFactory.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"
#include "qSlicerUtils.h"

// SlicerLogic includes
#include "vtkDataIOManagerLogic.h"

// MRMLDisplayable includes
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>

// MRMLLogic includes
#include <vtkMRMLRemoteIOLogic.h>

// MRML includes
#include <vtkCacheManager.h>
#include <vtkMRMLCrosshairNode.h>
#ifdef Slicer_BUILD_CLI_SUPPORT
# include <vtkMRMLCommandLineModuleNode.h>
#endif

// VTK includes
#include <vtkNew.h>
#include <vtksys/SystemTools.hxx>

// Slicer includes
#include "vtkSlicerVersionConfigure.h" // For Slicer_VERSION_{MINOR, MAJOR}, Slicer_VERSION_FULL

//-----------------------------------------------------------------------------
// qSlicerCoreApplicationPrivate methods

//-----------------------------------------------------------------------------
qSlicerCoreApplicationPrivate::qSlicerCoreApplicationPrivate(
  qSlicerCoreApplication& object,
  qSlicerCoreCommandOptions * coreCommandOptions,
  qSlicerCoreIOManager * coreIOManager) : q_ptr(&object)
{
  qRegisterMetaType<qSlicerCoreApplication::ReturnCode>("qSlicerCoreApplication::ReturnCode");
  this->Settings = 0;
  this->ReturnCode = qSlicerCoreApplication::ExitNotRequested;
  this->CoreCommandOptions = QSharedPointer<qSlicerCoreCommandOptions>(coreCommandOptions);
  this->CoreIOManager = QSharedPointer<qSlicerCoreIOManager>(coreIOManager);
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
void qSlicerCoreApplicationPrivate::init()
{
  Q_Q(qSlicerCoreApplication);

  if(qApp->arguments().contains("--attach-process"))
    {
    QString msg("This message box is here to give you time to attach "
                "your debugger to process [PID %1]");
    QMessageBox::information(0, "Attach process", msg.arg(QCoreApplication::applicationPid()));
    }

  // Minimize the number of call to 'systemEnvironment()' by keeping
  // a reference to 'Environment'. Indeed, re-creating QProcessEnvironment is a non-trivial
  // operation. See http://doc.qt.nokia.com/4.7/qprocessenvironment.html#systemEnvironment
  // Note also that since environment variables are set using 'setEnvironmentVariable()',
  // 'Environment' is maintained 'up-to-date'. Nevertheless, if the environment
  // is udpated solely using 'putenv(...)' function, 'Environment' won't be updated.
  this->Environment = QProcessEnvironment::systemEnvironment();

  QCoreApplication::setOrganizationDomain("www.na-mic.org");
  QCoreApplication::setOrganizationName("NA-MIC");

  QSettings::setDefaultFormat(QSettings::IniFormat);

  if (q->arguments().isEmpty())
    {
    qDebug() << "qSlicerCoreApplication must be given the True argc/argv";
    }
  this->discoverSlicerBinDirectory();

  this->SlicerHome = this->discoverSlicerHomeDirectory();
  this->setEnvironmentVariable("SLICER_HOME", this->SlicerHome);

  // Add 'SLICER_SHARE_DIR' to the environment so that Tcl scripts can reference
  // their dependencies.
  this->setEnvironmentVariable("SLICER_SHARE_DIR", Slicer_SHARE_DIR);

  this->ITKFactoriesDir = this->discoverITKFactoriesDirectory();
  this->setEnvironmentVariable("ITK_AUTOLOAD_PATH", this->ITKFactoriesDir);

  this->discoverRepository();
  this->setPythonEnvironmentVariables();
  this->setTclEnvironmentVariables();

  // Instantiate ErrorLogModel
  this->ErrorLogModel = QSharedPointer<ctkErrorLogModel>(new ctkErrorLogModel);
  this->ErrorLogModel->setLogEntryGrouping(true);
#if defined (_WIN32) && !defined (Slicer_BUILD_WIN32_CONSOLE)
  this->ErrorLogModel->setTerminalOutputs(ctkErrorLogModel::None);
#else
  this->ErrorLogModel->setTerminalOutputs(ctkErrorLogModel::All);
  this->ErrorLogModel->registerMsgHandler(new ctkErrorLogFDMessageHandler);
#endif
  this->ErrorLogModel->registerMsgHandler(new ctkErrorLogQtMessageHandler);
  this->ErrorLogModel->registerMsgHandler(new ctkErrorLogStreamMessageHandler);
  this->ErrorLogModel->registerMsgHandler(new ctkITKErrorLogMessageHandler);
  this->ErrorLogModel->registerMsgHandler(new ctkVTKErrorLogMessageHandler);
  this->ErrorLogModel->setAllMsgHandlerEnabled(true);

  // Create the application Logic object,
  this->AppLogic = vtkSmartPointer<vtkSlicerApplicationLogic>::New();
  q->qvtkConnect(this->AppLogic, vtkCommand::ModifiedEvent,
              q, SLOT(onSlicerApplicationLogicModified()));
  q->qvtkConnect(this->AppLogic, vtkSlicerApplicationLogic::RequestModifiedEvent,
              q, SLOT(onSlicerApplicationLogicRequest(vtkObject*,void*,ulong)));
  q->qvtkConnect(this->AppLogic, vtkSlicerApplicationLogic::RequestReadDataEvent,
              q, SLOT(onSlicerApplicationLogicRequest(vtkObject*,void*,ulong)));
  q->qvtkConnect(this->AppLogic, vtkSlicerApplicationLogic::RequestWriteDataEvent,
              q, SLOT(onSlicerApplicationLogicRequest(vtkObject*,void*,ulong)));
  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->SetMRMLApplicationLogic(
    this->AppLogic.GetPointer());
  vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->SetMRMLApplicationLogic(
    this->AppLogic.GetPointer());

  // pass through event handling once without observing the scene
  // -- allows any dependent nodes to be created
  // Note that Interaction and Selection Node are now created
  // in MRMLApplicationLogic.
  //this->AppLogic->ProcessMRMLEvents(scene, vtkCommand::ModifiedEvent, NULL);
  //this->AppLogic->SetAndObserveMRMLScene(scene);
  this->AppLogic->CreateProcessingThread();

  // Create MRMLRemoteIOLogic
  this->MRMLRemoteIOLogic = vtkSmartPointer<vtkMRMLRemoteIOLogic>::New();
  // Default cache location, can be changed in settings.
  this->MRMLRemoteIOLogic->GetCacheManager()->SetRemoteCacheDirectory(
    QFileInfo(q->temporaryPath(), "RemoteIO").
    absoluteFilePath().toLatin1());

  this->DataIOManagerLogic = vtkSmartPointer<vtkDataIOManagerLogic>::New();
  this->DataIOManagerLogic->SetMRMLApplicationLogic(this->AppLogic);
  this->DataIOManagerLogic->SetAndObserveDataIOManager(
    this->MRMLRemoteIOLogic->GetDataIOManager());

  // Create MRML scene
  vtkNew<vtkMRMLScene> scene;
  q->setMRMLScene(scene.GetPointer());

  // Instantiate moduleManager
  this->ModuleManager = QSharedPointer<qSlicerModuleManager>(new qSlicerModuleManager);
  this->ModuleManager->factoryManager()->setAppLogic(this->AppLogic.GetPointer());
  this->ModuleManager->factoryManager()->setMRMLScene(scene.GetPointer());
  q->connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                 this->ModuleManager->factoryManager(), SLOT(setMRMLScene(vtkMRMLScene*)));

  this->parseArguments();
#ifdef Slicer_USE_PYTHONQT
  if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
    {
    if (q->corePythonManager())
      {
      q->corePythonManager()->mainContext(); // Initialize python
      }
# ifdef Q_WS_WIN
    // HACK - Since on windows setting an environment variable using putenv doesn't propagate
    // to the environment initialized in python, let's make sure 'os.environ' is updated.
    this->updatePythonOsEnviron();
# endif
    }
#endif

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT

  this->createDirectory(q->extensionsInstallPath(), "extensions"); // Make sure the path exists

  qSlicerExtensionsManagerModel * model = new qSlicerExtensionsManagerModel(q);
  model->setLauncherSettingsFilePath(q->launcherSettingsFilePath());
  model->setSlicerRequirements(q->repositoryRevision(), q->os(), q->arch());
  q->setExtensionManagerModel(model);
  model->updateModel();
#endif
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::setupQtConf()
{
  QSettings qtConfSettings(":/qt/etc/qt.conf", QSettings::IniFormat);
  qtConfSettings.beginGroup("Paths");
  qtConfSettings.setValue("Plugins", Slicer_QtPlugins_DIR);
  qtConfSettings.endGroup();
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
QString qSlicerCoreApplicationPrivate::discoverSlicerHomeDirectory()
{
  Q_Q(qSlicerCoreApplication);

  // Since some standalone executable (i.e EMSegmentCommandLine) can create
  // an instance of qSlicer(Core)Application so that the environment and the
  // python manager are properly initialized. This executable will have
  // to set SLICER_HOME. If not, the current directory associated with that
  // executable will be considered and initialization code expecting SLICER_HOME
  // to be properly set will fail.
  QString slicerHome = this->Environment.value("SLICER_HOME");
  if (!slicerHome.isEmpty())
    {
#ifndef Q_OS_MAC
    if (this->IntDir.isEmpty())
      {
      // Additionally, in an attempt to compute InDir, let's note that its value
      // is first set with the help of "pathWithoutIntDir" in the method
      // "discoverSlicerBinDirectory" happening before this call.
      // When first called, the parameters to "pathWithoutIntDir" are "applicationDirPath" and "Slicer_BIN_DIR".
      // If set to an empty value, we should then consider the case of command line module,
      // by trying with "Slicer_CLIMODULES_BIN_DIR".
      qSlicerUtils::pathWithoutIntDir(q->applicationDirPath(), Slicer_CLIMODULES_BIN_DIR, this->IntDir);
      }
#endif

    //qDebug() << "qSlicerCoreApplication: SLICER_HOME externally set to" << slicerHome;
    return slicerHome;
    }
  QDir slicerBinDir(this->SlicerBin);
  bool cdUpRes = slicerBinDir.cdUp();
  if (!cdUpRes)
    {
    qDebug() << "Warning, can't cdUp in " << slicerBinDir;
    }
  return slicerBinDir.canonicalPath();
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::setEnvironmentVariable(const QString& key, const QString& value)
{
  Q_Q(qSlicerCoreApplication);
  q->setEnvironmentVariable(key, value);

#if defined(Slicer_USE_PYTHONQT) && defined(Q_WS_WIN)
  // Cache environment variable
  this->EnvironmentVariablesCache[key] = value;
#endif
}

//-----------------------------------------------------------------------------
#ifdef Slicer_USE_PYTHONQT
void qSlicerCoreApplicationPrivate::setPythonOsEnviron(const QString& key, const QString& value)
{
  if(!this->CorePythonManager->isPythonInitialized())
    {
    return;
    }
  this->CorePythonManager->executeString(
        QString("import os; os.environ['%1']='%2'; del os").arg(key).arg(value));
}
#endif

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::updateEnvironmentVariable(const QString& key, const QString& value,
                                                              QChar separator, bool prepend)
{
  Q_Q(qSlicerCoreApplication);
  if(q->isEnvironmentVariableValueSet(key, value))
    {
    return;
    }
  std::string currentValue;
  vtksys::SystemTools::GetEnv(key.toLatin1(), currentValue);
  if(currentValue.size() > 0)
    {
    QString updatedValue(value);
    if(prepend)
      {
      q->setEnvironmentVariable(key, updatedValue.prepend(separator).prepend(QString::fromStdString(currentValue)));
      }
    else
      {
      q->setEnvironmentVariable(key, updatedValue.append(separator).append(QString::fromStdString(currentValue)));
      }
    }
  else
    {
    q->setEnvironmentVariable(key, value);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::discoverSlicerBinDirectory()
{
  Q_Q(qSlicerCoreApplication);
  // Note: On Linux, QCoreApplication::applicationDirPath() will attempt
  //       to get the path using the "/proc" filesystem.
  if (!QFile::exists(q->applicationDirPath()))
    {
    qCritical() << "Cannot find Slicer executable" << q->applicationDirPath();
    return ;
    }
#ifndef Q_OS_MAC
  this->SlicerBin =
      qSlicerUtils::pathWithoutIntDir(q->applicationDirPath(), Slicer_BIN_DIR, this->IntDir);
#else
  // There are two cases to consider, the application could be started from:
  //   1) Install tree
  //        Application location: /path/to/Foo.app/Contents/MacOSX/myapp
  //        Binary directory:     /path/to/Foo.app/Contents/bin
  //   2) Build tree
  //        Application location: /path/to/build-dir/bin/Foo.app/Contents/MacOSX/myapp
  //        Binary directory:     /path/to/build-dir/bin
  //
  QDir slicerBinAsDir(q->applicationDirPath());
  slicerBinAsDir.cdUp(); // Move from /path/to/Foo.app/Contents/MacOSX to /path/to/Foo.app/Contents
  if(!slicerBinAsDir.cd(Slicer_BIN_DIR))
    {
    slicerBinAsDir.cdUp(); // Move from /path/to/build-dir/bin/Foo.app/Contents to /path/to/build-dir/bin/Foo.app
    slicerBinAsDir.cdUp(); // Move from /path/to/build-dir/bin/Foo.app          to /path/to/build-dir/bin
    slicerBinAsDir.cd(Slicer_BIN_DIR);
    }
  this->SlicerBin = slicerBinAsDir.path();
  Q_ASSERT(qSlicerUtils::pathEndsWith(this->SlicerBin, Slicer_BIN_DIR));
#endif
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplicationPrivate::discoverITKFactoriesDirectory()
{
  QDir itkFactoriesDir(this->SlicerHome);
  itkFactoriesDir.cd(Slicer_ITKFACTORIES_DIR);
  if (!this->IntDir.isEmpty())
    {
    itkFactoriesDir.cd(this->IntDir);
    }
  if (!itkFactoriesDir.exists())
    {
    qWarning() << "ITK_AUTOLOAD_PATH doesn't exists:"<< this->ITKFactoriesDir;
    }
  return itkFactoriesDir.absolutePath();
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::discoverRepository()
{
  QDir slicerShareDir(this->SlicerHome);
  slicerShareDir.cd(Slicer_SHARE_DIR);
  QFileInfo slicerVersion(slicerShareDir, "SlicerVersion.txt");
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
void qSlicerCoreApplicationPrivate::setPythonEnvironmentVariables()
{
#ifdef Slicer_USE_PYTHONQT
  Q_Q(qSlicerCoreApplication);
  // Set PYTHONHOME if not already done
  if (this->Environment.value("PYTHONHOME").isEmpty())
    {
    if (!q->isInstalled())
      {
      // TODO
      }
    else
      {
      qSlicerCoreApplication * app = qSlicerCoreApplication::application();
      this->setEnvironmentVariable("PYTHONHOME", app->slicerHome() + "/lib/Python");
      }
    }

  // Set PYTHONPATH if not already done
  if (this->Environment.value("PYTHONPATH").isEmpty())
    {
    this->setEnvironmentVariable(
          "PYTHONPATH", qSlicerCorePythonManager().pythonPaths().join(":"));
    }
#endif
}

//-----------------------------------------------------------------------------
#if defined(Slicer_USE_PYTHONQT) && defined(Q_WS_WIN)
void qSlicerCoreApplicationPrivate::updatePythonOsEnviron()
{
  foreach(const QString& key, this->EnvironmentVariablesCache.keys())
    {
    this->setPythonOsEnviron(key, this->EnvironmentVariablesCache.value(key));
    }
}
#endif

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::setTclEnvironmentVariables()
{
#ifdef Slicer_USE_PYTHONQT_WITH_TCL
  Q_Q(qSlicerCoreApplication);
  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  if (this->Environment.value("TCL_LIBRARY").isEmpty())
    {
    if (!q->isInstalled())
      {
      // TODO
      }
    else
      {
      this->setEnvironmentVariable(
            "TCL_LIBRARY", app->slicerHome() + "/lib/TclTk/lib/tcl"Slicer_TCL_TK_VERSION_DOT);
      }
    }
  if (this->Environment.value("TK_LIBRARY").isEmpty())
    {
    if (!q->isInstalled())
      {
      // TODO
      }
    else
      {
      this->setEnvironmentVariable(
            "TK_LIBRARY", app->slicerHome() + "/lib/TclTk/lib/tk"Slicer_TCL_TK_VERSION_DOT);
      }
    }
  if (this->Environment.value("TCLLIBPATH").isEmpty())
    {
    if (!q->isInstalled())
      {
      // TODO
      }
    else
      {
      QStringList tclLibPaths;
      tclLibPaths << app->slicerHome() + "/lib/TclTk/lib/itcl"Slicer_INCR_TCL_VERSION_DOT;
      tclLibPaths << app->slicerHome() + "/lib/TclTk/lib/itk"Slicer_INCR_TCL_VERSION_DOT;
      this->setEnvironmentVariable("TCLLIBPATH", tclLibPaths.join(" "));
      }
    }
#endif
}

//-----------------------------------------------------------------------------
bool qSlicerCoreApplicationPrivate::createDirectory(const QString& path, const QString& description) const
{
  if (path.isEmpty())
    {
    return false;
    }
  if (QDir(path).exists())
    {
    return true;
    }
  if (!QDir::root().mkpath(path))
    {
    qCritical() << QObject::tr("Failed to create %1 directory").arg(description) << path;
    return false;
    }
  return true;
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
    this->terminate(EXIT_FAILURE);
    return;
    }
  if (!options->parse(q->arguments()))
    {
    qCritical("Problem parsing command line arguments.  Try with --help.");
    this->terminate(EXIT_FAILURE);
    return;
    }

  q->handlePreApplicationCommandLineArguments();
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::terminate(int returnCode)
{
  Q_Q(qSlicerCoreApplication);
  this->ReturnCode = returnCode;
  // Does nothing if the event loop is not running
  q->exit(returnCode);
}

//-----------------------------------------------------------------------------
// qSlicerCoreApplication methods

//-----------------------------------------------------------------------------
qSlicerCoreApplication::qSlicerCoreApplication(int &_argc, char **_argv):Superclass(_argc, _argv)
  , d_ptr(new qSlicerCoreApplicationPrivate(*this, new qSlicerCoreCommandOptions, new qSlicerCoreIOManager))
{
  Q_D(qSlicerCoreApplication);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerCoreApplication::qSlicerCoreApplication(
  qSlicerCoreApplicationPrivate* pimpl, int &argc, char **argv)
  : Superclass(argc, argv), d_ptr(pimpl)
{
  // Note: You are responsible to call init() in the constructor of derived class.
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
void qSlicerCoreApplication::setEnvironmentVariable(const QString& key, const QString& value)
{
  Q_D(qSlicerCoreApplication);

  d->Environment.insert(key, value);
  // Since QProcessEnvironment can't be used to update the environment of the
  // current process, let's use 'putenv()'.
  // See http://doc.qt.nokia.com/4.6/qprocessenvironment.html#details
  vtksys::SystemTools::PutEnv(QString("%1=%2").arg(key).arg(value).toLatin1());

#ifdef Slicer_USE_PYTHONQT
  d->setPythonOsEnviron(key, value);
#endif
}

//-----------------------------------------------------------------------------
bool qSlicerCoreApplication::isEnvironmentVariableValueSet(const QString& key, const QString& value)
{
  std::string currentValue;
  vtksys::SystemTools::GetEnv(key.toLatin1(), currentValue);
  return QString::fromStdString(currentValue).contains(value);
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::prependEnvironmentVariable(const QString& key, const QString& value, QChar separator)
{
  Q_D(qSlicerCoreApplication);
  d->updateEnvironmentVariable(key, value, separator, true);
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::appendEnvironmentVariable(const QString& key, const QString& value, QChar separator)
{
  Q_D(qSlicerCoreApplication);
  d->updateEnvironmentVariable(key, value, separator, false);
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::parseArguments(bool& exitWhenDone)
{
  Q_D(qSlicerCoreApplication);
  d->parseArguments();
  exitWhenDone = (d->ReturnCode != ExitNotRequested);
}

//-----------------------------------------------------------------------------
int qSlicerCoreApplication::returnCode()const
{
  Q_D(const qSlicerCoreApplication);
  return d->ReturnCode;
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
    d->terminate(EXIT_SUCCESS);
    return;
    }

  if (options->displayVersionAndExit())
    {
    std::cout << qPrintable(this->applicationName() + " " +
                            this->applicationVersion()) << std::endl;
    d->terminate(EXIT_SUCCESS);
    return;
    }

  if (options->displayProgramPathAndExit())
    {
    std::cout << qPrintable(this->arguments().at(0)) << std::endl;
    d->terminate(EXIT_SUCCESS);
    return;
    }

  if (options->displayHomePathAndExit())
    {
    std::cout << qPrintable(this->slicerHome()) << std::endl;
    d->terminate(EXIT_SUCCESS);
    return;
    }

  if (options->displaySettingsPathAndExit())
    {
    std::cout << qPrintable(this->settings()->fileName()) << std::endl;
    d->terminate(EXIT_SUCCESS);
    return;
    }

  if (options->ignoreRest())
    {
    qDebug() << "Ignored arguments:" << options->unparsedArguments();
    return;
    }

  if (options->isTestingEnabled())
    {
    this->setAttribute(AA_EnableTesting);
    // Change the application name to change what settings file to use (to
    // prevent conflicts with user settings).
    // \todo improve settings switch mechanism.
    this->setApplicationName(this->applicationName() + "Testing");
    }
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::handleCommandLineArguments()
{
  qSlicerCoreCommandOptions* options = this->coreCommandOptions();

#ifndef Slicer_USE_PYTHONQT
  Q_UNUSED(options);
#else
  if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
    {
    // Note that 'pythonScript' is ignored if 'extraPythonScript' is specified
    QString pythonScript = options->pythonScript();
    QString extraPythonScript = options->extraPythonScript();
    QStringList scriptArgs = options->unparsedArguments();
    if(!extraPythonScript.isEmpty())
      {
      scriptArgs.removeFirst();
      pythonScript = extraPythonScript;
      }

    // Set 'argv' so that python script can retrieve its associated arguments
    int pythonArgc = 1 /*scriptname*/ + scriptArgs.count();
    char** pythonArgv = new char*[pythonArgc];
    pythonArgv[0] = new char[pythonScript.size() + 1];
    strcpy(pythonArgv[0], pythonScript.toLatin1());
    for(int i = 0; i < scriptArgs.count(); ++i)
      {
      pythonArgv[i + 1] = new char[scriptArgs.at(i).size() + 1];
      strcpy(pythonArgv[i + 1], scriptArgs.at(i).toLatin1());
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
        qApp->processEvents();
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
      qApp->processEvents();
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
  return !QFile::exists(d->SlicerHome + "/CMakeCache.txt");
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

#ifdef Slicer_BUILD_CLI_SUPPORT
  // Register the node type for the command line modules
  // TODO: should probably done in the command line logic
  vtkNew<vtkMRMLCommandLineModuleNode> clmNode;
  newMRMLScene->RegisterNodeClass(clmNode.GetPointer());
#endif

  // First scene needs a crosshair to be added manually
  vtkNew<vtkMRMLCrosshairNode> crosshair;
  crosshair->SetCrosshairName("default");
  newMRMLScene->AddNode(crosshair.GetPointer());

  if (d->AppLogic.GetPointer())
    {
    d->AppLogic->SetMRMLScene(newMRMLScene);
    }
  if (d->MRMLRemoteIOLogic.GetPointer())
    {
    if (d->MRMLScene)
      {
      d->MRMLRemoteIOLogic->RemoveDataIOFromScene();
      }
    d->MRMLRemoteIOLogic->SetMRMLScene(newMRMLScene);
    }
  if (d->DataIOManagerLogic.GetPointer())
    {
    d->DataIOManagerLogic->SetMRMLScene(newMRMLScene);
    }

  d->MRMLScene = newMRMLScene;

  if (d->MRMLScene)
    {
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
CTK_GET_CPP(qSlicerCoreApplication, vtkSlicerApplicationLogic*, applicationLogic, AppLogic);

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::slicerHome() const
{
  Q_D(const qSlicerCoreApplication);
  return d->SlicerHome;
}

//-----------------------------------------------------------------------------
bool qSlicerCoreApplication::isExtension(const QString& moduleFileName)const
{
  return vtkSlicerApplicationLogic::IsExtension(moduleFileName.toStdString(),
                                                this->slicerHome().toStdString());
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::defaultTemporaryPath() const
{
  return QFileInfo(QDir::tempPath(), this->applicationName()).absoluteFilePath();
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::temporaryPath() const
{
  Q_D(const qSlicerCoreApplication);
  QSettings* appSettings = this->settings();
  Q_ASSERT(appSettings);
  QString temporaryPath = appSettings->value("TemporaryPath", this->defaultTemporaryPath()).toString();
  d->createDirectory(temporaryPath, "temporary"); // Make sure the path exists
  return temporaryPath;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::launcherExecutableFilePath()const
{
#ifdef Q_OS_MAC
  if (this->isInstalled())
    {
    return QString();
    }
#endif
  return this->slicerHome() + "/Slicer" + qSlicerUtils::executableExtension();
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::launcherSettingsFilePath()const
{
  if (this->isInstalled())
    {
#ifdef Q_OS_MAC
    return QString();
#else
    return this->slicerHome() + "/" Slicer_BIN_DIR "/SlicerLauncherSettings.ini";
#endif
    }
  else
    {
    return this->slicerHome() + "/SlicerLauncherSettings.ini";
    }
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setTemporaryPath(const QString& path)
{
  QSettings* appSettings = this->settings();
  Q_ASSERT(appSettings);
  appSettings->setValue("TemporaryPath", path);
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::defaultExtensionsInstallPath() const
{
  QSettings* appSettings = this->settings();
  Q_ASSERT(appSettings);
  return QFileInfo(appSettings->fileName()).dir().filePath("Extensions");
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::extensionsInstallPath() const
{
  QSettings* appSettings = this->settings();
  Q_ASSERT(appSettings);
  return appSettings->value("Extensions/InstallPath", this->defaultExtensionsInstallPath()).toString();
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setExtensionsInstallPath(const QString& path)
{
  if (this->extensionsInstallPath() == path)
    {
    return;
    }
  QSettings* appSettings = this->settings();
  Q_ASSERT(appSettings);
  appSettings->setValue("Extensions/InstallPath", path);
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  this->extensionManagerModel()->updateModel();
#endif
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

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setExtensionManagerModel(qSlicerExtensionsManagerModel* model)
{
  Q_D(qSlicerCoreApplication);
  d->ExtensionsManagerModel = QSharedPointer<qSlicerExtensionsManagerModel>(model);
}

//-----------------------------------------------------------------------------
qSlicerExtensionsManagerModel* qSlicerCoreApplication::extensionManagerModel()const
{
  Q_D(const qSlicerCoreApplication);
  return d->ExtensionsManagerModel.data();
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
QString qSlicerCoreApplication::libraries()const
{
  QString librariesText(
    "Built on top of: "
    "<a href=\"http://www.vtk.org/\">VTK</a>, "
    "<a href=\"http://www.itk.org/\">ITK</a>, "
    "<a href=\"http://www.commontk.org/index.php/Main_Page\">CTK</a>, "
    "<a href=\"http://qt.nokia.com/products/\">Qt</a>, "
    "<a href=\"http://www.tcl.tk\">Tcl/Tk</a>, "
    "<a href=\"http://teem.sf.net\">Teem</a>, "
    "<a href=\"http://www.python.org/\">Python</a>, "
    "<a href=\"http://dicom.offis.de/dcmtk\">DCMTK</a>, "
    "<a href=\"http://www.jqplot.com/\">JQPlot</a><br />");
  return librariesText;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::copyrights()const
{
  QString copyrightsText(
    "<table align=\"center\" border=\"0\" width=\"80%\"><tr>"
    "<td align=\"center\"><a href=\"http://slicer.org/pages/License\">Licensing Information</a></td>"
    "<td align=\"center\"><a href=\"http://slicer.org/\">Website</a></td>"
    "<td align=\"center\"><a href=\"http://slicer.org/pages/Acknowledgments\">Acknowledgments</a></td>"
    "</tr></table>");
  return copyrightsText;
}
//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::acknowledgment()const
{
  QString acknowledgmentText(
    "Slicer is NOT an FDA approved medical device.<br /><br />"
    "Supported by: NA-MIC, NAC, BIRN, NCIGT and the Slicer Community.<br /><br />"
    "Special thanks to the NIH and our other supporters.<br /><br />"
    "This work is part of the National Alliance for Medical Image Computing "
    "(NA-MIC), funded by the National Institutes of Health through the NIH "
    "Roadmap for Medical Research, Grant U54 EB005149. Information on the "
    "National Centers for Biomedical Computing can be obtained from "
    "<a href=\"http://nihroadmap.nih.gov/bioinformatics\">http://nihroadmap.nih.gov/bioinformatics</a>.<br /><br />");
  return acknowledgmentText;
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
int qSlicerCoreApplication::majorVersion() const
{
  return Slicer_VERSION_MAJOR;
}

//-----------------------------------------------------------------------------
int qSlicerCoreApplication::minorVersion() const
{
  return Slicer_VERSION_MINOR;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::platform()const
{
  Q_D(const qSlicerCoreApplication);
  return d->Platform;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::arch()const
{
  Q_D(const qSlicerCoreApplication);
  return d->Platform.split("-").at(1);
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::os()const
{
  Q_D(const qSlicerCoreApplication);
  return d->Platform.split("-").at(0);
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::restart()
{
  qSlicerCoreApplication * coreApp = qSlicerCoreApplication::application();
  bool launcherAvailable = QFile::exists(coreApp->launcherExecutableFilePath());
  if (launcherAvailable)
    {
    QProcess::startDetached(coreApp->launcherExecutableFilePath(), coreApp->arguments());
    }
  else
    {
    QProcess::startDetached(coreApp->applicationFilePath(), coreApp->arguments());
    }
  QCoreApplication::quit();
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::onSlicerApplicationLogicModified()
{
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

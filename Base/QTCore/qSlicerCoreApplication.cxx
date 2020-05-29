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

// standard library includes
#include  <clocale>

// Qt includes
#include <QDebug>
#include <QDir>
#include <QLocale>
#include <QMessageBox>
#include <QTimer>
#include <QNetworkProxyFactory>
#include <QResource>
#include <QSettings>
#include <QTranslator>
#include <QStandardPaths>

// For:
//  - Slicer_QTLOADABLEMODULES_LIB_DIR
//  - Slicer_CLIMODULES_BIN_DIR
//  - Slicer_LIB_DIR
//  - Slicer_SHARE_DIR
//  - Slicer_USE_PYTHONQT
//  - Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
//  - Slicer_BUILD_WIN32_CONSOLE
//  - Slicer_BUILD_CLI_SUPPORT
//  - Slicer_BUILD_I18N_SUPPORT
//  - Slicer_ORGANIZATION_DOMAIN
//  - Slicer_ORGANIZATION_NAME
//  - SLICER_REVISION_SPECIFIC_USER_SETTINGS_FILEBASENAME
#include "vtkSlicerConfigure.h"

#ifdef Slicer_USE_PYTHONQT
// PythonQt includes
#include <PythonQt.h>
#endif

#ifdef Slicer_USE_PYTHONQT_WITH_OPENSSL
#include <QSslCertificate>
#include <QSslSocket>
#endif

// Slicer includes
#include "qSlicerCoreApplication_p.h"
#include "qSlicerCoreCommandOptions.h"
#include "qSlicerCoreIOManager.h"
#ifdef Slicer_USE_PYTHONQT
# include "qSlicerCorePythonManager.h"
# include "ctkPythonConsole.h"
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
#include <vtkMRMLScene.h>

// CTKLauncherLib includes
#include <ctkAppLauncherEnvironment.h>
#include <ctkAppLauncherSettings.h>

// VTK includes
#include <vtkNew.h>
#include <vtksys/SystemTools.hxx>

// VTKAddon includes
#include <vtkPersonInformation.h>

// Slicer includes
#include "vtkSlicerVersionConfigure.h" // For Slicer_VERSION_{MINOR, MAJOR}, Slicer_VERSION_FULL

#ifdef Slicer_BUILD_DICOM_SUPPORT
// XXX Avoid  warning: "HAVE_XXXX" redefined
#undef HAVE_STAT
#undef HAVE_FTIME
#undef HAVE_GETPID
#undef HAVE_IO_H
#undef HAVE_STRERROR
#undef HAVE_SYS_UTIME_H
#undef HAVE_TEMPNAM
#undef HAVE_TMPNAM
#undef HAVE_LONG_LONG
// XXX Fix windows build error
#undef HAVE_INT64_T
#include <ctkDICOMDatabase.h>
#endif

//-----------------------------------------------------------------------------
// qSlicerCoreApplicationPrivate methods

//-----------------------------------------------------------------------------
qSlicerCoreApplicationPrivate::qSlicerCoreApplicationPrivate(
  qSlicerCoreApplication& object,
  qSlicerCoreCommandOptions * coreCommandOptions,
  qSlicerCoreIOManager * coreIOManager) : q_ptr(&object)
{
  qRegisterMetaType<qSlicerCoreApplication::ReturnCode>("qSlicerCoreApplication::ReturnCode");
  this->DefaultSettings = nullptr;
  this->UserSettings = nullptr;
  this->RevisionUserSettings = nullptr;
  this->ReturnCode = qSlicerCoreApplication::ExitNotRequested;
  this->CoreCommandOptions = QSharedPointer<qSlicerCoreCommandOptions>(coreCommandOptions);
  this->CoreIOManager = QSharedPointer<qSlicerCoreIOManager>(coreIOManager);
#ifdef Slicer_BUILD_DICOM_SUPPORT
  this->DICOMDatabase = QSharedPointer<ctkDICOMDatabase>(new ctkDICOMDatabase);
#endif
  this->NextResourceHandle = 0;
}

//-----------------------------------------------------------------------------
qSlicerCoreApplicationPrivate::~qSlicerCoreApplicationPrivate()
{
  // - The ModuleManager deals with scripted module which internally work with
  // python references. (I.e calling Py_DECREF, ...)
  // - The PythonManager takes care of initializing and terminating the
  // python embedded interpreter
  // => De facto, it's important to make sure PythonManager is destructed
  // after the ModuleManager.
  // To do so, the associated SharedPointer are cleared in the appropriate order
  this->ModuleManager.clear();
  this->CoreIOManager.clear();
#ifdef Slicer_USE_PYTHONQT
  this->CorePythonManager.clear();
#endif

  this->AppLogic->TerminateProcessingThread();
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::init()
{
  Q_Q(qSlicerCoreApplication);

  // Minimize the number of call to 'systemEnvironment()' by keeping
  // a reference to 'Environment'. Indeed, re-creating QProcessEnvironment is a non-trivial
  // operation. See http://doc.qt.nokia.com/4.7/qprocessenvironment.html#systemEnvironment
  // Note also that since environment variables are set using 'setEnvironmentVariable()',
  // 'Environment' is maintained 'up-to-date'. Nevertheless, if the environment
  // is updated solely using 'putenv(...)' function, 'Environment' won't be updated.
  this->Environment = QProcessEnvironment::systemEnvironment();

  // Set the locale to be "C" to avoid issues related to reading and writing
  // of floating point numbers.  For example, when the decimal point is set to be
  // a comma instead of a period, there can be truncation of data.
  // See these previous commits and the bug report.
  // http://viewvc.slicer.org/viewvc.cgi/Slicer4?view=revision&revision=21856
  // http://viewvc.slicer.org/viewvc.cgi/Slicer4?view=revision&revision=21865
  // http://slicer-devel.65872.n3.nabble.com/Re-Rounding-to-integer-tt4027985.html
  // http://slicer-devel.65872.n3.nabble.com/Re-slicer-users-Slicer4-can-t-really-use-it-yet-td4028040.html
  // http://slicer-users.65878.n3.nabble.com/Slicer4-DICOM-many-problems-td4025919.html
  // and issue #3029
  // Set both the Qt locale, and the standard library locale to cover
  // all supported read and write methods.
  QLocale::setDefault(QLocale::C);
  setlocale(LC_ALL, "C");

  // allow a debugger to be attached during startup
  if(qApp->arguments().contains("--attach-process"))
    {
    QString msg("This message box is here to give you time to attach "
                "your debugger to process [PID %1]");
    QMessageBox::information(nullptr, "Attach process", msg.arg(QCoreApplication::applicationPid()));
    }

  QCoreApplication::setOrganizationDomain(Slicer_ORGANIZATION_DOMAIN);
  QCoreApplication::setOrganizationName(Slicer_ORGANIZATION_NAME);

  QSettings::setDefaultFormat(QSettings::IniFormat);

  if (q->arguments().isEmpty())
    {
    qDebug() << "qSlicerCoreApplication must be given the True argc/argv";
    }

  this->parseArguments();

  this->SlicerHome = this->discoverSlicerHomeDirectory();

  // Save the environment if no launcher is used (this is for example the case
  // on MacOSX when slicer is started from an install tree)
  if (ctkAppLauncherEnvironment::currentLevel() == 0)
    {
    QProcessEnvironment updatedEnv;
    ctkAppLauncherEnvironment::saveEnvironment(
          this->Environment, this->Environment.keys(), updatedEnv);
    foreach(const QString& varname, updatedEnv.keys())
      {
      q->setEnvironmentVariable(varname, updatedEnv.value(varname));
      }
    }

  q->setEnvironmentVariable("SLICER_HOME", this->SlicerHome);

  ctkAppLauncherSettings appLauncherSettings;
  appLauncherSettings.setLauncherName(q->applicationName());
  appLauncherSettings.setLauncherDir(this->SlicerHome);
  if (!appLauncherSettings.readSettings(q->launcherSettingsFilePath()))
    {
    q->showConsoleMessage(QString("Failed to read launcher settings %1").arg(q->launcherSettingsFilePath()));
    }

  // Regular environment variables
  QHash<QString, QString> envVars = appLauncherSettings.envVars();
  foreach(const QString& key, envVars.keys())
    {
    q->setEnvironmentVariable(key, envVars.value(key));
    }
  // Path environment variables (includes PATH and/or (DY)LD_LIBRARY_PATH)
  QHash<QString, QStringList> pathsEnvVars = appLauncherSettings.pathsEnvVars();
  foreach(const QString& key, pathsEnvVars.keys())
    {
    QString value = pathsEnvVars.value(key).join(appLauncherSettings.pathSep());
    if (this->Environment.contains(key))
      {
      if (!this->Environment.value(key).contains(value))
        {
        value = QString("%1%2%3").arg(value, appLauncherSettings.pathSep(), this->Environment.value(key));
        q->setEnvironmentVariable(key, value);
        }
      }
    else
      {
      q->setEnvironmentVariable(key, value);
      }
    }

#ifdef Slicer_USE_PYTHONQT_WITH_OPENSSL
  if (!QSslSocket::supportsSsl())
    {
    qWarning() << "[SSL] SSL support disabled - Failed to load SSL library !";
    }
  if (!qSlicerCoreApplication::loadCaCertificates(this->SlicerHome))
    {
    qWarning() << "[SSL] Failed to load Slicer.crt";
    }
#endif

  // Add 'SLICER_SHARE_DIR' to the environment so that Tcl scripts can reference
  // their dependencies.
  q->setEnvironmentVariable("SLICER_SHARE_DIR", Slicer_SHARE_DIR);

  // Load default settings if any.
  if (q->defaultSettings())
    {
    foreach(const QString& key, q->defaultSettings()->allKeys())
      {
      if (!q->userSettings()->contains(key))
        {
        q->userSettings()->setValue(key, q->defaultSettings()->value(key));
        }
      if (!q->revisionUserSettings()->contains(key))
        {
        q->revisionUserSettings()->setValue(key, q->defaultSettings()->value(key));
        }
      }
    }

  // Create the application Logic object,
  this->AppLogic = vtkSmartPointer<vtkSlicerApplicationLogic>::New();
  this->AppLogic->SetTemporaryPath(q->temporaryPath().toUtf8());
  vtkPersonInformation* userInfo = this->AppLogic->GetUserInformation();
  if (userInfo)
    {
    QString userInfoString = q->userSettings()->value("UserInformation").toString();
    userInfo->SetFromString(userInfoString.toUtf8().constData());
    }
  q->qvtkConnect(this->AppLogic, vtkCommand::ModifiedEvent,
              q, SLOT(onSlicerApplicationLogicModified()));
  q->qvtkConnect(this->AppLogic, vtkSlicerApplicationLogic::RequestInvokeEvent,
                 q, SLOT(requestInvokeEvent(vtkObject*,void*)), 0.0, Qt::DirectConnection);
  q->connect(q, SIGNAL(invokeEventRequested(unsigned int,void*,unsigned long,void*)),
             q, SLOT(scheduleInvokeEvent(unsigned int,void*,unsigned long,void*)), Qt::AutoConnection);
  q->qvtkConnect(this->AppLogic, vtkSlicerApplicationLogic::RequestModifiedEvent,
              q, SLOT(onSlicerApplicationLogicRequest(vtkObject*,void*,ulong)));
  q->qvtkConnect(this->AppLogic, vtkSlicerApplicationLogic::RequestReadDataEvent,
              q, SLOT(onSlicerApplicationLogicRequest(vtkObject*,void*,ulong)));
  q->qvtkConnect(this->AppLogic, vtkSlicerApplicationLogic::RequestWriteDataEvent,
              q, SLOT(onSlicerApplicationLogicRequest(vtkObject*,void*,ulong)));
  q->qvtkConnect(this->AppLogic, vtkMRMLApplicationLogic::PauseRenderEvent,
              q, SLOT(pauseRender()));
  q->qvtkConnect(this->AppLogic, vtkMRMLApplicationLogic::ResumeRenderEvent,
              q, SLOT(resumeRender()));
  q->qvtkConnect(this->AppLogic, vtkSlicerApplicationLogic::EditNodeEvent,
              q, SLOT(editNode(vtkObject*, void*, ulong)));
  q->qvtkConnect(this->AppLogic->GetUserInformation(), vtkCommand::ModifiedEvent,
    q, SLOT(onUserInformationModified()));

  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->SetMRMLApplicationLogic(
    this->AppLogic.GetPointer());
  vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->SetMRMLApplicationLogic(
    this->AppLogic.GetPointer());

  // pass through event handling once without observing the scene
  // -- allows any dependent nodes to be created
  // Note that Interaction and Selection Node are now created
  // in MRMLApplicationLogic.
  //this->AppLogic->ProcessMRMLEvents(scene, vtkCommand::ModifiedEvent, nullptr);
  //this->AppLogic->SetAndObserveMRMLScene(scene);
  this->AppLogic->CreateProcessingThread();

  // Set up Slicer to use the system proxy
  QNetworkProxyFactory::setUseSystemConfiguration(true);

  // Set up Data IO
  this->initDataIO();

  // Create MRML scene
  vtkNew<vtkMRMLScene> scene;
  q->setMRMLScene(scene.GetPointer());

  // Instantiate moduleManager
  this->ModuleManager = QSharedPointer<qSlicerModuleManager>(new qSlicerModuleManager);
  this->ModuleManager->factoryManager()->setAppLogic(this->AppLogic.GetPointer());
  this->ModuleManager->factoryManager()->setMRMLScene(scene.GetPointer());
  q->connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                 this->ModuleManager->factoryManager(), SLOT(setMRMLScene(vtkMRMLScene*)));

  q->handlePreApplicationCommandLineArguments();

#ifdef Slicer_USE_PYTHONQT
  if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
    {
    if (q->corePythonManager())
      {
      q->corePythonManager()->mainContext(); // Initialize python
      q->corePythonManager()->setSystemExitExceptionHandlerEnabled(true);
      q->connect(q->corePythonManager(), SIGNAL(systemExitExceptionRaised(int)),
                 q, SLOT(terminate(int)));
      }
    }
#endif

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT

  qSlicerExtensionsManagerModel * model = new qSlicerExtensionsManagerModel(q);
  model->setExtensionsSettingsFilePath(q->slicerRevisionUserSettingsFilePath());
  model->setExtensionsHistorySettingsFilePath(q->slicerUserSettingsFilePath());
  model->setSlicerRequirements(q->revision(), q->os(), q->arch());
  q->setExtensionsManagerModel(model);

# ifdef Q_OS_MAC
  this->createDirectory(this->defaultExtensionsInstallPathForMacOSX(), "extensions"); // Make sure the path exists
  q->addLibraryPath(this->defaultExtensionsInstallPathForMacOSX());
  q->setExtensionsInstallPath(this->defaultExtensionsInstallPathForMacOSX());
# endif

  this->createDirectory(q->extensionsInstallPath(), "extensions"); // Make sure the path exists

  model->updateModel();

  QStringList updatedExtensions;
  model->updateScheduledExtensions(updatedExtensions);
  foreach(const QString& extensionName, updatedExtensions)
    {
    qDebug() << "Successfully updated extension" << extensionName;
    }

  QStringList uninstalledExtensions;
  model->uninstallScheduledExtensions(uninstalledExtensions);
  foreach(const QString& extensionName, uninstalledExtensions)
    {
    qDebug() << "Successfully uninstalled extension" << extensionName;
    }

#endif

  if (q->userSettings()->value("Internationalization/Enabled").toBool())
    {
    // We load the language selected for the application
    qSlicerCoreApplication::loadLanguage();
    }

  q->connect(q, SIGNAL(aboutToQuit()), q, SLOT(onAboutToQuit()));
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::quickExit(int exitCode)
{
  // XXX When supporting exclusively C++11, replace with std::quick_exit
#ifdef Q_OS_WIN32
  ExitProcess(exitCode);
#else
  _exit(exitCode);
#endif
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplicationPrivate::initDataIO()
{
  Q_Q(qSlicerCoreApplication);

  // Create MRMLRemoteIOLogic
  this->MRMLRemoteIOLogic = vtkSmartPointer<vtkMRMLRemoteIOLogic>::New();
  // Default cache location, can be changed in settings.
  this->MRMLRemoteIOLogic->GetCacheManager()->SetRemoteCacheDirectory(
    QFileInfo(q->temporaryPath(), "RemoteIO").
    absoluteFilePath().toUtf8());

  this->DataIOManagerLogic = vtkSmartPointer<vtkDataIOManagerLogic>::New();
  this->DataIOManagerLogic->SetMRMLApplicationLogic(this->AppLogic);
  this->DataIOManagerLogic->SetAndObserveDataIOManager(
    this->MRMLRemoteIOLogic->GetDataIOManager());
}

//-----------------------------------------------------------------------------
QSettings* qSlicerCoreApplicationPrivate::newSettings()
{
  Q_Q(qSlicerCoreApplication);
  return new QSettings(q);
}

//-----------------------------------------------------------------------------
QSettings* qSlicerCoreApplicationPrivate::instantiateSettings(bool useTmp)
{
  Q_Q(qSlicerCoreApplication);
  if (useTmp)
    {
    q->setApplicationName(q->applicationName() + "-tmp");
    }
  QSettings* settings = this->newSettings();
  if (useTmp && !q->coreCommandOptions()->keepTemporarySettings())
    {
    settings->clear();
    }
  return settings;
}

//-----------------------------------------------------------------------------
bool qSlicerCoreApplicationPrivate::isInstalled(const QString& slicerHome)const
{
  return !QFile::exists(slicerHome + "/CMakeCache.txt");
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplicationPrivate::discoverSlicerHomeDirectory()
{
  // Since some standalone executables (i.e EMSegmentCommandLine) can create
  // an instance of qSlicer(Core)Application so that the environment and the
  // python manager are properly initialized. These executables will have
  // to set SLICER_HOME. If not, the current directory associated with that
  // executable will be considered and initialization code expecting SLICER_HOME
  // to be properly set will fail.
  QString slicerHome = this->Environment.value("SLICER_HOME");
  if (slicerHome.isEmpty())
    {
    QString slicerBin = this->discoverSlicerBinDirectory();
    QDir slicerBinDir(slicerBin);
    bool cdUpRes = slicerBinDir.cdUp();
    Q_ASSERT(cdUpRes);
    (void)cdUpRes;
    slicerHome = slicerBinDir.canonicalPath();
    }

#ifdef Q_OS_WIN32
  Q_Q(qSlicerCoreApplication);
  if (!this->isInstalled(slicerHome))
    {
    foreach(const QString& subDir,
            QStringList() << Slicer_BIN_DIR << Slicer_CLIMODULES_BIN_DIR << "Cxx")
      {
      qSlicerUtils::pathWithoutIntDir(q->applicationDirPath(), subDir, this->IntDir);
      if (!this->IntDir.isEmpty())
        {
        break;
        }
      }
    }
  Q_ASSERT(this->isInstalled(slicerHome) ? this->IntDir.isEmpty() : !this->IntDir.isEmpty());
#endif

  return slicerHome;
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
  vtksys::SystemTools::GetEnv(key.toUtf8(), currentValue);
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
QString qSlicerCoreApplicationPrivate::discoverSlicerBinDirectory()
{
  Q_Q(qSlicerCoreApplication);
  QString slicerBin;
  // Note: On Linux, QCoreApplication::applicationDirPath() will attempt
  //       to get the path using the "/proc" filesystem.
  if (!QFile::exists(q->applicationDirPath()))
    {
    q->showConsoleMessage(QString("Cannot find Slicer executable %1").arg(q->applicationDirPath()));
    return slicerBin;
    }
#ifndef Q_OS_MAC
  slicerBin =
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
  slicerBin = slicerBinAsDir.path();
#endif
  Q_ASSERT(qSlicerUtils::pathEndsWith(slicerBin, Slicer_BIN_DIR));
  return slicerBin;
}

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
//-----------------------------------------------------------------------------
QString qSlicerCoreApplicationPrivate::defaultExtensionsInstallPathForMacOSX()const
{
  Q_Q(const qSlicerCoreApplication);
  if (q->isInstalled())
    {
    QDir slicerHomeDir(q->slicerHome());
    slicerHomeDir.cdUp();
    return slicerHomeDir.absolutePath() + "/Contents/" Slicer_EXTENSIONS_DIRNAME;
    }
  else
    {
    return q->slicerHome() + "/bin/" Slicer_BUNDLE_LOCATION "/" Slicer_EXTENSIONS_DIRNAME;
    }
}
#endif

//-----------------------------------------------------------------------------
bool qSlicerCoreApplicationPrivate::isUsingLauncher()const
{
  Q_Q(const qSlicerCoreApplication);
  if (!q->isInstalled())
    {
    return true;
    }
  else
    {
#ifdef Q_OS_MAC
    return false;
#else
    return true;
#endif
    }
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
    qCritical() << qSlicerCoreApplication::tr("Failed to create %1 directory").arg(description) << path;
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
    q->showConsoleMessage("Failed to parse arguments - "
                  "it seems you forgot to call setCoreCommandOptions()");
    this->quickExit(EXIT_FAILURE);
    }
  if (!options->parse(q->arguments()))
    {
    q->showConsoleMessage("Problem parsing command line arguments.  Try with --help.");
    this->quickExit(EXIT_FAILURE);
    }
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
qSlicerCoreApplication::~qSlicerCoreApplication() = default;

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
QProcessEnvironment qSlicerCoreApplication::startupEnvironment() const
{
  return ctkAppLauncherEnvironment::environment(0);
}

//-----------------------------------------------------------------------------
QProcessEnvironment qSlicerCoreApplication::environment() const
{
  Q_D(const qSlicerCoreApplication);
  return d->Environment;
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setEnvironmentVariable(const QString& key, const QString& value)
{
  Q_D(qSlicerCoreApplication);

  d->Environment.insert(key, value);
  // Since QProcessEnvironment can't be used to update the environment of the
  // current process, let's use 'putenv()'.
  // See http://doc.qt.nokia.com/4.6/qprocessenvironment.html#details
  vtksys::SystemTools::PutEnv(QString("%1=%2").arg(key).arg(value).toUtf8().constData());

#ifdef Slicer_USE_PYTHONQT
  d->setPythonOsEnviron(key, value);
#endif
}

//-----------------------------------------------------------------------------
bool qSlicerCoreApplication::isEnvironmentVariableValueSet(const QString& key, const QString& value)
{
  std::string currentValue;
  vtksys::SystemTools::GetEnv(key.toUtf8(), currentValue);
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
int qSlicerCoreApplication::exec()
{
  int exit_code = QApplication::exec();
  if (exit_code == qSlicerCoreApplication::ExitSuccess)
    {
    int return_code = qSlicerCoreApplication::application()->returnCode();
    if (return_code != qSlicerCoreApplication::ExitNotRequested)
      {
      exit_code = return_code;
      }
    }
  return exit_code;
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::handlePreApplicationCommandLineArguments()
{
  Q_D(qSlicerCoreApplication);

  qSlicerCoreCommandOptions* options = this->coreCommandOptions();
  Q_ASSERT(options);

  if (options->displayHelpAndExit())
    {
    if(!d->isUsingLauncher())
      {
      std::cout << "Usage\n"
                << "  " << qPrintable(this->applicationName()) << " [options]\n\n"
                << "Options\n";
      }
    std::cout << qPrintable(options->helpText()) << std::endl;
    d->quickExit(EXIT_SUCCESS);
    }

  if (options->displayVersionAndExit())
    {
    std::cout << qPrintable(this->applicationName() + " " +
                            this->applicationVersion()) << std::endl;
    d->quickExit(EXIT_SUCCESS);
    }

  if (options->displayProgramPathAndExit())
    {
    std::cout << qPrintable(this->arguments().at(0)) << std::endl;
    d->quickExit(EXIT_SUCCESS);
    }

  if (options->displayHomePathAndExit())
    {
    std::cout << qPrintable(this->slicerHome()) << std::endl;
    d->quickExit(EXIT_SUCCESS);
    }

  if (options->displaySettingsPathAndExit())
    {
    std::cout << qPrintable(this->userSettings()->fileName()) << std::endl;
    d->quickExit(EXIT_SUCCESS);
    }

  if (options->displayTemporaryPathAndExit())
    {
    std::cout << qPrintable(this->temporaryPath()) << std::endl;
    d->quickExit(EXIT_SUCCESS);
    }

  if (options->ignoreRest())
    {
    qDebug() << "Ignored arguments:" << options->unparsedArguments();
    return;
    }

  if (!options->settingsDisabled() && options->keepTemporarySettings())
    {
    this->showConsoleMessage("Argument '--keep-temporary-settings' requires "
                  "'--settings-disabled' to be specified.");
    }

  if (options->isTestingEnabled())
    {
    this->setAttribute(AA_EnableTesting);
    }

#ifdef Slicer_USE_PYTHONQT
  if (options->isPythonDisabled())
    {
    this->setAttribute(AA_DisablePython);
    }
#endif
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::handleCommandLineArguments()
{
  qSlicerCoreCommandOptions* options = this->coreCommandOptions();

  QStringList unparsedArguments = options->unparsedArguments();
  if (unparsedArguments.length() > 0 &&
      options->pythonScript().isEmpty() &&
      options->extraPythonScript().isEmpty())
    {
    foreach(QString fileName, unparsedArguments)
      {
      QUrl url = QUrl(fileName);
      if (url.scheme().toLower() == this->applicationName().toLower()) // Scheme is case insensitive
        {
        emit urlReceived(fileName);
        continue;
        }

      QFileInfo file(fileName);
      if (file.exists())
        {
        qSlicerCoreIOManager* ioManager =this->coreIOManager();
        qSlicerIO::IOFileType fileType = ioManager->fileType(fileName);
        qSlicerIO::IOProperties fileProperties;
        // It is important to use absolute file path, as in the scene relative path
        // always relative to the .mrml scene file (while the user specified the path
        // relative to the current working directory)
        fileProperties.insert("fileName", file.absoluteFilePath());
        ioManager->loadNodes(fileType, fileProperties);
        }
      }
    }

#ifndef Slicer_USE_PYTHONQT
  Q_UNUSED(options);
#else
  if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
    {
    // Note that 'pythonScript' is ignored if 'extraPythonScript' is specified
    QString pythonScript = options->pythonScript();
    QString extraPythonScript = options->extraPythonScript();
    QStringList scriptArgs = options->unparsedArguments();
    // Do not pass "--attach-process", it will avoid some python script to complain about
    // unknown argument.
    scriptArgs.removeAll("--attach-process");
    if(!extraPythonScript.isEmpty())
      {
      scriptArgs.removeFirst();
      pythonScript = extraPythonScript;
      }

    // Set 'argv' so that python script can retrieve its associated arguments

    // TODO do we need validation here?

    int pythonArgc = 1 /*scriptname*/ + scriptArgs.count();
    wchar_t** pythonArgv = new wchar_t*[pythonArgc];
    //pythonArgv[0] = new wchar_t[pythonScript.size() + 1];
    //pythonScript.toWCharArray(pythonArgv[0]);
    pythonArgv[0] = Py_DecodeLocale(pythonScript.toUtf8(), nullptr);
    for(int i = 0; i < scriptArgs.count(); ++i)
      {
      //pythonArgv[i + 1] = new wchar_t[scriptArgs.at(i).size() + 1];
      //scriptArgs.at(i).toWCharArray(pythonArgv[i + 1]);
      pythonArgv[i + 1] = Py_DecodeLocale(scriptArgs.at(i).toUtf8(), nullptr);
      }

    // See http://docs.python.org/c-api/init.html
    PySys_SetArgvEx(pythonArgc, pythonArgv, /*updatepath=*/false);

    // Set 'sys.executable' so that Slicer can be used as a "regular" python interpreter
    this->corePythonManager()->executeString(
          QString("import sys; sys.executable = '%1'; del sys").arg(QStandardPaths::findExecutable("PythonSlicer"))
          );

    // Clean memory
    for (int i = 0; i < pythonArgc; i++)
      {
      PyMem_RawFree(pythonArgv[i]);
      }
    delete[] pythonArgv;
    pythonArgv = nullptr;
    pythonArgc = 0;

    // Attempt to load Slicer RC file only if 'display...AndExit' options are not True
    if (!(options->displayMessageAndExit() ||
        options->ignoreSlicerRC()))
      {
      this->corePythonManager()->executeString("loadSlicerRCFile()");
      }

    if (this->testAttribute(AA_EnableTesting))
      {
      options->setRunPythonAndExit(true);
      }

    // Execute python script
    if(!pythonScript.isEmpty())
      {
      if (QFile::exists(pythonScript))
        {
        qApp->processEvents();
        this->corePythonManager()->executeFile(pythonScript);
        }
      else
        {
        this->showConsoleMessage(QString("Specified python script doesn't exist: %1").arg(pythonScript));
        }
      }
    QString pythonCode = options->pythonCode();
    if(!pythonCode.isEmpty())
      {
      qApp->processEvents();
      this->corePythonManager()->executeString(pythonCode);
      }
    if (options->runPythonAndExit())
      {
      qSlicerCoreApplication::exit(
            this->corePythonManager()->pythonErrorOccured() ? EXIT_FAILURE : EXIT_SUCCESS);
      }
    }
#endif
}

//-----------------------------------------------------------------------------
QSettings* qSlicerCoreApplication::defaultSettings()const
{
  Q_D(const qSlicerCoreApplication);
  if (!QFile(this->slicerDefaultSettingsFilePath()).exists())
    {
    return nullptr;
    }
  qSlicerCoreApplication* mutable_self =
    const_cast<qSlicerCoreApplication*>(this);
  qSlicerCoreApplicationPrivate* mutable_d =
    const_cast<qSlicerCoreApplicationPrivate*>(d);
  // If required, instantiate Settings
  if(!mutable_d->DefaultSettings)
    {
    mutable_d->DefaultSettings =
        new QSettings(this->slicerDefaultSettingsFilePath(), QSettings::IniFormat, mutable_self);
    }
  return mutable_d->DefaultSettings;
}

//-----------------------------------------------------------------------------
QSettings* qSlicerCoreApplication::userSettings()const
{
  Q_D(const qSlicerCoreApplication);
  qSlicerCoreApplicationPrivate* mutable_d =
    const_cast<qSlicerCoreApplicationPrivate*>(d);
  // If required, instantiate Settings
  if(!mutable_d->UserSettings)
    {
    mutable_d->UserSettings = mutable_d->instantiateSettings(
          this->coreCommandOptions()->settingsDisabled());
    }
  return mutable_d->UserSettings;
}

//-----------------------------------------------------------------------------
QSettings* qSlicerCoreApplication::settings()const
{
  return this->userSettings();
}

//-----------------------------------------------------------------------------
QSettings* qSlicerCoreApplication::revisionUserSettings()const
{
  Q_D(const qSlicerCoreApplication);
  qSlicerCoreApplicationPrivate* mutable_d =
    const_cast<qSlicerCoreApplicationPrivate*>(d);
  // If required, instantiate Settings
  if(!mutable_d->RevisionUserSettings)
    {
    mutable_d->RevisionUserSettings =
        new QSettings(this->slicerRevisionUserSettingsFilePath(),
                      QSettings::IniFormat, const_cast<qSlicerCoreApplication*>(this));
    }
  return mutable_d->RevisionUserSettings;
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerCoreApplication, QString, intDir, IntDir);

//-----------------------------------------------------------------------------
bool qSlicerCoreApplication::isInstalled()const
{
  Q_D(const qSlicerCoreApplication);
  return d->isInstalled(d->SlicerHome);
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::releaseType()const
{
  return QString(Slicer_RELEASE_TYPE);
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setMRMLScene(vtkMRMLScene* newMRMLScene)
{
  Q_D(qSlicerCoreApplication);
  if (d->MRMLScene == newMRMLScene)
    {
    return;
    }

  // Set the default scene save directory
  newMRMLScene->SetRootDirectory(this->defaultScenePath().toUtf8());

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
    d->AppLogic->SetMRMLSceneDataIO(newMRMLScene, d->MRMLRemoteIOLogic.GetPointer(), d->DataIOManagerLogic.GetPointer());
    }

  d->MRMLScene = newMRMLScene;

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
QString qSlicerCoreApplication::defaultScenePath() const
{
  QSettings* appSettings = this->userSettings();
  Q_ASSERT(appSettings);
  QString defaultScenePath = appSettings->value(
        "DefaultScenePath", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();

  return defaultScenePath;
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setDefaultScenePath(const QString& path)
{
  if (this->defaultScenePath() == path)
    {
    return;
    }
  QSettings* appSettings = this->userSettings();
  Q_ASSERT(appSettings);
  appSettings->setValue("DefaultScenePath", path);
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::slicerSharePath() const
{
  return Slicer_SHARE_DIR;
}

//-----------------------------------------------------------------------------
bool qSlicerCoreApplication::isEmbeddedModule(const QString& moduleFileName)const
{
  QString slicerRevision = this->revision();
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  slicerRevision = this->extensionsManagerModel()->slicerRevision();
#endif
  return vtkSlicerApplicationLogic::IsEmbeddedModule(moduleFileName.toStdString(),
                                                     this->slicerHome().toStdString(),
                                                     slicerRevision.toStdString());
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::defaultTemporaryPath() const
{
#ifdef Q_OS_UNIX
  // In multi-user Linux environment, a single temporary directory is shared
  // by all users. We need to create a separate directory for each user,
  // as users do not have access to another user's directory.
  QString userName = qgetenv("USER");
  return QFileInfo(QDir::tempPath(), this->applicationName()+"-"+userName).absoluteFilePath();
#else
  return QFileInfo(QDir::tempPath(), this->applicationName()).absoluteFilePath();
#endif
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::temporaryPath() const
{
  Q_D(const qSlicerCoreApplication);
  QSettings* appSettings = this->userSettings();
  Q_ASSERT(appSettings);
  QString temporaryPath = appSettings->value("TemporaryPath", this->defaultTemporaryPath()).toString();
  d->createDirectory(temporaryPath, "temporary"); // Make sure the path exists
  return temporaryPath;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::launcherExecutableFilePath()const
{
  Q_D(const qSlicerCoreApplication);
  if (!d->isUsingLauncher())
    {
    return QString();
    }
  return this->slicerHome() + "/Slicer" + qSlicerUtils::executableExtension();
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::launcherSettingsFilePath()const
{
  QString appName = this->applicationName().replace("-tmp", "");
  if (this->isInstalled())
    {
    return this->slicerHome() + "/" Slicer_BIN_DIR "/" + appName + "LauncherSettings.ini";
    }
  else
    {
    return this->slicerHome() + "/" + appName + "LauncherSettings.ini";
    }
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::launcherRevisionSpecificUserSettingsFilePath()const
{
  if (this->isInstalled())
    {
#ifdef Q_OS_MAC
    return QString();
#else
    return this->slicerRevisionUserSettingsFilePath();
#endif
    }
  else
    {
    return this->slicerRevisionUserSettingsFilePath();
    }
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::slicerDefaultSettingsFilePath()const
{
  return this->slicerHome() + "/" Slicer_SHARE_DIR "/" + this->applicationName() + "DefaultSettings.ini";
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::slicerUserSettingsFilePath()const
{
  return this->userSettings()->fileName();
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::slicerRevisionUserSettingsFilePath()const
{
  QFileInfo fileInfo = QFileInfo(this->userSettings()->fileName());
  QString prefix = fileInfo.completeBaseName();
  QString suffix = "-" + this->revision();
  bool useTmp = this->coreCommandOptions()->settingsDisabled();
  if (useTmp)
    {
    suffix += "-tmp";
    useTmp = true;
    }
  QString fileName =
      QDir(fileInfo.path()).filePath(QString("%1%2%3.ini")
                                     .arg(prefix)
                                     .arg(SLICER_REVISION_SPECIFIC_USER_SETTINGS_FILEBASENAME)
                                     .arg(suffix));
  if (useTmp && !this->coreCommandOptions()->keepTemporarySettings())
    {
    QSettings(fileName, QSettings::IniFormat).clear();
    }
  return fileName;
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setTemporaryPath(const QString& path)
{
  QSettings* appSettings = this->userSettings();
  Q_ASSERT(appSettings);
  appSettings->setValue("TemporaryPath", path);
  this->applicationLogic()->SetTemporaryPath(path.toUtf8());
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::defaultExtensionsInstallPath() const
{
  QSettings* appSettings = this->userSettings();
  Q_ASSERT(appSettings);
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  return QFileInfo(appSettings->fileName()).dir().filePath(Slicer_EXTENSIONS_DIRNAME);
#else
  Q_UNUSED(appSettings);
  return QString();
#endif
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::extensionsInstallPath() const
{
  QSettings settings(this->slicerRevisionUserSettingsFilePath(), QSettings::IniFormat);
  return settings.value("Extensions/InstallPath", this->defaultExtensionsInstallPath()).toString();
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setExtensionsInstallPath(const QString& path)
{
  if (this->extensionsInstallPath() == path)
    {
    return;
    }
  this->revisionUserSettings()->setValue("Extensions/InstallPath", path);
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  Q_ASSERT(this->extensionsManagerModel());
  this->extensionsManagerModel()->updateModel();
#endif
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::gatherExtensionsHistoryInformationOnStartup()
{
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  this->extensionsManagerModel()->gatherExtensionsHistoryInformationOnStartup();
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

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setPythonConsole(ctkPythonConsole* console)
{
  Q_D(qSlicerCoreApplication);
  d->PythonConsole = console;
}

//-----------------------------------------------------------------------------
ctkPythonConsole* qSlicerCoreApplication::pythonConsole()const
{
  Q_D(const qSlicerCoreApplication);
  return d->PythonConsole.data();
}

#endif

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::setExtensionsManagerModel(qSlicerExtensionsManagerModel* model)
{
  Q_D(qSlicerCoreApplication);
  d->ExtensionsManagerModel = QSharedPointer<qSlicerExtensionsManagerModel>(model);
}

//-----------------------------------------------------------------------------
qSlicerExtensionsManagerModel* qSlicerCoreApplication::extensionsManagerModel()const
{
  Q_D(const qSlicerCoreApplication);
  return d->ExtensionsManagerModel.data();
}

#endif

//-----------------------------------------------------------------------------
ctkErrorLogAbstractModel* qSlicerCoreApplication::errorLogModel()const
{
  Q_D(const qSlicerCoreApplication);
  return d->ErrorLogModel.data();
}

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
bool qSlicerCoreApplication::isCustomMainApplication()const
{
  return (this->mainApplicationName() != QString("Slicer"));
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::mainApplicationName()const
{
  return QString(Slicer_MAIN_PROJECT_APPLICATION_NAME);
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::mainApplicationRepositoryUrl()const
{
  return QString(Slicer_MAIN_PROJECT_WC_URL);
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::mainApplicationRepositoryRevision()const
{
  return QString(Slicer_MAIN_PROJECT_WC_REVISION);
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::mainApplicationRevision()const
{
  return QString(Slicer_MAIN_PROJECT_REVISION);
}

//-----------------------------------------------------------------------------
int qSlicerCoreApplication::mainApplicationMajorVersion()const
{
  return Slicer_MAIN_PROJECT_VERSION_MAJOR;
}

//-----------------------------------------------------------------------------
int qSlicerCoreApplication::mainApplicationMinorVersion()const
{
  return Slicer_MAIN_PROJECT_VERSION_MINOR;
}

//-----------------------------------------------------------------------------
int qSlicerCoreApplication::mainApplicationPatchVersion()const
{
  return Slicer_MAIN_PROJECT_VERSION_PATCH;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::libraries()const
{
  QString librariesText(
    "Built on top of: "
    "<a href=\"http://www.vtk.org/\">VTK</a>, "
    "<a href=\"http://www.itk.org/\">ITK</a>, "
    "<a href=\"http://www.commontk.org/index.php/Main_Page\">CTK</a>, "
    "<a href=\"https://www.qt.io/\">Qt</a>, "
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
    "<td align=\"center\"><a href=\"https://www.slicer.org/wiki/License\">Licensing Information</a></td>"
    "<td align=\"center\"><a href=\"https://slicer.org/\">Website</a></td>"
    "<td align=\"center\"><a href=\"https://www.slicer.org/wiki/Documentation/4.x/Acknowledgments\">Acknowledgments</a></td>"
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
    "<a href=\"https://commonfund.nih.gov/bioinformatics\">https://commonfund.nih.gov/bioinformatics</a>.<br /><br />");
  return acknowledgmentText;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::repositoryUrl()const
{
  return Slicer_WC_URL;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::repositoryBranch()const
{
  return QFileInfo(this->repositoryUrl()).fileName();
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::repositoryRevision()const
{
  return Slicer_WC_REVISION;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::revision()const
{
  return Slicer_REVISION;
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
  return QString("%1-%2").arg(Slicer_OS).arg(Slicer_ARCHITECTURE);
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::arch()const
{
  return Slicer_ARCHITECTURE;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreApplication::os()const
{
  return Slicer_OS;
}

#ifdef Slicer_BUILD_DICOM_SUPPORT
//-----------------------------------------------------------------------------
ctkDICOMDatabase* qSlicerCoreApplication::dicomDatabase()const
{
  Q_D(const qSlicerCoreApplication);
  return d->DICOMDatabase.data();
}

//-----------------------------------------------------------------------------
QSharedPointer<ctkDICOMDatabase> qSlicerCoreApplication::dicomDatabaseShared()const
{
  Q_D(const qSlicerCoreApplication);
  return d->DICOMDatabase;
}
#endif


//-----------------------------------------------------------------------------
void qSlicerCoreApplication::restart()
{
  qSlicerCoreApplication * coreApp = qSlicerCoreApplication::application();
  bool launcherAvailable = QFile::exists(coreApp->launcherExecutableFilePath());
  QStringList arguments = coreApp->arguments();
  arguments.removeFirst(); // Remove program name
#if defined (Q_OS_WIN32) && !defined (Slicer_BUILD_WIN32_CONSOLE)
#else
  arguments.prepend("--disable-terminal-outputs");
#endif
  if (launcherAvailable)
    {
    QProcess::startDetached(coreApp->launcherExecutableFilePath(), arguments);
    }
  else
    {
    QProcess::startDetached(coreApp->applicationFilePath(), arguments);
    }
  QCoreApplication::quit();
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::onSlicerApplicationLogicModified()
{
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::onUserInformationModified()
{
  vtkPersonInformation* userInfo = this->applicationLogic()->GetUserInformation();
  if (!userInfo)
    {
    return;
    }
  this->userSettings()->setValue("UserInformation", userInfo->GetAsString().c_str());
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication
::requestInvokeEvent(vtkObject* caller, void* callData)
{
  // This method can be called by any thread.
  Q_UNUSED(caller);
  vtkMRMLApplicationLogic::InvokeRequest* request =
    reinterpret_cast<vtkMRMLApplicationLogic::InvokeRequest *>(callData);
  // If the thread is the same as the main thread then it is executed directly,
  // otherwise it is queued to be executed by the main thread.
  emit invokeEventRequested(request->Delay, request->Caller,
                            request->EventID, request->CallData);
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication
::scheduleInvokeEvent(unsigned int delay, void* caller,
                      unsigned long eventID, void* callData)
{
  QTimer* timer = new QTimer(this);
  timer->setSingleShot(true);
  timer->setProperty("caller", qVariantFromValue(caller));
  timer->setProperty("eventID", qVariantFromValue(eventID));
  timer->setProperty("callData", qVariantFromValue(callData));
  timer->connect(timer, SIGNAL(timeout()),this, SLOT(invokeEvent()));
  timer->start(delay);
}

//-----------------------------------------------------------------------------
void qSlicerCoreApplication
::invokeEvent()
{
  QTimer* timer = qobject_cast<QTimer*>(this->sender());
  Q_ASSERT(timer);
  if (!timer)
    {
    return;
    }
  QVariant callerVariant = timer->property("caller");
  QVariant eventIDVariant = timer->property("eventID");
  QVariant callDataVariant = timer->property("callData");
  vtkObject* caller =
    reinterpret_cast<vtkObject*>(callerVariant.value<void*>());
  unsigned long eventID = eventIDVariant.toULongLong();
  void* callData = callDataVariant.value<void*>();
  if (caller)
    {
    caller->InvokeEvent(eventID, callData);
    }
  timer->deleteLater();
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

//-----------------------------------------------------------------------------
void qSlicerCoreApplication::terminate(int returnCode)
{
  Q_D(qSlicerCoreApplication);
  d->ReturnCode = returnCode;
  // Does nothing if the event loop is not running
  this->exit(returnCode);
}

//----------------------------------------------------------------------------
void qSlicerCoreApplication::onAboutToQuit()
{
  Q_D(qSlicerCoreApplication);

  d->ModuleManager->factoryManager()->unloadModules();

#ifdef Slicer_USE_PYTHONQT
  // Override return code only if testing mode is enabled
  if (this->corePythonManager()->pythonErrorOccured() && this->coreCommandOptions()->isTestingEnabled())
    {
    d->ReturnCode = qSlicerCoreApplication::ExitFailure;
    }
#endif
}

//----------------------------------------------------------------------------
void qSlicerCoreApplication::loadTranslations(const QString& dir)
{
#ifdef Slicer_BUILD_I18N_SUPPORT
  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  Q_ASSERT(app);

  QString localeFilter =
      QString( QString("*") + app->settings()->value("language").toString());
  localeFilter += QString(".qm");

  QDir directory(dir);
  QStringList qmFiles = directory.entryList(QStringList(localeFilter));

  foreach(QString qmFile, qmFiles)
    {
    QTranslator* translator = new QTranslator();
    QString qmFilePath = QString(dir + QString("/") + qmFile);

    if(!translator->load(qmFilePath))
      {
      qDebug() << "The File " << qmFile << " hasn't been loaded in the translator";
      return;
      }
    app->installTranslator(translator);
    }
#else
  Q_UNUSED(dir)
#endif
}

//----------------------------------------------------------------------------
void qSlicerCoreApplication::loadLanguage()
{
#ifdef Slicer_BUILD_I18N_SUPPORT
  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  Q_ASSERT(app);

  // we check if the application is installed or not.
  if (app->isInstalled())
    {
    QString qmDir = QString(Slicer_QM_DIR);
    app->loadTranslations(qmDir);
    }
  else
    {
    QStringList qmDirs = QString(Slicer_QM_OUTPUT_DIRS).split(";");
    foreach(QString qmDir, qmDirs)
      {
      app->loadTranslations(qmDir);
      }
    }
#endif
}

//----------------------------------------------------------------------------
bool qSlicerCoreApplication::loadCaCertificates(const QString& slicerHome)
{
#ifdef Slicer_USE_PYTHONQT_WITH_OPENSSL
  if (QSslSocket::supportsSsl())
    {
    QSslSocket::setDefaultCaCertificates(
          QSslCertificate::fromPath(
            slicerHome + "/" Slicer_SHARE_DIR "/Slicer.crt"));
    }
  return !QSslSocket::defaultCaCertificates().empty();
#else
  Q_UNUSED(slicerHome);
  return false;
#endif
}

//----------------------------------------------------------------------------
int qSlicerCoreApplication::registerResource(const QByteArray& data)
{
  Q_D(qSlicerCoreApplication);

  const int handle = d->NextResourceHandle++;
  d->LoadedResources.insert(handle, data);

  const uchar* pdata =
    reinterpret_cast<const uchar*>(d->LoadedResources[handle].constData());

  if (!QResource::registerResource(pdata))
    {
    d->LoadedResources.remove(handle);
    return -1;
    }

  return handle;
}

//----------------------------------------------------------------------------
bool qSlicerCoreApplication::unregisterResource(int handle)
{
  Q_D(qSlicerCoreApplication);

  if (d->LoadedResources.contains(handle))
    {
    const uchar* pdata =
      reinterpret_cast<const uchar*>(d->LoadedResources[handle].constData());
    const bool result = QResource::unregisterResource(pdata);
    d->LoadedResources.remove(handle);
    return result;
    }

  return false;
}

// --------------------------------------------------------------------------
void qSlicerCoreApplication::addModuleAssociatedNodeType(const QString& nodeClassName, const QString& moduleName)
{
  Q_D(qSlicerCoreApplication);
  d->ModulesForNodes.insert(nodeClassName, moduleName);
}

// --------------------------------------------------------------------------
void qSlicerCoreApplication::removeModuleAssociatedNodeType(const QString& nodeClassName, const QString& moduleName)
{
  Q_D(qSlicerCoreApplication);
  d->ModulesForNodes.remove(nodeClassName, moduleName);
}

// --------------------------------------------------------------------------
QStringList qSlicerCoreApplication::modulesAssociatedWithNodeType(const QString& nodeClassName) const
{
  Q_D(const qSlicerCoreApplication);
  QList<QString> moduleNames = d->ModulesForNodes.values(nodeClassName);
  return moduleNames;
}

// --------------------------------------------------------------------------
QStringList qSlicerCoreApplication::allModuleAssociatedNodeTypes() const
{
  Q_D(const qSlicerCoreApplication);
  QList<QString> nodeClassNames = d->ModulesForNodes.uniqueKeys();
  return nodeClassNames;
}

// --------------------------------------------------------------------------
void qSlicerCoreApplication::showConsoleMessage(QString message, bool error/*=true*/) const
{
  Q_D(const qSlicerCoreApplication);
  if (error)
  {
    std::cerr << message.toLocal8Bit().constData() << std::endl;
  }
  else
  {
    std::cout << message.toLocal8Bit().constData() << std::endl;
  }
}

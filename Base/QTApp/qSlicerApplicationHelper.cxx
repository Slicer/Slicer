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

#include "qSlicerApplicationHelper.h"

// Qt includes
#include <QFont>
#include <QtGlobal> // For Q_OS_*, QT_VERSION
#include <QLabel>
#if defined(Q_OS_MACOS) && (QT_VERSION < QT_VERSION_CHECK(5, 15, 10))
# include <QLoggingCategory>
#endif
#include <QSettings>
#include <QSysInfo>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerCLIExecutableModuleFactory.h"
#include "qSlicerCLILoadableModuleFactory.h"
#include "qSlicerCommandOptions.h"
#include "qSlicerCoreModuleFactory.h"
#include "qSlicerLoadableModuleFactory.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"
#include "vtkSlicerConfigure.h"        // For Slicer_QtPlugins_DIR
#include "vtkSlicerVersionConfigure.h" // For Slicer_MAIN_PROJECT_VERSION_FULL

#ifdef Slicer_USE_PYTHONQT
# include "qSlicerScriptedLoadableModuleFactory.h"
#endif

#include <vtkSystemInformation.h>

// CTK includes
#include <ctkAppLauncherEnvironment.h>
#include <ctkMessageBox.h>
#include <ctkProxyStyle.h>
#ifdef Slicer_USE_PYTHONQT
# include <ctkPythonConsole.h>
#endif

// MRMLWidgets includes
#include <qMRMLEventLoggerWidget.h>
#include <qMRMLWidget.h>

// ITK includes
#include <itkFactoryRegistration.h>

// VTK includes
#include <vtkNew.h>
#include <vtkLogger.h>

// PythonQt includes
#ifdef Slicer_USE_PYTHONQT
# include <PythonQtObjectPtr.h>
# include <PythonQtPythonInclude.h>
#endif

#ifdef _WIN32
# include <Windows.h> //for SHELLEXECUTEINFO
#endif

#ifdef __APPLE__
# include <sys/sysctl.h> // for sysctl, which reports the creation time of the process
# include <sys/time.h>   // for gettimeofday
# include <unistd.h>     // for getpid
#endif

#ifdef __linux__
# include <cstdio>   // for fopen, to read /proc/self/stat
# include <cstring>  // for strrchr
# include <ctime>    // for clock_gettime
# include <unistd.h> // for sysconf
#endif

//----------------------------------------------------------------------------
double qSlicerApplicationHelper::TimeElapsedBeforeProcessEntryPointMs = 0.0;

//----------------------------------------------------------------------------
QElapsedTimer qSlicerApplicationHelper::TimeElapsedAfterProcessEntryPointTimer;

//----------------------------------------------------------------------------
qSlicerApplicationHelper::qSlicerApplicationHelper(QObject* parent)
  : Superclass(parent)
{
}

//----------------------------------------------------------------------------
qSlicerApplicationHelper::~qSlicerApplicationHelper() = default;

//----------------------------------------------------------------------------
void qSlicerApplicationHelper::recordProcessEntryPointTime()
{
  // How much the loader spent before this point can only be had from the operating
  // system, because no clock of ours was running yet. Where it will not say,
  // TimeElapsedBeforeProcessEntryPointMs is left at 0 and the startup is reported as if
  // it began at the entry point.
#if defined(_WIN32)
  FILETIME creationTime, exitTime, kernelTime, userTime;
  if (GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime))
  {
    FILETIME nowTime;
    GetSystemTimeAsFileTime(&nowTime);
    ULARGE_INTEGER creation, now;
    creation.LowPart = creationTime.dwLowDateTime;
    creation.HighPart = creationTime.dwHighDateTime;
    now.LowPart = nowTime.dwLowDateTime;
    now.HighPart = nowTime.dwHighDateTime;
    // FILETIME is expressed in 100-nanosecond units.
    Self::TimeElapsedBeforeProcessEntryPointMs = static_cast<double>(now.QuadPart - creation.QuadPart) / 10000.0;
  }
#elif defined(__APPLE__)
  // The kernel reports the creation time of the process as a wall-clock timestamp in
  // kinfo_proc.
  struct kinfo_proc processInfo;
  size_t processInfoSize = sizeof(processInfo);
  int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid() };
  timeval nowTime;
  if (sysctl(mib, 4, &processInfo, &processInfoSize, nullptr, 0) == 0 //
      && gettimeofday(&nowTime, nullptr) == 0)
  {
    const timeval& creationTime = processInfo.kp_proc.p_starttime;
    Self::TimeElapsedBeforeProcessEntryPointMs = (nowTime.tv_sec - creationTime.tv_sec) * 1000.0 //
                                                 + (nowTime.tv_usec - creationTime.tv_usec) / 1000.0;
  }
#elif defined(__linux__)
  // The kernel reports the creation time of the process in field 22 of /proc/self/stat,
  // in clock ticks since boot; CLOCK_BOOTTIME is the matching clock for "now". Both ends
  // being boot-relative, this is immune to wall clock adjustments.
  if (FILE* statFile = fopen("/proc/self/stat", "r"))
  {
    char stat[2048] = { 0 };
    size_t statLength = fread(stat, 1, sizeof(stat) - 1, statFile);
    fclose(statFile);
    // Parsing starts after the last ')', which closes field 2, the executable name: that
    // name may itself contain spaces and parentheses, but the fields after it cannot.
    // Field 22 is then the 20th field, and %*s skips a field whatever its format.
    const char* afterExecutableName = statLength > 0 ? strrchr(stat, ')') : nullptr;
    long long creationTimeTicks = 0;
    long ticksPerSecond = sysconf(_SC_CLK_TCK);
    struct timespec nowTime;
    if (afterExecutableName //
        && sscanf(afterExecutableName + 1,
                  " %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %lld",
                  &creationTimeTicks)
             == 1
        && creationTimeTicks > 0 && ticksPerSecond > 0 //
        && clock_gettime(CLOCK_BOOTTIME, &nowTime) == 0)
    {
      Self::TimeElapsedBeforeProcessEntryPointMs = (nowTime.tv_sec * 1000.0 + nowTime.tv_nsec / 1e6) //
                                                   - creationTimeTicks * 1000.0 / ticksPerSecond;
    }
  }
#endif

  Self::TimeElapsedAfterProcessEntryPointTimer.start();
}

//----------------------------------------------------------------------------
void qSlicerApplicationHelper::preInitializeApplication(const char* argv0, ctkProxyStyle* style)
{
  // Fall back to starting the clock here if recordProcessEntryPointTime() was not called
  // (a custom application that provides its own main() instead of including
  // qSlicerApplicationMainWrapper.cxx), without restarting a clock that is already running.
  if (!Self::TimeElapsedAfterProcessEntryPointTimer.isValid())
  {
    Self::TimeElapsedAfterProcessEntryPointTimer.start();
  }

#if defined(Q_OS_MACOS) && (QT_VERSION < QT_VERSION_CHECK(5, 15, 10))
  // See https://github.com/Slicer/Slicer/issues/7261
  QLoggingCategory::setFilterRules("qt.qpa.fonts=false");
#endif

  vtkLogger::SetStderrVerbosity(vtkLogger::VERBOSITY_OFF);
  itk::itkFactoryRegistration();
  qMRMLWidget::preInitializeApplication();

#ifdef Q_OS_MACOS
  // An installed macOS application bundle has no separate launcher process to
  // set up the environment (unlike Linux and Windows, and unlike the build
  // tree where the CTK application launcher is used). The Qt platform plugin
  // ("cocoa") is loaded by the QApplication base class constructor, before
  // qSlicerCoreApplication is able to apply the launcher settings, so
  // QT_PLUGIN_PATH must already point at the Qt plugins bundled in the
  // application. Without it the application aborts at startup with
  // "Could not find the Qt platform plugin cocoa".
  // This is only needed when running without a launcher (currentLevel() == 0):
  // when a launcher is used, it has already set QT_PLUGIN_PATH.
  if (argv0 && ctkAppLauncherEnvironment::currentLevel() == 0 && !qEnvironmentVariableIsSet("QT_PLUGIN_PATH"))
  {
    std::string executableDir = vtksys::SystemTools::GetFilenamePath(vtksys::SystemTools::CollapseFullPath(argv0));
    // <bundle>/Contents/MacOS -> <bundle>/Contents/<Slicer_QtPlugins_DIR>
    std::string pluginPath = vtksys::SystemTools::CollapseFullPath(executableDir + "/../" + Slicer_QtPlugins_DIR);
    if (vtksys::SystemTools::FileIsDirectory(pluginPath))
    {
      qputenv("QT_PLUGIN_PATH", QByteArray::fromStdString(pluginPath));
    }
  }
#endif

  // Allow a custom application name so that the settings
  // can be distinct for differently named applications
  QString applicationName("Slicer");
  if (argv0)
  {
    applicationName = qSlicerCoreApplication::applicationNameFromExecutablePath(QString::fromLocal8Bit(argv0));
  }
  QCoreApplication::setApplicationName(applicationName);

  QCoreApplication::setApplicationVersion(Slicer_MAIN_PROJECT_VERSION_FULL);
  // vtkObject::SetGlobalWarningDisplay(false);
  QApplication::setDesktopSettingsAware(false);
  if (style)
  {
    QApplication::setStyle(style);
  }

  qMRMLWidget::postInitializeApplication();
}

//----------------------------------------------------------------------------
void qSlicerApplicationHelper::setupModuleFactoryManager(qSlicerModuleFactoryManager* moduleFactoryManager)
{
  qSlicerApplication* app = qSlicerApplication::application();
  // Register module factories
  moduleFactoryManager->registerFactory(new qSlicerCoreModuleFactory);

  qSlicerCommandOptions* options = qSlicerApplication::application()->commandOptions();

  if (options->disableModules())
  {
    return;
  }

  // Modules that only exist to be exercised by a test are of no use to an end user, and
  // several of them are scripted, so registering them means importing Python modules
  // that will never be used. Skip them unless developer mode is on, matching the
  // existing behavior of the module menu and the module list, which already hide them.
  //
  // Testing mode is excluded as well, because that is how the automated tests launch the
  // application and they do need their own modules.
  const bool developerModeEnabled = app->userSettings()->value("Developer/DeveloperMode", false).toBool();
  const bool testingEnabled = app->testAttribute(qSlicerCoreApplication::AA_EnableTesting);
  moduleFactoryManager->setIgnoreTestingModules(!developerModeEnabled && !testingEnabled);

  if (!options->disableLoadableModules())
  {
    moduleFactoryManager->registerFactory(new qSlicerLoadableModuleFactory);
    if (!options->disableBuiltInModules() &&         //
        !options->disableBuiltInLoadableModules() && //
        !options->runPythonAndExit())
    {
      QString loadablePath = app->slicerHome() + "/" + Slicer_QTLOADABLEMODULES_LIB_DIR + "/";
      moduleFactoryManager->addSearchPath(loadablePath);
      // On Win32, *both* paths have to be there, since scripts are installed
      // in the install location, and exec/libs are *automatically* installed
      // in intDir.
      moduleFactoryManager->addSearchPath(loadablePath + app->intDir());
    }
  }

#ifdef Slicer_USE_PYTHONQT
  if (!options->disableScriptedLoadableModules())
  {
    moduleFactoryManager->registerFactory(new qSlicerScriptedLoadableModuleFactory);
    if (!options->disableBuiltInModules() &&                                        //
        !options->disableBuiltInScriptedLoadableModules() &&                        //
        !qSlicerApplication::testAttribute(qSlicerApplication::AA_DisablePython) && //
        !options->runPythonAndExit())
    {
      QString scriptedPath = app->slicerHome() + "/" + Slicer_QTSCRIPTEDMODULES_LIB_DIR + "/";
      moduleFactoryManager->addSearchPath(scriptedPath);
      // On Win32, *both* paths have to be there, since scripts are installed
      // in the install location, and exec/libs are *automatically* installed
      // in intDir.
      moduleFactoryManager->addSearchPath(scriptedPath + app->intDir());
    }
  }
#endif

  if (!options->disableCLIModules())
  {
    QString tempDirectory = qSlicerCoreApplication::application()->temporaryPath();

    // Always prefer executable CLIs. While launching a new process and transfer data via files may take slightly
    // longer, the file transfer is more robust, the CLI module can be stopped at any time (while a thread may be
    // requested to stop, but there is no way to force it to stop cleanly), errors in the CLI module cannot crash
    // the application, and startup time and memory usage is reduced by avoiding loading all CLI modules into the
    // main process. See more information in https://github.com/Slicer/Slicer/issues/4893.
    const bool preferExecutableCLIs = true;

    qSlicerCLILoadableModuleFactory* cliLoadableFactory = new qSlicerCLILoadableModuleFactory();
    cliLoadableFactory->setTempDirectory(tempDirectory);
    moduleFactoryManager->registerFactory(cliLoadableFactory, preferExecutableCLIs ? 0 : 1);

    qSlicerCLIExecutableModuleFactory* cliExecutableFactory = new qSlicerCLIExecutableModuleFactory();
    cliExecutableFactory->setTempDirectory(tempDirectory);
    moduleFactoryManager->registerFactory(cliExecutableFactory, preferExecutableCLIs ? 1 : 0);

    if (!options->disableBuiltInModules() &&    //
        !options->disableBuiltInCLIModules() && //
        !options->runPythonAndExit())
    {
      QString cliPath = app->slicerHome() + "/" + Slicer_CLIMODULES_LIB_DIR + "/";
      moduleFactoryManager->addSearchPath(cliPath);
      // On Win32, *both* paths have to be there, since scripts are installed
      // in the install location, and exec/libs are *automatically* installed
      // in intDir.
      moduleFactoryManager->addSearchPath(cliPath + app->intDir());
#ifdef Q_OS_MAC
      moduleFactoryManager->addSearchPath(app->slicerHome() + "/" + Slicer_CLIMODULES_SUBDIR);
#endif
    }
  }

  moduleFactoryManager->addSearchPaths(app->toSlicerHomeAbsolutePaths(app->revisionUserSettings()->value("Modules/AdditionalPaths").toStringList()));

  QStringList modulesToAlwaysIgnore = app->revisionUserSettings()->value("Modules/IgnoreModules").toStringList();
  QStringList modulesToTemporarlyIgnore = options->modulesToIgnore();
  // Discard modules already listed in the settings
  for (const QString& moduleToAlwaysIgnore : modulesToAlwaysIgnore)
  {
    modulesToTemporarlyIgnore.removeAll(moduleToAlwaysIgnore);
  }
  QStringList modulesToIgnore = modulesToAlwaysIgnore << modulesToTemporarlyIgnore;
  moduleFactoryManager->setModulesToIgnore(modulesToIgnore);

  moduleFactoryManager->setVerboseModuleDiscovery(app->commandOptions()->verboseModuleDiscovery());
}

//----------------------------------------------------------------------------
void qSlicerApplicationHelper::showMRMLEventLoggerWidget()
{
  qMRMLEventLoggerWidget* logger = new qMRMLEventLoggerWidget(nullptr);
  logger->setAttribute(Qt::WA_DeleteOnClose);
  logger->setConsoleOutputEnabled(false);
  logger->setMRMLScene(qSlicerApplication::application()->mrmlScene());

  QObject::connect(qSlicerApplication::application(), SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), logger, SLOT(setMRMLScene(vtkMRMLScene*)));

  logger->show();
}

//----------------------------------------------------------------------------
bool qSlicerApplicationHelper::checkRenderingCapabilities()
{
  vtkNew<vtkSystemInformation> systemInfo;
  systemInfo->RunRenderingCheck();
  if (systemInfo->GetRenderingCapabilities() & vtkSystemInformation::OPENGL)
  {
    return true;
  }

  qWarning("Graphics capability of this computer is not sufficient to run this application");

  QString message = tr("Graphics capability of this computer is not sufficient to "
                       "run this application. The application most likely will not function properly.");

  QString details = tr("See more information and help at:\n%1/user_guide/get_help.html#slicer-application-does-not-start\n\n"
                       "Graphics capabilities of this computer:\n\n")
                      .arg(qSlicerApplication::application()->documentationBaseUrl());
  details += systemInfo->GetRenderingCapabilitiesDetails().c_str();

  ctkMessageBox* messageBox = new ctkMessageBox(nullptr);
  messageBox->setAttribute(Qt::WA_DeleteOnClose, true);
  messageBox->setIcon(QMessageBox::Warning);
  messageBox->setWindowTitle(tr("Insufficient graphics capability"));
  messageBox->setText(message);
  messageBox->setDetailedText(details);
#if defined(_WIN32)
  // Older versions of Windows Remote Desktop protocol (RDP) makes the system report lower
  // OpenGL capability than the actual capability is (when the system is used locally).
  // On these systems, Slicer cannot be started while an RDP connection is active,
  // but an already started Slicer can be operated without problems.
  // Retry option allows delayed restart of Slicer through remote connection.
  // There is no need to offer "retry" option on other operating systems.
  messageBox->setStandardButtons(QMessageBox::Close | QMessageBox::Ignore | QMessageBox::Retry);
#else
  messageBox->setStandardButtons(QMessageBox::Close | QMessageBox::Ignore);
#endif
  messageBox->setDefaultButton(QMessageBox::Close);
  int result = messageBox->exec();

#if defined(_WIN32)
  if (result == QMessageBox::Retry)
  {
    // This option is for restarting the application outside of a
    // remote desktop session (during remote desktop sessions, system
    // may report lower OpenGL capabilities).

    // Run tscon system tool to create a new session, which terminates
    // the existing session (closes remote desktop connection).
    qSlicerApplicationHelper::runAsAdmin("tscon.exe", "1 /dest:console");

    QApplication::processEvents();

    // By now the remote desktop session is terminated, we restart
    // the application in a normal local desktop session.
    qSlicerApplication::restart();
  }
#endif

  return (result == QMessageBox::Ignore);
}

//----------------------------------------------------------------------------
int qSlicerApplicationHelper::runAsAdmin(QString executable, QString parameters /*=QString()*/, QString workingDir /*=QString()*/)
{
#if defined(_WIN32)
  // Run tscon system tool to create a new session, which terminates
  // the existing session (closes remote desktop connection).
  SHELLEXECUTEINFO shExecInfo;
  ZeroMemory(&shExecInfo, sizeof(shExecInfo));
  shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
  shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
  shExecInfo.hwnd = nullptr;
  // tscon requires administrator access, therefore "runas" verb is needed.
  // UAC popup will be displayed.
  shExecInfo.lpVerb = TEXT("runas");

# ifdef UNICODE
  // ShellExecuteExW expects UTF-16
  // Keep all converted strings alive during the entire call.
  std::wstring exeW = executable.toStdWString();
  std::wstring paramsW = parameters.isEmpty() ? L"" : parameters.toStdWString();
  std::wstring workdirW = workingDir.isEmpty() ? L"" : workingDir.toStdWString();

  shExecInfo.lpFile = exeW.c_str();
  shExecInfo.lpParameters = paramsW.empty() ? nullptr : paramsW.c_str();
  shExecInfo.lpDirectory = workdirW.empty() ? nullptr : workdirW.c_str();
# else
  // ShellExecuteExA expects ANSI (current ACP). Use UTF-8 and let Windows convert.
  // Keep all converted strings alive during the entire call.
  QByteArray exeA = executable.toUtf8();
  QByteArray paramsA = parameters.toUtf8();
  QByteArray workdirA = workingDir.toUtf8();

  shExecInfo.lpFile = exeA.constData();
  shExecInfo.lpParameters = parameters.isEmpty() ? nullptr : paramsA.constData();
  shExecInfo.lpDirectory = workingDir.isEmpty() ? nullptr : workdirA.constData();
# endif

  shExecInfo.nShow = SW_MAXIMIZE;
  shExecInfo.hInstApp = nullptr;
  if (!ShellExecuteEx(&shExecInfo))
  {
    return -1;
  }

  WaitForSingleObject(shExecInfo.hProcess, INFINITE);
  DWORD exitCode = 0;
  GetExitCodeProcess(shExecInfo.hProcess, &exitCode);
  CloseHandle(shExecInfo.hProcess);
  return static_cast<int>(exitCode);

#else
  Q_UNUSED(executable);
  Q_UNUSED(parameters);
  Q_UNUSED(workingDir);
  qFatal("%s: not implemented for Linux and macOS.", Q_FUNC_INFO);
  return -1;
#endif
}

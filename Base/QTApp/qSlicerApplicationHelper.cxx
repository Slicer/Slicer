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
#include <QLabel>
#include <QSettings>
#include <QSysInfo>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>

// Slicer includes
#include "qSlicerApplication.h"
#ifdef Slicer_BUILD_CLI_SUPPORT
# include "qSlicerCLIExecutableModuleFactory.h"
# include "qSlicerCLILoadableModuleFactory.h"
#endif
#include "qSlicerCommandOptions.h"
#include "qSlicerCoreModuleFactory.h"
#include "qSlicerLoadableModuleFactory.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"

#ifdef Slicer_USE_PYTHONQT
# include "qSlicerScriptedLoadableModuleFactory.h"
#endif

#include <vtkSystemInformation.h>

// CTK includes
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
#include <vtksys/SystemTools.hxx>
#include <vtkNew.h>

// PythonQt includes
#ifdef Slicer_USE_PYTHONQT
# include <PythonQtObjectPtr.h>
# include <PythonQtPythonInclude.h>
#endif

#ifdef _WIN32
#include <Windows.h> //for SHELLEXECUTEINFO
#endif

//----------------------------------------------------------------------------
qSlicerApplicationHelper::qSlicerApplicationHelper(QObject * parent) : Superclass(parent)
{
}

//----------------------------------------------------------------------------
qSlicerApplicationHelper::~qSlicerApplicationHelper() = default;

//----------------------------------------------------------------------------
void qSlicerApplicationHelper::preInitializeApplication(
    const char* argv0, ctkProxyStyle* style)
{
  itk::itkFactoryRegistration();
  qMRMLWidget::preInitializeApplication();

  // Allow a custom application name so that the settings
  // can be distinct for differently named applications
  QString applicationName("Slicer");
  if (argv0)
    {
    std::string name = vtksys::SystemTools::GetFilenameWithoutExtension(argv0);
    applicationName = QString::fromLocal8Bit(name.c_str());
    applicationName.remove(QString("App-real"));
    }
  QCoreApplication::setApplicationName(applicationName);

  QCoreApplication::setApplicationVersion(Slicer_MAIN_PROJECT_VERSION_FULL);
  //vtkObject::SetGlobalWarningDisplay(false);
  QApplication::setDesktopSettingsAware(false);
  if (style)
    {
    QApplication::setStyle(style);
    }

  qMRMLWidget::postInitializeApplication();
}

//----------------------------------------------------------------------------
void qSlicerApplicationHelper::setupModuleFactoryManager(qSlicerModuleFactoryManager * moduleFactoryManager)
{
  qSlicerApplication* app = qSlicerApplication::application();
  // Register module factories
  moduleFactoryManager->registerFactory(new qSlicerCoreModuleFactory);

  qSlicerCommandOptions* options = qSlicerApplication::application()->commandOptions();

  if(options->disableModules())
    {
    return;
    }

  if (!options->disableLoadableModules())
    {
    moduleFactoryManager->registerFactory(new qSlicerLoadableModuleFactory);
    if (!options->disableBuiltInModules() &&
        !options->disableBuiltInLoadableModules() &&
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
    moduleFactoryManager->registerFactory(
      new qSlicerScriptedLoadableModuleFactory);
    if (!options->disableBuiltInModules() &&
        !options->disableBuiltInScriptedLoadableModules() &&
        !qSlicerApplication::testAttribute(qSlicerApplication::AA_DisablePython) &&
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

#ifdef Slicer_BUILD_CLI_SUPPORT
  if (!options->disableCLIModules())
    {
    QString tempDirectory =
      qSlicerCoreApplication::application()->temporaryPath();

    // Option to prefer executable CLIs to limit memory consumption.
    bool preferExecutableCLIs =
      app->userSettings()->value("Modules/PreferExecutableCLI", Slicer_CLI_PREFER_EXECUTABLE_DEFAULT).toBool();

    qSlicerCLILoadableModuleFactory* cliLoadableFactory = new qSlicerCLILoadableModuleFactory();
    cliLoadableFactory->setTempDirectory(tempDirectory);
    moduleFactoryManager->registerFactory(cliLoadableFactory, preferExecutableCLIs ? 0 : 1);

    qSlicerCLIExecutableModuleFactory* cliExecutableFactory = new qSlicerCLIExecutableModuleFactory();
    cliExecutableFactory->setTempDirectory(tempDirectory);
    moduleFactoryManager->registerFactory(cliExecutableFactory, preferExecutableCLIs ? 1 : 0);

    if (!options->disableBuiltInModules() &&
        !options->disableBuiltInCLIModules() &&
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
#endif
  moduleFactoryManager->addSearchPaths(
    app->revisionUserSettings()->value("Modules/AdditionalPaths").toStringList());

  QStringList modulesToAlwaysIgnore =
    app->revisionUserSettings()->value("Modules/IgnoreModules").toStringList();
  QStringList modulesToTemporarlyIgnore = options->modulesToIgnore();
  // Discard modules already listed in the settings
  foreach(const QString& moduleToAlwaysIgnore, modulesToAlwaysIgnore)
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

  QObject::connect(qSlicerApplication::application(),
                   SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   logger,
                   SLOT(setMRMLScene(vtkMRMLScene*)));

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

  QString details = tr(
    "See more information and help at:\nhttps://www.slicer.org/wiki/Documentation/Nightly/FAQ/General#Slicer_does_not_start \n\n"
    "Graphics capabilities of this computer:\n\n");
  details += systemInfo->GetRenderingCapabilitiesDetails().c_str();

  ctkMessageBox *messageBox = new ctkMessageBox(nullptr);
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
int qSlicerApplicationHelper::runAsAdmin(QString executable, QString parameters/*=QString()*/, QString workingDir/*=QString()*/)
{
#if defined(_WIN32)
  // Run tscon system tool to create a new session, which terminates
  // the existing session (closes remote desktop connection).
  SHELLEXECUTEINFO shExecInfo;
  shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
  shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
  shExecInfo.hwnd = nullptr;
  // tscon requires administrator access, therefore "runas" verb is needed.
  // UAC popup will be displayed.
  shExecInfo.lpVerb = "runas";
  shExecInfo.lpFile = executable.toUtf8().constData();
  shExecInfo.lpParameters = nullptr;
  if (!parameters.isEmpty())
    {
    shExecInfo.lpParameters = parameters.toUtf8().constData();
    }
  shExecInfo.lpDirectory = nullptr;
  if (!workingDir.isEmpty())
    {
    shExecInfo.lpDirectory = workingDir.toUtf8().constData();
    }
  shExecInfo.nShow = SW_MAXIMIZE;
  shExecInfo.hInstApp = nullptr;
  ShellExecuteEx(&shExecInfo);
  WaitForSingleObject(shExecInfo.hProcess, INFINITE);
  DWORD exitCode = 0;
  GetExitCodeProcess(shExecInfo.hProcess, &exitCode);
  CloseHandle(shExecInfo.hProcess);
  return exitCode;
#else
  Q_UNUSED(executable);
  Q_UNUSED(parameters);
  Q_UNUSED(workingDir);
  qFatal("%s: not implemented for Linux and macOS.", Q_FUNC_INFO);
  return -1;
#endif
}

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
#include <QSettings>
#include <QSysInfo>

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

// CTK includes
#include <ctkProxyStyle.h>
#ifdef Slicer_USE_PYTHONQT
# include <ctkPythonConsole.h>
#endif

// MRMLWidgets includes
#include <qMRMLEventLoggerWidget.h>

// ITK includes
#include <itkFactoryRegistration.h>

// VTK includes
#include <vtksys/SystemTools.hxx>
#ifdef Slicer_VTK_USE_QVTKOPENGLWIDGET
#include <QSurfaceFormat>
#include <QVTKOpenGLWidget.h>
#endif

// PythonQt includes
#ifdef Slicer_USE_PYTHONQT
# include <PythonQtObjectPtr.h>
# include <PythonQtPythonInclude.h>
#endif

#ifdef _WIN32
#include <Windows.h> //for SetProcessDPIAware
#endif

//----------------------------------------------------------------------------
qSlicerApplicationHelper::qSlicerApplicationHelper(QObject * parent) : Superclass(parent)
{
}

//----------------------------------------------------------------------------
qSlicerApplicationHelper::~qSlicerApplicationHelper()
{
}

//----------------------------------------------------------------------------
void qSlicerApplicationHelper::preInitializeApplication(
    const char* argv0, ctkProxyStyle* style)
{
  itk::itkFactoryRegistration();
#if QT_VERSION >= 0x040803
#ifdef Q_OS_MACX
  if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_8)
    {
    // Fix Mac OS X 10.9 (mavericks) font issue
    // https://bugreports.qt-project.org/browse/QTBUG-32789
    QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    }
#endif
#endif

#ifdef Slicer_VTK_USE_QVTKOPENGLWIDGET
  // Set default surface format for QVTKOpenGLWidget. Disable multisampling to
  // support volume rendering and other VTK functionality that reads from the
  // framebuffer; see https://gitlab.kitware.com/vtk/vtk/issues/17095.
  QSurfaceFormat format = QVTKOpenGLWidget::defaultFormat();
  format.setSamples(0);
  QSurfaceFormat::setDefaultFormat(format);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
#ifdef _WIN32
  // Qt windows defaults to the PROCESS_PER_MONITOR_DPI_AWARE for DPI display
  // on windows. Unfortunately, this doesn't work well on multi-screens setups.
  // By calling SetProcessDPIAware(), we force the value to
  // PROCESS_SYSTEM_DPI_AWARE instead which fixes those issues.
  SetProcessDPIAware();
#endif

  // Enable automatic scaling based on the pixel density of the monitor
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

  // Allow a custom appliction name so that the settings
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
  qMRMLEventLoggerWidget* logger = new qMRMLEventLoggerWidget(0);
  logger->setAttribute(Qt::WA_DeleteOnClose);
  logger->setConsoleOutputEnabled(false);
  logger->setMRMLScene(qSlicerApplication::application()->mrmlScene());

  QObject::connect(qSlicerApplication::application(),
                   SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   logger,
                   SLOT(setMRMLScene(vtkMRMLScene*)));

  logger->show();
}

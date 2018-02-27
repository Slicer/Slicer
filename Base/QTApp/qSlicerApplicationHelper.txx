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

// Slicer includes
#include "vtkSlicerConfigure.h" // For Slicer_* macros
#include "vtkSlicerVersionConfigure.h" // For Slicer_* version macros

// Slicer includes
#include <qSlicerApplication.h>

// Qt includes
#include <QApplication>
#include <QDesktopWidget>
#include <QFont>
#include <QList>
#include <QSettings>
#include <QSplashScreen>
#include <QSurfaceFormat>
#include <QString>
#include <QStyle>
#include <QSysInfo>
#include <QTimer>

// CTK includes
#include <ctkAbstractLibraryFactory.h>
#ifdef Slicer_USE_PYTHONQT
# include <ctkPythonConsole.h>
#endif

// ITK includes
#include <itkFactoryRegistration.h>

// VTK includes
#include <vtksys/SystemTools.hxx>
#ifdef Slicer_VTK_USE_QVTKOPENGLWIDGET
#include <QVTKOpenGLWidget.h>
#endif

#ifdef Slicer_BUILD_CLI_SUPPORT
# include "qSlicerCLIExecutableModuleFactory.h"
# include "qSlicerCLILoadableModuleFactory.h"
#endif
#include "qSlicerCommandOptions.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"

//----------------------------------------------------------------------------
template<typename SlicerStyleType>
void qSlicerApplicationHelper::preInitializeApplication(const char* argv0)
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

  QCoreApplication::setApplicationVersion(Slicer_VERSION_FULL);
  //vtkObject::SetGlobalWarningDisplay(false);
  QApplication::setDesktopSettingsAware(false);
  QApplication::setStyle(new SlicerStyleType());
}

namespace
{

#ifdef Slicer_USE_QtTesting
//-----------------------------------------------------------------------------
void setEnableQtTesting()
{
  if (qSlicerApplication::application()->commandOptions()->enableQtTesting() ||
      qSlicerApplication::application()->userSettings()->value("QtTesting/Enabled").toBool())
    {
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
    }
}
#endif

//----------------------------------------------------------------------------
void splashMessage(QScopedPointer<QSplashScreen>& splashScreen, const QString& message)
{
  if (splashScreen.isNull())
    {
    return;
    }
  splashScreen->showMessage(message, Qt::AlignBottom | Qt::AlignHCenter);
}

} // end of anonymous namespace

//----------------------------------------------------------------------------
template<typename SlicerMainWindowType>
void qSlicerApplicationHelper::postInitializeApplication(
    qSlicerApplication& app,
    QScopedPointer<QSplashScreen>& splashScreen,
    QScopedPointer<SlicerMainWindowType>& window
    )
{
  app.installEventFilter(app.style());

#ifdef Slicer_USE_QtTesting
  setEnableQtTesting(); // disabled the native menu bar.
#endif

  bool enableMainWindow = !app.commandOptions()->noMainWindow();
  enableMainWindow = enableMainWindow && !app.commandOptions()->runPythonAndExit();
  bool showSplashScreen = !app.commandOptions()->noSplash() && enableMainWindow;

  if (showSplashScreen)
    {
    QPixmap pixmap(":/SplashScreen.png");
    splashScreen.reset(new QSplashScreen(pixmap));
    splashMessage(splashScreen, "Initializing...");
    splashScreen->show();
    }

  qSlicerModuleManager * moduleManager = app.moduleManager();
  qSlicerModuleFactoryManager * moduleFactoryManager = moduleManager->factoryManager();
  QStringList additionalModulePaths;
  foreach(const QString& extensionOrModulePath, app.commandOptions()->additionalModulePaths())
    {
    QStringList modulePaths = moduleFactoryManager->modulePaths(extensionOrModulePath);
    if (!modulePaths.empty())
      {
      additionalModulePaths << modulePaths;
      }
    else
      {
      additionalModulePaths << extensionOrModulePath;
      }
    }
  moduleFactoryManager->addSearchPaths(additionalModulePaths);
  qSlicerApplicationHelper::setupModuleFactoryManager(moduleFactoryManager);

  // Set list of modules to ignore
  foreach(const QString& moduleToIgnore, app.commandOptions()->modulesToIgnore())
    {
    moduleFactoryManager->addModuleToIgnore(moduleToIgnore);
    }

  // Register and instantiate modules
  splashMessage(splashScreen, "Registering modules...");
  moduleFactoryManager->registerModules();
  if (app.commandOptions()->verboseModuleDiscovery())
    {
    qDebug() << "Number of registered modules:"
             << moduleFactoryManager->registeredModuleNames().count();
    }
  splashMessage(splashScreen, "Instantiating modules...");
  moduleFactoryManager->instantiateModules();
  if (app.commandOptions()->verboseModuleDiscovery())
    {
    qDebug() << "Number of instantiated modules:"
             << moduleFactoryManager->instantiatedModuleNames().count();
    }
  // Create main window
  splashMessage(splashScreen, "Initializing user interface...");
  if (enableMainWindow)
    {
    window.reset(new SlicerMainWindowType);
    }
  else if (app.commandOptions()->showPythonInteractor()
    && !app.commandOptions()->runPythonAndExit())
    {
    // there is no main window but we need to show Python interactor
#ifdef Slicer_USE_PYTHONQT
    ctkPythonConsole* pythonConsole = app.pythonConsole();
    pythonConsole->setWindowTitle("Slicer Python Interactor");
    pythonConsole->resize(600, 280);
    pythonConsole->show();
    pythonConsole->activateWindow();
    pythonConsole->raise();
#endif
    }

  // Load all available modules
  foreach(const QString& name, moduleFactoryManager->instantiatedModuleNames())
    {
    Q_ASSERT(!name.isNull());
    splashMessage(splashScreen, "Loading module \"" + name + "\"...");
    moduleFactoryManager->loadModule(name);
    }
  if (app.commandOptions()->verboseModuleDiscovery())
    {
    qDebug() << "Number of loaded modules:" << moduleManager->modulesNames().count();
    }

  splashMessage(splashScreen, QString());

  if (window)
    {
    QObject::connect(window.data(), SIGNAL(initialWindowShown()), &app, SIGNAL(startupCompleted()));
    }
  else
    {
    QTimer::singleShot(0, &app, SIGNAL(startupCompleted()));
    }

  if (window)
    {
    window->setHomeModuleCurrent();
    window->show();
    }

  if (splashScreen && window)
    {
    splashScreen->finish(window.data());
    }

  // Process command line argument after the event loop is started
  QTimer::singleShot(0, &app, SLOT(handleCommandLineArguments()));

  // qSlicerApplicationHelper::showMRMLEventLoggerWidget();
}

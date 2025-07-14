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

// Slicer includes
#include <qSlicerApplication.h>

// Qt includes
#include <QMouseEvent>
#include <QSettings>
#include <QSplashScreen>
#include <QTimer>

// CTK includes
#include <ctkAbstractLibraryFactory.h>
#include <ctkProxyStyle.h>
#ifdef Slicer_USE_PYTHONQT
# include <ctkPythonConsole.h>
#endif
#include <ctkUtils.h>

#ifdef Slicer_BUILD_CLI_SUPPORT
# include "qSlicerCLIExecutableModuleFactory.h"
# include "qSlicerCLILoadableModuleFactory.h"
#endif
#include "qSlicerCommandOptions.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"

namespace
{

#ifdef Slicer_USE_QtTesting
//-----------------------------------------------------------------------------
void setEnableQtTesting()
{
  if (qSlicerApplication::application()->commandOptions()->enableQtTesting() || //
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
template <typename SlicerMainWindowType>
int qSlicerApplicationHelper::postInitializeApplication(qSlicerApplication& app, QScopedPointer<QSplashScreen>& splashScreen, QScopedPointer<SlicerMainWindowType>& window)
{

#if defined(Q_CC_GNU) && Q_CC_GNU <= 703
  // The lines below are needed for suppressing "-Wunused-function" warnings
  // reported when using GCC<=7.3 to build translation units not making use
  // of the "qSlicerApplicationHelper::postInitializeApplication" function.
# ifdef Slicer_USE_QtTesting
  (void)setEnableQtTesting; // Fix -Wunused-function warning
# endif
  (void)splashMessage; // Fix -Wunused-function warning
#endif

  if (app.style())
  {
    app.installEventFilter(app.style());
  }

#ifdef Slicer_USE_QtTesting
  setEnableQtTesting(); // disabled the native menu bar.
#endif

  bool enableMainWindow = !app.commandOptions()->noMainWindow();
  enableMainWindow = enableMainWindow && !app.commandOptions()->runPythonAndExit();
  bool showSplashScreen = !app.commandOptions()->noSplash() && enableMainWindow;

// qSlicerApplicationHelper::checkRenderingCapabilities() seems only work reliably
// on Windows, therefore we skip it on other platforms.
// See details at https://issues.slicer.org/view.php?id=4252
#if defined(_WIN32)
  if (enableMainWindow && !app.testAttribute(qSlicerCoreApplication::AA_EnableTesting))
  {
    // Warn the user if rendering requirements are not met and offer
    // exiting from the application.
    if (!qSlicerApplicationHelper::checkRenderingCapabilities())
    {
      return 1;
    }
  }
#endif

  if (showSplashScreen)
  {
    QPixmap pixmap(":/SplashScreen.png");

    // The application launcher shows the splash screen without DPI scaling (if the screen resolution is higher
    // then the splashscreen icon appears smaller).
    // To match this behavior, we set the same device pixel ratio in the pixmap as the window's device pixel ratio.
    QGuiApplication* guiApp = qobject_cast<QGuiApplication*>(&app);
    if (guiApp)
    {
      pixmap.setDevicePixelRatio(guiApp->devicePixelRatio());
    }

    splashScreen.reset(new QSplashScreen(pixmap, Qt::WindowStaysOnTopHint));
    splashMessage(splashScreen, qSlicerApplication::tr("Initializing..."));
    splashScreen->show();
  }

  DraggableWidgetEventFilter draggable;
  if (splashScreen)
  {
    draggable.setWidget(splashScreen.get());
    splashScreen->installEventFilter(&draggable);
  }

  qSlicerModuleManager* moduleManager = app.moduleManager();
  qSlicerModuleFactoryManager* moduleFactoryManager = moduleManager->factoryManager();
  QStringList additionalModulePaths;
  foreach (const QString& extensionOrModulePath, app.commandOptions()->additionalModulePaths())
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
  foreach (const QString& moduleToIgnore, app.commandOptions()->modulesToIgnore())
  {
    moduleFactoryManager->addModuleToIgnore(moduleToIgnore);
  }

  // Register and instantiate modules
  splashMessage(splashScreen, qSlicerApplication::tr("Registering modules..."));
  moduleFactoryManager->registerModules();
  if (app.commandOptions()->verboseModuleDiscovery())
  {
    qDebug() << "Number of registered modules:" << moduleFactoryManager->registeredModuleNames().count();
  }

  splashMessage(splashScreen, qSlicerApplication::tr("Instantiating modules..."));
  // Show the name of each module that is being instantiated to make it easier to see if a module
  // inappropriately performs some lengthy operations during instantiation.
  QMetaObject::Connection moduleAboutToBeInstantiatedConnection =
    QObject::connect(moduleFactoryManager,
                     &qSlicerAbstractModuleFactoryManager::moduleAboutToBeInstantiated,
                     [&splashScreen](QString moduleName) { splashMessage(splashScreen, qSlicerApplication::tr("Instantiating module \"%1\"...").arg(moduleName)); });
  moduleFactoryManager->instantiateModules();
  QObject::disconnect(moduleAboutToBeInstantiatedConnection);

  if (splashScreen)
  {
    splashScreen->removeEventFilter(&draggable);
  }

  if (app.commandOptions()->verboseModuleDiscovery())
  {
    qDebug() << "Number of instantiated modules:" << moduleFactoryManager->instantiatedModuleNames().count();
  }

  QStringList failedToBeInstantiatedModuleNames =
    ctk::qSetToQStringList(ctk::qStringListToQSet(moduleFactoryManager->registeredModuleNames()) - ctk::qStringListToQSet(moduleFactoryManager->instantiatedModuleNames()));
  if (!failedToBeInstantiatedModuleNames.isEmpty())
  {
    qCritical() << "The following modules failed to be instantiated:";
    foreach (const QString& moduleName, failedToBeInstantiatedModuleNames)
    {
      qCritical().noquote() << "  " << moduleName;
    }
  }

  // Exit if testing module is enabled and not all modules are instantiated
  if (!failedToBeInstantiatedModuleNames.isEmpty() && app.testAttribute(qSlicerCoreApplication::AA_EnableTesting))
  {
    return EXIT_FAILURE;
  }

  // Create main window
  splashMessage(splashScreen, qSlicerApplication::tr("Initializing user interface..."));
  if (enableMainWindow)
  {
    window.reset(new SlicerMainWindowType);
  }
  else if (app.commandOptions()->showPythonConsole() //
           && !app.commandOptions()->runPythonAndExit())
  {
    // there is no main window but we need to show Python console
#ifdef Slicer_USE_PYTHONQT
    ctkPythonConsole* pythonConsole = app.pythonConsole();
    pythonConsole->setWindowTitle("Slicer Python Console");
    pythonConsole->resize(600, 280);
    pythonConsole->show();
    pythonConsole->activateWindow();
    pythonConsole->raise();
#endif
  }

  // Load all available modules
  foreach (const QString& name, moduleFactoryManager->instantiatedModuleNames())
  {
    Q_ASSERT(!name.isNull());
    splashMessage(splashScreen, qSlicerApplication::tr("Loading module \"%1\"...").arg(name));
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
    if (splashScreen)
    {
      splashScreen->close();
    }
    window->setHomeModuleCurrent();
    window->show();
  }

  // Process command line argument after the event loop is started
  QTimer::singleShot(0, &app, SLOT(handleCommandLineArguments()));

  // qSlicerApplicationHelper::showMRMLEventLoggerWidget();
  return EXIT_SUCCESS;
}

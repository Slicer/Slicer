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
#include <QElapsedTimer>
#include <QMap>
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

#include "qSlicerCLIExecutableModuleFactory.h"
#include "qSlicerCLILoadableModuleFactory.h"
#include "qSlicerCommandOptions.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"
#ifdef Slicer_BUILD_STARTUP_FILE_PREFETCH
# include "qSlicerStartupFilePrefetcher.h"
#endif

// STD includes
#include <algorithm>
#include <iostream>

// Minimal class definition so that Qt lupdate can parse qSlicerApplicationHelper::
// method definitions when processing this .hxx file (forward declaration is not sufficient).
#ifndef __qSlicerApplicationHelper_h
class qSlicerApplicationHelper
{
};
#endif

namespace
{

/// \brief Event filter for enabling draggable behavior on a widget.
///
/// This event filter allows the user to move a widget by clicking anywhere on it.
/// It also removes the WindowStaysOnTopHint, preventing the window from staying
/// above all other applications. The filter should be installed on and uninstalled
/// from the application.
class DraggableWidgetEventFilter : public QObject
{
public:
  /// Set the widget that will become draggable.
  void setWidget(QWidget* w) { this->Widget = w; }

protected:
  bool eventFilter(QObject* obj, QEvent* event) override
  {
    if (event->type() == QEvent::MouseButtonPress && this->Widget)
    {
      // Record the mouse press position for later reference during dragging.
      QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
      this->PressPosition = mouseEvent->pos();
      this->Dragging = true;
      return true; // do not process the event further
    }
    else if (event->type() == QEvent::MouseMove && this->Dragging && this->Widget)
    {
      // Move the widget
      QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
      this->Widget->move(this->Widget->pos() + mouseEvent->pos() - this->PressPosition);
      return true; // do not process the event further
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
      // End the dragging process.
      this->Dragging = false;
      // Disable the WindowStaysOnTop hint to allow other windows to be shown above it.
      // Do it after the mouse button is released, because the widget may be reparented
      // as a result of changing the window hint, and during reparenting some events
      // might not arrive to the widget.
      if (this->DisableTopMost)
      {
        this->DisableTopMost = false;
        this->Widget->setWindowFlags(this->Widget->windowFlags() & ~Qt::WindowStaysOnTopHint);
        // After removing the WindowStaysOnTopHint hint, we need to show the window again
        this->Widget->show();
      }
      return true; // do not process the event further
    }
    // If the event is not one of the specified types, pass it to the base class.
    return QObject::eventFilter(obj, event);
  }

private:
  bool DisableTopMost{ true };
  QWidget* Widget{ nullptr };
  QPoint PressPosition;
  bool Dragging{ false };
};

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

//----------------------------------------------------------------------------
/// Structure to store startup timings of a module.
/// Bringing a module up takes two steps: instantiating it (i.e., constructing it)
/// and loading it (a module reads its settings, creates its logic, registers node types, etc.).
struct ModuleTiming
{
  QString Name;
  double InstantiatedMs = 0.0;
  double LoadedMs = 0.0;
  double totalMs() const { return this->InstantiatedMs + this->LoadedMs; }
};

//----------------------------------------------------------------------------
/// A phase of the startup that has just finished, named, with the time elapsed from the
/// creation of the process to the moment it finished.
///
/// Kept as a timestamp rather than as the duration of the phase, because consecutive
/// timestamps subtract to give the duration of the phase between them, and the last one
/// is the total startup time.
struct StartupPhase
{
  QString Name;
  double TimeElapsedSinceProcessCreationMs = 0.0;
};

//----------------------------------------------------------------------------
/// How long each phase of the startup took.
///
/// Measured whether or not anyone asked for the report, because collecting it costs
/// almost nothing, and a timing that is only collected when requested is one that cannot
/// be added to a log after the fact.
struct StartupPhaseTimings
{
  /// The phases of the startup, in the order they finished. One list rather than a field
  /// per phase, so that adding, removing or reordering a phase is a change at the place
  /// where the phase happens and nowhere else. The last entry is the end of the startup.
  QList<StartupPhase> Phases;

  /// The same time as the module instantiating and loading phases, broken down by module,
  /// so that a slow phase can be traced to whichever module made it slow. Registration is
  /// not among them: the factory reports that a module has been registered but not that
  /// it is about to be, so there is nothing to measure between.
  QList<ModuleTiming> ModuleTimings;
};

//----------------------------------------------------------------------------
/// Writes to the application log how the startup went, if --report-startup-timing
/// asked for it.
///
/// Called explicitly at the end of postInitializeApplication(), not from
/// qSlicerApplication::startupCompleted(), which any number of modules also connect to:
/// as one slot among many it would run wherever its connection happened to fall in the
/// order, and whatever the slots after it did would be missing from the report.
///
/// The report goes to the log rather than to the standard output because a windowed
/// build on Windows has no console to write to. It is written as a single entry so that
/// nothing logged from another thread can land in the middle of it.
void reportStartupTiming(const StartupPhaseTimings& timings)
{
  if (!qSlicerApplication::application()->commandOptions()->reportStartupTiming())
  {
    return;
  }

  QStringList lines;
  lines << "Startup timing report";
  // Each phase is stored as a timestamp, so a phase lasted whatever separates it from
  // the phase before it.
  double phaseStartMs = 0.0;
  for (const StartupPhase& phase : timings.Phases)
  {
    lines << QString("  %1: %2 ms") //
               .arg(phase.Name)
               .arg(phase.TimeElapsedSinceProcessCreationMs - phaseStartMs, 0, 'f', 0);
    phaseStartMs = phase.TimeElapsedSinceProcessCreationMs;
  }
  // Slowest first, because the question this answers is which module to look at, and only
  // as far as the slowest few: past those the list becomes every module in the build, in
  // an order that no longer says anything about where the startup went.
  QList<ModuleTiming> modules = timings.ModuleTimings;
  std::sort(modules.begin(), modules.end(), [](const ModuleTiming& a, const ModuleTiming& b) { return a.totalMs() > b.totalMs(); });
  const int reportedModuleCount = qMin(modules.count(), 15);
  lines << QString("  Modules, slowest first, as instantiate + load (%1 of %2):") //
             .arg(reportedModuleCount)
             .arg(modules.count());
  for (int moduleIndex = 0; moduleIndex < reportedModuleCount; ++moduleIndex)
  {
    const ModuleTiming& module = modules.at(moduleIndex);
    // One decimal, so that modules costing a fraction of a millisecond are not all
    // reported as zero.
    lines << QString("    %1: %2 ms (%3 ms + %4 ms)") //
               .arg(module.Name)
               .arg(module.totalMs(), 0, 'f', 1)
               .arg(module.InstantiatedMs, 0, 'f', 1)
               .arg(module.LoadedMs, 0, 'f', 1);
  }
  // The phases cover the whole startup, so the last one ended when the startup did.
  const double totalMs = timings.Phases.isEmpty() ? 0.0 : timings.Phases.constLast().TimeElapsedSinceProcessCreationMs;
  lines << QString("  Total, from the creation of the process: %1 ms").arg(totalMs, 0, 'f', 0);
#ifdef Slicer_BUILD_STARTUP_FILE_PREFETCH
  lines << qSlicerStartupFilePrefetcher::reportLines();
#endif
  qInfo().noquote() << lines.join("\n");
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
  (void)reportStartupTiming; // Fix -Wunused-function warning
  (void)splashMessage;       // Fix -Wunused-function warning
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
    // Write a new-line character on the process output to hide the application launcher's splashscreen
    // (SPLASHSCREEN_IGNORE_OUTPUT option is disabled by default).
    std::cout << std::endl;
    // Ensure that the output is not delayed by buffering.
    std::cout.flush();
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
  for (const QString& extensionOrModulePath : app.commandOptions()->additionalModulePaths())
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
  for (const QString& moduleToIgnore : app.commandOptions()->modulesToIgnore())
  {
    moduleFactoryManager->addModuleToIgnore(moduleToIgnore);
  }

  StartupPhaseTimings startupPhaseTimings;

  // Note that a phase of the startup has just finished. The entry point timer is already
  // running, so there is nothing to start or restart: recording a phase is one call at
  // the place where the phase ends, and the report works out the durations.
  auto recordStartupPhaseCompletedTime = [&startupPhaseTimings](const QString& name)
  { startupPhaseTimings.Phases.push_back({ name, Self::TimeElapsedBeforeProcessEntryPointMs + Self::TimeElapsedAfterProcessEntryPointTimer.elapsed() }); };

  // What the loader did before the entry point, recorded as the first phase so that the
  // phases account for the whole startup from the creation of the process.
  startupPhaseTimings.Phases.push_back({ "Before the process entry point, loading the application's own libraries", Self::TimeElapsedBeforeProcessEntryPointMs });

  // Everything up to here was the application getting itself ready to have modules:
  // building qSlicerApplication, reading the settings, starting Python.
  recordStartupPhaseCompletedTime("Initializing the application");

  // Register and instantiate modules
  splashMessage(splashScreen, qSlicerApplication::tr("Registering modules..."));
  moduleFactoryManager->registerModules();
  recordStartupPhaseCompletedTime("Registering modules");
  if (app.commandOptions()->verboseModuleDiscovery())
  {
    qDebug() << "Number of registered modules:" << moduleFactoryManager->registeredModuleNames().count();
  }

  splashMessage(splashScreen, qSlicerApplication::tr("Instantiating modules..."));

  // Time each module separately. instantiateModules() below brackets every construction
  // with these two signals (instantiateModule() on its own emits only the second, but
  // these connections do not outlive the call below). Showing the name of each module on
  // the splash screen also makes it easier to see if a module inappropriately performs
  // some lengthy operations during instantiation.
  QMap<QString, ModuleTiming> moduleTimings;
  QElapsedTimer moduleTimer;
  moduleTimer.start();
  QMetaObject::Connection moduleAboutToBeInstantiatedConnection = //
    QObject::connect(moduleFactoryManager,
                     &qSlicerAbstractModuleFactoryManager::moduleAboutToBeInstantiated,
                     [&splashScreen, &moduleTimer](QString moduleName)
                     {
                       splashMessage(splashScreen, qSlicerApplication::tr("Instantiating module \"%1\"...").arg(moduleName));
                       // Last, so that updating the splash screen is not charged to the module.
                       moduleTimer.restart();
                     });
  QMetaObject::Connection moduleInstantiatedConnection = //
    QObject::connect(moduleFactoryManager,
                     &qSlicerAbstractModuleFactoryManager::moduleInstantiated,
                     [&moduleTimer, &moduleTimings](QString moduleName)
                     {
                       ModuleTiming& timing = moduleTimings[moduleName];
                       timing.Name = moduleName;
                       timing.InstantiatedMs = moduleTimer.nsecsElapsed() / 1e6;
                     });

  moduleFactoryManager->instantiateModules();
  QObject::disconnect(moduleAboutToBeInstantiatedConnection);
  QObject::disconnect(moduleInstantiatedConnection);
  recordStartupPhaseCompletedTime("Instantiating modules");

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
    for (const QString& moduleName : failedToBeInstantiatedModuleNames)
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

  recordStartupPhaseCompletedTime("Initializing the user interface");

  // Load all available modules.
  //
  // Loading a module first loads its not-yet-loaded dependencies, inside the same call,
  // and a module that is already loaded is skipped. Bracketing loadModule() with a timer
  // would therefore charge a dependency's load to whichever module happened to pull it in
  // first, and report the dependency itself as instantaneous. Instead, listen to
  // moduleLoaded(), which the factory emits at the end of every load, nested ones
  // included: a dependency's own emission closes its interval before its dependee's, so
  // each module is charged only its own time.
  double moduleLoadStartedMs = 0.0;
  QMetaObject::Connection moduleLoadedConnection = //
    QObject::connect(moduleFactoryManager,
                     &qSlicerModuleFactoryManager::moduleLoaded,
                     [&moduleTimer, &moduleTimings, &moduleLoadStartedMs](QString moduleName)
                     {
                       const double nowMs = moduleTimer.nsecsElapsed() / 1e6;
                       moduleTimings[moduleName].LoadedMs = nowMs - moduleLoadStartedMs;
                       moduleLoadStartedMs = nowMs;
                     });
  for (const QString& name : moduleFactoryManager->instantiatedModuleNames())
  {
    Q_ASSERT(!name.isNull());
    splashMessage(splashScreen, qSlicerApplication::tr("Loading module \"%1\"...").arg(name));
    // Taken after the splash message, so that updating the splash screen is not charged
    // to the module.
    moduleLoadStartedMs = moduleTimer.nsecsElapsed() / 1e6;
    moduleFactoryManager->loadModule(name);
  }
  QObject::disconnect(moduleLoadedConnection);
  if (app.commandOptions()->verboseModuleDiscovery())
  {
    qDebug() << "Number of loaded modules:" << moduleManager->modulesNames().count();
  }

  recordStartupPhaseCompletedTime("Loading modules");
  startupPhaseTimings.ModuleTimings = moduleTimings.values();

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
  recordStartupPhaseCompletedTime("Showing the main window");

  // Process command line argument after the event loop is started
  QTimer::singleShot(0, &app, SLOT(handleCommandLineArguments()));

  // Startup is over: the window is up and the event loop is about to take over.
#ifdef Slicer_BUILD_STARTUP_FILE_PREFETCH
  // Records the libraries this startup needed and stops the clock on the prefetch, so it
  // has to run whether or not a report was asked for, and before the report is collected.
  qSlicerStartupFilePrefetcher::finish();
#endif
  reportStartupTiming(startupPhaseTimings);

  // qSlicerApplicationHelper::showMRMLEventLoggerWidget();
  return EXIT_SUCCESS;
}

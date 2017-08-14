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
#include <QList>
#include <QSettings>
#include <QSplashScreen>
#include <QString>
#include <QStyleFactory>
#include <QSysInfo>
#include <QTimer>

// Slicer includes
#include "vtkSlicerConfigure.h"

// CTK includes
#include <ctkAbstractLibraryFactory.h>
#ifdef Slicer_USE_PYTHONQT
# include <ctkPythonConsole.h>
#endif

// Slicer includes
#include "vtkSlicerVersionConfigure.h" // For Slicer_VERSION_FULL, Slicer_BUILD_CLI_SUPPORT

// SlicerApp includes
#include "qSlicerApplication.h"
#include "qSlicerApplicationHelper.h"
#ifdef Slicer_BUILD_CLI_SUPPORT
# include "qSlicerCLIExecutableModuleFactory.h"
# include "qSlicerCLILoadableModuleFactory.h"
#endif
#include "qSlicerAppMainWindow.h"
#include "qSlicerCommandOptions.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"
#include "qSlicerStyle.h"

// ITK includes
#include <itkFactoryRegistration.h>

// VTK includes
//#include <vtkObject.h>
#include <vtksys/SystemTools.hxx>

#if defined (_WIN32) && !defined (Slicer_BUILD_WIN32_CONSOLE)
# include <windows.h>
# include <vtksys/Encoding.hxx>
#endif

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

//----------------------------------------------------------------------------
int SlicerAppMain(int argc, char* argv[])
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


  // Allow a custom appliction name so that the settings
  // can be distinct for differently named applications
  QString applicationName("Slicer");
  if (argv[0])
    {
    std::string name = vtksys::SystemTools::GetFilenameWithoutExtension(argv[0]);
    applicationName = QString::fromLocal8Bit(name.c_str());
    applicationName.remove(QString("App-real"));
    }
  QCoreApplication::setApplicationName(applicationName);

  QCoreApplication::setApplicationVersion(Slicer_VERSION_FULL);
  //vtkObject::SetGlobalWarningDisplay(false);
  QApplication::setDesktopSettingsAware(false);
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
  QApplication::setStyle(new qSlicerStyle());
#endif

  qSlicerApplication app(argc, argv);
  if (app.returnCode() != -1)
    {
    return app.returnCode();
    }
  app.installEventFilter(app.style());


#ifdef Slicer_USE_QtTesting
  setEnableQtTesting(); // disabled the native menu bar.
#endif

  bool enableMainWindow = !app.commandOptions()->noMainWindow();
  enableMainWindow = enableMainWindow && !app.commandOptions()->runPythonAndExit();
  bool showSplashScreen = !app.commandOptions()->noSplash() && enableMainWindow;

  QScopedPointer<QSplashScreen> splashScreen;
  if (showSplashScreen)
    {
    QPixmap pixmap(":/SplashScreen.png");
    splashScreen.reset(new QSplashScreen(pixmap));
    splashMessage(splashScreen, "Initializing...");
    splashScreen->show();
    }

  qSlicerModuleManager * moduleManager = qSlicerApplication::application()->moduleManager();
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
  if (app.commandOptions()->verbose())
    {
    qDebug() << "Number of registered modules:"
             << moduleFactoryManager->registeredModuleNames().count();
    }
  splashMessage(splashScreen, "Instantiating modules...");
  moduleFactoryManager->instantiateModules();
  if (app.commandOptions()->verbose())
    {
    qDebug() << "Number of instantiated modules:"
             << moduleFactoryManager->instantiatedModuleNames().count();
    }
  // Create main window
  splashMessage(splashScreen, "Initializing user interface...");
  QScopedPointer<qSlicerAppMainWindow> window;
  if (enableMainWindow)
    {
    window.reset(new qSlicerAppMainWindow);
    window->setWindowTitle(window->windowTitle()+ " " + Slicer_VERSION_FULL);
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
  if (app.commandOptions()->verbose())
    {
    qDebug() << "Number of loaded modules:" << moduleManager->modulesNames().count();
    }

  splashMessage(splashScreen, QString());

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

  // Look at QApplication::exec() documentation, it is recommended to connect
  // clean up code to the aboutToQuit() signal
  return app.exec();
}

} // end of anonymous namespace

#if defined (_WIN32) && !defined (Slicer_BUILD_WIN32_CONSOLE)
int __stdcall WinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPSTR lpCmdLine, int nShowCmd)
{
  Q_UNUSED(hInstance);
  Q_UNUSED(hPrevInstance);
  Q_UNUSED(nShowCmd);

  // CommandLineToArgvW has no narrow-character version, so we get the arguments in wide strings
  // and then convert to regular string.
  int argc;
  LPWSTR* argvStringW = CommandLineToArgvW(GetCommandLineW(), &argc);

  std::vector< const char* > argv(argc); // usual const char** array used in main() functions
  std::vector< std::string > argvString(argc); // this stores the strings that the argv pointers point to
  for(int i=0; i<argc; i++)
    {
    argvString[i] = vtksys::Encoding::ToNarrow(argvStringW[i]);
    argv[i] = argvString[i].c_str();
    }

  LocalFree(argvStringW);

  return SlicerAppMain(argc, const_cast< char** >(&argv[0]));
}
#else
int main(int argc, char *argv[])
{
  return SlicerAppMain(argc, argv);
}
#endif

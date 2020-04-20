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

#ifndef __qSlicerApplication_h
#define __qSlicerApplication_h

// Qt includes
#include <QPalette>

// CTK includes
#include <ctkPimpl.h>
#include <ctkSettingsDialog.h>

// QTCORE includes
#include "qSlicerCoreApplication.h"

// QTGUI includes
#include "qSlicerBaseQTGUIExport.h"

class QMainWindow;
class qSlicerApplicationPrivate;
class qSlicerCommandOptions;
class qSlicerIOManager;
#ifdef Slicer_USE_PYTHONQT
class qSlicerPythonManager;
#endif
class qSlicerLayoutManager;
class qSlicerWidget;

class ctkErrorLogModel;
#ifdef Slicer_USE_QtTesting
class ctkQtTestingUtility;
#endif

#ifdef Slicer_BUILD_DICOM_SUPPORT
class ctkDICOMBrowser;
#endif

// MRML includes
class vtkMRMLNode;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerApplication : public qSlicerCoreApplication
{
  Q_OBJECT
public:

  typedef qSlicerCoreApplication Superclass;
  qSlicerApplication(int &argc, char **argv);
  ~qSlicerApplication() override;

  /// Return a reference to the application singleton
  static qSlicerApplication* application();

  /// Avoid some crashes due to exceptions thrown during inside event handlers
  /// (such as slots).  When exceptions are thrown from slots, Qt generates this message:
  ///
  ///   > Qt has caught an exception thrown from an event handler. Throwing
  ///   > exceptions from an event handler is not supported in Qt. You must
  ///   > reimplement QApplication::notify() and catch all exceptions there.
  ///
  /// so we follow the pattern suggested here:
  ///
  /// http://stackoverflow.com/questions/13878373/where-am-i-supposed-to-reimplement-qapplicationnotify-function
  ///
  bool notify(QObject * receiver, QEvent * event) override;

  /// Get commandOptions
  Q_INVOKABLE qSlicerCommandOptions* commandOptions()const;

  /// Get IO Manager
  Q_INVOKABLE qSlicerIOManager* ioManager();

#ifdef Slicer_USE_PYTHONQT
  /// Get Python Manager
  Q_INVOKABLE qSlicerPythonManager * pythonManager();
  Q_INVOKABLE ctkPythonConsole * pythonConsole();
#endif

  #ifdef Slicer_USE_QtTesting
  /// Get test utility
  Q_INVOKABLE ctkQtTestingUtility* testingUtility();
  #endif

  /// Set/Get layout manager
  Q_INVOKABLE qSlicerLayoutManager* layoutManager()const;
  Q_INVOKABLE void setLayoutManager(qSlicerLayoutManager* layoutManager);

  /// Return a pointer on the main window of the application if any.
  QMainWindow* mainWindow()const;

  /// TODO
  /// See http://doc.trolltech.com/4.6/qapplication.html#commitData
  /// and http://doc.trolltech.com/4.6/qsessionmanager.html#allowsInteraction
  //virtual void commitData(QSessionManager & manager);

  /// Enable/Disable tooltips
  void setToolTipsEnabled(bool enable);

  /// Return the module name that is most suitable for editing the specified node.
  QString nodeModule(vtkMRMLNode* node)const;

  Q_INVOKABLE ctkSettingsDialog* settingsDialog()const;

  /// Log application information.
  ///
  /// This function will log the following
  /// details:
  ///   - Session start time
  ///   - Slicer version
  ///   - Operating system
  ///   - Memory
  ///   - CPU
  ///   - Developer mode enabled
  ///   - Prefer executable CLI
  ///   - Additional module paths
  ///
  /// \note Starting the application with `--application-information` will
  /// also print the information to standard output.
  ///
  /// \sa qSlicerCoreCommandOptions::displayApplicationInformation()
  Q_INVOKABLE virtual void logApplicationInformation() const;

#ifdef Slicer_BUILD_DICOM_SUPPORT
  /// Create a DICOM browser that uses the application's main DICOM database.
  /// This method is added because PythonQt does not allow wrapping of QSharedPointer.
  Q_INVOKABLE ctkDICOMBrowser* createDICOMBrowserForMainDatabase();
#endif

#ifdef Q_OS_WIN32
  /// Get Windows operating system build number
  Q_INVOKABLE static unsigned long int windowsOSBuildNumber();

  /// Get process code page. On recent Windows versions it is expected to be UTF-8 (65001)
  Q_INVOKABLE static unsigned int windowsActiveCodePage();
#endif

  /// Return true if process code page is UTF-8.
  /// It is true for all MacOS and Linux versions and for Windows version 1903 (May 2019 Update) or later.
  Q_INVOKABLE static bool isCodePageUtf8();

public slots:

  /// Utility function that retrieve the best module for a node and trigger
  /// its associated QAction which eventually opens the module.
  /// \note qSlicerApplication is a temporary host for the function as it should be
  /// moved into a DataManager where module can register new node
  /// types/modules
  void openNodeModule(vtkMRMLNode* node);

  /// Popup a dialog asking the user if the application should be restarted.
  /// If no \a reason is given, the text will default to ""Are you sure you want to restart?"
  void confirmRestart(QString reason = QString());

  /// Launch Qt Designer application (bundled with the application).
  /// If uiFile argument is specified then designer loads that UI file on startup.
  bool launchDesigner(const QStringList& args = QStringList());

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  void openExtensionsManagerDialog();
#endif

  /// Number of recent log files to keep. Older log files are deleted automatically.
  int numberOfRecentLogFilesToKeep();

  /// Paths of recent log files
  QStringList recentLogFiles();

  /// Path of the current log file
  /// \sa recentLogFiles(), setupFileLogging()
  QString currentLogFile()const;

  /// When turning on OpenGL and using the full screen mode, menus and tooltips
  /// are no longer visible. By enabling hasBorderInFullScreen, a one-pixel border
  /// is added around the window, which fixes the problem.
  /// Border has to be enabled before going to full screen mode.
  /// This method has only effect when using Qt5 on Windows.
  /// See http://doc.qt.io/qt-5/windows-issues.html#fullscreen-opengl-based-windows
  void setHasBorderInFullScreen(bool);

  /// Calls setRenderPaused(pause) on the current layout manager.
  /// Emits pauseRenderRequested() if pause is true and resumeRenderRequested() if pause is false.
  /// The caller is responsible for making sure that each setRenderPaused(true) is paired with
  /// setRenderPaused(false).
  /// \sa qMRMLLayoutManager::setRenderPaused()
  void setRenderPaused(bool pause) override;

  /// Equivalent to setRenderPaused(true)
  /// \sa setRenderPaused
  void pauseRender() override;

  /// Equivalent to setRenderPaused(false)
  /// \sa setRenderPaused
  void resumeRender() override;

signals:

  /// Emitted when the startup phase has been completed.
  ///
  /// Startup is complete when all the modules have been
  /// initialized and the main window is shown to the user.
  ///
  /// \note If the application is started without the mainwindow,
  /// the signal is emitted after the modules are initialized.
  ///
  /// \sa qSlicerAppMainWindow::initialWindowShown()
  void startupCompleted();

  /// Emitted when setRenderPaused() is called.
  /// This can be used by modules that manage their own viewers
  /// to pause and resume rendering.
  /// \sa setRenderPaused
  void renderPaused(bool);

protected slots:

  /// Request editing of a MRML node
  void editNode(vtkObject*, void*, unsigned long) override;

protected:
  /// Reimplemented from qSlicerCoreApplication
  void handlePreApplicationCommandLineArguments() override;
  void handleCommandLineArguments() override;
  void onSlicerApplicationLogicModified() override;

  /// Set up file logging. Creates and sets new log file and deletes the oldest
  /// one from the stored queue
  void setupFileLogging();

private:
  Q_DECLARE_PRIVATE(qSlicerApplication);
  Q_DISABLE_COPY(qSlicerApplication);
};

/// Apply the Slicer palette to the \c palette
/// Note also that the palette parameter is passed by reference and will be
/// updated using the native paletter and applying Slicer specific properties.
void  Q_SLICER_BASE_QTGUI_EXPORT qSlicerApplyPalette(QPalette& palette);

#endif

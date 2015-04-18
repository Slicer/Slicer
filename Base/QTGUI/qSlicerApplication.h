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

// MRML includes
class vtkMRMLNode;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerApplication : public qSlicerCoreApplication
{
  Q_OBJECT
public:

  typedef qSlicerCoreApplication Superclass;
  qSlicerApplication(int &argc, char **argv);
  virtual ~qSlicerApplication();

  /// Return a reference to the application singleton
  static qSlicerApplication* application();

  /// Avoid some crashes due to execeptions thrown during inside event handlers
  /// (such as slots).  When exceptions are thown from slots, Qt generates this message:
  ///
  ///   > Qt has caught an exception thrown from an event handler. Throwing
  ///   > exceptions from an event handler is not supported in Qt. You must
  ///   > reimplement QApplication::notify() and catch all exceptions there.
  ///
  /// so we follow the pattern suggested here:
  ///
  /// http://stackoverflow.com/questions/13878373/where-am-i-supposed-to-reimplement-qapplicationnotify-function
  ///
  virtual bool notify(QObject * receiver, QEvent * event);

  /// Get errorLogModel
  Q_INVOKABLE ctkErrorLogModel* errorLogModel()const;

  /// Get commandOptions
  Q_INVOKABLE qSlicerCommandOptions* commandOptions();

  /// Get IO Manager
  Q_INVOKABLE qSlicerIOManager* ioManager();

  #ifdef Slicer_USE_PYTHONQT
  /// Get Python Manager
  Q_INVOKABLE qSlicerPythonManager * pythonManager();
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

  /// Return the best module name for a given node.
  /// \note qSlicerApplication is a temporary host for the function as it should be
  /// moved into a DataManager where module can register new node
  /// types/modules
  QString nodeModule(vtkMRMLNode* node)const;

  Q_INVOKABLE ctkSettingsDialog* settingsDialog()const;

  /// Display application informations.
  /// This function will print to standard output the following
  /// details:
  ///   - Session start time
  ///   - Slicer version
  ///   - Operating system
  ///   - Memory
  ///   - CPU
  ///   - Developer mode enabled
  ///   - Prefer executable CLI
  ///   - Additional module paths
  Q_INVOKABLE virtual void displayApplicationInformations() const;

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

protected:
  /// Reimplemented from qSlicerCoreApplication
  virtual void handlePreApplicationCommandLineArguments();
  virtual void handleCommandLineArguments();
  virtual void onSlicerApplicationLogicModified();

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

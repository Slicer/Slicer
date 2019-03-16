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

#ifndef __qSlicerCoreCommandOptions_h
#define __qSlicerCoreCommandOptions_h

// CTK includes
#include <ctkCommandLineParser.h>

#include "qSlicerBaseQTCoreExport.h"
#include "vtkSlicerConfigure.h" // For Slicer_USE_PYTHONQT, Slicer_BUILD_WIN32_CONSOLE

class QSettings;
class qSlicerCoreCommandOptionsPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerCoreCommandOptions : public ctkCommandLineParser
{
  Q_OBJECT
  Q_PROPERTY(bool displayHelpAndExit READ displayHelpAndExit CONSTANT)
  Q_PROPERTY(bool ignoreSlicerRC READ ignoreSlicerRC CONSTANT)
  Q_PROPERTY(QString pythonScript READ pythonScript CONSTANT)
  Q_PROPERTY(QString extraPythonScript READ extraPythonScript CONSTANT)
  Q_PROPERTY(QString pythonCode READ pythonCode CONSTANT)
  Q_PROPERTY(bool runPythonAndExit READ runPythonAndExit WRITE setRunPythonAndExit)
  Q_PROPERTY(bool disableCLIModules READ disableCLIModules CONSTANT)
  Q_PROPERTY(bool disableLoadableModules READ disableLoadableModules CONSTANT)
  Q_PROPERTY(bool disableScriptedLoadableModules READ disableScriptedLoadableModules CONSTANT)
  Q_PROPERTY(bool disableBuiltInCLIModules READ disableBuiltInCLIModules CONSTANT)
  Q_PROPERTY(bool disableBuiltInLoadableModules READ disableBuiltInLoadableModules CONSTANT)
  Q_PROPERTY(bool disableBuiltInScriptedLoadableModules READ disableBuiltInScriptedLoadableModules CONSTANT)
  Q_PROPERTY(bool displayApplicationInformation READ displayApplicationInformation CONSTANT)
  Q_PROPERTY(bool displayVersionAndExit READ displayVersionAndExit CONSTANT)
  Q_PROPERTY(bool displayProgramPathAndExit READ displayProgramPathAndExit CONSTANT)
  Q_PROPERTY(bool displayHomePathAndExit READ displayHomePathAndExit CONSTANT)
  Q_PROPERTY(bool displaySettingsPathAndExit READ displaySettingsPathAndExit CONSTANT)
  Q_PROPERTY(bool displayTemporaryPathAndExit READ displayTemporaryPathAndExit CONSTANT)
  Q_PROPERTY(bool displayMessageAndExit READ displayMessageAndExit STORED false CONSTANT)
  Q_PROPERTY(bool verboseModuleDiscovery READ verboseModuleDiscovery CONSTANT)
  Q_PROPERTY(bool disableMessageHandlers READ disableMessageHandlers CONSTANT)
  Q_PROPERTY(bool testingEnabled READ isTestingEnabled CONSTANT)
#ifdef Slicer_USE_PYTHONQT
  Q_PROPERTY(bool pythonDisabled READ isPythonDisabled CONSTANT)
#endif
  Q_PROPERTY(QStringList additionalModulePaths READ additionalModulePaths CONSTANT)
  Q_PROPERTY(QStringList modulesToIgnore READ modulesToIgnore CONSTANT)
public:
  typedef ctkCommandLineParser Superclass;
  qSlicerCoreCommandOptions();
  ~qSlicerCoreCommandOptions() override;

  /// Convenient method allowing to parse arguments
  bool parse(const QStringList& arguments);

  /// Return True if slicer should display help and exit
  bool displayHelpAndExit()const;

  /// Return True if the ignore rest argument has been passed
  bool ignoreRest() const;

  /// Return True if the loading of SlicerRC should be skipped
  bool ignoreSlicerRC()const;

  /// Return path of the python script to execute after slicer is loaded
  QString pythonScript()const;

  /// Return extra python script
  QString extraPythonScript()const;

  /// Set extra python script to be executed after slicer is loaded
  void setExtraPythonScript(const QString& newExtraPythonScript);

  /// Return python code snippet to execute after slicer is loaded
  QString pythonCode()const;

  /// Return True if Slicer should execute python code or script and exit
  bool runPythonAndExit()const;

  /// \sa runPythonAndExit
  void setRunPythonAndExit(bool value);

  /// Return list of additional module path that should be considered when searching for modules to load.
  QStringList additionalModulePaths()const;

  /// Return list of modules that should not be loaded.
  QStringList modulesToIgnore()const;

  /// Return True if the loading of any modules should be disabled
  bool disableModules()const;

  /// Return True if the loading of all built-in modules should be disabled
  bool disableBuiltInModules()const;

  /// Return True if the loading of any Command Line Modules should be disabled
  bool disableCLIModules()const;

  /// Return True if the loading of built-in Command Line Modules should be disabled
  bool disableBuiltInCLIModules()const;

  /// Return True if the loading of any Loadable Modules should be disabled
  bool disableLoadableModules()const;

  /// Return True if the loading of built-in Loadable Modules should be disabled
  bool disableBuiltInLoadableModules()const;

  /// Return True if the loading of any Scripted Loadable Modules should be disabled
  bool disableScriptedLoadableModules()const;

  /// Return True if the loading of built-in Scripted Loadable Modules should be disabled
  bool disableBuiltInScriptedLoadableModules()const;

  /// Return True if slicer should display version and exit
  bool displayVersionAndExit()const;

  /// Return True if slicer should display program path and exit
  bool displayProgramPathAndExit()const;

  /// Return True if slicer should display home path and exit
  bool displayHomePathAndExit()const;

  /// Return True if slicer should display settings path and exit
  bool displaySettingsPathAndExit()const;

  /// Return True if slicer should display temporary path and exit
  bool displayTemporaryPathAndExit()const;

  /// Return True if slicer should display a message and exit.
  /// A message is displayed if any of the
  /// function named like \a display[Something]AndExit returns True.
  /// \sa displayHelpAndExit()
  /// \sa displayVersionAndExit()
  /// \sa displayProgramPathAndExit()
  /// \sa displayHomePathAndExit()
  /// \sa displaySettingsPathAndExit()
  /// \sa displayTemporaryPathAndExit()
  virtual bool displayMessageAndExit() const;

  /// Return True if slicer should display application information in the terminal.
  bool displayApplicationInformation() const;

  /// Return True if slicer should display details regarding the module discovery process
  bool verboseModuleDiscovery()const;

  /// Return True if slicer should display information at startup
  bool verbose()const;

  /// Return True if slicer shouldn't catch messages printed to the terminal.
  bool disableMessageHandlers()const;

  /// Return True if slicer shouldn't print messages on the terminal.
  /// Messages won't be displayed on either stdout or stderr and will only be captured
  /// in the error log model.
  /// \sa qSlicerCoreApplication::errorLogModel()
  /// \sa ctkErrorLogModel::setTerminalOutputs()
  bool disableTerminalOutputs()const;

  /// Return a value indicating if slicer settings should be disabled.
  ///
  /// When disabled, temporary settings file are created.
  ///
  /// By default, temporary settings are cleared unless keepTemporarySettings()
  /// returns \a True.
  bool settingsDisabled() const;

  /// Returns a value indicating whether temporary settings should be maintained.
  ///
  /// Temporary settings are created when settingsDisabled() is \a true and
  /// are cleared by default.
  bool keepTemporarySettings() const;

  /// Return True if slicer is in testing mode.
  /// Typically set when running unit tests:
  ///  ./Slicer --testing --launch ./bin/qSlicerXXXTests ...
  /// \note Using this option implies 'disable-settings' option.
  /// \sa settingsDisabled()
  bool isTestingEnabled()const;

#ifdef Slicer_USE_PYTHONQT
  /// Return True if slicer has no python infrastructure initialized.
  /// Python is still compiled with the app, but not enabled at run-time.
  /// \sa settingsDisabled()
  bool isPythonDisabled()const;
#endif


protected:
  /// Add arguments - Called from parse() method
  /// \sa parse(const QStringList&)
  virtual void addArguments();

  /// Return the parsed arguments
  /// \sa qSlicerCoreCommandOptions::parse(const QStringList&)
  QHash<QString, QVariant> parsedArgs() const;

protected:
  QScopedPointer<qSlicerCoreCommandOptionsPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCoreCommandOptions);
  Q_DISABLE_COPY(qSlicerCoreCommandOptions);

};

#endif

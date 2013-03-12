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
#include "vtkSlicerConfigure.h" // For Slicer_USE_PYTHONQT

class QSettings;
class qSlicerCoreCommandOptionsPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerCoreCommandOptions : public ctkCommandLineParser
{
  Q_OBJECT
  Q_PROPERTY(bool displayHelpAndExit READ displayHelpAndExit)
  Q_PROPERTY(bool disableCLIModules READ disableCLIModules)
  Q_PROPERTY(bool ignoreSlicerRC READ ignoreSlicerRC)
  Q_PROPERTY(QString pythonScript READ pythonScript)
  Q_PROPERTY(QString extraPythonScript READ extraPythonScript)
  Q_PROPERTY(QString pythonCode READ pythonCode)
  Q_PROPERTY(bool runPythonAndExit READ runPythonAndExit WRITE setRunPythonAndExit)
  Q_PROPERTY(bool disableLoadableModules READ disableLoadableModules)
  Q_PROPERTY(bool displayVersionAndExit READ displayVersionAndExit)
  Q_PROPERTY(bool displayProgramPathAndExit READ displayProgramPathAndExit)
  Q_PROPERTY(bool displayHomePathAndExit READ displayHomePathAndExit)
  Q_PROPERTY(bool displaySettingsPathAndExit READ displaySettingsPathAndExit)
  Q_PROPERTY(bool displayTemporaryPathAndExit READ displayTemporaryPathAndExit)
  Q_PROPERTY(bool verboseModuleDiscovery READ verboseModuleDiscovery)
  Q_PROPERTY(bool disableMessageHandlers READ disableMessageHandlers)
  Q_PROPERTY(bool testingEnabled READ isTestingEnabled)
#ifdef Slicer_USE_PYTHONQT
  Q_PROPERTY(bool pythonDisabled READ isPythonDisabled)
#endif
  Q_PROPERTY(QStringList additonalModulePaths READ additonalModulePaths)
public:
  typedef ctkCommandLineParser Superclass;
  qSlicerCoreCommandOptions();
  virtual ~qSlicerCoreCommandOptions();

  /// Convenient method allowing to parse arguments
  bool parse(const QStringList& arguments);

  /// Return True if slicer should display help and exit
  bool displayHelpAndExit()const;

  /// Return True if the ignore rest argument has been passed
  bool ignoreRest() const;

  /// Return True if the loading of Command Line Modules should be disabled
  bool disableCLIModules()const;

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
  QStringList additonalModulePaths()const;

  /// Return True if the loading of Loadable Modules should be disabled
  bool disableLoadableModules()const;

  /// Return True if the loading of Scripted Loadable Modules should be disabled
  bool disableScriptedLoadableModules()const;

  /// Return True if slicer should display version and exit
  bool displayVersionAndExit()const;

  /// Return True if slicer should display program path and exit
  bool displayProgramPathAndExit()const;

  /// Return True if slicer should display home path and exit
  bool displayHomePathAndExit()const;

  /// Return True if slicer should display settings path and exit
  bool displaySettingsPathAndExit()const;

  /// Return True if slicer should display settings path and exit
  bool displayTemporaryPathAndExit()const;

  /// Return True if slicer should display details regarding the module discovery process
  bool verboseModuleDiscovery()const;

  /// Return True if slicer shouldn't catch messages printed to the terminal.
  bool disableMessageHandlers()const;

  /// Return True if slicer settings are ignored
  bool settingsDisabled() const;

  /// Return True if slicer is in testing mode.
  /// Typically set when running unit tests:
  ///  ./Slicer --testing --launch ./bin/qSlicerXXXTests ...
  /// \note Using this option is equivalent to 'disable-settings' option. Note that
  /// this may change in the future.
  /// \sa settingsEnabled()
  bool isTestingEnabled()const;

#ifdef Slicer_USE_PYTHONQT
  /// Return True if slicer has no python infrastructure initialized.
  /// Python is still compiled with the app, but not enabled at run-time.
  /// \sa settingsEnabled()
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

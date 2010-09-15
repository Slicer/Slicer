/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <ctkPimpl.h>
#include <ctkCommandLineParser.h>

#include "qSlicerBaseQTCoreExport.h"

class QSettings;
class qSlicerCoreCommandOptionsPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerCoreCommandOptions : public ctkCommandLineParser
{
public:
  typedef ctkCommandLineParser Superclass;
  qSlicerCoreCommandOptions(QSettings* _settings);
  virtual ~qSlicerCoreCommandOptions();

  /// Convenient method allowing to parse arguments
  bool parse(const QStringList& arguments);

  /// Return True if slicer should display help and exit
  bool displayHelpAndExit()const;

  /// Return True if the ignore rest argument has been passed
  bool ignoreRest() const;

  /// Return True if the loading of Command Line Modules should be disabled
  bool disableCLIModule()const;

  /// Return True if the loading of Loadable Modules should be disabled
  bool disableLoadableModule()const;

  /// Return True if slicer should display version and exit
  bool displayVersionAndExit()const;

  /// Return True if slicer should display program path and exit
  bool displayProgramPathAndExit()const;

  /// Return True if slicer should display home path and exit
  bool displayHomePathAndExit()const;

  /// Return True if slicer should display settings path and exit
  bool displaySettingsPathAndExit()const;

  /// Return True if slicer should display details regarding the module discovery process
  bool verboseModuleDiscovery()const;

  /// Return temp directory
  /// Try to read the value from the setting, if no value is found it defaults to
  /// the value returned by QDir::tempPath()
  /// See http://qt.nokia.com/doc/4.6/qdir.html#tempPath
  QString tempDirectory()const;

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

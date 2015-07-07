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

#ifndef __qSlicerCoreApplication_p_h
#define __qSlicerCoreApplication_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Qt includes
#include <QProcessEnvironment>
#include <QSettings>
#include <QSharedPointer>

// SlicerQt includes
#include "qSlicerBaseQTCoreExport.h"
#include "qSlicerCoreApplication.h"

// VTK includes
#include <vtkSmartPointer.h>

class vtkCacheManager;
class vtkDataIOManagerLogic;
class vtkMRMLRemoteIOLogic;

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTCORE_EXPORT qSlicerCoreApplicationPrivate
{
  Q_DECLARE_PUBLIC(qSlicerCoreApplication);
protected:
  qSlicerCoreApplication* q_ptr;
public:
  typedef qSlicerCoreApplicationPrivate Self;

  qSlicerCoreApplicationPrivate(qSlicerCoreApplication& object,
                                qSlicerCoreCommandOptions * coreCommandOptions,
                                qSlicerCoreIOManager * coreIOManager);
  virtual ~qSlicerCoreApplicationPrivate();

  virtual void init();

  /// Set up the local and remote data input/output for this application.
  /// Use this as a template for creating stand alone scenes, then call
  /// vtkSlicerApplicationLogic::SetMRMLSceneDataIO to hook it into a scene.
  virtual void initDataIO();

  /// Instanciate settings object
  virtual QSettings* newSettings();
  QSettings* instantiateSettings(bool useTmp);

  /// Return true is this instance of Slicer is running from an installed directory
  bool isInstalled(const QString& slicerHome)const;

  /// Given the program name, should return Slicer Home Directory
  QString discoverSlicerHomeDirectory();

  /// Set environment variable
  void setEnvironmentVariable(const QString& key, const QString& value);

#ifdef Slicer_USE_PYTHONQT
  void setPythonOsEnviron(const QString& key, const QString& value);
#endif

#ifdef Q_WS_WIN
  void updatePythonOsEnviron();
#endif

  /// Prepend or append value to environment variable using \a separator
  void updateEnvironmentVariable(
    const QString& key, const QString& value, QChar separator, bool prepend = false);

  /// If it successfully obtains 'applicationDirPath()', returns Slicer binary directory
  /// \note SlicerBin doesn't contain Debug/Release/... (see IntDir)
  /// \sa QCoreApplication::applicationDirPath
  QString discoverSlicerBinDirectory();

  /// Set 'ITKFactoriesDir' variable using 'ITK_AUTOLOAD_PATH' environment variable
  QString discoverITKFactoriesDirectory();

  /// Set PYTHONHOME and PYTHONPATH environment variables is not already set.
  void setPythonEnvironmentVariables();

  /// Set TCL_LIBRARY, TK_LIBRARY and TCLLIBPATH environment variable is not already set.
  void setTclEnvironmentVariables();

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  QString defaultExtensionsInstallPathForMacOSX()const;
#endif

  /// Convenient function used to create a \a path.
  /// If it fails, print an error message using given \a description
  bool createDirectory(const QString& path, const QString& description) const;

  /// Parse arguments
  void parseArguments();

public:
  /// MRMLScene and AppLogic pointers
  vtkSmartPointer<vtkMRMLScene>               MRMLScene;
  vtkSmartPointer<vtkSlicerApplicationLogic>  AppLogic;
  vtkSmartPointer<vtkMRMLRemoteIOLogic>       MRMLRemoteIOLogic;

  /// Data manager
  vtkSmartPointer<vtkDataIOManagerLogic>      DataIOManagerLogic;

  QString                                     SlicerHome;
  QString                                     ITKFactoriesDir;
  /// On windows platform, after the method 'discoverSlicerBinDirectory' has been called,
  /// IntDir should be set to either Debug,
  /// Release, RelWithDebInfo, MinSizeRel or any other custom build type.
  QString                                     IntDir;

  QSettings*                                  DefaultSettings;
  QSettings*                                  UserSettings;
  QSettings*                                  RevisionUserSettings;

  /// ModuleManager - It should exist only one instance of the factory
  QSharedPointer<qSlicerModuleManager>        ModuleManager;

  /// CoreIOManager - It should exist only one instance of the IOManager
  QSharedPointer<qSlicerCoreIOManager>        CoreIOManager;

  /// CoreCommandOptions - It should exist only one instance of the CoreCommandOptions
  QSharedPointer<qSlicerCoreCommandOptions>   CoreCommandOptions;

  /// ReturnCode flag
  int                                         ReturnCode;

#ifdef Slicer_USE_PYTHONQT
  /// CorePythonManager - It should exist only one instance of the CorePythonManager
  QSharedPointer<qSlicerCorePythonManager>    CorePythonManager;
#endif

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  QSharedPointer<qSlicerExtensionsManagerModel> ExtensionsManagerModel;
#endif

  QProcessEnvironment                         Environment;

#if defined(Slicer_USE_PYTHONQT) && defined(Q_WS_WIN)
  QHash<QString, QString>                     EnvironmentVariablesCache;
#endif

#ifdef Slicer_BUILD_DICOM_SUPPORT
  /// Application-wide database instance
  ctkDICOMDatabase*                           DICOMDatabase;
#endif

  QHash<int, QByteArray>                      LoadedResources;
  int                                         NextResourceHandle;
};

#endif


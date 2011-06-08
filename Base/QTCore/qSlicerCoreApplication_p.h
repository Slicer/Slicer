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

#ifndef __qSlicerCoreApplication_p_h
#define __qSlicerCoreApplication_p_h

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
  
  /// Instanciate settings object
  QSettings* instantiateSettings(const QString& suffix, bool useTmp);

  /// Given the program name, should return Slicer Home Directory
  QString discoverSlicerHomeDirectory();

  /// Set environment variable
  void setEnvironmentVariable(const QString& key, const QString& value);

  /// If it successfully obtains 'applicationDirPath()', sets the variable 'SlicerBin'
  /// \sa QCoreApplication::applicationDirPath
  void discoverSlicerBinDirectory();

  /// Set 'ITKFactoriesDir' variable using 'ITK_AUTOLOAD_PATH' environment variable
  QString discoverITKFactoriesDirectory();

  /// Set variable 'Repository{Url, Revision, Url}' and Platform extracting information
  /// from SlicerVersion.txt
  void discoverRepository();

  /// Detect PythonPath. If one doesn't exist attempt to generate one.
  void discoverPythonPath();

  /// Parse arguments
  void parseArguments();

  /// Set the ExitWhenDone flag to True
  void terminate();

public:
  /// MRMLScene and AppLogic pointers
  vtkSmartPointer<vtkMRMLScene>               MRMLScene;
  vtkSmartPointer<vtkSlicerApplicationLogic>  AppLogic;
  vtkSmartPointer<vtkMRMLApplicationLogic>    MRMLApplicationLogic;
  vtkSmartPointer<vtkMRMLRemoteIOLogic>       MRMLRemoteIOLogic;

  /// Data manager
  vtkSmartPointer<vtkDataIOManagerLogic>      DataIOManagerLogic;

  /// SlicerBin doesn't contain Debug/Release/... (see IntDir)
  QString                                     SlicerBin;
  QString                                     SlicerHome;
  QString                                     ITKFactoriesDir;
  /// On windows platform, after the method 'discoverSlicerBinDirectory' has been called,
  /// IntDir should be set to either Debug,
  /// Release, RelWithDebInfo, MinSizeRel or any other custom build type.
  QString                                     IntDir;
  
  /// Variable set extracting information from SlicerVersion.txt
  /// \sa discoverRepository
  QString                                     RepositoryUrl;
  QString                                     RepositoryBranch;
  QString                                     RepositoryRevision;
  QString                                     Platform;
  
  QSettings*                                  Settings;

  /// ModuleManager - It should exist only one instance of the factory
  QSharedPointer<qSlicerModuleManager>        ModuleManager;

  /// CoreIOManager - It should exist only one instance of the IOManager
  QSharedPointer<qSlicerCoreIOManager>        CoreIOManager;

  /// CoreCommandOptions - It should exist only one instance of the CoreCommandOptions
  QSharedPointer<qSlicerCoreCommandOptions>   CoreCommandOptions;

  /// CoreCommandOptions - It should exist only one instance of the CoreCommandOptions
  QSharedPointer<ctkErrorLogModel>            ErrorLogModel;

  /// ExitWhenDone flag
  bool                                        ExitWhenDone;

#ifdef Slicer_USE_PYTHONQT
  /// CorePythonManager - It should exist only one instance of the CorePythonManager
  QSharedPointer<qSlicerCorePythonManager>    CorePythonManager;
#endif

  QProcessEnvironment                         Environment;

#if defined(Slicer_USE_PYTHONQT) && defined(Q_WS_WIN)
  QHash<QString, QString>                     EnvironmentVariablesCache;
#endif
};

#endif


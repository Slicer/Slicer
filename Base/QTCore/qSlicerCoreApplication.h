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

#ifndef __qSlicerCoreApplication_h
#define __qSlicerCoreApplication_h

// Qt includes
#include <QApplication>

// CTK includes
#include <ctkPimpl.h>

#include "vtkSlicerConfigure.h" // For Slicer_USE_KWWIDGETS, Slicer_USE_PYTHONQT

#include "qSlicerBaseQTCoreExport.h"

class QSettings;
class qSlicerCoreIOManager;
class qSlicerCoreCommandOptions;
class qSlicerCoreApplicationPrivate;
class qSlicerModuleManager;
#ifdef Slicer_USE_PYTHONQT
class qSlicerCorePythonManager;
#endif
class vtkSlicerApplicationLogic;
class vtkMRMLApplicationLogic;
class vtkMRMLScene;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerCoreApplication : public QApplication
{
  Q_OBJECT

public:

  typedef QApplication Superclass;
  qSlicerCoreApplication(int &argc, char **argv);
  virtual ~qSlicerCoreApplication();

  ///
  /// Return a reference to the application singleton
  static qSlicerCoreApplication* application();

  ///
  /// Instantiate MRMLScene and Application logic.
  ///  - Configure scene
  ///  - AppLogic is set as a scene observer.
  ///  - Create processing thread
  /// If exitWhenDone is True, it's your responsability to exit the application
  void initialize(bool& exitWhenDone);

  ///
  /// Return true if the application has been initialized
  /// Note: initialize() should be called only one time.
  bool initialized() const;

  ///
  /// Get MRML Scene
  vtkMRMLScene* mrmlScene() const;

  ///
  /// Set MRML Scene
  /// DEPRECATED: This method was used by the KWWidgests GUI only and it will be
  /// moved as a protected member.
  void setMRMLScene(vtkMRMLScene * scene);

#ifdef Slicer_USE_KWWIDGETS

  ///
  /// Convenient method allowing to set the initialized flag
  /// DEPRECATED: This method was used by the KWWidgests GUI only and it will be
  /// removed once the QT GUI is functional. Only vtkSlicerApplication should use
  /// that method. Improper use of that function may lead to unstable state
  void setInitialized(bool initialized);

  ///
  /// Set application logic
  /// DEPRECATED: This method was used by the KWWidgets GUI only and it will be
  /// removed once the QT GUI is functional.
  void setAppLogic(vtkSlicerApplicationLogic* appLogic);

  ///
  /// Set the module manager
  /// Note that qSlicerCoreApplication takes ownership of the object
  /// DEPRECATED: This method was used by the KWWidgets GUI only and it will be
  /// removed once the QT GUI is functional.
  void setModuleManager(qSlicerModuleManager* moduleManager);

#endif //Slicer_USE_KWWIDGETS

  /// Get application logic
  vtkSlicerApplicationLogic* appLogic() const;

  /// Get MRML ApplicationLogic
  vtkMRMLApplicationLogic* mrmlApplicationLogic() const;

  ///
  /// Get slicer home directory
  /// Valid only if qSlicerCoreApplication is initialized. 
  QString slicerHome() const;

  ///
  /// If any, this method return the build intermediate directory
  /// See $(IntDir) on http://msdn.microsoft.com/en-us/library/c02as0cs%28VS.71%29.aspx
  QString intDir()const;

  ///
  /// Return true is this instance of Slicer is running from an installed directory
  bool isInstalled()const;

#ifdef Slicer_USE_PYTHONQT
  ///
  /// Get python manager
  qSlicerCorePythonManager* corePythonManager()const;
  
  /// Set the IO manager
  /// Note that qSlicerCoreApplication takes ownership of the object
  void setCorePythonManager(qSlicerCorePythonManager* pythonManager);

#endif

  ///
  /// Get the module manager
  qSlicerModuleManager* moduleManager()const;

  ///
  /// Get the IO manager
  qSlicerCoreIOManager* coreIOManager()const;

  /// Set the IO manager
  /// Note that qSlicerCoreApplication takes ownership of the object
  void setCoreIOManager(qSlicerCoreIOManager* ioManager);

  ///
  /// Get coreCommandOptions
  qSlicerCoreCommandOptions* coreCommandOptions()const;

  /// Set coreCommandOptions
  /// Note that qSlicerCoreApplication takes ownership of the object
  void setCoreCommandOptions(qSlicerCoreCommandOptions* options);

  ///
  /// Get application settings
  /// Note that his method will also instanciate a QSettings object if required.
  QSettings* settings();

  ///
  /// Disable application settings
  /// Instanciate a new empty ctkSettings object and associate it with this instance.
  /// Note that the original settings won't deleted.
  void disableSettings();

  /// Clear application settings
  void clearSettings();

  /// Return the copyrights of Slicer
  virtual QString copyrights()const;

protected:
  ///
  virtual void handlePreApplicationCommandLineArguments();
  ///
  virtual QSettings* newSettings(const QString& organization, const QString& application);

protected slots:

  ///
  virtual void handleCommandLineArguments();

signals:
  void mrmlSceneChanged(vtkMRMLScene* mrmlScene);

protected:
  QScopedPointer<qSlicerCoreApplicationPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCoreApplication);
  Q_DISABLE_COPY(qSlicerCoreApplication);
};

#endif

/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerCoreApplication_h
#define __qSlicerCoreApplication_h

// Qt includes
#include <QApplication>

// CTK includes
#include <ctkPimpl.h>

#include "vtkSlicerConfigure.h" // For Slicer3_USE_KWWIDGETS, Slicer3_USE_PYTHONQT

#include "qSlicerBaseQTCoreExport.h"

class vtkSlicerApplicationLogic;
class vtkMRMLScene;
class qSlicerModuleManager;
class qSlicerCoreIOManager;
class qSlicerCoreCommandOptions; 
class qSlicerCoreApplicationPrivate;
class ctkSettings;
#ifdef Slicer3_USE_PYTHONQT
class qSlicerPythonManager;
#endif

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

#ifdef Slicer3_USE_KWWIDGETS

  /// 
  /// initialize paths for module discovery
  /// DEPRECATED: This method was used by the KWWidgets GUI only and it will be
  /// removed once the QT GUI is functional.
  void initializePaths(const QString& programPath);
  
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

#endif //Slicer3_USE_KWWIDGETS

  /// Get application logic
  vtkSlicerApplicationLogic* appLogic() const;
  
  /// 
  /// Get slicer home directory
  QString slicerHome() const;

  /// 
  /// Convenient method to set slicer home directory
  /// Note: SlicerHome should be auto-discovered. See implementation details.
  void setSlicerHome(const QString& slicerHome);

  /// 
  /// If any, this method return the build intermediate directory
  /// See $(IntDir) on http://msdn.microsoft.com/en-us/library/c02as0cs%28VS.71%29.aspx
  QString intDir()const;

  ///
  /// Return true is this instance of Slicer is running from an installed directory
  bool isInstalled();

#ifdef Slicer3_USE_PYTHONQT
  ///
  /// Get python manager
  qSlicerPythonManager* pythonManager()const;
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
  /// Note that his method will also instanciate a ctkSettings object if required.
  ctkSettings* settings();

  ///
  /// Disable application settings
  /// Instanciate a new empty ctkSettings object and associate it with this instance.
  /// Note that the original settings won't deleted.
  void disableSettings();

  /// Clear application settings
  void clearSettings();

protected:
  
  /// 
  virtual void handlePreApplicationCommandLineArguments();

protected slots:

  ///
  virtual void handleCommandLineArguments();

signals:
  void currentMRMLSceneChanged(vtkMRMLScene* mrmlScene);

private:
  CTK_DECLARE_PRIVATE(qSlicerCoreApplication);
};

#endif

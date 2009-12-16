/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerCoreApplication_h
#define __qSlicerCoreApplication_h

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QApplication>

#include "qSlicerBaseQTCoreExport.h"

class vtkSlicerApplicationLogic;
class vtkMRMLScene;
class qSlicerModuleManager;
class qSlicerCoreIOManager; 
class qSlicerCoreApplicationPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerCoreApplication : public QApplication
{
  Q_OBJECT

public:

  typedef QApplication Superclass;
  qSlicerCoreApplication(int &argc, char **argv);
  virtual ~qSlicerCoreApplication();

  // Description:
  // Return a reference to the application singleton
  static qSlicerCoreApplication* application();

  // Description:
  // Instantiate MRMLScene and Application logic.
  //  - Configure scene
  //  - AppLogic is set as a scene observer.
  //  - Create processing thread
  void initialize();

  // Description:
  // Return true if the application has been initialized
  // Note: initialize() should be called only one time.
  bool initialized() const; 

  // Description:
  // Set/Get MRML Scene
  void setMRMLScene(vtkMRMLScene * scene);
  vtkMRMLScene* mrmlScene() const;

  // Description:
  // Set/Get application logic
  void setAppLogic(vtkSlicerApplicationLogic* appLogic);
  vtkSlicerApplicationLogic* appLogic() const;
  
  // Description:
  // Get slicer home directory
  QString slicerHome();

  // Description:
  // Convenient method to set slicer home directory
  // Note: SlicerHome should be auto-discovered. See implementation details.
  void setSlicerHome(const QString& slicerHome);

  // Description:
  // Initialize paths
  void initializeLoadableModulesPaths();
  void initializeCmdLineModulesPaths();

  // Description:
  // Set/Get the module manager
  qSlicerModuleManager* moduleManager()const;
  void setModuleManager(qSlicerModuleManager* moduleManager);

  // Description:
  // Set/Get the IO manager
  qSlicerCoreIOManager* coreIOManager()const;
  void setCoreIOManager(qSlicerCoreIOManager* ioManager); 

signals:
  void currentMRMLSceneChanged(vtkMRMLScene* mrmlScene);

private:
  QCTK_DECLARE_PRIVATE(qSlicerCoreApplication);
};

#endif

#ifndef __qSlicerCoreApplication_h
#define __qSlicerCoreApplication_h

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QApplication>

#include "qSlicerBaseQTGUIWin32Header.h"

class vtkSlicerApplicationLogic;
class vtkMRMLScene;
class qSlicerCoreApplicationPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerCoreApplication : public QApplication
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


signals:
  void currentMRMLSceneChanged(vtkMRMLScene* mrmlScene);

private:
  QCTK_DECLARE_PRIVATE(qSlicerCoreApplication);
};

#endif

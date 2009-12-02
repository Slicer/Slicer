#ifndef __qSlicerApplication_h
#define __qSlicerApplication_h

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QApplication>

#include "qSlicerBaseQTGUIWin32Header.h"

class qSlicerAbstractModule;
// class qSlicerAbstractModulePanel;
class qSlicerWidget;
class vtkSlicerApplicationLogic;
class vtkMRMLScene;
class qSlicerApplicationPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerApplication : public QApplication
{
  Q_OBJECT

public:

  typedef QApplication Superclass;
  qSlicerApplication(int &argc, char **argv);
  virtual ~qSlicerApplication();

  // Description:
  // Return a reference to the application singleton
  static qSlicerApplication* application();

  // Description:
  // Instantiate MRMLScene and Application logic.
  //  - Configure scene
  //  - AppLogic is set as a scene observer.
  //  - Create processing thread
  void initialize();

  // Description:
  // Set the visibility of the top level widgets.
  // If set to 'False', the state of the widget will be saved.
  // If set to 'True, if possible the saved state will be restored.
  void setTopLevelWidgetsVisible(bool visible);

  // Description:
  // Set the visibility of a given top level widgets
  void setTopLevelWidgetVisible(qSlicerWidget* widget, bool visible);

  // Description:
  // Set/Get default window flags that could be used when displaying top level widgets
  void setDefaultWindowFlags(Qt::WindowFlags defaultWindowFlags);
  Qt::WindowFlags defaultWindowFlags() const;

  // Description:
  // Set/Get MRML Scene
  void setMRMLScene(vtkMRMLScene * scene);
  vtkMRMLScene* mrmlScene() const;

  // Description:
  // Set/Get application logic
  void setAppLogic(vtkSlicerApplicationLogic* appLogic);
  vtkSlicerApplicationLogic* appLogic() const;

  // Description:
  // Return slicer home directory
  QString slicerHome();
  void setSlicerHome(const QString& slicerHome);

  // Description:
  // Initialize paths
  void initializeLoadableModulesPaths();
  void initializeCmdLineModulesPaths();

signals:
  void currentMRMLSceneChanged(vtkMRMLScene* mrmlScene);

// public slots:
//   void showModule(qSlicerAbstractModule*);

private:
  QCTK_DECLARE_PRIVATE(qSlicerApplication);
};

#endif

#ifndef __qSlicerApplication_h
#define __qSlicerApplication_h 

#include <QApplication>

#include "qSlicerBaseQTGUIWin32Header.h"

class vtkMRMLScene; 
class qSlicerAbstractModule;
class qSlicerAbstractModulePanel;
class qSlicerWidget;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerApplication : public QApplication
{
  Q_OBJECT

public:

  typedef QApplication Superclass;
  qSlicerApplication(int &argc, char **argv);
  virtual ~qSlicerApplication();
  
  // Description:
  // Set the visibility of the top level widgets. 
  // If set to 'False', the state of the widget will be saved.
  // If set to 'True, if possible the saved state will be restored.
  void setTopLevelWidgetsVisible(bool visible); 
  
  // Description:
  // Set/Get MRML Scene
  void setMRMLScene(vtkMRMLScene * scene);
  vtkMRMLScene* getMRMLScene();

  static qSlicerApplication* application();
  
  // Description:
  // Return slicer home directory
  QString slicerHome();
  void setSlicerHome(const QString& slicerHome);
  
  void initializeLoadableModulesPaths();
  void initializeCmdLineModulesPaths();

  qSlicerAbstractModulePanel* modulePanel();
  void setTopLevelWidgetVisibility(qSlicerWidget* widget, bool visible);

public slots:
  void showModule(qSlicerAbstractModule*);

protected:
  // Description:
  // Initialize application Palette/Font
  void initPalette(); 
  void initFont(); 
  
  // Description:
  // Load application styleSheet
  void loadStyleSheet(); 

private:
  class qInternal;
  qInternal* Internal;
};

#endif

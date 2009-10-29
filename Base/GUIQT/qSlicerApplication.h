#ifndef __qSlicerApplication_h
#define __qSlicerApplication_h 

#include <QApplication>

#include "qSlicerBaseGUIQTWin32Header.h"

class vtkMRMLScene; 

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerApplication : public QApplication
{
  Q_OBJECT

public:

  typedef QApplication Superclass;
  qSlicerApplication(int &argc, char **argv);
  virtual ~qSlicerApplication();
  
  // Description:
  // Set/Get MRML Scene
  void setMRMLScene(vtkMRMLScene * scene);
  vtkMRMLScene* getMRMLScene();

  static qSlicerApplication* application();

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

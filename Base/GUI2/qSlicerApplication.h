#ifndef __qSlicerApplication_h
#define __qSlicerApplication_h 

#include "qSlicerBaseGUI2Win32Header.h"

#include <QApplication>

class vtkMRMLScene; 

class Q_SLICER_BASE_GUI2_EXPORT qSlicerApplication : public QApplication
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
  
private:
  class qInternal;
  qInternal* Internal;
};

#endif

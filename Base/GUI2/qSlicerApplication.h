#ifndef __qSlicerApplication_h
#define __qSlicerApplication_h 

#include <QApplication>

#include "qSlicerBaseGUI2Win32Header.h"

class vtkMRMLScene; 
#ifdef Slicer3_USE_KWWidget
class vtkSlicerApplication;
#endif

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

#ifdef Slicer3_USE_KWWidget
  void setSlicerApplication(vtkSlicerApplication*);
  vtkSlicerApplication* getSlicerApplication()const;
#endif

  static qSlicerApplication* application();
  
private:
  class qInternal;
  qInternal* Internal;
};

#endif

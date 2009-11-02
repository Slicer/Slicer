#ifndef __qSlicerCameraModule_h
#define __qSlicerCameraModule_h 

#include "qSlicerAbstractCoreModule.h"

#include "qSlicerBaseGUIQTWin32Header.h"

class vtkMRMLNode; 

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerCameraModule : 
  public qSlicerAbstractCoreModule
{ 
  Q_OBJECT
public:

  typedef qSlicerAbstractCoreModule Superclass;
  qSlicerCameraModule(QWidget *parent=0);
  virtual ~qSlicerCameraModule(); 

  virtual void printAdditionalInfo();
  
  virtual QString moduleTitle();
  
  // Return help/about text
  virtual QString helpText();
  virtual QString aboutText();

protected slots:
  void onViewNodeSelected(vtkMRMLNode*);

private:
  class qInternal;
  qInternal* Internal;
};

#endif

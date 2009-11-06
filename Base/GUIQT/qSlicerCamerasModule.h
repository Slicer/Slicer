#ifndef __qSlicerCamerasModule_h
#define __qSlicerCamerasModule_h 

#include "qSlicerAbstractCoreModule.h"

#include "qSlicerBaseGUIQTWin32Header.h"

class vtkMRMLNode; 

class Q_SLICER_BASE_GUIQT_EXPORT qSlicerCamerasModule : 
  public qSlicerAbstractCoreModule
{ 
  Q_OBJECT
public:

  typedef qSlicerAbstractCoreModule Superclass;
  qSlicerCamerasModule(QWidget *parent=0);
  virtual ~qSlicerCamerasModule(); 

  virtual void printAdditionalInfo();
  
  qSlicerGetModuleTitleMacro("Cameras");
  
  // Return help/about text
  virtual QString helpText();
  virtual QString aboutText();

protected:
  virtual void initializer();
  
protected slots:
  void onViewNodeSelected(vtkMRMLNode*);

private:
  class qInternal;
  qInternal* Internal;
};

#endif

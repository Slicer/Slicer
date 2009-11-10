#ifndef __qSlicerCamerasModule_h
#define __qSlicerCamerasModule_h 

#include "qSlicerAbstractCoreModule.h"

#include "qSlicerBaseGUIQTWin32Header.h"

class vtkMRMLNode; 
class vtkMRMLViewNode;

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
  
  // Return help/acknowledgement text
  virtual QString helpText();
  virtual QString acknowledgementText();
  
public slots:
  // Description:
  // Inherited from qSlicerWidget. Reimplemented for refresh issues.
  virtual void setMRMLScene(vtkMRMLScene*);

protected:
  virtual void initializer();
  void synchronizeCameraWithView(vtkMRMLViewNode* currentViewNode);
  
protected slots:
  void onCurrentViewNodeChanged(vtkMRMLNode*);
  void onCurrentCameraNodeChanged(vtkMRMLNode*);
  void onCameraNodeAdded(vtkMRMLNode*);
  void onCameraNodeRemoved(vtkMRMLNode*);
  void synchronizeCameraWithView(void * currentView= 0);

private:
  class qInternal;
  qInternal* Internal;
};

#endif

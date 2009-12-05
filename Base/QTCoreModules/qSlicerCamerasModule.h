#ifndef __qSlicerCamerasModule_h
#define __qSlicerCamerasModule_h

// Slicer includes
#include "qSlicerAbstractCoreModule.h"

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerBaseQTCoreModulesWin32Header.h"

class vtkMRMLNode;
class vtkMRMLViewNode;
class qSlicerCamerasModulePrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerCamerasModule :
  public qSlicerAbstractCoreModule
{
  Q_OBJECT
public:

  typedef qSlicerAbstractCoreModule Superclass;
  qSlicerCamerasModule(QWidget *parent=0);

  virtual void printAdditionalInfo();

  qSlicerGetTitleMacro("Cameras");

  virtual QAction* showModuleAction();

  // Return help/acknowledgement text
  virtual QString helpText()const;
  virtual QString acknowledgementText()const;

public slots:
  // Description:
  // Inherited from qSlicerWidget. Reimplemented for refresh issues.
  virtual void setMRMLScene(vtkMRMLScene*);

protected:
  virtual void setup();
  void synchronizeCameraWithView(vtkMRMLViewNode* currentViewNode);

protected slots:
  void onCurrentViewNodeChanged(vtkMRMLNode*);
  void onCurrentCameraNodeChanged(vtkMRMLNode*);
  void onCameraNodeAdded(vtkMRMLNode*);
  void onCameraNodeRemoved(vtkMRMLNode*);
  void synchronizeCameraWithView(void * currentView= 0);

private:
  QCTK_DECLARE_PRIVATE(qSlicerCamerasModule);
};

#endif

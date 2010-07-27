#ifndef __qSlicerSliceControllerModuleWidget_h
#define __qSlicerSliceControllerModuleWidget_h


// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerBaseQTCoreModulesExport.h"

class qSlicerSliceControllerModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerSliceControllerModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerSliceControllerModuleWidget(QWidget *parent=0);

public slots:
  virtual void setMRMLScene(vtkMRMLScene *newScene);

protected:
  virtual void setup();

private:
  CTK_DECLARE_PRIVATE(qSlicerSliceControllerModuleWidget);
};

#endif

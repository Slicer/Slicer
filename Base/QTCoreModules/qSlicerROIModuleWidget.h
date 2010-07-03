#ifndef __qSlicerROIModuleWidget_h
#define __qSlicerROIModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerBaseQTCoreModulesExport.h"

class vtkMRMLNode;
class vtkMRMLViewNode;
class qSlicerROIModuleWidgetPrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerROIModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
public:
  qSlicerROIModuleWidget(QWidget *parentWidget = 0);

protected:
  virtual void setup();

private:
  CTK_DECLARE_PRIVATE(qSlicerROIModuleWidget);
};

#endif

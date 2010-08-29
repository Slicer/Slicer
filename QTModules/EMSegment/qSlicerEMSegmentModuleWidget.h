#ifndef __qSlicerEMSegmentModuleWidget_h
#define __qSlicerEMSegmentModuleWidget_h


// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentModuleWidgetPrivate;
class vtkMRMLNode;
class ctkWorkflowStep;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  const static QString SimpleMode;
  const static QString AdvancedMode;

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerEMSegmentModuleWidget(QWidget *parent=0);

protected:
  virtual void setup();

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentModuleWidget);
};

#endif

#ifndef __qSlicerEMSegmentModuleWidget_h
#define __qSlicerEMSegmentModuleWidget_h


// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerEMSegmentModuleWidget(QWidget *parent=0);

  virtual QAction* showModuleAction();

public slots:


protected:
  virtual void setup();

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentModuleWidget);
};

#endif

#ifndef __qSlicerEMSegmentEditNodeBasedParametersPanel_h
#define __qSlicerEMSegmentEditNodeBasedParametersPanel_h 

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentEditNodeBasedParametersPanelPrivate;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentEditNodeBasedParametersPanel : public qSlicerWidget
{ 
  Q_OBJECT

public:

  typedef qSlicerWidget Superclass;
  qSlicerEMSegmentEditNodeBasedParametersPanel(QWidget *parent=0);
  
  virtual void printAdditionalInfo();

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentEditNodeBasedParametersPanel);
};

#endif

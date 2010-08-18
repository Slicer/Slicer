#ifndef __qSlicerEMSegmentEditNodeBasedParametersPanel_h
#define __qSlicerEMSegmentEditNodeBasedParametersPanel_h 

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentEditNodeBasedParametersPanelPrivate;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentEditNodeBasedParametersPanel :
    public qSlicerEMSegmentWidget
{ 
  Q_OBJECT

public:

  typedef qSlicerEMSegmentWidget Superclass;
  qSlicerEMSegmentEditNodeBasedParametersPanel(QWidget *newParent=0);
  
  virtual void printAdditionalInfo();

  virtual void setup();

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentEditNodeBasedParametersPanel);
};

#endif

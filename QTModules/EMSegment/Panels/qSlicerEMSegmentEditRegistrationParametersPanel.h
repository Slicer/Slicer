#ifndef __qSlicerEMSegmentEditRegistrationParametersPanel_h
#define __qSlicerEMSegmentEditRegistrationParametersPanel_h 

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentEditRegistrationParametersPanelPrivate;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentEditRegistrationParametersPanel : public qSlicerWidget
{ 
  Q_OBJECT

public:

  typedef qSlicerWidget Superclass;
  qSlicerEMSegmentEditRegistrationParametersPanel(QWidget *parent=0);
  
  virtual void printAdditionalInfo();

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentEditRegistrationParametersPanel);
};

#endif

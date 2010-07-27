#ifndef __qSlicerEMSegmentSpecifyIntensityDistributionPanel_h
#define __qSlicerEMSegmentSpecifyIntensityDistributionPanel_h 

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentSpecifyIntensityDistributionPanelPrivate;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentSpecifyIntensityDistributionPanel : public qSlicerWidget
{ 
  Q_OBJECT

public:

  typedef qSlicerWidget Superclass;
  qSlicerEMSegmentSpecifyIntensityDistributionPanel(QWidget *parent=0);
  
  virtual void printAdditionalInfo();

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentSpecifyIntensityDistributionPanel);
};

#endif

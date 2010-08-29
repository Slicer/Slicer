#ifndef __qSlicerEMSegmentSpecifyIntensityDistributionPanel_h
#define __qSlicerEMSegmentSpecifyIntensityDistributionPanel_h 

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentSpecifyIntensityDistributionPanelPrivate;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentSpecifyIntensityDistributionPanel :
    public qSlicerEMSegmentWidget
{ 
  Q_OBJECT

public:

  typedef qSlicerEMSegmentWidget Superclass;
  qSlicerEMSegmentSpecifyIntensityDistributionPanel(QWidget *newParent=0);

  void updateWidgetFromMRML();

public slots:
  void openGraphDialog();

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentSpecifyIntensityDistributionPanel);
};

#endif

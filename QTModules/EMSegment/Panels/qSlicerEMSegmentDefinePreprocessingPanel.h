#ifndef __qSlicerEMSegmentDefinePreprocessingPanel_h
#define __qSlicerEMSegmentDefinePreprocessingPanel_h 

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentDefinePreprocessingPanelPrivate;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentDefinePreprocessingPanel :
    public qSlicerEMSegmentWidget
{ 
  Q_OBJECT

public:

  typedef qSlicerEMSegmentWidget Superclass;
  qSlicerEMSegmentDefinePreprocessingPanel(QWidget *newParent=0);

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentDefinePreprocessingPanel);
};

#endif

#ifndef __qSlicerEMSegmentDefineAtlasPanel_h
#define __qSlicerEMSegmentDefineAtlasPanel_h 

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentDefineAtlasPanelPrivate;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentDefineAtlasPanel :
    public qSlicerEMSegmentWidget
{ 
  Q_OBJECT

public:

  typedef qSlicerEMSegmentWidget Superclass;
  qSlicerEMSegmentDefineAtlasPanel(QWidget *newParent=0);

  void updateWidgetFromMRML();

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentDefineAtlasPanel);
};

#endif

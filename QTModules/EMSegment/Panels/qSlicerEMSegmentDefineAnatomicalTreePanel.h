#ifndef __qSlicerEMSegmentDefineAnatomicalTreePanel_h
#define __qSlicerEMSegmentDefineAnatomicalTreePanel_h 

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentDefineAnatomicalTreePanelPrivate;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentDefineAnatomicalTreePanel : public qSlicerWidget
{ 
  Q_OBJECT

public:

  typedef qSlicerWidget Superclass;
  qSlicerEMSegmentDefineAnatomicalTreePanel(QWidget *parent=0);
  
  virtual void printAdditionalInfo();

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentDefineAnatomicalTreePanel);
};

#endif

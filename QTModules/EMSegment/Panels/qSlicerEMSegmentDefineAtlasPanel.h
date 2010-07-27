#ifndef __qSlicerEMSegmentDefineAtlasPanel_h
#define __qSlicerEMSegmentDefineAtlasPanel_h 

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentDefineAtlasPanelPrivate;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentDefineAtlasPanel : public qSlicerWidget
{ 
  Q_OBJECT

public:

  typedef qSlicerWidget Superclass;
  qSlicerEMSegmentDefineAtlasPanel(QWidget *parent=0);
  
  virtual void printAdditionalInfo();

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentDefineAtlasPanel);
};

#endif

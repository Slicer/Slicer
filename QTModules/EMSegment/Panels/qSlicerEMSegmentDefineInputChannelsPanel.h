#ifndef __qSlicerEMSegmentDefineInputChannelsPanel_h
#define __qSlicerEMSegmentDefineInputChannelsPanel_h 

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentDefineInputChannelsPanelPrivate;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentDefineInputChannelsPanel : public qSlicerWidget
{ 
  Q_OBJECT

public:

  typedef qSlicerWidget Superclass;
  qSlicerEMSegmentDefineInputChannelsPanel(QWidget *parent=0);
  
  virtual void printAdditionalInfo();

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentDefineInputChannelsPanel);
};

#endif

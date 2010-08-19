#ifndef __qSlicerEMSegmentInputChannelListWidget_h
#define __qSlicerEMSegmentInputChannelListWidget_h

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentInputChannelListWidgetPrivate;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentInputChannelListWidget :
    public qSlicerEMSegmentWidget
{
  Q_OBJECT

public:

  typedef qSlicerEMSegmentWidget Superclass;
  explicit qSlicerEMSegmentInputChannelListWidget(QWidget *newParent = 0);

public slots:

  void addInputChannel();

  void removeInputChannel();

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentInputChannelListWidget);
};

#endif // __qSlicerEMSegmentInputChannelListWidget_h

#ifndef __qSlicerEMSegmentGraphWidget_h
#define __qSlicerEMSegmentGraphWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerEMSegmentWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentGraphWidgetPrivate;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentGraphWidget : public qSlicerEMSegmentWidget
{
  Q_OBJECT

public:
  typedef qSlicerEMSegmentWidget Superclass;
  qSlicerEMSegmentGraphWidget(QWidget *parent=0);

public slots:
  virtual void setMRMLManager(vtkEMSegmentMRMLManager* manager);
  void updateFromMRMLManager();

protected slots:
  void onCurrentInput0VolumeChanged(const QString& volumeName);
  void onCurrentInput1VolumeChanged(const QString& volumeName);
  void onClassVisibilityToggled(bool);

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentGraphWidget);
};

#endif

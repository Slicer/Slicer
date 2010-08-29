#ifndef __qSlicerEMSegmentWidget_h
#define __qSlicerEMSegmentWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// SlicerQt includes
#include "qSlicerWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentWidgetPrivate;
class vtkEMSegmentMRMLManager;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentWidget : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerWidget Superclass;
  explicit qSlicerEMSegmentWidget(QWidget *newParent = 0);

  vtkEMSegmentMRMLManager* mrmlManager() const;

public slots:
  virtual void setMRMLManager(vtkEMSegmentMRMLManager * newMRMLManager);

signals:
  void mrmlManagerChanged(vtkEMSegmentMRMLManager* newMRMLManager);

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentWidget);

};

#endif

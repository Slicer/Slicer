#ifndef __qSlicerEMSegmentWidget_h
#define __qSlicerEMSegmentWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerWidget.h"

class qSlicerEMSegmentWidgetPrivate;
class vtkEMSegmentMRMLManager;

class qSlicerEMSegmentWidget : public qSlicerWidget
{
  Q_OBJECT
public:
  typedef qSlicerWidget Superclass;
  explicit qSlicerEMSegmentWidget(QWidget *newParent = 0);

  vtkEMSegmentMRMLManager* mrmlManager() const;

public slots:
  void setMRMLManager(vtkEMSegmentMRMLManager * newMRMLManager);

signals:
  void mrmlManagerChanged(vtkEMSegmentMRMLManager* newMRMLManager);

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentWidget);

};

#endif

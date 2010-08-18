// EMSegment includes
#include "qSlicerEMSegmentWidget.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>

//-----------------------------------------------------------------------------
class qSlicerEMSegmentWidgetPrivate : public ctkPrivate<qSlicerEMSegmentWidget>
{
public:
  qSlicerEMSegmentWidgetPrivate();

  vtkEMSegmentMRMLManager *  MRMLManager;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentWidgetPrivate::qSlicerEMSegmentWidgetPrivate()
{
  this->MRMLManager = 0;
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentWidget methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentWidget::qSlicerEMSegmentWidget(QWidget *newParent) :
    Superclass(newParent)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentWidget);
}

//-----------------------------------------------------------------------------
CTK_GET_CXX(qSlicerEMSegmentWidget, vtkEMSegmentMRMLManager*, mrmlManager, MRMLManager);

//-----------------------------------------------------------------------------
void qSlicerEMSegmentWidget::setMRMLManager(vtkEMSegmentMRMLManager* newMRMLManager)
{
  CTK_D(qSlicerEMSegmentWidget);
  if (d->MRMLManager == newMRMLManager)
    {
    return;
    }
  d->MRMLManager = newMRMLManager;
  emit mrmlManagerChanged(newMRMLManager);
}

// EMSegment includes
#include "qSlicerEMSegmentWorkflowWidget.h"

// CTK
#include "ctkWorkflowWidget.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentWorkflowWidgetPrivate : public ctkPrivate<qSlicerEMSegmentWorkflowWidget>
{
public:
  qSlicerEMSegmentWorkflowWidgetPrivate();

  ctkWorkflowWidget* workflowWidget;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentWorkflowWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentWorkflowWidgetPrivate::qSlicerEMSegmentWorkflowWidgetPrivate()
{
  this->workflowWidget = 0;
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentWorkflowWidget methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentWorkflowWidget::qSlicerEMSegmentWorkflowWidget(QWidget *newParent)
  : Superclass(newParent)
  , d_ptr(new qSlicerEMSegmentWorkflowWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerEMSegmentWorkflowWidget, ctkWorkflowWidget*, workflowWidget, workflowWidget);
CTK_SET_CPP(qSlicerEMSegmentWorkflowWidget, ctkWorkflowWidget*, setWorkflowWidget, workflowWidget);

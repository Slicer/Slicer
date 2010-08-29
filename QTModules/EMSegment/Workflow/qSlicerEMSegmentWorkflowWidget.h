#ifndef __qSlicerEMSegmentWorkflowWidget_h
#define __qSlicerEMSegmentWorkflowWidget_h

// CTK includes
#include <ctkPimpl.h>
class ctkWorkflowWidget;

// EMSegment includes
#include "qMRMLWidget.h"

class qSlicerEMSegmentWorkflowWidgetPrivate;

class qSlicerEMSegmentWorkflowWidget : public qMRMLWidget
{
  Q_OBJECT
public:
  typedef qMRMLWidget Superclass;
  explicit qSlicerEMSegmentWorkflowWidget(QWidget *newParent = 0);

  virtual ctkWorkflowWidget* workflowWidget()const;
  virtual void setWorkflowWidget(ctkWorkflowWidget* workflowWidget);

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentWorkflowWidget);

};

#endif

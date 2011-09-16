#ifndef __qSlicerEMSegmentWorkflowWidget_h
#define __qSlicerEMSegmentWorkflowWidget_h

// CTK includes
#include <ctkPimpl.h>
class ctkWorkflowWidget;

// EMSegment includes
#include "qMRMLWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentWorkflowWidgetPrivate;

class qSlicerEMSegmentWorkflowWidget : public qMRMLWidget
{
  Q_OBJECT
public:
  typedef qMRMLWidget Superclass;
  explicit qSlicerEMSegmentWorkflowWidget(QWidget *newParent = 0);
  virtual ~qSlicerEMSegmentWorkflowWidget();

  virtual ctkWorkflowWidget* workflowWidget()const;
  virtual void setWorkflowWidget(ctkWorkflowWidget* workflowWidget);
protected:
  QScopedPointer<qSlicerEMSegmentWorkflowWidgetPrivate> d_ptr;
private:
  Q_DECLARE_PRIVATE(qSlicerEMSegmentWorkflowWidget);
  Q_DISABLE_COPY(qSlicerEMSegmentWorkflowWidget);

};

#endif

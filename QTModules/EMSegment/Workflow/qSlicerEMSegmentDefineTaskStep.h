#ifndef __qSlicerEMSegmentDefineTaskStep_h
#define __qSlicerEMSegmentDefineTaskStep_h

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWorkflowWidgetStep.h"

class qSlicerEMSegmentDefineTaskStepPrivate;

class qSlicerEMSegmentDefineTaskStep : public qSlicerEMSegmentWorkflowWidgetStep
{
  Q_OBJECT

public:

  const static QString StepId;

  typedef qSlicerEMSegmentWorkflowWidgetStep Superclass;
  explicit qSlicerEMSegmentDefineTaskStep(ctkWorkflow* newWorkflow);

public slots:

  virtual void populateStepWidgetsList(QList<QWidget*>& stepWidgetsList);

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentDefineTaskStep);

};

#endif

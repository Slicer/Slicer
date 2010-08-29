#ifndef __qSlicerEMSegmentSegmentationLogicStep_h
#define __qSlicerEMSegmentSegmentationLogicStep_h

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWorkflowWidgetStep.h"

class qSlicerEMSegmentSegmentationLogicStepPrivate;

class qSlicerEMSegmentSegmentationLogicStep : public qSlicerEMSegmentWorkflowWidgetStep
{
  Q_OBJECT

public:

  static const QString StepId;

  typedef qSlicerEMSegmentWorkflowWidgetStep Superclass;
  explicit qSlicerEMSegmentSegmentationLogicStep(ctkWorkflow* newWorkflow);

public slots:

  virtual void validate(const QString& desiredBranchId = QString());

  virtual void populateStepWidgetsList(QList<QWidget*>& stepWidgetsList);

  virtual void onEntry(const ctkWorkflowStep* comingFrom,
                       const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType);

  virtual void onExit(const ctkWorkflowStep* goingTo,
                      const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType);

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentSegmentationLogicStep);

};

#endif

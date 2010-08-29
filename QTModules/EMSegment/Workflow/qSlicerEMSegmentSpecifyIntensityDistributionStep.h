#ifndef __qSlicerEMSegmentSpecifyIntensityDistributionStep_h
#define __qSlicerEMSegmentSpecifyIntensityDistributionStep_h

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWorkflowWidgetStep.h"

class qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate;

class qSlicerEMSegmentSpecifyIntensityDistributionStep : public qSlicerEMSegmentWorkflowWidgetStep
{
  Q_OBJECT

public:

  static const QString StepId;

  typedef qSlicerEMSegmentWorkflowWidgetStep Superclass;
  explicit qSlicerEMSegmentSpecifyIntensityDistributionStep(ctkWorkflow* newWorkflow);

public slots:

  virtual void validate(const QString& desiredBranchId = QString());

  virtual void populateStepWidgetsList(QList<QWidget*>& stepWidgetsList);

  virtual void onEntry(const ctkWorkflowStep* comingFrom,
                       const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType);

  virtual void onExit(const ctkWorkflowStep* goingTo,
                      const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType);

  virtual void showUserInterface();

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentSpecifyIntensityDistributionStep);

};

#endif

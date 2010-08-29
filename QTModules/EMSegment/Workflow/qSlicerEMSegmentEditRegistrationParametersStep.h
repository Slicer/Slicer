#ifndef __qSlicerEMSegmentEditRegistrationParametersStep_h
#define __qSlicerEMSegmentEditRegistrationParametersStep_h

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWorkflowWidgetStep.h"

class qSlicerEMSegmentEditRegistrationParametersStepPrivate;

class qSlicerEMSegmentEditRegistrationParametersStep : public qSlicerEMSegmentWorkflowWidgetStep
{
  Q_OBJECT

public:

  static const QString StepId;

  typedef qSlicerEMSegmentWorkflowWidgetStep Superclass;
  explicit qSlicerEMSegmentEditRegistrationParametersStep(ctkWorkflow* newWorkflow);

public slots:

  virtual void validate(const QString& desiredBranchId = QString());

  virtual void populateStepWidgetsList(QList<QWidget*>& stepWidgetsList);

  virtual void showUserInterface();

  virtual void onEntry(const ctkWorkflowStep* comingFrom,
                       const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType);

  virtual void onExit(const ctkWorkflowStep* goingTo,
                      const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType);

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentEditRegistrationParametersStep);

};

#endif

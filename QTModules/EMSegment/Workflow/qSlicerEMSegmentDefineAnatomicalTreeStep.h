#ifndef __qSlicerEMSegmentDefineAnatomicalTreeStep_h
#define __qSlicerEMSegmentDefineAnatomicalTreeStep_h

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWorkflowWidgetStep.h"

class qSlicerEMSegmentDefineAnatomicalTreeStepPrivate;

class qSlicerEMSegmentDefineAnatomicalTreeStep : public qSlicerEMSegmentWorkflowWidgetStep
{
  Q_OBJECT

public:

  const static QString StepId;

  typedef qSlicerEMSegmentWorkflowWidgetStep Superclass;
  explicit qSlicerEMSegmentDefineAnatomicalTreeStep(ctkWorkflow* newWorkflow);

public slots:

  virtual void validate(const QString& desiredBranchId = QString());

  virtual void populateStepWidgetsList(QList<QWidget*>& stepWidgetsList);

  virtual void onEntry(const ctkWorkflowStep* comingFrom,
                       const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType);

  virtual void onExit(const ctkWorkflowStep* goingTo,
                      const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType);

  virtual void showUserInterface();

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentDefineAnatomicalTreeStep);

};

#endif

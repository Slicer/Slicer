#ifndef __qSlicerEMSegmentDefineInputChannelsStep_h
#define __qSlicerEMSegmentDefineInputChannelsStep_h

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWorkflowWidgetStep.h"

class qSlicerEMSegmentDefineInputChannelsStepPrivate;

class qSlicerEMSegmentDefineInputChannelsStep : public qSlicerEMSegmentWorkflowWidgetStep
{
  Q_OBJECT
public:

  enum StepModeType
    {
    Simple = 0,
    Advanced
    };

  const static QString SimpleStepId;
  const static QString AdvancedStepId;

  typedef qSlicerEMSegmentWorkflowWidgetStep Superclass;
  explicit qSlicerEMSegmentDefineInputChannelsStep(StepModeType stepMode, ctkWorkflow* newWorkflow);

public slots:

  virtual void validate(const QString& desiredBranchId = QString());

  virtual void populateStepWidgetsList(QList<QWidget*>& stepWidgetsList);

  virtual void onEntry(const ctkWorkflowStep* comingFrom,
                       const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType);

  virtual void onExit(const ctkWorkflowStep* goingTo,
                      const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType);

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentDefineInputChannelsStep);

};

#endif

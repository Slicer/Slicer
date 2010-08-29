
// EMSegment includes
#include "qSlicerEMSegmentEditRegistrationParametersStep.h"
#include "qSlicerEMSegmentEditRegistrationParametersPanel.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentEditRegistrationParametersStepPrivate : public ctkPrivate<qSlicerEMSegmentEditRegistrationParametersStep>
{
public:
  qSlicerEMSegmentEditRegistrationParametersStepPrivate();
  qSlicerEMSegmentEditRegistrationParametersPanel* Panel;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentEditRegistrationParametersStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentEditRegistrationParametersStepPrivate::qSlicerEMSegmentEditRegistrationParametersStepPrivate()
{
  this->Panel = 0;
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentEditRegistrationParametersStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentEditRegistrationParametersStep::StepId = "EditRegistrationParameters";

//-----------------------------------------------------------------------------
qSlicerEMSegmentEditRegistrationParametersStep::qSlicerEMSegmentEditRegistrationParametersStep(
    ctkWorkflow* newWorkflow) : Superclass(newWorkflow, Self::StepId)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentEditRegistrationParametersStep);
  this->setName("5/9. Edit Registration Parameters");
  this->setDescription("Specify atlas-to-input scans registration parameters.");
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersStep::populateStepWidgetsList(QList<QWidget*>& stepWidgetsList)
{
  CTK_D(qSlicerEMSegmentEditRegistrationParametersStep);
  if (!d->Panel)
    {
    d->Panel = new qSlicerEMSegmentEditRegistrationParametersPanel;
    connect(this, SIGNAL(mrmlManagerChanged(vtkEMSegmentMRMLManager*)),
            d->Panel, SLOT(setMRMLManager(vtkEMSegmentMRMLManager*)));
    d->Panel->setMRMLManager(this->mrmlManager());
    }
  stepWidgetsList << d->Panel;
  emit populateStepWidgetsListComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersStep::showUserInterface()
{
  CTK_D(qSlicerEMSegmentEditRegistrationParametersStep);
  this->Superclass::showUserInterface();
  d->Panel->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  emit validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  // Signals that we are finished
  emit onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditRegistrationParametersStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  Q_UNUSED(goingTo);
  Q_UNUSED(transitionType);

  CTK_D(qSlicerEMSegmentEditRegistrationParametersStep);

  Q_ASSERT(this->mrmlManager());

  if (transitionType != ctkWorkflowInterstepTransition::TransitionToNextStep)
    {
    // Signals that we are finished
    emit onExitComplete();
    return;
    }

  d->Panel->updateMRMLFromWidget();

  // Signals that we are finished
  emit onExitComplete();
}

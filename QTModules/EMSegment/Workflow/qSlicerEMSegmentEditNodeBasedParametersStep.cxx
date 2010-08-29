
// EMSegment includes
#include "qSlicerEMSegmentEditNodeBasedParametersStep.h"
#include "qSlicerEMSegmentEditNodeBasedParametersPanel.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentEditNodeBasedParametersStepPrivate : public ctkPrivate<qSlicerEMSegmentEditNodeBasedParametersStep>
{
public:
  qSlicerEMSegmentEditNodeBasedParametersStepPrivate();
  qSlicerEMSegmentEditNodeBasedParametersPanel* panel;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentEditNodeBasedParametersStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentEditNodeBasedParametersStepPrivate::qSlicerEMSegmentEditNodeBasedParametersStepPrivate()
{
  this->panel = 0;
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentEditNodeBasedParametersStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentEditNodeBasedParametersStep::StepId = "EditNodeBasedParameters";

//-----------------------------------------------------------------------------
qSlicerEMSegmentEditNodeBasedParametersStep::qSlicerEMSegmentEditNodeBasedParametersStep(
    ctkWorkflow* newWorkflow) : Superclass(newWorkflow, Self::StepId)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentEditNodeBasedParametersStep);
  this->setName("8/9. Edit Node-based Parameters");
  this->setDescription("Specify node-based segmentation parameters.");
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditNodeBasedParametersStep::populateStepWidgetsList(QList<QWidget*>& stepWidgetsList)
{
  CTK_D(qSlicerEMSegmentEditNodeBasedParametersStep);
  if (!d->panel)
    {
    d->panel = new qSlicerEMSegmentEditNodeBasedParametersPanel;
    connect(this, SIGNAL(mrmlManagerChanged(vtkEMSegmentMRMLManager*)),
            d->panel, SLOT(setMRMLManager(vtkEMSegmentMRMLManager*)));
    d->panel->setMRMLManager(this->mrmlManager());
    }
  stepWidgetsList << d->panel;
  emit populateStepWidgetsListComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditNodeBasedParametersStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  emit validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditNodeBasedParametersStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{

  // Signals that we are finished
  emit onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditNodeBasedParametersStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  // Signals that we are finished
  emit onExitComplete();
}


//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditNodeBasedParametersStep::showUserInterface()
{
  CTK_D(qSlicerEMSegmentEditNodeBasedParametersStep);
  this->Superclass::showUserInterface();
  d->panel->updateWidgetFromMRML();
}

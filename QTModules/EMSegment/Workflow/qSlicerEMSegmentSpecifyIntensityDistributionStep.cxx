
// EMSegment includes
#include "qSlicerEMSegmentSpecifyIntensityDistributionStep.h"
#include "qSlicerEMSegmentSpecifyIntensityDistributionPanel.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate : public ctkPrivate<qSlicerEMSegmentSpecifyIntensityDistributionStep>
{
public:
  qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate();
  qSlicerEMSegmentSpecifyIntensityDistributionPanel* panel;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate::qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate()
{
  this->panel = 0;
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentSpecifyIntensityDistributionStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentSpecifyIntensityDistributionStep::StepId =
    "SpecifyIntensityDistribution";

//-----------------------------------------------------------------------------
qSlicerEMSegmentSpecifyIntensityDistributionStep::qSlicerEMSegmentSpecifyIntensityDistributionStep(
    ctkWorkflow* newWorkflow) : Superclass(newWorkflow, Self::StepId)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  this->setName("7/9. Specify Intensity Distributions");
  this->setDescription("Define intensity distribution for each anatomical structure.");
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::populateStepWidgetsList(QList<QWidget*>& stepWidgetsList)
{
  CTK_D(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  if (!d->panel)
    {
    d->panel = new qSlicerEMSegmentSpecifyIntensityDistributionPanel;
    connect(this, SIGNAL(mrmlManagerChanged(vtkEMSegmentMRMLManager*)),
            d->panel, SLOT(setMRMLManager(vtkEMSegmentMRMLManager*)));
    d->panel->setMRMLManager(this->mrmlManager());
    }
  stepWidgetsList << d->panel;
  emit populateStepWidgetsListComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  emit validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  // Signals that we are finished
  emit onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  // Signals that we are finished
  emit onExitComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::showUserInterface()
{
  CTK_D(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  this->Superclass::showUserInterface();
  d->panel->updateWidgetFromMRML();
}

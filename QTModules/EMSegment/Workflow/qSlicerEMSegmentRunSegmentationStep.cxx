
// EMSegment includes
#include "qSlicerEMSegmentRunSegmentationStep.h"
#include "qSlicerEMSegmentRunSegmentationPanel.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentRunSegmentationStepPrivate : public ctkPrivate<qSlicerEMSegmentRunSegmentationStep>
{
public:
  qSlicerEMSegmentRunSegmentationStepPrivate();
  qSlicerEMSegmentRunSegmentationPanel* panel;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentRunSegmentationStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentRunSegmentationStepPrivate::qSlicerEMSegmentRunSegmentationStepPrivate()
{
  this->panel = 0;
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentRunSegmentationStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentRunSegmentationStep::StepId = "Segment";

//-----------------------------------------------------------------------------
qSlicerEMSegmentRunSegmentationStep::qSlicerEMSegmentRunSegmentationStep(
    ctkWorkflow* newWorkflow) : Superclass(newWorkflow, Self::StepId)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentRunSegmentationStep);
  this->setName("9/9. Run Segmentation");
  this->setDescription("Apply EM algorithm to segment target image.");
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStep::populateStepWidgetsList(QList<QWidget*>& stepWidgetsList)
{
  CTK_D(qSlicerEMSegmentRunSegmentationStep);
  if (!d->panel)
    {
    d->panel = new qSlicerEMSegmentRunSegmentationPanel;
    connect(this, SIGNAL(mrmlManagerChanged(vtkEMSegmentMRMLManager*)),
            d->panel, SLOT(setMRMLManager(vtkEMSegmentMRMLManager*)));
    d->panel->setMRMLManager(this->mrmlManager());
    }
  stepWidgetsList << d->panel;
  emit populateStepWidgetsListComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  emit validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  // Signals that we are finished
  emit onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  // Signals that we are finished
  emit onExitComplete();
}

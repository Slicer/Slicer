
// EMSegment includes
#include "qSlicerEMSegmentDefinePreprocessingStep.h"
#include "qSlicerEMSegmentDefinePreprocessingPanel.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentDefinePreprocessingStepPrivate : public ctkPrivate<qSlicerEMSegmentDefinePreprocessingStep>
{
public:
  qSlicerEMSegmentDefinePreprocessingStepPrivate();
  qSlicerEMSegmentDefinePreprocessingPanel* panel;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefinePreprocessingStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefinePreprocessingStepPrivate::qSlicerEMSegmentDefinePreprocessingStepPrivate()
{
  this->panel = 0;
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefinePreprocessingStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentDefinePreprocessingStep::StepId = "DefinePreprocessing";

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefinePreprocessingStep::qSlicerEMSegmentDefinePreprocessingStep(
    ctkWorkflow* newWorkflow) : Superclass(newWorkflow, Self::StepId)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentDefinePreprocessingStep);
  this->setName("6/9. Define Preprocessing");
  this->setDescription("Answer questions for preprocessing of input images.");
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefinePreprocessingStep::populateStepWidgetsList(QList<QWidget*>& stepWidgetsList)
{
  CTK_D(qSlicerEMSegmentDefinePreprocessingStep);
  if (!d->panel)
    {
    d->panel = new qSlicerEMSegmentDefinePreprocessingPanel;
    connect(this, SIGNAL(mrmlManagerChanged(vtkEMSegmentMRMLManager*)),
            d->panel, SLOT(setMRMLManager(vtkEMSegmentMRMLManager*)));
    d->panel->setMRMLManager(this->mrmlManager());
    }
  stepWidgetsList << d->panel;
  emit populateStepWidgetsListComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefinePreprocessingStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  emit validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefinePreprocessingStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  // Don't forget to call the superclass's function
  this->Superclass::onEntry(comingFrom, transitionType);

  // Signals that we are finished
  emit onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefinePreprocessingStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  // Signals that we are finished
  emit onExitComplete();
}

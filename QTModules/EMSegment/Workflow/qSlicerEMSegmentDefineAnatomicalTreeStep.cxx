// EMSegment includes
#include "qSlicerEMSegmentDefineAnatomicalTreeStep.h"
#include "qSlicerEMSegmentDefineAnatomicalTreePanel.h"

// CTK includes
#include "ctkWorkflowWidgetStep.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentDefineAnatomicalTreeStepPrivate : public ctkPrivate<qSlicerEMSegmentDefineAnatomicalTreeStep>
{
public:
  qSlicerEMSegmentDefineAnatomicalTreeStepPrivate();
  qSlicerEMSegmentDefineAnatomicalTreePanel* panel;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefineAnatomicalTreeStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineAnatomicalTreeStepPrivate::qSlicerEMSegmentDefineAnatomicalTreeStepPrivate()
{
  this->panel = 0;
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefineAnatomicalTreeStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentDefineAnatomicalTreeStep::StepId = "DefineAnatomicalTree";

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineAnatomicalTreeStep::qSlicerEMSegmentDefineAnatomicalTreeStep(
    ctkWorkflow* newWorkflow) : Superclass(newWorkflow, Self::StepId)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentDefineAnatomicalTreeStep);
  this->setName("3/9. DefineAnatomicalTree");
  this->setDescription("Define a hierarchy of structures.");
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAnatomicalTreeStep::populateStepWidgetsList(QList<QWidget*>& stepWidgetsList)
{
  CTK_D(qSlicerEMSegmentDefineAnatomicalTreeStep);
  if (!d->panel)
    {
    d->panel = new qSlicerEMSegmentDefineAnatomicalTreePanel;
    connect(this, SIGNAL(mrmlManagerChanged(vtkEMSegmentMRMLManager*)),
            d->panel, SLOT(setMRMLManager(vtkEMSegmentMRMLManager*)));
    d->panel->setMRMLManager(this->mrmlManager());
    }
  stepWidgetsList << d->panel;
  emit populateStepWidgetsListComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAnatomicalTreeStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  emit validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAnatomicalTreeStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  CTK_D(qSlicerEMSegmentDefineAnatomicalTreeStep);

  // Signals that we are finished
  emit onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAnatomicalTreeStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{

  // Signals that we are finished
  emit onExitComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAnatomicalTreeStep::showUserInterface()
{
  CTK_D(qSlicerEMSegmentDefineAnatomicalTreeStep);
  this->Superclass::showUserInterface();
  d->panel->updateWidgetFromMRML();
}


// Qt includes
#include <QMessageBox>

// EMSegment includes
#include "qSlicerEMSegmentDefinePreprocessingStep.h"
#include "qSlicerEMSegmentDefinePreprocessingPanel.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>
#include <vtkMRMLEMSWorkingDataNode.h>

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

  // TODO original code goes into tcl script to place custom preprocessing widgets here

  stepWidgetsList << d->panel;
  emit populateStepWidgetsListComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefinePreprocessingStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  CTK_D(qSlicerEMSegmentDefinePreprocessingStep);
  Q_ASSERT(this->mrmlManager());

  vtkEMSegmentMRMLManager* mrmlManager = this->mrmlManager();

  // If they are still valid then don't repeat preprocessing unless the user wants to
  if (mrmlManager->GetWorkingDataNode()->GetAlignedTargetNodeIsValid()
      && mrmlManager->GetWorkingDataNode()->GetAlignedAtlasNodeIsValid())
    {
    // If the  user doesn't want to redo processing, then we're done
    if (QMessageBox::No == QMessageBox::question(d->panel, "EMSegmenter",
                                                 tr("Do you want to redo preprocessing of input images?"),
                                                 QMessageBox::Yes, QMessageBox::No))
      {
      emit validationComplete(true);
      return;
      }
    }
  // If they are not valid, then notify them that preprocessing may take a while
  else
    {
    // If the user doesn't want to do processing, then validation fails
    if (QMessageBox::No == QMessageBox::question(d->panel, "EMSegmenter",
                                                 tr("Start preprocessing of images?  Preprocessing of images might take a while, but segmentation cannot continue without preprocessing.  Do you want to proceed?"),
                                                 QMessageBox::Yes, QMessageBox::No))
      {
      emit validationComplete(false);
      return;
      }
    }

  // TODO: original code runs tcl script to do preprocessing
  // TODO: return validationComplete(false) if preprocessing fails

  // Set it to valid so next time we do not have to recompute it 
  mrmlManager->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(1);
  mrmlManager->GetWorkingDataNode()->SetAlignedAtlasNodeIsValid(1);

  cout << "=============================================" << endl;
  cout << "Pre-processing completed successfully" << endl;
  cout << "=============================================" << endl;

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

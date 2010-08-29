
// Qt includes
#include <QMessageBox>

// CTK includes
#include <ctkLogger.h>

// qMRMLWidgets includes
#include <qMRMLNodeFactory.h>

// EMSegment includes
#include "qSlicerEMSegmentDefineInputChannelsStep.h"
#include "qSlicerEMSegmentDefineInputChannelsPanel.h"
#include "qSlicerEMSegmentInputChannelListWidget.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>
#include <vtkMRMLEMSTargetNode.h>

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>

//--------------------------------------------------------------------------
static ctkLogger logger(
    "org.slicer.qtmodules.emsegment.workflow.qSlicerEMSegmentDefineInputChannelsStep");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class qSlicerEMSegmentDefineInputChannelsStepPrivate : public ctkPrivate<qSlicerEMSegmentDefineInputChannelsStep>
{
public:
  qSlicerEMSegmentDefineInputChannelsStepPrivate();

  qSlicerEMSegmentDefineInputChannelsPanel* Panel;
  qSlicerEMSegmentDefineInputChannelsStep::StepModeType StepMode;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefineInputChannelsStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineInputChannelsStepPrivate::qSlicerEMSegmentDefineInputChannelsStepPrivate()
{
  this->Panel = 0;
  this->StepMode = qSlicerEMSegmentDefineInputChannelsStep::Advanced;
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefineInputChannelsStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentDefineInputChannelsStep::SimpleStepId = "DefineInputChannelsSimple";
const QString qSlicerEMSegmentDefineInputChannelsStep::AdvancedStepId =
    "DefineInputChannelsAdvanced";

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineInputChannelsStep::qSlicerEMSegmentDefineInputChannelsStep(
    qSlicerEMSegmentDefineInputChannelsStep::StepModeType stepMode, ctkWorkflow* newWorkflow)
      : Superclass(newWorkflow)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentDefineInputChannelsStep);
  CTK_D(qSlicerEMSegmentDefineInputChannelsStep);

  if (stepMode == qSlicerEMSegmentDefineInputChannelsStep::Advanced)
    {
    this->setId(Self::AdvancedStepId);
    this->setName("2/9. Define Input Channels");
    this->setDescription("Name the input channels and choose the set of scans for segmentation.");
    }
  else
    {
    this->setId(Self::SimpleStepId);
    this->setName("2/2. Define Input Channels");
    this->setDescription("Choose the set of scans for segmentation.");
    this->setButtonBoxHints(ctkWorkflowWidgetStep::NextButtonDisabled);
    }

  d->StepMode = stepMode;
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineInputChannelsStep::populateStepWidgetsList(QList<QWidget*>& stepWidgetsList)
{
  CTK_D(qSlicerEMSegmentDefineInputChannelsStep);
  if (!d->Panel)
    {
    d->Panel = new qSlicerEMSegmentDefineInputChannelsPanel;
    connect(this, SIGNAL(mrmlManagerChanged(vtkEMSegmentMRMLManager*)),
            d->Panel, SLOT(setMRMLManager(vtkEMSegmentMRMLManager*)));
    d->Panel->setMRMLManager(this->mrmlManager());
    }
  stepWidgetsList << d->Panel;
  emit populateStepWidgetsListComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineInputChannelsStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  CTK_D(qSlicerEMSegmentDefineInputChannelsStep);

  Q_ASSERT(this->mrmlManager());
  Q_ASSERT(this->mrmlManager()->GetTargetInputNode());

  qSlicerEMSegmentInputChannelListWidget * inputChannelWidget =
      d->Panel->inputChannelListWidget();

  int requiredInputChannelCount = 1;

  // Check required number of input
  if (inputChannelWidget->inputChannelCount() < requiredInputChannelCount)
    {
    QMessageBox::warning(d->Panel, "EMSegmenter",
                         tr("At least %1 image is required !").arg(requiredInputChannelCount));
    emit validationComplete(false);
    return;
    }

  int inputChannelCount = inputChannelWidget->inputChannelCount();

  // Check if number of input channels changed ...
  if (inputChannelCount != this->mrmlManager()->GetTargetInputNode()->GetNumberOfVolumes())
    {
    if (QMessageBox::No == QMessageBox::question(
        d->Panel, "EMSegmenter", tr("Are you sure you want to change the number of input images?"),
        QMessageBox::Yes, QMessageBox::No))
      {
      emit validationComplete(false);
      return;
      }
    }

  QStringList errors;

  // Loop though all volumes and check if ImageData is Null
  for(int rowId = 0; rowId < inputChannelCount; rowId++)
    {
    QString inputChannelName = inputChannelWidget->inputChannelName(rowId);
    vtkMRMLVolumeNode* volumeNode = inputChannelWidget->inputChannelVolume(rowId);

    // Is ImageData Null ?
    if (volumeNode->GetImageData() == 0)
      {
      errors << tr("Volume of %1th Input channel (%2) is empty !").arg(rowId).arg(inputChannelName);
      }
    }

  if (errors.count() > 0)
    {
    QMessageBox::warning(d->Panel, "EMSegmenter", errors.join("<br/>"));
    emit validationComplete(false);
    return;
    }

  // Loop though all volumes and check if ImageData is Negative
  for(int rowId = 0; rowId < inputChannelCount; rowId++)
    {
    QString inputChannelName = inputChannelWidget->inputChannelName(rowId);
    vtkMRMLVolumeNode* volumeNode = inputChannelWidget->inputChannelVolume(rowId);

    if (volumeNode->GetImageData()->GetScalarRange()[0] < 0)
      {
      errors << tr("Volume of %1th Input channel (%2)").arg(rowId).arg(inputChannelName);
      }
    }

  if (errors.count() > 0)
    {
    QString message = "EMSegmenter can currently only process non-negative input images. "
                    "The following images have negative values:<br/>";

    QMessageBox::warning(d->Panel, "EMSegmenter", message.append(errors.join("<br/>")));
    emit validationComplete(false);
    return;
    }

  // Check for identical selected input volumes
  if (inputChannelWidget->identicalInputVolumes())
    {
    QMessageBox::warning(d->Panel, "EMSegmenter",
                         tr("No two Input volumes can be the same"));
    emit validationComplete(false);
    return;
    }

  // Check
  emit validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineInputChannelsStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  // Signals that we are finished
  emit onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineInputChannelsStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  Q_UNUSED(goingTo);
  Q_UNUSED(transitionType);

  CTK_D(qSlicerEMSegmentDefineInputChannelsStep);

  Q_ASSERT(this->mrmlManager());

  if (transitionType != ctkWorkflowInterstepTransition::TransitionToNextStep)
    {
    // Signals that we are finished
    emit onExitComplete();
    return;
    }

  // Update MRML with selected volumes
  d->Panel->updateMRMLFromWidget();

  // Make sure output volume is defined
  if(!this->mrmlManager()->GetOutputVolumeMRMLID())
    {
    vtkMRMLScalarVolumeNode* outputVolumeNode =
        vtkMRMLScalarVolumeNode::SafeDownCast(
            qMRMLNodeFactory::createNode(this->mrmlScene(), "vtkMRMLScalarVolumeNode"));
    outputVolumeNode->SetName("EMSegment1"); // TODO Should name be changed to something meaningful ?
    outputVolumeNode->LabelMapOn();
    this->mrmlManager()->SetOutputVolumeMRMLID(outputVolumeNode->GetID());
    }

  // Signals that we are finished
  emit onExitComplete();
}

/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by
    Danielle Pace and Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QMessageBox>

// CTK includes
#include <ctkLogger.h>

// qMRMLWidgets includes
#include <qMRMLNodeFactory.h>

// EMSegment includes
#include "qSlicerEMSegmentDefineInputChannelsStep.h"
#include "ui_qSlicerEMSegmentDefineInputChannelsStep.h"
#include "qSlicerEMSegmentInputChannelListWidget.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>
#include <vtkMRMLEMSTargetNode.h>

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>

#include <vtkMRMLEMSWorkingDataNode.h>

//--------------------------------------------------------------------------
static ctkLogger logger(
                        "org.slicer.qtmodules.emsegment.workflow.qSlicerEMSegmentDefineInputChannelsStep");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_EMSegment
class qSlicerEMSegmentDefineInputChannelsStepPrivate : public Ui_qSlicerEMSegmentDefineInputChannelsStep
{
public:
  qSlicerEMSegmentDefineInputChannelsStepPrivate();

  void updateMRMLFromWidget();

  qSlicerEMSegmentDefineInputChannelsStep::StepModeType StepMode;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefineInputChannelsStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineInputChannelsStepPrivate::qSlicerEMSegmentDefineInputChannelsStepPrivate()
{
  this->StepMode = qSlicerEMSegmentDefineInputChannelsStep::Advanced;
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineInputChannelsStepPrivate::updateMRMLFromWidget()
{
  this->EMSegmentInputChannelListWidget->updateMRMLFromWidget();
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefineInputChannelsStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentDefineInputChannelsStep::SimpleStepId = "DefineInputChannelsSimple";
const QString qSlicerEMSegmentDefineInputChannelsStep::AdvancedStepId =
    "DefineInputChannelsAdvanced";

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineInputChannelsStep::qSlicerEMSegmentDefineInputChannelsStep(ctkWorkflow* newWorkflow, qSlicerEMSegmentDefineInputChannelsStep::StepModeType stepMode, QWidget* newWidget)
  : Superclass(newWorkflow, "", newWidget)
  , d_ptr(new qSlicerEMSegmentDefineInputChannelsStepPrivate)
{
  Q_D(qSlicerEMSegmentDefineInputChannelsStep);
  d->setupUi(this);

  if (stepMode == qSlicerEMSegmentDefineInputChannelsStep::Advanced)
    {
    this->setId(qSlicerEMSegmentDefineInputChannelsStep::AdvancedStepId);
    this->setName("2/9. Define Input Channels");
    this->setDescription("Name the input channels and choose the set of scans for segmentation.");
    }
  else
    {
    this->setId(qSlicerEMSegmentDefineInputChannelsStep::SimpleStepId);
    this->setName("2/2. Define Input Channels");
    this->setDescription("Choose the set of scans for segmentation.");
    this->setButtonBoxHints(ctkWorkflowWidgetStep::NextButtonDisabled);
    }

  d->StepMode = stepMode;
}

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineInputChannelsStep::~qSlicerEMSegmentDefineInputChannelsStep()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineInputChannelsStep::createUserInterface()
{
  this->createUserInterfaceComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineInputChannelsStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  Q_D(qSlicerEMSegmentDefineInputChannelsStep);

  Q_ASSERT(this->mrmlManager());
  //Q_ASSERT(this->mrmlManager()->GetTargetInputNode());

  int requiredInputChannelCount = 1;

  // Check required number of input
  if (d->EMSegmentInputChannelListWidget->inputChannelCount() < requiredInputChannelCount)
    {
    QMessageBox::warning(this, "EMSegmenter", tr("At least %1 image is required !").arg(requiredInputChannelCount));
    this->validationComplete(false);
    return;
    }

  int inputChannelCount = d->EMSegmentInputChannelListWidget->inputChannelCount();

  // Check if number of input channels changed ...
  if (inputChannelCount != this->mrmlManager()->GetTargetNumberOfSelectedVolumes())
    {
    if (QMessageBox::No == QMessageBox::question(this, "EMSegmenter", tr("Are you sure you want to change the number of input images?"),
        QMessageBox::Yes, QMessageBox::No))
      {
      this->validationComplete(false);
      return;
      }
    }

  QStringList errors;

  // Loop though all volumes and check if ImageData is Null
  for(int rowId = 0; rowId < inputChannelCount; rowId++)
    {
    QString inputChannelName = d->EMSegmentInputChannelListWidget->inputChannelName(rowId);
    vtkMRMLVolumeNode* volumeNode = d->EMSegmentInputChannelListWidget->inputChannelVolume(rowId);

    // Is ImageData Null ?
    if (volumeNode->GetImageData() == 0)
      {
      errors << tr("Volume of %1th Input channel (%2) is empty !").arg(rowId + 1).arg(inputChannelName);
      }
    }

  if (errors.count() > 0)
    {
    QMessageBox::warning(this, "EMSegmenter", errors.join("\n"));
    this->validationComplete(false);
    return;
    }

  // Loop though all volumes and check if ImageData is Negative
  for(int rowId = 0; rowId < inputChannelCount; rowId++)
    {
    QString inputChannelName = d->EMSegmentInputChannelListWidget->inputChannelName(rowId);
    vtkMRMLVolumeNode* volumeNode = d->EMSegmentInputChannelListWidget->inputChannelVolume(rowId);

    if (volumeNode->GetImageData()->GetScalarRange()[0] < 0)
      {
      errors << tr("Volume of %1th Input channel (%2)").arg(rowId).arg(inputChannelName);
      }
    }

  if (errors.count() > 0)
    {
    QString message = "EMSegmenter can currently only process non-negative input images. "
                    "The following images have negative values:<br/>";

    QMessageBox::warning(this, "EMSegmenter", message.append(errors.join("<br/>")));
    this->validationComplete(false);
    return;
    }

  // Check for identical selected input volumes
  if (d->EMSegmentInputChannelListWidget->identicalInputVolumes())
    {
    QMessageBox::warning(this, "EMSegmenter",
                         tr("No two Input volumes can be the same"));
    this->validationComplete(false);
    return;
    }

  // Update whether the AlignedTargetNode and AlignedAtlasNode is valid in the MRML manager,
  // according to whether we are in simple or advanced modes.
  if (this->id() == qSlicerEMSegmentDefineInputChannelsStep::SimpleStepId)
    {
    // TODO Original code calls tcl script here for customized EM Segment workflows
    // TODO Original code calls:
    //this->UpdateTaskPreprocessingSetting();
    this->mrmlManager()->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(0);
    this->mrmlManager()->GetWorkingDataNode()->SetAlignedAtlasNodeIsValid(0);
    }

  // Check
  this->validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineInputChannelsStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  Q_UNUSED(comingFrom);
  Q_UNUSED(transitionType);
  // Indicates that we are finished
  this->onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineInputChannelsStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  Q_UNUSED(goingTo);

  Q_D(qSlicerEMSegmentDefineInputChannelsStep);

  Q_ASSERT(this->mrmlManager());

  if (transitionType != ctkWorkflowInterstepTransition::TransitionToNextStep)
    {
    // Signals that we are finished
    this->onExitComplete();
    return;
    }

  // Update MRML with selected volumes
  d->updateMRMLFromWidget();

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
  this->onExitComplete();
}

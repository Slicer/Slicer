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

  This file was originally developed by Danielle Pace, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QMessageBox>
#include <QStyle>

// EMSegment includes
#include "qSlicerEMSegmentSegmentationLogicStep.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>

// EMSegment/Logic includes
#include <vtkSlicerEMSegmentLogic.h>

//-----------------------------------------------------------------------------
class qSlicerEMSegmentSegmentationLogicStepPrivate : public ctkPrivate<qSlicerEMSegmentSegmentationLogicStep>
{
public:
  qSlicerEMSegmentSegmentationLogicStepPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentSegmentationLogicStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentSegmentationLogicStepPrivate::qSlicerEMSegmentSegmentationLogicStepPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentSegmentationLogicStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentSegmentationLogicStep::StepId = "Segment";

//-----------------------------------------------------------------------------
qSlicerEMSegmentSegmentationLogicStep::qSlicerEMSegmentSegmentationLogicStep(
    ctkWorkflow* newWorkflow) : Superclass(newWorkflow, Self::StepId)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentSegmentationLogicStep);
  this->setName("***Runs segmentation logic***");
  this->setDescription("***EM algorithm processing to segment target image.***");
  this->setIcon(QIcon(":/Icons/Go.png"));
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSegmentationLogicStep::populateStepWidgetsList(QList<QWidget*>& stepWidgetsList)
{
  emit populateStepWidgetsListComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSegmentationLogicStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  emit validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSegmentationLogicStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{

  Q_UNUSED(comingFrom);
  Q_UNUSED(transitionType);

  Q_ASSERT(this->mrmlManager());

  vtkSlicerEMSegmentLogic* logic = this->emSegmentLogic();
  Q_ASSERT(logic);

  // make sure that data types are the same
  if (!this->mrmlManager()->DoTargetAndAtlasDataTypesMatch())
    {
    QMessageBox::critical(this->stepArea(), "EMSegmenter",
                         tr("Scalar type mismatch for input images.  All image scalar types must be the same (including input channels and atlas images)."));
    emit onEntryComplete();
    return;
    }

  // start the segmentation
  logic->StartSegmentationWithoutPreprocessing();

  // Signals that we are finished
  emit onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSegmentationLogicStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  // Signals that we are finished
  emit onExitComplete();
}

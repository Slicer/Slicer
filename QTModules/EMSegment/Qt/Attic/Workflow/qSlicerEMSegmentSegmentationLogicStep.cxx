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
#include <QStyle>
#include <QDebug>

// EMSegment includes
#include "qSlicerEMSegmentSegmentationLogicStep.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>

// EMSegment/Logic includes
#include <vtkEMSegmentLogic.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_EMSegment
class qSlicerEMSegmentSegmentationLogicStepPrivate
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
  ctkWorkflow* newWorkflow, QWidget* newWidget)
  : Superclass(newWorkflow, qSlicerEMSegmentSegmentationLogicStep::StepId, newWidget)
  , d_ptr(new qSlicerEMSegmentSegmentationLogicStepPrivate)
{

  this->setName("***Runs segmentation logic***");
  this->setDescription("***EM algorithm processing to segment target image.***");
  this->setIcon(QIcon(":/Icons/Go.png"));
}

//-----------------------------------------------------------------------------
qSlicerEMSegmentSegmentationLogicStep::~qSlicerEMSegmentSegmentationLogicStep()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSegmentationLogicStep::createUserInterface()
{
  this->createUserInterfaceComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSegmentationLogicStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  this->validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSegmentationLogicStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{

  Q_UNUSED(comingFrom);
  Q_UNUSED(transitionType);

  Q_ASSERT(this->mrmlManager());

  vtkEMSegmentLogic* logic = this->emSegmentLogic();
  Q_ASSERT(logic);

  // make sure that data types are the same
  if (!this->mrmlManager()->DoTargetAndAtlasDataTypesMatch(this->mrmlManager()->GetTargetInputNode(),this->mrmlManager()->GetAtlasInputNode()))
    {
    QMessageBox::critical(this->stepArea(), "EMSegmenter",
                         tr("Scalar type mismatch for input images.  All image scalar types must be the same (including input channels and atlas images)."));
    this->onEntryComplete();
    return;
    }

  // start the segmentation
  logic->StartSegmentationWithoutPreprocessingAndSaving();

  // Indicates that we are finished
  this->onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSegmentationLogicStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  Q_UNUSED(goingTo);
  Q_UNUSED(transitionType);
  // Indicates that we are finished
  this->onExitComplete();
}

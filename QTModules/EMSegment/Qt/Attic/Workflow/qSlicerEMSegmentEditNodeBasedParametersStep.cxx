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

// EMSegment includes
#include "qSlicerEMSegmentEditNodeBasedParametersStep.h"
#include "ui_qSlicerEMSegmentEditNodeBasedParametersStep.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_EMSegment
class qSlicerEMSegmentEditNodeBasedParametersStepPrivate: public Ui_qSlicerEMSegmentEditNodeBasedParametersStep
{
public:
  qSlicerEMSegmentEditNodeBasedParametersStepPrivate();

  void updateWidgetFromMRML();
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentEditNodeBasedParametersStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentEditNodeBasedParametersStepPrivate::qSlicerEMSegmentEditNodeBasedParametersStepPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditNodeBasedParametersStepPrivate::updateWidgetFromMRML()
{
  this->EMSegmentAnatomicalTreeWidget->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentEditNodeBasedParametersStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentEditNodeBasedParametersStep::StepId = "EditNodeBasedParameters";

//-----------------------------------------------------------------------------
qSlicerEMSegmentEditNodeBasedParametersStep::qSlicerEMSegmentEditNodeBasedParametersStep(
  ctkWorkflow* newWorkflow, QWidget* newWidget)
  : Superclass(newWorkflow, qSlicerEMSegmentEditNodeBasedParametersStep::StepId, newWidget)
  , d_ptr(new qSlicerEMSegmentEditNodeBasedParametersStepPrivate)
{
  Q_D(qSlicerEMSegmentEditNodeBasedParametersStep);
  d->setupUi(this);

  this->setName("8/9. Edit Node-based Parameters");
  this->setDescription("Specify node-based segmentation parameters.");
}

//-----------------------------------------------------------------------------
qSlicerEMSegmentEditNodeBasedParametersStep::~qSlicerEMSegmentEditNodeBasedParametersStep()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditNodeBasedParametersStep::createUserInterface()
{
  this->createUserInterfaceComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditNodeBasedParametersStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  this->validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditNodeBasedParametersStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  Q_UNUSED(comingFrom);
  Q_UNUSED(transitionType);
  Q_D(qSlicerEMSegmentEditNodeBasedParametersStep);
  d->updateWidgetFromMRML();

  // Indicates that we are finished
  this->onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditNodeBasedParametersStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  Q_UNUSED(goingTo);
  Q_UNUSED(transitionType);
  // Indicates that we are finished
  this->onExitComplete();
}


//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditNodeBasedParametersStep::showUserInterface()
{
  Q_D(qSlicerEMSegmentEditNodeBasedParametersStep);
  this->Superclass::showUserInterface();
  d->updateWidgetFromMRML();
}

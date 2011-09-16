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
#include "qSlicerEMSegmentDefineAnatomicalTreeStep.h"
#include "ui_qSlicerEMSegmentDefineAnatomicalTreeStep.h"

// CTK includes
#include "ctkWorkflowWidgetStep.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_EMSegment
class qSlicerEMSegmentDefineAnatomicalTreeStepPrivate: public Ui_qSlicerEMSegmentDefineAnatomicalTreeStep
{
public:
  qSlicerEMSegmentDefineAnatomicalTreeStepPrivate();

  void updateWidgetFromMRML();
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefineAnatomicalTreeStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineAnatomicalTreeStepPrivate::qSlicerEMSegmentDefineAnatomicalTreeStepPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAnatomicalTreeStepPrivate::updateWidgetFromMRML()
{
  this->EMSegmentAnatomicalTreeWidget->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefineAnatomicalTreeStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentDefineAnatomicalTreeStep::StepId = "DefineAnatomicalTree";

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineAnatomicalTreeStep::qSlicerEMSegmentDefineAnatomicalTreeStep(
  ctkWorkflow* newWorkflow, QWidget* newWidget)
  : Superclass(newWorkflow, qSlicerEMSegmentDefineAnatomicalTreeStep::StepId, newWidget)
  , d_ptr(new qSlicerEMSegmentDefineAnatomicalTreeStepPrivate)
{
  Q_D(qSlicerEMSegmentDefineAnatomicalTreeStep);
  d->setupUi(this);

  this->setName("3/9. Define Anatomical Tree");
  this->setDescription("Define a hierarchy of structures.");
}

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineAnatomicalTreeStep::~qSlicerEMSegmentDefineAnatomicalTreeStep()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAnatomicalTreeStep::createUserInterface()
{
  this->createUserInterfaceComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAnatomicalTreeStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  this->validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAnatomicalTreeStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  Q_UNUSED(comingFrom);
  Q_UNUSED(transitionType);
  Q_D(qSlicerEMSegmentDefineAnatomicalTreeStep);
  d->updateWidgetFromMRML();

  // Indicates that we are finished
  this->onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAnatomicalTreeStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  Q_UNUSED(goingTo);
  Q_UNUSED(transitionType);
  // Indicates that we are finished
  this->onExitComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAnatomicalTreeStep::showUserInterface()
{
  Q_D(qSlicerEMSegmentDefineAnatomicalTreeStep);
  this->Superclass::showUserInterface();
  d->updateWidgetFromMRML();
}

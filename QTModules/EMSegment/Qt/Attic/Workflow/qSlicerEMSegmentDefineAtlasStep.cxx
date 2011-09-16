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
#include "qSlicerEMSegmentDefineAtlasStep.h"
#include "ui_qSlicerEMSegmentDefineAtlasStep.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_EMSegment
class qSlicerEMSegmentDefineAtlasStepPrivate: public Ui_qSlicerEMSegmentDefineAtlasStep
{
public:
  qSlicerEMSegmentDefineAtlasStepPrivate();

  void updateWidgetFromMRML();
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefineAtlasStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineAtlasStepPrivate::qSlicerEMSegmentDefineAtlasStepPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAtlasStepPrivate::updateWidgetFromMRML()
{
  this->EMSegmentAnatomicalTreeWidget->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefineAtlasStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentDefineAtlasStep::StepId = "DefineAtlas";

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineAtlasStep::qSlicerEMSegmentDefineAtlasStep(
  ctkWorkflow* newWorkflow, QWidget* newWidget)
  : Superclass(newWorkflow, qSlicerEMSegmentDefineAtlasStep::StepId, newWidget)
  , d_ptr(new qSlicerEMSegmentDefineAtlasStepPrivate)
{
  Q_D(qSlicerEMSegmentDefineAtlasStep);
  d->setupUi(this);

  this->setName("4/9. Define Atlas");
  this->setDescription("Define probability maps and image scans of atlas.");
}

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineAtlasStep::~qSlicerEMSegmentDefineAtlasStep()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAtlasStep::createUserInterface()
{
  this->createUserInterfaceComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAtlasStep::showUserInterface()
{
  Q_D(qSlicerEMSegmentDefineAtlasStep);
  this->Superclass::showUserInterface();
  d->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAtlasStep::onEntry(const ctkWorkflowStep* comingFrom,
   const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  Q_UNUSED(comingFrom);
  Q_UNUSED(transitionType);
  Q_D(qSlicerEMSegmentDefineAtlasStep);
  d->updateWidgetFromMRML();

  // Indicates that we are finished
  this->onEntryComplete();
}

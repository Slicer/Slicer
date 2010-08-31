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

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
#include "qSlicerEMSegmentEditNodeBasedParametersStep.h"
#include "qSlicerEMSegmentEditNodeBasedParametersPanel.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentEditNodeBasedParametersStepPrivate : public ctkPrivate<qSlicerEMSegmentEditNodeBasedParametersStep>
{
public:
  qSlicerEMSegmentEditNodeBasedParametersStepPrivate();
  qSlicerEMSegmentEditNodeBasedParametersPanel* panel;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentEditNodeBasedParametersStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentEditNodeBasedParametersStepPrivate::qSlicerEMSegmentEditNodeBasedParametersStepPrivate()
{
  this->panel = 0;
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentEditNodeBasedParametersStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentEditNodeBasedParametersStep::StepId = "EditNodeBasedParameters";

//-----------------------------------------------------------------------------
qSlicerEMSegmentEditNodeBasedParametersStep::qSlicerEMSegmentEditNodeBasedParametersStep(
    ctkWorkflow* newWorkflow) : Superclass(newWorkflow, Self::StepId)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentEditNodeBasedParametersStep);
  this->setName("8/9. Edit Node-based Parameters");
  this->setDescription("Specify node-based segmentation parameters.");
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditNodeBasedParametersStep::populateStepWidgetsList(QList<QWidget*>& stepWidgetsList)
{
  CTK_D(qSlicerEMSegmentEditNodeBasedParametersStep);
  if (!d->panel)
    {
    d->panel = new qSlicerEMSegmentEditNodeBasedParametersPanel;
    connect(this, SIGNAL(mrmlManagerChanged(vtkEMSegmentMRMLManager*)),
            d->panel, SLOT(setMRMLManager(vtkEMSegmentMRMLManager*)));
    d->panel->setMRMLManager(this->mrmlManager());
    }
  stepWidgetsList << d->panel;
  emit populateStepWidgetsListComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditNodeBasedParametersStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  emit validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditNodeBasedParametersStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{

  // Signals that we are finished
  emit onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditNodeBasedParametersStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  // Signals that we are finished
  emit onExitComplete();
}


//-----------------------------------------------------------------------------
void qSlicerEMSegmentEditNodeBasedParametersStep::showUserInterface()
{
  CTK_D(qSlicerEMSegmentEditNodeBasedParametersStep);
  this->Superclass::showUserInterface();
  d->panel->updateWidgetFromMRML();
}

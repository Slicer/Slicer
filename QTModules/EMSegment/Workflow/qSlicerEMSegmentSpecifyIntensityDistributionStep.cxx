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
#include "qSlicerEMSegmentSpecifyIntensityDistributionStep.h"
#include "qSlicerEMSegmentSpecifyIntensityDistributionPanel.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate : public ctkPrivate<qSlicerEMSegmentSpecifyIntensityDistributionStep>
{
public:
  qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate();
  qSlicerEMSegmentSpecifyIntensityDistributionPanel* panel;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate::qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate()
{
  this->panel = 0;
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentSpecifyIntensityDistributionStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentSpecifyIntensityDistributionStep::StepId =
    "SpecifyIntensityDistribution";

//-----------------------------------------------------------------------------
qSlicerEMSegmentSpecifyIntensityDistributionStep::qSlicerEMSegmentSpecifyIntensityDistributionStep(
    ctkWorkflow* newWorkflow) : Superclass(newWorkflow, Self::StepId)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  this->setName("7/9. Specify Intensity Distributions");
  this->setDescription("Define intensity distribution for each anatomical structure.");
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::populateStepWidgetsList(QList<QWidget*>& stepWidgetsList)
{
  CTK_D(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  if (!d->panel)
    {
    d->panel = new qSlicerEMSegmentSpecifyIntensityDistributionPanel;
    connect(this, SIGNAL(mrmlManagerChanged(vtkEMSegmentMRMLManager*)),
            d->panel, SLOT(setMRMLManager(vtkEMSegmentMRMLManager*)));
    d->panel->setMRMLManager(this->mrmlManager());
    }
  stepWidgetsList << d->panel;
  emit populateStepWidgetsListComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  emit validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  // Signals that we are finished
  emit onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  // Signals that we are finished
  emit onExitComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::showUserInterface()
{
  CTK_D(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  this->Superclass::showUserInterface();
  d->panel->updateWidgetFromMRML();
}

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
#include <QDebug>

// CTK includes
#include <ctkWorkflow.h>

// EMSegment includes
#include "qSlicerEMSegmentDefineTaskStep.h"
#include "qSlicerEMSegmentDefineTaskPanel.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentDefineTaskStepPrivate : public ctkPrivate<qSlicerEMSegmentDefineTaskStep>
{
public:
  qSlicerEMSegmentDefineTaskStepPrivate();
  qSlicerEMSegmentDefineTaskPanel* panel;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefineTaskStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineTaskStepPrivate::qSlicerEMSegmentDefineTaskStepPrivate()
{
  this->panel = 0;
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefineTaskStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentDefineTaskStep::StepId = "DefineTask";

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineTaskStep::qSlicerEMSegmentDefineTaskStep(
    ctkWorkflow* newWorkflow) : Superclass(newWorkflow, Self::StepId)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentDefineTaskStep);
  this->setName("1/9. Define Task");
  this->setDescription("Select a (new) task.");
  this->setButtonBoxHints(ctkWorkflowWidgetStep::ButtonBoxHidden);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineTaskStep::populateStepWidgetsList(QList<QWidget*>& stepWidgetsList)
{
  qDebug() << "qSlicerEMSegmentDefineTaskStep::populateStepWidgetsList";
  CTK_D(qSlicerEMSegmentDefineTaskStep);
  if (!d->panel)
    {
    d->panel = new qSlicerEMSegmentDefineTaskPanel;

    // Connect panels
    connect(this, SIGNAL(mrmlManagerChanged(vtkEMSegmentMRMLManager*)),
            d->panel, SLOT(setMRMLManager(vtkEMSegmentMRMLManager*)));
    d->panel->setMRMLManager(this->mrmlManager());

    connect(d->panel, SIGNAL(modeChanged(const QString&)),
            this->workflow(), SLOT(goForward(const QString&)));
    }
  stepWidgetsList << d->panel;
  emit populateStepWidgetsListComplete();
}

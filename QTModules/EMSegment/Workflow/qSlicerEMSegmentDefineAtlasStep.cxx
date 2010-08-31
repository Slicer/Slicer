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
#include "qSlicerEMSegmentDefineAtlasStep.h"
#include "qSlicerEMSegmentDefineAtlasPanel.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentDefineAtlasStepPrivate : public ctkPrivate<qSlicerEMSegmentDefineAtlasStep>
{
public:
  qSlicerEMSegmentDefineAtlasStepPrivate();
  qSlicerEMSegmentDefineAtlasPanel* panel;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefineAtlasStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineAtlasStepPrivate::qSlicerEMSegmentDefineAtlasStepPrivate()
{
  this->panel = 0;
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentDefineAtlasStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentDefineAtlasStep::StepId = "DefineAtlas";

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineAtlasStep::qSlicerEMSegmentDefineAtlasStep(
    ctkWorkflow* newWorkflow) : Superclass(newWorkflow, Self::StepId)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentDefineAtlasStep);
  this->setName("4/9. DefineAtlas");
  this->setDescription("Define probability maps and image scans of atlas.");
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAtlasStep::populateStepWidgetsList(QList<QWidget*>& stepWidgetsList)
{
  CTK_D(qSlicerEMSegmentDefineAtlasStep);
  if (!d->panel)
    {
    d->panel = new qSlicerEMSegmentDefineAtlasPanel;
    connect(this, SIGNAL(mrmlManagerChanged(vtkEMSegmentMRMLManager*)),
            d->panel, SLOT(setMRMLManager(vtkEMSegmentMRMLManager*)));
    d->panel->setMRMLManager(this->mrmlManager());
    }
  stepWidgetsList << d->panel;
  emit populateStepWidgetsListComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineAtlasStep::showUserInterface()
{
  CTK_D(qSlicerEMSegmentDefineAtlasStep);
  this->Superclass::showUserInterface();
  d->panel->updateWidgetFromMRML();
}

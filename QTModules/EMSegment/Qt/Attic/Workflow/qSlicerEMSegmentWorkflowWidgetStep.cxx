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
#include <QVBoxLayout>

// CTK includes
#include <ctkWorkflow.h>
#include <ctkWorkflowWidget.h>

// EMSegment includes
#include "qSlicerEMSegmentWorkflowWidgetStep.h"
#include "qSlicerEMSegmentWorkflowWidget.h"

// EMSegment/Logic includes
#include <vtkEMSegmentLogic.h>

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_EMSegment
class qSlicerEMSegmentWorkflowWidgetStepPrivate
{
public:
  qSlicerEMSegmentWorkflowWidgetStepPrivate();

  vtkEMSegmentMRMLManager *        MRMLManager;
  vtkEMSegmentLogic *        EMSegmentLogic;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentWorkflowWidgetStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentWorkflowWidgetStepPrivate::qSlicerEMSegmentWorkflowWidgetStepPrivate()
{
  this->MRMLManager = 0;
  this->EMSegmentLogic = 0;
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentWorkflowWidgetStep methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentWorkflowWidgetStep::qSlicerEMSegmentWorkflowWidgetStep(
  ctkWorkflow* newWorkflow, const QString& newId, QWidget* newParent) : 
Superclass(newWorkflow, newId, newParent)
  , d_ptr(new qSlicerEMSegmentWorkflowWidgetStepPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerEMSegmentWorkflowWidgetStep::qSlicerEMSegmentWorkflowWidgetStep(QWidget* newParent) : 
Superclass(newParent)
  , d_ptr(new qSlicerEMSegmentWorkflowWidgetStepPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerEMSegmentWorkflowWidgetStep::~qSlicerEMSegmentWorkflowWidgetStep()
{
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerEMSegmentWorkflowWidgetStep, vtkEMSegmentMRMLManager*, mrmlManager, MRMLManager);
CTK_GET_CPP(qSlicerEMSegmentWorkflowWidgetStep, vtkEMSegmentLogic*, emSegmentLogic, EMSegmentLogic);
CTK_SET_CPP(qSlicerEMSegmentWorkflowWidgetStep, vtkEMSegmentLogic*, setEMSegmentLogic, EMSegmentLogic);

//-----------------------------------------------------------------------------
vtkMRMLScene * qSlicerEMSegmentWorkflowWidgetStep::mrmlScene()const
{
  Q_D(const qSlicerEMSegmentWorkflowWidgetStep);
  return d->MRMLManager ? d->MRMLManager->GetMRMLScene() : 0;
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentWorkflowWidgetStep::setMRMLManager(vtkEMSegmentMRMLManager * newMRMLManager)
{
  Q_D(qSlicerEMSegmentWorkflowWidgetStep);
  if (d->MRMLManager == newMRMLManager)
    {
    return;
    }

  d->MRMLManager = newMRMLManager;

  emit mrmlSceneChanged(newMRMLManager ? newMRMLManager->GetMRMLScene() : 0);

  emit mrmlManagerChanged(newMRMLManager);
}

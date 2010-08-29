
// CTK includes
#include <ctkWorkflow.h>
#include <ctkWorkflowWidget.h>

// EMSegment includes
#include "qSlicerEMSegmentWorkflowWidgetStep.h"
#include "qSlicerEMSegmentWorkflowWidget.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>

//-----------------------------------------------------------------------------
class qSlicerEMSegmentWorkflowWidgetStepPrivate : public ctkPrivate<qSlicerEMSegmentWorkflowWidgetStep>
{
public:
  qSlicerEMSegmentWorkflowWidgetStepPrivate();

  vtkEMSegmentMRMLManager *        MRMLManager;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentWorkflowWidgetStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentWorkflowWidgetStepPrivate::qSlicerEMSegmentWorkflowWidgetStepPrivate()
{
  this->MRMLManager = 0;
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentWorkflowWidgetStep methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentWorkflowWidgetStep::qSlicerEMSegmentWorkflowWidgetStep(
    ctkWorkflow* newWorkflow, const QString& newId) : Superclass(newWorkflow, newId)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentWorkflowWidgetStep);
}

//-----------------------------------------------------------------------------
CTK_GET_CXX(qSlicerEMSegmentWorkflowWidgetStep, vtkEMSegmentMRMLManager*, mrmlManager, MRMLManager);

//-----------------------------------------------------------------------------
vtkMRMLScene * qSlicerEMSegmentWorkflowWidgetStep::mrmlScene()const
{
  CTK_D(const qSlicerEMSegmentWorkflowWidgetStep);
  return d->MRMLManager ? d->MRMLManager->GetMRMLScene() : 0;
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentWorkflowWidgetStep::setMRMLManager(vtkEMSegmentMRMLManager * newMRMLManager)
{
  CTK_D(qSlicerEMSegmentWorkflowWidgetStep);
  if (d->MRMLManager == newMRMLManager)
    {
    return;
    }

  d->MRMLManager = newMRMLManager;

  emit mrmlSceneChanged(newMRMLManager ? newMRMLManager->GetMRMLScene() : 0);

  emit mrmlManagerChanged(newMRMLManager);
}

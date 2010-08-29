
// CTK includes
#include <ctkWorkflow.h>
#include <ctkWorkflowWidget.h>

// EMSegment includes
#include "qSlicerEMSegmentWorkflowWidgetStep.h"
#include "qSlicerEMSegmentWorkflowWidget.h"

// EMSegment/Logic includes
#include <vtkSlicerEMSegmentLogic.h>

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>

//-----------------------------------------------------------------------------
class qSlicerEMSegmentWorkflowWidgetStepPrivate : public ctkPrivate<qSlicerEMSegmentWorkflowWidgetStep>
{
public:
  qSlicerEMSegmentWorkflowWidgetStepPrivate();

  vtkEMSegmentMRMLManager *        MRMLManager;
  vtkSlicerEMSegmentLogic *        EMSegmentLogic;
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
    ctkWorkflow* newWorkflow, const QString& newId) : Superclass(newWorkflow, newId)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentWorkflowWidgetStep);
}

//-----------------------------------------------------------------------------
CTK_GET_CXX(qSlicerEMSegmentWorkflowWidgetStep, vtkEMSegmentMRMLManager*, mrmlManager, MRMLManager);
CTK_GET_CXX(qSlicerEMSegmentWorkflowWidgetStep, vtkSlicerEMSegmentLogic*, emSegmentLogic, EMSegmentLogic);
CTK_SET_CXX(qSlicerEMSegmentWorkflowWidgetStep, vtkSlicerEMSegmentLogic*, setEMSegmentLogic, EMSegmentLogic);

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

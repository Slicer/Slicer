#ifndef __qSlicerEMSegmentWorkflowWidgetStep_h
#define __qSlicerEMSegmentWorkflowWidgetStep_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkWorkflowWidgetStep.h>
#include <ctkWorkflowTransitions.h>

// EMSegment includes
#include "qSlicerEMSegmentWidget.h"

class qSlicerEMSegmentWorkflowWidgetStepPrivate;
class qSlicerEMSegmentWorkflowWidget;
class vtkEMSegmentMRMLManager;

class qSlicerEMSegmentWorkflowWidgetStep : public ctkWorkflowWidgetStep
{
  Q_OBJECT

public:

  typedef ctkWorkflowWidgetStep Superclass;
  explicit qSlicerEMSegmentWorkflowWidgetStep(ctkWorkflow* newWorkflow, const QString& newId = "");

  vtkMRMLScene *           mrmlScene() const;
  vtkEMSegmentMRMLManager* mrmlManager() const;

public slots:
  void setMRMLManager(vtkEMSegmentMRMLManager * newMRMLManager);

signals:

  void mrmlSceneChanged(vtkMRMLScene* newMRMLScene);

  void mrmlManagerChanged(vtkEMSegmentMRMLManager* newMRMLManager);

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentWorkflowWidgetStep);

};

#endif

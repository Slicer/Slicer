#ifndef __qSlicerEMSegmentDefineAtlasStep_h
#define __qSlicerEMSegmentDefineAtlasStep_h

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWorkflowWidgetStep.h"

class qSlicerEMSegmentDefineAtlasStepPrivate;

class qSlicerEMSegmentDefineAtlasStep : public qSlicerEMSegmentWorkflowWidgetStep
{
  Q_OBJECT

public:

  const static QString StepId;

  typedef qSlicerEMSegmentWorkflowWidgetStep Superclass;
  explicit qSlicerEMSegmentDefineAtlasStep(ctkWorkflow* newWorkflow);

public slots:

  virtual void populateStepWidgetsList(QList<QWidget*>& stepWidgetsList);

  virtual void showUserInterface();

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentDefineAtlasStep);

};



#endif

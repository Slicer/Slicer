
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

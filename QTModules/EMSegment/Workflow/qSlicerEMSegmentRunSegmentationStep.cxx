
// EMSegment includes
#include "qSlicerEMSegmentRunSegmentationStep.h"
#include "qSlicerEMSegmentRunSegmentationPanel.h"
#include "vtkSlicerEMSegmentLogic.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>
#include <vtkMRMLEMSWorkingDataNode.h>
#include <vtkMRMLEMSTargetNode.h>
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLROINode.h>

//-----------------------------------------------------------------------------
class qSlicerEMSegmentRunSegmentationStepPrivate : public ctkPrivate<qSlicerEMSegmentRunSegmentationStep>
{
public:
  qSlicerEMSegmentRunSegmentationStepPrivate();
  qSlicerEMSegmentRunSegmentationPanel* Panel;

  vtkMRMLROINode* ROINode;

  // Delete any ROI nodes named "SegmentationROI", and create a new ROI node
  void createROINode();

  // Propagate changes in ROINode MRML to EMSegmentRunSegmentationStep ROI MRML
  void MRMLUpdateROINodeFromROI();
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentRunSegmentationStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentRunSegmentationStepPrivate::qSlicerEMSegmentRunSegmentationStepPrivate()
{
  this->Panel = 0;
  this->ROINode = 0;
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStepPrivate::createROINode()
{
  CTK_P(qSlicerEMSegmentRunSegmentationStep);

  Q_ASSERT(!this->ROINode);

  // First find out if ROI node in scene
  vtkMRMLScene* scene = p->mrmlScene();
  int numNodes = scene->GetNumberOfNodesByClass("vtkMRMLROINode");
  const char nodeName[40] = "SegmentationROI";

  // Remove any ROI nodes named "SegmentationROI"
  for (int i = 0; i < numNodes; ++i)
    {
    vtkMRMLROINode* node = (vtkMRMLROINode*)  scene->GetNthNodeByClass(i, "vtkMRMLROINode");
    if (node && node->GetName() && !strcmp(node->GetName(), nodeName) )
      {
      // This does not work - so  have to delete it 
      scene->RemoveNode(node);
      }
    }

  // Create new ROI if necessary
  this->ROINode = static_cast<vtkMRMLROINode*>(scene->CreateNodeByClass("vtkMRMLROINode"));
  this->ROINode->SetName("SegmentationROI");
  scene->AddNode(this->ROINode);
  this->ROINode->SetVisibility(0);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStepPrivate::MRMLUpdateROINodeFromROI()
{
  CTK_P(qSlicerEMSegmentRunSegmentationStep);

  double ROIMinIJK[4], ROIMaxIJK[4], ROIMinRAS[4], ROIMaxRAS[4];
  double radius[3], center[3];

  vtkMRMLVolumeNode* volumeNode =  p->mrmlManager()->GetWorkingDataNode()->GetInputTargetNode()->GetNthVolumeNode(0);
  if (!volumeNode)
    {
    return;
    }

  p->emSegmentLogic()->DefineValidSegmentationBoundary();
  int minPoint[3];
  int maxPoint[3];
  p->mrmlManager()->GetSegmentationBoundaryMin(minPoint);
  p->mrmlManager()->GetSegmentationBoundaryMax(maxPoint);

  // Has to be set to -1 because segmentationboundarymin/max starts at 1 not 0 !   
  ROIMinIJK[0] = minPoint[0] - 1;
  ROIMinIJK[1] = minPoint[1] -1 ;
  ROIMinIJK[2] = minPoint[2] -1 ;
  ROIMinIJK[3] = 1.;
  ROIMaxIJK[0] = maxPoint[0] -1 ;
  ROIMaxIJK[1] = maxPoint[1] -1 ;
  ROIMaxIJK[2] = maxPoint[2] -1 ;
  ROIMaxIJK[3] = 1.;

  vtkMatrix4x4 *ijkToras = vtkMatrix4x4::New();
  volumeNode->GetIJKToRASMatrix(ijkToras);
  ijkToras->MultiplyPoint(ROIMinIJK,ROIMinRAS);
  ijkToras->MultiplyPoint(ROIMaxIJK,ROIMaxRAS);
  ijkToras->Delete();

  center[0] = (ROIMaxRAS[0]+ROIMinRAS[0])/2.;
  center[1] = (ROIMaxRAS[1]+ROIMinRAS[1])/2.;
  center[2] = (ROIMaxRAS[2]+ROIMinRAS[2])/2.;

  radius[0] = fabs(ROIMaxRAS[0]-ROIMinRAS[0])/2.;
  radius[1] = fabs(ROIMaxRAS[1]-ROIMinRAS[1])/2.;
  radius[2] = fabs(ROIMaxRAS[2]-ROIMinRAS[2])/2.;
 
  this->ROINode->SetXYZ(center[0], center[1], center[2]);
  this->ROINode->SetRadiusXYZ(radius[0], radius[1], radius[2]);
  this->ROINode->Modified();
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentRunSegmentationStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentRunSegmentationStep::StepId = "RunSegmentation";

//-----------------------------------------------------------------------------
qSlicerEMSegmentRunSegmentationStep::qSlicerEMSegmentRunSegmentationStep(
    ctkWorkflow* newWorkflow) : Superclass(newWorkflow, Self::StepId)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentRunSegmentationStep);
  this->setName("9/9. Run Segmentation");
  this->setDescription("Apply EM algorithm to segment target image.");
  this->setButtonBoxHints(ctkWorkflowWidgetStep::NextButtonDisabled);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStep::populateStepWidgetsList(QList<QWidget*>& stepWidgetsList)
{
  CTK_D(qSlicerEMSegmentRunSegmentationStep);
  if (!d->Panel)
    {
    d->Panel = new qSlicerEMSegmentRunSegmentationPanel;
    connect(this, SIGNAL(mrmlManagerChanged(vtkEMSegmentMRMLManager*)),
            d->Panel, SLOT(setMRMLManager(vtkEMSegmentMRMLManager*)));
    d->Panel->setMRMLManager(this->mrmlManager());
    d->Panel->setMRMLROINode(d->ROINode);
    }
  stepWidgetsList << d->Panel;

  emit populateStepWidgetsListComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStep::showUserInterface()
{
  CTK_D(qSlicerEMSegmentRunSegmentationStep);
  this->Superclass::showUserInterface();
  d->Panel->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  emit validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  Q_UNUSED(comingFrom);
  Q_UNUSED(transitionType);

  CTK_D(qSlicerEMSegmentRunSegmentationStep);

  // Create ROI MRML node
  if (!d->ROINode)
    {
    d->createROINode();
    }

  // update the roiNode ROI to reflect what is stored in ROI MRML
  // SegmentationBoundary sets ROI Node
  d->MRMLUpdateROINodeFromROI();

  // TODO the node has to be connected to the ROIMRMLCallback

  

  // Signals that we are finished
  emit onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  Q_UNUSED(goingTo);
  Q_UNUSED(transitionType);

  CTK_D(qSlicerEMSegmentRunSegmentationStep);

  Q_ASSERT(this->mrmlManager());

  if (transitionType != ctkWorkflowInterstepTransition::TransitionToNextStep)
    {
    // Signals that we are finished
    emit onExitComplete();
    return;
    }

  d->Panel->updateMRMLFromWidget();

  // Signals that we are finished
  emit onExitComplete();
}

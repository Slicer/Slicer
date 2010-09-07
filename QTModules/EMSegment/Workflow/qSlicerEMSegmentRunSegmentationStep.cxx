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
#include <QDebug>

// CTK includes
#include <ctkLogger.h>

// qMRMLWidgets includes
#include <qMRMLNodeFactory.h>

// EMSegment includes
#include "qSlicerEMSegmentRunSegmentationStep.h"
#include "ui_qSlicerEMSegmentRunSegmentationStep.h"
#include "qSlicerEMSegmentRunSegmentationStep_p.h"
#include "vtkSlicerEMSegmentLogic.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>
#include <vtkMRMLEMSWorkingDataNode.h>
#include <vtkMRMLEMSTargetNode.h>
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLROINode.h>

// VTKMRML includes
#include "vtkMRMLSliceLogic.h"

//--------------------------------------------------------------------------
static ctkLogger logger(
    "org.slicer.qtmodules.emsegment.workflow.qSlicerEMSegmentRunSegmentationStep");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// qSlicerEMSegmentRunSegmentationStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentRunSegmentationStepPrivate::qSlicerEMSegmentRunSegmentationStepPrivate()
{
  this->ROINode = 0;
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStepPrivate::setupUi(
    qSlicerEMSegmentWorkflowWidgetStep* step)
{
  this->Ui_qSlicerEMSegmentRunSegmentationStep::setupUi(step);

  // Set attributes on the qMRMLNodeComboBox to select the output label map
  this->OutputLabelMapComboBox->addAttribute("vtkMRMLScalarVolumeNode", "LabelMap", "1");

  // Setup connections
  QObject::connect(this->Display2DVOIButton, SIGNAL(clicked(bool)),
                   this, SLOT(display2DVOI(bool)));
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStepPrivate::setMRMLROINode(vtkMRMLROINode* node)
{
  CTK_P(qSlicerEMSegmentRunSegmentationStep);

  Q_ASSERT(node);
  Q_ASSERT(p->mrmlManager());
  Q_ASSERT(this->ROIWidget);

  // Finish setting up the ROI widget, now that we have a MRML manager
  vtkMRMLVolumeNode* volumeNode =
      p->mrmlManager()->GetWorkingDataNode()->GetInputTargetNode()->GetNthVolumeNode(0);

  double rasDimensions[3];
  double rasCenter[3];
  vtkMRMLSliceLogic::GetVolumeRASBox(volumeNode, rasDimensions, rasCenter);

  double rasBounds[6];
  rasBounds[0] = qMin(rasCenter[0]-rasDimensions[0]/2.,rasCenter[0]+rasDimensions[0]/2.);
  rasBounds[1] = qMin(rasCenter[1]-rasDimensions[1]/2.,rasCenter[1]+rasDimensions[1]/2.);
  rasBounds[2] = qMin(rasCenter[2]-rasDimensions[2]/2.,rasCenter[2]+rasDimensions[2]/2.);
  rasBounds[3] = qMax(rasCenter[0]-rasDimensions[0]/2.,rasCenter[0]+rasDimensions[0]/2.);
  rasBounds[4] = qMax(rasCenter[1]-rasDimensions[1]/2.,rasCenter[1]+rasDimensions[1]/2.);
  rasBounds[5] = qMax(rasCenter[2]-rasDimensions[2]/2.,rasCenter[2]+rasDimensions[2]/2.);
    
  // TODO
  // this->ROIWidget->SetXRangeExtent(rasBounds[0],rasBounds[3]);
  // this->ROIWidget->SetYRangeExtent(rasBounds[1],rasBounds[4]);
  // this->ROIWidget->SetZRangeExtent(rasBounds[2],rasBounds[5]);

  // this->ROIWidget->SetXResolution(fabs(rasBounds[3]-rasBounds[0])/100.);
  // this->ROIWidget->SetYResolution(fabs(rasBounds[4]-rasBounds[1])/100.);
  // this->ROIWidget->SetZResolution(fabs(rasBounds[5]-rasBounds[2])/100.);

  this->ROIWidget->setMRMLROINode(node);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStepPrivate::updateWidgetFromMRML()
{
  CTK_P(qSlicerEMSegmentRunSegmentationStep);

  if (!p->mrmlManager())
    {
    logger.warn("updateWidgetFromMRML - MRMLManager is NULL");
    return;
    }

  // Disable widget if needed
  QList<QWidget*> widgets;
  widgets << this->SaveIntermediateResultsCheckBox
          << this->OutputLabelMapComboBox
          << this->ROIWidget
          << this->Display2DVOIButton
          << this->MultithreadingCheckBox;

  foreach(QWidget* w, widgets)
    {
    w->setEnabled(p->mrmlManager()->HasGlobalParametersNode());
    }

  // Working directory
  this->SaveIntermediateResultsCheckBox->setChecked(
      p->mrmlManager()->GetSaveIntermediateResults());

  if (p->mrmlManager()->GetSaveWorkingDirectory())
    {
    this->DirectoryButton->setDirectory(
        p->mrmlManager()->GetSaveWorkingDirectory());
    }
  else
    {
    this->DirectoryButton->setCaption(tr("Select Working Directory"));
    }

  // Output label map
  if (!p->mrmlManager()->GetOutputVolumeMRMLID())
    {
    qMRMLNodeFactory::AttributeType attributes;
    attributes.insert("LabelMap", "1");
    qMRMLNodeFactory::createNode(
        p->mrmlScene(), "vtkMRMLScalarVolumeNode", vtkMRMLNodeInitializer(), attributes);
    }

  this->OutputLabelMapComboBox->setCurrentNode(p->mrmlManager()->GetOutputVolumeMRMLID());

  // Define VOI
  // TODO: should call SetMRMLROINode above? Incase the working volume has changed?

  // Miscellaneous
  this->MultithreadingCheckBox->setChecked(p->mrmlManager()->GetEnableMultithreading());
  
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStepPrivate::updateMRMLFromWidget()
{
  CTK_P(qSlicerEMSegmentRunSegmentationStep);

  Q_ASSERT(p->mrmlManager());

  // Working directory
  p->mrmlManager()->SetSaveIntermediateResults(this->SaveIntermediateResultsCheckBox->isChecked());
  p->mrmlManager()->SetSaveWorkingDirectory(this->DirectoryButton->directory().toLatin1());

  // Output label map
  p->mrmlManager()->SetOutputVolumeMRMLID(this->OutputLabelMapComboBox->currentNode()->GetID());
  
  // Define VOI

  // Miscellaneous
  p->mrmlManager()->SetEnableMultithreading(this->MultithreadingCheckBox->isChecked());
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStepPrivate::display2DVOI(bool show)
{

}

namespace
{
//------------------------------------------------------------------------------
struct vtkMRMLROINodeInitializer : public vtkMRMLNodeInitializer
{
  vtkMRMLROINodeInitializer(const QString& nodeName):NodeName(nodeName){}
  virtual void operator()(vtkMRMLNode* node)const
    {
    vtkMRMLROINode * roiNode = vtkMRMLROINode::SafeDownCast(node);
    Q_ASSERT(roiNode);
    roiNode->SetName(this->NodeName.toLatin1());
    roiNode->SetVisibility(0);
    }
  QString NodeName;
};
}

//-----------------------------------------------------------------------------
vtkMRMLROINode* qSlicerEMSegmentRunSegmentationStepPrivate::createROINode()
{
  CTK_P(qSlicerEMSegmentRunSegmentationStep);

  QString roiNodeName("SegmentationROI");

  // Look up existing ROI nodes
  vtkMRMLScene* scene = p->mrmlScene();
  int numNodes = scene->GetNumberOfNodesByClass("vtkMRMLROINode");

  // Remove any ROI nodes named "SegmentationROI"
  for (int i = 0; i < numNodes; ++i)
    {
    vtkMRMLNode* node = scene->GetNthNodeByClass(i, "vtkMRMLROINode");
    if (node && node->GetName() && !roiNodeName.compare(node->GetName()))
      {
      scene->RemoveNode(node);
      }
    }

  // Create a new ROI
  return vtkMRMLROINode::SafeDownCast(qMRMLNodeFactory::createNode(
      scene, "vtkMRMLROINode", vtkMRMLROINodeInitializer(roiNodeName)));
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStepPrivate::updateMRMLROINodeUsingInputVolume()
{
  CTK_P(qSlicerEMSegmentRunSegmentationStep);

  Q_ASSERT(this->ROINode);

  double ROIMinIJK[4], ROIMaxIJK[4], ROIMinRAS[4], ROIMaxRAS[4];
  double radius[3], center[3];

  vtkMRMLVolumeNode* volumeNode =
      p->mrmlManager()->GetWorkingDataNode()->GetInputTargetNode()->GetNthVolumeNode(0);
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
    ctkWorkflow* newWorkflow, QWidget* newWidget) : Superclass(newWorkflow, Self::StepId, newWidget)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentRunSegmentationStep);
  CTK_D(qSlicerEMSegmentRunSegmentationStep);
  d->setupUi(this);

  this->setName("9/9. Run Segmentation");
  this->setDescription("Apply EM algorithm to segment target image.");
  this->setButtonBoxHints(ctkWorkflowWidgetStep::NextButtonDisabled);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStep::createUserInterface()
{
  this->createUserInterfaceComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStep::showUserInterface()
{
  CTK_D(qSlicerEMSegmentRunSegmentationStep);
  this->Superclass::showUserInterface();
  d->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  this->validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  Q_UNUSED(comingFrom);
  Q_UNUSED(transitionType);

  CTK_D(qSlicerEMSegmentRunSegmentationStep);

  d->updateWidgetFromMRML();

  // Create ROI MRML node
  if (!d->ROINode)
    {
    d->ROINode = d->createROINode();
    }
  d->setMRMLROINode(d->ROINode);

  // Update the roiNode ROI to reflect what is stored in ROI MRML
  // SegmentationBoundary sets ROI Node
  d->updateMRMLROINodeUsingInputVolume();

  // TODO the node has to be connected to the ROIMRMLCallback

  // Indicates that we are finished
  this->onEntryComplete();
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
    // Indicates that we are finished
    this->onExitComplete();
    return;
    }

  d->updateMRMLFromWidget(); 

  // Indicates that we are finished
  this->onExitComplete();
}

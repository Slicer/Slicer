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
#include "vtkEMSegmentLogic.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>
#include <vtkMRMLEMSWorkingDataNode.h>
#include <vtkMRMLEMSTargetNode.h>
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLAnnotationROINode.h>

// VTKMRML includes
#include "vtkMRMLSliceLogic.h"

//--------------------------------------------------------------------------
static ctkLogger logger(
    "org.slicer.qtmodules.emsegment.workflow.qSlicerEMSegmentRunSegmentationStep");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// qSlicerEMSegmentRunSegmentationStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentRunSegmentationStepPrivate::qSlicerEMSegmentRunSegmentationStepPrivate(qSlicerEMSegmentRunSegmentationStep& object)
  : q_ptr(&object)
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
void qSlicerEMSegmentRunSegmentationStepPrivate::setMRMLROINode(vtkMRMLAnnotationROINode* node)
{
  Q_Q(qSlicerEMSegmentRunSegmentationStep);

  Q_ASSERT(node);
  Q_ASSERT(q->mrmlManager());
  Q_ASSERT(this->ROIWidget);

  // Finish setting up the ROI widget, now that we have a MRML manager
  vtkMRMLVolumeNode* volumeNode =
      q->mrmlManager()->GetWorkingDataNode()->GetInputTargetNode()->GetNthVolumeNode(0);

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
  Q_Q(qSlicerEMSegmentRunSegmentationStep);

  if (!q->mrmlManager())
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
    w->setEnabled(q->mrmlManager()->HasGlobalParametersNode());
    }

  // Working directory
  this->SaveIntermediateResultsCheckBox->setChecked(
      q->mrmlManager()->GetSaveIntermediateResults());

  if (q->mrmlManager()->GetSaveWorkingDirectory())
    {
    this->DirectoryButton->setDirectory(
        q->mrmlManager()->GetSaveWorkingDirectory());
    }
  else
    {
    this->DirectoryButton->setCaption(tr("Select Working Directory"));
    }

  // Output label map
  if (!q->mrmlManager()->GetOutputVolumeMRMLID())
    {
    qMRMLNodeFactory::AttributeType attributes;
    attributes.insert("LabelMap", "1");
    qMRMLNodeFactory::createNode(
        q->mrmlScene(), "vtkMRMLScalarVolumeNode", attributes);
    }

  this->OutputLabelMapComboBox->setCurrentNode(q->mrmlManager()->GetOutputVolumeMRMLID());

  // Define VOI
  // TODO: should call SetMRMLROINode above? Incase the working volume has changed?

  // Miscellaneous
  this->MultithreadingCheckBox->setChecked(q->mrmlManager()->GetEnableMultithreading());
  
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStepPrivate::updateMRMLFromWidget()
{
  Q_Q(qSlicerEMSegmentRunSegmentationStep);

  Q_ASSERT(q->mrmlManager());

  // Working directory
  q->mrmlManager()->SetSaveIntermediateResults(this->SaveIntermediateResultsCheckBox->isChecked());
  q->mrmlManager()->SetSaveWorkingDirectory(this->DirectoryButton->directory().toLatin1());

  // Output label map
  q->mrmlManager()->SetOutputVolumeMRMLID(this->OutputLabelMapComboBox->currentNode()->GetID());
  
  // Define VOI

  // Miscellaneous
  q->mrmlManager()->SetEnableMultithreading(this->MultithreadingCheckBox->isChecked());
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStepPrivate::display2DVOI(bool show)
{
  Q_UNUSED(show);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStepPrivate::initializeNode(vtkMRMLNode* node)
{
  Q_ASSERT(this->sender());
  QString nodeName = this->sender()->property("NodeName").toString();
  vtkMRMLAnnotationROINode * roiNode = vtkMRMLAnnotationROINode::SafeDownCast(node);
  Q_ASSERT(roiNode);
  roiNode->SetName(nodeName.toLatin1());
  roiNode->SetVisibility(0);
}

//-----------------------------------------------------------------------------
vtkMRMLAnnotationROINode* qSlicerEMSegmentRunSegmentationStepPrivate::createROINode()
{
  Q_Q(qSlicerEMSegmentRunSegmentationStep);

  QString roiNodeName("SegmentationROI");

  // Look up existing ROI nodes
  vtkMRMLScene* scene = q->mrmlScene();
  int numNodes = scene->GetNumberOfNodesByClass("vtkMRMLAnnotationROINode");

  // Remove any ROI nodes named "SegmentationROI"
  for (int i = 0; i < numNodes; ++i)
    {
    vtkMRMLNode* node = scene->GetNthNodeByClass(i, "vtkMRMLAnnotationROINode");
    if (node && node->GetName() && !roiNodeName.compare(node->GetName()))
      {
      scene->RemoveNode(node);
      }
    }

  // Create a new ROI
  qMRMLNodeFactory nodeFactory;
  nodeFactory.setMRMLScene(scene);
  nodeFactory.setProperty("NodeName", roiNodeName);
  connect(&nodeFactory, SIGNAL(nodeInitialized(vtkMRMLNode*)),
         this, SLOT(initializeNode(vtkMRMLNode*)));
  return vtkMRMLAnnotationROINode::SafeDownCast(nodeFactory.createNode("vtkMRMLAnnotationROINode"));
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStepPrivate::updateMRMLROINodeUsingInputVolume()
{
  Q_Q(qSlicerEMSegmentRunSegmentationStep);

  Q_ASSERT(this->ROINode);

  double ROIMinIJK[4], ROIMaxIJK[4], ROIMinRAS[4], ROIMaxRAS[4];
  double radius[3], center[3];

  vtkMRMLVolumeNode* volumeNode =
      q->mrmlManager()->GetWorkingDataNode()->GetInputTargetNode()->GetNthVolumeNode(0);
  if (!volumeNode)
    {
    return;
    }

  q->emSegmentLogic()->DefineValidSegmentationBoundary();
  int minPoint[3];
  int maxPoint[3];
  q->mrmlManager()->GetSegmentationBoundaryMin(minPoint);
  q->mrmlManager()->GetSegmentationBoundaryMax(maxPoint);

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
  ctkWorkflow* newWorkflow, QWidget* newWidget)
  : Superclass(newWorkflow, qSlicerEMSegmentRunSegmentationStep::StepId, newWidget)
  , d_ptr(new qSlicerEMSegmentRunSegmentationStepPrivate(*this))
{
  Q_D(qSlicerEMSegmentRunSegmentationStep);
  d->setupUi(this);

  this->setName("9/9. Run Segmentation");
  this->setDescription("Apply EM algorithm to segment target image.");
  this->setButtonBoxHints(ctkWorkflowWidgetStep::NextButtonDisabled);
}

//-----------------------------------------------------------------------------
qSlicerEMSegmentRunSegmentationStep::~qSlicerEMSegmentRunSegmentationStep()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStep::createUserInterface()
{
  this->createUserInterfaceComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentRunSegmentationStep::showUserInterface()
{
  Q_D(qSlicerEMSegmentRunSegmentationStep);
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

  Q_D(qSlicerEMSegmentRunSegmentationStep);

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

  Q_D(qSlicerEMSegmentRunSegmentationStep);

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

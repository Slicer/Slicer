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
#include <QDialog>
#include <QVBoxLayout>

// CTK includes
#include <ctkLogger.h>

// EMSegment includes
#include "qSlicerEMSegmentSpecifyIntensityDistributionStep.h"
#include "ui_qSlicerEMSegmentSpecifyIntensityDistributionStep.h"
#include "qSlicerEMSegmentSpecifyIntensityDistributionStep_p.h"


// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>

// MRML includes
#include <vtkMRMLEMSTreeNode.h>

//--------------------------------------------------------------------------
static ctkLogger logger(
    "org.slicer.qtmodules.emsegment.workflow.qSlicerEMSegmentSpecifyIntensityDistributionStep");
//--------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate::qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate(qSlicerEMSegmentSpecifyIntensityDistributionStep& object)
  : q_ptr(&object)
{
  this->CurrentTreeNodeId = 0;
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate::setupUi(qSlicerEMSegmentWorkflowWidgetStep* step)
{
  Q_Q(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  this->Ui_qSlicerEMSegmentSpecifyIntensityDistributionStep::setupUi(step);

  this->DistributionSpecificationMethodComboBox->addItem(
      q->tr("Manual"),
      QVariant(vtkEMSegmentMRMLManager::DistributionSpecificationManual));

//  this->DistributionSpecificationMethodComboBox->addItem(
//      q->tr("Manual Sampling"),
//      QVariant(vtkEMSegmentMRMLManager::DistributionSpecificationManuallySample));

//  this->DistributionSpecificationMethodComboBox->addItem(
//      q->tr("Auto Sampling"),
//      QVariant(vtkEMSegmentMRMLManager::DistributionSpecificationAutoSample));

  QObject::connect(this->PlotDistributionButton, SIGNAL(clicked()),
                   this, SLOT(openGraphDialog()));

  QObject::connect(this->EMSegmentAnatomicalTreeWidget,
                   SIGNAL(currentTreeNodeChanged(vtkMRMLNode*)),
                   this, SLOT(onCurrentTreeNodeChanged(vtkMRMLNode*)));

  QObject::connect(this->DistributionSpecificationMethodComboBox,
                   SIGNAL(currentIndexChanged(int)),
                   this, SLOT(onCurrentDistributionSpecificationMethodComboBoxIndexChanged(int)));
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate::
    updateMeanAndCovarianceMatrixWidget(int distribution)
{
  this->MeanMatrixWidget->setEditable(
      distribution == vtkEMSegmentMRMLManager::DistributionSpecificationManual);

  this->CovarianceMatrixWidget->setEditable(
      distribution == vtkEMSegmentMRMLManager::DistributionSpecificationManual);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate::updateWidgetFromMRML()
{
  this->EMSegmentAnatomicalTreeWidget->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate::openGraphDialog()
{
  Q_Q(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  QDialog dialog(q);

  dialog.exec();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate::onCurrentTreeNodeChanged(
    vtkMRMLNode* node)
{
  Q_Q(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  Q_ASSERT(q->mrmlManager());

  vtkMRMLEMSTreeNode * treeNode = vtkMRMLEMSTreeNode::SafeDownCast(node);
  Q_ASSERT(treeNode);

  bool isLeaf = treeNode->GetNumberOfChildNodes() == 0;

  this->IntensityDistributionTab->setEnabled(isLeaf);

  int targetVolumeCount = q->mrmlManager()->GetTargetNumberOfSelectedVolumes();

  // MeanMatrix
  this->MeanMatrixWidget->setColumnCount(targetVolumeCount);
  QVector<double> meanValues;

  // CovarianceMatrix
  this->CovarianceMatrixWidget->setRowCount(targetVolumeCount);
  this->CovarianceMatrixWidget->setColumnCount(targetVolumeCount);
  QVector<double> covarianceValues;

  vtkIdType treeNodeId = 0;

  if (isLeaf)
    {
    treeNodeId = q->mrmlManager()->MapMRMLNodeIDToVTKNodeID(treeNode->GetID());

    int distribution =
        q->mrmlManager()->GetTreeNodeDistributionSpecificationMethod(treeNodeId);
    Q_ASSERT(
        distribution == vtkEMSegmentMRMLManager::DistributionSpecificationManuallySample ||
        distribution == vtkEMSegmentMRMLManager::DistributionSpecificationManual ||
        distribution == vtkEMSegmentMRMLManager::DistributionSpecificationAutoSample);

    this->DistributionSpecificationMethodComboBox->setCurrentIndex(
        this->DistributionSpecificationMethodComboBox->findData(QVariant(distribution)));

    this->updateMeanAndCovarianceMatrixWidget(distribution);

    // MeanMatrix
    for(int colId = 0; colId < targetVolumeCount; colId++)
      {
      meanValues << q->mrmlManager()->GetTreeNodeDistributionMeanWithCorrection(treeNodeId, colId);
      }

    // CovarianceMatrix
    for(int rowId = 0; rowId < targetVolumeCount; rowId++)
      {
      for(int colId = 0; colId < targetVolumeCount; colId++)
        {
        covarianceValues << q->mrmlManager()->GetTreeNodeDistributionLogCovarianceWithCorrection(
            treeNodeId, rowId, colId);
        }
      }
    }
  else
    {
    meanValues.fill(0.0, targetVolumeCount);
    covarianceValues.fill(0.0, targetVolumeCount * targetVolumeCount);
    }

  this->MeanMatrixWidget->setVector(meanValues);
  this->CovarianceMatrixWidget->setVector(covarianceValues);

  this->CurrentTreeNodeId = treeNodeId;
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate::
    onCurrentDistributionSpecificationMethodComboBoxIndexChanged(int currentIndex)
{
  Q_Q(qSlicerEMSegmentSpecifyIntensityDistributionStep);

  int distribution = this->DistributionSpecificationMethodComboBox->itemData(currentIndex).toInt();
  Q_ASSERT(
      distribution == vtkEMSegmentMRMLManager::DistributionSpecificationManuallySample ||
      distribution == vtkEMSegmentMRMLManager::DistributionSpecificationManual ||
      distribution == vtkEMSegmentMRMLManager::DistributionSpecificationAutoSample);

  q->mrmlManager()->SetTreeNodeDistributionSpecificationMethod(
      this->CurrentTreeNodeId, distribution);

  this->updateMeanAndCovarianceMatrixWidget(distribution);
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentSpecifyIntensityDistributionStep methods

//-----------------------------------------------------------------------------
const QString qSlicerEMSegmentSpecifyIntensityDistributionStep::StepId =
    "SpecifyIntensityDistribution";

//-----------------------------------------------------------------------------
qSlicerEMSegmentSpecifyIntensityDistributionStep::qSlicerEMSegmentSpecifyIntensityDistributionStep(
  ctkWorkflow* newWorkflow, QWidget* newWidget)
  : Superclass(newWorkflow, qSlicerEMSegmentSpecifyIntensityDistributionStep::StepId, newWidget)
  , d_ptr(new qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate(*this))
{
  Q_D(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  d->setupUi(this);

  this->setName("7/9. Specify Intensity Distributions");
  this->setDescription("Define intensity distribution for each anatomical structure.");
}

//-----------------------------------------------------------------------------
qSlicerEMSegmentSpecifyIntensityDistributionStep::~qSlicerEMSegmentSpecifyIntensityDistributionStep()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::createUserInterface()
{
  this->createUserInterfaceComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  this->validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  Q_UNUSED(comingFrom);
  Q_UNUSED(transitionType);
  Q_D(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  d->updateWidgetFromMRML();

  // Indicates that we are finished
  this->onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  Q_UNUSED(goingTo);
  Q_UNUSED(transitionType);
  // Indicates that we are finished
  this->onExitComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::showUserInterface()
{
  Q_D(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  this->Superclass::showUserInterface();
  d->updateWidgetFromMRML();
}


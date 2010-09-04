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
#include "ui_qSlicerEMSegmentSpecifyIntensityDistributionPanel.h"
#include "qSlicerEMSegmentSpecifyIntensityDistributionStep_p.h"
#include "qSlicerEMSegmentGraphWidget.h"

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
qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate::qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate()
{
  this->CurrentTreeNodeId = 0;
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate::setupUi(qSlicerEMSegmentWorkflowWidgetStep* step)
{
  CTK_P(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  this->Ui_qSlicerEMSegmentSpecifyIntensityDistributionPanel::setupUi(step);

  this->DistributionSpecificationMethodComboBox->addItem(
      p->tr("Manual"),
      QVariant(vtkEMSegmentMRMLManager::DistributionSpecificationManual));

//  this->DistributionSpecificationMethodComboBox->addItem(
//      p->tr("Manual Sampling"),
//      QVariant(vtkEMSegmentMRMLManager::DistributionSpecificationManuallySample));

//  this->DistributionSpecificationMethodComboBox->addItem(
//      p->tr("Auto Sampling"),
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
  CTK_P(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  QDialog dialog(p);
  qSlicerEMSegmentGraphWidget* graph = new qSlicerEMSegmentGraphWidget(&dialog);
  QVBoxLayout* boxLayout = new QVBoxLayout;
  boxLayout->addWidget(graph);
  dialog.setLayout(boxLayout);
  graph->setMRMLManager(p->mrmlManager());
  dialog.exec();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStepPrivate::onCurrentTreeNodeChanged(
    vtkMRMLNode* node)
{
  CTK_P(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  Q_ASSERT(p->mrmlManager());

  vtkMRMLEMSTreeNode * treeNode = vtkMRMLEMSTreeNode::SafeDownCast(node);
  Q_ASSERT(treeNode);

  bool isLeaf = treeNode->GetNumberOfChildNodes() == 0;

  this->IntensityDistributionTab->setEnabled(isLeaf);

  int targetVolumeCount = p->mrmlManager()->GetTargetNumberOfSelectedVolumes();

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
    treeNodeId = p->mrmlManager()->MapMRMLNodeIDToVTKNodeID(treeNode->GetID());

    int distribution =
        p->mrmlManager()->GetTreeNodeDistributionSpecificationMethod(treeNodeId);
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
      meanValues << p->mrmlManager()->GetTreeNodeDistributionMean(treeNodeId, colId);
      }

    // CovarianceMatrix
    for(int rowId = 0; rowId < targetVolumeCount; rowId++)
      {
      for(int colId = 0; colId < targetVolumeCount; colId++)
        {
        covarianceValues << p->mrmlManager()->GetTreeNodeDistributionCovariance(
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
  CTK_P(qSlicerEMSegmentSpecifyIntensityDistributionStep);

  int distribution = this->DistributionSpecificationMethodComboBox->itemData(currentIndex).toInt();
  Q_ASSERT(
      distribution == vtkEMSegmentMRMLManager::DistributionSpecificationManuallySample ||
      distribution == vtkEMSegmentMRMLManager::DistributionSpecificationManual ||
      distribution == vtkEMSegmentMRMLManager::DistributionSpecificationAutoSample);

  p->mrmlManager()->SetTreeNodeDistributionSpecificationMethod(
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
ctkWorkflow* newWorkflow, QWidget* newWidget) : Superclass(newWorkflow, Self::StepId, newWidget)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  CTK_D(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  d->setupUi(this);

  this->setName("7/9. Specify Intensity Distributions");
  this->setDescription("Define intensity distribution for each anatomical structure.");
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::createUserInterface()
{
  emit createUserInterfaceComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::validate(const QString& desiredBranchId)
{
  Q_UNUSED(desiredBranchId);

  emit validationComplete(true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::onEntry(
    const ctkWorkflowStep* comingFrom,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  CTK_D(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  d->updateWidgetFromMRML();

  // Signals that we are finished
  emit onEntryComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::onExit(
    const ctkWorkflowStep* goingTo,
    const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType)
{
  // Signals that we are finished
  emit onExitComplete();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionStep::showUserInterface()
{
  CTK_D(qSlicerEMSegmentSpecifyIntensityDistributionStep);
  this->Superclass::showUserInterface();
  d->updateWidgetFromMRML();
}


// Qt includes
#include <QDebug>
#include <QDialog>
#include <QVBoxLayout>

// CTK includes
#include <ctkLogger.h>

// EMSegment includes
#include "qSlicerEMSegmentSpecifyIntensityDistributionPanel.h" 
#include "ui_qSlicerEMSegmentSpecifyIntensityDistributionPanel.h"
#include "qSlicerEMSegmentGraphWidget.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>

// MRML includes
#include <vtkMRMLEMSTreeNode.h>

//--------------------------------------------------------------------------
static ctkLogger logger(
    "org.slicer.qtmodules.emsegment.panels.qSlicerEMSegmentSpecifyIntensityDistributionPanel");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class qSlicerEMSegmentSpecifyIntensityDistributionPanelPrivate :
    public ctkPrivate<qSlicerEMSegmentSpecifyIntensityDistributionPanel>,
    public Ui_qSlicerEMSegmentSpecifyIntensityDistributionPanel
{
public:
  qSlicerEMSegmentSpecifyIntensityDistributionPanelPrivate()
    {
    this->CurrentTreeNodeId = 0;
    }
  void setupUi(qSlicerEMSegmentWidget* widget);

  void updateMeanAndCovarianceMatrixWidget(int distribution);

  vtkIdType CurrentTreeNodeId;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentSpecifyIntensityDistributionPanelPrivate methods

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionPanelPrivate::setupUi(qSlicerEMSegmentWidget* widget)
{
  CTK_P(qSlicerEMSegmentSpecifyIntensityDistributionPanel);
  this->Ui_qSlicerEMSegmentSpecifyIntensityDistributionPanel::setupUi(widget);

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
                   p, SLOT(openGraphDialog()));

  QObject::connect(this->EMSegmentAnatomicalTreeWidget,
                   SIGNAL(currentTreeNodeChanged(vtkMRMLNode*)),
                   p, SLOT(onCurrentTreeNodeChanged(vtkMRMLNode*)));

  QObject::connect(this->DistributionSpecificationMethodComboBox,
                   SIGNAL(currentIndexChanged(int)),
                   p, SLOT(onCurrentDistributionSpecificationMethodCombBoxIndexChanged(int)));
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionPanelPrivate::
    updateMeanAndCovarianceMatrixWidget(int distribution)
{
  this->MeanMatrixWidget->setEditable(
      distribution == vtkEMSegmentMRMLManager::DistributionSpecificationManual);

  this->CovarianceMatrixWidget->setEditable(
      distribution == vtkEMSegmentMRMLManager::DistributionSpecificationManual);
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentSpecifyIntensityDistributionPanel methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentSpecifyIntensityDistributionPanel::qSlicerEMSegmentSpecifyIntensityDistributionPanel(QWidget *newParent):
Superclass(newParent)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentSpecifyIntensityDistributionPanel);
  CTK_D(qSlicerEMSegmentSpecifyIntensityDistributionPanel);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionPanel::updateWidgetFromMRML()
{
  CTK_D(qSlicerEMSegmentSpecifyIntensityDistributionPanel);
  d->EMSegmentAnatomicalTreeWidget->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionPanel::openGraphDialog()
{
  QDialog dialog(this);
  qSlicerEMSegmentGraphWidget* graph = new qSlicerEMSegmentGraphWidget(&dialog);
  QVBoxLayout* boxLayout = new QVBoxLayout;
  boxLayout->addWidget(graph);
  dialog.setLayout(boxLayout);
  graph->setMRMLManager(this->mrmlManager());
  dialog.exec();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionPanel::onCurrentTreeNodeChanged(
    vtkMRMLNode* node)
{
  CTK_D(qSlicerEMSegmentSpecifyIntensityDistributionPanel);
  Q_ASSERT(this->mrmlManager());

  vtkMRMLEMSTreeNode * treeNode = vtkMRMLEMSTreeNode::SafeDownCast(node);
  Q_ASSERT(treeNode);

  bool isLeaf = treeNode->GetNumberOfChildNodes() == 0;

  d->IntensityDistributionTab->setEnabled(isLeaf);

  int targetVolumeCount = this->mrmlManager()->GetTargetNumberOfSelectedVolumes();

  // MeanMatrix
  d->MeanMatrixWidget->setColumnCount(targetVolumeCount);
  QVector<double> meanValues;

  // CovarianceMatrix
  d->CovarianceMatrixWidget->setRowCount(targetVolumeCount);
  d->CovarianceMatrixWidget->setColumnCount(targetVolumeCount);
  QVector<double> covarianceValues;

  vtkIdType treeNodeId = 0;

  if (isLeaf)
    {
    treeNodeId = this->mrmlManager()->MapMRMLNodeIDToVTKNodeID(treeNode->GetID());

    int distribution =
        this->mrmlManager()->GetTreeNodeDistributionSpecificationMethod(treeNodeId);
    Q_ASSERT(
        distribution == vtkEMSegmentMRMLManager::DistributionSpecificationManuallySample ||
        distribution == vtkEMSegmentMRMLManager::DistributionSpecificationManual ||
        distribution == vtkEMSegmentMRMLManager::DistributionSpecificationAutoSample);

    d->DistributionSpecificationMethodComboBox->setCurrentIndex(
        d->DistributionSpecificationMethodComboBox->findData(QVariant(distribution)));

    d->updateMeanAndCovarianceMatrixWidget(distribution);

    // MeanMatrix
    for(int colId = 0; colId < targetVolumeCount; colId++)
      {
      meanValues << this->mrmlManager()->GetTreeNodeDistributionMean(treeNodeId, colId);
      }

    // CovarianceMatrix
    for(int rowId = 0; rowId < targetVolumeCount; rowId++)
      {
      for(int colId = 0; colId < targetVolumeCount; colId++)
        {
        covarianceValues << this->mrmlManager()->GetTreeNodeDistributionCovariance(
            treeNodeId, rowId, colId);
        }
      }
    }
  else
    {
    meanValues.fill(0.0, targetVolumeCount);
    covarianceValues.fill(0.0, targetVolumeCount * targetVolumeCount);
    }

  d->MeanMatrixWidget->setVector(meanValues);
  d->CovarianceMatrixWidget->setVector(covarianceValues);

  d->CurrentTreeNodeId = treeNodeId;
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentSpecifyIntensityDistributionPanel::
    onCurrentDistributionSpecificationMethodCombBoxIndexChanged(int currentIndex)
{
  CTK_D(qSlicerEMSegmentSpecifyIntensityDistributionPanel);

  int distribution = d->DistributionSpecificationMethodComboBox->itemData(currentIndex).toInt();
  Q_ASSERT(
      distribution == vtkEMSegmentMRMLManager::DistributionSpecificationManuallySample ||
      distribution == vtkEMSegmentMRMLManager::DistributionSpecificationManual ||
      distribution == vtkEMSegmentMRMLManager::DistributionSpecificationAutoSample);

  this->mrmlManager()->SetTreeNodeDistributionSpecificationMethod(
      d->CurrentTreeNodeId, distribution);

  d->updateMeanAndCovarianceMatrixWidget(distribution);
}

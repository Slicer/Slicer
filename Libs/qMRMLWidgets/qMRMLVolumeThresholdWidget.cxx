
// Qt includes
#include <QDebug>
#include <QStack>

// qMRML includes
#include "qMRMLUtils.h"
#include "qMRMLVolumeThresholdWidget.h"
#include "ui_qMRMLVolumeThresholdWidget.h"

// MRML includes
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"

// VTK includes
#include "vtkSmartPointer.h"


//-----------------------------------------------------------------------------
class qMRMLVolumeThresholdWidgetPrivate: public Ui_qMRMLVolumeThresholdWidget
{
public:
  qMRMLVolumeThresholdWidgetPrivate();

  vtkMRMLScalarVolumeNode*        VolumeNode;
  vtkMRMLScalarVolumeDisplayNode* VolumeDisplayNode;
};

// --------------------------------------------------------------------------
qMRMLVolumeThresholdWidgetPrivate::qMRMLVolumeThresholdWidgetPrivate()
{
  this->VolumeNode = 0;
  this->VolumeDisplayNode = 0;
}

// --------------------------------------------------------------------------
qMRMLVolumeThresholdWidget::qMRMLVolumeThresholdWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qMRMLVolumeThresholdWidgetPrivate)
{
  Q_D(qMRMLVolumeThresholdWidget);

  d->setupUi(this);

  this->setAutoThreshold(2);

  this->connect(d->VolumeThresholdRangeWidget, SIGNAL(valuesChanged(double, double)),
                SLOT(setThreshold(double, double)));

  this->connect(d->AutoManualComboBox, SIGNAL(currentIndexChanged(int)),
                SLOT(setAutoThreshold(int)));

  // disable as there is not MRML Node associated with the widget
  this->setEnabled(d->VolumeDisplayNode != NULL);
}

// --------------------------------------------------------------------------
qMRMLVolumeThresholdWidget::~qMRMLVolumeThresholdWidget()
{
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setAutoThreshold(int autoThreshold)
{
  // 0-manual, 1-auto, 2-off
  Q_D(qMRMLVolumeThresholdWidget);

  if (d->VolumeDisplayNode)
  {
    int oldAuto = d->VolumeDisplayNode->GetAutoThreshold();
    int oldApply = d->VolumeDisplayNode->GetApplyThreshold();

    int disabledModify = d->VolumeDisplayNode->StartModify();
    if (autoThreshold != 2)
      {
      d->VolumeDisplayNode->SetApplyThreshold(1);
      d->VolumeDisplayNode->SetAutoThreshold(autoThreshold);
      }
    else
      {
      d->VolumeDisplayNode->SetApplyThreshold(0);
      }
    d->VolumeDisplayNode->EndModify(disabledModify);

    if (oldAuto != d->VolumeDisplayNode->GetAutoThreshold() ||
        oldApply != d->VolumeDisplayNode->GetApplyThreshold())
      {
      emit this->autoThresholdValueChanged(autoThreshold);
      }

  }
}

// --------------------------------------------------------------------------
int qMRMLVolumeThresholdWidget::autoThreshold() const
{
  Q_D(const qMRMLVolumeThresholdWidget);

  // Assumes settings of the sliders are all the same
  return d->AutoManualComboBox->currentIndex();
}

// --------------------------------------------------------------------------
bool qMRMLVolumeThresholdWidget::isOff() const
{
  Q_D(const qMRMLVolumeThresholdWidget);
  return d->AutoManualComboBox->currentIndex() == 2;
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setThreshold(double lowerThreshold, double upperThreshold)
{
  Q_D(qMRMLVolumeThresholdWidget);
  if (d->VolumeDisplayNode)
    {
    double oldLowerThreshold = d->VolumeDisplayNode->GetLowerThreshold();
    double oldUpperThreshold  = d->VolumeDisplayNode->GetUpperThreshold();

    int disabledModify = d->VolumeDisplayNode->StartModify();
    d->VolumeDisplayNode->SetLowerThreshold(lowerThreshold);
    d->VolumeDisplayNode->SetUpperThreshold(upperThreshold);
    if (d->VolumeDisplayNode->GetApplyThreshold() &&
        (oldLowerThreshold != d->VolumeDisplayNode->GetLowerThreshold() ||
         oldUpperThreshold != d->VolumeDisplayNode->GetUpperThreshold()) )
      {
      this->setAutoThreshold(0);
      emit this->thresholdValuesChanged(lowerThreshold, upperThreshold);
      }
    d->VolumeDisplayNode->EndModify(disabledModify);
    }
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setLowerThreshold(double lowerThreshold)
{
  Q_D(qMRMLVolumeThresholdWidget);
  if (d->VolumeDisplayNode)
    {
    double upperThreshold  = d->VolumeDisplayNode->GetUpperThreshold();
    this->setThreshold(lowerThreshold, upperThreshold);
    }
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setUpperThreshold(double upperThreshold)
{
  Q_D(qMRMLVolumeThresholdWidget);
  if (d->VolumeDisplayNode)
  {
    double lowerThreshold = d->VolumeDisplayNode->GetLowerThreshold();
    this->setThreshold(lowerThreshold, upperThreshold);
  }
}

// --------------------------------------------------------------------------
double qMRMLVolumeThresholdWidget::lowerThreshold() const
{
  Q_D(const qMRMLVolumeThresholdWidget);

  double min = d->VolumeThresholdRangeWidget->minimumValue();

  return min;
}

// --------------------------------------------------------------------------
double qMRMLVolumeThresholdWidget::upperThreshold() const
{
  Q_D(const qMRMLVolumeThresholdWidget);

  double max = d->VolumeThresholdRangeWidget->maximumValue();

  return max;
}

// --------------------------------------------------------------------------
vtkMRMLScalarVolumeDisplayNode* qMRMLVolumeThresholdWidget::mrmlDisplayNode()const
{
  Q_D(const qMRMLVolumeThresholdWidget);
  return d->VolumeDisplayNode;
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setMRMLVolumeDisplayNode(vtkMRMLScalarVolumeDisplayNode* node)
{
  Q_D(qMRMLVolumeThresholdWidget);
  if (d->VolumeDisplayNode == node)
    {
    return;
    }

  // each time the node is modified, the qt widgets are updated
  this->qvtkReconnect(d->VolumeDisplayNode, node, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromMRML()));

  d->VolumeDisplayNode = node;

  this->setEnabled(node != 0);
  
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* qMRMLVolumeThresholdWidget::mrmlVolumeNode()const
{
  Q_D(const qMRMLVolumeThresholdWidget);
  return d->VolumeNode;
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setMRMLVolumeNode(vtkMRMLNode* node)
{
  this->setMRMLVolumeNode(vtkMRMLScalarVolumeNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setMRMLVolumeNode(vtkMRMLScalarVolumeNode* volumeNode)
{
  Q_D(qMRMLVolumeThresholdWidget);
  if (d->VolumeNode == volumeNode)
    {
    return;
    }
  // each time the node is modified, the qt widgets are updated
  this->qvtkReconnect(d->VolumeNode, volumeNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateDisplayNode()));
  d->VolumeNode = volumeNode;
  this->updateDisplayNode();
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setMinimum(double min)
{
  Q_D(qMRMLVolumeThresholdWidget);
  d->VolumeThresholdRangeWidget->setMinimum(min);
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setMaximum(double max)
{
  Q_D(qMRMLVolumeThresholdWidget);
  d->VolumeThresholdRangeWidget->setMaximum(max);
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::updateDisplayNode()
{
  Q_D(qMRMLVolumeThresholdWidget);
  this->setMRMLVolumeDisplayNode(
    d->VolumeNode ? vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
      d->VolumeNode->GetVolumeDisplayNode()) : 0);
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLVolumeThresholdWidget);

  if (!d->VolumeDisplayNode)
    {
    return;
    }
  const int autoThresh = d->VolumeDisplayNode->GetAutoThreshold();
  const int applyThresh = d->VolumeDisplayNode->GetApplyThreshold();
  // 0 = manual, 1 = auto, 2 = off
  int index = (applyThresh == 0) ? 2 : autoThresh;
  d->AutoManualComboBox->setCurrentIndex(index);

  if (d->VolumeNode && d->VolumeNode->GetImageData())
    {
    double range[2];
    if (d->VolumeDisplayNode && d->VolumeDisplayNode->GetInputImageData())
      {
      d->VolumeDisplayNode->GetDisplayScalarRange(range);
      }
    else
      {
      d->VolumeNode->GetImageData()->GetScalarRange(range);
      }
    d->VolumeThresholdRangeWidget->setRange(range[0], range[1]);
    }

  const double min = d->VolumeDisplayNode->GetLowerThreshold();
  const double max = d->VolumeDisplayNode->GetUpperThreshold();
  d->VolumeThresholdRangeWidget->setValues(min, max );
}

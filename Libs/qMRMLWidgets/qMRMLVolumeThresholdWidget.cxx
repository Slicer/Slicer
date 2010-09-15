
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
  qMRMLVolumeThresholdWidgetPrivate()
    {
    }

};

// --------------------------------------------------------------------------
qMRMLVolumeThresholdWidget::qMRMLVolumeThresholdWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qMRMLVolumeThresholdWidgetPrivate)
{
  Q_D(qMRMLVolumeThresholdWidget);

  d->setupUi(this);

  this->VolumeNode = NULL;
  this->VolumeDisplayNode = NULL;

  this->setAutoThreshold(2);

  this->connect(d->VolumeThresholdRangeWidget, SIGNAL(valuesChanged(double, double)),
                SLOT(setThreshold(double, double)));

  this->connect(d->AutoManualComboBox, SIGNAL(currentIndexChanged(int)),
                SLOT(setAutoThreshold(int)));

  // disable as there is not MRML Node associated with the widget
  this->setEnabled(this->VolumeDisplayNode != NULL);
}

// --------------------------------------------------------------------------
qMRMLVolumeThresholdWidget::~qMRMLVolumeThresholdWidget()
{
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setAutoThreshold(int autoThreshold)
{
  // 0-manual, 1-auto, 2-off
  //Q_D(qMRMLVolumeThresholdWidget);

  if (this->VolumeDisplayNode)
  {
    int oldAuto = this->VolumeDisplayNode->GetAutoThreshold();
    int oldApply = this->VolumeDisplayNode->GetApplyThreshold();

    int disabledModify = this->VolumeDisplayNode->StartModify();
    if (autoThreshold != 2)
      {
      this->VolumeDisplayNode->SetApplyThreshold(1);
      this->VolumeDisplayNode->SetAutoThreshold(autoThreshold);
      }
    else
      {
      this->VolumeDisplayNode->SetApplyThreshold(0);
      }
    this->VolumeDisplayNode->EndModify(disabledModify);

    if (oldAuto != this->VolumeDisplayNode->GetAutoThreshold() ||
        oldApply != this->VolumeDisplayNode->GetApplyThreshold())
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
  if (this->VolumeDisplayNode)
    {
    double oldLowerThreshold = this->VolumeDisplayNode->GetLowerThreshold();
    double oldUpperThreshold  = this->VolumeDisplayNode->GetUpperThreshold();

    int disabledModify = this->VolumeDisplayNode->StartModify();
    this->VolumeDisplayNode->SetLowerThreshold(lowerThreshold);
    this->VolumeDisplayNode->SetUpperThreshold(upperThreshold);
    if (this->VolumeDisplayNode->GetApplyThreshold() &&
        (oldLowerThreshold != this->VolumeDisplayNode->GetLowerThreshold() ||
         oldUpperThreshold != this->VolumeDisplayNode->GetUpperThreshold()) )
      {
      this->setAutoThreshold(0);
      emit this->thresholdValuesChanged(lowerThreshold, upperThreshold);
      }
    this->VolumeDisplayNode->EndModify(disabledModify);
    }
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setLowerThreshold(double lowerThreshold)
{
  if (this->VolumeDisplayNode)
    {
    double upperThreshold  = this->VolumeDisplayNode->GetUpperThreshold();
    this->setThreshold(lowerThreshold, upperThreshold);
    }
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setUpperThreshold(double upperThreshold)
{
  if (this->VolumeDisplayNode)
  {
    double lowerThreshold = this->VolumeDisplayNode->GetLowerThreshold();
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
void qMRMLVolumeThresholdWidget::setMRMLVolumeDisplayNode(vtkMRMLScalarVolumeDisplayNode* node)
{
  // each time the node is modified, the qt widgets are updated
  this->qvtkReconnect(this->VolumeDisplayNode, node, 
                       vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));

  this->VolumeDisplayNode = node;

  this->setEnabled(node != 0);
  
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setMRMLVolumeNode(vtkMRMLNode* node)
{
  this->setMRMLVolumeNode(vtkMRMLScalarVolumeNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setMRMLVolumeNode(vtkMRMLScalarVolumeNode* volumeNode)
{
  this->VolumeNode = volumeNode;
  this->setMRMLVolumeDisplayNode(
    volumeNode ? vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
      volumeNode->GetVolumeDisplayNode()) : 0);
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
void qMRMLVolumeThresholdWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLVolumeThresholdWidget);

  if (this->VolumeDisplayNode)
    {
    int autoThresh = this->VolumeDisplayNode->GetAutoThreshold();
    int applyThresh = this->VolumeDisplayNode->GetApplyThreshold();
    int index = 0;
    if (applyThresh == 0)
      {
      index = 2; // Off
      }
    else
      {
      index = autoThresh; // manual 0; auto 1
      }
    d->AutoManualComboBox->setCurrentIndex(index);

    if (this->VolumeNode)
      {
      double range[2];
      this->VolumeNode->GetImageData()->GetScalarRange(range);
      d->VolumeThresholdRangeWidget->setRange(range[0], range[1]);
      }

    double min = this->VolumeDisplayNode->GetLowerThreshold();
    double max = this->VolumeDisplayNode->GetUpperThreshold();
    d->VolumeThresholdRangeWidget->setValues(min, max );
    }
}


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
class qMRMLVolumeThresholdWidgetPrivate: public ctkPrivate<qMRMLVolumeThresholdWidget>,
                                         public Ui_qMRMLVolumeThresholdWidget
{
public:
  qMRMLVolumeThresholdWidgetPrivate()
    {
    }
    
};

// --------------------------------------------------------------------------
qMRMLVolumeThresholdWidget::qMRMLVolumeThresholdWidget(QWidget* _parent) : Superclass(_parent)
{
  CTK_INIT_PRIVATE(qMRMLVolumeThresholdWidget);
  CTK_D(qMRMLVolumeThresholdWidget);
  
  d->setupUi(this);

  this->VolumeNode = NULL;
  this->VolumeDisplayNode = NULL;

  this->setAutoThreshold(2);

  // TODO replace with double window/level
  this->connect(d->VolumeThresholdRangeWidget, SIGNAL(minimumValueIsChanging(double)),
                SLOT(setLowerThreshold(double)));
  this->connect(d->VolumeThresholdRangeWidget, SIGNAL(maximumValueIsChanging(double)),
                SLOT(setUpperThreshold(double)));

  this->connect(d->AutoManualComboBox, SIGNAL(currentIndexChanged(int)),
                SLOT(setAutoThreshold(int)));

  // disable as there is not MRML Node associated with the widget
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setAutoThreshold(int autoThreshold)
{
  // 0-manual, 1-auto, 2-off
  //CTK_D(qMRMLVolumeThresholdWidget);
  
  if (this->VolumeDisplayNode)
  {
    int oldAuto = this->VolumeDisplayNode->GetAutoThreshold();
    //int oldApply = this->VolumeDisplayNode->GetApplyThreshold();

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

    if (autoThreshold != oldAuto)
    {
       emit this->autoThresholdValueChanged(autoThreshold);
    }

  }
}

// --------------------------------------------------------------------------
int qMRMLVolumeThresholdWidget::autoThreshold() const
{
  CTK_D(const qMRMLVolumeThresholdWidget);
  
  // Assumes settings of the sliders are all the same
  return d->AutoManualComboBox->currentIndex();
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
  CTK_D(const qMRMLVolumeThresholdWidget);

  double min = d->VolumeThresholdRangeWidget->minimumValue();

  return min;
}

// --------------------------------------------------------------------------
double qMRMLVolumeThresholdWidget::upperThreshold() const
{
  CTK_D(const qMRMLVolumeThresholdWidget);

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
  CTK_D(qMRMLVolumeThresholdWidget);
  
  if (volumeNode) 
  {
    double range[2];
    volumeNode->GetImageData()->GetScalarRange(range);
    d->VolumeThresholdRangeWidget->setMinimum(range[0]);
    d->VolumeThresholdRangeWidget->setMaximum(range[1]);
    this->setMRMLVolumeDisplayNode(vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
              volumeNode->GetVolumeDisplayNode()));
  }
  this->VolumeNode = volumeNode;
}



// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setMinimum(double min)
{
  CTK_D(qMRMLVolumeThresholdWidget);
  d->VolumeThresholdRangeWidget->setMinimum(min);
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setMaximum(double max)
{
  CTK_D(qMRMLVolumeThresholdWidget);
  d->VolumeThresholdRangeWidget->setMaximum(max);

}


// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::updateWidgetFromMRML()
{
  CTK_D(qMRMLVolumeThresholdWidget);
  
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

    double min = this->VolumeDisplayNode->GetLowerThreshold();
    double max = this->VolumeDisplayNode->GetUpperThreshold();

    if (this->VolumeNode) 
    {
      double range[2];
      this->VolumeNode->GetImageData()->GetScalarRange(range);
      d->VolumeThresholdRangeWidget->setMinimum(range[0]);
      d->VolumeThresholdRangeWidget->setMaximum(range[1]);
    }
    d->VolumeThresholdRangeWidget->setValues(min, max );
  }

}

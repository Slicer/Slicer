
#include "qMRMLVolumeThresholdWidget.h"
#include "ui_qMRMLVolumeThresholdWidget.h"

// qMRML includes
#include "qMRMLUtils.h"

// MRML includes
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"

// VTK includes
#include "vtkSmartPointer.h"

// QT includes
#include <QDebug>
#include <QStack>


//-----------------------------------------------------------------------------
class qMRMLVolumeThresholdWidgetPrivate: public qCTKPrivate<qMRMLVolumeThresholdWidget>,
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
  QCTK_INIT_PRIVATE(qMRMLVolumeThresholdWidget);
  QCTK_D(qMRMLVolumeThresholdWidget);
  
  d->setupUi(this);

  this->VolumeNode = NULL;
  this->VolumeDisplayNode = NULL;

  this->setAutoThreshold(2);

  // TODO replace with double window/level
  this->connect(d->VolumeThresholdRangeSlider, SIGNAL(valuesChanged(int,int)),
                SLOT(setMinMaxRange(int, int)));
  this->connect(d->AutoManualComboBox, SIGNAL(currentIndexChanged(int)),
                SLOT(setAutoThreshold(int)));

  // disable as there is not MRML Node associated with the widget
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setAutoThreshold(int autoThreshold)
{
  // 0-manual, 1-auto, 2-off
  //QCTK_D(qMRMLVolumeThresholdWidget);
  
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
  QCTK_D(const qMRMLVolumeThresholdWidget);
  
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

// TODO remove when range becomes double
// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setMinMaxRange(int min, int max)
{
  this->setThreshold((double)min, (double)max);
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
  QCTK_D(const qMRMLVolumeThresholdWidget);

  double min = d->VolumeThresholdRangeSlider->minimumValue();

  return min;
}

// --------------------------------------------------------------------------
double qMRMLVolumeThresholdWidget::upperThreshold() const
{
  QCTK_D(const qMRMLVolumeThresholdWidget);

  double max = d->VolumeThresholdRangeSlider->maximumValue();

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
  //QCTK_D(qMRMLVolumeThresholdWidget);
  
  if (volumeNode) 
  {
    // TODO: set image data range in the range widget
    // d->VolumeThresholdRangeSlider  volumeNode->GetImageData()->GetScalarRange()
    this->setMRMLVolumeDisplayNode(vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
              volumeNode->GetVolumeDisplayNode()));
  }
  this->VolumeNode = volumeNode;
}



// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setMinimum(double min)
{
  //QCTK_D(qMRMLVolumeThresholdWidget);
  // TODO set min in the range widget d->VolumeThresholdRangeSlider
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setMaximum(double max)
{
  //QCTK_D(qMRMLVolumeThresholdWidget);
  // TODO set max in the range widget d->VolumeThresholdRangeSlider

}


// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::updateWidgetFromMRML()
{
  QCTK_D(qMRMLVolumeThresholdWidget);
  
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

    //TODO: set correct bounds of the range widget
    // clipping in the range widget causing multiple volume display node updates
    // also a problem if values get clipped, the mode switches from Auto to Manual
    d->VolumeThresholdRangeSlider->setValues(min, max );

    //d->VolumeThresholdRangeSlider->setRangeMinimumPosition(upperThreshold - 0.5 * lowerThreshold);
    //d->VolumeThresholdRangeSlider->setRangeMaximumPosition(upperThreshold + 0.5 * lowerThreshold);
  }

}

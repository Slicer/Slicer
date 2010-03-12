
#include "qMRMLWindowLevelWidget.h"
#include "ui_qMRMLWindowLevelWidget.h"

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
class qMRMLWindowLevelWidgetPrivate: public qCTKPrivate<qMRMLWindowLevelWidget>,
                                     public Ui_qMRMLWindowLevelWidget
{
public:
  qMRMLWindowLevelWidgetPrivate()
    {
    }
    
};

// --------------------------------------------------------------------------
qMRMLWindowLevelWidget::qMRMLWindowLevelWidget(QWidget* _parent) : Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qMRMLWindowLevelWidget);
  QCTK_D(qMRMLWindowLevelWidget);
  
  d->setupUi(this);

  this->VolumeNode = NULL;
  this->VolumeDisplayNode = NULL;

  this->setAutoWindowLevel(1);

  // TODO replace with double window/level
  this->connect(d->WindowLevelRangeSlider, SIGNAL(rangeValuesChanged(int,int)),
                SLOT(setMinMaxRange(int, int)));
  this->connect(d->AutoManualComboBox, SIGNAL(currentIndexChanged(int)),
                SLOT(setAutoWindowLevel(int)));

  // disable as there is not MRML Node associated with the widget
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setAutoWindowLevel(int autoWindowLevel)
{
  QCTK_D(qMRMLWindowLevelWidget);
  
  if (this->VolumeDisplayNode)
  {
    int oldAuto = this->VolumeDisplayNode->GetAutoWindowLevel();

    int disabledModify = this->VolumeDisplayNode->StartModify();
    this->VolumeDisplayNode->SetAutoWindowLevel(autoWindowLevel);
    this->VolumeDisplayNode->EndModify(disabledModify);

    if (autoWindowLevel != oldAuto)
    {
       emit this->autoWindowLevelValueChanged(autoWindowLevel);
    }

  }
}

// --------------------------------------------------------------------------
int qMRMLWindowLevelWidget::autoWindowLevel() const
{
  QCTK_D(const qMRMLWindowLevelWidget);
  
  // Assumes settings of the sliders are all the same
  return d->AutoManualComboBox->currentIndex();
}


// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setWindowLevel(double window, double level)
{
  if (this->VolumeDisplayNode)
  {
    double oldWindow = this->VolumeDisplayNode->GetWindow();
    double oldLevel  = this->VolumeDisplayNode->GetLevel();

    int disabledModify = this->VolumeDisplayNode->StartModify();
    this->VolumeDisplayNode->SetWindow(window);
    this->VolumeDisplayNode->SetLevel(level);
    if (oldWindow != this->VolumeDisplayNode->GetWindow() ||
        oldLevel  != this->VolumeDisplayNode->GetLevel())
    {
      this->setAutoWindowLevel(0);
      emit this->windowLevelValuesChanged(window, level);
    }
    this->VolumeDisplayNode->EndModify(disabledModify);
  }
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setMinMaxRange(double min, double max)
{
  double window = max - min;
  double level = 0.5*(min+max);
  
  this->setWindowLevel(window, level);
}

// TODO remove when range becomes double
// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setMinMaxRange(int min, int max)
{
  this->setMinMaxRange((double)min, (double)max);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setWindow(double window)
{
  if (this->VolumeDisplayNode)
  {
    double level  = this->VolumeDisplayNode->GetLevel();
    this->setWindowLevel(window, level);
  }
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setLevel(double level)
{
  if (this->VolumeDisplayNode)
  {
    double window = this->VolumeDisplayNode->GetWindow();
    this->setWindowLevel(window, level);
  }
}

// --------------------------------------------------------------------------
double qMRMLWindowLevelWidget::window() const
{
  QCTK_D(const qMRMLWindowLevelWidget);

  double min = d->WindowLevelRangeSlider->rangeMinimum();
  double max = d->WindowLevelRangeSlider->rangeMaximum();

  return max - min;
}

// --------------------------------------------------------------------------
double qMRMLWindowLevelWidget::level() const
{
  QCTK_D(const qMRMLWindowLevelWidget);

  double min = d->WindowLevelRangeSlider->rangeMinimum();
  double max = d->WindowLevelRangeSlider->rangeMaximum();

  return 0.5*(max + min);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setMRMLVolumeDisplayNode(vtkMRMLScalarVolumeDisplayNode* node)
{

  // each time the node is modified, the qt widgets are updated
  this->qvtkReconnect(this->VolumeDisplayNode, node, 
                       vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));

  this->VolumeDisplayNode = node;

  this->setEnabled(node != 0);
  
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setMRMLVolumeNode(vtkMRMLNode* node)
{
  this->setMRMLVolumeNode(vtkMRMLScalarVolumeNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setMRMLVolumeNode(vtkMRMLScalarVolumeNode* volumeNode)
{
  QCTK_D(qMRMLWindowLevelWidget);
  
  if (volumeNode) 
  {
    // TODO: set image data range in the range widget
    // d->WindowLevelRangeSlider  volumeNode->GetImageData()->GetScalarRange()
    this->setMRMLVolumeDisplayNode(vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
              volumeNode->GetVolumeDisplayNode()));
  }
  this->VolumeNode = volumeNode;
}



// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setMinimum(double min)
{
  QCTK_D(qMRMLWindowLevelWidget);
  // TODO set min in the range widget d->WindowLevelRangeSlider
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setMaximum(double max)
{
  QCTK_D(qMRMLWindowLevelWidget);
  // TODO set max in the range widget d->WindowLevelRangeSlider

}


// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::updateWidgetFromMRML()
{
  QCTK_D(qMRMLWindowLevelWidget);
  
  if (this->VolumeDisplayNode) 
  {
    int oldAuto = d->AutoManualComboBox->currentIndex();
    double oldMin = d->WindowLevelRangeSlider->rangeMinimum();
    double oldMax = d->WindowLevelRangeSlider->rangeMaximum();

    double window = this->VolumeDisplayNode->GetWindow();
    double level = this->VolumeDisplayNode->GetLevel();
    d->AutoManualComboBox->setCurrentIndex(this->VolumeDisplayNode->GetAutoWindowLevel());

    double min = level - 0.5 * window;
    double max = level + 0.5 * window;

    //TODO: set correct bounds of the range widget
    // clipping in the range widget causing multiple volume display node updates
    // also a problem if values get clipped, the mode switches from Auto to Manual
    d->WindowLevelRangeSlider->setRangeBounds(min, max );

    //d->WindowLevelRangeSlider->setRangeMinimumPosition(level - 0.5 * window);
    //d->WindowLevelRangeSlider->setRangeMaximumPosition(level + 0.5 * window);
  }

}

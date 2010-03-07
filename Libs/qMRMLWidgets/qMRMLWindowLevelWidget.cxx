
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
                SLOT(setWindowLevel(int, int)));
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
    this->VolumeDisplayNode->SetAutoWindowLevel(autoWindowLevel);
    if (this->VolumeNode && autoWindowLevel == 1)
    {
      this->VolumeNode->CalculateAutoLevels();
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
      this->VolumeDisplayNode->SetAutoWindowLevel(0);
    }
    this->VolumeDisplayNode->EndModify(disabledModify);
  }
}

// TODO remove when range becomes double
// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setWindowLevel(int window, int level)
{
  this->setWindowLevel((double)window, (double)level);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setWindow(double window)
{
  if (this->VolumeDisplayNode)
  {
    double oldWindow = this->VolumeDisplayNode->GetWindow();

    int disabledModify = this->VolumeDisplayNode->StartModify();
    this->VolumeDisplayNode->SetWindow(window);
    if (oldWindow != this->VolumeDisplayNode->GetWindow())
    {
      this->VolumeDisplayNode->SetAutoWindowLevel(0);
    }
    this->VolumeDisplayNode->EndModify(disabledModify);

  }
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setLevel(double level)
{
  if (this->VolumeDisplayNode)
  {
    double oldLevel  = this->VolumeDisplayNode->GetLevel();

    int disabledModify = this->VolumeDisplayNode->StartModify();
    this->VolumeDisplayNode->SetLevel(level);
    if (oldLevel  != this->VolumeDisplayNode->GetLevel())
    {
      this->VolumeDisplayNode->SetAutoWindowLevel(0);
    }
    this->VolumeDisplayNode->EndModify(disabledModify);

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
    d->WindowLevelRangeSlider->setRange(level - 0.5 * window, level + 0.5 * window);

    if (oldAuto != d->AutoManualComboBox->currentIndex())
    {
       emit this->autoWindowLevelValueChanged(d->AutoManualComboBox->currentIndex());
    }

    if (oldMin != d->WindowLevelRangeSlider->rangeMinimum() ||
        oldMax != d->WindowLevelRangeSlider->rangeMaximum() )
    {
      emit this->windowLevelValuesChanged(window, level);
    }
  }

}

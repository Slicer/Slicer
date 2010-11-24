
// Qt includes
#include <QDebug>
#include <QMenu>
#include <QStack>
#include <QWidgetAction>

// qMRML includes
#include "qMRMLUtils.h"
#include "qMRMLWindowLevelWidget.h"
#include "ui_qMRMLWindowLevelWidget.h"

// MRML includes
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"

// VTK includes
#include "vtkSmartPointer.h"


//-----------------------------------------------------------------------------
class qMRMLWindowLevelWidgetPrivate: public Ui_qMRMLWindowLevelWidget
{
public:
  qMRMLWindowLevelWidgetPrivate();
  void init();
 
  vtkMRMLScalarVolumeNode* VolumeNode;
  vtkMRMLScalarVolumeDisplayNode* VolumeDisplayNode;
  QDoubleSpinBox* MinSpinBox;
  QDoubleSpinBox* MaxSpinBox;
};

// --------------------------------------------------------------------------
qMRMLWindowLevelWidgetPrivate::qMRMLWindowLevelWidgetPrivate()
{
  this->VolumeNode = 0;
  this->VolumeDisplayNode = 0;
  this->MinSpinBox = 0;
  this->MaxSpinBox = 0;
}

// --------------------------------------------------------------------------
qMRMLWindowLevelWidget::qMRMLWindowLevelWidget(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLWindowLevelWidgetPrivate)
{
  Q_D(qMRMLWindowLevelWidget);
  
  d->setupUi(this);
  this->setupMoreOptions();

  this->setAutoWindowLevel(qMRMLWindowLevelWidget::Auto);

  this->connect(d->WindowLevelRangeSlider, SIGNAL(minimumValueChanged(double)),
                this, SLOT(setMinimumValue(double)));
  this->connect(d->WindowLevelRangeSlider, SIGNAL(maximumValueChanged(double)),
                this, SLOT(setMaximumValue(double)));

  this->connect(d->WindowSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(setWindow(double)));
  this->connect(d->LevelSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(setLevel(double)));

  this->connect(d->AutoManualComboBox, SIGNAL(currentIndexChanged(int)),
                SLOT(setAutoWindowLevel(int)));

  // disable as there is not MRML Node associated with the widget
  this->setEnabled(d->VolumeDisplayNode != 0);
}

// --------------------------------------------------------------------------
qMRMLWindowLevelWidget::~qMRMLWindowLevelWidget()
{
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setupMoreOptions()
{
  Q_D(qMRMLWindowLevelWidget);
  QWidget* rangeWidget = new QWidget;
  d->MinSpinBox = new QDoubleSpinBox;
  d->MaxSpinBox = new QDoubleSpinBox;
  d->MinSpinBox->setPrefix("Min: ");
  d->MinSpinBox->setRange(-1000000., 1000000.);
  d->MinSpinBox->setValue(d->WindowLevelRangeSlider->minimum());
  d->MaxSpinBox->setPrefix("Max: ");
  d->MaxSpinBox->setRange(-1000000., 1000000.);
  d->MaxSpinBox->setValue(d->WindowLevelRangeSlider->maximum());
  connect(d->MinSpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateRange()));
  connect(d->MaxSpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(updateRange()));
  connect(d->WindowLevelRangeSlider, SIGNAL(rangeChanged(double, double)),
          this, SLOT(updateSpinBoxRange(double, double)));
  QHBoxLayout* rangeLayout = new QHBoxLayout;
  rangeLayout->addWidget(d->MinSpinBox);
  rangeLayout->addWidget(d->MaxSpinBox);
  rangeLayout->setContentsMargins(0,0,0,0);
  rangeWidget->setLayout(rangeLayout);
  
  QWidgetAction* rangeAction = new QWidgetAction(0);
  rangeAction->setDefaultWidget(rangeWidget);
  
  QAction* symmetricAction = new QAction(tr("Symmetric handles"),0);
  symmetricAction->setCheckable(true);
  connect(symmetricAction, SIGNAL(toggled(bool)),
          this, SLOT(updateSymmetricMoves(bool)));
  symmetricAction->setChecked(true);
  
  QMenu* optionsMenu = new QMenu;
  optionsMenu->addAction(rangeAction);
  optionsMenu->addAction(symmetricAction);
  
  d->MoreOptionsButton->setMenu(optionsMenu);
  d->MoreOptionsButton->setPopupMode(QToolButton::InstantPopup);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setAutoWindowLevel(ControlMode autoWindowLevel)
{
  Q_D(qMRMLWindowLevelWidget);

  if (d->VolumeDisplayNode)
    {
    int oldAuto = d->VolumeDisplayNode->GetAutoWindowLevel();

    //int disabledModify = this->VolumeDisplayNode->StartModify();
    d->VolumeDisplayNode->SetAutoWindowLevel(autoWindowLevel);
    //this->VolumeDisplayNode->EndModify(disabledModify);

    if (autoWindowLevel != oldAuto)
      {
       emit this->autoWindowLevelValueChanged(autoWindowLevel);
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setAutoWindowLevel(int autoWindowLevel)
{
  switch(autoWindowLevel)
    {
    case qMRMLWindowLevelWidget::Manual:
      this->setAutoWindowLevel(qMRMLWindowLevelWidget::Manual);
      break;
    case qMRMLWindowLevelWidget::Auto:
      this->setAutoWindowLevel(qMRMLWindowLevelWidget::Auto);
      break;
    default:
      break;
    }
}

// --------------------------------------------------------------------------
qMRMLWindowLevelWidget::ControlMode qMRMLWindowLevelWidget::autoWindowLevel() const
{
  Q_D(const qMRMLWindowLevelWidget);
#ifndef _NDEBUG
  if (d->VolumeDisplayNode)
    {
    Q_ASSERT(d->VolumeDisplayNode->GetAutoWindowLevel() ==
             d->AutoManualComboBox->currentIndex());
    }
#endif
  return d->AutoManualComboBox->currentIndex() == 0 ?
    qMRMLWindowLevelWidget::Manual : qMRMLWindowLevelWidget::Auto;
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setWindowLevel(double window, double level)
{
  Q_D(const qMRMLWindowLevelWidget);
  if (d->VolumeDisplayNode)
    {
    double oldWindow = d->VolumeDisplayNode->GetWindow();
    double oldLevel  = d->VolumeDisplayNode->GetLevel();

    int disabledModify = d->VolumeDisplayNode->StartModify();
    d->VolumeDisplayNode->SetWindow(window);
    d->VolumeDisplayNode->SetLevel(level);
    if (oldWindow != d->VolumeDisplayNode->GetWindow() ||
        oldLevel  != d->VolumeDisplayNode->GetLevel())
      {
      this->setAutoWindowLevel(qMRMLWindowLevelWidget::Manual);
      emit this->windowLevelValuesChanged(window, level);
      }
    d->VolumeDisplayNode->EndModify(disabledModify);
  }
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setMinMaxRangeValue(double min, double max)
{
  double window = max - min;
  double level = 0.5*(min+max);

  this->setWindowLevel(window, level);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setWindow(double window)
{
  Q_D(const qMRMLWindowLevelWidget);
  if (d->VolumeDisplayNode)
    {
    double level  = d->VolumeDisplayNode->GetLevel();
    this->setWindowLevel(window, level);
    }
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setLevel(double level)
{
  Q_D(qMRMLWindowLevelWidget);
  if (d->VolumeDisplayNode)
    {
    double window = d->VolumeDisplayNode->GetWindow();
    this->setWindowLevel(window, level);
    }
}

// --------------------------------------------------------------------------
double qMRMLWindowLevelWidget::window() const
{
  Q_D(const qMRMLWindowLevelWidget);

  double min = d->WindowLevelRangeSlider->minimumValue();
  double max = d->WindowLevelRangeSlider->maximumValue();

  return max - min;
}

// --------------------------------------------------------------------------
double qMRMLWindowLevelWidget::minimumValue() const
{
  Q_D(const qMRMLWindowLevelWidget);

  double min = d->WindowLevelRangeSlider->minimumValue();
  return min;
}

// --------------------------------------------------------------------------
double qMRMLWindowLevelWidget::maximumValue() const
{
  Q_D(const qMRMLWindowLevelWidget);

  double max = d->WindowLevelRangeSlider->maximumValue();
  return max;
}

// --------------------------------------------------------------------------
double qMRMLWindowLevelWidget::level() const
{
  Q_D(const qMRMLWindowLevelWidget);

  double min = d->WindowLevelRangeSlider->minimumValue();
  double max = d->WindowLevelRangeSlider->maximumValue();

  return 0.5*(max + min);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setMRMLVolumeDisplayNode(vtkMRMLScalarVolumeDisplayNode* node)
{
  Q_D(qMRMLWindowLevelWidget);
  if (d->VolumeDisplayNode == node)
    {
    return;
    }

  // each time the node is modified, the qt widgets are updated
  this->qvtkReconnect(d->VolumeDisplayNode, node,
                       vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));

  d->VolumeDisplayNode = node;

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
  Q_D(qMRMLWindowLevelWidget);
  if (volumeNode == d->VolumeNode)
    {
    return;
    }

  this->qvtkReconnect(d->VolumeNode, volumeNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateDisplayNode()));

  d->VolumeNode = volumeNode;
  if (d->VolumeNode)
    {
    double range[2];
    d->VolumeNode->GetImageData()->GetScalarRange(range);
    //give us some space
    range[0] = qMin(-1200., range[0]);
    range[1] = qMax(900., range[1]);
    this->setRange(range[0], range[1]);
    }
  this->updateDisplayNode();
}

// --------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* qMRMLWindowLevelWidget::mrmlVolumeNode()const
{
  Q_D(const qMRMLWindowLevelWidget);
  return d->VolumeNode;
}

// --------------------------------------------------------------------------
vtkMRMLScalarVolumeDisplayNode* qMRMLWindowLevelWidget::mrmlDisplayNode()const
{
  Q_D(const qMRMLWindowLevelWidget);
  return d->VolumeDisplayNode;
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setMinimumValue(double min)
{
  this->setMinMaxRangeValue(min, this->maximumValue());
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setMaximumValue(double max)
{
  this->setMinMaxRangeValue(this->minimumValue(), max);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::updateDisplayNode()
{
  Q_D(qMRMLWindowLevelWidget);
  
  this->setMRMLVolumeDisplayNode( d->VolumeNode ? 
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
      d->VolumeNode->GetVolumeDisplayNode()) :
    0);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLWindowLevelWidget);

  if (!d->VolumeDisplayNode)
    {
    return;
    }
  d->AutoManualComboBox->setCurrentIndex(
    d->VolumeDisplayNode->GetAutoWindowLevel());
  double window = d->VolumeDisplayNode->GetWindow();
  double level = d->VolumeDisplayNode->GetLevel();
  d->WindowSpinBox->setValue(window);
  d->LevelSpinBox->setValue(level);
  double min = level - 0.5 * window;
  double max = level + 0.5 * window;
  d->WindowLevelRangeSlider->setValues(min, max );
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::updateSpinBoxRange(double min, double max)
{
  Q_D(qMRMLWindowLevelWidget);
  d->MinSpinBox->setValue(min);
  d->MaxSpinBox->setValue(max);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::updateRange()
{
  Q_D(qMRMLWindowLevelWidget);
  d->WindowLevelRangeSlider->setRange(d->MinSpinBox->value(),
                                      d->MaxSpinBox->value());
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setRange(double min, double max)
{
  Q_D(qMRMLWindowLevelWidget);
  d->WindowLevelRangeSlider->setRange(min, max);
  d->WindowSpinBox->setRange(0, max - min);
  d->LevelSpinBox->setRange(min, max);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::updateSymmetricMoves(bool symmetric)
{
  Q_D(qMRMLWindowLevelWidget);
  d->WindowLevelRangeSlider->setSymmetricMoves(symmetric);
}


// Qt includes

// CTK includes
#include <ctkRangeWidget.h>
#include <ctkPopupWidget.h>
#include <ctkUtils.h>

// qMRML includes
#include "qMRMLWindowLevelWidget.h"
#include "ui_qMRMLWindowLevelWidget.h"

// MRML includes
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"

// VTK includes
#include <vtkImageData.h>

//-----------------------------------------------------------------------------
class qMRMLWindowLevelWidgetPrivate: public Ui_qMRMLWindowLevelWidget
{
  Q_DECLARE_PUBLIC(qMRMLWindowLevelWidget);
protected:
  qMRMLWindowLevelWidget* const q_ptr;

public:
  qMRMLWindowLevelWidgetPrivate(qMRMLWindowLevelWidget& object);
  virtual ~qMRMLWindowLevelWidgetPrivate();
  void init();
  bool blockSignals(bool block);
  void scalarRange(vtkMRMLScalarVolumeDisplayNode* displayNode, double range[2]);
  void updateSingleStep(double min, double max);
  
  vtkMRMLScalarVolumeNode*        VolumeNode;
  vtkMRMLScalarVolumeDisplayNode* VolumeDisplayNode;
  ctkPopupWidget*                 PopupWidget;
  ctkRangeWidget*                 RangeWidget;
  double                          DisplayScalarRange[2];
};

// --------------------------------------------------------------------------
qMRMLWindowLevelWidgetPrivate::qMRMLWindowLevelWidgetPrivate(
  qMRMLWindowLevelWidget& object)
  : q_ptr(&object)
{
  this->VolumeNode = 0;
  this->VolumeDisplayNode = 0;
  this->PopupWidget = 0;
  this->RangeWidget = 0;
  this->DisplayScalarRange[0] = 0;
  this->DisplayScalarRange[1] = 0;
}

// --------------------------------------------------------------------------
qMRMLWindowLevelWidgetPrivate::~qMRMLWindowLevelWidgetPrivate()
{
  delete this->PopupWidget;
  this->PopupWidget = 0;
  this->RangeWidget = 0;
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidgetPrivate::init()
{
  Q_Q(qMRMLWindowLevelWidget);

  this->setupUi(q);

  q->setAutoWindowLevel(qMRMLWindowLevelWidget::Auto);

  QObject::connect(this->WindowLevelRangeSlider, SIGNAL(valuesChanged(double, double)),
                   q, SLOT(setMinMaxRangeValue(double, double)));

  QObject::connect(this->WindowSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(setWindow(double)));
  QObject::connect(this->LevelSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(setLevel(double)));

  QObject::connect(this->MinSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(setMinimumValue(double)));
  QObject::connect(this->MaxSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(setMaximumValue(double)));
  this->MinSpinBox->setVisible(false);
  this->MaxSpinBox->setVisible(false);

  QObject::connect(this->AutoManualComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(setAutoWindowLevel(int)));

  // disable as there is not MRML Node associated with the widget
  q->setEnabled(this->VolumeDisplayNode != 0);

  // we can't use the flag Qt::Popup as it automatically closes when there is
  // a click outside of the rangewidget
  this->PopupWidget = new ctkPopupWidget(0);

  QPalette popupPalette = q->palette();
  QColor windowColor = popupPalette.color(QPalette::Window);
  windowColor.setAlpha(200);
  QColor darkColor = popupPalette.color(QPalette::Dark);
  darkColor.setAlpha(200);
  /*
  QLinearGradient gradient(QPointF(0.,0.),QPointF(0.,0.5));
  gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
  gradient.setColorAt(0, windowColor);
  gradient.setColorAt(1, darkColor);
  popupPalette.setBrush(QPalette::Window, gradient);
  */
  popupPalette.setColor(QPalette::Window, darkColor);
  this->PopupWidget->setPalette(popupPalette);
  this->PopupWidget->setAttribute(Qt::WA_TranslucentBackground, true);

  this->PopupWidget->setAutoShow(false);
  this->PopupWidget->setAutoHide(true);
  this->PopupWidget->setBaseWidget(q);
  this->RangeWidget = new ctkRangeWidget;

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(this->RangeWidget);
  this->PopupWidget->setLayout(layout);

  QMargins margins = layout->contentsMargins();
  margins.setTop(0);
  layout->setContentsMargins(margins);
  
  this->RangeWidget->setSpinBoxAlignment(Qt::AlignBottom);
  this->RangeWidget->setRange(-1000000., 1000000.);
  QObject::connect(this->RangeWidget, SIGNAL(valuesChanged(double, double)),
                   q, SLOT(setRange(double, double)));
  this->RangeWidget->setToolTip("Set the range boundaries of Window/Level to control large numbers or allow fine tuning");
}

// --------------------------------------------------------------------------
bool qMRMLWindowLevelWidgetPrivate::blockSignals(bool block)
{
  this->WindowLevelRangeSlider->blockSignals(block);
  this->WindowSpinBox->blockSignals(block);
  this->LevelSpinBox->blockSignals(block);
  this->MinSpinBox->blockSignals(block);
  this->MaxSpinBox->blockSignals(block);
  return this->RangeWidget->blockSignals(block);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidgetPrivate::scalarRange(vtkMRMLScalarVolumeDisplayNode* dNode, double range[2])
{
  // vtkMRMLScalarVolumeDisplayNode::GetDisplayScalarRange() can be a bit
  // slow if there is no input as it searches the scene for the associated
  // volume node.
  // Here we already know the volumenode so we can manually use it to
  // retrieve the scalar range.
  if (dNode && dNode->GetInputImageData())
    {
    dNode->GetDisplayScalarRange(range);
    }
  else
    {
    this->VolumeNode->GetImageData()->GetScalarRange(range);
    }
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidgetPrivate::updateSingleStep(double min, double max)
{
  double interval = max - min;
  int order = interval != 0. ? ctk::orderOfMagnitude(interval) : -2;

  int ratio = 2;
  double singleStep = pow(10., order - ratio);
  int decimals = qMax(0, -order + ratio);

  this->WindowSpinBox->setDecimals(decimals);
  this->LevelSpinBox->setDecimals(decimals);
  this->MinSpinBox->setDecimals(decimals);
  this->MaxSpinBox->setDecimals(decimals);

  this->WindowLevelRangeSlider->setSingleStep(singleStep);
  this->WindowSpinBox->setSingleStep(singleStep);
  this->LevelSpinBox->setSingleStep(singleStep);
  this->MinSpinBox->setSingleStep(singleStep);
  this->MaxSpinBox->setSingleStep(singleStep);

  // The RangeWidget doesn't have to be as precise as the sliders/spinboxes.
  ratio = 1;
  singleStep = pow(10., order - ratio);
  decimals = qMax(0, -order + ratio);

  this->RangeWidget->setDecimals(decimals);
  this->RangeWidget->setSingleStep(singleStep);
}

// --------------------------------------------------------------------------
qMRMLWindowLevelWidget::qMRMLWindowLevelWidget(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLWindowLevelWidgetPrivate(*this))
{
  Q_D(qMRMLWindowLevelWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLWindowLevelWidget::~qMRMLWindowLevelWidget()
{
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setAutoWindowLevel(ControlMode autoWindowLevel)
{
  Q_D(qMRMLWindowLevelWidget);

  if (!d->VolumeDisplayNode)
    {
    return;
    }
  int oldAuto = d->VolumeDisplayNode->GetAutoWindowLevel();

  //int disabledModify = this->VolumeDisplayNode->StartModify();
  d->VolumeDisplayNode->SetAutoWindowLevel(
    autoWindowLevel == qMRMLWindowLevelWidget::Auto ? 1 : 0);
  //this->VolumeDisplayNode->EndModify(disabledModify);

  switch (autoWindowLevel)
    {
    case qMRMLWindowLevelWidget::ManualMinMax:
      d->WindowLevelRangeSlider->setSymmetricMoves(false);
      d->WindowSpinBox->setVisible(false);
      d->LevelSpinBox->setVisible(false);
      d->MinSpinBox->setVisible(true);
      d->MaxSpinBox->setVisible(true);
      break;
    default:
    case qMRMLWindowLevelWidget::Auto:
    case qMRMLWindowLevelWidget::Manual:
      d->WindowLevelRangeSlider->setSymmetricMoves(true);
      d->MinSpinBox->setVisible(false);
      d->MaxSpinBox->setVisible(false);
      d->WindowSpinBox->setVisible(true);
      d->LevelSpinBox->setVisible(true);
      break;
    }
  if (autoWindowLevel != qMRMLWindowLevelWidget::Auto)
    {
    d->PopupWidget->setAutoShow(true);
    d->PopupWidget->showPopup();
    }
  else
    {
    d->PopupWidget->setAutoShow(false);
    d->PopupWidget->hidePopup();
    }
  
  if (autoWindowLevel != oldAuto)
    {
    emit this->autoWindowLevelValueChanged(
      autoWindowLevel == qMRMLWindowLevelWidget::Auto ? 
        qMRMLWindowLevelWidget::Auto : qMRMLWindowLevelWidget::Manual);
    }
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setAutoWindowLevel(int autoWindowLevel)
{
  switch(autoWindowLevel)
    {
    case qMRMLWindowLevelWidget::Auto:
      this->setAutoWindowLevel(qMRMLWindowLevelWidget::Auto);
      break;
    case qMRMLWindowLevelWidget::Manual:
      this->setAutoWindowLevel(qMRMLWindowLevelWidget::Manual);
      break;
    case qMRMLWindowLevelWidget::ManualMinMax:
      this->setAutoWindowLevel(qMRMLWindowLevelWidget::ManualMinMax);
      break;
    default:
      break;
    }
}

// --------------------------------------------------------------------------
qMRMLWindowLevelWidget::ControlMode qMRMLWindowLevelWidget::autoWindowLevel() const
{
  Q_D(const qMRMLWindowLevelWidget);
  switch (d->AutoManualComboBox->currentIndex())
    {
    case qMRMLWindowLevelWidget::Auto:
      return qMRMLWindowLevelWidget::Auto;
      break;
    case qMRMLWindowLevelWidget::Manual:
      return qMRMLWindowLevelWidget::Manual;
      break;
    case qMRMLWindowLevelWidget::ManualMinMax:
      return qMRMLWindowLevelWidget::ManualMinMax;
      break;
    }
  return qMRMLWindowLevelWidget::Manual;
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setWindowLevel(double window, double level)
{
  Q_D(const qMRMLWindowLevelWidget);
  if (!d->VolumeDisplayNode)
    {
    return;
    }
  double oldWindow = d->VolumeDisplayNode->GetWindow();
  double oldLevel  = d->VolumeDisplayNode->GetLevel();

  int disabledModify = d->VolumeDisplayNode->StartModify();
  d->VolumeDisplayNode->SetWindowLevel(window, level);
  if (!qFuzzyCompare(oldWindow, d->VolumeDisplayNode->GetWindow()) ||
      !qFuzzyCompare(oldLevel, d->VolumeDisplayNode->GetLevel()))
    {
    if (this->autoWindowLevel() == qMRMLWindowLevelWidget::Auto)
      {
      this->setAutoWindowLevel(qMRMLWindowLevelWidget::Manual);
      }
    emit this->windowLevelValuesChanged(window, level);
    }
  d->VolumeDisplayNode->EndModify(disabledModify);
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
                       vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRMLDisplayNode()));

  d->VolumeDisplayNode = node;

  this->setEnabled(node != 0);

  this->updateWidgetFromMRMLDisplayNode();
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
                      this, SLOT(updateWidgetFromMRMLVolumeNode()));

  d->VolumeNode = volumeNode;
  this->updateWidgetFromMRMLVolumeNode();
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
void qMRMLWindowLevelWidget::updateWidgetFromMRMLVolumeNode()
{
  Q_D(qMRMLWindowLevelWidget);
  vtkMRMLScalarVolumeDisplayNode* newVolumeDisplayNode = d->VolumeNode ?
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
      d->VolumeNode->GetVolumeDisplayNode()) :0;
/*
  if (d->VolumeNode && d->VolumeNode->GetImageData())
    {
    this->updateRangeForVolumeDisplayNode(newVolumeDisplayNode);
    }
*/
  this->setMRMLVolumeDisplayNode( newVolumeDisplayNode );
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::updateWidgetFromMRMLDisplayNode()
{
  Q_D(qMRMLWindowLevelWidget);
  if (!d->VolumeDisplayNode)
    {
    return;
    }

  double range[2];
  d->scalarRange(d->VolumeDisplayNode, range);
  if (range[0] != d->DisplayScalarRange[0] ||
      range[1] != d->DisplayScalarRange[1])
    {
    this->updateRangeForVolumeDisplayNode(d->VolumeDisplayNode);
    }

  double window = d->VolumeDisplayNode->GetWindow();
  double level = d->VolumeDisplayNode->GetLevel();
  double min = d->VolumeDisplayNode->GetWindowLevelMin();
  double max = d->VolumeDisplayNode->GetWindowLevelMax();

  // We block here to prevent the widgets to call setWindowLevel wich could
  // change the AutoLevel from Auto into Manual.
  bool blocked = d->blockSignals(true);

  d->WindowSpinBox->setValue(window);
  d->LevelSpinBox->setValue(level);
  d->WindowLevelRangeSlider->setValues(min, max);
  d->MinSpinBox->setValue(min);
  d->MaxSpinBox->setValue(max);

  d->blockSignals(blocked);

  switch (d->VolumeDisplayNode->GetAutoWindowLevel())
    {
    case 1:
      d->AutoManualComboBox->setCurrentIndex(qMRMLWindowLevelWidget::Auto);
      break;
    case 0:
      if (d->AutoManualComboBox->currentIndex() == qMRMLWindowLevelWidget::Auto)
        {
        d->AutoManualComboBox->setCurrentIndex(qMRMLWindowLevelWidget::Manual);
        }
      break;
    }
}

//------------------------------------------------------------------------------
void qMRMLWindowLevelWidget::updateRangeForVolumeDisplayNode(vtkMRMLScalarVolumeDisplayNode* dNode)
{
  Q_D(qMRMLWindowLevelWidget);
  double range[2];
  d->scalarRange(dNode, range);
  d->DisplayScalarRange[0] = range[0];
  d->DisplayScalarRange[1] = range[1];
  // we don't want RangeWidget to fire any signal because we don't have
  // a display node correctly set here (it's done )
  d->RangeWidget->blockSignals(true);
  double interval = range[1] - range[0];
  Q_ASSERT(interval >= 0.);
  double min, max;

  if (interval <= 10.)
    {
    min = qMin(-10., range[0] - 2.*interval);
    max = qMax(10., range[1] + 2.*interval);
    }
  else
    {
    min = qMin(-1200., range[0] - 2.*interval);
    max = qMax(900., range[1] + 2.*interval);
    }

  d->RangeWidget->setRange(min, max);
  d->RangeWidget->blockSignals(false);

  if (interval < 10.)
    {
    //give us some space
    range[0] = range[0] - interval*0.1;
    range[1] = range[1] + interval*0.1;
    }
  else
    {
    //give us some space
    range[0] = qMin(-600., range[0] - interval*0.1);
    range[1] = qMax(600., range[1] + interval*0.1);
    }
  bool blocked = d->blockSignals(true);
  this->setRange(range[0], range[1]);
  d->blockSignals(blocked);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setRange(double min, double max)
{
  Q_D(qMRMLWindowLevelWidget);

  d->updateSingleStep(min, max);

  d->WindowLevelRangeSlider->setRange(min, max);
  d->WindowSpinBox->setRange(0, max - min);
  d->LevelSpinBox->setRange(min, max);
  d->MinSpinBox->setRange(min, max);
  d->MaxSpinBox->setRange(min, max);
  d->RangeWidget->setValues(min, max);
}

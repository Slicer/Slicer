
// Qt includes
#include <QDebug>
#include <QMenu>
#include <QStack>
#include <QPropertyAnimation>
#include <QTimer>
#include <QWidgetAction>

// CTK includes
#include <ctkRangeWidget.h>

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
  Q_DECLARE_PUBLIC(qMRMLWindowLevelWidget);
protected:
  qMRMLWindowLevelWidget* const q_ptr;

public:
  qMRMLWindowLevelWidgetPrivate(qMRMLWindowLevelWidget& object);
  virtual ~qMRMLWindowLevelWidgetPrivate();
  void init();
  void openRangeWidget();
  void closeRangeWidget();
  
  vtkMRMLScalarVolumeNode*        VolumeNode;
  vtkMRMLScalarVolumeDisplayNode* VolumeDisplayNode;
  ctkRangeWidget*                 RangeWidget;
  QPropertyAnimation*             RangeWidgetAnimation;
  double                          DisplayScalarRange[2];
};

// --------------------------------------------------------------------------
qMRMLWindowLevelWidgetPrivate::qMRMLWindowLevelWidgetPrivate(
  qMRMLWindowLevelWidget& object)
  : q_ptr(&object)
{
  this->VolumeNode = 0;
  this->VolumeDisplayNode = 0;
  this->RangeWidget = 0;
  this->DisplayScalarRange[0] = 0;
  this->DisplayScalarRange[1] = 0;
}

// --------------------------------------------------------------------------
qMRMLWindowLevelWidgetPrivate::~qMRMLWindowLevelWidgetPrivate()
{
  delete this->RangeWidget;
  this->RangeWidget = 0;
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidgetPrivate::init()
{
  Q_Q(qMRMLWindowLevelWidget);

  this->setupUi(q);

  q->setAutoWindowLevel(qMRMLWindowLevelWidget::Auto);

  QObject::connect(this->WindowLevelRangeSlider, SIGNAL(valuesChanged(double, double)),
                   q, SLOT(onMinMaxValuesChanged(double, double)));

  QObject::connect(this->WindowSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(onWindowValueChanged(double)));
  QObject::connect(this->LevelSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(onLevelValueChanged(double)));

  QObject::connect(this->MinSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(onMinValueChanged(double)));
  QObject::connect(this->MaxSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(onMaxValueChanged(double)));
  this->MinSpinBox->setVisible(false);
  this->MaxSpinBox->setVisible(false);

  QObject::connect(this->AutoManualComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(setAutoWindowLevel(int)));

  // disable as there is not MRML Node associated with the widget
  q->setEnabled(this->VolumeDisplayNode != 0);

  this->RangeWidget = new ctkRangeWidget(0);
  // you can't use the flag Qt::Popup as it automatically closes when there is
  // a click outside of the rangewidget
  this->RangeWidget->setWindowFlags(Qt::ToolTip);
  //this->RangeWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  this->RangeWidget->setSpinBoxAlignment(Qt::AlignBottom);
  this->RangeWidget->setRange(-1000000., 1000000.);
  //this->RangeWidget->resize(this->RangeWidget->sizeHint());
  this->RangeWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  //this->RangeWidget->setVisible(false);
  this->RangeWidget->installEventFilter(q);
  QObject::connect(this->RangeWidget, SIGNAL(valuesChanged(double, double)),
                   q, SLOT(setRange(double, double)));
  this->RangeWidget->setToolTip("Set the range boundaries of Window/Level to control large numbers or allow fine tuning");
  this->RangeWidgetAnimation = new QPropertyAnimation(this->RangeWidget, "geometry");
  this->RangeWidgetAnimation->setDuration(100);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidgetPrivate::openRangeWidget()
{
  Q_Q(qMRMLWindowLevelWidget);
  if (this->RangeWidget->isVisible() ||
      !q->underMouse())
    {
    return;
    }
  QPoint bottomLeft = QPoint(q->geometry().x(), q->geometry().bottom());
  QPoint pos = q->parentWidget() ? q->parentWidget()->mapToGlobal(bottomLeft) : bottomLeft;
  this->RangeWidgetAnimation->setStartValue(
    QRect(pos, QSize(q->width(), 0)));
  this->RangeWidgetAnimation->setEndValue(
    QRect(pos, QSize(q->width(), this->RangeWidget->sizeHint().height())));
  this->RangeWidget->setVisible(true);
  this->RangeWidgetAnimation->start();
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidgetPrivate::closeRangeWidget()
{
  Q_Q(qMRMLWindowLevelWidget);

  if (!this->RangeWidget->isVisible())
    {
    return;
    }
  QPoint bottomLeft = QPoint(q->geometry().x(), q->geometry().bottom());
  QPoint pos = q->parentWidget() ? q->parentWidget()->mapToGlobal(bottomLeft) : bottomLeft;
  this->RangeWidgetAnimation->setStartValue(
    QRect(pos, QSize(q->width(), this->RangeWidget->sizeHint().height())));
  this->RangeWidgetAnimation->setEndValue(
    QRect(pos, QSize(q->width(), 0)));

  this->RangeWidgetAnimation->start();
  QTimer::singleShot(this->RangeWidgetAnimation->duration(),
                     this->RangeWidget, SLOT(hide()));
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
    d->openRangeWidget();
    }
  else
    {
    d->closeRangeWidget();
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
  d->VolumeDisplayNode->GetDisplayScalarRange(range);
  if (range[0] != d->DisplayScalarRange[0] ||
      range[1] != d->DisplayScalarRange[1])
    {
    this->updateRangeForVolumeDisplayNode(d->VolumeDisplayNode);
    }

  double window = d->VolumeDisplayNode->GetWindow();
  double level = d->VolumeDisplayNode->GetLevel();
  d->WindowSpinBox->setValue(window);
  d->LevelSpinBox->setValue(level);
  double min = d->VolumeDisplayNode->GetWindowLevelMin();
  double max = d->VolumeDisplayNode->GetWindowLevelMax();
  d->WindowLevelRangeSlider->setValues(min, max);
  d->MinSpinBox->setValue(min);
  d->MaxSpinBox->setValue(max);
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
  if (dNode)
    {
    dNode->GetDisplayScalarRange(range);
    }
  else
    {
    d->VolumeNode->GetImageData()->GetScalarRange(range);
    }
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
    d->WindowLevelRangeSlider->setSingleStep(0.01);
    d->WindowSpinBox->setSingleStep(0.01);
    d->LevelSpinBox->setSingleStep(0.01);
    d->MinSpinBox->setSingleStep(0.01);
    d->MaxSpinBox->setSingleStep(0.01);
    d->RangeWidget->setSingleStep(0.01);

    //give us some space
    range[0] = range[0] - interval*0.1;
    range[1] = range[1] + interval*0.1;
    }
  else
    {
    d->WindowLevelRangeSlider->setSingleStep(1.0);
    d->WindowSpinBox->setSingleStep(1.0);
    d->LevelSpinBox->setSingleStep(1.0);
    d->MinSpinBox->setSingleStep(1.0);
    d->MaxSpinBox->setSingleStep(1.0);
    d->RangeWidget->setSingleStep(1.0);

    //give us some space
    range[0] = qMin(-600., range[0]);
    range[1] = qMax(600., range[1]);
    }
  d->WindowLevelRangeSlider->blockSignals(true);
  d->WindowSpinBox->blockSignals(true);
  d->LevelSpinBox->blockSignals(true);
  d->MinSpinBox->blockSignals(true);
  d->MaxSpinBox->blockSignals(true);
  d->RangeWidget->blockSignals(true);
  this->setRange(range[0], range[1]);
  d->WindowLevelRangeSlider->blockSignals(false);
  d->WindowSpinBox->blockSignals(false);
  d->LevelSpinBox->blockSignals(false);
  d->MinSpinBox->blockSignals(false);
  d->MaxSpinBox->blockSignals(false);
  d->RangeWidget->blockSignals(false);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::setRange(double min, double max)
{
  Q_D(qMRMLWindowLevelWidget);
  d->WindowLevelRangeSlider->setRange(min, max);
  d->WindowSpinBox->setRange(0, max - min);
  d->LevelSpinBox->setRange(min, max);
  d->MinSpinBox->setRange(min, max);
  d->MaxSpinBox->setRange(min, max);
  d->RangeWidget->setValues(min, max);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::onWindowValueChanged(double windowValue)
{
  Q_D(qMRMLWindowLevelWidget);
  const double nodeWindow = d->VolumeDisplayNode->GetWindow();
  const double roundNodeWindow =
    QString::number(nodeWindow, 'f', d->WindowSpinBox->decimals()).toDouble();
  if (windowValue == roundNodeWindow)
    {
    return;
    }
  this->setWindow(windowValue);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::onLevelValueChanged(double levelValue)
{
  Q_D(qMRMLWindowLevelWidget);
  const double nodeLevel = d->VolumeDisplayNode->GetLevel();
  const double roundNodeLevel =
    QString::number(nodeLevel, 'f', d->LevelSpinBox->decimals()).toDouble();
  if (levelValue == roundNodeLevel)
    {
    return;
    }
  this->setLevel(levelValue);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::onMinValueChanged(double minValue)
{
  Q_D(qMRMLWindowLevelWidget);
  const double nodeMin = d->VolumeDisplayNode->GetWindowLevelMin();
  const double roundNodeMin =
    QString::number(nodeMin, 'f', d->MinSpinBox->decimals()).toDouble();
  if (minValue == roundNodeMin)
    {
    return;
    }
  this->setMinimumValue(minValue);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::onMaxValueChanged(double maxValue)
{
  Q_D(qMRMLWindowLevelWidget);
  const double nodeMax = d->VolumeDisplayNode->GetWindowLevelMax();
  const double roundNodeMax =
    QString::number(nodeMax, 'f', d->MaxSpinBox->decimals()).toDouble();
  if (maxValue == roundNodeMax)
    {
    return;
    }
  this->setMaximumValue(maxValue);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::onMinMaxValuesChanged(double minValue, double maxValue)
{
  Q_D(qMRMLWindowLevelWidget);
  const double nodeMin = d->VolumeDisplayNode->GetWindowLevelMin();
  const double nodeMax = d->VolumeDisplayNode->GetWindowLevelMax();
  const double roundNodeMin =
    QString::number(nodeMin, 'f', 2).toDouble();
  const double roundNodeMax =
    QString::number(nodeMax, 'f', 2).toDouble();
  if (minValue == roundNodeMin && maxValue == roundNodeMax)
    {
    return;
    }
  this->setMinMaxRangeValue(minValue, maxValue);
}


// --------------------------------------------------------------------------
bool qMRMLWindowLevelWidget::eventFilter(QObject* obj, QEvent* event)
{
  if (event->type() == QEvent::Leave)
    {
    QTimer::singleShot(10, this, SLOT(hideRangeWidget()));
    }
  return this->QObject::eventFilter(obj, event);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::enterEvent(QEvent* event)
{
  Q_D(qMRMLWindowLevelWidget);
  if (this->autoWindowLevel() != Auto)
    {
    d->openRangeWidget();
    }
  this->QWidget::enterEvent(event);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::leaveEvent(QEvent* event)
{
  QTimer::singleShot(10, this, SLOT(hideRangeWidget()));
  this->QWidget::leaveEvent(event);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidget::hideRangeWidget()
{
  Q_D(qMRMLWindowLevelWidget);
/*
  QPoint topLeft = QPoint(this->geometry().left(), this->geometry().top());
  QPoint bottomRight = QPoint(this->geometry().right(), this->geometry().bottom());
  topLeft = this->parentWidget() ? this->parentWidget()->mapToGlobal(topLeft) : topLeft;
  bottomRight = this->parentWidget() ? this->parentWidget()->mapToGlobal(bottomRight) : bottomRight;
  QRect geom(topLeft, bottomRight);
*/
  if (d->RangeWidget->underMouse()
      || this->underMouse()
//      ||geom.contains(QCursor::pos())
  )
    {
    return;
    }
  d->closeRangeWidget();
}

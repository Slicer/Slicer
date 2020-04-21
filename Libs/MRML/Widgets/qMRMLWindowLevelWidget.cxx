/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// CTK includes
#include <ctkPopupWidget.h>

// qMRML includes
#include "qMRMLVolumeWidget_p.h"
#include "qMRMLWindowLevelWidget.h"
#include "ui_qMRMLWindowLevelWidget.h"

// MRML includes
#include "vtkMRMLScalarVolumeDisplayNode.h"

//-----------------------------------------------------------------------------
class qMRMLWindowLevelWidgetPrivate
  : public qMRMLVolumeWidgetPrivate
  , public Ui_qMRMLWindowLevelWidget
{
  Q_DECLARE_PUBLIC(qMRMLWindowLevelWidget);
protected:
  typedef qMRMLVolumeWidgetPrivate Superclass;

public:
  qMRMLWindowLevelWidgetPrivate(qMRMLWindowLevelWidget& object);
  ~qMRMLWindowLevelWidgetPrivate() override;
  void init() override;

  bool blockSignals(bool block) override;
  void setRange(double min, double max) override;
  void setDecimals(int decimals) override;
  void setSingleStep(double singleStep) override;
};

// --------------------------------------------------------------------------
qMRMLWindowLevelWidgetPrivate
::qMRMLWindowLevelWidgetPrivate(qMRMLWindowLevelWidget& object)
  : Superclass(object)
{
}

// --------------------------------------------------------------------------
qMRMLWindowLevelWidgetPrivate::~qMRMLWindowLevelWidgetPrivate() = default;

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidgetPrivate::init()
{
  Q_Q(qMRMLWindowLevelWidget);

  this->Superclass::init();
  this->setupUi(q);

  q->setAutoWindowLevel(qMRMLWindowLevelWidget::Auto);

  QObject::connect(this->WindowLevelRangeSlider, SIGNAL(valuesChanged(double,double)),
                   q, SLOT(setMinMaxRangeValue(double,double)));

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
}

// --------------------------------------------------------------------------
bool qMRMLWindowLevelWidgetPrivate::blockSignals(bool block)
{
  bool res = this->Superclass::blockSignals(block);
  this->WindowLevelRangeSlider->blockSignals(block);
  this->WindowSpinBox->blockSignals(block);
  this->LevelSpinBox->blockSignals(block);
  this->MinSpinBox->blockSignals(block);
  this->MaxSpinBox->blockSignals(block);
  return res;
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidgetPrivate::setRange(double min, double max)
{
  this->Superclass::setRange(min, max);
  this->WindowLevelRangeSlider->setRange(min, max);
  this->WindowSpinBox->setRange(0, max - min);
  this->LevelSpinBox->setRange(min, max);
  this->MinSpinBox->setRange(min, max);
  this->MaxSpinBox->setRange(min, max);
  //this->RangeWidget->setValues(min, max);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidgetPrivate::setDecimals(int decimals)
{
  this->Superclass::setDecimals(decimals);
  this->WindowSpinBox->setDecimals(decimals);
  this->LevelSpinBox->setDecimals(decimals);
  this->MinSpinBox->setDecimals(decimals);
  this->MaxSpinBox->setDecimals(decimals);
}

// --------------------------------------------------------------------------
void qMRMLWindowLevelWidgetPrivate::setSingleStep(double singleStep)
{
  this->Superclass::setSingleStep(singleStep);
  this->WindowLevelRangeSlider->setSingleStep(singleStep);
  this->WindowSpinBox->setSingleStep(singleStep);
  this->LevelSpinBox->setSingleStep(singleStep);
  this->MinSpinBox->setSingleStep(singleStep);
  this->MaxSpinBox->setSingleStep(singleStep);
}

// --------------------------------------------------------------------------
qMRMLWindowLevelWidget::qMRMLWindowLevelWidget(QWidget* parentWidget)
  : Superclass(new qMRMLWindowLevelWidgetPrivate(*this), parentWidget)
{
  Q_D(qMRMLWindowLevelWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLWindowLevelWidget::~qMRMLWindowLevelWidget() = default;

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
void qMRMLWindowLevelWidget::updateWidgetFromMRMLDisplayNode()
{
  Q_D(qMRMLWindowLevelWidget);
  this->Superclass::updateWidgetFromMRMLDisplayNode();
  if (!d->VolumeDisplayNode)
    {
    return;
    }

  double window = d->VolumeDisplayNode->GetWindow();
  double level = d->VolumeDisplayNode->GetLevel();
  double min = d->VolumeDisplayNode->GetWindowLevelMin();
  double max = d->VolumeDisplayNode->GetWindowLevelMax();

  // We block here to prevent the widgets to call setWindowLevel which could
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

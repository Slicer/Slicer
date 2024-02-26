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

// qMRML includes
#include "qMRMLSpinBox.h"
#include "qMRMLVolumeThresholdWidget.h"
#include "qMRMLVolumeWidget_p.h"
#include "ui_qMRMLVolumeThresholdWidget.h"

// MRML includes
#include "vtkMRMLScalarVolumeDisplayNode.h"

//-----------------------------------------------------------------------------
class qMRMLVolumeThresholdWidgetPrivate
  : public qMRMLVolumeWidgetPrivate
  , public Ui_qMRMLVolumeThresholdWidget
{
  Q_DECLARE_PUBLIC(qMRMLVolumeThresholdWidget);

protected:
  typedef qMRMLVolumeWidgetPrivate Superclass;

public:
  qMRMLVolumeThresholdWidgetPrivate(qMRMLVolumeThresholdWidget& object);
  void init() override;

  bool blockSignals(bool block) override;
  void setRange(double min, double max) override;
  void setDecimals(int decimals) override;
  void setSingleStep(double singleStep) override;
};

// --------------------------------------------------------------------------
qMRMLVolumeThresholdWidgetPrivate ::qMRMLVolumeThresholdWidgetPrivate(qMRMLVolumeThresholdWidget& object)
  : Superclass(object)
{
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidgetPrivate::init()
{
  Q_Q(qMRMLVolumeThresholdWidget);

  this->Superclass::init();
  this->setupUi(q);
  this->VolumeThresholdRangeWidget->minimumSpinBox()->setDecimalsOption(ctkDoubleSpinBox::DecimalsByKey
                                                                        | ctkDoubleSpinBox::DecimalsByShortcuts);
  this->VolumeThresholdRangeWidget->maximumSpinBox()->setDecimalsOption(ctkDoubleSpinBox::DecimalsByKey
                                                                        | ctkDoubleSpinBox::DecimalsByShortcuts);

  q->setAutoThreshold(qMRMLVolumeThresholdWidget::Off);

  this->connect(
    this->VolumeThresholdRangeWidget, SIGNAL(valuesChanged(double, double)), q, SLOT(setThreshold(double, double)));

  this->connect(this->AutoManualComboBox, SIGNAL(currentIndexChanged(int)), q, SLOT(setAutoThreshold(int)));

  this->RangeButton->setMenu(this->OptionsMenu);
  this->RangeButton->setPopupMode(QToolButton::InstantPopup);
}

// --------------------------------------------------------------------------
bool qMRMLVolumeThresholdWidgetPrivate::blockSignals(bool block)
{
  bool res = this->Superclass::blockSignals(block);
  this->VolumeThresholdRangeWidget->blockSignals(block);
  return res;
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidgetPrivate::setRange(double min, double max)
{
  this->Superclass::setRange(min, max);
  this->VolumeThresholdRangeWidget->setRange(min, max);
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidgetPrivate::setDecimals(int decimals)
{
  this->Superclass::setDecimals(decimals);
  this->VolumeThresholdRangeWidget->setDecimals(decimals);
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidgetPrivate::setSingleStep(double singleStep)
{
  this->Superclass::setSingleStep(singleStep);
  this->VolumeThresholdRangeWidget->setSingleStep(singleStep);
}

// --------------------------------------------------------------------------
qMRMLVolumeThresholdWidget::qMRMLVolumeThresholdWidget(QWidget* parentWidget)
  : Superclass(new qMRMLVolumeThresholdWidgetPrivate(*this), parentWidget)
{
  Q_D(qMRMLVolumeThresholdWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLVolumeThresholdWidget::~qMRMLVolumeThresholdWidget() = default;

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setThresholdBounds(double min, double max)
{
  Q_D(qMRMLVolumeThresholdWidget);
  d->setRange(min, max);
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setAutoThreshold(int autoThreshold)
{
  this->setAutoThreshold(static_cast<ControlMode>(autoThreshold));
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setAutoThreshold(ControlMode autoThreshold)
{
  Q_D(qMRMLVolumeThresholdWidget);

  if (!d->VolumeDisplayNode)
  {
    return;
  }
  int oldAuto = d->VolumeDisplayNode->GetAutoThreshold();
  int oldApply = d->VolumeDisplayNode->GetApplyThreshold();

  int disabledModify = d->VolumeDisplayNode->StartModify();
  if (autoThreshold == qMRMLVolumeThresholdWidget::Off)
  {
    d->VolumeDisplayNode->SetApplyThreshold(0);
  }
  else
  {
    d->VolumeDisplayNode->SetApplyThreshold(1);
    d->VolumeDisplayNode->SetAutoThreshold(autoThreshold == qMRMLVolumeThresholdWidget::Auto ? 1 : 0);
  }

  if (!oldApply && autoThreshold == qMRMLVolumeThresholdWidget::Manual)
  {
    // Previously the threshold was turned off and now it is set to manual.
    // Since the default threshold range is VTK_SHORT_MIN to VTK_SHORT_MAX,
    // we don't want these values to appear on the GUI but instead set
    // the threshold range to the full scalar range of the volume (because
    // this corresponds to the previous state of the thresholding: having
    // the full scalar range of the volume in the threshold range).
    d->VolumeDisplayNode->SetThreshold(d->DisplayScalarRange[0], d->DisplayScalarRange[1]);
  }

  d->VolumeDisplayNode->EndModify(disabledModify);

  if (oldAuto != d->VolumeDisplayNode->GetAutoThreshold() || oldApply != d->VolumeDisplayNode->GetApplyThreshold())
  {
    emit this->autoThresholdValueChanged(autoThreshold);
  }
}

// --------------------------------------------------------------------------
qMRMLVolumeThresholdWidget::ControlMode qMRMLVolumeThresholdWidget ::autoThreshold() const
{
  Q_D(const qMRMLVolumeThresholdWidget);
  return static_cast<ControlMode>(d->AutoManualComboBox->currentIndex());
}

// --------------------------------------------------------------------------
bool qMRMLVolumeThresholdWidget::isOff() const
{
  Q_D(const qMRMLVolumeThresholdWidget);
  return d->AutoManualComboBox->currentIndex() == qMRMLVolumeThresholdWidget::Off;
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setThreshold(double lowerThreshold, double upperThreshold)
{
  Q_D(qMRMLVolumeThresholdWidget);
  if (d->VolumeDisplayNode)
  {
    double oldLowerThreshold = d->VolumeDisplayNode->GetLowerThreshold();
    double oldUpperThreshold = d->VolumeDisplayNode->GetUpperThreshold();

    int wasModify = d->VolumeDisplayNode->StartModify();
    d->VolumeDisplayNode->SetLowerThreshold(lowerThreshold);
    d->VolumeDisplayNode->SetUpperThreshold(upperThreshold);
    bool changed = (oldLowerThreshold != d->VolumeDisplayNode->GetLowerThreshold()
                    || oldUpperThreshold != d->VolumeDisplayNode->GetUpperThreshold());
    if (changed)
    {
      this->setAutoThreshold(qMRMLVolumeThresholdWidget::Manual);
      emit this->thresholdValuesChanged(lowerThreshold, upperThreshold);
    }
    d->VolumeDisplayNode->EndModify(wasModify);
  }
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setLowerThreshold(double lowerThreshold)
{
  Q_D(qMRMLVolumeThresholdWidget);
  if (d->VolumeDisplayNode)
  {
    double upperThreshold = d->VolumeDisplayNode->GetUpperThreshold();
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
  return d->VolumeThresholdRangeWidget->minimumValue();
}

// --------------------------------------------------------------------------
double qMRMLVolumeThresholdWidget::upperThreshold() const
{
  Q_D(const qMRMLVolumeThresholdWidget);
  return d->VolumeThresholdRangeWidget->maximumValue();
}

// --------------------------------------------------------------------------
double qMRMLVolumeThresholdWidget::lowerThresholdBound() const
{
  Q_D(const qMRMLVolumeThresholdWidget);
  return d->VolumeThresholdRangeWidget->minimum();
}

// --------------------------------------------------------------------------
double qMRMLVolumeThresholdWidget::upperThresholdBound() const
{
  Q_D(const qMRMLVolumeThresholdWidget);
  return d->VolumeThresholdRangeWidget->maximum();
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setLowerThresholdBound(double lowerThresholdBound)
{
  double upperThresholdBound = this->upperThresholdBound();
  this->setThresholdBounds(lowerThresholdBound, upperThresholdBound);
}

// --------------------------------------------------------------------------
void qMRMLVolumeThresholdWidget::setUpperThresholdBound(double upperThresholdBound)
{
  double lowerThresholdBound = this->lowerThresholdBound();
  this->setThresholdBounds(lowerThresholdBound, upperThresholdBound);
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
void qMRMLVolumeThresholdWidget::updateWidgetFromMRMLDisplayNode()
{
  Q_D(qMRMLVolumeThresholdWidget);
  Superclass::updateWidgetFromMRMLDisplayNode();

  if (!d->VolumeDisplayNode)
  {
    return;
  }

  // We don't want the slider to fire signals saying that the threshold values
  // have changed, it would set the AutoThrehold mode to Manual automatically
  // even if the values have been just set programmatically/automatically.
  bool wasBlocking = d->VolumeThresholdRangeWidget->blockSignals(true);

  const int autoThresh = d->VolumeDisplayNode->GetAutoThreshold();
  const int applyThresh = d->VolumeDisplayNode->GetApplyThreshold();
  // 0 = auto, 1 = manual, 2 = off
  ControlMode index = (applyThresh == 0)  ? qMRMLVolumeThresholdWidget::Off
                      : (autoThresh == 1) ? qMRMLVolumeThresholdWidget::Auto
                                          : qMRMLVolumeThresholdWidget::Manual;
  d->AutoManualComboBox->setCurrentIndex(index);

  if (applyThresh)
  {
    // Thresholding is on.
    // Show the threshold values (and widen the slider's range if needed).
    double range[2] = { 0.0, 0.0 };
    d->VolumeThresholdRangeWidget->range(range);
    double lower = d->VolumeDisplayNode->GetLowerThreshold();
    double upper = d->VolumeDisplayNode->GetUpperThreshold();
    double minRangeValue = std::min(range[0], lower);
    double maxRangeValue = std::max(range[1], upper);
    d->setRange(minRangeValue, maxRangeValue);
    d->VolumeThresholdRangeWidget->setValues(lower, upper);
  }
  else
  {
    // Thresholding is off.
    // Instead of showing the threshold values (which may be very large),
    // show the volume's entire scalar range.
    double minRangeValue = d->DisplayScalarRange[0];
    double maxRangeValue = d->DisplayScalarRange[1];
    d->setRange(minRangeValue, maxRangeValue);
    d->VolumeThresholdRangeWidget->setValues(minRangeValue, maxRangeValue);
  }
  d->VolumeThresholdRangeWidget->blockSignals(wasBlocking);
}

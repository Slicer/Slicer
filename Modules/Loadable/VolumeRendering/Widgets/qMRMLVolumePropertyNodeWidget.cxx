/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes

// qMRML includes
#include "qMRMLVolumePropertyNodeWidget.h"
#include "ui_qMRMLVolumePropertyNodeWidget.h"

// MRML includes
#include <vtkMRMLVolumePropertyNode.h>

// VTK includes
#include <vtkVolumeProperty.h>
#include <vtkWeakPointer.h>

// Qt includes
#include <QButtonGroup>

//-----------------------------------------------------------------------------
class qMRMLVolumePropertyNodeWidgetPrivate : public Ui_qMRMLVolumePropertyNodeWidget
{
  Q_DECLARE_PUBLIC(qMRMLVolumePropertyNodeWidget);

protected:
  qMRMLVolumePropertyNodeWidget* const q_ptr;

public:
  qMRMLVolumePropertyNodeWidgetPrivate(qMRMLVolumePropertyNodeWidget& object);
  virtual ~qMRMLVolumePropertyNodeWidgetPrivate();

  virtual void setupUi();

  vtkWeakPointer<vtkMRMLVolumePropertyNode> VolumePropertyNode;
  int ComponentCount{ 1 };
  QButtonGroup* ComponentsButtonGroup;
};

// --------------------------------------------------------------------------
qMRMLVolumePropertyNodeWidgetPrivate::qMRMLVolumePropertyNodeWidgetPrivate(qMRMLVolumePropertyNodeWidget& object)
  : q_ptr(&object)
{
  this->VolumePropertyNode = nullptr;
}

// --------------------------------------------------------------------------
qMRMLVolumePropertyNodeWidgetPrivate::~qMRMLVolumePropertyNodeWidgetPrivate() = default;

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidgetPrivate::setupUi()
{
  Q_Q(qMRMLVolumePropertyNodeWidget);
  this->Ui_qMRMLVolumePropertyNodeWidget::setupUi(q);
  QObject::connect(this->VolumePropertyWidget, SIGNAL(chartsExtentChanged()), q, SIGNAL(chartsExtentChanged()));
  QObject::connect(this->VolumePropertyWidget, SIGNAL(thresholdEnabledChanged(bool)), q, SIGNAL(thresholdChanged(bool)));
  QObject::connect(this->ComponentSpinBox, SIGNAL(valueChanged(int)), this->VolumePropertyWidget, SLOT(setCurrentComponent(int)));

  this->ComponentsButtonGroup = new QButtonGroup(q);
  this->ComponentsButtonGroup->addButton(this->RGBColorsRadioButton);
  this->ComponentsButtonGroup->addButton(this->IndependentRadioButton);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  QObject::connect(this->ComponentsButtonGroup, &QButtonGroup::buttonToggled, q, &qMRMLVolumePropertyNodeWidget::updateIndependentComponents);
#else
  QObject::connect(this->ComponentsButtonGroup, SIGNAL(buttonToggled(int, bool)), q, SLOT(updateIndependentComponents()));
#endif
}

// --------------------------------------------------------------------------
// qMRMLVolumePropertyNodeWidget
// --------------------------------------------------------------------------
qMRMLVolumePropertyNodeWidget::qMRMLVolumePropertyNodeWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLVolumePropertyNodeWidgetPrivate(*this))
{
  Q_D(qMRMLVolumePropertyNodeWidget);
  d->setupUi();
}

// --------------------------------------------------------------------------
qMRMLVolumePropertyNodeWidget::~qMRMLVolumePropertyNodeWidget() = default;

// --------------------------------------------------------------------------
vtkVolumeProperty* qMRMLVolumePropertyNodeWidget::volumeProperty() const
{
  Q_D(const qMRMLVolumePropertyNodeWidget);
  return d->VolumePropertyWidget->volumeProperty();
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget::setMRMLVolumePropertyNode(vtkMRMLNode* volumePropertyNode)
{
  this->setMRMLVolumePropertyNode(vtkMRMLVolumePropertyNode::SafeDownCast(volumePropertyNode));
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget::setMRMLVolumePropertyNode(vtkMRMLVolumePropertyNode* volumePropertyNode)
{
  Q_D(qMRMLVolumePropertyNodeWidget);
  this->qvtkReconnect(d->VolumePropertyNode, volumePropertyNode, vtkCommand::ModifiedEvent, this, SLOT(updateFromVolumePropertyNode()));
  d->VolumePropertyNode = volumePropertyNode;
  this->updateFromVolumePropertyNode();
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget::updateFromVolumePropertyNode()
{
  Q_D(qMRMLVolumePropertyNodeWidget);
  vtkVolumeProperty* newVolumeProperty = d->VolumePropertyNode ? d->VolumePropertyNode->GetVolumeProperty() : nullptr;
  qvtkReconnect(d->VolumePropertyWidget->volumeProperty(), newVolumeProperty, vtkCommand::ModifiedEvent, this, SIGNAL(volumePropertyChanged()));
  d->VolumePropertyWidget->setVolumeProperty(newVolumeProperty);

  bool independentComponents = newVolumeProperty ? newVolumeProperty->GetIndependentComponents() : false;
  int currentComponent = 0;
  if (independentComponents)
  {
    currentComponent = d->ComponentSpinBox->value();
  }
  d->VolumePropertyWidget->setCurrentComponent(currentComponent);

  bool wasBlocking = d->ComponentsButtonGroup->blockSignals(true);
  d->RGBColorsRadioButton->setChecked(!independentComponents);
  d->IndependentRadioButton->setChecked(independentComponents);
  d->ComponentsButtonGroup->blockSignals(wasBlocking);

  d->RGBColorsRadioButton->setEnabled(d->ComponentCount == 3 || d->ComponentCount == 4);

  bool independentComponentsVisible = newVolumeProperty && d->ComponentCount > 1;
  d->IndependentComponentsLabel->setVisible(independentComponentsVisible);
  d->RGBColorsRadioButton->setVisible(independentComponentsVisible);
  d->IndependentRadioButton->setVisible(independentComponentsVisible);

  d->ComponentLabel->setVisible(independentComponentsVisible);
  d->ComponentSpinBox->setVisible(independentComponentsVisible);
  d->ComponentLabel->setEnabled(independentComponents);
  d->ComponentSpinBox->setEnabled(independentComponents);
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget::chartsBounds(double bounds[4]) const
{
  Q_D(const qMRMLVolumePropertyNodeWidget);
  d->VolumePropertyWidget->chartsBounds(bounds);
}

// ----------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget::setChartsExtent(double extent[2])
{
  Q_D(qMRMLVolumePropertyNodeWidget);
  d->VolumePropertyWidget->chartsExtent(extent);
}

// ----------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget::setChartsExtent(double min, double max)
{
  Q_D(qMRMLVolumePropertyNodeWidget);
  d->VolumePropertyWidget->setChartsExtent(min, max);
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget::chartsExtent(double extent[4]) const
{
  Q_D(const qMRMLVolumePropertyNodeWidget);
  d->VolumePropertyWidget->chartsExtent(extent);
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget::setThreshold(bool enable)
{
  Q_D(qMRMLVolumePropertyNodeWidget);
  d->VolumePropertyWidget->setThresholdEnabled(enable);
}

// --------------------------------------------------------------------------
bool qMRMLVolumePropertyNodeWidget::hasThreshold() const
{
  Q_D(const qMRMLVolumePropertyNodeWidget);
  return d->VolumePropertyWidget->isThresholdEnabled();
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget::moveAllPoints(double x, double y, bool dontMoveFirstAndLast)
{
  Q_D(const qMRMLVolumePropertyNodeWidget);
  return d->VolumePropertyWidget->moveAllPoints(x, y, dontMoveFirstAndLast);
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget::spreadAllPoints(double factor, bool dontSpreadFirstAndLast)
{
  Q_D(const qMRMLVolumePropertyNodeWidget);
  return d->VolumePropertyWidget->spreadAllPoints(factor, dontSpreadFirstAndLast);
}

// --------------------------------------------------------------------------
int qMRMLVolumePropertyNodeWidget::componentCount() const
{
  Q_D(const qMRMLVolumePropertyNodeWidget);
  return d->ComponentCount;
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget::setComponentCount(int componentCount)
{
  Q_D(qMRMLVolumePropertyNodeWidget);

  // Limit the component count to the max range for volume rendering in VTK
  d->ComponentCount = std::clamp(componentCount, 0, VTK_MAX_VRCOMP);
  d->ComponentSpinBox->setRange(0, d->ComponentCount - 1);

  int currentComponent = d->VolumePropertyWidget->currentComponent();
  currentComponent = std::clamp(currentComponent, 0, d->ComponentCount - 1);
  d->VolumePropertyWidget->setCurrentComponent(currentComponent);

  this->updateFromVolumePropertyNode();
}

// --------------------------------------------------------------------------
void qMRMLVolumePropertyNodeWidget::updateIndependentComponents()
{
  Q_D(qMRMLVolumePropertyNodeWidget);
  if (!d->VolumePropertyNode)
  {
    return;
  }
  vtkVolumeProperty* volumeProperty = d->VolumePropertyNode->GetVolumeProperty();
  if (!volumeProperty)
  {
    return;
  }

  MRMLNodeModifyBlocker blocker(d->VolumePropertyNode);
  bool indepdendentComponents = d->IndependentRadioButton->isChecked();
  volumeProperty->SetIndependentComponents(indepdendentComponents);
}

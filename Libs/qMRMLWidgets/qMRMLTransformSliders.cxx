/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qMRMLTransformSliders.h"
#include "ui_qMRMLTransformSliders.h"

// Qt includes
#include <QDebug>
#include <QStack>

// qMRML includes
#include "qMRMLUtils.h"
#include "qMRMLLinearTransformSlider.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"

// VTK includes
#include "vtkSmartPointer.h"
#include "vtkTransform.h"
#include "vtkMatrix4x4.h"


//-----------------------------------------------------------------------------
class qMRMLTransformSlidersPrivate: public Ui_qMRMLTransformSliders
{
public:
  qMRMLTransformSlidersPrivate()
    {
    this->MRMLTransformNode = 0;
    }
    
  qMRMLTransformSliders::TransformType   TypeOfTransform;
  vtkMRMLLinearTransformNode*            MRMLTransformNode;
  QStack<qMRMLLinearTransformSlider*>    ActiveSliders;
};

// --------------------------------------------------------------------------
qMRMLTransformSliders::qMRMLTransformSliders(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLTransformSlidersPrivate)
{
  Q_D(qMRMLTransformSliders);
  
  d->setupUi(this);

  this->setCoordinateReference(qMRMLTransformSliders::GLOBAL);
  this->setTypeOfTransform(qMRMLTransformSliders::TRANSLATION);

  
  this->connect(d->LRSlider, SIGNAL(valueChanged(double)),
                SLOT(onLRSliderPositionChanged(double)));
  this->connect(d->PASlider, SIGNAL(valueChanged(double)),
                SLOT(onPASliderPositionChanged(double)));
  this->connect(d->ISSlider, SIGNAL(valueChanged(double)),
                SLOT(onISSliderPositionChanged(double)));
  
  this->connect(d->MinValueSpinBox, SIGNAL(valueChanged(double)),
                SLOT(onMinimumChanged(double)));
  this->connect(d->MaxValueSpinBox, SIGNAL(valueChanged(double)),
                SLOT(onMaximumChanged(double)));
  // the default values of min and max are set in the .ui file
  this->onMinimumChanged(d->MinValueSpinBox->value());
  this->onMaximumChanged(d->MaxValueSpinBox->value());

  // disable as there is not MRML Node associated with the widget
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
qMRMLTransformSliders::~qMRMLTransformSliders()
{
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setCoordinateReference(CoordinateReferenceType _coordinateReference)
{
  Q_D(qMRMLTransformSliders);
  
  qMRMLLinearTransformSlider::CoordinateReferenceType ref = qMRMLLinearTransformSlider::GLOBAL;
  if (_coordinateReference == LOCAL)
    {
    ref = qMRMLLinearTransformSlider::LOCAL;
    }
  d->LRSlider->setCoordinateReference(ref);
  d->PASlider->setCoordinateReference(ref);
  d->ISSlider->setCoordinateReference(ref);
}

// --------------------------------------------------------------------------
qMRMLTransformSliders::CoordinateReferenceType qMRMLTransformSliders::coordinateReference() const
{
  Q_D(const qMRMLTransformSliders);
  
  // Assumes settings of the sliders are all the same
  qMRMLLinearTransformSlider::CoordinateReferenceType ref =
    d->LRSlider->coordinateReference();
  return (ref == qMRMLLinearTransformSlider::GLOBAL) ? GLOBAL : LOCAL;
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setTypeOfTransform(TransformType _typeOfTransform)
{
  Q_D(qMRMLTransformSliders);
  
  if (d->TypeOfTransform == _typeOfTransform) { return; }
  if (_typeOfTransform == qMRMLTransformSliders::TRANSLATION)
    {
    d->LRSlider->setTypeOfTransform(qMRMLLinearTransformSlider::TRANSLATION_LR);
    d->PASlider->setTypeOfTransform(qMRMLLinearTransformSlider::TRANSLATION_PA);
    d->ISSlider->setTypeOfTransform(qMRMLLinearTransformSlider::TRANSLATION_IS);
    }
  else if (_typeOfTransform == qMRMLTransformSliders::ROTATION)
    {
    d->LRSlider->setTypeOfTransform(qMRMLLinearTransformSlider::ROTATION_LR);
    d->PASlider->setTypeOfTransform(qMRMLLinearTransformSlider::ROTATION_PA);
    d->ISSlider->setTypeOfTransform(qMRMLLinearTransformSlider::ROTATION_IS);
    }
  d->TypeOfTransform = _typeOfTransform;
}

// --------------------------------------------------------------------------
qMRMLTransformSliders::TransformType qMRMLTransformSliders::typeOfTransform() const
{
  Q_D(const qMRMLTransformSliders);
  return d->TypeOfTransform;
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setMRMLTransformNode(vtkMRMLNode* node)
{
  this->setMRMLTransformNode(vtkMRMLLinearTransformNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setMRMLTransformNode(vtkMRMLLinearTransformNode* transformNode)
{
  Q_D(qMRMLTransformSliders);
  
  d->LRSlider->setMRMLTransformNode(transformNode);
  d->PASlider->setMRMLTransformNode(transformNode);
  d->ISSlider->setMRMLTransformNode(transformNode);
  this->setEnabled(transformNode != 0);
  d->MRMLTransformNode = transformNode;
}

// --------------------------------------------------------------------------
CTK_GET_CPP(qMRMLTransformSliders, vtkMRMLLinearTransformNode*, mrmlTransformNode, MRMLTransformNode);

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setTitle(const QString& _title)
{
  Q_D(qMRMLTransformSliders);
  d->SlidersGroupBox->setTitle(_title);
}

// --------------------------------------------------------------------------
QString qMRMLTransformSliders::title()const
{
  Q_D(const qMRMLTransformSliders);
  return d->SlidersGroupBox->title();
}

// --------------------------------------------------------------------------
double qMRMLTransformSliders::minimum()const
{
  Q_D(const qMRMLTransformSliders);
  return d->MinValueSpinBox->value();
}

// --------------------------------------------------------------------------
double qMRMLTransformSliders::maximum()const
{
  Q_D(const qMRMLTransformSliders);
  return d->MaxValueSpinBox->value();
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setMinimum(double min)
{
  Q_D(qMRMLTransformSliders);
  d->MinValueSpinBox->setValue(min);
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setMaximum(double max)
{
  Q_D(qMRMLTransformSliders);
  d->MaxValueSpinBox->setValue(max);
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setRange(double min, double max)
{
  Q_D(qMRMLTransformSliders);
  
  // Could be optimized here by blocking signals on spinboxes and manually
  // call the setRange method on the sliders. Does it really worth it ?
  d->MinValueSpinBox->setValue(min);
  d->MaxValueSpinBox->setValue(max);
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::onMinimumChanged(double min)
{
  Q_D(qMRMLTransformSliders);
  
  d->LRSlider->setMinimum(min);
  d->PASlider->setMinimum(min);
  d->ISSlider->setMinimum(min);

  emit this->minimumChanged(min);
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::onMaximumChanged(double max)
{
  Q_D(qMRMLTransformSliders);
  
  d->LRSlider->setMaximum(max);
  d->PASlider->setMaximum(max);
  d->ISSlider->setMaximum(max);

  emit this->maximumChanged(max);
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setMinMaxVisible(bool visible)
{
  Q_D(qMRMLTransformSliders);
  d->MinMaxWidget->setVisible(visible);
}

// --------------------------------------------------------------------------
bool qMRMLTransformSliders::isMinMaxVisible()const
{
  Q_D(const qMRMLTransformSliders);
  return d->MinMaxWidget->isVisibleTo(
    const_cast<qMRMLTransformSliders*>(this));
}

// --------------------------------------------------------------------------
double qMRMLTransformSliders::singleStep()const
{
  Q_D(const qMRMLTransformSliders);
  // Assumes settings of the sliders are all the same
  return d->PASlider->singleStep();
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setSingleStep(double step)
{
  Q_D(qMRMLTransformSliders);
  
  d->LRSlider->setSingleStep(step);
  d->PASlider->setSingleStep(step);
  d->ISSlider->setSingleStep(step);
}

// --------------------------------------------------------------------------
QString qMRMLTransformSliders::lrLabel()const
{
  Q_D(const qMRMLTransformSliders);
  return d->LRLabel->text();
}

// --------------------------------------------------------------------------
QString qMRMLTransformSliders::paLabel()const
{
  Q_D(const qMRMLTransformSliders);
  return d->PALabel->text();
}

// --------------------------------------------------------------------------
QString qMRMLTransformSliders::isLabel()const
{
  Q_D(const qMRMLTransformSliders);
  return d->ISLabel->text();
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setLRLabel(const QString& label)
{
  Q_D(qMRMLTransformSliders);
  d->LRLabel->setText(label);
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setPALabel(const QString& label)
{
  Q_D(qMRMLTransformSliders);
  d->PALabel->setText(label);
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setISLabel(const QString& label)
{
  Q_D(qMRMLTransformSliders);
  d->ISLabel->setText(label);
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::reset()
{
  Q_D(qMRMLTransformSliders);
  
  d->LRSlider->reset();
  d->PASlider->reset();
  d->ISSlider->reset();
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::resetUnactiveSliders()
{
  Q_D(qMRMLTransformSliders);
  
  if (!d->ActiveSliders.contains(d->LRSlider))
    {
    d->LRSlider->blockSignals(true);
    d->LRSlider->reset();
    d->LRSlider->blockSignals(false);
    }
  if (!d->ActiveSliders.contains(d->PASlider))
    {
    d->PASlider->blockSignals(true);
    d->PASlider->reset();
    d->PASlider->blockSignals(false);
    }
  if (!d->ActiveSliders.contains(d->ISSlider))
    {
    d->ISSlider->blockSignals(true);
    d->ISSlider->reset();
    d->ISSlider->blockSignals(false);
    }
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::onLRSliderPositionChanged(double position)
{
  Q_D(qMRMLTransformSliders);
  d->ActiveSliders.push(d->LRSlider);

  if (this->typeOfTransform() == qMRMLTransformSliders::ROTATION)
    {
    // Reset other sliders
    d->PASlider->blockSignals(true);
    d->PASlider->reset();
    d->PASlider->blockSignals(false);
    d->ISSlider->blockSignals(true);
    d->ISSlider->reset();
    d->ISSlider->blockSignals(false);
    }
  // Update LR slider
  d->LRSlider->applyTransformation(position);
  emit this->valuesChanged();

  d->ActiveSliders.pop();
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::onPASliderPositionChanged(double position)
{
  Q_D(qMRMLTransformSliders);
  d->ActiveSliders.push(d->PASlider);

  if (this->typeOfTransform() == qMRMLTransformSliders::ROTATION)
    {
    // Reset other sliders
    d->LRSlider->blockSignals(true);
    d->LRSlider->reset();
    d->LRSlider->blockSignals(false);
    d->ISSlider->blockSignals(true);
    d->ISSlider->reset();
    d->ISSlider->blockSignals(false);
    }
  // Update PA slider
  d->PASlider->applyTransformation(position);
  emit this->valuesChanged();
  d->ActiveSliders.pop();
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::onISSliderPositionChanged(double position)
{
  Q_D(qMRMLTransformSliders);
   d->ActiveSliders.push(d->ISSlider);
  if (this->typeOfTransform() == qMRMLTransformSliders::ROTATION)
    {
    // Reset other sliders
    d->LRSlider->blockSignals(true);
    d->LRSlider->reset();
    d->LRSlider->blockSignals(false);
    d->PASlider->blockSignals(true);
    d->PASlider->reset();
    d->PASlider->blockSignals(false);
    }
  
  // Update IS slider
  d->ISSlider->applyTransformation(position);
  emit this->valuesChanged();
  d->ActiveSliders.pop();
}

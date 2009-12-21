
#include "qMRMLTransformSliders.h"
#include "ui_qMRMLTransformSliders.h"

// qMRML includes
#include "qMRMLUtils.h"
#include "qMRMLLinearTransformSlider.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"

// VTK includes
#include "vtkSmartPointer.h"
#include "vtkTransform.h"
#include "vtkMatrix4x4.h"

// QT includes
#include <QDebug>

//-----------------------------------------------------------------------------
class qMRMLTransformSlidersPrivate: public qCTKPrivate<qMRMLTransformSliders>,
                                     public Ui_qMRMLTransformSliders
{
public:
  qMRMLTransformSlidersPrivate()
    {
    this->MRMLTransformNode = 0;
    }
    
  qMRMLTransformSliders::TransformType   TypeOfTransform;
  vtkMRMLLinearTransformNode*            MRMLTransformNode;
};

// --------------------------------------------------------------------------
qMRMLTransformSliders::qMRMLTransformSliders(QWidget* parent) : Superclass(parent)
{
  QCTK_INIT_PRIVATE(qMRMLTransformSliders);
  QCTK_D(qMRMLTransformSliders);
  
  d->setupUi(this);

  this->setCoordinateReference(Self::GLOBAL);
  this->setTypeOfTransform(Self::TRANSLATION);


  this->connect(d->LRSlider, SIGNAL(sliderMoved(double)),
                SLOT(onLRSliderPositionChanged(double)));
  this->connect(d->PASlider, SIGNAL(sliderMoved(double)),
                SLOT(onPASliderPositionChanged(double)));
  this->connect(d->ISSlider, SIGNAL(sliderMoved(double)),
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
void qMRMLTransformSliders::setCoordinateReference(CoordinateReferenceType coordinateReference)
{
  QCTK_D(qMRMLTransformSliders);
  
  qMRMLLinearTransformSlider::CoordinateReferenceType ref = qMRMLLinearTransformSlider::GLOBAL;
  if (coordinateReference == LOCAL)
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
  QCTK_D(const qMRMLTransformSliders);
  
  // Assumes settings of the sliders are all the same
  qMRMLLinearTransformSlider::CoordinateReferenceType ref =
    d->LRSlider->coordinateReference();
  return (ref == qMRMLLinearTransformSlider::GLOBAL) ? GLOBAL : LOCAL;
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setTypeOfTransform(TransformType typeOfTransform)
{
  QCTK_D(qMRMLTransformSliders);
  
  if (d->TypeOfTransform == typeOfTransform) { return; }
  if (typeOfTransform == Self::TRANSLATION)
    {
    d->LRSlider->setTypeOfTransform(qMRMLLinearTransformSlider::TRANSLATION_LR);
    d->PASlider->setTypeOfTransform(qMRMLLinearTransformSlider::TRANSLATION_PA);
    d->ISSlider->setTypeOfTransform(qMRMLLinearTransformSlider::TRANSLATION_IS);
    }
  else if (typeOfTransform == Self::ROTATION)
    {
    d->LRSlider->setTypeOfTransform(qMRMLLinearTransformSlider::ROTATION_LR);
    d->PASlider->setTypeOfTransform(qMRMLLinearTransformSlider::ROTATION_PA);
    d->ISSlider->setTypeOfTransform(qMRMLLinearTransformSlider::ROTATION_IS);
    }
  d->TypeOfTransform = typeOfTransform;
}

// --------------------------------------------------------------------------
qMRMLTransformSliders::TransformType qMRMLTransformSliders::typeOfTransform() const
{
  return qctk_d()->TypeOfTransform;
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setMRMLTransformNode(vtkMRMLNode* node)
{
  this->setMRMLTransformNode(vtkMRMLLinearTransformNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setMRMLTransformNode(vtkMRMLLinearTransformNode* transformNode)
{
  QCTK_D(qMRMLTransformSliders);
  
  d->LRSlider->setMRMLTransformNode(transformNode);
  d->PASlider->setMRMLTransformNode(transformNode);
  d->ISSlider->setMRMLTransformNode(transformNode);
  this->setEnabled(transformNode != 0);
  d->MRMLTransformNode = transformNode;
}

// --------------------------------------------------------------------------
QCTK_GET_CXX(qMRMLTransformSliders, vtkMRMLLinearTransformNode*, mrmlTransformNode, MRMLTransformNode);

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setTitle(const QString& title)
{
  qctk_d()->SlidersGroupBox->setTitle(title);
}

// --------------------------------------------------------------------------
QString qMRMLTransformSliders::title()const
{
  return qctk_d()->SlidersGroupBox->title();
}

// --------------------------------------------------------------------------
double qMRMLTransformSliders::minimum()const
{
  return qctk_d()->MinValueSpinBox->value();
}

// --------------------------------------------------------------------------
double qMRMLTransformSliders::maximum()const
{
  return qctk_d()->MaxValueSpinBox->value();
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setMinimum(double min)
{
  qctk_d()->MinValueSpinBox->setValue(min);
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setMaximum(double max)
{
  qctk_d()->MaxValueSpinBox->setValue(max);
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setRange(double min, double max)
{
  QCTK_D(qMRMLTransformSliders);
  
  // Could be optimized here by blocking signals on spinboxes and manually
  // call the setRange method on the sliders. Does it really worth it ?
  d->MinValueSpinBox->setValue(min);
  d->MaxValueSpinBox->setValue(max);
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::onMinimumChanged(double min)
{
  QCTK_D(qMRMLTransformSliders);
  
  d->LRSlider->setMinimum(min);
  d->PASlider->setMinimum(min);
  d->ISSlider->setMinimum(min);
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::onMaximumChanged(double max)
{
  QCTK_D(qMRMLTransformSliders);
  
  d->LRSlider->setMaximum(max);
  d->PASlider->setMaximum(max);
  d->ISSlider->setMaximum(max);
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setMinMaxVisible(bool visible)
{
  qctk_d()->MinMaxWidget->setVisible(visible);
}

// --------------------------------------------------------------------------
bool qMRMLTransformSliders::isMinMaxVisible()const
{
  return qctk_d()->MinMaxWidget->isVisibleTo(
    const_cast<qMRMLTransformSliders*>(this));
}

// --------------------------------------------------------------------------
double qMRMLTransformSliders::singleStep()const
{
  // Assumes settings of the sliders are all the same
  return qctk_d()->PASlider->singleStep();
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setSingleStep(double step)
{
  QCTK_D(qMRMLTransformSliders);
  
  d->LRSlider->setSingleStep(step);
  d->PASlider->setSingleStep(step);
  d->ISSlider->setSingleStep(step);
}

// --------------------------------------------------------------------------
QString qMRMLTransformSliders::getLRLabel()const
{
  return qctk_d()->LRLabel->text();
}

// --------------------------------------------------------------------------
QString qMRMLTransformSliders::getPALabel()const
{
  return qctk_d()->PALabel->text();
}

// --------------------------------------------------------------------------
QString qMRMLTransformSliders::getISLabel()const
{
  return qctk_d()->ISLabel->text();
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setLRLabel(const QString& label)
{
  qctk_d()->LRLabel->setText(label);
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setPALabel(const QString& label)
{
  qctk_d()->PALabel->setText(label);
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::setISLabel(const QString& label)
{
  qctk_d()->ISLabel->setText(label);
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::reset()
{
  QCTK_D(qMRMLTransformSliders);
  
  d->LRSlider->reset();
  d->PASlider->reset();
  d->ISSlider->reset();
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::onLRSliderPositionChanged(double position)
{
  QCTK_D(qMRMLTransformSliders);
  
  if (this->typeOfTransform() == Self::ROTATION)
    {
    // Reset other sliders
    d->PASlider->reset();
    d->ISSlider->reset();
    }
  else
    {
    emit this->sliderMoved();
    }
  // Update LR slider
  d->LRSlider->applyTransformation(position);
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::onPASliderPositionChanged(double position)
{
  QCTK_D(qMRMLTransformSliders);
  
  if (this->typeOfTransform() == Self::ROTATION)
    {
    // Reset other sliders
    d->LRSlider->reset();
    d->ISSlider->reset();
    }
  else
    {
    emit this->sliderMoved();
    }
  // Update PA slider
  d->PASlider->applyTransformation(position);
}

// --------------------------------------------------------------------------
void qMRMLTransformSliders::onISSliderPositionChanged(double position)
{
  QCTK_D(qMRMLTransformSliders);
  
  if (this->typeOfTransform() == Self::ROTATION)
    {
    // Reset other sliders
    d->LRSlider->reset();
    d->PASlider->reset();
    }
  else
    {
    emit this->sliderMoved();
    }
  // Update IS slider
  d->ISSlider->applyTransformation(position);
}

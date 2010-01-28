#include "qMRMLLinearTransformSlider.h"

// qMRML includes
#include "qMRMLUtils.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"

// VTK includes
#include "vtkSmartPointer.h"
#include "vtkTransform.h"
#include "vtkMatrix4x4.h"

// QT includes
#include <QDebug>

//-----------------------------------------------------------------------------
class qMRMLLinearTransformSliderPrivate: public qCTKPrivate<qMRMLLinearTransformSlider>
{
public:
  qMRMLLinearTransformSliderPrivate();
  qMRMLLinearTransformSlider::TransformType            TypeOfTransform;
  qMRMLLinearTransformSlider::CoordinateReferenceType  CoordinateReference;
  vtkMRMLLinearTransformNode*                          MRMLTransformNode;
  double                                               OldPosition;
};

// --------------------------------------------------------------------------
qMRMLLinearTransformSliderPrivate::qMRMLLinearTransformSliderPrivate()
{
  this->TypeOfTransform = qMRMLLinearTransformSlider::TRANSLATION_LR;
  this->CoordinateReference = qMRMLLinearTransformSlider::GLOBAL;
  this->MRMLTransformNode = 0;
  this->OldPosition = 0;
}

// --------------------------------------------------------------------------
qMRMLLinearTransformSlider::qMRMLLinearTransformSlider(QWidget* _parent) : Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qMRMLLinearTransformSlider);
}

// --------------------------------------------------------------------------
void qMRMLLinearTransformSlider::setTypeOfTransform(TransformType _typeOfTransform)
{
  QCTK_D(qMRMLLinearTransformSlider);
  d->TypeOfTransform = _typeOfTransform;
  this->onMRMLTransformNodeModified(d->MRMLTransformNode);
}

// --------------------------------------------------------------------------
qMRMLLinearTransformSlider::TransformType qMRMLLinearTransformSlider::typeOfTransform() const
{
  return qctk_d()->TypeOfTransform;
}

// --------------------------------------------------------------------------
bool qMRMLLinearTransformSlider::isRotation()const
{
  return (this->typeOfTransform() == ROTATION_LR ||
          this->typeOfTransform() == ROTATION_PA ||
          this->typeOfTransform() == ROTATION_IS);
}

// --------------------------------------------------------------------------
bool qMRMLLinearTransformSlider::isTranslation()const
{
  return (this->typeOfTransform() == TRANSLATION_LR ||
          this->typeOfTransform() == TRANSLATION_PA ||
          this->typeOfTransform() == TRANSLATION_IS);
}

// --------------------------------------------------------------------------
void qMRMLLinearTransformSlider::
setCoordinateReference(CoordinateReferenceType _coordinateReference)
{
  QCTK_D(qMRMLLinearTransformSlider);
  d->CoordinateReference = _coordinateReference;
  this->onMRMLTransformNodeModified(d->MRMLTransformNode);
}

// --------------------------------------------------------------------------
qMRMLLinearTransformSlider::CoordinateReferenceType qMRMLLinearTransformSlider::coordinateReference() const
{
  return qctk_d()->CoordinateReference;
}

// --------------------------------------------------------------------------
void qMRMLLinearTransformSlider::setMRMLTransformNode(vtkMRMLLinearTransformNode* transformNode)
{
  QCTK_D(qMRMLLinearTransformSlider);
  
  if (d->MRMLTransformNode == transformNode) { return; }

  this->qvtkReconnect(d->MRMLTransformNode, transformNode,
    vtkMRMLTransformableNode::TransformModifiedEvent,
    this, SLOT(onMRMLTransformNodeModified(vtkObject*)));

  d->MRMLTransformNode = transformNode;
  this->onMRMLTransformNodeModified(transformNode);
  // If the node is NULL, any action on the widget is meaningless, this is why
  // the widget is disabled
  this->setEnabled(transformNode != 0);
}

// --------------------------------------------------------------------------
vtkMRMLLinearTransformNode* qMRMLLinearTransformSlider::mrmlTransformNode()const
{
  return qctk_d()->MRMLTransformNode;
}

// --------------------------------------------------------------------------
void qMRMLLinearTransformSlider::onMRMLTransformNodeModified(vtkObject* caller)
{
  QCTK_D(qMRMLLinearTransformSlider);
  
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(caller);
  if (!transformNode) { return; }
  Q_ASSERT(d->MRMLTransformNode == transformNode);

  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  qMRMLUtils::getTransformInCoordinateSystem(d->MRMLTransformNode,
    d->CoordinateReference == Self::GLOBAL, transform);

  vtkMatrix4x4 * matrix = transform->GetMatrix();
  Q_ASSERT(matrix);
  if (!matrix) { return; }

  double _value = 0.0;
  if (this->typeOfTransform() == TRANSLATION_LR)
    {
    _value = matrix->GetElement(0,3);
    }
  else if (this->typeOfTransform() == TRANSLATION_PA)
    {
    _value = matrix->GetElement(1,3);
    }
  else if (this->typeOfTransform() == TRANSLATION_IS)
    {
    _value = matrix->GetElement(2,3);
    }

  if (this->isTranslation())
    {
    d->OldPosition = _value;
    this->setValue(_value);
    }
  else if (this->isRotation())
    {
    d->OldPosition = this->sliderPosition();
    //this->setValue(this->sliderPosition());
    }
}

// --------------------------------------------------------------------------
void qMRMLLinearTransformSlider::applyTransformation(double _sliderPosition)
{
  QCTK_D(qMRMLLinearTransformSlider);
  qDebug() << "Apply Transform: " << _sliderPosition;
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  qMRMLUtils::getTransformInCoordinateSystem(d->MRMLTransformNode,
    d->CoordinateReference == Self::GLOBAL, transform);

  vtkMatrix4x4 * matrix = transform->GetMatrix();
  Q_ASSERT(matrix);
  if (!matrix) { return; }

  if (this->typeOfTransform() == ROTATION_LR)
    {
    double angle = _sliderPosition - d->OldPosition;
    transform->RotateX(angle);
    }
  else if (this->typeOfTransform() == ROTATION_PA)
    {
    double angle = _sliderPosition - d->OldPosition;
    transform->RotateY(angle);
    }
  else if (this->typeOfTransform() == ROTATION_IS)
    {
    double angle = _sliderPosition - d->OldPosition;
    transform->RotateZ(angle);
    }
  else if (this->typeOfTransform() == TRANSLATION_LR)
    {
    double vector[] = {0., 0., 0.};
    vector[0] = _sliderPosition - matrix->GetElement(0,3);
    transform->Translate(vector);
    }
  else if (this->typeOfTransform() == TRANSLATION_PA)
    {
    double vector[] = {0., 0., 0.};
    vector[1] = _sliderPosition - matrix->GetElement(1,3);
    transform->Translate(vector);
    }
  else if (this->typeOfTransform() == TRANSLATION_IS)
    {
    double vector[] = {0., 0., 0.};
    vector[2] = _sliderPosition - matrix->GetElement(2,3);
    transform->Translate(vector);
    }
  d->OldPosition = _sliderPosition;

  d->MRMLTransformNode->GetMatrixTransformToParent()->DeepCopy(
    transform->GetMatrix());
}

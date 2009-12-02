#include "qMRMLMatrixWidget.h"

// qMRML includes
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>

// QT includes
#include <QDebug>

//-----------------------------------------------------------------------------
struct qMRMLMatrixWidgetPrivate: public qCTKPrivate<qMRMLMatrixWidget>
{
  qMRMLMatrixWidgetPrivate()
    {
    this->CoordinateReference = qMRMLMatrixWidget::GLOBAL; 
    this->MRMLTransformNode = 0; 
    }
  
  qMRMLMatrixWidget::CoordinateReferenceType   CoordinateReference;
  vtkMRMLLinearTransformNode*                  MRMLTransformNode;
};

// --------------------------------------------------------------------------
qMRMLMatrixWidget::qMRMLMatrixWidget(QWidget* parent) : Superclass(parent)
{
  QCTK_INIT_PRIVATE(qMRMLMatrixWidget);

  this->setEnabled(false); 
}

// --------------------------------------------------------------------------
void qMRMLMatrixWidget::setCoordinateReference(CoordinateReferenceType coordinateReference)
{
  this->reset(); 
  qctk_d()->CoordinateReference = coordinateReference;
}

// --------------------------------------------------------------------------
qMRMLMatrixWidget::CoordinateReferenceType qMRMLMatrixWidget::coordinateReference() const
{
  return qctk_d()->CoordinateReference;
}

// --------------------------------------------------------------------------
void qMRMLMatrixWidget::setMRMLTransformNode(vtkMRMLNode* node)
{
  this->setMRMLTransformNode(vtkMRMLLinearTransformNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qMRMLMatrixWidget::setMRMLTransformNode(vtkMRMLLinearTransformNode* transformNode)
{
  QCTK_D(qMRMLMatrixWidget);
  
  if (d->MRMLTransformNode == transformNode) { return; }

  this->qvtkReconnect(d->MRMLTransformNode, transformNode,
    vtkMRMLTransformableNode::TransformModifiedEvent, 
    this, SLOT(onMRMLTransformNodeModified(void*,vtkObject*))); 

  d->MRMLTransformNode = transformNode;
  
  //this->reset(); 
  this->onMRMLTransformNodeModified(0, transformNode);

  // Enable/Disable the widget
  this->setEnabled(transformNode != 0); 
}

// --------------------------------------------------------------------------
vtkMRMLLinearTransformNode* qMRMLMatrixWidget::mrmlTransformNode()const
{
  return qctk_d()->MRMLTransformNode;
}

// --------------------------------------------------------------------------
void qMRMLMatrixWidget::onMRMLTransformNodeModified(void* /*call_data*/, vtkObject* caller)
{
  QCTK_D(qMRMLMatrixWidget);
  
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(caller);
  if (!transformNode) { return; }
  Q_ASSERT( d->MRMLTransformNode == transformNode);
  
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  qMRMLUtils::getTransformInCoordinateSystem(d->MRMLTransformNode,
    d->CoordinateReference == Self::GLOBAL, transform);
  
  QVector<double> vector; 
  qMRMLUtils::vtkMatrixToQVector(transform->GetMatrix(), vector); 
  this->setVector( vector ); 
  
}

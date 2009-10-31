#include "qMRMLMatrixWidget.h"
#include "qMRMLUtils.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLLinearTransformNode.h"

#include "vtkSmartPointer.h"
#include "vtkTransform.h"
#include "vtkMatrix4x4.h"

#include <QDebug>

//-----------------------------------------------------------------------------
struct qMRMLMatrixWidget::qInternal
{
public:
  qInternal()
    {
    this->CoordinateReference = GLOBAL; 
    this->MRMLTransformNode = 0; 
    }
  
  CoordinateReferenceType                CoordinateReference; 
  vtkMRMLLinearTransformNode*            MRMLTransformNode; 
};

// --------------------------------------------------------------------------
qMRMLMatrixWidget::qMRMLMatrixWidget(QWidget* parent) : Superclass(parent)
{
  this->Internal = new qInternal;

  this->setEnabled(false); 
}

// --------------------------------------------------------------------------
qMRMLMatrixWidget::~qMRMLMatrixWidget()
{
  delete this->Internal; 
}

// --------------------------------------------------------------------------
void qMRMLMatrixWidget::setCoordinateReference(CoordinateReferenceType coordinateReference)
{
  this->reset(); 
  this->Internal->CoordinateReference = coordinateReference; 
}

// --------------------------------------------------------------------------
qMRMLMatrixWidget::CoordinateReferenceType qMRMLMatrixWidget::coordinateReference() const
{
  return this->Internal->CoordinateReference;
}

// --------------------------------------------------------------------------
void qMRMLMatrixWidget::setMRMLTransformNode(vtkMRMLNode* node)
{
  this->setMRMLTransformNode(vtkMRMLLinearTransformNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qMRMLMatrixWidget::setMRMLTransformNode(vtkMRMLLinearTransformNode* transformNode)
{
  if (this->Internal->MRMLTransformNode == transformNode) { return; }

  this->qvtkReConnect(this->Internal->MRMLTransformNode, transformNode, 
    vtkMRMLTransformableNode::TransformModifiedEvent, 
    this, SLOT(onMRMLTransformNodeModified(void*,vtkObject*))); 

  this->Internal->MRMLTransformNode = transformNode; 
  
  this->reset(); 
  
  // Enable/Disable the widget
  this->setEnabled(transformNode != 0); 
}

// --------------------------------------------------------------------------
vtkMRMLLinearTransformNode* qMRMLMatrixWidget::mrmlTransformNode()const
{
  return this->Internal->MRMLTransformNode;
}

// --------------------------------------------------------------------------
void qMRMLMatrixWidget::onMRMLTransformNodeModified(void* /*call_data*/, vtkObject* caller)
{
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(caller);
  if (!transformNode) { return; }
  
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  qMRMLUtils::getTransformInCoordinateSystem(this->Internal->MRMLTransformNode, 
    this->Internal->CoordinateReference == Self::GLOBAL, transform);
  
  QVector<double> vector; 
  qMRMLUtils::vtkMatrixToQVector(transform->GetMatrix(), vector); 
  this->setVector( vector ); 
  
}

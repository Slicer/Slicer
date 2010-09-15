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

// Qt includes
#include <QDebug>

// qMRML includes
#include "qMRMLUtils.h"
#include "qMRMLMatrixWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkWeakPointer.h>

//-----------------------------------------------------------------------------
class qMRMLMatrixWidgetPrivate
{
public:
  qMRMLMatrixWidgetPrivate()
    {
    this->CoordinateReference = qMRMLMatrixWidget::GLOBAL; 
    this->MRMLTransformNode = 0; 
    }
  
  qMRMLMatrixWidget::CoordinateReferenceType   CoordinateReference;
  vtkWeakPointer<vtkMRMLLinearTransformNode>   MRMLTransformNode;
  // Warning, this is not the real "transform, the real can be retrieved
  // by qVTKAbstractMatrixWidget->transform();
  vtkSmartPointer<vtkTransform>                Transform;
};

// --------------------------------------------------------------------------
qMRMLMatrixWidget::qMRMLMatrixWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qMRMLMatrixWidgetPrivate)
{
}

// --------------------------------------------------------------------------
qMRMLMatrixWidget::~qMRMLMatrixWidget()
{
}

// --------------------------------------------------------------------------
void qMRMLMatrixWidget::setCoordinateReference(CoordinateReferenceType _coordinateReference)
{
  Q_D(qMRMLMatrixWidget);
  if (d->CoordinateReference == _coordinateReference)
    {
    return;
    }

  d->CoordinateReference = _coordinateReference;

  this->updateMatrix();
}

// --------------------------------------------------------------------------
qMRMLMatrixWidget::CoordinateReferenceType qMRMLMatrixWidget::coordinateReference() const
{
  Q_D(const qMRMLMatrixWidget);
  return d->CoordinateReference;
}

// --------------------------------------------------------------------------
void qMRMLMatrixWidget::setMRMLTransformNode(vtkMRMLNode* node)
{
  this->setMRMLTransformNode(vtkMRMLLinearTransformNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qMRMLMatrixWidget::setMRMLTransformNode(vtkMRMLLinearTransformNode* transformNode)
{
  Q_D(qMRMLMatrixWidget);
  
  if (d->MRMLTransformNode == transformNode) 
    { 
    return; 
    }

  this->qvtkReconnect(d->MRMLTransformNode, transformNode,
                      vtkMRMLTransformableNode::TransformModifiedEvent, 
                      this, SLOT(updateMatrix())); 

  d->MRMLTransformNode = transformNode;
  
  this->updateMatrix();
}

// --------------------------------------------------------------------------
vtkMRMLLinearTransformNode* qMRMLMatrixWidget::mrmlTransformNode()const
{
  Q_D(const qMRMLMatrixWidget);
  return d->MRMLTransformNode;
}

// --------------------------------------------------------------------------
void qMRMLMatrixWidget::updateMatrix()
{
  Q_D(qMRMLMatrixWidget);

  if (d->MRMLTransformNode == 0)
    {
    this->setMatrixInternal(0);
    d->Transform = 0;
    return;
    }
  
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  qMRMLUtils::getTransformInCoordinateSystem(
    d->MRMLTransformNode,
    d->CoordinateReference == qMRMLMatrixWidget::GLOBAL, 
    transform);
  // update the matrix with the new values.
  this->setMatrixInternal(transform->GetMatrix());
  // keep a ref on the transform otherwise, the matrix will be reset when transform
  // goes out of scope (because qVTKAbstractMatrixWidget has a weak ref on the matrix).
  d->Transform = transform;  
}

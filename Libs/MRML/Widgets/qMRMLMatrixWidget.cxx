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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>

// qMRML includes
#include "qMRMLUtils.h"
#include "qMRMLMatrixWidget.h"

// MRML includes
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkNew.h>
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
    this->MRMLTransformNode = nullptr;
    this->UserUpdates = true;
    }

  qMRMLMatrixWidget::CoordinateReferenceType   CoordinateReference;
  vtkWeakPointer<vtkMRMLTransformNode>         MRMLTransformNode;
  // Warning, this is not the real "transform, the real can be retrieved
  // by qVTKAbstractMatrixWidget->transform();
  vtkSmartPointer<vtkTransform>                Transform;
  // Indicates whether the changes come from the user or are programmatic
  bool                                         UserUpdates;
};

// --------------------------------------------------------------------------
qMRMLMatrixWidget::qMRMLMatrixWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qMRMLMatrixWidgetPrivate)
{
  connect(this, SIGNAL(matrixChanged()),
          this, SLOT(updateTransformNode()));
}

// --------------------------------------------------------------------------
qMRMLMatrixWidget::~qMRMLMatrixWidget() = default;

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
  this->setMRMLTransformNode(vtkMRMLTransformNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qMRMLMatrixWidget::setMRMLTransformNode(vtkMRMLTransformNode* transformNode)
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

  this->setEnabled(transformNode ? transformNode->IsLinear() : false);

  this->updateMatrix();
}

// --------------------------------------------------------------------------
vtkMRMLTransformNode* qMRMLMatrixWidget::mrmlTransformNode()const
{
  Q_D(const qMRMLMatrixWidget);
  return d->MRMLTransformNode;
}

// --------------------------------------------------------------------------
void qMRMLMatrixWidget::updateMatrix()
{
  Q_D(qMRMLMatrixWidget);

  if (d->MRMLTransformNode == nullptr)
    {
    this->setMatrixInternal(nullptr);
    d->Transform = nullptr;
    return;
    }

  bool isLinear = d->MRMLTransformNode->IsLinear();
  this->setEnabled(isLinear);
  if (!isLinear)
    {
    return;
    }

  vtkNew<vtkTransform> transform;
  qMRMLUtils::getTransformInCoordinateSystem(
    d->MRMLTransformNode,
    d->CoordinateReference == qMRMLMatrixWidget::GLOBAL,
    transform.GetPointer());
  int oldUserUpdates = d->UserUpdates;
  d->UserUpdates = false;

  // update the matrix with the new values.
  this->setMatrixInternal( transform->GetMatrix() );
  d->UserUpdates = oldUserUpdates;
  // keep a ref on the transform otherwise, the matrix will be reset when transform
  // goes out of scope (because ctkVTKAbstractMatrixWidget has a weak ref on the matrix).
  d->Transform = transform.GetPointer();
}

// --------------------------------------------------------------------------
void qMRMLMatrixWidget::updateTransformNode()
{
  Q_D(qMRMLMatrixWidget);
  if (d->MRMLTransformNode == nullptr ||
      !d->UserUpdates)
    {
    return;
    }
  d->MRMLTransformNode->SetMatrixTransformToParent(this->matrix());
}

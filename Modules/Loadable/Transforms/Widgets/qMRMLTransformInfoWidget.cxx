/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/


// qMRML includes
#include "qMRMLTransformInfoWidget.h"
#include "ui_qMRMLTransformInfoWidget.h"

// MRML includes
#include <vtkMRMLCrosshairNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkMath.h>
#include <vtkTransform.h>
#include <vtkWeakPointer.h>

//------------------------------------------------------------------------------
class qMRMLTransformInfoWidgetPrivate: public Ui_qMRMLTransformInfoWidget
{
  Q_DECLARE_PUBLIC(qMRMLTransformInfoWidget);

protected:
  qMRMLTransformInfoWidget* const q_ptr;

public:
  qMRMLTransformInfoWidgetPrivate(qMRMLTransformInfoWidget& object);
  void init();

  void setAndObserveCrosshairNode();

  vtkWeakPointer<vtkMRMLScene> MRMLScene;
  vtkWeakPointer<vtkMRMLTransformNode> TransformNode;
  vtkWeakPointer<vtkMRMLCrosshairNode> CrosshairNode;
};

//------------------------------------------------------------------------------
qMRMLTransformInfoWidgetPrivate
::qMRMLTransformInfoWidgetPrivate(qMRMLTransformInfoWidget& object)
  : q_ptr(&object)
{
  this->TransformNode = nullptr;
  this->CrosshairNode = nullptr;
  this->MRMLScene = nullptr;
}

//------------------------------------------------------------------------------
void qMRMLTransformInfoWidgetPrivate::init()
{
  Q_Q(qMRMLTransformInfoWidget);
  this->setupUi(q);
  q->setEnabled(this->TransformNode.GetPointer() != nullptr);
  this->TransformToParentInfoTextBrowser->setTextInteractionFlags(Qt::TextSelectableByMouse);
  this->TransformFromParentInfoTextBrowser->setTextInteractionFlags(Qt::TextSelectableByMouse);
}

// --------------------------------------------------------------------------
void qMRMLTransformInfoWidgetPrivate::setAndObserveCrosshairNode()
{
  Q_Q(qMRMLTransformInfoWidget);

  vtkMRMLCrosshairNode* crosshairNode = nullptr;
  if (this->MRMLScene.GetPointer())
    {
    crosshairNode = vtkMRMLCrosshairNode::SafeDownCast(this->MRMLScene->GetFirstNodeByClass("vtkMRMLCrosshairNode"));
    }

  q->qvtkReconnect(this->CrosshairNode.GetPointer(), crosshairNode,
    vtkMRMLCrosshairNode::CursorPositionModifiedEvent,
    q, SLOT(updateTransformVectorDisplayFromMRML()));
  this->CrosshairNode = crosshairNode;
  q->updateTransformVectorDisplayFromMRML();
}

//------------------------------------------------------------------------------
qMRMLTransformInfoWidget::qMRMLTransformInfoWidget(QWidget *_parent)
  : Superclass(_parent)
  , d_ptr(new qMRMLTransformInfoWidgetPrivate(*this))
{
  Q_D(qMRMLTransformInfoWidget);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLTransformInfoWidget::~qMRMLTransformInfoWidget() = default;

//------------------------------------------------------------------------------
void qMRMLTransformInfoWidget::processEvent(
  vtkObject* caller, void* callData, unsigned long eventId, void* clientData)
{
  Q_UNUSED(caller);
  Q_UNUSED(callData);
  Q_UNUSED(clientData);

  if (eventId == vtkMRMLCrosshairNode::CursorPositionModifiedEvent)
    {
    this->updateTransformVectorDisplayFromMRML();
    }
}

//------------------------------------------------------------------------------
vtkMRMLTransformNode* qMRMLTransformInfoWidget::mrmlTransformNode()const
{
  Q_D(const qMRMLTransformInfoWidget);
  return d->TransformNode.GetPointer();
}

//------------------------------------------------------------------------------
void qMRMLTransformInfoWidget::setMRMLTransformNode(vtkMRMLNode* node)
{
  this->setMRMLTransformNode(vtkMRMLTransformNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLTransformInfoWidget::mrmlScene()const
{
  Q_D(const qMRMLTransformInfoWidget);
  return d->MRMLScene.GetPointer();
}

// --------------------------------------------------------------------------
void qMRMLTransformInfoWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLTransformInfoWidget);

  if (this->mrmlScene() == scene)
    {
    return;
    }

  d->MRMLScene = scene;
  d->setAndObserveCrosshairNode();
}

//------------------------------------------------------------------------------
void qMRMLTransformInfoWidget::setMRMLTransformNode(vtkMRMLTransformNode* transformNode)
{
  Q_D(qMRMLTransformInfoWidget);

  qvtkReconnect(d->TransformNode.GetPointer(), transformNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));
  qvtkReconnect(d->TransformNode.GetPointer(), transformNode, vtkMRMLTransformableNode::TransformModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));
  d->TransformNode = transformNode;

  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qMRMLTransformInfoWidget::showEvent(QShowEvent *)
{
  // Update the widget, now that it becomes becomes visible
  // (we might have missed some updates, because widget contents is not updated
  // if the widget is not visible).
  updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qMRMLTransformInfoWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLTransformInfoWidget);
  if (!this->isVisible())
    {
    // Getting the transform information is too expensive,
    // so if the widget is not visible then do not update
    return;
    }

  if (d->TransformNode.GetPointer())
    {
    d->TransformToParentInfoTextBrowser->setText(d->TransformNode->GetTransformToParentInfo());
    d->TransformFromParentInfoTextBrowser->setText(d->TransformNode->GetTransformFromParentInfo());
    }
  else
    {
    d->TransformToParentInfoTextBrowser->clear();
    d->TransformFromParentInfoTextBrowser->clear();
    }

  updateTransformVectorDisplayFromMRML();

  this->setEnabled(d->TransformNode.GetPointer() != nullptr);
}

//------------------------------------------------------------------------------
void qMRMLTransformInfoWidget::updateTransformVectorDisplayFromMRML()
{
  Q_D(qMRMLTransformInfoWidget);
  if (!this->isVisible())
    {
    // Getting the transform information is too expensive,
    // so if the widget is not visible then do not update
    return;
    }

  if (d->TransformNode.GetPointer() && d->CrosshairNode.GetPointer())
    {
    double ras[3]={0};
    bool validPosition = d->CrosshairNode->GetCursorPositionRAS(ras);
    if (validPosition)
      {
      // Get the displacement vector
      vtkAbstractTransform* transformToParent = d->TransformNode->GetTransformToParent();
      if (transformToParent)
        {
        double* rasDisplaced = transformToParent->TransformDoublePoint(ras[0], ras[1], ras[2]);

        // Verify if the transform is invertible at the current position
        vtkAbstractTransform* transformFromParent = d->TransformNode->GetTransformFromParent();
        if (transformFromParent)
          {
          double* rasDisplacedTransformedBack = transformFromParent->TransformDoublePoint(rasDisplaced[0], rasDisplaced[1], rasDisplaced[2]);
          static double INVERSE_COMPUTATION_ALLOWED_SQUARED_ERROR=0.1;
          bool inverseAccurate = vtkMath::Distance2BetweenPoints(ras,rasDisplacedTransformedBack)<INVERSE_COMPUTATION_ALLOWED_SQUARED_ERROR;

          d->ViewerDisplacementVectorRAS->setText(QString("Displacement vector  RAS: (%1, %2, %3)%4").
            arg(rasDisplaced[0] - ras[0], /* fieldWidth= */ 0, /* format = */ 'f', /* precision= */ 1).
            arg(rasDisplaced[1] - ras[1], /* fieldWidth= */ 0, /* format = */ 'f', /* precision= */ 1).
            arg(rasDisplaced[2] - ras[2], /* fieldWidth= */ 0, /* format = */ 'f', /* precision= */ 1).
            arg(inverseAccurate?"":"   Warning: inverse is inaccurate!") );
          return;
          }
        }
      }
    }
  // transform value is not available, so let's clear the display
  d->ViewerDisplacementVectorRAS->clear();
}

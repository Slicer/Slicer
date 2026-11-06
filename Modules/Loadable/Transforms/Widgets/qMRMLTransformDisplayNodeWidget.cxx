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
#include "qMRMLTransformDisplayNodeWidget.h"
#include "ui_qMRMLTransformDisplayNodeWidget.h"

// Qt includes
#include <QRegularExpression>
#include <QRegularExpressionValidator>

// MRML includes
#include <vtkMRMLColorNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLProceduralColorNode.h>

// VTK includes
#include "vtkColorTransferFunction.h"

// CTK includes
#include "ctkVTKScalarsToColorsView.h"

// Qt includes
#include <QDebug>

//-----------------------------------------------------------------------------
class qMRMLTransformDisplayNodeWidgetPrivate : public Ui_qMRMLTransformDisplayNodeWidget
{
  Q_DECLARE_PUBLIC(qMRMLTransformDisplayNodeWidget);

protected:
  qMRMLTransformDisplayNodeWidget* const q_ptr;

public:
  qMRMLTransformDisplayNodeWidgetPrivate(qMRMLTransformDisplayNodeWidget& object);
  ~qMRMLTransformDisplayNodeWidgetPrivate();
  void init();

  vtkMRMLTransformDisplayNode* TransformDisplayNode;
  vtkColorTransferFunction* ColorTransferFunction;
};

//-----------------------------------------------------------------------------
// qMRMLTransformDisplayNodeWidgetPrivate methods

//-----------------------------------------------------------------------------
qMRMLTransformDisplayNodeWidgetPrivate::qMRMLTransformDisplayNodeWidgetPrivate(qMRMLTransformDisplayNodeWidget& object)
  : q_ptr(&object)
{
  this->TransformDisplayNode = nullptr;
  this->ColorTransferFunction = vtkColorTransferFunction::New();
}

//-----------------------------------------------------------------------------
qMRMLTransformDisplayNodeWidgetPrivate::~qMRMLTransformDisplayNodeWidgetPrivate()
{
  this->ColorTransferFunction->Delete();
  this->ColorTransferFunction = nullptr;
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidgetPrivate::init()
{
  Q_Q(qMRMLTransformDisplayNodeWidget);
  this->setupUi(q);

  // The color map of a transform is a coloring option, so it belongs in the one coloring
  // section of the shared widget rather than in a second section next to it
  this->VectorFieldDisplayWidget->addColoringWidget(this->ColorMapWidget);

  double validBounds[4] = { VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, 0., 1. };

  this->ColorMapWidget->view()->setValidBounds(validBounds);
  this->ColorMapWidget->view()->addColorTransferFunction(nullptr);
  this->ColorMapWidget->view()->setColorTransferFunctionToPlots(this->ColorTransferFunction);

  double chartBounds[8] = { 0 };
  this->ColorMapWidget->view()->chartBounds(chartBounds);
  chartBounds[2] = 0;
  chartBounds[3] = 100;
  this->ColorMapWidget->view()->setChartUserBounds(chartBounds);
  chartBounds[2] = 1;
  chartBounds[3] = 50;
  this->ColorMapWidget->view()->setPlotsUserBounds(chartBounds);
  this->ColorMapWidget->view()->update();

  q->qvtkConnect(this->ColorTransferFunction, vtkCommand::EndInteractionEvent, q, SLOT(onColorInteractionEvent()), 0., Qt::QueuedConnection);
  q->qvtkConnect(this->ColorTransferFunction, vtkCommand::EndEvent, q, SLOT(onColorInteractionEvent()), 0., Qt::QueuedConnection);
  q->qvtkConnect(this->ColorTransferFunction, vtkCommand::ModifiedEvent, q, SLOT(onColorModifiedEvent()), 0., Qt::QueuedConnection);

  q->updateWidgetFromDisplayNode();
}

//-----------------------------------------------------------------------------
// qMRMLTransformDisplayNodeWidget methods

//-----------------------------------------------------------------------------
qMRMLTransformDisplayNodeWidget::qMRMLTransformDisplayNodeWidget(QWidget* newParent)
  : Superclass(newParent)
  , d_ptr(new qMRMLTransformDisplayNodeWidgetPrivate(*this))
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  d->init();
}

//-----------------------------------------------------------------------------
qMRMLTransformDisplayNodeWidget::~qMRMLTransformDisplayNodeWidget() = default;

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::addVisualizationWidget(QWidget* widget)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!widget)
  {
    return;
  }
  d->visualizationLayout->addWidget(widget);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::addColoringWidget(QWidget* widget)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  d->VectorFieldDisplayWidget->addColoringWidget(widget);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setMRMLTransformNode(vtkMRMLNode* transformNode)
{
  setMRMLTransformNode(vtkMRMLTransformNode::SafeDownCast(transformNode));
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setMRMLTransformNode(vtkMRMLTransformNode* transformNode)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  vtkMRMLTransformDisplayNode* displayNode = nullptr;
  if (transformNode != nullptr)
  {
    displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetDisplayNode());
  }

  qvtkReconnect(d->TransformDisplayNode, displayNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromDisplayNode()));

  d->TransformDisplayNode = displayNode;
  // How the field is drawn is edited by the shared vector field widget; this widget only
  // adds what is specific to a transform: the displacement color map and the editor handles.
  d->VectorFieldDisplayWidget->setMRMLVectorFieldDisplayNode(displayNode);
  this->updateWidgetFromDisplayNode();
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::updateWidgetFromDisplayNode()
{
  Q_D(qMRMLTransformDisplayNodeWidget);

  this->setEnabled(d->TransformDisplayNode != nullptr);

  if (!d->TransformDisplayNode)
  {
    return;
  }

  // Update ColorMap
  vtkColorTransferFunction* colorTransferFunctionInNode = d->TransformDisplayNode->GetColorMap();
  if (colorTransferFunctionInNode)
  {
    if (!vtkMRMLProceduralColorNode::IsColorMapEqual(d->ColorTransferFunction, colorTransferFunctionInNode))
    {
      // only update the range if the colormap is changed to avoid immediate update,
      // because we don't want to change the colormap plot range while dragging the control point
      d->ColorTransferFunction->DeepCopy(colorTransferFunctionInNode);
      this->colorUpdateRange();
    }
  }
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setColorTableNode(vtkMRMLNode* colorTableNode)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
  {
    return;
  }
  d->TransformDisplayNode->SetAndObserveColorNodeID(colorTableNode ? colorTableNode->GetID() : nullptr);
}

// ----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::colorUpdateRange()
{
  Q_D(qMRMLTransformDisplayNodeWidget);

  if (!d->TransformDisplayNode)
  {
    return;
  }
  // Rescale the chart so that all the points are visible
  vtkColorTransferFunction* colorMap = d->TransformDisplayNode->GetColorMap();
  if (colorMap == nullptr)
  {
    return;
  }
  double range[2] = { 0.0, 10.0 };
  colorMap->GetRange(range);
  double chartBounds[8] = { 0 };
  d->ColorMapWidget->view()->chartBounds(chartBounds);
  chartBounds[2] = 0;
  chartBounds[3] = range[1] * 1.1;
  d->ColorMapWidget->view()->setChartUserBounds(chartBounds);
  d->ColorMapWidget->view()->update();
}

// ----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::onColorInteractionEvent()
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
  {
    return;
  }
  colorUpdateRange();
}

// ----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::onColorModifiedEvent()
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
  {
    return;
  }
  d->TransformDisplayNode->SetColorMap(d->ColorTransferFunction);
}

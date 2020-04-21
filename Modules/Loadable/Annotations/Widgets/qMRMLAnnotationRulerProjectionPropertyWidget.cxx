/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Laurent Chauvin, Brigham and Women's
  Hospital. The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 5R42CA137886

==============================================================================*/

// qMRML includes
#include "qMRMLAnnotationRulerProjectionPropertyWidget.h"
#include "ui_qMRMLAnnotationRulerProjectionPropertyWidget.h"

// MRML includes
#include <vtkMRMLAnnotationRulerNode.h>
#include <vtkMRMLAnnotationLineDisplayNode.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Annotation
class qMRMLAnnotationRulerProjectionPropertyWidgetPrivate
  : public Ui_qMRMLAnnotationRulerProjectionPropertyWidget
{
  Q_DECLARE_PUBLIC(qMRMLAnnotationRulerProjectionPropertyWidget);
protected:
  qMRMLAnnotationRulerProjectionPropertyWidget* const q_ptr;
public:
  qMRMLAnnotationRulerProjectionPropertyWidgetPrivate(qMRMLAnnotationRulerProjectionPropertyWidget& object);
  void init();

  vtkMRMLAnnotationLineDisplayNode* RulerDisplayNode;
};

//-----------------------------------------------------------------------------
// qMRMLAnnotationRulerProjectionPropertyWidgetPrivate methods

//-----------------------------------------------------------------------------
qMRMLAnnotationRulerProjectionPropertyWidgetPrivate
::qMRMLAnnotationRulerProjectionPropertyWidgetPrivate(qMRMLAnnotationRulerProjectionPropertyWidget& object)
  : q_ptr(&object)
{
  this->RulerDisplayNode = nullptr;
}

//-----------------------------------------------------------------------------
void qMRMLAnnotationRulerProjectionPropertyWidgetPrivate
::init()
{
  Q_Q(qMRMLAnnotationRulerProjectionPropertyWidget);
  this->setupUi(q);
  QObject::connect(this->Line2DProjectionCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setProjectionVisibility(bool)));
  QObject::connect(this->LineUseRulerColorCheckBox, SIGNAL(toggled(bool)),
                  q, SLOT(setUseRulerColor(bool)));
  QObject::connect(this->LineProjectionColorPickerButton, SIGNAL(colorChanged(QColor)),
                   q, SLOT(setProjectionColor(QColor)));
  QObject::connect(this->LineOverlineThicknessSpinBox, SIGNAL(valueChanged(int)),
                   q, SLOT(setOverlineThickness(int)));
  QObject::connect(this->LineUnderlineThicknessSpinBox, SIGNAL(valueChanged(int)),
                   q, SLOT(setUnderlineThickness(int)));
  QObject::connect(this->LineColoredWhenParallelCheckBox, SIGNAL(toggled(bool)),
                  q, SLOT(setColoredWhenParallel(bool)));
  QObject::connect(this->LineThickerOnTopCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setThickerOnTop(bool)));
  QObject::connect(this->LineDashedCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setDashed(bool)));
  q->updateWidgetFromDisplayNode();
}

//-----------------------------------------------------------------------------
// qMRMLAnnotationRulerProjectionPropertyWidget methods

//-----------------------------------------------------------------------------
qMRMLAnnotationRulerProjectionPropertyWidget
::qMRMLAnnotationRulerProjectionPropertyWidget(QWidget *newParent) :
    Superclass(newParent)
  , d_ptr(new qMRMLAnnotationRulerProjectionPropertyWidgetPrivate(*this))
{
  Q_D(qMRMLAnnotationRulerProjectionPropertyWidget);
  d->init();
}

//-----------------------------------------------------------------------------
qMRMLAnnotationRulerProjectionPropertyWidget
::~qMRMLAnnotationRulerProjectionPropertyWidget() = default;

//-----------------------------------------------------------------------------
void qMRMLAnnotationRulerProjectionPropertyWidget
::setMRMLRulerNode(vtkMRMLAnnotationRulerNode* rulerNode)
{
  Q_D(qMRMLAnnotationRulerProjectionPropertyWidget);
  vtkMRMLAnnotationLineDisplayNode* displayNode
    = rulerNode->GetAnnotationLineDisplayNode();
  if (!displayNode)
    {
    return;
    }

  qvtkReconnect(d->RulerDisplayNode, displayNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromDisplayNode()));

  d->RulerDisplayNode = displayNode;
  this->updateWidgetFromDisplayNode();
}

//-----------------------------------------------------------------------------
void qMRMLAnnotationRulerProjectionPropertyWidget
::setProjectionVisibility(bool showProjection)
{
  Q_D(qMRMLAnnotationRulerProjectionPropertyWidget);
  if (!d->RulerDisplayNode)
    {
    return;
    }

  if (showProjection)
    {
    d->RulerDisplayNode->SliceProjectionOn();
    }
  else
    {
    d->RulerDisplayNode->SliceProjectionOff();
    }
}

//-----------------------------------------------------------------------------
void qMRMLAnnotationRulerProjectionPropertyWidget
::setUseRulerColor(bool useRulerColor)
{
  Q_D(qMRMLAnnotationRulerProjectionPropertyWidget);
  if (!d->RulerDisplayNode)
    {
    return;
    }

  if (useRulerColor)
    {
    d->RulerDisplayNode->SliceProjectionUseRulerColorOn();
    }
  else
    {
    d->RulerDisplayNode->SliceProjectionUseRulerColorOff();
    }
}

//-----------------------------------------------------------------------------
void qMRMLAnnotationRulerProjectionPropertyWidget
::setProjectionColor(QColor newColor)
{
  Q_D(qMRMLAnnotationRulerProjectionPropertyWidget);
  if (!d->RulerDisplayNode)
    {
    return;
    }
  d->RulerDisplayNode
    ->SetProjectedColor(newColor.redF(), newColor.greenF(), newColor.blueF());
}

//-----------------------------------------------------------------------------
void qMRMLAnnotationRulerProjectionPropertyWidget
::setOverlineThickness(int thickness)
{
  Q_D(qMRMLAnnotationRulerProjectionPropertyWidget);
  if (!d->RulerDisplayNode)
    {
    return;
    }

  if (d->RulerDisplayNode->GetOverLineThickness() != thickness)
    {
    d->RulerDisplayNode->SetOverLineThickness(thickness);
    }
}

//-----------------------------------------------------------------------------
void qMRMLAnnotationRulerProjectionPropertyWidget
::setUnderlineThickness(int thickness)
{
  Q_D(qMRMLAnnotationRulerProjectionPropertyWidget);
  if (!d->RulerDisplayNode)
    {
    return;
    }

  if (d->RulerDisplayNode->GetUnderLineThickness() != thickness)
    {
    d->RulerDisplayNode->SetUnderLineThickness(thickness);
    }
}

//-----------------------------------------------------------------------------
void qMRMLAnnotationRulerProjectionPropertyWidget
::setColoredWhenParallel(bool coloredWhenParallel)
{
  Q_D(qMRMLAnnotationRulerProjectionPropertyWidget);
  if (!d->RulerDisplayNode)
    {
    return;
    }

  if (coloredWhenParallel)
    {
    d->RulerDisplayNode->SliceProjectionColoredWhenParallelOn();
    }
  else
    {
    d->RulerDisplayNode->SliceProjectionColoredWhenParallelOff();
    }
}

//-----------------------------------------------------------------------------
void qMRMLAnnotationRulerProjectionPropertyWidget
::setThickerOnTop(bool thickerOnTop)
{
  Q_D(qMRMLAnnotationRulerProjectionPropertyWidget);
  if (!d->RulerDisplayNode)
    {
    return;
    }

  if (thickerOnTop)
    {
    d->RulerDisplayNode->SliceProjectionThickerOnTopOn();
    }
  else
    {
    d->RulerDisplayNode->SliceProjectionThickerOnTopOff();
    }
}

//-----------------------------------------------------------------------------
void qMRMLAnnotationRulerProjectionPropertyWidget
::setDashed(bool dashed)
{
  Q_D(qMRMLAnnotationRulerProjectionPropertyWidget);
  if (!d->RulerDisplayNode)
    {
    return;
    }

  if (dashed)
    {
    d->RulerDisplayNode->SliceProjectionDashedOn();
    }
  else
    {
    d->RulerDisplayNode->SliceProjectionDashedOff();
    }
}

//-----------------------------------------------------------------------------
void qMRMLAnnotationRulerProjectionPropertyWidget
::updateWidgetFromDisplayNode()
{
  Q_D(qMRMLAnnotationRulerProjectionPropertyWidget);

  this->setEnabled(d->RulerDisplayNode != nullptr);

  if (!d->RulerDisplayNode)
    {
    return;
    }

  // Update widget if different from MRML node
  // -- 2D Projection Visibility
  d->Line2DProjectionCheckBox->setChecked(
    d->RulerDisplayNode->GetSliceProjection() &
    vtkMRMLAnnotationDisplayNode::ProjectionOn);

  // -- Projection Color
  double pColor[3];
  d->RulerDisplayNode->GetProjectedColor(pColor);
  QColor displayColor = QColor(pColor[0]*255, pColor[1]*255, pColor[2]*255);
  d->LineProjectionColorPickerButton->setColor(displayColor);

  // -- Overline thickness
  double mrmlOverlineThickness = d->RulerDisplayNode->GetOverLineThickness();
  if (d->LineOverlineThicknessSpinBox->value() != mrmlOverlineThickness)
    {
    d->LineOverlineThicknessSpinBox->setValue(mrmlOverlineThickness);
    }

  // -- Underline thickness
  double mrmlUnderlineThickness = d->RulerDisplayNode->GetUnderLineThickness();
  if (d->LineUnderlineThicknessSpinBox->value() != mrmlUnderlineThickness)
    {
    d->LineUnderlineThicknessSpinBox->setValue(mrmlUnderlineThickness);
    }

  // -- Colored When Parallel
  d->LineColoredWhenParallelCheckBox->setChecked(
    d->RulerDisplayNode->GetSliceProjection() &
    vtkMRMLAnnotationLineDisplayNode::ProjectionColoredWhenParallel);

  // -- Thicker On Top
  d->LineThickerOnTopCheckBox->setChecked(
    d->RulerDisplayNode->GetSliceProjection() &
    vtkMRMLAnnotationLineDisplayNode::ProjectionThickerOnTop);

  // -- Dashed
  d->LineDashedCheckBox->setChecked(
    d->RulerDisplayNode->GetSliceProjection() &
    vtkMRMLAnnotationLineDisplayNode::ProjectionDashed);
}

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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// QT includes

// qMRML includes
#include "qMRMLDisplayNodeWidget.h"
#include "ui_qMRMLDisplayNodeWidget.h"

// MRML includes
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayNode.h>

// VTK includes
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
class qMRMLDisplayNodeWidgetPrivate: public Ui_qMRMLDisplayNodeWidget
{
  Q_DECLARE_PUBLIC(qMRMLDisplayNodeWidget);

protected:
  qMRMLDisplayNodeWidget* const q_ptr;

public:
  qMRMLDisplayNodeWidgetPrivate(qMRMLDisplayNodeWidget& object);
  void init();

  vtkSmartPointer<vtkMRMLDisplayNode> MRMLDisplayNode;
};

//------------------------------------------------------------------------------
qMRMLDisplayNodeWidgetPrivate::qMRMLDisplayNodeWidgetPrivate(qMRMLDisplayNodeWidget& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidgetPrivate::init()
{
  Q_Q(qMRMLDisplayNodeWidget);
  this->setupUi(q);
  
  QObject::connect(this->VisibilityCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setVisibility(bool)));
  QObject::connect(this->SelectedCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setSelected(bool)));
  QObject::connect(this->ClippingCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setClipping(bool)));
  QObject::connect(this->SliceIntersectionVisibilityCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setSliceIntersectionVisible(bool)));
  QObject::connect(this->SliceIntersectionThicknessSpinBox, SIGNAL(valueChanged(int)),
                   q, SLOT(setSliceIntersectionThickness(int)));

  QObject::connect(this->MaterialPropertyWidget, SIGNAL(colorChanged(QColor)),
                   q, SLOT(setColor(QColor)));
  QObject::connect(this->MaterialPropertyWidget, SIGNAL(opacityChanged(double)),
                   q, SLOT(setOpacity(double)));
  QObject::connect(this->MaterialPropertyWidget, SIGNAL(ambientChanged(double)),
                   q, SLOT(setAmbient(double)));
  QObject::connect(this->MaterialPropertyWidget, SIGNAL(diffuseChanged(double)),
                   q, SLOT(setDiffuse(double)));
  QObject::connect(this->MaterialPropertyWidget, SIGNAL(specularChanged(double)),
                   q, SLOT(setSpecular(double)));
  QObject::connect(this->MaterialPropertyWidget, SIGNAL(specularPowerChanged(double)),
                   q, SLOT(setSpecularPower(double)));
  QObject::connect(this->MaterialPropertyWidget, SIGNAL(backfaceCullingChanged(bool)),
                   q, SLOT(setBackfaceCulling(bool)));
  q->setEnabled(this->MRMLDisplayNode.GetPointer() != 0);
}

//------------------------------------------------------------------------------
qMRMLDisplayNodeWidget::qMRMLDisplayNodeWidget(QWidget *_parent)
  : QWidget(_parent)
  , d_ptr(new qMRMLDisplayNodeWidgetPrivate(*this))
{
  Q_D(qMRMLDisplayNodeWidget);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLDisplayNodeWidget::~qMRMLDisplayNodeWidget()
{
}


//------------------------------------------------------------------------------
vtkMRMLDisplayNode* qMRMLDisplayNodeWidget::mrmlDisplayNode()const
{
  Q_D(const qMRMLDisplayNodeWidget);
  return d->MRMLDisplayNode;
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setMRMLDisplayableNode(vtkMRMLNode* node)
{
  vtkMRMLDisplayableNode* displayableNode =
    vtkMRMLDisplayableNode::SafeDownCast(node);
  this->setMRMLDisplayNode(displayableNode ? displayableNode->GetDisplayNode() : 0);
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setMRMLDisplayNode(vtkMRMLNode* node)
{
  this->setMRMLDisplayNode(vtkMRMLDisplayNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setMRMLDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  Q_D(qMRMLDisplayNodeWidget);
  qvtkReconnect(d->MRMLDisplayNode, displayNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));
  d->MRMLDisplayNode = displayNode;
  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setVisibility(bool visible)
{
  Q_D(qMRMLDisplayNodeWidget);
  if (!d->MRMLDisplayNode.GetPointer())
    {
    return;
    }
  d->MRMLDisplayNode->SetVisibility(visible);
}

//------------------------------------------------------------------------------
bool qMRMLDisplayNodeWidget::visibility()const
{
  Q_D(const qMRMLDisplayNodeWidget);
  return d->VisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setVisibilityVisible(bool visible)
{
  Q_D(const qMRMLDisplayNodeWidget);
  d->VisibilityCheckBox->setVisible(visible);
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setSelected(bool selected)
{
  Q_D(qMRMLDisplayNodeWidget);
  if (!d->MRMLDisplayNode.GetPointer())
    {
    return;
    }
  d->MRMLDisplayNode->SetSelected(selected);
}

//------------------------------------------------------------------------------
bool qMRMLDisplayNodeWidget::selected()const
{
  Q_D(const qMRMLDisplayNodeWidget);
  return d->SelectedCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setSelectedVisible(bool visible)
{
  Q_D(const qMRMLDisplayNodeWidget);
  d->SelectedLabel->setVisible(visible);
  d->SelectedCheckBox->setVisible(visible);
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setClipping(bool clip)
{
  Q_D(qMRMLDisplayNodeWidget);
  if (!d->MRMLDisplayNode.GetPointer())
    {
    return;
    }
  d->MRMLDisplayNode->SetClipping(clip);
}

//------------------------------------------------------------------------------
bool qMRMLDisplayNodeWidget::clipping()const
{
  Q_D(const qMRMLDisplayNodeWidget);
  return d->ClippingCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setClippingVisible(bool visible)
{
  Q_D(qMRMLDisplayNodeWidget);
  d->ClippingCheckBox->setVisible(visible);
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setSliceIntersectionVisible(bool visible)
{
  Q_D(qMRMLDisplayNodeWidget);
  if (!d->MRMLDisplayNode.GetPointer())
    {
    return;
    }
  d->MRMLDisplayNode->SetSliceIntersectionVisibility(visible);
}

//------------------------------------------------------------------------------
bool qMRMLDisplayNodeWidget::sliceIntersectionVisible()const
{
  Q_D(const qMRMLDisplayNodeWidget);
  return d->SliceIntersectionVisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setSliceIntersectionVisibleVisible(bool visible)
{
  Q_D(qMRMLDisplayNodeWidget);
  d->SliceIntersectionVisibilityCheckBox->setVisible(visible);
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setSliceIntersectionThickness(int thickness)
{
  Q_D(qMRMLDisplayNodeWidget);
  if (!d->MRMLDisplayNode.GetPointer())
    {
    return;
    }
  d->MRMLDisplayNode->SetSliceIntersectionThickness(thickness);
}

//------------------------------------------------------------------------------
int qMRMLDisplayNodeWidget::sliceIntersectionThickness()const
{
  Q_D(const qMRMLDisplayNodeWidget);
  return d->SliceIntersectionThicknessSpinBox->value();
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setSliceIntersectionThicknessVisible(bool visible)
{
  Q_D(qMRMLDisplayNodeWidget);
  d->SliceIntersectionThicknessSpinBox->setVisible(visible);
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setColor(const QColor& color)
{
  Q_D(qMRMLDisplayNodeWidget);
  if (!d->MRMLDisplayNode.GetPointer())
    {
    return;
    }
  // QColors loose precision in the numbers, don't reset the color if it didn't
  // "really" change.
  double* oldColor = d->MRMLDisplayNode->GetColor();
  if (QColor::fromRgbF(oldColor[0], oldColor[1], oldColor[2]) != color)
    {
    d->MRMLDisplayNode->SetColor(color.redF(), color.greenF(), color.blueF());
    }
}

//------------------------------------------------------------------------------
QColor qMRMLDisplayNodeWidget::color()const
{
  Q_D(const qMRMLDisplayNodeWidget);
  return d->MaterialPropertyWidget->color();
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setOpacity(double opacity)
{
  Q_D(qMRMLDisplayNodeWidget);
  if (!d->MRMLDisplayNode.GetPointer())
    {
    return;
    }
  d->MRMLDisplayNode->SetOpacity(opacity);
}

//------------------------------------------------------------------------------
double qMRMLDisplayNodeWidget::opacity()const
{
  Q_D(const qMRMLDisplayNodeWidget);
  return d->MaterialPropertyWidget->opacity();
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setAmbient(double ambient)
{
  Q_D(qMRMLDisplayNodeWidget);
  if (!d->MRMLDisplayNode.GetPointer())
    {
    return;
    }
  d->MRMLDisplayNode->SetAmbient(ambient);
}

//------------------------------------------------------------------------------
double qMRMLDisplayNodeWidget::ambient()const
{
  Q_D(const qMRMLDisplayNodeWidget);
  return d->MaterialPropertyWidget->ambient();
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setDiffuse(double diffuse)
{
  Q_D(qMRMLDisplayNodeWidget);
  if (!d->MRMLDisplayNode.GetPointer())
    {
    return;
    }
  d->MRMLDisplayNode->SetDiffuse(diffuse);
}

//------------------------------------------------------------------------------
double qMRMLDisplayNodeWidget::diffuse()const
{
  Q_D(const qMRMLDisplayNodeWidget);
  return d->MaterialPropertyWidget->diffuse();
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setSpecular(double specular)
{
  Q_D(qMRMLDisplayNodeWidget);
  if (!d->MRMLDisplayNode.GetPointer())
    {
    return;
    }
  d->MRMLDisplayNode->SetSpecular(specular);
}

//------------------------------------------------------------------------------
double qMRMLDisplayNodeWidget::specular()const
{
  Q_D(const qMRMLDisplayNodeWidget);
  return d->MaterialPropertyWidget->specular();
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setSpecularPower(double specularPower)
{
  Q_D(qMRMLDisplayNodeWidget);
  if (!d->MRMLDisplayNode.GetPointer())
    {
    return;
    }
  d->MRMLDisplayNode->SetPower(specularPower);
}

//------------------------------------------------------------------------------
double qMRMLDisplayNodeWidget::specularPower()const
{
  Q_D(const qMRMLDisplayNodeWidget);
  return d->MaterialPropertyWidget->specularPower();
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setBackfaceCulling(bool backfaceCulling)
{
  Q_D(qMRMLDisplayNodeWidget);
  if (!d->MRMLDisplayNode.GetPointer())
    {
    return;
    }
  d->MRMLDisplayNode->SetBackfaceCulling(backfaceCulling);
}

//------------------------------------------------------------------------------
bool qMRMLDisplayNodeWidget::backfaceCulling()const
{
  Q_D(const qMRMLDisplayNodeWidget);
  return d->MaterialPropertyWidget->backfaceCulling();
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLDisplayNodeWidget);
  this->setEnabled(d->MRMLDisplayNode.GetPointer() != 0);
  if (!d->MRMLDisplayNode.GetPointer())
    {
    return;
    }
  d->VisibilityCheckBox->setChecked(d->MRMLDisplayNode->GetVisibility());
  d->SelectedCheckBox->setEnabled(d->MRMLDisplayNode->GetSelectable());
  d->SelectedCheckBox->setChecked(d->MRMLDisplayNode->GetSelected());
  d->ClippingCheckBox->setChecked(d->MRMLDisplayNode->GetClipping());
  d->SliceIntersectionVisibilityCheckBox->setChecked(
    d->MRMLDisplayNode->GetSliceIntersectionVisibility());
  d->SliceIntersectionThicknessSpinBox->setValue(
    d->MRMLDisplayNode->GetSliceIntersectionThickness());
  d->MaterialPropertyWidget->setColor(
    QColor::fromRgbF(d->MRMLDisplayNode->GetColor()[0],
                     d->MRMLDisplayNode->GetColor()[1],
                     d->MRMLDisplayNode->GetColor()[2]));
  d->MaterialPropertyWidget->setOpacity(d->MRMLDisplayNode->GetOpacity());
  d->MaterialPropertyWidget->setAmbient(d->MRMLDisplayNode->GetAmbient());
  d->MaterialPropertyWidget->setDiffuse(d->MRMLDisplayNode->GetDiffuse());
  d->MaterialPropertyWidget->setSpecular(d->MRMLDisplayNode->GetSpecular());
  d->MaterialPropertyWidget->setSpecularPower(d->MRMLDisplayNode->GetPower());
  d->MaterialPropertyWidget->setBackfaceCulling(d->MRMLDisplayNode->GetBackfaceCulling());
}

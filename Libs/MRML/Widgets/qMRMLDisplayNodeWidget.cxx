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

// Qt includes

// qMRML includes
#include "qMRMLDisplayNodeWidget.h"
#include "ui_qMRMLDisplayNodeWidget.h"

// MRML includes
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayNode.h>

// VTK includes
#include <vtkProperty.h>
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
  vtkSmartPointer<vtkProperty> Property;
};

//------------------------------------------------------------------------------
qMRMLDisplayNodeWidgetPrivate::qMRMLDisplayNodeWidgetPrivate(
  qMRMLDisplayNodeWidget& object)
  : q_ptr(&object)
{
  this->Property = vtkSmartPointer<vtkProperty>::New();
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
  QObject::connect(this->ThreeDVisibilityCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(set3DVisible(bool)));
  QObject::connect(this->SliceIntersectionVisibilityCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setSliceIntersectionVisible(bool)));
  QObject::connect(this->SliceIntersectionThicknessSpinBox, SIGNAL(valueChanged(int)),
                   q, SLOT(setSliceIntersectionThickness(int)));
  QObject::connect(this->SliceIntersectionOpacitySlider, SIGNAL(valueChanged(double)),
                   q, SLOT(setSliceIntersectionOpacity(double)));

  this->PropertyWidget->setProperty(this->Property);
  q->qvtkConnect(this->Property, vtkCommand::ModifiedEvent,
                 q, SLOT(updateNodeFromProperty()));
  q->setEnabled(this->MRMLDisplayNode.GetPointer() != nullptr);
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
qMRMLDisplayNodeWidget::~qMRMLDisplayNodeWidget() = default;


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
  this->setMRMLDisplayNode(displayableNode ? displayableNode->GetDisplayNode() : nullptr);
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
void qMRMLDisplayNodeWidget::setThreeDVisible(bool visible)
{
  Q_D(qMRMLDisplayNodeWidget);
  if (!d->MRMLDisplayNode.GetPointer())
    {
    return;
    }
  d->MRMLDisplayNode->SetVisibility3D(visible);
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setSliceIntersectionVisible(bool visible)
{
  Q_D(qMRMLDisplayNodeWidget);
  if (!d->MRMLDisplayNode.GetPointer())
    {
    return;
    }
  d->MRMLDisplayNode->SetVisibility2D(visible);
}

//------------------------------------------------------------------------------
bool qMRMLDisplayNodeWidget::threeDVisible()const
{
  Q_D(const qMRMLDisplayNodeWidget);
  return d->ThreeDVisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
bool qMRMLDisplayNodeWidget::sliceIntersectionVisible()const
{
  Q_D(const qMRMLDisplayNodeWidget);
  return d->SliceIntersectionVisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setThreeDVisibleVisible(bool visible)
{
  Q_D(qMRMLDisplayNodeWidget);
  d->ThreeDVisibilityCheckBox->setVisible(visible);
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
void qMRMLDisplayNodeWidget::setSliceIntersectionOpacity(double opacity)
{
  Q_D(qMRMLDisplayNodeWidget);
  if (!d->MRMLDisplayNode.GetPointer())
    {
    return;
    }
  d->MRMLDisplayNode->SetSliceIntersectionOpacity(opacity);
}

//------------------------------------------------------------------------------
double qMRMLDisplayNodeWidget::sliceIntersectionOpacity()const
{
  Q_D(const qMRMLDisplayNodeWidget);
  return d->SliceIntersectionOpacitySlider->value();
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::setSliceIntersectionOpacityVisible(bool visible)
{
  Q_D(qMRMLDisplayNodeWidget);
  d->SliceIntersectionOpacitySlider->setVisible(visible);
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLDisplayNodeWidget);
  this->setEnabled(d->MRMLDisplayNode.GetPointer() != nullptr);
  if (!d->MRMLDisplayNode.GetPointer())
    {
    return;
    }
  d->VisibilityCheckBox->setChecked(d->MRMLDisplayNode->GetVisibility());
  d->DisplayNodeViewComboBox->setMRMLDisplayNode(d->MRMLDisplayNode);
  d->SelectedCheckBox->setEnabled(d->MRMLDisplayNode->GetSelectable());
  d->SelectedCheckBox->setChecked(d->MRMLDisplayNode->GetSelected());
  d->ClippingCheckBox->setChecked(d->MRMLDisplayNode->GetClipping());
  d->ThreeDVisibilityCheckBox->setChecked(
    d->MRMLDisplayNode->GetVisibility3D());
  d->SliceIntersectionVisibilityCheckBox->setChecked(
    d->MRMLDisplayNode->GetVisibility2D());
  d->SliceIntersectionThicknessSpinBox->setValue(
    d->MRMLDisplayNode->GetSliceIntersectionThickness());
  d->SliceIntersectionOpacitySlider->setValue(
    d->MRMLDisplayNode->GetSliceIntersectionOpacity());

  // While updating Property, its state is unstable.
  qvtkBlock(d->Property, vtkCommand::ModifiedEvent, this);

  // Representation
  d->Property->SetRepresentation(d->MRMLDisplayNode->GetRepresentation());
  d->Property->SetPointSize(d->MRMLDisplayNode->GetPointSize());
  d->Property->SetLineWidth(d->MRMLDisplayNode->GetLineWidth());
  d->Property->SetFrontfaceCulling(d->MRMLDisplayNode->GetFrontfaceCulling());
  d->Property->SetBackfaceCulling(d->MRMLDisplayNode->GetBackfaceCulling());
  // Color
  d->Property->SetColor(d->MRMLDisplayNode->GetColor()[0],
                        d->MRMLDisplayNode->GetColor()[1],
                        d->MRMLDisplayNode->GetColor()[2]);
  d->Property->SetOpacity(d->MRMLDisplayNode->GetOpacity());
  d->Property->SetEdgeVisibility(d->MRMLDisplayNode->GetEdgeVisibility());
  d->Property->SetEdgeColor(d->MRMLDisplayNode->GetEdgeColor()[0],
                            d->MRMLDisplayNode->GetEdgeColor()[1],
                            d->MRMLDisplayNode->GetEdgeColor()[2]);
  // Lighting
  d->Property->SetLighting(d->MRMLDisplayNode->GetLighting());
  d->Property->SetInterpolation(d->MRMLDisplayNode->GetInterpolation());
  d->Property->SetShading(d->MRMLDisplayNode->GetShading());
  // Material
  d->Property->SetAmbient(d->MRMLDisplayNode->GetAmbient());
  d->Property->SetDiffuse(d->MRMLDisplayNode->GetDiffuse());
  d->Property->SetSpecular(d->MRMLDisplayNode->GetSpecular());
  d->Property->SetSpecularPower(d->MRMLDisplayNode->GetPower());
  qvtkUnblock(d->Property, vtkCommand::ModifiedEvent, this);
}

//------------------------------------------------------------------------------
void qMRMLDisplayNodeWidget::updateNodeFromProperty()
{
  Q_D(qMRMLDisplayNodeWidget);
  if (!d->MRMLDisplayNode.GetPointer())
    {
    return;
    }
  int wasModifying = d->MRMLDisplayNode->StartModify();
  // Representation
  d->MRMLDisplayNode->SetRepresentation(d->Property->GetRepresentation());
  d->MRMLDisplayNode->SetPointSize(d->Property->GetPointSize());
  d->MRMLDisplayNode->SetLineWidth(d->Property->GetLineWidth());
  d->MRMLDisplayNode->SetFrontfaceCulling(d->Property->GetFrontfaceCulling());
  d->MRMLDisplayNode->SetBackfaceCulling(d->Property->GetBackfaceCulling());
  // Color
  d->MRMLDisplayNode->SetColor(d->Property->GetColor()[0],
                               d->Property->GetColor()[1],
                               d->Property->GetColor()[2]);
  d->MRMLDisplayNode->SetOpacity(d->Property->GetOpacity());
  d->MRMLDisplayNode->SetEdgeVisibility(d->Property->GetEdgeVisibility());
  d->MRMLDisplayNode->SetEdgeColor(d->Property->GetEdgeColor()[0],
                                   d->Property->GetEdgeColor()[1],
                                   d->Property->GetEdgeColor()[2]);
  // Lighting
  d->MRMLDisplayNode->SetLighting(d->Property->GetLighting());
  d->MRMLDisplayNode->SetInterpolation(d->Property->GetInterpolation());
  d->MRMLDisplayNode->SetShading(d->Property->GetShading());
  // Material
  d->MRMLDisplayNode->SetAmbient(d->Property->GetAmbient());
  d->MRMLDisplayNode->SetDiffuse(d->Property->GetDiffuse());
  d->MRMLDisplayNode->SetSpecular(d->Property->GetSpecular());
  d->MRMLDisplayNode->SetPower(d->Property->GetSpecularPower());

  d->MRMLDisplayNode->EndModify(wasModifying);
}

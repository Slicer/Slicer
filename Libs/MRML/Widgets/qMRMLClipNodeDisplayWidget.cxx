/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported in part through NIH grant R01 HL153166.

==============================================================================*/

// Qt includes
#include <QAbstractButton>
#include <QButtonGroup>
#include <QFormLayout>
#include <QStyle>
#include <QToolButton>

// qMRML includes
#include "qMRMLClipNodeDisplayWidget.h"
#include "ui_qMRMLClipNodeDisplayWidget.h"

// MRML includes
#include <vtkMRMLClipNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLSegmentationDisplayNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
class qMRMLClipNodeDisplayWidgetPrivate : public Ui_qMRMLClipNodeDisplayWidget
{
  Q_DECLARE_PUBLIC(qMRMLClipNodeDisplayWidget);

protected:
  qMRMLClipNodeDisplayWidget* const q_ptr;

public:
  qMRMLClipNodeDisplayWidgetPrivate(qMRMLClipNodeDisplayWidget& object);
  void init();

  /// Row index in the form layout where the next row below the clip node selector is inserted
  int rowBelowClipNodeSelector();

  /// Show or hide a row of options that only apply to some display node types (rows after the clipping type).
  /// Hidden rows are taken out of the form layout, as they would still add spacing to it.
  void setOptionalRowVisible(QWidget* label, QWidget* field, bool visible);

  vtkWeakPointer<vtkMRMLDisplayNode> MRMLDisplayNode;
  bool IsUpdatingWidgetFromMRML{ false };
  int NumberOfRowsBelowClipNodeSelector{ 0 };
};

//------------------------------------------------------------------------------
qMRMLClipNodeDisplayWidgetPrivate::qMRMLClipNodeDisplayWidgetPrivate(qMRMLClipNodeDisplayWidget& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qMRMLClipNodeDisplayWidgetPrivate::init()
{
  Q_Q(qMRMLClipNodeDisplayWidget);
  this->setupUi(q);
  q->setEnabled(this->MRMLDisplayNode != nullptr);

  QObject::connect(this->checkBox_Clipping, SIGNAL(toggled(bool)), q, SLOT(onClippingToggled(bool)));
  QObject::connect(this->checkBox_ClippingCapping, SIGNAL(toggled(bool)), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->sliderWidget_ClippingCapOpacity, SIGNAL(valueChanged(double)), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->checkBox_ClippingOutline, SIGNAL(toggled(bool)), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->checkBox_ClippingKeepWholeCells, SIGNAL(toggled(bool)), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->ClipNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, SLOT(onClipNodeSelected(vtkMRMLNode*)));
  QButtonGroup* clipTypeGroup = new QButtonGroup(q);
  clipTypeGroup->addButton(this->UnionButton);
  clipTypeGroup->addButton(this->IntersectionButton);
  QObject::connect(this->UnionButton, SIGNAL(toggled(bool)), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->IntersectionButton, SIGNAL(toggled(bool)), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), this->ClipNodeSelector, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), this->ClipNodeWidget, SLOT(setMRMLScene(vtkMRMLScene*)));
}

//------------------------------------------------------------------------------
int qMRMLClipNodeDisplayWidgetPrivate::rowBelowClipNodeSelector()
{
  int clipNodeSelectorRow = 0;
  QFormLayout::ItemRole role = QFormLayout::FieldRole;
  this->formLayout->getLayoutPosition(this->horizontalLayout, &clipNodeSelectorRow, &role);
  return clipNodeSelectorRow + 1 + this->NumberOfRowsBelowClipNodeSelector;
}

//------------------------------------------------------------------------------
void qMRMLClipNodeDisplayWidgetPrivate::setOptionalRowVisible(QWidget* label, QWidget* field, bool visible)
{
  int row = -1;
  QFormLayout::ItemRole role = QFormLayout::FieldRole;
  this->formLayout->getWidgetPosition(field, &row, &role);
  bool inLayout = (row >= 0);
  if (visible && !inLayout)
  {
    // Insert after the clipping type row and the optional rows that precede this one
    int insertRow = 0;
    this->formLayout->getLayoutPosition(this->ClipTypeLayout, &insertRow, &role);
    insertRow++;
    for (QWidget* optionalField : { static_cast<QWidget*>(this->ClippingCapWidget), //
                                    static_cast<QWidget*>(this->checkBox_ClippingOutline),
                                    static_cast<QWidget*>(this->checkBox_ClippingKeepWholeCells) })
    {
      if (optionalField == field)
      {
        break;
      }
      int optionalRow = -1;
      this->formLayout->getWidgetPosition(optionalField, &optionalRow, &role);
      if (optionalRow >= 0)
      {
        insertRow++;
      }
    }
    this->formLayout->insertRow(insertRow, label, field);
  }
  else if (!visible && inLayout)
  {
    QFormLayout::TakeRowResult takenRow = this->formLayout->takeRow(field);
    delete takenRow.labelItem; // only the layout items are deleted, not the widgets
    delete takenRow.fieldItem;
  }
  label->setVisible(visible);
  field->setVisible(visible);
}

//------------------------------------------------------------------------------
qMRMLClipNodeDisplayWidget::qMRMLClipNodeDisplayWidget(QWidget* _parent /*=nullptr*/)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLClipNodeDisplayWidgetPrivate(*this))
{
  Q_D(qMRMLClipNodeDisplayWidget);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLClipNodeDisplayWidget::~qMRMLClipNodeDisplayWidget() = default;

//------------------------------------------------------------------------------
vtkMRMLDisplayNode* qMRMLClipNodeDisplayWidget::mrmlDisplayNode() const
{
  Q_D(const qMRMLClipNodeDisplayWidget);
  return d->MRMLDisplayNode;
}

//------------------------------------------------------------------------------
void qMRMLClipNodeDisplayWidget::setMRMLDisplayNode(vtkMRMLNode* node)
{
  this->setMRMLDisplayNode(vtkMRMLDisplayNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLClipNodeDisplayWidget::setMRMLDisplayNode(vtkMRMLDisplayNode* clipNode)
{
  Q_D(qMRMLClipNodeDisplayWidget);
  qvtkReconnect(d->MRMLDisplayNode, clipNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  d->MRMLDisplayNode = clipNode;
  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
vtkMRMLClipNode* qMRMLClipNodeDisplayWidget::ensureClipNode(vtkMRMLDisplayNode* displayNode)
{
  vtkMRMLScene* scene = displayNode ? displayNode->GetScene() : nullptr;
  if (!scene)
  {
    return nullptr;
  }
  if (displayNode->GetClipNode())
  {
    return displayNode->GetClipNode();
  }
  // Usually there is only one clip node in the scene, used by all nodes
  vtkMRMLNode* clipNode = scene->GetFirstNodeByClass("vtkMRMLClipNode");
  if (!clipNode)
  {
    clipNode = scene->AddNewNodeByClass("vtkMRMLClipNode");
  }
  if (clipNode)
  {
    displayNode->SetAndObserveClipNodeID(clipNode->GetID());
  }
  return displayNode->GetClipNode();
}

//------------------------------------------------------------------------------
void qMRMLClipNodeDisplayWidget::addWidgetNextToClipNodeSelector(QWidget* widget)
{
  Q_D(qMRMLClipNodeDisplayWidget);
  if (!widget)
  {
    return;
  }
  // Same height as the clip node selector (a button is square, with its icon fitting in it)
  int height = d->ClipNodeSelector->sizeHint().height();
  widget->setFixedHeight(height);
  QAbstractButton* button = qobject_cast<QAbstractButton*>(widget);
  if (button)
  {
    button->setFixedWidth(height);
    int iconExtent = height - 2 * button->style()->pixelMetric(QStyle::PM_ButtonMargin, nullptr, button);
    if (iconExtent > 0 && button->iconSize().height() > iconExtent)
    {
      button->setIconSize(QSize(iconExtent, iconExtent));
    }
  }
  d->horizontalLayout->addWidget(widget);
}

//------------------------------------------------------------------------------
void qMRMLClipNodeDisplayWidget::addRowBelowClipNodeSelector(const QString& labelText, QWidget* field)
{
  Q_D(qMRMLClipNodeDisplayWidget);
  d->formLayout->insertRow(d->rowBelowClipNodeSelector(), labelText, field);
  d->NumberOfRowsBelowClipNodeSelector++;
}

//------------------------------------------------------------------------------
void qMRMLClipNodeDisplayWidget::addRowBelowClipNodeSelector(QWidget* label, QWidget* field)
{
  Q_D(qMRMLClipNodeDisplayWidget);
  d->formLayout->insertRow(d->rowBelowClipNodeSelector(), label, field);
  d->NumberOfRowsBelowClipNodeSelector++;
}

//------------------------------------------------------------------------------
void qMRMLClipNodeDisplayWidget::addRow(const QString& labelText, QWidget* field)
{
  Q_D(qMRMLClipNodeDisplayWidget);
  d->formLayout->addRow(labelText, field);
}

//------------------------------------------------------------------------------
void qMRMLClipNodeDisplayWidget::addRow(QWidget* label, QWidget* field)
{
  Q_D(qMRMLClipNodeDisplayWidget);
  d->formLayout->addRow(label, field);
}

//------------------------------------------------------------------------------
void qMRMLClipNodeDisplayWidget::addRow(QWidget* widget)
{
  Q_D(qMRMLClipNodeDisplayWidget);
  d->formLayout->addRow(widget);
}

//------------------------------------------------------------------------------
void qMRMLClipNodeDisplayWidget::onClippingToggled(bool enabled)
{
  Q_D(qMRMLClipNodeDisplayWidget);
  if (d->IsUpdatingWidgetFromMRML || !d->MRMLDisplayNode)
  {
    return;
  }
  MRMLNodeModifyBlocker displayNodeBlocker(d->MRMLDisplayNode);
  if (enabled && !d->MRMLDisplayNode->GetClipNode())
  {
    // Clipping requires a clip node: save the user from selecting or creating one
    qMRMLClipNodeDisplayWidget::ensureClipNode(d->MRMLDisplayNode);
  }
  this->updateMRMLFromWidget();
}

//------------------------------------------------------------------------------
void qMRMLClipNodeDisplayWidget::onClipNodeSelected(vtkMRMLNode* node)
{
  Q_D(qMRMLClipNodeDisplayWidget);
  if (!d->MRMLDisplayNode || d->IsUpdatingWidgetFromMRML)
  {
    return;
  }
  d->MRMLDisplayNode->SetAndObserveClipNodeID(node ? node->GetID() : nullptr);
}

//------------------------------------------------------------------------------
void qMRMLClipNodeDisplayWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLClipNodeDisplayWidget);
  if (d->IsUpdatingWidgetFromMRML)
  {
    return;
  }

  this->setEnabled(d->MRMLDisplayNode != nullptr);
  if (!d->MRMLDisplayNode)
  {
    return;
  }

  bool oldUpdating = d->IsUpdatingWidgetFromMRML;
  d->IsUpdatingWidgetFromMRML = true;

  bool wasBlocking = false;

  vtkMRMLClipNode* clipNode = d->MRMLDisplayNode ? vtkMRMLClipNode::SafeDownCast(d->MRMLDisplayNode->GetClipNode()) : nullptr;
  vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(d->MRMLDisplayNode);
  vtkMRMLSegmentationDisplayNode* segmentationDisplayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(d->MRMLDisplayNode);

  bool surfaceWidgetsVisible = modelDisplayNode != nullptr || segmentationDisplayNode != nullptr;
  bool capping = false;
  double capOpacity = 0.0;
  bool outline = false;
  if (modelDisplayNode)
  {
    capping = modelDisplayNode->GetClippingCapSurface();
    capOpacity = modelDisplayNode->GetClippingCapOpacity();
    outline = modelDisplayNode->GetClippingOutline();
  }
  else if (segmentationDisplayNode)
  {
    capping = segmentationDisplayNode->GetClippingCapSurface();
    capOpacity = segmentationDisplayNode->GetClippingCapOpacity();
    outline = segmentationDisplayNode->GetClippingOutline();
  }

  wasBlocking = d->ClipNodeSelector->blockSignals(true);
  d->ClipNodeSelector->setCurrentNode(clipNode);
  d->ClipNodeSelector->blockSignals(wasBlocking);
  d->ClipNodeWidget->setMRMLClipNode(clipNode);
  d->ClipNodeWidget->setEnabled(clipNode != nullptr);
  d->label_ClippingType->setEnabled(clipNode != nullptr);
  d->label_ClipBy->setEnabled(clipNode != nullptr);
  for (QAbstractButton* button : { d->UnionButton, d->IntersectionButton })
  {
    wasBlocking = button->blockSignals(true);
    button->setEnabled(clipNode != nullptr);
    button->setChecked(clipNode && (button == d->UnionButton) == (clipNode->GetClipType() == vtkMRMLClipNode::ClipUnion));
    button->blockSignals(wasBlocking);
  }

  wasBlocking = d->checkBox_Clipping->blockSignals(true);
  // Clipping can be enabled without a clip node: then a clip node is selected or created (see onClippingToggled)
  d->checkBox_Clipping->setEnabled(d->MRMLDisplayNode != nullptr);
  d->checkBox_Clipping->setChecked(d->MRMLDisplayNode ? d->MRMLDisplayNode->GetClipping() : false);
  d->checkBox_Clipping->blockSignals(wasBlocking);

  wasBlocking = d->checkBox_ClippingCapping->blockSignals(true);
  d->checkBox_ClippingCapping->setEnabled(clipNode != nullptr);
  d->checkBox_ClippingCapping->setChecked(capping);
  // The cap options are in a widget (not just a layout) so that their row can be taken out of the form layout
  d->setOptionalRowVisible(d->label_ClippingCapping, d->ClippingCapWidget, surfaceWidgetsVisible);
  d->checkBox_ClippingCapping->blockSignals(wasBlocking);

  wasBlocking = d->sliderWidget_ClippingCapOpacity->blockSignals(true);
  d->sliderWidget_ClippingCapOpacity->setEnabled(capping);
  d->sliderWidget_ClippingCapOpacity->setValue(capOpacity);
  d->sliderWidget_ClippingCapOpacity->blockSignals(wasBlocking);

  wasBlocking = d->checkBox_ClippingOutline->blockSignals(true);
  d->checkBox_ClippingOutline->setEnabled(clipNode != nullptr);
  d->checkBox_ClippingOutline->setChecked(outline);
  d->setOptionalRowVisible(d->label_ClippingOutline, d->checkBox_ClippingOutline, surfaceWidgetsVisible);
  d->checkBox_ClippingOutline->blockSignals(wasBlocking);

  // Keeping whole cells is only meaningful for models (segmentations may not be shown as meshes)
  wasBlocking = d->checkBox_ClippingKeepWholeCells->blockSignals(true);
  d->checkBox_ClippingKeepWholeCells->setEnabled(clipNode != nullptr);
  d->checkBox_ClippingKeepWholeCells->setChecked(clipNode ? clipNode->GetClippingMethod() == vtkMRMLClipNode::WholeCells : false);
  d->setOptionalRowVisible(d->label, d->checkBox_ClippingKeepWholeCells, modelDisplayNode != nullptr);
  d->checkBox_ClippingKeepWholeCells->blockSignals(wasBlocking);

  d->IsUpdatingWidgetFromMRML = oldUpdating;
}

//------------------------------------------------------------------------------
void qMRMLClipNodeDisplayWidget::updateMRMLFromWidget()
{
  Q_D(qMRMLClipNodeDisplayWidget);
  if (d->IsUpdatingWidgetFromMRML)
  {
    return;
  }

  if (!d->MRMLDisplayNode)
  {
    return;
  }

  MRMLNodeModifyBlocker displayNodeBlocker(d->MRMLDisplayNode);
  d->MRMLDisplayNode->SetClipping(d->checkBox_Clipping->isChecked());

  vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(d->MRMLDisplayNode);
  if (modelDisplayNode)
  {
    modelDisplayNode->SetClippingCapSurface(d->checkBox_ClippingCapping->isChecked());
    modelDisplayNode->SetClippingCapOpacity(d->sliderWidget_ClippingCapOpacity->value());
    modelDisplayNode->SetClippingOutline(d->checkBox_ClippingOutline->isChecked());
  }

  vtkMRMLSegmentationDisplayNode* segmentationDisplayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(d->MRMLDisplayNode);
  if (segmentationDisplayNode)
  {
    segmentationDisplayNode->SetClippingCapSurface(d->checkBox_ClippingCapping->isChecked());
    segmentationDisplayNode->SetClippingCapOpacity(d->sliderWidget_ClippingCapOpacity->value());
    segmentationDisplayNode->SetClippingOutline(d->checkBox_ClippingOutline->isChecked());
  }

  vtkMRMLClipNode* clipNode = d->MRMLDisplayNode->GetClipNode();
  MRMLNodeModifyBlocker clipNodeBlocker(clipNode);
  if (clipNode)
  {
    clipNode->SetClippingMethod(d->checkBox_ClippingKeepWholeCells->isChecked() ? vtkMRMLClipNode::WholeCells : vtkMRMLClipNode::Straight);
    clipNode->SetClipType(d->UnionButton->isChecked() ? vtkMRMLClipNode::ClipUnion : vtkMRMLClipNode::ClipIntersection);
  }
}

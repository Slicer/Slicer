/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

// VectorFieldRendering includes
#include "qMRMLColorLegendGroupBox.h"
#include "qSlicerVectorFieldRenderingModuleWidget.h"
#include "ui_qSlicerVectorFieldRenderingModuleWidget.h"
#include "vtkMRMLVectorFieldDisplayNode.h"
#include "vtkSlicerVectorFieldRenderingLogic.h"

// SubjectHierarchy includes
#include <qMRMLSortFilterSubjectHierarchyProxyModel.h>
#include <qMRMLSubjectHierarchyModel.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// MRML includes
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLProceduralColorNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLVolumeNode.h>

// CTK includes
#include <ctkVTKScalarsToColorsView.h>
#include <ctkVTKScalarsToColorsWidget.h>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkCommand.h>
#include <vtkNew.h>
#include <vtkWeakPointer.h>

// Qt includes
#include <QListWidgetItem>

//-----------------------------------------------------------------------------
class qSlicerVectorFieldRenderingModuleWidgetPrivate : public Ui_qSlicerVectorFieldRenderingModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerVectorFieldRenderingModuleWidget);

protected:
  qSlicerVectorFieldRenderingModuleWidget* const q_ptr;

public:
  qSlicerVectorFieldRenderingModuleWidgetPrivate(qSlicerVectorFieldRenderingModuleWidget& object);

  /// The procedural color node that the selected display node is colored by, if any: its color map can be edited here
  vtkMRMLProceduralColorNode* currentProceduralColorNode() const;

  vtkWeakPointer<vtkMRMLDisplayableNode> CurrentNode;
  vtkWeakPointer<vtkMRMLProceduralColorNode> ObservedColorNode;
  qMRMLColorLegendGroupBox* ColorLegendGroupBox{ nullptr };
  /// Editor of a color map that maps values (such as displacement in mm) to colors
  ctkVTKScalarsToColorsWidget* ColorMapWidget{ nullptr };
  /// The color map being edited, copied to and from the color node
  vtkNew<vtkColorTransferFunction> ColorMap;
};

//-----------------------------------------------------------------------------
qSlicerVectorFieldRenderingModuleWidgetPrivate::qSlicerVectorFieldRenderingModuleWidgetPrivate(qSlicerVectorFieldRenderingModuleWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
vtkMRMLProceduralColorNode* qSlicerVectorFieldRenderingModuleWidgetPrivate::currentProceduralColorNode() const
{
  Q_Q(const qSlicerVectorFieldRenderingModuleWidget);
  vtkMRMLVectorFieldDisplayNode* displayNode = q->currentDisplayNode();
  return (displayNode ? vtkMRMLProceduralColorNode::SafeDownCast(displayNode->GetColorNode()) : nullptr);
}

//-----------------------------------------------------------------------------
qSlicerVectorFieldRenderingModuleWidget::qSlicerVectorFieldRenderingModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVectorFieldRenderingModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerVectorFieldRenderingModuleWidget::~qSlicerVectorFieldRenderingModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModuleWidget::setup()
{
  Q_D(qSlicerVectorFieldRenderingModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // The nodes that can hold a vector field, and folders to organize them
  qMRMLSortFilterSubjectHierarchyProxyModel* sortFilterProxyModel = d->SubjectHierarchyTreeView->sortFilterProxyModel();
  sortFilterProxyModel->setNodeTypes(QStringList() << "vtkMRMLModelNode" << "vtkMRMLVectorVolumeNode" << "vtkMRMLTransformNode" << "vtkMRMLFolderDisplayNode");
  d->SubjectHierarchyTreeView->setColumnHidden(d->SubjectHierarchyTreeView->model()->idColumn(), true);
  d->SubjectHierarchyTreeView->setColumnHidden(d->SubjectHierarchyTreeView->model()->transformColumn(), true);
  d->SubjectHierarchyTreeView->setPluginAllowlist(QStringList() << "Models" << "Volumes" << "Transforms" << "Folder" << "Opacity" << "Visibility" << "VectorFieldRendering");
  connect(d->SubjectHierarchyTreeView, SIGNAL(currentItemsChanged(QList<vtkIdType>)), this, SLOT(onSubjectHierarchyItemsChanged(QList<vtkIdType>)));
  connect(d->FilterSearchBox, SIGNAL(textChanged(QString)), sortFilterProxyModel, SLOT(setNameFilter(QString)));

  connect(d->DisplayNodesListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(onDisplayNodeSelectionChanged()));
  connect(d->DisplayNodesListWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(onDisplayNodeItemChanged(QListWidgetItem*)));
  connect(d->AddDisplayNodeButton, SIGNAL(clicked()), this, SLOT(addDisplayNode()));
  connect(d->RemoveDisplayNodeButton, SIGNAL(clicked()), this, SLOT(removeCurrentDisplayNode()));

  // The color legend and the color map explain and set what the colors of the field mean, so they belong with the
  // rest of the coloring
  d->ColorLegendGroupBox = new qMRMLColorLegendGroupBox;
  d->ColorLegendGroupBox->setObjectName("ColorLegendGroupBox");
  d->VectorFieldDisplayWidget->addColoringWidget(d->ColorLegendGroupBox);
  connect(d->VectorFieldDisplayWidget,
          &qMRMLVectorFieldDisplayWidget::mrmlVectorFieldDisplayNodeChanged,
          d->ColorLegendGroupBox,
          [d](vtkMRMLVectorFieldDisplayNode* displayNode) { d->ColorLegendGroupBox->setPrimaryDisplayNode(displayNode); });

  d->ColorMapWidget = new ctkVTKScalarsToColorsWidget;
  d->ColorMapWidget->setObjectName("ColorMapWidget");
  d->ColorMapWidget->setToolTip(tr("Colors that the values are mapped to, for example the displacement magnitude in mm"));
  d->ColorMapWidget->setMinimumHeight(80);
  d->ColorMapWidget->setVerticalSliderVisible(false);
  d->VectorFieldDisplayWidget->addColoringWidget(d->ColorMapWidget);
  double validBounds[4] = { VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, 0.0, 1.0 };
  d->ColorMapWidget->view()->setValidBounds(validBounds);
  d->ColorMapWidget->view()->addColorTransferFunction(nullptr);
  d->ColorMapWidget->view()->setColorTransferFunctionToPlots(d->ColorMap);
  this->qvtkConnect(d->ColorMap, vtkCommand::ModifiedEvent, this, SLOT(onColorMapModified()), 0.0, Qt::QueuedConnection);

  this->setCurrentNode(nullptr);
}

//-----------------------------------------------------------------------------
vtkMRMLDisplayableNode* qSlicerVectorFieldRenderingModuleWidget::currentNode() const
{
  Q_D(const qSlicerVectorFieldRenderingModuleWidget);
  return d->CurrentNode;
}

//-----------------------------------------------------------------------------
vtkMRMLVectorFieldDisplayNode* qSlicerVectorFieldRenderingModuleWidget::currentDisplayNode() const
{
  Q_D(const qSlicerVectorFieldRenderingModuleWidget);
  QListWidgetItem* item = d->DisplayNodesListWidget->currentItem();
  if (!item || !this->mrmlScene())
  {
    return nullptr;
  }
  return vtkMRMLVectorFieldDisplayNode::SafeDownCast(this->mrmlScene()->GetNodeByID(item->data(Qt::UserRole).toString().toUtf8().constData()));
}

//-----------------------------------------------------------------------------
bool qSlicerVectorFieldRenderingModuleWidget::setEditedNode(vtkMRMLNode* node, QString role /* = QString()*/, QString context /* = QString()*/)
{
  Q_UNUSED(role);
  Q_UNUSED(context);
  vtkMRMLVectorFieldDisplayNode* displayNode = vtkMRMLVectorFieldDisplayNode::SafeDownCast(node);
  vtkMRMLDisplayableNode* displayableNode = (displayNode ? displayNode->GetDisplayableNode() : vtkMRMLDisplayableNode::SafeDownCast(node));
  if (!displayableNode || (!displayNode && !vtkSlicerVectorFieldRenderingLogic::HasVectorField(displayableNode)))
  {
    return false;
  }
  this->setCurrentNode(displayableNode);
  if (displayNode)
  {
    this->setCurrentDisplayNode(displayNode);
  }
  return true;
}

//-----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModuleWidget::onSubjectHierarchyItemsChanged(QList<vtkIdType> itemIDs)
{
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->mrmlScene());
  vtkMRMLNode* node = (shNode && itemIDs.size() == 1 ? shNode->GetItemDataNode(itemIDs[0]) : nullptr);
  this->setCurrentNode(node);
}

//-----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModuleWidget::setCurrentNode(vtkMRMLNode* node)
{
  Q_D(qSlicerVectorFieldRenderingModuleWidget);
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
  if (displayableNode != d->CurrentNode)
  {
    // The list follows the display nodes of the node, and whether it has a vector field follows its data
    this->qvtkReconnect(d->CurrentNode, displayableNode, vtkMRMLDisplayableNode::DisplayModifiedEvent, this, SLOT(updateDisplayNodesList()));
    this->qvtkReconnect(d->CurrentNode, displayableNode, vtkCommand::ModifiedEvent, this, SLOT(updateDisplayNodesList()));
    this->qvtkReconnect(d->CurrentNode, displayableNode, vtkMRMLModelNode::MeshModifiedEvent, this, SLOT(updateDisplayNodesList()));
    this->qvtkReconnect(d->CurrentNode, displayableNode, vtkMRMLVolumeNode::ImageDataModifiedEvent, this, SLOT(updateDisplayNodesList()));
    d->CurrentNode = displayableNode;
    if (displayableNode && d->SubjectHierarchyTreeView->currentNode() != displayableNode)
    {
      bool wasBlocked = d->SubjectHierarchyTreeView->blockSignals(true);
      d->SubjectHierarchyTreeView->setCurrentNode(displayableNode);
      d->SubjectHierarchyTreeView->blockSignals(wasBlocked);
    }
  }
  this->updateDisplayNodesList();
}

//-----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModuleWidget::updateDisplayNodesList()
{
  Q_D(qSlicerVectorFieldRenderingModuleWidget);
  bool hasVectorField = vtkSlicerVectorFieldRenderingLogic::HasVectorField(d->CurrentNode);
  d->NoVectorFieldLabel->setVisible(d->CurrentNode != nullptr && !hasVectorField);
  d->AddDisplayNodeButton->setEnabled(hasVectorField);

  // Keep the selection if that display node is still there
  QListWidgetItem* selectedItem = d->DisplayNodesListWidget->currentItem();
  QString selectedNodeID = (selectedItem ? selectedItem->data(Qt::UserRole).toString() : QString());
  bool wasBlocked = d->DisplayNodesListWidget->blockSignals(true);
  d->DisplayNodesListWidget->clear();
  int selectedRow = -1;
  for (vtkMRMLVectorFieldDisplayNode* displayNode : vtkSlicerVectorFieldRenderingLogic::GetVectorFieldDisplayNodes(d->CurrentNode))
  {
    QString nodeID = QString::fromUtf8(displayNode->GetID());
    QListWidgetItem* item = new QListWidgetItem(QString::fromUtf8(displayNode->GetName() ? displayNode->GetName() : ""), d->DisplayNodesListWidget);
    item->setData(Qt::UserRole, nodeID);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    // Checked if it is shown with the node: hidden by the eye icon, it is still checked
    item->setCheckState(displayNode->GetShowMode() == vtkMRMLDisplayNode::ShowDefault ? Qt::Checked : Qt::Unchecked);
    if (nodeID == selectedNodeID)
    {
      selectedRow = d->DisplayNodesListWidget->count() - 1;
    }
  }
  if (selectedRow < 0 && d->DisplayNodesListWidget->count() > 0)
  {
    selectedRow = 0;
  }
  d->DisplayNodesListWidget->setCurrentRow(selectedRow);
  d->DisplayNodesListWidget->blockSignals(wasBlocked);
  this->onDisplayNodeSelectionChanged();
}

//-----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModuleWidget::onDisplayNodeSelectionChanged()
{
  Q_D(qSlicerVectorFieldRenderingModuleWidget);
  vtkMRMLVectorFieldDisplayNode* displayNode = this->currentDisplayNode();
  d->RemoveDisplayNodeButton->setEnabled(displayNode != nullptr);
  d->VectorFieldDisplayWidget->setMRMLVectorFieldDisplayNode(displayNode);
  d->DisplayCollapsibleButton->setEnabled(displayNode != nullptr);
  this->updateColorMapFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModuleWidget::onDisplayNodeItemChanged(QListWidgetItem* item)
{
  if (!item || !this->mrmlScene())
  {
    return;
  }
  vtkMRMLVectorFieldDisplayNode* displayNode =
    vtkMRMLVectorFieldDisplayNode::SafeDownCast(this->mrmlScene()->GetNodeByID(item->data(Qt::UserRole).toString().toUtf8().constData()));
  vtkSlicerVectorFieldRenderingLogic::SetVectorFieldDisplayNodeShown(displayNode, item->checkState() == Qt::Checked);
}

//-----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModuleWidget::setCurrentDisplayNode(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  Q_D(qSlicerVectorFieldRenderingModuleWidget);
  QString nodeID = QString::fromUtf8(displayNode && displayNode->GetID() ? displayNode->GetID() : "");
  for (int row = 0; row < d->DisplayNodesListWidget->count(); ++row)
  {
    if (d->DisplayNodesListWidget->item(row)->data(Qt::UserRole).toString() == nodeID)
    {
      d->DisplayNodesListWidget->setCurrentRow(row);
      return;
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModuleWidget::addDisplayNode()
{
  Q_D(qSlicerVectorFieldRenderingModuleWidget);
  vtkMRMLVectorFieldDisplayNode* displayNode = vtkSlicerVectorFieldRenderingLogic::AddVectorFieldDisplayNode(d->CurrentNode);
  this->updateDisplayNodesList();
  this->setCurrentDisplayNode(displayNode);
}

//-----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModuleWidget::removeCurrentDisplayNode()
{
  vtkSlicerVectorFieldRenderingLogic::RemoveVectorFieldDisplayNode(this->currentDisplayNode());
  this->updateDisplayNodesList();
}

//-----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModuleWidget::updateColorMapFromMRML()
{
  Q_D(qSlicerVectorFieldRenderingModuleWidget);
  vtkMRMLProceduralColorNode* colorNode = d->currentProceduralColorNode();
  if (colorNode != d->ObservedColorNode)
  {
    this->qvtkReconnect(d->ObservedColorNode, colorNode, vtkCommand::ModifiedEvent, this, SLOT(updateColorMapFromMRML()));
    d->ObservedColorNode = colorNode;
  }
  // Only a color map from values to colors can be edited here; a color table is chosen in the coloring section
  vtkColorTransferFunction* colorMap = (colorNode ? colorNode->GetColorTransferFunction() : nullptr);
  d->ColorMapWidget->setVisible(colorMap != nullptr);
  if (!colorMap || vtkMRMLProceduralColorNode::IsColorMapEqual(d->ColorMap, colorMap))
  {
    return;
  }
  d->ColorMap->DeepCopy(colorMap);
  // The plot shows all the points of the color map
  double range[2] = { 0.0, 10.0 };
  colorMap->GetRange(range);
  double chartBounds[8] = { 0.0 };
  d->ColorMapWidget->view()->chartBounds(chartBounds);
  chartBounds[2] = 0.0;
  chartBounds[3] = range[1] * 1.1;
  d->ColorMapWidget->view()->setChartUserBounds(chartBounds);
  d->ColorMapWidget->view()->update();
}

//-----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModuleWidget::onColorMapModified()
{
  Q_D(qSlicerVectorFieldRenderingModuleWidget);
  vtkMRMLProceduralColorNode* colorNode = d->currentProceduralColorNode();
  vtkColorTransferFunction* colorMap = (colorNode ? colorNode->GetColorTransferFunction() : nullptr);
  if (colorMap && !vtkMRMLProceduralColorNode::IsColorMapEqual(d->ColorMap, colorMap))
  {
    colorMap->DeepCopy(d->ColorMap);
    colorNode->Modified();
  }
}

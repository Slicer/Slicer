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
#include <QTreeView>
#include <QStandardItemModel>
#include <QItemEditorFactory>
#include <QStyledItemDelegate>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QToolButton>
#include <QCheckBox>
#include <QDoubleSpinBox>

// CTK includes
#include <ctkLogger.h>

// qMRMLWidgets includes
#include <qMRMLLabelComboBox.h>
#include <qMRMLNodeComboBox.h>
#include <qMRMLUtils.h>

// EMSegment includes
#include "qSlicerEMSegmentAnatomicalTreeWidget.h"
#include "qSlicerEMSegmentAnatomicalTreeWidget_p.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>
#include <vtkMRMLEMSTreeNode.h>

// MRML includes
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLColorTableNode.h>

// VTK includes
#include <vtkLookupTable.h>

//--------------------------------------------------------------------------
static ctkLogger logger(
    "org.slicer.qtmodules.emsegment.widgets.qSlicerEMSegmentAnatomicalTreeWidget");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Custom item editors

namespace
{
//-----------------------------------------------------------------------------
class CustomDoubleSpinBox : public QDoubleSpinBox
{
public:
  CustomDoubleSpinBox(QWidget * newParent):QDoubleSpinBox(newParent)
    {
    this->setMinimum(0);
    this->setMaximum(1.0);
    this->setDecimals(2);
    this->setSingleStep(0.01);
    }
};
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentAnatomicalTreeWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentAnatomicalTreeWidgetPrivate::qSlicerEMSegmentAnatomicalTreeWidgetPrivate(qSlicerEMSegmentAnatomicalTreeWidget& object)
  : q_ptr(&object)
{
  this->EMSNode = 0;
  this->CurrentColorTableNode = 0;

  this->TreeView = 0;
  this->TreeModel = new QStandardItemModel;

  this->StructureNameEditable = false;
  this->LabelColumnVisible = false;
  this->ClassWeightColumnVisible = false;
  this->UpdateClassWeightColumnVisible = false;
  this->AtlasWeightColumnVisible = false;
  this->AlphaColumnVisible = false;
  this->ProbabilityMapColumnVisible = false;

  this->initializeHorizontalHeader();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidgetPrivate::setupUi(qSlicerEMSegmentWidget * widget)
{
  Q_Q(qSlicerEMSegmentAnatomicalTreeWidget);

  this->Ui_qSlicerEMSegmentAnatomicalTreeWidget::setupUi(widget);

  // Initialize treeView
  this->TreeView->setModel(this->TreeModel);
  this->TreeView->header()->setResizeMode(QHeaderView::ResizeToContents);

  // Register custom editors
  QItemEditorFactory *editorFactory = new QItemEditorFactory;
  editorFactory->registerEditor(
      QVariant::Double, new QStandardItemEditorCreator<CustomDoubleSpinBox>());
  QStyledItemDelegate* defaultItemDelegate =
      qobject_cast<QStyledItemDelegate*>(this->TreeView->itemDelegate());
  Q_ASSERT(defaultItemDelegate);
  defaultItemDelegate->setItemEditorFactory(editorFactory);

  // Connect Display MRML Id checkbox
  connect(this->DisplayMRMLIDsCheckBox, SIGNAL(toggled(bool)),
          q, SLOT(setMRMLIDsColumnVisible(bool)));

  // Connect Display Alpha checkbox
  connect(this->DisplayAlphaCheckBox, SIGNAL(toggled(bool)),
          q, SLOT(setAlphaColumnVisible(bool)));

  // Connect control buttons
  connect(this->CollapseAllButton, SIGNAL(clicked()), q, SLOT(collapseToDepthZero()));
  connect(this->ExpandAllButton, SIGNAL(clicked()), this->TreeView, SLOT(expandAll()));

  // Connect TreeModel
  connect(this->TreeModel, SIGNAL(itemChanged(QStandardItem*)),
          SLOT(onTreeItemChanged(QStandardItem*)));

  // Connect TreeView
  connect(this->TreeView, SIGNAL(clicked(QModelIndex)),
          SLOT(onTreeItemSelected(QModelIndex)));
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidgetPrivate::initializeHorizontalHeader()
{
  QStringList headerNames;
  headerNames.insert(Self::StructureColumn, "Structure");
  headerNames.insert(Self::IdColumn, "Id");
  headerNames.insert(Self::LabelColumn, "Label");
  headerNames.insert(Self::ClassWeightColumn, "Class");
  headerNames.insert(Self::UpdateClassWeightColumn, "Update");
  headerNames.insert(Self::AtlasWeightColumn, "Atlas");
  headerNames.insert(Self::AlphaColumn, "Alpha");
  headerNames.insert(Self::ProbabilityMapColumn, "Probability map");
  this->TreeModel->setHorizontalHeaderLabels(headerNames);

  this->TreeModel->setHeaderData(Self::ClassWeightColumn, Qt::Horizontal,
                                 QVariant(QIcon(":/Icons/Weight.png")), Qt::DecorationRole);
  this->TreeModel->setHeaderData(Self::AtlasWeightColumn, Qt::Horizontal,
                                 QVariant(QIcon(":/Icons/Weight.png")), Qt::DecorationRole);
  this->TreeModel->setHeaderData(Self::AlphaColumn, Qt::Horizontal,
                                 QVariant(QIcon(":/Icons/Weight.png")), Qt::DecorationRole);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidgetPrivate::populateTreeModel(
    vtkIdType treeNodeId, QStandardItem * item)
{
  Q_Q(qSlicerEMSegmentAnatomicalTreeWidget);
  Q_ASSERT(q->mrmlManager());
  Q_ASSERT(item);

  // Return if no valid treeNodeId is given
  if (treeNodeId == 0)
    {
    return;
    }

  // Get a reference to the associated treeNode
  vtkMRMLEMSTreeNode * treeNode = q->mrmlManager()->GetTreeNode(treeNodeId);
  Q_ASSERT(treeNode);
  if (!treeNode)
    {
    logger.error(QString("populateTreeModel - No treeNode associated with id: %1").arg(treeNodeId));
    return;
    }
  Q_ASSERT(treeNode->GetParametersNode());

  //logger.debug(QString("populateTreeModel - treeNodeId:%1, treeNodeName: %2").
               //arg(treeNodeId).arg(treeNode->GetName()));

  QStandardItem * structureItem = this->insertTreeRow(item, treeNodeId, treeNode);

  // Loop through current node children and recursively call ourself
  int numberOfChildren = q->mrmlManager()->GetTreeNodeNumberOfChildren(treeNodeId);
  for (int i = 0; i < numberOfChildren; i++)
    {
    this->populateTreeModel(
      q->mrmlManager()->GetTreeNodeChildNodeID(treeNodeId, i), structureItem);
    }
}

//-----------------------------------------------------------------------------
QStandardItem* qSlicerEMSegmentAnatomicalTreeWidgetPrivate::insertTreeRow(
    QStandardItem * parentItem, vtkIdType treeNodeId, vtkMRMLEMSTreeNode * treeNode)
{
  Q_Q(qSlicerEMSegmentAnatomicalTreeWidget);
  Q_ASSERT(treeNode);

  bool isLeaf = treeNode->GetNumberOfChildNodes() == 0;

  QList<QStandardItem*> itemList;

  // Structure item
  QStandardItem * structureItem = new QStandardItem(QString("%1").arg(treeNode->GetName()));
  structureItem->setData(QVariant(treeNodeId), Self::TreeNodeIDRole);
  structureItem->setData(QVariant(Self::StructureNameItemType), Self::TreeItemTypeRole);
  if (isLeaf && !this->LabelColumnVisible)
    {
    int labelId = treeNode->GetParametersNode()->GetLeafParametersNode()->GetIntensityLabel();
    structureItem->setData(
        qMRMLUtils::createColorPixmap(q->style(), this->colorFromLabelId(labelId)), Qt::DecorationRole);
    }
  structureItem->setEditable(this->StructureNameEditable);
  itemList << structureItem;

  // MRML ID item
  QStandardItem * mrmlIDItem = new QStandardItem(QString("%1").arg(treeNode->GetID()));
  mrmlIDItem->setData(QVariant(treeNodeId), Self::TreeNodeIDRole);
  mrmlIDItem->setData(QVariant(Self::MRMLIDItemType), Self::TreeItemTypeRole);
  mrmlIDItem->setEditable(false);
  itemList << mrmlIDItem;

  // Label item - Available only for tree leaf
  QStandardItem * labelItem = new QStandardItem();
  labelItem->setData(QVariant(treeNodeId), Self::TreeNodeIDRole);
  labelItem->setEditable(false);
  if (isLeaf) // Is treeNode a leaf ?
    {
    //Q_ASSERT(treeNode->GetParametersNode()->GetLeafParametersNode());
    //labelItem->setText(QString("%1").arg(
    //    treeNode->GetParametersNode()->GetLeafParametersNode()->GetIntensityLabel()));
    labelItem->setData(QVariant(Self::LabelItemType), Self::TreeItemTypeRole);
    }
  itemList << labelItem;

  // ClassWeight item
  QStandardItem * classWeightItem = new QStandardItem();
  classWeightItem->setData(
      QVariant(treeNode->GetParametersNode()->GetClassProbability()), Qt::DisplayRole);
  classWeightItem->setEditable(true);
  classWeightItem->setData(QVariant(treeNodeId), Self::TreeNodeIDRole);
  classWeightItem->setData(QVariant(Self::ClassWeightItemType), Self::TreeItemTypeRole);
  itemList << classWeightItem;

  // UpdateClassWeight item
  QStandardItem * updateClassWeightItem = new QStandardItem();
  //updateClassWeightItem->setData(QVariant(false), Qt::DisplayRole);
  updateClassWeightItem->setCheckable(true);
  updateClassWeightItem->setCheckState(Qt::Unchecked);
  updateClassWeightItem->setData(QVariant(treeNodeId), Self::TreeNodeIDRole);
  updateClassWeightItem->setData(QVariant(Self::UpdateClassWeightItemType),
                                 Self::TreeItemTypeRole);
  itemList << updateClassWeightItem;

  // AtlasWeight item
  QStandardItem * atlasWeightItem = new QStandardItem();
  atlasWeightItem->setData(
      QVariant(treeNode->GetParametersNode()->GetSpatialPriorWeight()), Qt::DisplayRole);
  atlasWeightItem->setEditable(true);
  atlasWeightItem->setData(QVariant(treeNodeId), Self::TreeNodeIDRole);
  atlasWeightItem->setData(QVariant(Self::AtlasWeightItemType), Self::TreeItemTypeRole);
  itemList << atlasWeightItem;

  // Alpha item - Available only for none tree leaf
  QStandardItem * alphaItem = new QStandardItem();
  alphaItem->setData(QVariant(treeNodeId), Self::TreeNodeIDRole);
  alphaItem->setEditable(false);
  if (!isLeaf) // Is treeNode NOT a leaf ?
    {
    Q_ASSERT(treeNode->GetParametersNode()->GetParentParametersNode());
    alphaItem->setData(
        QVariant(treeNode->GetParametersNode()->GetParentParametersNode()->GetAlpha()),
        Qt::DisplayRole);
    alphaItem->setEditable(true);
    alphaItem->setData(QVariant(Self::AlphaItemType), Self::TreeItemTypeRole);
    }
  itemList << alphaItem;

  // ProbabilityMap item - Available only for tree leaf
  QStandardItem * probabilityMapItem = new QStandardItem();
  probabilityMapItem->setData(QVariant(treeNodeId), Self::TreeNodeIDRole);
  probabilityMapItem->setEditable(false);
  if (isLeaf) // Is treeNode a leaf ?
    {
    probabilityMapItem->setData(QVariant(Self::ProbabilityMapItemType), Self::TreeItemTypeRole);
    }
  itemList << probabilityMapItem;

  parentItem->appendRow(itemList);

  // Set widget associated with labelItem
  if (isLeaf && this->LabelColumnVisible)
    {
    Q_ASSERT(treeNode->GetParametersNode()->GetLeafParametersNode());
    qMRMLLabelComboBox * labelComboBox = new qMRMLLabelComboBox;
    labelComboBox->setMaximumColorCount(10);
    labelComboBox->setMRMLScene(q->mrmlScene());
    labelComboBox->setMRMLColorNode(this->CurrentColorTableNode);
    logger.debug(QString("insertTreeRow - IntensityLabel: %1").
                 arg(treeNode->GetParametersNode()->GetLeafParametersNode()->GetIntensityLabel()));
    labelComboBox->setCurrentColor(
        treeNode->GetParametersNode()->GetLeafParametersNode()->GetIntensityLabel());
    this->TreeView->setIndexWidget(
        this->TreeModel->indexFromItem(labelItem), labelComboBox);
    }

  // Set widget associated with probabilityMapItem
  if (isLeaf && this->ProbabilityMapColumnVisible)
    {
    vtkIdType volumeId = q->mrmlManager()->GetTreeNodeSpatialPriorVolumeID(treeNodeId);
    vtkMRMLVolumeNode * volumeNode = q->mrmlManager()->GetVolumeNode(volumeId);
    qMRMLNodeComboBox * probabilityMapComboBox = new qMRMLNodeComboBox;
    QStringList nodeTypes;
    nodeTypes << "vtkMRMLVolumeNode";
    // Set treeNodeId property so that "onProbabilityMapChanged" can retrieve it
    probabilityMapComboBox->setProperty("treeNodeId", QVariant(treeNodeId));
    probabilityMapComboBox->setNodeTypes(nodeTypes);
    probabilityMapComboBox->setShowHidden(true);
    probabilityMapComboBox->setAddEnabled(false);
    probabilityMapComboBox->setRemoveEnabled(false);
    probabilityMapComboBox->setEditEnabled(false);
    probabilityMapComboBox->setMRMLScene(q->mrmlScene());
    probabilityMapComboBox->setCurrentNode(volumeNode);
    this->TreeView->setIndexWidget(
        this->TreeModel->indexFromItem(probabilityMapItem), probabilityMapComboBox);
    connect(probabilityMapComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
            this, SLOT(onProbabilityMapChanged(vtkMRMLNode*)));
    }

  return structureItem;
}

//-----------------------------------------------------------------------------
QColor qSlicerEMSegmentAnatomicalTreeWidgetPrivate::colorFromLabelId(int labelId)
{
  Q_ASSERT(this->CurrentColorTableNode);

  if (labelId < 0)
    {
    return QColor::Invalid;
    }

  double color[4];
  vtkLookupTable *table = this->CurrentColorTableNode->GetLookupTable();

  table->GetTableValue(labelId, color);

  // HACK - The alpha associated with Black was 0
  if (color[0] == 0 && color[1] == 0 && color[2] == 0)
    {
    color[3] = 1;
    }

  return QColor::fromRgbF(color[0], color[1], color[2], color[3]);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidgetPrivate::onTreeItemChanged(QStandardItem * treeItem)
{
  Q_ASSERT(treeItem);

  Q_Q(qSlicerEMSegmentAnatomicalTreeWidget);

  //logger.debug(QString("onTreeItemChanged - DisplayRole: %1").arg(treeItem->text()));
  int treeItemType = treeItem->data(Self::TreeItemTypeRole).toInt();
  int treeNodeId = treeItem->data(Self::TreeNodeIDRole).toInt();

  if (treeItemType == Self::StructureNameItemType)
    {
    q->mrmlManager()->SetTreeNodeName(treeNodeId, treeItem->text().toLatin1());
    }
  else if (treeItemType == Self::ClassWeightItemType)
    {
    q->mrmlManager()->SetTreeNodeClassProbability(
        treeNodeId, treeItem->data(Qt::DisplayRole).toDouble());
    }
  else if (treeItemType == Self::UpdateClassWeightItemType)
    {
    //bool update = (treeItem->checkState() == Qt::Checked);
    //logger.debug(QString("onTreeItemChanged - CheckStateRole: %1").arg(update));
    }
  else if (treeItemType == Self::AtlasWeightItemType)
    {
    q->mrmlManager()->SetTreeNodeSpatialPriorWeight(
        treeNodeId, treeItem->data(Qt::DisplayRole).toDouble());
    }
  else if (treeItemType == Self::AlphaItemType)
    {
    q->mrmlManager()->SetTreeNodeAlpha(
        treeNodeId, treeItem->data(Qt::DisplayRole).toDouble());
    }
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidgetPrivate::onTreeItemSelected(const QModelIndex & index)
{
  Q_Q(qSlicerEMSegmentAnatomicalTreeWidget);
  QStandardItem * item = this->TreeModel->itemFromIndex(index);
  Q_ASSERT(item);

  int treeNodeId = item->data(Self::TreeNodeIDRole).toInt();

  // Get a reference to the associated treeNode
  vtkMRMLEMSTreeNode * currentTreeNode = q->mrmlManager()->GetTreeNode(treeNodeId);
  Q_ASSERT(currentTreeNode);
  if (!currentTreeNode)
    {
    logger.error(QString("onTreeItemSelected - No treeNode associated with id: %1").arg(treeNodeId));
    return;
    }

  emit q->currentTreeNodeChanged(currentTreeNode);

//  vtkIdType volumeId = q->mrmlManager()->GetTreeNodeSpatialPriorVolumeID(treeNodeId);
//  vtkMRMLVolumeNode * volumeNode = q->mrmlManager()->GetVolumeNode(volumeId);
//  emit q->currentSpatialPriorVolumeNodeChanged(volumeNode);
//  emit q->currentSpatialPriorVolumeNodeChanged(volumeNode != 0);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidgetPrivate::onProbabilityMapChanged(vtkMRMLNode * node)
{
  Q_Q(qSlicerEMSegmentAnatomicalTreeWidget);
  if (!node)
    {
    return;
    }
  int treeNodeId = QObject::sender()->property("treeNodeId").toInt();
  Q_ASSERT(treeNodeId > 0);
  q->mrmlManager()->SetTreeNodeSpatialPriorVolumeID(
      treeNodeId, q->mrmlManager()->MapMRMLNodeIDToVTKNodeID(node->GetID()));
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentAnatomicalTreeWidget methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentAnatomicalTreeWidget::qSlicerEMSegmentAnatomicalTreeWidget(QWidget *newParent)
  : Superclass(newParent)
  , d_ptr(new qSlicerEMSegmentAnatomicalTreeWidgetPrivate(*this))
{
  Q_D(qSlicerEMSegmentAnatomicalTreeWidget);
  d->setupUi(this);

  // Columns hidden by default
  this->setStructureNameEditable(false);
  this->setMRMLIDsColumnVisible(false);
  this->setLabelColumnVisible(false);
  this->setClassWeightColumnVisible(false);
  this->setUpdateClassWeightColumnVisible(false);
  this->setAtlasWeightColumnVisible(false);
  this->setAlphaColumnVisible(false);
  this->setProbabilityMapColumnVisible(false);

  // Display checkboxes hidden by default
  this->setDisplayMRMLIDsCheckBoxVisible(false);
  this->setDisplayAlphaCheckBoxVisible(false);
}

//-----------------------------------------------------------------------------
qSlicerEMSegmentAnatomicalTreeWidget::~qSlicerEMSegmentAnatomicalTreeWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidget::setMRMLManager(vtkEMSegmentMRMLManager * newMRMLManager)
{
  // Listen if the current EMSNode changes
  this->qvtkReconnect(this->mrmlManager(), newMRMLManager, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromMRML()));

  this->Superclass::setMRMLManager(newMRMLManager);

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerEMSegmentAnatomicalTreeWidget);

  if (!this->mrmlManager())
    {
    logger.warn("updateWidgetFromMRML - MRMLManager is NULL");
    return;
    }
  if (!this->mrmlScene())
    {
    logger.warn("updateWidgetFromMRML - MRML Scene is NULL");
    return;
    }

  d->CurrentColorTableNode = vtkMRMLColorTableNode::SafeDownCast(
      this->mrmlScene()->GetNodeByID(this->mrmlManager()->GetColormap()?
                                     this->mrmlManager()->GetColormap():
                                     "vtkMRMLColorTableNodeLabels"));

  this->setUpdatesEnabled(false);

  // Clear model
  d->TreeModel->invisibleRootItem()->removeRows(0, d->TreeModel->invisibleRootItem()->rowCount());

  d->populateTreeModel(this->mrmlManager()->GetTreeRootNodeID(),
                       d->TreeModel->invisibleRootItem());


  d->TreeView->expandAll();

  this->setUpdatesEnabled(true);
}

//-----------------------------------------------------------------------------
namespace
{
void setStructureNameEditableRecursively(QStandardItem * item, bool editable)
{
  Q_ASSERT(item);
  item->setEditable(editable);
  for(int i = 0; i < item->rowCount(); ++i)
    {
    setStructureNameEditableRecursively(item->child(i), editable);
    }
}
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerEMSegmentAnatomicalTreeWidget, bool,
            structureNameEditable, StructureNameEditable);

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidget::setStructureNameEditable(bool editable)
{
  Q_D(qSlicerEMSegmentAnatomicalTreeWidget);
  if (d->StructureNameEditable == editable)
    {
    return;
    }

  setStructureNameEditableRecursively(d->TreeModel->invisibleRootItem(), editable);

  d->StructureNameEditable = editable;
}

//-----------------------------------------------------------------------------
bool qSlicerEMSegmentAnatomicalTreeWidget::mrmlIDsColumnVisible() const
{
  Q_D(const qSlicerEMSegmentAnatomicalTreeWidget);
  return (d->DisplayMRMLIDsCheckBox->checkState() == Qt::Checked);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidget::setMRMLIDsColumnVisible(bool visible)
{
  Q_D(qSlicerEMSegmentAnatomicalTreeWidget);
  d->TreeView->header()->setSectionHidden(ctkPimpl::IdColumn, !visible);
  d->DisplayMRMLIDsCheckBox->setChecked(visible);
}

//-----------------------------------------------------------------------------
bool qSlicerEMSegmentAnatomicalTreeWidget::isDisplayMRMLIDsCheckBoxVisible() const
{
  Q_D(const qSlicerEMSegmentAnatomicalTreeWidget);
  return d->DisplayMRMLIDsCheckBox->isVisible();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidget::setDisplayMRMLIDsCheckBoxVisible(bool visible)
{
  Q_D(qSlicerEMSegmentAnatomicalTreeWidget);
  d->DisplayMRMLIDsCheckBox->setVisible(visible);
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerEMSegmentAnatomicalTreeWidget, bool, labelColumnVisible, LabelColumnVisible);

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidget::setLabelColumnVisible(bool visible)
{
  Q_D(qSlicerEMSegmentAnatomicalTreeWidget);
  d->TreeView->header()->setSectionHidden(ctkPimpl::LabelColumn, !visible);
  d->LabelColumnVisible = visible;
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerEMSegmentAnatomicalTreeWidget, bool,
            classWeightColumnVisible, ClassWeightColumnVisible);

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidget::setClassWeightColumnVisible(bool visible)
{
  Q_D(qSlicerEMSegmentAnatomicalTreeWidget);
  d->TreeView->header()->setSectionHidden(ctkPimpl::ClassWeightColumn, !visible);
  d->ClassWeightColumnVisible = visible;
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerEMSegmentAnatomicalTreeWidget, bool,
            updateClassWeightColumnVisible, UpdateClassWeightColumnVisible);

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidget::setUpdateClassWeightColumnVisible(bool visible)
{
  Q_D(qSlicerEMSegmentAnatomicalTreeWidget);
  d->TreeView->header()->setSectionHidden(ctkPimpl::UpdateClassWeightColumn, !visible);
  d->UpdateClassWeightColumnVisible = visible;
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerEMSegmentAnatomicalTreeWidget, bool,
            atlasWeightColumnVisible, AtlasWeightColumnVisible);

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidget::setAtlasWeightColumnVisible(bool visible)
{
  Q_D(qSlicerEMSegmentAnatomicalTreeWidget);
  d->TreeView->header()->setSectionHidden(ctkPimpl::AtlasWeightColumn, !visible);
  d->AtlasWeightColumnVisible = visible;
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerEMSegmentAnatomicalTreeWidget, bool, alphaColumnVisible, AlphaColumnVisible);

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidget::setAlphaColumnVisible(bool visible)
{
  Q_D(qSlicerEMSegmentAnatomicalTreeWidget);
  d->TreeView->header()->setSectionHidden(ctkPimpl::AlphaColumn, !visible);
  d->AlphaColumnVisible = visible;
  d->DisplayAlphaCheckBox->setChecked(visible);
}

//-----------------------------------------------------------------------------
bool qSlicerEMSegmentAnatomicalTreeWidget::isDisplayAlphaCheckBoxVisible() const
{
  Q_D(const qSlicerEMSegmentAnatomicalTreeWidget);
  return d->DisplayAlphaCheckBox->isVisible();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidget::setDisplayAlphaCheckBoxVisible(bool visible)
{
  Q_D(qSlicerEMSegmentAnatomicalTreeWidget);
  d->DisplayAlphaCheckBox->setVisible(visible);
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerEMSegmentAnatomicalTreeWidget, bool,
            probabilityMapColumnVisible, ProbabilityMapColumnVisible);

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidget::setProbabilityMapColumnVisible(bool visible)
{
  Q_D(qSlicerEMSegmentAnatomicalTreeWidget);
  d->TreeView->header()->setSectionHidden(ctkPimpl::ProbabilityMapColumn, !visible);
  d->ProbabilityMapColumnVisible = visible;
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentAnatomicalTreeWidget::collapseToDepthZero()
{
  Q_D(qSlicerEMSegmentAnatomicalTreeWidget);

  d->TreeView->setUpdatesEnabled(false);
  d->TreeView->collapseAll();
  d->TreeView->expandToDepth(0);
  d->TreeView->setUpdatesEnabled(true);
}

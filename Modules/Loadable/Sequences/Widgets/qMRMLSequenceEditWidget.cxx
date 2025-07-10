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

==============================================================================*/

// qMRML includes
#include "qMRMLSequenceEditWidget.h"
#include "ui_qMRMLSequenceEditWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSequenceNode.h>

// VTK includes
#include <vtkCollection.h>

// Qt includes
#include <QDebug>
#include <QListWidgetItem>

#define FROM_CHAR_PTR_SAFE(charPtr) QString::fromStdString(charPtr == nullptr ? "" : charPtr)

enum
{
  DATA_NODE_VALUE_COLUMN,
  DATA_NODE_NAME_COLUMN,
  DATA_NODE_NUMBER_OF_COLUMNS // this must be the last line in this enum
};

//-----------------------------------------------------------------------------
class qMRMLSequenceEditWidgetPrivate : public Ui_qMRMLSequenceEditWidget
{
  Q_DECLARE_PUBLIC(qMRMLSequenceEditWidget);

protected:
  qMRMLSequenceEditWidget* const q_ptr;

public:
  qMRMLSequenceEditWidgetPrivate(qMRMLSequenceEditWidget& object);
  void init();

  /// Add node to the candidate node list widget.
  void addNodeToCandidateNodes(vtkMRMLNode* node);

  bool isDataNodeCandidate(vtkMRMLNode* node);

  /// Get a list of MLRML nodes that are in the scene but not added to the sequences data node at the chosen index value
  void dataNodeCandidates(std::vector<vtkSmartPointer<vtkMRMLNode>>& foundNodes, vtkMRMLSequenceNode* sequenceNode);

  void setCurrentDataNodeCandidate(vtkMRMLNode* node);

  void scrollToDataNodeByIndexValue(const QString& indexValue);

  vtkWeakPointer<vtkMRMLSequenceNode> SequenceNode;
  // data node class name that was used for populating the candidate node list
  QString DataNodeCandidatesClassName;
  bool DataNodeCandidatesUpdateNeeded{ true };
};

//-----------------------------------------------------------------------------
// qMRMLSequenceEditWidgetPrivate methods

//-----------------------------------------------------------------------------
qMRMLSequenceEditWidgetPrivate::qMRMLSequenceEditWidgetPrivate(qMRMLSequenceEditWidget& object)
  : q_ptr(&object)
{
  this->SequenceNode = nullptr;
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidgetPrivate::init()
{
  Q_Q(qMRMLSequenceEditWidget);
  this->setupUi(q);

  if (this->ComboBox_IndexType->count() == 0)
  {
    for (int indexType = 0; indexType < vtkMRMLSequenceNode::NumberOfIndexTypes; indexType++)
    {
      this->ComboBox_IndexType->addItem(vtkMRMLSequenceNode::GetIndexTypeAsString(indexType).c_str());
    }
  }

  this->TableWidget_DataNodes->setColumnWidth(DATA_NODE_VALUE_COLUMN, 30);
  this->TableWidget_DataNodes->setColumnWidth(DATA_NODE_NAME_COLUMN, 100);

  this->PushButton_AddCandidateNode->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowLeft));
  this->PushButton_RemoveDataNode->setIcon(QIcon(":/Icons/DataNodeDelete.png"));

  QObject::connect(this->ExpandButton_DataNodes, SIGNAL(toggled(bool)), q, SLOT(setCandidateNodesSectionVisible(bool)));

  QObject::connect(this->ListWidget_CandidateNodes,
                   SIGNAL(itemClicked(QListWidgetItem*)),
                   q,
                   SLOT(candidateNodeItemClicked(QListWidgetItem*)));
  QObject::connect(this->ListWidget_CandidateNodes,
                   SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                   q,
                   SLOT(candidateNodeItemDoubleClicked(QListWidgetItem*)));
  QObject::connect(LineEdit_NewCandidateNodeIndexValue, SIGNAL(returnPressed()), q, SLOT(onAddDataNodeButtonClicked()));

  QObject::connect(this->LineEdit_IndexName, SIGNAL(textEdited(const QString&)), q, SLOT(onIndexNameEdited()));
  QObject::connect(this->LineEdit_IndexUnit, SIGNAL(textEdited(const QString&)), q, SLOT(onIndexUnitEdited()));
  QObject::connect(
    this->ComboBox_IndexType, SIGNAL(currentIndexChanged(const QString&)), q, SLOT(onIndexTypeEdited(QString)));
  QObject::connect(this->TableWidget_DataNodes, SIGNAL(cellChanged(int, int)), q, SLOT(onDataNodeEdited(int, int)));
  QObject::connect(this->PushButton_AddCandidateNode, SIGNAL(clicked()), q, SLOT(onAddDataNodeButtonClicked()));
  QObject::connect(this->PushButton_RemoveDataNode, SIGNAL(clicked()), q, SLOT(onRemoveDataNodeButtonClicked()));

  this->ExpandButton_DataNodes->setChecked(false);

  q->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidgetPrivate::addNodeToCandidateNodes(vtkMRMLNode* node)
{
  Q_Q(qMRMLSequenceEditWidget);
  if (!node)
  {
    return;
  }
  QListWidgetItem* qlwi = new QListWidgetItem();
  qlwi->setText(node->GetName());
  qlwi->setData(Qt::UserRole, QString(node->GetID()));
  this->ListWidget_CandidateNodes->addItem(qlwi);
}

//-----------------------------------------------------------------------------
bool qMRMLSequenceEditWidgetPrivate::isDataNodeCandidate(vtkMRMLNode* node)
{
  Q_Q(const qMRMLSequenceEditWidget);
  if (node->GetHideFromEditors())
  {
    // don't show hidden nodes, they would clutter the view
    return false;
  }
  if (node->GetSingletonTag())
  {
    // don't allow adding singletons (mainly because we can only store one singleton node in a scene, so we couldn't
    // store it)
    return false;
  }
  if (node == this->SequenceNode)
  {
    // don't allow adding itself as data node
    return false;
  }
  if (!this->DataNodeCandidatesClassName.isEmpty())
  {
    if (this->DataNodeCandidatesClassName != node->GetClassName())
    {
      // class is not compatible with elements already in the sequence, don't show it
      return false;
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidgetPrivate::setCurrentDataNodeCandidate(vtkMRMLNode* node)
{
  Q_Q(const qMRMLSequenceEditWidget);
  if (!node || !node->GetID())
  {
    this->ListWidget_CandidateNodes->setCurrentRow(-1);
    return;
  }
  QString nodeIdToFind = node->GetID();
  int rowCount = this->ListWidget_CandidateNodes->count();
  for (int row = 0; row < rowCount; row++)
  {
    QListWidgetItem* item = this->ListWidget_CandidateNodes->item(row);
    if (item)
    {
      QString nodeID = item->data(Qt::UserRole).toString();
      if (nodeID == nodeIdToFind)
      {
        // found the node, set it as current
        this->ListWidget_CandidateNodes->setCurrentItem(item);
        return;
      }
    }
  }
  // not found
  this->ListWidget_CandidateNodes->setCurrentRow(-1);
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidgetPrivate::dataNodeCandidates(std::vector<vtkSmartPointer<vtkMRMLNode>>& foundNodes,
                                                        vtkMRMLSequenceNode* sequenceNode)
{
  Q_Q(const qMRMLSequenceEditWidget);
  foundNodes.clear();
  if (!sequenceNode)
  {
    qWarning() << Q_FUNC_INFO << "failed: invalid sequence node";
    return;
  }
  vtkMRMLScene* scene = q->mrmlScene();
  if (!scene)
  {
    return;
  }

  std::string dataNodeClassName = sequenceNode->GetDataNodeClassName();

  for (int i = 0; i < scene->GetNumberOfNodes(); i++)
  {
    vtkMRMLNode* currentNode = vtkMRMLNode::SafeDownCast(scene->GetNthNode(i));
    if (!this->isDataNodeCandidate(currentNode))
    {
      continue;
    }
    foundNodes.push_back(currentNode);
  }
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidgetPrivate::scrollToDataNodeByIndexValue(const QString& indexValue)
{
  Q_Q(const qMRMLSequenceEditWidget);
  if (indexValue.isEmpty())
  {
    return;
  }
  int itemNumber = this->SequenceNode->GetItemNumberFromIndexValue(indexValue.toStdString());
  if (itemNumber >= 0)
  {
    QModelIndex modelIndex = this->TableWidget_DataNodes->model()->index(itemNumber, 0);
    this->TableWidget_DataNodes->scrollTo(modelIndex);
    this->TableWidget_DataNodes->setCurrentIndex(modelIndex);
  }
}
//-----------------------------------------------------------------------------
// qMRMLSequenceEditWidget methods

//-----------------------------------------------------------------------------
qMRMLSequenceEditWidget::qMRMLSequenceEditWidget(QWidget* newParent)
  : Superclass(newParent)
  , d_ptr(new qMRMLSequenceEditWidgetPrivate(*this))
{
  Q_D(qMRMLSequenceEditWidget);
  d->init();
}

//-----------------------------------------------------------------------------
qMRMLSequenceEditWidget::~qMRMLSequenceEditWidget() = default;

//------------------------------------------------------------------------------
void qMRMLSequenceEditWidget::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
  this->qvtkConnect(
    this->mrmlScene(), vtkMRMLScene::NodeAddedEvent, this, SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));
  this->qvtkConnect(
    this->mrmlScene(), vtkMRMLScene::NodeRemovedEvent, this, SLOT(onNodeRemovedEvent(vtkObject*, vtkObject*)));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndImportEvent, this, SLOT(onMRMLSceneEndImportEvent()));
  this->qvtkConnect(
    this->mrmlScene(), vtkMRMLScene::EndBatchProcessEvent, this, SLOT(onMRMLSceneEndBatchProcessEvent()));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndCloseEvent, this, SLOT(onMRMLSceneEndCloseEvent()));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndRestoreEvent, this, SLOT(onMRMLSceneEndRestoreEvent()));

  this->updateWidgetFromMRML();
  this->updateCandidateNodesWidgetFromMRML(true);
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::setMRMLSequenceNode(vtkMRMLNode* browserNode)
{
  setMRMLSequenceNode(vtkMRMLSequenceNode::SafeDownCast(browserNode));
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::setMRMLSequenceNode(vtkMRMLSequenceNode* sequenceNode)
{
  Q_D(qMRMLSequenceEditWidget);

  if (d->SequenceNode == sequenceNode)
  {
    return; // no change
  }

  // Reconnect the input node's Modified() event observer
  this->qvtkReconnect(d->SequenceNode, sequenceNode, vtkCommand::ModifiedEvent, this, SLOT(onSequenceNodeModified()));
  d->SequenceNode = sequenceNode;

  if (d->SequenceNode && d->SequenceNode->GetIndexType() == vtkMRMLSequenceNode::NumericIndex)
  {
    d->LineEdit_NewCandidateNodeIndexValue->setText("0");
  }
  else
  {
    d->LineEdit_NewCandidateNodeIndexValue->setText("");
  }

  this->updateWidgetFromMRML();
  this->updateCandidateNodesWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLSequenceEditWidget);

  if (!d->SequenceNode || !this->mrmlScene())
  {
    d->Label_DataNodeTypeValue->setText("-");
    d->LineEdit_IndexName->setText("");
    d->LineEdit_IndexUnit->setText("");
    d->ComboBox_IndexType->setCurrentIndex(-1);
    d->TableWidget_DataNodes->clear();
    d->TableWidget_DataNodes->setRowCount(0);
    d->TableWidget_DataNodes->setColumnCount(0);
    d->ListWidget_CandidateNodes->clear();
    this->setEnabled(false);
    return;
  }

  this->setEnabled(true);

  // Put the correct properties in the sequence node table
  QString nodeType = QString::fromStdString(d->SequenceNode->GetDataNodeTagName());
  if (nodeType.isEmpty())
  {
    nodeType = "-";
  }
  d->Label_DataNodeTypeValue->setText(nodeType);

  bool numericIndex = d->SequenceNode->GetIndexType() == vtkMRMLSequenceNode::NumericIndex;
  d->Label_IndexIncrement->setVisible(numericIndex);
  d->DoubleSpinBox_IndexValueAutoIncrement->setVisible(numericIndex);
  d->Label_UseNodeNameAsIndexValue->setVisible(!numericIndex);
  d->CheckBox_UseNodeNameAsIndexValue->setVisible(!numericIndex);

  d->LineEdit_IndexName->setText(QString::fromStdString(d->SequenceNode->GetIndexName()));
  d->LineEdit_IndexUnit->setText(QString::fromStdString(d->SequenceNode->GetIndexUnit()));
  d->ComboBox_IndexType->setCurrentIndex(
    d->ComboBox_IndexType->findText(QString::fromStdString(d->SequenceNode->GetIndexTypeAsString())));

  // Display all of the sequence nodes
  d->TableWidget_DataNodes->clear();
  d->TableWidget_DataNodes->setRowCount(d->SequenceNode->GetNumberOfDataNodes());
  d->TableWidget_DataNodes->setColumnCount(DATA_NODE_NUMBER_OF_COLUMNS);
  std::stringstream valueHeader;
  valueHeader << d->SequenceNode->GetIndexName();
  valueHeader << " (" << d->SequenceNode->GetIndexUnit() << ")";
  QStringList SequenceNodesTableHeader;
  SequenceNodesTableHeader.insert(DATA_NODE_VALUE_COLUMN, valueHeader.str().c_str());
  SequenceNodesTableHeader.insert(DATA_NODE_NAME_COLUMN, tr("Name"));
  d->TableWidget_DataNodes->setHorizontalHeaderLabels(SequenceNodesTableHeader);

  int numberOfDataNodes = d->SequenceNode->GetNumberOfDataNodes();
  for (int dataNodeIndex = 0; dataNodeIndex < numberOfDataNodes; dataNodeIndex++)
  {
    std::string currentValue = d->SequenceNode->GetNthIndexValue(dataNodeIndex);
    vtkMRMLNode* currentDataNode = d->SequenceNode->GetNthDataNode(dataNodeIndex);

    if (currentDataNode == nullptr)
    {
      qCritical() << "qMRMLSequenceEditWidget::updateWidgetFromMRML invalid data node";
      continue;
    }

    QTableWidgetItem* valueItem = new QTableWidgetItem(QString::fromStdString(currentValue));
    valueItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    QTableWidgetItem* nameItem = new QTableWidgetItem(FROM_CHAR_PTR_SAFE(currentDataNode->GetName()));

    d->TableWidget_DataNodes->setItem(dataNodeIndex, DATA_NODE_VALUE_COLUMN, valueItem);
    d->TableWidget_DataNodes->setItem(dataNodeIndex, DATA_NODE_NAME_COLUMN, nameItem);
  }

  // d->TableWidget_DataNodes->resizeColumnsToContents();
  d->TableWidget_DataNodes->resizeRowsToContents();
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::updateCandidateNodesWidgetFromMRML(bool forceUpdate /* =false */)
{
  Q_D(qMRMLSequenceEditWidget);

  // If candidate nodes section is hidden then do not update candidate nodes list,
  // because it is an expensive operation.
  if (!d->SequenceNode || !this->isCandidateNodesSectionVisible())
  {
    d->ListWidget_CandidateNodes->clear();
    d->DataNodeCandidatesClassName.clear();
    d->DataNodeCandidatesUpdateNeeded = true;
    return;
  }

  QString newDataNodeCandidatesClassName(QString::fromStdString(d->SequenceNode->GetDataNodeClassName()));
  if (d->DataNodeCandidatesClassName != newDataNodeCandidatesClassName)
  {
    d->DataNodeCandidatesUpdateNeeded = true;
  }
  if (!forceUpdate && !d->DataNodeCandidatesUpdateNeeded)
  {
    // already up-to-date
    return;
  }
  d->DataNodeCandidatesClassName = newDataNodeCandidatesClassName;

  // Display the candidate data nodes
  std::vector<vtkSmartPointer<vtkMRMLNode>> candidateNodes;
  d->dataNodeCandidates(candidateNodes, d->SequenceNode);

  d->ListWidget_CandidateNodes->clear();

  for (vtkSmartPointer<vtkMRMLNode>& currentCandidateNode : candidateNodes)
  {
    d->addNodeToCandidateNodes(currentCandidateNode);
  }
  d->DataNodeCandidatesUpdateNeeded = false;
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::onDataNodeEdited(int row, int column)
{
  Q_D(qMRMLSequenceEditWidget);

  // Ensure that the user is editing, not the index changed programmatically
  if (d->TableWidget_DataNodes->currentRow() != row || d->TableWidget_DataNodes->currentColumn() != column)
  {
    return;
  }
  if (!d->SequenceNode)
  {
    return;
  }
  std::string currentIndexValue = d->SequenceNode->GetNthIndexValue(d->TableWidget_DataNodes->currentRow());
  if (currentIndexValue.empty())
  {
    return;
  }
  vtkMRMLNode* currentDataNode = d->SequenceNode->GetDataNodeAtValue(currentIndexValue.c_str());
  if (currentDataNode == nullptr)
  {
    return;
  }

  // Grab the text from the modified item
  QTableWidgetItem* qItem = d->TableWidget_DataNodes->item(row, column);
  QString qText = qItem->text();
  if (column == DATA_NODE_VALUE_COLUMN)
  {
    d->SequenceNode->UpdateIndexValue(currentIndexValue.c_str(), qText.toStdString().c_str());
  }
  if (column == DATA_NODE_NAME_COLUMN)
  {
    currentDataNode->SetName(qText.toStdString().c_str());
  }

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::onNodeAddedEvent(vtkObject* scene, vtkObject* aNode)
{
  Q_D(qMRMLSequenceEditWidget);
  Q_UNUSED(scene);
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing() || !this->isCandidateNodesSectionVisible())
  {
    d->DataNodeCandidatesUpdateNeeded = true;
    return;
  }
  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(aNode);
  if (node && d->isDataNodeCandidate(node))
  {
    d->addNodeToCandidateNodes(node);
  }
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::onNodeRemovedEvent(vtkObject* scene, vtkObject* aNode)
{
  Q_D(qMRMLSequenceEditWidget);
  Q_UNUSED(scene);
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing() || !this->isCandidateNodesSectionVisible())
  {
    d->DataNodeCandidatesUpdateNeeded = true;
    return;
  }
  // Remove single candidate node
  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(aNode);
  if (!node || !node->GetID() || !d->isDataNodeCandidate(node))
  {
    return;
  }
  QString nodeIdToRemove = QString::fromStdString(node->GetID());
  int numberOfRows = d->TableWidget_DataNodes->rowCount();
  for (int row = 0; row < numberOfRows; row++)
  {
    QTableWidgetItem* item = d->TableWidget_DataNodes->item(row, 0);
    QString nodeID = item->data(Qt::UserRole).toString();
    if (nodeID == nodeIdToRemove)
    {
      d->TableWidget_DataNodes->removeRow(row);
      return;
    }
  }
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::onMRMLSceneEndImportEvent()
{
  this->updateCandidateNodesWidgetFromMRML(true);
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::onMRMLSceneEndRestoreEvent()
{
  this->updateCandidateNodesWidgetFromMRML(true);
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::onMRMLSceneEndBatchProcessEvent()
{
  if (!this->mrmlScene())
  {
    return;
  }
  this->updateCandidateNodesWidgetFromMRML(true);
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::onMRMLSceneEndCloseEvent()
{
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
  {
    return;
  }
  this->updateCandidateNodesWidgetFromMRML(true);
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::onSequenceNodeModified()
{
  Q_D(qMRMLSequenceEditWidget);
  this->updateWidgetFromMRML();
  this->updateCandidateNodesWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::onIndexNameEdited()
{
  Q_D(qMRMLSequenceEditWidget);
  if (!d->SequenceNode)
  {
    return;
  }
  d->SequenceNode->SetIndexName(d->LineEdit_IndexName->text().toStdString().c_str());
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::onIndexUnitEdited()
{
  Q_D(qMRMLSequenceEditWidget);
  if (!d->SequenceNode)
  {
    return;
  }
  d->SequenceNode->SetIndexUnit(d->LineEdit_IndexUnit->text().toStdString().c_str());
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::onIndexTypeEdited(QString indexTypeString)
{
  Q_D(qMRMLSequenceEditWidget);
  if (!d->SequenceNode)
  {
    return;
  }
  d->SequenceNode->SetIndexTypeFromString(indexTypeString.toStdString().c_str());
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::onAddDataNodeButtonClicked()
{
  Q_D(qMRMLSequenceEditWidget);
  if (!d->SequenceNode)
  {
    return;
  }
  QList<QListWidgetItem*> selectedItems = d->ListWidget_CandidateNodes->selectedItems();
  std::vector<vtkSmartPointer<vtkMRMLNode>> nodesToAdd;
  for (QListWidgetItem* item : selectedItems)
  {
    QString currentCandidateNodeId = item->data(Qt::UserRole).toString();
    vtkMRMLNode* currentCandidateNode =
      d->SequenceNode->GetScene()->GetNodeByID(currentCandidateNodeId.toStdString().c_str());
    if (currentCandidateNode)
    {
      nodesToAdd.push_back(currentCandidateNode);
    }
  }
  QString addedNodeIndexValue;
  for (vtkSmartPointer<vtkMRMLNode>& node : nodesToAdd)
  {
    d->setCurrentDataNodeCandidate(node);
    addedNodeIndexValue = this->onAddCurrentCandidateNode();
  }
  d->scrollToDataNodeByIndexValue(addedNodeIndexValue);
}

//-----------------------------------------------------------------------------
QString qMRMLSequenceEditWidget::onAddCurrentCandidateNode()
{
  Q_D(qMRMLSequenceEditWidget);
  if (!d->SequenceNode)
  {
    return QString();
  }

  std::string currentIndexValue = d->LineEdit_NewCandidateNodeIndexValue->text().toStdString();
  if (currentIndexValue.empty())
  {
    qCritical() << Q_FUNC_INFO << "failed: Cannot add new data node, as Index value is not specified";
    return QString();
  }

  // Get the selected node
  QListWidgetItem* currentItem = d->ListWidget_CandidateNodes->currentItem();
  if (!currentItem)
  {
    qCritical() << Q_FUNC_INFO << "failed: Cannot add new data node, as current data item selection is invalid";
    return QString();
  }

  QString currentCandidateNodeId = currentItem->data(Qt::UserRole).toString();
  vtkMRMLNode* currentCandidateNode =
    d->SequenceNode->GetScene()->GetNodeByID(currentCandidateNodeId.toStdString().c_str());
  if (!currentCandidateNode)
  {
    qCritical() << Q_FUNC_INFO << "failed: Cannot add new data node, as current data item is invalid";
    return QString();
  }
  int wasModified = d->SequenceNode->StartModify();
  d->SequenceNode->SetDataNodeAtValue(currentCandidateNode, currentIndexValue.c_str());
  d->SequenceNode->EndModify(wasModified);

  // Restore candidate node selection / auto-advance to the next node
  if (d->CheckBox_AutoAdvanceDataSelection->checkState() == Qt::Checked)
  {
    // Get row index of current node
    QAbstractItemModel* candidateNodesModel = d->ListWidget_CandidateNodes->model();
    QModelIndex start = candidateNodesModel->index(0, 0);
    QModelIndexList moduleIndexes =
      candidateNodesModel->match(start, Qt::UserRole, currentCandidateNodeId, /* hits= */ 1, Qt::MatchExactly);
    if (moduleIndexes.count() > 0)
    {
      // Found the current node index, move to the next one
      int nextItemRowIndex = moduleIndexes.at(0).row() + 1;
      if (nextItemRowIndex < d->ListWidget_CandidateNodes->count())
      {
        // not at the end of the list, so select the next item
        d->ListWidget_CandidateNodes->setCurrentRow(nextItemRowIndex);
        // update index value from the upcoming node's name
        if (d->SequenceNode->GetIndexType() != vtkMRMLSequenceNode::NumericIndex
            && d->CheckBox_UseNodeNameAsIndexValue->isChecked())
        {
          this->candidateNodeItemClicked(d->ListWidget_CandidateNodes->currentItem());
        }
      }
      else
      {
        // we are at the end of the list (already added the last element),
        // so unselect the item to prevent duplicate adding of the last element
        d->ListWidget_CandidateNodes->setCurrentRow(-1);
      }
    }
  }

  if (d->SequenceNode->GetIndexType() == vtkMRMLSequenceNode::NumericIndex)
  {
    // Auto-increment the Index value in the new data textbox
    QString oldIndexValue = d->LineEdit_NewCandidateNodeIndexValue->text();
    bool isIndexValueNumeric = false;
    double oldIndexNumber = oldIndexValue.toDouble(&isIndexValueNumeric);
    if (isIndexValueNumeric)
    {
      double incrementValue = d->DoubleSpinBox_IndexValueAutoIncrement->value();
      QString newIndexValue = QString::number(oldIndexNumber + incrementValue);
      d->LineEdit_NewCandidateNodeIndexValue->setText(newIndexValue);
    }
  }

  return QString::fromStdString(currentIndexValue);
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::onRemoveDataNodeButtonClicked()
{
  Q_D(qMRMLSequenceEditWidget);
  if (!d->SequenceNode)
  {
    return;
  }
  QList<QTableWidgetItem*> selectedItems = d->TableWidget_DataNodes->selectedItems();
  QList<int> rowsToDelete;
  for (QTableWidgetItem* item : selectedItems)
  {
    if (item->column() == DATA_NODE_VALUE_COLUMN)
    {
      rowsToDelete << item->row();
    }
  }
  std::sort(rowsToDelete.begin(), rowsToDelete.end());
  if (rowsToDelete.size() == 1)
  {
    // remove single node
    std::string currentIndexValue = d->SequenceNode->GetNthIndexValue(rowsToDelete[0]);
    d->SequenceNode->RemoveDataNodeAtValue(currentIndexValue);
  }
  else
  {
    // remove many nodes
    bool wasModify = d->SequenceNode->StartModify();
    for (QList<int>::reverse_iterator rowIt = rowsToDelete.rbegin(); rowIt != rowsToDelete.rend(); ++rowIt)
    {
      std::string currentIndexValue = d->SequenceNode->GetNthIndexValue(*rowIt);
      d->SequenceNode->RemoveDataNodeAtValue(currentIndexValue);
    }
    d->SequenceNode->EndModify(wasModify);
  }
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::candidateNodeItemClicked(QListWidgetItem* item)
{
  Q_D(qMRMLSequenceEditWidget);
  if (!d->SequenceNode || d->SequenceNode->GetIndexType() == vtkMRMLSequenceNode::NumericIndex)
  {
    return;
  }
  if (item)
  {
    d->LineEdit_NewCandidateNodeIndexValue->setText(item->text());
  }
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::candidateNodeItemDoubleClicked(QListWidgetItem* item)
{
  Q_D(qMRMLSequenceEditWidget);
  if (item)
  {
    QString addedNodeIndexValue = this->onAddCurrentCandidateNode();
    d->scrollToDataNodeByIndexValue(addedNodeIndexValue);
  }
}

//-----------------------------------------------------------------------------
void qMRMLSequenceEditWidget::setCandidateNodesSectionVisible(bool show)
{
  Q_D(qMRMLSequenceEditWidget);

  QSignalBlocker blocker(d->ExpandButton_DataNodes);
  d->ExpandButton_DataNodes->setChecked(show);

  d->GroupBox_CandidateNodes->setVisible(show);
  if (show)
  {
    this->updateCandidateNodesWidgetFromMRML();
  }
}

//-----------------------------------------------------------------------------
bool qMRMLSequenceEditWidget::isCandidateNodesSectionVisible()
{
  Q_D(qMRMLSequenceEditWidget);
  return d->GroupBox_CandidateNodes->isVisible();
}

//-----------------------------------------------------------------------------
vtkMRMLSequenceNode* qMRMLSequenceEditWidget::mrmlSequenceNode()
{
  Q_D(qMRMLSequenceEditWidget);
  return d->SequenceNode;
}

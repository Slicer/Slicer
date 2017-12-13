/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

// QT includes
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <QString>
#include <QToolButton>

// CTK includes
#include <ctkPopupWidget.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkStringArray.h>
#include <vtkTable.h>

// qMRML includes
#include "qMRMLTableView.h"
#include "qMRMLTableView_p.h"
#include "qMRMLTableModel.h"

// MRML includes
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLPlotDataNode.h>
#include <vtkMRMLPlotChartNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLTableNode.h>
#include <vtkMRMLTableViewNode.h>

// STL includes
#include <deque>

#define CTK_CHECK_AND_RETURN_IF_FAIL(FUNC) \
  if (!FUNC(Q_FUNC_INFO))       \
    {                              \
    return;                        \
    }

#define CTK_CHECK_AND_RETURN_FALSE_IF_FAIL(FUNC) \
  if (!FUNC(Q_FUNC_INFO))            \
    {                                   \
    return false;                       \
    }

//------------------------------------------------------------------------------
qMRMLTableViewPrivate::qMRMLTableViewPrivate(qMRMLTableView& object)
  : q_ptr(&object)
  , MRMLScene(0)
  , MRMLTableViewNode(0)
{
}

//---------------------------------------------------------------------------
qMRMLTableViewPrivate::~qMRMLTableViewPrivate()
{
}

//------------------------------------------------------------------------------
void qMRMLTableViewPrivate::init()
{
  Q_Q(qMRMLTableView);

  qMRMLTableModel* tableModel = new qMRMLTableModel(q);
  QSortFilterProxyModel* sortFilterModel = new QSortFilterProxyModel(q);
  sortFilterModel->setSourceModel(tableModel);
  q->setModel(sortFilterModel);

  q->horizontalHeader()->setStretchLastSection(false);

  // Let the view expand in both directions
  q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  this->PopupWidget = new ctkPopupWidget;
  QHBoxLayout* popupLayout = new QHBoxLayout;
  popupLayout->addWidget(new QToolButton);
  this->PopupWidget->setLayout(popupLayout);
}

//---------------------------------------------------------------------------
void qMRMLTableViewPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  //Q_Q(qMRMLTableView);
  if (newScene == this->MRMLScene)
    {
    return;
    }
  this->qvtkReconnect(
    this->mrmlScene(), newScene,
    vtkMRMLScene::StartBatchProcessEvent, this, SLOT(startProcessing()));

  this->qvtkReconnect(
    this->mrmlScene(), newScene,
    vtkMRMLScene::EndBatchProcessEvent, this, SLOT(endProcessing()));
  this->MRMLScene = newScene;
}

// --------------------------------------------------------------------------
void qMRMLTableViewPrivate::startProcessing()
{
}

// --------------------------------------------------------------------------
void qMRMLTableViewPrivate::endProcessing()
{
  this->updateWidgetFromViewNode();
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLTableViewPrivate::mrmlScene()
{
  return this->MRMLScene;
}

// --------------------------------------------------------------------------
bool qMRMLTableViewPrivate::verifyTableModelAndNode(const char* methodName) const
{
  Q_Q(const qMRMLTableView);
  if (!q->tableModel())
    {
    qWarning() << "qMRMLTableView:: " << methodName << " failed: invalid model";
    return false;
    }
  if (!q->mrmlTableNode())
    {
    qWarning() << "qMRMLTableView::" << methodName << " failed: invalid node";
    return false;
    }
  return true;
}

// --------------------------------------------------------------------------
void qMRMLTableViewPrivate::updateWidgetFromViewNode()
{
  Q_Q(qMRMLTableView);
  if (!this->MRMLScene || !this->MRMLTableViewNode)
    {
    q->setMRMLTableNode((vtkMRMLNode*)NULL);
    return;
    }

  if (!q->isEnabled())
    {
    return;
    }

  // Get the TableNode
  q->setMRMLTableNode(this->MRMLTableViewNode->GetTableNode());
}

//------------------------------------------------------------------------------
qMRMLTableView::qMRMLTableView(QWidget *_parent)
  : QTableView(_parent)
  , d_ptr(new qMRMLTableViewPrivate(*this))
{
  Q_D(qMRMLTableView);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLTableView::~qMRMLTableView()
{
}

//------------------------------------------------------------------------------
qMRMLTableModel* qMRMLTableView::tableModel()const
{
  return qobject_cast<qMRMLTableModel*>(this->sortFilterProxyModel()->sourceModel());
}

//------------------------------------------------------------------------------
QSortFilterProxyModel* qMRMLTableView::sortFilterProxyModel()const
{
  return qobject_cast<QSortFilterProxyModel*>(this->model());
}

//------------------------------------------------------------------------------
void qMRMLTableView::setMRMLTableNode(vtkMRMLNode* node)
{
  this->setMRMLTableNode(vtkMRMLTableNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLTableView::setMRMLTableNode(vtkMRMLTableNode* node)
{
  qMRMLTableModel* mrmlModel = this->tableModel();
  if (!mrmlModel)
    {
    qCritical("qMRMLTableView::setMRMLTableNode failed: invalid model");
    return;
    }

  mrmlModel->setMRMLTableNode(node);
  this->sortFilterProxyModel()->invalidate();

  this->horizontalHeader()->setMinimumSectionSize(60);
  this->resizeColumnsToContents();

  emit selectionChanged();
}

//------------------------------------------------------------------------------
vtkMRMLTableNode* qMRMLTableView::mrmlTableNode()const
{
  qMRMLTableModel* mrmlModel = this->tableModel();
  if (!mrmlModel)
    {
    qCritical("qMRMLTableView::mrmlTableNode failed: model is invalid");
    return 0;
    }
  return mrmlModel->mrmlTableNode();
}

//------------------------------------------------------------------------------
bool qMRMLTableView::transposed()const
{
  Q_D(const qMRMLTableView);
  CTK_CHECK_AND_RETURN_FALSE_IF_FAIL(d->verifyTableModelAndNode)
  return tableModel()->transposed();
}

//------------------------------------------------------------------------------
void qMRMLTableView::setTransposed(bool transposed)
{
  Q_D(qMRMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)
  tableModel()->setTransposed(transposed);
}

//------------------------------------------------------------------------------
void qMRMLTableView::keyPressEvent(QKeyEvent *event)
{
  if(event->matches(QKeySequence::Copy) )
    {
    this->copySelection();
    return;
    }
  if(event->matches(QKeySequence::Paste) )
    {
    this->pasteSelection();
    return;
    }

  // Prevent giving the focus to the previous/next widget if arrow keys are used
  // at the edge of the table (without this: if the current cell is in the top
  // row and user press the Up key, the focus goes from the table to the previous
  // widget in the tab order)
  if (model() && (
    (event->key() == Qt::Key_Left && currentIndex().column() == 0)
    || (event->key() == Qt::Key_Up && currentIndex().row() == 0)
    || (event->key() == Qt::Key_Right && currentIndex().column() == model()->columnCount()-1)
    || (event->key() == Qt::Key_Down && currentIndex().row() == model()->rowCount()-1) ) )
    {
    return;
    }
  QTableView::keyPressEvent(event);
}

//-----------------------------------------------------------------------------
void qMRMLTableView::copySelection()
{
  Q_D(qMRMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)

  if (!selectionModel()->hasSelection())
    {
    return;
    }

  qMRMLTableModel* mrmlModel = tableModel();
  QItemSelectionModel* selection = selectionModel();
  QString textToCopy;
  bool firstLine = true;
  for (int rowIndex=0; rowIndex<mrmlModel->rowCount(); rowIndex++)
    {
    if (!selection->rowIntersectsSelection(rowIndex, QModelIndex()))
      {
      // no items are selected in this entire row, skip it
      continue;
      }
    if (firstLine)
      {
      firstLine = false;
      }
    else
      {
      textToCopy.append('\n');
      }
    bool firstItemInLine = true;
    for (int columnIndex=0; columnIndex<mrmlModel->columnCount(); columnIndex++)
      {
      if (!selection->columnIntersectsSelection(columnIndex, QModelIndex()))
        {
        // no items are selected in this entire column, skip it
        continue;
        }
      if (firstItemInLine)
        {
        firstItemInLine = false;
        }
      else
        {
        textToCopy.append('\t');
        }
      QStandardItem *item = mrmlModel->item(rowIndex, columnIndex);
      if (item->isCheckable())
        {
        textToCopy.append(item->checkState() == Qt::Checked ? "1" : "0");
        }
      else
        {
        textToCopy.append(item->text());
        }
      }
    }

  QApplication::clipboard()->setText(textToCopy);
}

//-----------------------------------------------------------------------------
void qMRMLTableView::pasteSelection()
{
  Q_D(qMRMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)

  QString text = QApplication::clipboard()->text();
  if (text.isEmpty())
    {
    return;
    }
  QStringList lines = text.split('\n');
  if (lines.empty())
    {
    // nothing to paste
    return;
    }
  if (lines.back().isEmpty())
    {
    // usually there is an extra empty line at the end
    // remove that to avoid adding an extra empty line to the table
    lines.pop_back();
    }
  if (lines.empty())
    {
    // nothing to paste
    return;
    }

  // If there is no selection then paste from top-left
  qMRMLTableModel* mrmlModel = tableModel();
  int rowIndex = currentIndex().row();
  if (rowIndex < 0)
    {
    rowIndex = 0;
    }
  int startColumnIndex = currentIndex().column();
  if (startColumnIndex < 0)
    {
    startColumnIndex = 0;
    }

  // If there are multiple table views then each cell modification would trigger
  // a table update, which may be very slow in case of large tables, therefore
  // we need to use StartModify/EndModify.
  vtkMRMLTableNode* tableNode = mrmlTableNode();
  int wasModified = tableNode->StartModify();

  // Pre-allocate new rows (to reduce number of updateModelFromMRML() calls
  if (tableNode->GetNumberOfColumns() == 0)
    {
    // insertRow() may insert two rows if the table is empty (one column header + one data item),
    // which could cause an extra row added to the table. To prevent this, we add a column instead,
    // which is just a single value.
    insertColumn();
    mrmlModel->updateModelFromMRML();
    }
  for (int i = lines.size() - (mrmlModel->rowCount() - rowIndex); i>0; i--)
    {
    insertRow();
    }
  mrmlModel->updateModelFromMRML();

  foreach(QString line, lines)
    {
    int columnIndex = startColumnIndex;
    QStringList cells = line.split('\t');
    foreach(QString cell, cells)
      {
      // Pre-allocate new columns (enough for at least for storing all the items in the current row)
      if (columnIndex >= mrmlModel->columnCount())
        {
        for (int i = cells.size() - (mrmlModel->columnCount() - startColumnIndex); i>0; i--)
          {
          insertColumn();
          }
        mrmlModel->updateModelFromMRML();
        }
      // Set values in items
      QStandardItem* item = mrmlModel->item(rowIndex,columnIndex);
      if (item != NULL)
        {
        if (item->isCheckable())
          {
          item->setCheckState(cell.toInt() == 0 ? Qt::Unchecked : Qt::Checked);
          }
        else
          {
          item->setText(cell);
          }
        }
      else
        {
        qWarning() << "Failed to set " << cell << " in table cell (" << rowIndex << ", " << columnIndex << ")";
        }
      columnIndex++;
      }
    rowIndex++;
    }
  tableNode->EndModify(wasModified);
}

//-----------------------------------------------------------------------------
void qMRMLTableView::plotSelection()
{
  Q_D(qMRMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)

  vtkMRMLTableNode* tableNode = mrmlTableNode();

  if(!this->mrmlScene())
    {
    qWarning() << "qMRMLTableView::plotSelection failed: no mrmlScene available";
    return;
    }

  qMRMLTableModel* mrmlModel = tableModel();
  QItemSelectionModel* selection = selectionModel();
  std::deque<int> columnIndices;
  for (int columnIndex = 0; columnIndex < mrmlModel->columnCount(); columnIndex++)
    {
    if (!selection->columnIntersectsSelection(columnIndex, QModelIndex()))
      {
      // no items are selected in this entire column, skip it
      continue;
      }
    vtkAbstractArray* column = tableNode->GetTable()->GetColumn(columnIndex);
    if (!column || !column->GetName())
      {
      QString message = QString("Column %1 is invalid. Failed to generate a plot").arg(columnIndex);
      qCritical() << Q_FUNC_INFO << ": " << message;
      QMessageBox::warning(NULL, tr("Failed to create Plot"), message);
      }
    int columnDataType = column->GetDataType();
    if (columnDataType == VTK_STRING || columnDataType == VTK_BIT)
      {
      QString message = QString("Type of column %1 is 'string' or 'bit'. Plotting of these types are currently not supported."
        " Please convert the data type of this column to numeric using Table module's Column properties section,"
        " or select different columns for plotting.").arg(column->GetName());
      qCritical() << Q_FUNC_INFO << ": " << message;
      QMessageBox::warning(NULL, tr("Failed to create Plot"), message);
      return;
      }
    columnIndices.push_back(columnIndex);
    }

  if (columnIndices.size() == 0)
    {
    QString message = QString("To generate a plot, please select at least one cell in the table");
    qCritical() << Q_FUNC_INFO << ": " << message;
    QMessageBox::warning(NULL, tr("Failed to plot data"), message);
    return;
    }

  // Check the DataType of the (X-Axis) Column
  std::string xColumnName = "Indexes";
  // We need to provide a valid pointer to an array, even if we use Y axis indices
  vtkAbstractArray* xColumn = tableNode->GetTable()->GetColumn(columnIndices[0]);
  if (columnIndices.size() > 1)
    {
    // If there are more than one columns selected then use the first one as X column
    xColumnName = xColumn->GetName();
    columnIndices.pop_front();
    }

  vtkMRMLSelectionNode* selectionNode = vtkMRMLSelectionNode::SafeDownCast(
  this->mrmlScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton"));

  if (!selectionNode)
    {
    qWarning() << "qMRMLTableView::plotSelection failed: invalid selection Node";
    return;
    }

  // Set a Plot Layout
  vtkMRMLLayoutNode* layoutNode = vtkMRMLLayoutNode::SafeDownCast(
    this->mrmlScene()->GetFirstNodeByClass("vtkMRMLLayoutNode"));
  if (!layoutNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get layout node!";
    return;
    }
  int viewArray = layoutNode->GetViewArrangement();
  if (viewArray != vtkMRMLLayoutNode::SlicerLayoutConventionalPlotView  &&
      viewArray != vtkMRMLLayoutNode::SlicerLayoutFourUpPlotView        &&
      viewArray != vtkMRMLLayoutNode::SlicerLayoutFourUpPlotTableView   &&
      viewArray != vtkMRMLLayoutNode::SlicerLayoutOneUpPlotView         &&
      viewArray != vtkMRMLLayoutNode::SlicerLayoutThreeOverThreePlotView)
    {
    layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutConventionalPlotView);
    }

  vtkSmartPointer<vtkMRMLPlotChartNode> plotChartNode = vtkMRMLPlotChartNode::SafeDownCast(
    this->mrmlScene()->GetNodeByID(selectionNode->GetActivePlotChartID()));

  if (!plotChartNode)
    {
    plotChartNode = vtkSmartPointer<vtkMRMLPlotChartNode>::New();
    this->mrmlScene()->AddNode(plotChartNode);
    selectionNode->SetActivePlotChartID(plotChartNode->GetID());
    }

  // Remove columns/plots not selected from plotChartNode
  plotChartNode->RemoveAllPlotDataNodeIDs();

  for (std::deque<int>::iterator columnIndexIt = columnIndices.begin(); columnIndexIt != columnIndices.end(); ++columnIndexIt)
    {
    std::string yColumnName = tableNode->GetColumnName(*columnIndexIt);

    // Check if there is already a PlotDataNode for this Column and avoid duplication
    vtkSmartPointer<vtkCollection> colPlots = vtkSmartPointer<vtkCollection>::Take(
      this->mrmlScene()->GetNodesByClassByName("vtkMRMLPlotDataNode", yColumnName.c_str()));
    if (colPlots == NULL)
      {
      continue;
      }
    vtkMRMLPlotDataNode *plotDataNode = NULL;
    for (int plotIndex = 0; plotIndex < colPlots->GetNumberOfItems(); plotIndex++)
      {
      plotDataNode = vtkMRMLPlotDataNode::SafeDownCast(colPlots->GetItemAsObject(plotIndex));
      if (plotDataNode != NULL)
        {
        break;
        }
      }

    // Create a PlotDataNode if a usable node has not been found
    if (plotDataNode == NULL)
      {
      plotDataNode = vtkMRMLPlotDataNode::SafeDownCast(this->mrmlScene()->AddNewNodeByClass(
        "vtkMRMLPlotDataNode", yColumnName.c_str()));
      plotDataNode->SetXColumnName(xColumnName);
      plotDataNode->SetYColumnName(yColumnName);
      plotDataNode->SetAndObserveTableNodeID(tableNode->GetID());
      }

    std::string namePlotDataNode = plotDataNode->GetName();
    std::size_t found = namePlotDataNode.find("Markups");
    if (found != std::string::npos)
      {
      plotChartNode->RemovePlotDataNodeID(plotDataNode->GetID());
      plotDataNode->GetNodeReference("Markups")->RemoveNodeReferenceIDs("Markups");
      this->mrmlScene()->RemoveNode(plotDataNode);
      continue;
      }

    // Set the type of the PlotDataNode
    const char* Type = plotChartNode->GetAttribute("Type");
    if (strcmp(Type, "Custom"))
      {
      plotDataNode->SetType(plotDataNode->GetPlotTypeFromString(Type));
      }

    // Add the reference of the PlotDataNode in the active PlotChartNode
    plotChartNode->AddAndObservePlotDataNodeID(plotDataNode->GetID());
    }
}

//-----------------------------------------------------------------------------
void qMRMLTableView::insertColumn()
{
  Q_D(qMRMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)
  if (tableModel()->transposed())
    {
    mrmlTableNode()->AddEmptyRow();
    }
  else
    {
    mrmlTableNode()->AddColumn();
    }
}

//-----------------------------------------------------------------------------
void qMRMLTableView::deleteColumn()
{
  Q_D(qMRMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)
  tableModel()->removeSelectionFromMRML(selectionModel()->selectedIndexes(), false);
  clearSelection();
}

//-----------------------------------------------------------------------------
void qMRMLTableView::insertRow()
{
  Q_D(qMRMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)
  if (tableModel()->transposed())
    {
    mrmlTableNode()->AddColumn();
    }
  else
    {
    mrmlTableNode()->AddEmptyRow();
    }
}

//-----------------------------------------------------------------------------
void qMRMLTableView::deleteRow()
{
  Q_D(qMRMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)
  tableModel()->removeSelectionFromMRML(selectionModel()->selectedIndexes(), true);
  clearSelection();
}

//-----------------------------------------------------------------------------
bool qMRMLTableView::firstRowLocked()const
{
  Q_D(const qMRMLTableView);
  CTK_CHECK_AND_RETURN_FALSE_IF_FAIL(d->verifyTableModelAndNode)
  if (tableModel()->transposed())
    {
    return mrmlTableNode()->GetUseFirstColumnAsRowHeader();
    }
  else
    {
    return mrmlTableNode()->GetUseColumnNameAsColumnHeader();
    }
}


//-----------------------------------------------------------------------------
void qMRMLTableView::setFirstRowLocked(bool locked)
{
  Q_D(qMRMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)
  if (tableModel()->transposed())
    {
    if (mrmlTableNode()->GetUseFirstColumnAsRowHeader()==locked)
      {
      //no change
      return;
      }
    mrmlTableNode()->SetUseFirstColumnAsRowHeader(locked);
    }
  else
    {
    if (mrmlTableNode()->GetUseColumnNameAsColumnHeader()==locked)
      {
      //no change
      return;
      }
    mrmlTableNode()->SetUseColumnNameAsColumnHeader(locked);
    }
  this->resizeColumnsToContents();
}

//-----------------------------------------------------------------------------
bool qMRMLTableView::firstColumnLocked()const
{
  Q_D(const qMRMLTableView);
  CTK_CHECK_AND_RETURN_FALSE_IF_FAIL(d->verifyTableModelAndNode)
  if (tableModel()->transposed())
    {
    return mrmlTableNode()->GetUseColumnNameAsColumnHeader();
    }
  else
    {
    return mrmlTableNode()->GetUseFirstColumnAsRowHeader();
    }
}

//-----------------------------------------------------------------------------
void qMRMLTableView::setFirstColumnLocked(bool locked)
{
  Q_D(qMRMLTableView);
  CTK_CHECK_AND_RETURN_IF_FAIL(d->verifyTableModelAndNode)
  if (tableModel()->transposed())
    {
    if (mrmlTableNode()->GetUseColumnNameAsColumnHeader()==locked)
      {
      //no change
      return;
      }
    mrmlTableNode()->SetUseColumnNameAsColumnHeader(locked);
    }
  else
    {
    if (mrmlTableNode()->GetUseFirstColumnAsRowHeader()==locked)
      {
      //no change
      return;
      }
    mrmlTableNode()->SetUseFirstColumnAsRowHeader(locked);
    }
  this->resizeColumnsToContents();
}

//------------------------------------------------------------------------------
void qMRMLTableView::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLTableView);
  if (newScene == d->MRMLScene)
    {
    return;
    }

  d->setMRMLScene(newScene);

  if (d->MRMLTableViewNode && newScene != d->MRMLTableViewNode->GetScene())
    {
    this->setMRMLTableViewNode(0);
    }

  emit mrmlSceneChanged(newScene);
}

//---------------------------------------------------------------------------
void qMRMLTableView::setMRMLTableViewNode(vtkMRMLTableViewNode* newTableViewNode)
{
  Q_D(qMRMLTableView);
  if (d->MRMLTableViewNode == newTableViewNode)
    {
    return;
    }

  // connect modified event on TableViewNode to updating the widget
  d->qvtkReconnect(
    d->MRMLTableViewNode, newTableViewNode,
    vtkCommand::ModifiedEvent, d, SLOT(updateWidgetFromViewNode()));

  // cache the TableViewNode
  d->MRMLTableViewNode = newTableViewNode;

  // make sure the gui is up to date
  d->updateWidgetFromViewNode();
}

//---------------------------------------------------------------------------
vtkMRMLTableViewNode* qMRMLTableView::mrmlTableViewNode()const
{
  Q_D(const qMRMLTableView);
  return d->MRMLTableViewNode;
}

//---------------------------------------------------------------------------
vtkMRMLScene* qMRMLTableView::mrmlScene()const
{
  Q_D(const qMRMLTableView);
  return d->MRMLScene;
}

//---------------------------------------------------------------------------
QList<int> qMRMLTableView::selectedMRMLTableColumnIndices()const
{
  QList<int> mrmlColumnIndexList;
  QModelIndexList selection = selectionModel()->selectedIndexes();
  qMRMLTableModel* tableModel = this->tableModel();
  QModelIndex index;
  foreach(index, selection)
    {
    int mrmlColumnIndex = tableModel->mrmlTableColumnIndex(index);
    if (!mrmlColumnIndexList.contains(mrmlColumnIndex))
      {
      // insert unique row/column index only
      mrmlColumnIndexList.push_back(mrmlColumnIndex);
      }
    }
  return mrmlColumnIndexList;
}

//---------------------------------------------------------------------------
void qMRMLTableView::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
  QTableView::selectionChanged(selected, deselected);
  emit selectionChanged();
}

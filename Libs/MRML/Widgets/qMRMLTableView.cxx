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
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <QToolButton>

// CTK includes
#include <ctkPopupWidget.h>

// qMRML includes
#include "qMRMLTableView.h"
#include "qMRMLTableView_p.h"
#include "qMRMLTableModel.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLTableNode.h>
#include <vtkMRMLTableViewNode.h>

#define VERIFY_TABLE_MODEL_AND_NODE(methodName) \
  if (!this->tableModel()) \
    { \
    qWarning() << "qMRMLTableView:: " << #methodName << " failed: invalid model"; \
    return; \
    } \
  if (!this->mrmlTableNode()) \
    { \
    qWarning() << "qMRMLTableView::" << #methodName << " failed: invalid node"; \
    return; \
    }

#define VERIFY_TABLE_MODEL_AND_NODE_RETURN_VALUE(methodName, returnValue) \
  if (!this->tableModel()) \
    { \
    qWarning() << "qMRMLTableView:: " << #methodName << " failed: invalid model"; \
    return returnValue; \
    } \
  if (!this->mrmlTableNode()) \
    { \
    qWarning() << "qMRMLTableView::" << #methodName << " failed: invalid node"; \
    return returnValue; \
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
  VERIFY_TABLE_MODEL_AND_NODE_RETURN_VALUE(transposed, false);
  return tableModel()->transposed();
}

//------------------------------------------------------------------------------
void qMRMLTableView::setTransposed(bool transposed)
{
  VERIFY_TABLE_MODEL_AND_NODE(setTransposed);
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
  VERIFY_TABLE_MODEL_AND_NODE(copySelection);

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
      textToCopy.append(mrmlModel->item(rowIndex,columnIndex)->text());
      }
    }

  QApplication::clipboard()->setText(textToCopy);
}

//-----------------------------------------------------------------------------
void qMRMLTableView::pasteSelection()
{
  VERIFY_TABLE_MODEL_AND_NODE(pasteSelection);

  QString text = QApplication::clipboard()->text();
  if (text.isEmpty())
    {
    return;
    }

  qMRMLTableModel* mrmlModel = tableModel();
  int rowIndex = currentIndex().row();
  int startColumnIndex = currentIndex().column();
  QStringList lines = text.split('\n');
  if (!lines.empty())
    {
    // If there are multiple table views then each cell modification would trigger
    // a table update, which may be very slow in case of large tables, therefore
    // we need to use StartModify/EndModify.
    vtkMRMLTableNode* tableNode = mrmlTableNode();
    int wasModified = tableNode->StartModify();
    foreach(QString line, lines)
      {
      if (rowIndex>=mrmlModel->rowCount())
          {
          // reached last row in the table, ignore subsequent rows
          break;
          }
      int columnIndex = startColumnIndex;
      QStringList cells = line.split('\t');
      foreach(QString cell, cells)
        {
        if (columnIndex>=mrmlModel->columnCount())
          {
          // reached last column in the table, ignore subsequent columns
          break;
          }
        mrmlModel->item(rowIndex,columnIndex)->setText(cell);
        columnIndex++;
        }
      rowIndex++;
      }
    tableNode->EndModify(wasModified);
    }
}

//-----------------------------------------------------------------------------
void qMRMLTableView::insertColumn()
{
  VERIFY_TABLE_MODEL_AND_NODE(insertColumn);
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
  VERIFY_TABLE_MODEL_AND_NODE(deleteColumn);
  tableModel()->removeSelectionFromMRML(selectionModel()->selectedIndexes(), false);
  clearSelection();
}

//-----------------------------------------------------------------------------
void qMRMLTableView::insertRow()
{
  VERIFY_TABLE_MODEL_AND_NODE(insertRow);
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
  VERIFY_TABLE_MODEL_AND_NODE(deleteRow);
  tableModel()->removeSelectionFromMRML(selectionModel()->selectedIndexes(), true);
  clearSelection();
}

//-----------------------------------------------------------------------------
bool qMRMLTableView::firstRowLocked()const
{
  VERIFY_TABLE_MODEL_AND_NODE_RETURN_VALUE(firstRowLocked, false);
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
  VERIFY_TABLE_MODEL_AND_NODE(firstRowLocked);
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
  VERIFY_TABLE_MODEL_AND_NODE_RETURN_VALUE(firstColumnLocked, false);
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
  VERIFY_TABLE_MODEL_AND_NODE(setFirstColumnLocked);
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

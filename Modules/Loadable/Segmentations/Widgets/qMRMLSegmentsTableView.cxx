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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Segmentations includes
#include "qMRMLSegmentsModel.h"
#include "qMRMLSegmentsTableView.h"
#include "qMRMLSortFilterSegmentsProxyModel.h"
#include "ui_qMRMLSegmentsTableView.h"
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkSegmentation.h"
#include "vtkSegment.h"

// Segmentations logic includes
#include "vtkSlicerSegmentationsModuleLogic.h"

// Terminologies includes
#include "qSlicerTerminologyItemDelegate.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLSliceNode.h>

// SlicerQt includes
#include <qSlicerApplication.h>
#include <qSlicerCoreApplication.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerModuleManager.h>
#include <qSlicerAbstractCoreModule.h>
#include <qMRMLItemDelegate.h>
#include <qMRMLSliceWidget.h>

// VTK includes
#include <vtkWeakPointer.h>

// Qt includes
#include <QAction>
#include <QDebug>
#include <QKeyEvent>
#include <QStringList>
#include <QToolButton>
#include <QContextMenuEvent>
#include <QMenu>
#include <QModelIndex>
#include <QMessageBox>

#define ID_PROPERTY "ID"
#define VISIBILITY_PROPERTY "Visible"
#define STATUS_PROPERTY "Status"

//-----------------------------------------------------------------------------
class qMRMLSegmentsTableViewPrivate: public Ui_qMRMLSegmentsTableView
{
  Q_DECLARE_PUBLIC(qMRMLSegmentsTableView);

protected:
  qMRMLSegmentsTableView* const q_ptr;
public:
  qMRMLSegmentsTableViewPrivate(qMRMLSegmentsTableView& object);
  void init();

  /// Sets table message and takes care of the visibility of the label
  void setMessage(const QString& message);

public:
  /// Segmentation MRML node containing shown segments
  vtkWeakPointer<vtkMRMLSegmentationNode> SegmentationNode;

  /// Flag determining whether the long-press per-view segment visibility options are available
  bool AdvancedSegmentVisibility;

  QIcon VisibleIcon;
  QIcon InvisibleIcon;

  QIcon NotStartedIcon;
  QIcon InProgressIcon;
  QIcon FlaggedIcon;
  QIcon CompletedIcon;

  /// Currently, if we are requesting segment display information from the
  /// segmentation display node,  the display node may emit modification events.
  /// We make sure these events do not interrupt the update process by setting
  /// IsUpdatingWidgetFromMRML to true when an update is already in progress.
  bool IsUpdatingWidgetFromMRML;

  qMRMLSegmentsModel* Model;
  qMRMLSortFilterSegmentsProxyModel* SortFilterModel;

private:
  QStringList HiddenSegmentIDs;
};

//-----------------------------------------------------------------------------
qMRMLSegmentsTableViewPrivate::qMRMLSegmentsTableViewPrivate(qMRMLSegmentsTableView& object)
  : q_ptr(&object)
  , SegmentationNode(nullptr)
  , AdvancedSegmentVisibility(false)
  , IsUpdatingWidgetFromMRML(false)
  , Model(nullptr)
{
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableViewPrivate::init()
{
  Q_Q(qMRMLSegmentsTableView);

  this->setupUi(q);

  this->Model = new qMRMLSegmentsModel(this->SegmentsTable);
  this->SortFilterModel = new qMRMLSortFilterSegmentsProxyModel(this->SegmentsTable);
  this->SortFilterModel->setSourceModel(this->Model);
  this->SegmentsTable->setModel(this->SortFilterModel);

  this->VisibleIcon = QIcon(":/Icons/Small/SlicerVisible.png");
  this->InvisibleIcon = QIcon(":/Icons/Small/SlicerInvisible.png");

  this->NotStartedIcon = QIcon(":Icons/NotStarted.png");
  this->InProgressIcon = QIcon(":Icons/InProgress.png");
  this->FlaggedIcon = QIcon(":Icons/Flagged.png");
  this->CompletedIcon = QIcon(":Icons/Completed.png");

  this->setMessage(QString());

  this->SegmentsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  this->SegmentsTable->horizontalHeader()->setSectionResizeMode(this->Model->nameColumn(), QHeaderView::Stretch);
  this->SegmentsTable->horizontalHeader()->setStretchLastSection(0);
  this->SegmentsTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  // Select rows
  this->SegmentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

  // Unset read-only by default (edit triggers are double click and edit key press)
  q->setReadOnly(false);

  // Make connections
  QObject::connect(this->SegmentsTable->selectionModel(), &QItemSelectionModel::selectionChanged, q, &qMRMLSegmentsTableView::onSegmentSelectionChanged);
  QObject::connect(this->Model, &qMRMLSegmentsModel::segmentAboutToBeModified, q, &qMRMLSegmentsTableView::segmentAboutToBeModified);
  QObject::connect(this->SegmentsTable, &QTableView::clicked, q, &qMRMLSegmentsTableView::onSegmentsTableClicked);
  QObject::connect(this->FilterLineEdit, &QLineEdit::textChanged, this->SortFilterModel, &qMRMLSortFilterSegmentsProxyModel::setTextFilter);
  QObject::connect(this->ShowNotStartedButton, &QToolButton::toggled, this->SortFilterModel, &qMRMLSortFilterSegmentsProxyModel::setShowNotStarted);
  QObject::connect(this->ShowInProgressButton, &QToolButton::toggled, this->SortFilterModel, &qMRMLSortFilterSegmentsProxyModel::setShowInProgress);
  QObject::connect(this->ShowCompletedButton, &QToolButton::toggled, this->SortFilterModel, &qMRMLSortFilterSegmentsProxyModel::setShowCompleted);
  QObject::connect(this->ShowFlaggedButton, &QToolButton::toggled, this->SortFilterModel, &qMRMLSortFilterSegmentsProxyModel::setShowFlagged);

  // Set item delegate to handle color and opacity changes
  qMRMLItemDelegate* itemDelegate = new qMRMLItemDelegate(this->SegmentsTable);
  this->SegmentsTable->setItemDelegateForColumn(this->Model->colorColumn(), new qSlicerTerminologyItemDelegate(this->SegmentsTable));
  this->SegmentsTable->setItemDelegateForColumn(this->Model->opacityColumn(), itemDelegate);
  this->SegmentsTable->installEventFilter(q);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::onSegmentsTableClicked(const QModelIndex& modelIndex)
{
  Q_D(qMRMLSegmentsTableView);
  QString segmentId = d->SortFilterModel->segmentIDFromIndex(modelIndex);
  QStandardItem* item = d->Model->itemFromSegmentID(segmentId);
  if (!d->SegmentationNode)
    {
    return;
    }

  Qt::ItemFlags flags = item->flags();
  if (!flags.testFlag(Qt::ItemIsSelectable))
    {
    return;
    }

  vtkSegment* segment = d->SegmentationNode->GetSegmentation()->GetSegment(segmentId.toStdString());
  if (modelIndex.column() == d->Model->visibilityColumn())
    {
    // Set all visibility types to segment referenced by button toggled
    int visible = !item->data(qMRMLSegmentsModel::VisibilityRole).toInt();
    this->setSegmentVisibility(segmentId, visible, -1, -1, -1);
    }
  else if (modelIndex.column() == d->Model->statusColumn())
    {
    int status = vtkSlicerSegmentationsModuleLogic::GetSegmentStatus(segment);
    switch (status)
      {
       case vtkSlicerSegmentationsModuleLogic::SegmentStatus::Flagged:
         status = vtkSlicerSegmentationsModuleLogic::SegmentStatus::Completed;
         break;
       default:
         ++status;
         if (status >= vtkSlicerSegmentationsModuleLogic::SegmentStatus::LastStatus)
           {
           status = 0;
           }
         break;
      }
    vtkSlicerSegmentationsModuleLogic::SetSegmentStatus(segment, status);
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableViewPrivate::setMessage(const QString& message)
{
  this->SegmentsTableMessageLabel->setVisible(!message.isEmpty());
  this->SegmentsTableMessageLabel->setText(message);
}

//-----------------------------------------------------------------------------
// qMRMLSegmentsTableView methods

//-----------------------------------------------------------------------------
qMRMLSegmentsTableView::qMRMLSegmentsTableView(QWidget* _parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLSegmentsTableViewPrivate(*this))
{
  Q_D(qMRMLSegmentsTableView);
  d->init();
}

//-----------------------------------------------------------------------------
qMRMLSegmentsTableView::~qMRMLSegmentsTableView()
= default;

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::setSegmentationNode(vtkMRMLNode* node)
{
  Q_D(qMRMLSegmentsTableView);

  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(node);
  d->SegmentationNode = segmentationNode;
  d->Model->setSegmentationNode(d->SegmentationNode);

  // Connect segment added/removed and display modified events to population of the table
  qvtkReconnect(d->SegmentationNode, segmentationNode, vtkSegmentation::SegmentAdded,
    this, SLOT(onSegmentAddedOrRemoved()));
  qvtkReconnect(d->SegmentationNode, segmentationNode, vtkSegmentation::SegmentRemoved,
    this, SLOT(onSegmentAddedOrRemoved()));
  this->onSegmentAddedOrRemoved();
}

//---------------------------------------------------------------------------
void qMRMLSegmentsTableView::onSegmentAddedOrRemoved()
{
  Q_D(qMRMLSegmentsTableView);

  if (!d->SegmentationNode)
    {
    d->setMessage(tr("No node is selected"));
    return;
    }
  else if (d->SegmentationNode->GetSegmentation()->GetNumberOfSegments() == 0)
    {
    d->setMessage(tr("Empty segmentation"));
    return;
    }
  d->setMessage(QString());
}

//---------------------------------------------------------------------------
void qMRMLSegmentsTableView::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLSegmentsTableView);
  if (newScene == this->mrmlScene())
    {
    return;
    }

  if (d->SegmentationNode && newScene != d->SegmentationNode->GetScene())
    {
    this->setSegmentationNode(nullptr);
    }

  Superclass::setMRMLScene(newScene);
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qMRMLSegmentsTableView::segmentationNode()
{
  Q_D(qMRMLSegmentsTableView);

  return d->SegmentationNode;
}

//--------------------------------------------------------------------------
qMRMLSortFilterSegmentsProxyModel* qMRMLSegmentsTableView::sortFilterProxyModel()const
{
  Q_D(const qMRMLSegmentsTableView);
  if (!d->SortFilterModel)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid sort filter proxy model";
    return nullptr;
    }
  return d->SortFilterModel;
}

//--------------------------------------------------------------------------
qMRMLSegmentsModel* qMRMLSegmentsTableView::model()const
{
  Q_D(const qMRMLSegmentsTableView);
  if (!d->Model)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid data model";
    return nullptr;
    }
  return d->Model;
}

//-----------------------------------------------------------------------------
QTableView* qMRMLSegmentsTableView::tableWidget()
{
  Q_D(qMRMLSegmentsTableView);
  return d->SegmentsTable;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::onSegmentSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
  Q_D(qMRMLSegmentsTableView);
  if (d->SegmentsTable->signalsBlocked())
    {
    return;
    }
  emit selectionChanged(selected, deselected);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::onVisibility3DActionToggled(bool visible)
{
  QAction* senderAction = qobject_cast<QAction*>(sender());
  if (!senderAction)
    {
    return;
    }

  // Set 3D visibility to segment referenced by action toggled
  this->setSegmentVisibility(senderAction, -1, visible, -1, -1);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::onVisibility2DFillActionToggled(bool visible)
{
  QAction* senderAction = qobject_cast<QAction*>(sender());
  if (!senderAction)
    {
    return;
    }

  // Set 2D fill visibility to segment referenced by action toggled
  this->setSegmentVisibility(senderAction, -1, -1, visible, -1);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::onVisibility2DOutlineActionToggled(bool visible)
{
  QAction* senderAction = qobject_cast<QAction*>(sender());
  if (!senderAction)
    {
    return;
    }

  // Set 2D outline visibility to segment referenced by action toggled
  this->setSegmentVisibility(senderAction, -1, -1, -1, visible);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::setSegmentVisibility(QObject* senderObject, int visible, int visible3D, int visible2DFill, int visible2DOutline)
{
  Q_D(qMRMLSegmentsTableView);

  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: segmentation node is not set";
    return;
    }

  QString segmentId = senderObject->property(ID_PROPERTY).toString();
  this->setSegmentVisibility(segmentId, visible, visible3D, visible2DFill, visible2DOutline);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::setSegmentVisibility(QString segmentId, int visible, int visible3D, int visible2DFill, int visible2DOutline)
{
  Q_D(qMRMLSegmentsTableView);

  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: segmentation node is not set";
    return;
    }

  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(
    d->SegmentationNode->GetDisplayNode() );
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": No display node for segmentation!";
    return;
    }
  vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
  displayNode->GetSegmentDisplayProperties(segmentId.toStdString(), properties);

  // Change visibility to all modes
  bool valueChanged = false;
  if (visible == 0 || visible == 1)
    {
    properties.Visible = (bool)visible;

    // If overall visibility is explicitly set to true then enable all visibility options
    // to make sure that something is actually visible.
    if (properties.Visible && !properties.Visible3D && !properties.Visible2DFill && !properties.Visible2DOutline)
      {
      properties.Visible3D = true;
      properties.Visible2DFill = true;
      properties.Visible2DOutline = true;
      }

    valueChanged = true;
    }
  if (visible3D == 0 || visible3D == 1)
    {
    properties.Visible3D = (bool)visible3D;
    valueChanged = true;
    }
  if (visible2DFill == 0 || visible2DFill == 1)
    {
    properties.Visible2DFill = (bool)visible2DFill;
    valueChanged = true;
    }
  if (visible2DOutline == 0 || visible2DOutline == 1)
    {
    properties.Visible2DOutline = (bool)visible2DOutline;
    valueChanged = true;
    }

  // Set visibility to display node
  if (valueChanged)
    {
    displayNode->SetSegmentDisplayProperties(segmentId.toStdString(), properties);
    }
}

//-----------------------------------------------------------------------------
int qMRMLSegmentsTableView::segmentCount() const
{
  Q_D(const qMRMLSegmentsTableView);

  return d->Model->rowCount();
}

//-----------------------------------------------------------------------------
QStringList qMRMLSegmentsTableView::selectedSegmentIDs()
{
  Q_D(qMRMLSegmentsTableView);
  if (!d->SegmentsTable->selectionModel()->hasSelection())
    {
    return QStringList();
    }

  QModelIndexList selectedModelIndices = d->SegmentsTable->selectionModel()->selectedRows();
  QStringList selectedSegmentIds;
  foreach (QModelIndex selectedModelIndex, selectedModelIndices)
    {
    selectedSegmentIds << d->SortFilterModel->segmentIDFromIndex(selectedModelIndex);
    }

  return selectedSegmentIds;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::setSelectedSegmentIDs(QStringList segmentIDs)
{
  Q_D(qMRMLSegmentsTableView);

  if (!d->SegmentationNode && !segmentIDs.empty())
    {
    qCritical() << Q_FUNC_INFO << " failed: segmentation node is not set";
    return;
    }

  for (QString segmentID : segmentIDs)
    {
    QModelIndex index = d->SortFilterModel->indexFromSegmentID(segmentID);

    QItemSelectionModel::QItemSelectionModel::SelectionFlags flags = QFlags<QItemSelectionModel::SelectionFlag>();
    flags.setFlag(QItemSelectionModel::Select);
    flags.setFlag(QItemSelectionModel::Rows);
    d->SegmentsTable->selectionModel()->select(index, flags);
    }

  // Deselect items that don't have to be selected anymore
  for (int row = 0; row < d->SortFilterModel->rowCount(); ++row)
    {
    QModelIndex index = d->SortFilterModel->index(row, d->Model->nameColumn());
    QString segmentID = d->SortFilterModel->segmentIDFromIndex(index);
    if (segmentID.isEmpty())
      {
      // invalid item, canot determine selection state
      continue;
      }

    if (segmentIDs.contains(segmentID))
      {
      // selected
      continue;
      }

    QItemSelectionModel::QItemSelectionModel::SelectionFlags flags = QFlags<QItemSelectionModel::SelectionFlag>();
    flags.setFlag(QItemSelectionModel::Deselect);
    flags.setFlag(QItemSelectionModel::Rows);
    d->SegmentsTable->selectionModel()->select(index, flags);
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentsTableView::clearSelection()
{
  Q_D(qMRMLSegmentsTableView);
  d->SegmentsTable->clearSelection();
}

//------------------------------------------------------------------------------
bool qMRMLSegmentsTableView::eventFilter(QObject* target, QEvent* event)
{
  Q_D(qMRMLSegmentsTableView);
  if (target == d->SegmentsTable)
    {
    // Prevent giving the focus to the previous/next widget if arrow keys are used
    // at the edge of the table (without this: if the current cell is in the top
    // row and user press the Up key, the focus goes from the table to the previous
    // widget in the tab order)
    if (event->type() == QEvent::KeyPress)
      {
      QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);
      QAbstractItemModel* model = d->SegmentsTable->model();
      QModelIndex currentIndex = d->SegmentsTable->currentIndex();

      if (model && (
        (keyEvent->key() == Qt::Key_Left && currentIndex.column() == 0)
        || (keyEvent->key() == Qt::Key_Up && currentIndex.row() == 0)
        || (keyEvent->key() == Qt::Key_Right && currentIndex.column() == model->columnCount() - 1)
        || (keyEvent->key() == Qt::Key_Down && currentIndex.row() == model->rowCount() - 1)))
        {
        return true;
        }
      }
    }
  return this->QWidget::eventFilter(target, event);
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::endProcessing()
{
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::setSelectionMode(int mode)
{
  Q_D(qMRMLSegmentsTableView);
  d->SegmentsTable->setSelectionMode(static_cast<QAbstractItemView::SelectionMode>(mode));
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::setHeaderVisible(bool visible)
{
  Q_D(qMRMLSegmentsTableView);
  d->SegmentsTable->horizontalHeader()->setVisible(visible);
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::setVisibilityColumnVisible(bool visible)
{
  Q_D(qMRMLSegmentsTableView);
  d->SegmentsTable->setColumnHidden(d->Model->visibilityColumn(), !visible);
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::setColorColumnVisible(bool visible)
{
  Q_D(qMRMLSegmentsTableView);
  d->SegmentsTable->setColumnHidden(d->Model->colorColumn(), !visible);
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::setOpacityColumnVisible(bool visible)
{
  Q_D(qMRMLSegmentsTableView);
  d->SegmentsTable->setColumnHidden(d->Model->opacityColumn(), !visible);
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::setStatusColumnVisible(bool visible)
{
  Q_D(qMRMLSegmentsTableView);
  d->SegmentsTable->setColumnHidden(d->Model->statusColumn(), !visible);
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::setReadOnly(bool aReadOnly)
{
  Q_D(qMRMLSegmentsTableView);
  if (aReadOnly)
    {
    d->SegmentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
  else
    {
    d->SegmentsTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    }
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::setFilterBarVisible(bool visible)
{
  Q_D(qMRMLSegmentsTableView);
  d->FilterBar->setVisible(visible);
}

//------------------------------------------------------------------------------
int qMRMLSegmentsTableView::selectionMode()
{
  Q_D(qMRMLSegmentsTableView);
  return d->SegmentsTable->selectionMode();
}

//------------------------------------------------------------------------------
bool qMRMLSegmentsTableView::headerVisible()
{
  Q_D(qMRMLSegmentsTableView);
  return d->SegmentsTable->horizontalHeader()->isVisible();
}

//------------------------------------------------------------------------------
bool qMRMLSegmentsTableView::visibilityColumnVisible()
{
  Q_D(qMRMLSegmentsTableView);
  return !d->SegmentsTable->isColumnHidden(d->Model->visibilityColumn());
}

//------------------------------------------------------------------------------
bool qMRMLSegmentsTableView::colorColumnVisible()
{
  Q_D(qMRMLSegmentsTableView);
  return !d->SegmentsTable->isColumnHidden(d->Model->colorColumn());
}

//------------------------------------------------------------------------------
bool qMRMLSegmentsTableView::opacityColumnVisible()
{
  Q_D(qMRMLSegmentsTableView);
  return !d->SegmentsTable->isColumnHidden(d->Model->opacityColumn());
}

//------------------------------------------------------------------------------
bool qMRMLSegmentsTableView::statusColumnVisible()
{
  Q_D(qMRMLSegmentsTableView);
  return !d->SegmentsTable->isColumnHidden(d->Model->statusColumn());
}

//------------------------------------------------------------------------------
bool qMRMLSegmentsTableView::readOnly()
{
  Q_D(qMRMLSegmentsTableView);
  return (d->SegmentsTable->editTriggers() == QAbstractItemView::NoEditTriggers);
}

//------------------------------------------------------------------------------
bool qMRMLSegmentsTableView::filterBarVisible()
{
  Q_D(qMRMLSegmentsTableView);
  return d->FilterBar->isVisible();
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::contextMenuEvent(QContextMenuEvent* event)
{
  Q_D(qMRMLSegmentsTableView);

  QMenu* contextMenu = new QMenu(this);

  QStringList selectedSegmentIDs = this->selectedSegmentIDs();

  if (selectedSegmentIDs.size() > 0)
    {
    QAction* showOnlySelectedAction = new QAction("Show only selected segments", this);
    QObject::connect(showOnlySelectedAction, SIGNAL(triggered()), this, SLOT(showOnlySelectedSegments()));
    contextMenu->addAction(showOnlySelectedAction);

    contextMenu->addSeparator();

    QAction* jumpSlicesAction = new QAction("Jump slices", this);
    QObject::connect(jumpSlicesAction, SIGNAL(triggered()), this, SLOT(jumpSlices()));
    contextMenu->addAction(jumpSlicesAction);

    contextMenu->addSeparator();

    QAction* moveUpAction = new QAction("Move selected segments up", this);
    QObject::connect(moveUpAction, SIGNAL(triggered()), this, SLOT(moveSelectedSegmentsUp()));
    contextMenu->addAction(moveUpAction);

    QAction* moveDownAction = new QAction("Move selected segments down", this);
    QObject::connect(moveDownAction, SIGNAL(triggered()), this, SLOT(moveSelectedSegmentsDown()));
    contextMenu->addAction(moveDownAction);
    }

  contextMenu->addSeparator();

  QAction* showFilterAction = new QAction("Show filter bar", this);
  showFilterAction->setCheckable(true);
  showFilterAction->setChecked(d->FilterBar->isVisible());
  QObject::connect(showFilterAction, SIGNAL(triggered(bool)), this, SLOT(setFilterBarVisible(bool)));
  contextMenu->addAction(showFilterAction);

  QModelIndex index = d->SegmentsTable->indexAt(d->SegmentsTable->viewport()->mapFromGlobal(event->globalPos()));
  if (d->AdvancedSegmentVisibility && index.isValid())
    {
    QString segmentID = d->SortFilterModel->segmentIDFromIndex(index);
    vtkSegment* segment = d->SegmentationNode->GetSegmentation()->GetSegment(segmentID.toStdString());

    // Get segment display properties
    vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
    vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(d->SegmentationNode->GetDisplayNode());
    if (displayNode)
      {
      displayNode->GetSegmentDisplayProperties(segmentID.toLatin1().constData(), properties);
      }

    contextMenu->addSeparator();

    QAction* visibility3DAction = new QAction("Show in 3D", this);
    visibility3DAction->setCheckable(true);
    visibility3DAction->setChecked(properties.Visible3D);
    visibility3DAction->setProperty(ID_PROPERTY, segmentID);
    QObject::connect(visibility3DAction, SIGNAL(triggered(bool)), this, SLOT(onVisibility3DActionToggled(bool)));
    contextMenu->addAction(visibility3DAction);

    QAction* visibility2DFillAction = new QAction("Show in 2D as fill", this);
    visibility2DFillAction->setCheckable(true);
    visibility2DFillAction->setChecked(properties.Visible2DFill);
    visibility2DFillAction->setProperty(ID_PROPERTY, segmentID);
    connect(visibility2DFillAction, SIGNAL(triggered(bool)), this, SLOT(onVisibility2DFillActionToggled(bool)));
    contextMenu->addAction(visibility2DFillAction);

    QAction* visibility2DOutlineAction = new QAction("Show in 2D as outline", this);
    visibility2DOutlineAction->setCheckable(true);
    visibility2DOutlineAction->setChecked(properties.Visible2DOutline);
    visibility2DOutlineAction->setProperty(ID_PROPERTY, segmentID);
    connect(visibility2DOutlineAction, SIGNAL(triggered(bool)), this, SLOT(onVisibility2DOutlineActionToggled(bool)));
    contextMenu->addAction(visibility2DOutlineAction);
    }

  if (selectedSegmentIDs.size() > 0)
    {
    contextMenu->addSeparator();
    for (int i = 0; i < vtkSlicerSegmentationsModuleLogic::LastStatus; ++i)
      {
      QString name = vtkSlicerSegmentationsModuleLogic::GetSegmentStatusEnumAsString(i);
      QIcon icon;
      switch (i)
        {
        case vtkSlicerSegmentationsModuleLogic::NotStarted:
          icon = d->NotStartedIcon;
          break;
        case vtkSlicerSegmentationsModuleLogic::InProgress:
          icon = d->InProgressIcon;
          break;
        case vtkSlicerSegmentationsModuleLogic::Completed:
          icon = d->CompletedIcon;
          break;
        case vtkSlicerSegmentationsModuleLogic::Flagged:
          icon = d->FlaggedIcon;
          break;
        }

      QAction* setStatusAction = new QAction(name);
      setStatusAction->setIcon(icon);
      setStatusAction->setProperty(STATUS_PROPERTY, i);
      QObject::connect(setStatusAction, SIGNAL(triggered()), this, SLOT(setSelectedSegmentsStatus()));
      contextMenu->addAction(setStatusAction);
      }

    contextMenu->addSeparator();
    QAction* clearSelectedSegmentAction = new QAction("Clear selected segments", this);
    QObject::connect(clearSelectedSegmentAction, SIGNAL(triggered()), this, SLOT(clearSelectedSegments()));
    contextMenu->addAction(clearSelectedSegmentAction);
    }

  contextMenu->popup(event->globalPos());
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::setSelectedSegmentsStatus(int aStatus)
{
  Q_D(qMRMLSegmentsTableView);

  int status = aStatus;
  if (status == -1)
    {
    QAction* setStatusAction = qobject_cast<QAction*>(sender());
    Q_ASSERT(setStatusAction);
    if (!setStatusAction)
      {
      return;
      }
    status = setStatusAction->property(STATUS_PROPERTY).toInt();
    }

  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << "Invalid segmentation node";
    return;
    }
  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
  if (!segmentation)
    {
    qCritical() << Q_FUNC_INFO << "Invalid segmentation";
    return;
    }

  QStringList selectedSegmentIDs = this->selectedSegmentIDs();
  for (QString segmentID : selectedSegmentIDs)
    {
    vtkSegment* segment = segmentation->GetSegment(segmentID.toStdString());
    if (segment)
      {
      vtkSlicerSegmentationsModuleLogic::SetSegmentStatus(segment, status);
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::clearSelectedSegments()
{
  Q_D(qMRMLSegmentsTableView);

  QAction* clearSegmentAction = qobject_cast<QAction*>(sender());
  Q_ASSERT(clearSegmentAction);

  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << "Invalid segmentation node";
    return;
    }
  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
  if (!segmentation)
    {
    qCritical() << Q_FUNC_INFO << "Invalid segmentation";
    return;
    }

  QMessageBox messageBox;
  messageBox.setStandardButtons(QMessageBox::StandardButton::Discard | QMessageBox::StandardButton::Cancel);
  messageBox.setDefaultButton(QMessageBox::Cancel);
  messageBox.setText("Are you sure you want to discard the contents of the selected segments?");
  if (messageBox.exec() == QMessageBox::Cancel)
    {
    return;
    }

  QStringList selectedSegmentIDs = this->selectedSegmentIDs();
  for (QString segmentID : selectedSegmentIDs)
    {
    vtkSegment* segment = segmentation->GetSegment(segmentID.toStdString());
    if (!segment)
      {
      continue;
      }
    vtkDataObject* dataObject = segment->GetRepresentation(segmentation->GetMasterRepresentationName());
    dataObject->Initialize();
    dataObject->Modified();
    vtkSlicerSegmentationsModuleLogic::SetSegmentStatus(segment, vtkSlicerSegmentationsModuleLogic::NotStarted);
    segment->Modified();
    }
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::showOnlySelectedSegments()
{
  QStringList selectedSegmentIDs = this->selectedSegmentIDs();
  if (selectedSegmentIDs.size() == 0)
    {
    qWarning() << Q_FUNC_INFO << ": No segment selected";
    return;
    }

  Q_D(qMRMLSegmentsTableView);
  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": No current segmentation node";
    return;
    }
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(
    d->SegmentationNode->GetDisplayNode() );
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": No display node for segmentation " << d->SegmentationNode->GetName();
    return;
    }

  // Hide all segments except the selected ones
  MRMLNodeModifyBlocker displayNodeModify(displayNode);
  QStringList displayedSegmentIDs = this->displayedSegmentIDs();
  foreach (QString segmentId, displayedSegmentIDs)
    {
    bool visible = false;
    if (selectedSegmentIDs.contains(segmentId))
      {
      visible = true;
      }

    displayNode->SetSegmentVisibility(segmentId.toLatin1().constData(), visible);
    }
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::jumpSlices()
{
  QStringList selectedSegmentIDs = this->selectedSegmentIDs();
  if (selectedSegmentIDs.size() == 0)
    {
    qWarning() << Q_FUNC_INFO << ": No segment selected";
    return;
    }

  Q_D(qMRMLSegmentsTableView);
  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": No current segmentation node";
    return;
    }

  double* segmentCenterPosition = d->SegmentationNode->GetSegmentCenterRAS(selectedSegmentIDs[0].toLatin1().constData());
  if (!segmentCenterPosition)
    {
    return;
    }

  qSlicerLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
    {
    // application is closing
    return;
    }
  foreach(QString sliceViewName, layoutManager->sliceViewNames())
    {
    // Check if segmentation is visible in this view
    qMRMLSliceWidget* sliceWidget = layoutManager->sliceWidget(sliceViewName);
    vtkMRMLSliceNode* sliceNode = sliceWidget->mrmlSliceNode();
    if (!sliceNode || !sliceNode->GetID())
      {
      continue;
      }
    bool visibleInView = false;
    int numberOfDisplayNodes = d->SegmentationNode->GetNumberOfDisplayNodes();
    for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
      {
      vtkMRMLDisplayNode* segmentationDisplayNode = d->SegmentationNode->GetNthDisplayNode(displayNodeIndex);
      if (segmentationDisplayNode->IsDisplayableInView(sliceNode->GetID()))
        {
        visibleInView = true;
        break;
        }
      }
    if (!visibleInView)
      {
      continue;
      }
    sliceNode->JumpSliceByCentering(segmentCenterPosition[0], segmentCenterPosition[1], segmentCenterPosition[2]);
    }
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::moveSelectedSegmentsUp()
{
  QStringList selectedSegmentIDs = this->selectedSegmentIDs();
  if (selectedSegmentIDs.size() == 0)
    {
    qWarning() << Q_FUNC_INFO << ": No segment selected";
    return;
    }

  Q_D(qMRMLSegmentsTableView);
  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": No current segmentation node";
    return;
    }
  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();

  QList<int> segmentIndices;
  foreach (QString segmentId, selectedSegmentIDs)
    {
    segmentIndices << segmentation->GetSegmentIndex(segmentId.toLatin1().constData());
    }
  int minIndex = *(std::min_element(segmentIndices.begin(), segmentIndices.end()));
  if (minIndex == 0)
    {
    qDebug() << Q_FUNC_INFO << ": Cannot move top segment up";
    return;
    }
  for (int i=0; i<selectedSegmentIDs.count(); ++i)
    {
    segmentation->SetSegmentIndex(selectedSegmentIDs[i].toLatin1().constData(), segmentIndices[i]-1);
    }
}

//------------------------------------------------------------------------------
void qMRMLSegmentsTableView::moveSelectedSegmentsDown()
{
  QStringList selectedSegmentIDs = this->selectedSegmentIDs();
  if (selectedSegmentIDs.size() == 0)
    {
    qWarning() << Q_FUNC_INFO << ": No segment selected";
    return;
    }

  Q_D(qMRMLSegmentsTableView);
  if (!d->SegmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": No current segmentation node";
    return;
    }
  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();

  QList<int> segmentIndices;
  foreach (QString segmentId, selectedSegmentIDs)
    {
    segmentIndices << segmentation->GetSegmentIndex(segmentId.toLatin1().constData());
    }
  int maxIndex = *(std::max_element(segmentIndices.begin(), segmentIndices.end()));
  if (maxIndex == segmentation->GetNumberOfSegments()-1)
    {
    qDebug() << Q_FUNC_INFO << ": Cannot move bottom segment down";
    return;
    }
  for (int i=selectedSegmentIDs.count()-1; i>=0; --i)
    {
    segmentation->SetSegmentIndex(selectedSegmentIDs[i].toLatin1().constData(), segmentIndices[i]+1);
    }
}

// --------------------------------------------------------------------------
QString qMRMLSegmentsTableView::terminologyTooltipForSegment(vtkSegment* segment)
{
  return qMRMLSegmentsModel::terminologyTooltipForSegment(segment);
}

// --------------------------------------------------------------------------
void qMRMLSegmentsTableView::setHideSegments(const QStringList& segmentIDs)
{
  Q_D(qMRMLSegmentsTableView);
  d->SortFilterModel->setHideSegments(segmentIDs);
}

// --------------------------------------------------------------------------
QStringList qMRMLSegmentsTableView::hideSegments()const
{
  Q_D(const qMRMLSegmentsTableView);
  return d->SortFilterModel->hideSegments();
}

// --------------------------------------------------------------------------
QStringList qMRMLSegmentsTableView::displayedSegmentIDs()const
{
  Q_D(const qMRMLSegmentsTableView);

  QStringList displayedSegmentIDs;
  for (int row = 0; row < d->SortFilterModel->rowCount(); ++row)
    {
    displayedSegmentIDs << d->SortFilterModel->segmentIDFromIndex(d->SortFilterModel->index(row, 0));
    }
  return displayedSegmentIDs;
}

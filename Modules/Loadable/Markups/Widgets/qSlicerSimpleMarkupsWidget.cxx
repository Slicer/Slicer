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

  This file was originally developed by Matthew Holden, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Markups Widgets includes
#include "qSlicerSimpleMarkupsWidget.h"
#include "ui_qSlicerSimpleMarkupsWidget.h"

// Markups includes
#include <vtkSlicerMarkupsLogic.h>

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractCoreModule.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLMarkupsNode.h>

// Qt includes
#include <QAction>
#include <QDebug>
#include <QMenu>
#include <QTableWidgetItem>

int CONTROL_POINT_LABEL_COLUMN = 0;
int CONTROL_POINT_X_COLUMN = 1;
int CONTROL_POINT_Y_COLUMN = 2;
int CONTROL_POINT_Z_COLUMN = 3;
int CONTROL_POINT_STATE_COLUMN = 4;
int CONTROL_POINT_COLUMNS = 5;

//-----------------------------------------------------------------------------
class qSlicerSimpleMarkupsWidgetPrivate : public Ui_qSlicerSimpleMarkupsWidget
{
  Q_DECLARE_PUBLIC(qSlicerSimpleMarkupsWidget);

protected:
  qSlicerSimpleMarkupsWidget* const q_ptr;

public:
  qSlicerSimpleMarkupsWidgetPrivate(qSlicerSimpleMarkupsWidget& object);
  ~qSlicerSimpleMarkupsWidgetPrivate();
  virtual void setupUi(qSlicerSimpleMarkupsWidget*);

  void setupTableHeader();
  void updateStateItem(QTableWidgetItem* stateItem, int positionStatus);

public:
  vtkWeakPointer<vtkSlicerMarkupsLogic> MarkupsLogic;
  bool EnterPlaceModeOnNodeChange;
  bool JumpToSliceEnabled;
  bool PositionStatusColumnVisible;
  int ViewGroup;

  vtkWeakPointer<vtkMRMLMarkupsNode> CurrentMarkupsNode;

  // Cached pixmaps for position status icons
  QPixmap MarkupsDefinedIcon;
  QPixmap MarkupsInProgressIcon;
  QPixmap MarkupsMissingIcon;
  QPixmap MarkupsUndefinedIcon;
};

// --------------------------------------------------------------------------
qSlicerSimpleMarkupsWidgetPrivate::qSlicerSimpleMarkupsWidgetPrivate(qSlicerSimpleMarkupsWidget& object)
  : q_ptr(&object)
  , EnterPlaceModeOnNodeChange(true)
  , JumpToSliceEnabled(false)
  , PositionStatusColumnVisible(false)
  , ViewGroup(-1)
  , MarkupsDefinedIcon(":/Icons/XSmall/MarkupsDefined.png")
  , MarkupsInProgressIcon(":/Icons/XSmall/MarkupsInProgress.png")
  , MarkupsMissingIcon(":/Icons/XSmall/MarkupsMissing.png")
  , MarkupsUndefinedIcon(":/Icons/XSmall/MarkupsUndefined.png")
{
}

//-----------------------------------------------------------------------------
qSlicerSimpleMarkupsWidgetPrivate::~qSlicerSimpleMarkupsWidgetPrivate() = default;

// --------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidgetPrivate::setupUi(qSlicerSimpleMarkupsWidget* widget)
{
  this->Ui_qSlicerSimpleMarkupsWidget::setupUi(widget);
}

// --------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidgetPrivate::setupTableHeader()
{
  Q_Q(qSlicerSimpleMarkupsWidget);

  this->MarkupsControlPointsTableWidget->setHorizontalHeaderLabels(QStringList() << q->tr("Label") << q->tr("R") //: right
                                                                                 << q->tr("A")                   //: anterior
                                                                                 << q->tr("S")                   //: superior
                                                                                 << q->tr("")                    //: position status (icon only)
  );
  this->MarkupsControlPointsTableWidget->horizontalHeader()->setSectionResizeMode(CONTROL_POINT_LABEL_COLUMN, QHeaderView::Stretch);
  this->MarkupsControlPointsTableWidget->horizontalHeader()->setSectionResizeMode(CONTROL_POINT_X_COLUMN, QHeaderView::ResizeToContents);
  this->MarkupsControlPointsTableWidget->horizontalHeader()->setSectionResizeMode(CONTROL_POINT_Y_COLUMN, QHeaderView::ResizeToContents);
  this->MarkupsControlPointsTableWidget->horizontalHeader()->setSectionResizeMode(CONTROL_POINT_Z_COLUMN, QHeaderView::ResizeToContents);
  this->MarkupsControlPointsTableWidget->horizontalHeader()->setSectionResizeMode(CONTROL_POINT_STATE_COLUMN, QHeaderView::ResizeToContents);

  // Set the position status column header icon
  QTableWidgetItem* positionHeader = this->MarkupsControlPointsTableWidget->horizontalHeaderItem(CONTROL_POINT_STATE_COLUMN);
  positionHeader->setIcon(QIcon(":/Icons/Large/MarkupsPositionStatus.png"));
  positionHeader->setToolTip(q->tr("Click to cycle through position states:\n"
                                   "Defined -> Undefined (by clearing position) -> Preview (by entering into place mode) "
                                   "-> Missing (by marking as skip from placement) -> Defined (by restoring last position)"));

  // Hide position status column by default
  this->MarkupsControlPointsTableWidget->setColumnHidden(CONTROL_POINT_STATE_COLUMN, !this->PositionStatusColumnVisible);
}

// --------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidgetPrivate::updateStateItem(QTableWidgetItem* stateItem, int positionStatus)
{
  stateItem->setData(Qt::UserRole, positionStatus);
  switch (positionStatus)
  {
    case vtkMRMLMarkupsNode::PositionDefined: stateItem->setData(Qt::DecorationRole, this->MarkupsDefinedIcon); break;
    case vtkMRMLMarkupsNode::PositionPreview: stateItem->setData(Qt::DecorationRole, this->MarkupsInProgressIcon); break;
    case vtkMRMLMarkupsNode::PositionMissing: stateItem->setData(Qt::DecorationRole, this->MarkupsMissingIcon); break;
    case vtkMRMLMarkupsNode::PositionUndefined: stateItem->setData(Qt::DecorationRole, this->MarkupsUndefinedIcon); break;
    default:
      qWarning() << Q_FUNC_INFO << ": Unknown position status:" << positionStatus;
      stateItem->setData(Qt::DecorationRole, QVariant());
      break;
  }
}

//-----------------------------------------------------------------------------
// qSlicerSimpleMarkupsWidget methods

//-----------------------------------------------------------------------------
qSlicerSimpleMarkupsWidget::qSlicerSimpleMarkupsWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerSimpleMarkupsWidgetPrivate(*this))
{
  this->setup();
}

//-----------------------------------------------------------------------------
qSlicerSimpleMarkupsWidget::~qSlicerSimpleMarkupsWidget()
{
  this->setCurrentNode(nullptr);
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::setup()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  d->MarkupsLogic = vtkSlicerMarkupsLogic::SafeDownCast(this->moduleLogic(/*no tr*/ "Markups"));
  if (!d->MarkupsLogic)
  {
    qCritical() << Q_FUNC_INFO << ": Markups module is not found, some markup manipulation features will not be available";
  }

  d->setupUi(this);

  connect(d->MarkupsNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onMarkupsNodeChanged()));
  connect(d->MarkupsNodeComboBox, SIGNAL(nodeAddedByUser(vtkMRMLNode*)), this, SLOT(onMarkupsNodeAdded(vtkMRMLNode*)));
  connect(d->MarkupsPlaceWidget, SIGNAL(activeMarkupsPlaceModeChanged(bool)), this, SIGNAL(activeMarkupsPlaceModeChanged(bool)));

  d->MarkupsControlPointsTableWidget->setColumnCount(CONTROL_POINT_COLUMNS);
  d->setupTableHeader();

  // Reduce row height to minimum necessary
  d->MarkupsControlPointsTableWidget->setWordWrap(true);
  d->MarkupsControlPointsTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  d->MarkupsControlPointsTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  d->MarkupsControlPointsTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows); // only select rows rather than cells

  connect(d->MarkupsControlPointsTableWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onMarkupsControlPointsTableContextMenu(const QPoint&)));
  connect(d->MarkupsControlPointsTableWidget, SIGNAL(cellChanged(int, int)), this, SLOT(onMarkupsControlPointEdited(int, int)));
  // listen for click on a markup
  connect(d->MarkupsControlPointsTableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(onMarkupsControlPointSelected(int, int)));
  // Add handler for cell clicks to toggle position state
  connect(d->MarkupsControlPointsTableWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onMarkupsControlPointClicked(QTableWidgetItem*)));
  // listen for the current cell selection change (happens when arrows are used to navigate)
  connect(d->MarkupsControlPointsTableWidget, SIGNAL(currentCellChanged(int, int, int, int)), this, SLOT(onMarkupsControlPointSelected(int, int)));
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qSlicerSimpleMarkupsWidget::currentNode() const
{
  Q_D(const qSlicerSimpleMarkupsWidget);
  return d->MarkupsNodeComboBox->currentNode();
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qSlicerSimpleMarkupsWidget::getCurrentNode()
{
  qWarning("qSlicerSimpleMarkupsWidget::getCurrentNode() method is deprecated. Use qSlicerSimpleMarkupsWidget::currentNode() method instead");
  return this->currentNode();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::setCurrentNode(vtkMRMLNode* currentNode)
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(currentNode);
  if (currentMarkupsNode == d->CurrentMarkupsNode)
  {
    // not changed
    return;
  }

  // Don't change the active markups if the current node is changed programmatically
  bool wasBlocked = d->MarkupsNodeComboBox->blockSignals(true);
  d->MarkupsNodeComboBox->setCurrentNode(currentMarkupsNode);
  d->MarkupsNodeComboBox->blockSignals(wasBlocked);

  d->MarkupsPlaceWidget->setCurrentNode(currentMarkupsNode);

  // Reconnect the appropriate nodes
  this->qvtkReconnect(d->CurrentMarkupsNode, currentMarkupsNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidget()));
  this->qvtkReconnect(d->CurrentMarkupsNode, currentMarkupsNode, vtkMRMLMarkupsNode::PointAddedEvent, this, SLOT(onPointAdded()));
  this->qvtkReconnect(d->CurrentMarkupsNode, currentMarkupsNode, vtkMRMLMarkupsNode::PointRemovedEvent, this, SLOT(updateWidget()));
  this->qvtkReconnect(d->CurrentMarkupsNode, currentMarkupsNode, vtkMRMLMarkupsNode::PointModifiedEvent, this, SLOT(updateWidget()));

  d->CurrentMarkupsNode = currentMarkupsNode;

  this->updateWidget();

  emit markupsNodeChanged();
  emit markupsFiducialNodeChanged();
}

//-----------------------------------------------------------------------------
vtkMRMLInteractionNode* qSlicerSimpleMarkupsWidget::interactionNode() const
{
  Q_D(const qSlicerSimpleMarkupsWidget);
  return d->MarkupsPlaceWidget->interactionNode();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::setInteractionNode(vtkMRMLInteractionNode* interactionNode)
{
  Q_D(qSlicerSimpleMarkupsWidget);
  d->MarkupsPlaceWidget->setInteractionNode(interactionNode);
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::setNodeBaseName(const QString& newNodeBaseName)
{
  Q_D(qSlicerSimpleMarkupsWidget);
  d->MarkupsNodeComboBox->setBaseName(newNodeBaseName);
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::setDefaultNodeColor(QColor color)
{
  Q_D(qSlicerSimpleMarkupsWidget);
  d->MarkupsPlaceWidget->setDefaultNodeColor(color);
}

//-----------------------------------------------------------------------------
QColor qSlicerSimpleMarkupsWidget::defaultNodeColor() const
{
  Q_D(const qSlicerSimpleMarkupsWidget);
  return d->MarkupsPlaceWidget->defaultNodeColor();
}

//-----------------------------------------------------------------------------
bool qSlicerSimpleMarkupsWidget::enterPlaceModeOnNodeChange() const
{
  Q_D(const qSlicerSimpleMarkupsWidget);
  return d->EnterPlaceModeOnNodeChange;
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::setEnterPlaceModeOnNodeChange(bool enterPlaceMode)
{
  Q_D(qSlicerSimpleMarkupsWidget);
  d->EnterPlaceModeOnNodeChange = enterPlaceMode;
}

//-----------------------------------------------------------------------------
bool qSlicerSimpleMarkupsWidget::jumpToSliceEnabled() const
{
  Q_D(const qSlicerSimpleMarkupsWidget);
  return d->JumpToSliceEnabled;
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::setJumpToSliceEnabled(bool enable)
{
  Q_D(qSlicerSimpleMarkupsWidget);
  d->JumpToSliceEnabled = enable;
}

//-----------------------------------------------------------------------------
bool qSlicerSimpleMarkupsWidget::nodeSelectorVisible() const
{
  Q_D(const qSlicerSimpleMarkupsWidget);
  return d->MarkupsNodeComboBox->isVisible();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::setNodeSelectorVisible(bool visible)
{
  Q_D(qSlicerSimpleMarkupsWidget);
  d->MarkupsNodeComboBox->setVisible(visible);
}

//-----------------------------------------------------------------------------
bool qSlicerSimpleMarkupsWidget::optionsVisible() const
{
  Q_D(const qSlicerSimpleMarkupsWidget);
  return d->MarkupsPlaceWidget->isVisible();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::setOptionsVisible(bool visible)
{
  Q_D(qSlicerSimpleMarkupsWidget);
  d->MarkupsPlaceWidget->setVisible(visible);
}

//-----------------------------------------------------------------------------
QTableWidget* qSlicerSimpleMarkupsWidget::tableWidget() const
{
  Q_D(const qSlicerSimpleMarkupsWidget);
  return d->MarkupsControlPointsTableWidget;
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::setNodeColor(QColor color)
{
  Q_D(qSlicerSimpleMarkupsWidget);
  d->MarkupsPlaceWidget->setNodeColor(color);
}

//-----------------------------------------------------------------------------
QColor qSlicerSimpleMarkupsWidget::nodeColor() const
{
  Q_D(const qSlicerSimpleMarkupsWidget);
  return d->MarkupsPlaceWidget->nodeColor();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::setViewGroup(int newViewGroup)
{
  Q_D(qSlicerSimpleMarkupsWidget);
  d->ViewGroup = newViewGroup;
}

//-----------------------------------------------------------------------------
int qSlicerSimpleMarkupsWidget::viewGroup() const
{
  Q_D(const qSlicerSimpleMarkupsWidget);
  return d->ViewGroup;
}

//-----------------------------------------------------------------------------
bool qSlicerSimpleMarkupsWidget::positionStatusColumnVisible() const
{
  Q_D(const qSlicerSimpleMarkupsWidget);
  return d->PositionStatusColumnVisible;
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::setPositionStatusColumnVisible(bool visible)
{
  Q_D(qSlicerSimpleMarkupsWidget);
  if (d->PositionStatusColumnVisible == visible)
  {
    return;
  }
  d->PositionStatusColumnVisible = visible;
  d->MarkupsControlPointsTableWidget->setColumnHidden(CONTROL_POINT_STATE_COLUMN, !visible);
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::highlightNthControlPoint(int n)
{
  Q_D(qSlicerSimpleMarkupsWidget);
  if (n >= 0 && n < d->MarkupsControlPointsTableWidget->rowCount())
  {
    d->MarkupsControlPointsTableWidget->selectRow(n);
    d->MarkupsControlPointsTableWidget->setCurrentCell(n, 0);
    d->MarkupsControlPointsTableWidget->scrollTo(d->MarkupsControlPointsTableWidget->currentIndex());
  }
  else
  {
    d->MarkupsControlPointsTableWidget->clearSelection();
  }
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::highlightNthFiducial(int n)
{
  this->highlightNthControlPoint(n);
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::activate()
{
  Q_D(qSlicerSimpleMarkupsWidget);
  d->MarkupsPlaceWidget->setCurrentNodeActive(true);
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::placeActive(bool place)
{
  Q_D(qSlicerSimpleMarkupsWidget);
  d->MarkupsPlaceWidget->setPlaceModeEnabled(place);
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onMarkupsNodeChanged()
{
  Q_D(qSlicerSimpleMarkupsWidget);
  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->MarkupsNodeComboBox->currentNode());
  this->setCurrentNode(currentMarkupsNode);

  if (d->EnterPlaceModeOnNodeChange)
  {
    d->MarkupsPlaceWidget->setPlaceModeEnabled(currentMarkupsNode != nullptr);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onMarkupsNodeAdded(vtkMRMLNode* newNode)
{
  Q_D(qSlicerSimpleMarkupsWidget);
  if (d->MarkupsLogic == nullptr)
  {
    qCritical("qSlicerSimpleMarkupsWidget::onMarkupsNodeAdded failed: Markups module logic is invalid");
    return;
  }

  vtkMRMLMarkupsNode* newMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(newNode);
  if (newMarkupsNode->GetDisplayNode() == nullptr)
  {
    // Make sure there is an associated display node
    d->MarkupsLogic->AddNewDisplayNodeForMarkupsNode(newMarkupsNode);
  }
  d->MarkupsNodeComboBox->setCurrentNode(newMarkupsNode);
  this->setNodeColor(defaultNodeColor());
  this->onMarkupsNodeChanged();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onMarkupsControlPointsTableContextMenu(const QPoint& position)
{
  Q_D(qSlicerSimpleMarkupsWidget);

  if (d->MarkupsLogic == nullptr)
  {
    qCritical("qSlicerSimpleMarkupsWidget::onMarkupsControlPointsTableContextMenu failed: Markups module logic is invalid");
    return;
  }

  QPoint globalPosition = d->MarkupsControlPointsTableWidget->viewport()->mapToGlobal(position);

  QMenu* controlPointsMenu = new QMenu(d->MarkupsControlPointsTableWidget);
  QAction* deleteAction = new QAction(tr("Delete highlighted control points"), controlPointsMenu);
  QAction* upAction = new QAction(tr("Move current control point up"), controlPointsMenu);
  QAction* downAction = new QAction(tr("Move current control point down"), controlPointsMenu);
  QAction* jumpAction = new QAction(tr("Jump slices to control point"), controlPointsMenu);

  controlPointsMenu->addAction(deleteAction);
  controlPointsMenu->addAction(upAction);
  controlPointsMenu->addAction(downAction);
  controlPointsMenu->addAction(jumpAction);

  QAction* selectedAction = controlPointsMenu->exec(globalPosition);

  int currentControlPoint = d->MarkupsControlPointsTableWidget->currentRow();
  vtkMRMLMarkupsNode* currentNode = vtkMRMLMarkupsNode::SafeDownCast(d->MarkupsNodeComboBox->currentNode());

  if (currentNode == nullptr)
  {
    return;
  }

  // Only do this for non-null node
  if (selectedAction == deleteAction)
  {
    QItemSelectionModel* selectionModel = d->MarkupsControlPointsTableWidget->selectionModel();
    std::vector<int> deleteControlPoints;
    // Need to find selected before removing because removing automatically refreshes the table
    for (int i = 0; i < d->MarkupsControlPointsTableWidget->rowCount(); i++)
    {
      if (selectionModel->rowIntersectsSelection(i, d->MarkupsControlPointsTableWidget->rootIndex()))
      {
        deleteControlPoints.push_back(i);
      }
    }
    // Do this in batch mode
    int wasModifying = currentNode->StartModify();
    // Traversing this way should be more efficient and correct
    for (int i = static_cast<int>(deleteControlPoints.size()) - 1; i >= 0; i--)
    {
      // remove the point at that row
      currentNode->RemoveNthControlPoint(deleteControlPoints.at(static_cast<size_t>(i)));
    }
    currentNode->EndModify(wasModifying);
  }

  if (selectedAction == upAction)
  {
    if (currentControlPoint > 0)
    {
      currentNode->SwapControlPoints(currentControlPoint, currentControlPoint - 1);
    }
  }

  if (selectedAction == downAction)
  {
    if (currentControlPoint < currentNode->GetNumberOfControlPoints() - 1)
    {
      currentNode->SwapControlPoints(currentControlPoint, currentControlPoint + 1);
    }
  }

  if (selectedAction == jumpAction)
  {
    d->MarkupsLogic->JumpSlicesToNthPointInMarkup(this->currentNode()->GetID(), currentControlPoint, true /* centered */, d->ViewGroup);
  }

  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onMarkupsControlPointSelected(int row, int column)
{
  Q_UNUSED(column);
  Q_D(qSlicerSimpleMarkupsWidget);

  if (d->JumpToSliceEnabled)
  {
    vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(this->currentNode());
    if (currentMarkupsNode == nullptr)
    {
      return;
    }

    if (d->MarkupsLogic == nullptr)
    {
      qCritical("qSlicerSimpleMarkupsWidget::onMarkupsControlPointSelected "
                "failed: Cannot jump, markups module logic is invalid");
      return;
    }
    d->MarkupsLogic->JumpSlicesToNthPointInMarkup(currentMarkupsNode->GetID(), row, true /* centered */, d->ViewGroup);
  }

  emit currentMarkupsControlPointSelectionChanged(row);
  emit currentMarkupsFiducialSelectionChanged(row);
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onMarkupsControlPointEdited(int row, int column)
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(this->currentNode());

  if (currentMarkupsNode == nullptr)
  {
    return;
  }

  // Find the control point's current properties
  double currentControlPointPosition[3] = { 0, 0, 0 };
  currentMarkupsNode->GetNthControlPointPosition(row, currentControlPointPosition);
  std::string currentControlPointLabel = currentMarkupsNode->GetNthControlPointLabel(row);

  // Find the entry that we changed
  QTableWidgetItem* qItem = d->MarkupsControlPointsTableWidget->item(row, column);
  QString qText = qItem->text();

  if (column == CONTROL_POINT_LABEL_COLUMN)
  {
    currentMarkupsNode->SetNthControlPointLabel(row, qText.toStdString());
  }

  // Check if the value can be converted to double is already performed implicitly
  double newControlPointPosition = qText.toDouble();

  // Change the position values
  if (column == CONTROL_POINT_X_COLUMN)
  {
    currentControlPointPosition[0] = newControlPointPosition;
  }
  if (column == CONTROL_POINT_Y_COLUMN)
  {
    currentControlPointPosition[1] = newControlPointPosition;
  }
  if (column == CONTROL_POINT_Z_COLUMN)
  {
    currentControlPointPosition[2] = newControlPointPosition;
  }

  currentMarkupsNode->SetNthControlPointPosition(row, currentControlPointPosition);

  this->updateWidget(); // This may not be necessary the widget is updated whenever a control point is changed
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onMarkupsControlPointClicked(QTableWidgetItem* item)
{
  Q_D(qSlicerSimpleMarkupsWidget);
  if (!item || item->column() != CONTROL_POINT_STATE_COLUMN)
  {
    return;
  }

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(this->currentNode());
  if (!currentMarkupsNode)
  {
    return;
  }

  int row = item->row();

  // Cycle through position states: Defined -> Undefined -> Preview -> Missing -> Defined
  int currentStatus = item->data(Qt::UserRole).toInt();

  if (currentStatus == vtkMRMLMarkupsNode::PositionDefined)
  {
    currentMarkupsNode->UnsetNthControlPointPosition(row);
  }
  else if (currentStatus == vtkMRMLMarkupsNode::PositionUndefined)
  {
    currentMarkupsNode->ResetNthControlPointPosition(row);
    d->MarkupsPlaceWidget->setPlaceModeEnabled(true);
  }
  else if (currentStatus == vtkMRMLMarkupsNode::PositionPreview)
  {
    currentMarkupsNode->SetNthControlPointPositionMissing(row);
  }
  else if (currentStatus == vtkMRMLMarkupsNode::PositionMissing)
  {
    currentMarkupsNode->RestoreNthControlPointPosition(row);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::updateWidget()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  if (d->MarkupsLogic == nullptr || this->mrmlScene() == nullptr)
  {
    qCritical("qSlicerSimpleMarkupsWidget::updateWidget failed: Markups module logic or scene is invalid");
  }

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->MarkupsNodeComboBox->currentNode());
  if (currentMarkupsNode == nullptr || d->MarkupsLogic == nullptr)
  {
    d->MarkupsControlPointsTableWidget->clear();
    d->MarkupsControlPointsTableWidget->setRowCount(0);
    d->MarkupsControlPointsTableWidget->setColumnCount(0);
    d->MarkupsPlaceWidget->setEnabled(false);
    emit updateFinished();
    return;
  }

  d->MarkupsPlaceWidget->setEnabled(true);

  // Update the control points table
  bool wasBlockedTableWidget = d->MarkupsControlPointsTableWidget->blockSignals(true);

  if (d->MarkupsControlPointsTableWidget->rowCount() == currentMarkupsNode->GetNumberOfControlPoints())
  {
    // don't recreate the table if the number of items is not changed to preserve selection state
    double controlPointPosition[3] = { 0, 0, 0 };
    std::string controlPointLabel;
    for (int i = 0; i < currentMarkupsNode->GetNumberOfControlPoints(); i++)
    {
      controlPointLabel = currentMarkupsNode->GetNthControlPointLabel(i);
      currentMarkupsNode->GetNthControlPointPosition(i, controlPointPosition);
      d->MarkupsControlPointsTableWidget->item(i, CONTROL_POINT_LABEL_COLUMN)->setText(QString::fromStdString(controlPointLabel));

      // Update position status
      int positionStatus = currentMarkupsNode->GetNthControlPointPositionStatus(i);

      // Show position values only if it is defined or under preview
      bool showCoordinates = (positionStatus == vtkMRMLMarkupsNode::PositionDefined || //
                              positionStatus == vtkMRMLMarkupsNode::PositionPreview);
      if (showCoordinates)
      {
        d->MarkupsControlPointsTableWidget->item(i, CONTROL_POINT_X_COLUMN)->setText(QString::number(controlPointPosition[0], 'f', 3));
        d->MarkupsControlPointsTableWidget->item(i, CONTROL_POINT_Y_COLUMN)->setText(QString::number(controlPointPosition[1], 'f', 3));
        d->MarkupsControlPointsTableWidget->item(i, CONTROL_POINT_Z_COLUMN)->setText(QString::number(controlPointPosition[2], 'f', 3));
      }
      else
      {
        d->MarkupsControlPointsTableWidget->item(i, CONTROL_POINT_X_COLUMN)->setText(QString());
        d->MarkupsControlPointsTableWidget->item(i, CONTROL_POINT_Y_COLUMN)->setText(QString());
        d->MarkupsControlPointsTableWidget->item(i, CONTROL_POINT_Z_COLUMN)->setText(QString());
      }

      QTableWidgetItem* stateItem = d->MarkupsControlPointsTableWidget->item(i, CONTROL_POINT_STATE_COLUMN);
      if (stateItem)
      {
        d->updateStateItem(stateItem, positionStatus);
      }
    }
  }
  else
  {
    d->MarkupsControlPointsTableWidget->clear();
    d->MarkupsControlPointsTableWidget->setRowCount(currentMarkupsNode->GetNumberOfControlPoints());
    d->MarkupsControlPointsTableWidget->setColumnCount(CONTROL_POINT_COLUMNS);
    d->setupTableHeader();

    double controlPointPosition[3] = { 0, 0, 0 };
    std::string controlPointLabel;
    for (int i = 0; i < currentMarkupsNode->GetNumberOfControlPoints(); i++)
    {
      controlPointLabel = currentMarkupsNode->GetNthControlPointLabel(i);
      currentMarkupsNode->GetNthControlPointPosition(i, controlPointPosition);

      QTableWidgetItem* labelItem = new QTableWidgetItem(QString::fromStdString(controlPointLabel));

      // Get position status to determine if we should show position values
      int positionStatus = currentMarkupsNode->GetNthControlPointPositionStatus(i);

      // Create position items - hide values if status is Missing or Undefined
      QTableWidgetItem* xItem;
      QTableWidgetItem* yItem;
      QTableWidgetItem* zItem;

      if (positionStatus == vtkMRMLMarkupsNode::PositionMissing || positionStatus == vtkMRMLMarkupsNode::PositionUndefined)
      {
        xItem = new QTableWidgetItem(QString());
        yItem = new QTableWidgetItem(QString());
        zItem = new QTableWidgetItem(QString());
      }
      else
      {
        xItem = new QTableWidgetItem(QString::number(controlPointPosition[0], 'f', 3));
        yItem = new QTableWidgetItem(QString::number(controlPointPosition[1], 'f', 3));
        zItem = new QTableWidgetItem(QString::number(controlPointPosition[2], 'f', 3));
      }

      d->MarkupsControlPointsTableWidget->setItem(i, CONTROL_POINT_LABEL_COLUMN, labelItem);
      d->MarkupsControlPointsTableWidget->setItem(i, CONTROL_POINT_X_COLUMN, xItem);
      d->MarkupsControlPointsTableWidget->setItem(i, CONTROL_POINT_Y_COLUMN, yItem);
      d->MarkupsControlPointsTableWidget->setItem(i, CONTROL_POINT_Z_COLUMN, zItem);

      // Add position status column
      QTableWidgetItem* stateItem = new QTableWidgetItem();
      stateItem->setFlags(stateItem->flags() & ~Qt::ItemIsEditable);
      d->updateStateItem(stateItem, positionStatus);
      d->MarkupsControlPointsTableWidget->setItem(i, CONTROL_POINT_STATE_COLUMN, stateItem);
    }
  }

  d->MarkupsControlPointsTableWidget->blockSignals(wasBlockedTableWidget);

  emit updateFinished();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onPointAdded()
{
  Q_D(qSlicerSimpleMarkupsWidget);
  this->updateWidget();
  d->MarkupsControlPointsTableWidget->scrollToBottom();
}

//------------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
  this->updateWidget();
}

//-----------------------------------------------------------------------------
qSlicerMarkupsPlaceWidget* qSlicerSimpleMarkupsWidget::markupsPlaceWidget() const
{
  Q_D(const qSlicerSimpleMarkupsWidget);
  return d->MarkupsPlaceWidget;
}

//-----------------------------------------------------------------------------
qMRMLNodeComboBox* qSlicerSimpleMarkupsWidget::markupsSelectorComboBox() const
{
  Q_D(const qSlicerSimpleMarkupsWidget);
  return d->MarkupsNodeComboBox;
}

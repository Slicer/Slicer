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

// Markups includes
#include <vtkSlicerMarkupsLogic.h>

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
int CONTROL_POINT_COLUMNS = 4;


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerSimpleMarkupsWidgetPrivate
  : public Ui_qSlicerSimpleMarkupsWidget
{
  Q_DECLARE_PUBLIC(qSlicerSimpleMarkupsWidget);
protected:
  qSlicerSimpleMarkupsWidget* const q_ptr;

public:
  qSlicerSimpleMarkupsWidgetPrivate( qSlicerSimpleMarkupsWidget& object);
  ~qSlicerSimpleMarkupsWidgetPrivate();
  virtual void setupUi(qSlicerSimpleMarkupsWidget*);

public:
  vtkWeakPointer<vtkSlicerMarkupsLogic> MarkupsLogic;
  bool EnterPlaceModeOnNodeChange;
  bool JumpToSliceEnabled;
  int ViewGroup;

  vtkWeakPointer<vtkMRMLMarkupsNode> CurrentMarkupsNode;
};

// --------------------------------------------------------------------------
qSlicerSimpleMarkupsWidgetPrivate::qSlicerSimpleMarkupsWidgetPrivate( qSlicerSimpleMarkupsWidget& object)
  : q_ptr(&object)
  , EnterPlaceModeOnNodeChange(true)
  , JumpToSliceEnabled(false)
  , ViewGroup(-1)
{
}

//-----------------------------------------------------------------------------
qSlicerSimpleMarkupsWidgetPrivate::~qSlicerSimpleMarkupsWidgetPrivate() = default;

// --------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidgetPrivate::setupUi(qSlicerSimpleMarkupsWidget* widget)
{
  this->Ui_qSlicerSimpleMarkupsWidget::setupUi(widget);
}


//-----------------------------------------------------------------------------
// qSlicerSimpleMarkupsWidget methods

//-----------------------------------------------------------------------------
qSlicerSimpleMarkupsWidget::qSlicerSimpleMarkupsWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerSimpleMarkupsWidgetPrivate(*this) )
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

  d->MarkupsLogic = vtkSlicerMarkupsLogic::SafeDownCast(this->moduleLogic("Markups"));
  if (!d->MarkupsLogic)
    {
    qCritical() << Q_FUNC_INFO << ": Markups module is not found, some markup manipulation features will not be available";
    }

  d->setupUi(this);

  connect( d->MarkupsNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onMarkupsNodeChanged() ) );
  connect( d->MarkupsNodeComboBox, SIGNAL( nodeAddedByUser( vtkMRMLNode* ) ), this, SLOT( onMarkupsNodeAdded( vtkMRMLNode* ) ) );
  connect( d->MarkupsPlaceWidget, SIGNAL( activeMarkupsPlaceModeChanged(bool) ), this, SIGNAL( activeMarkupsPlaceModeChanged(bool) ) );

  d->MarkupsControlPointsTableWidget->setColumnCount( CONTROL_POINT_COLUMNS );
  d->MarkupsControlPointsTableWidget->setHorizontalHeaderLabels( QStringList() << "Label" << "R" << "A" << "S" );
  d->MarkupsControlPointsTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  d->MarkupsControlPointsTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  d->MarkupsControlPointsTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  d->MarkupsControlPointsTableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

  // Reduce row height to minimum necessary
  d->MarkupsControlPointsTableWidget->setWordWrap(true);
  d->MarkupsControlPointsTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  d->MarkupsControlPointsTableWidget->setContextMenuPolicy( Qt::CustomContextMenu );
  d->MarkupsControlPointsTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows); // only select rows rather than cells

  connect(d->MarkupsControlPointsTableWidget, SIGNAL( customContextMenuRequested(const QPoint&) ),
    this, SLOT( onMarkupsControlPointsTableContextMenu(const QPoint&) ) );
  connect(d->MarkupsControlPointsTableWidget, SIGNAL( cellChanged( int, int ) ), this, SLOT( onMarkupsControlPointEdited( int, int ) ) );
  // listen for click on a markup
  connect(d->MarkupsControlPointsTableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(onMarkupsControlPointSelected(int,int)));
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

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( currentNode );
  if (currentMarkupsNode==d->CurrentMarkupsNode)
    {
    // not changed
    return;
    }

  // Don't change the active markups if the current node is changed programmatically
  bool wasBlocked = d->MarkupsNodeComboBox->blockSignals(true);
  d->MarkupsNodeComboBox->setCurrentNode( currentMarkupsNode );
  d->MarkupsNodeComboBox->blockSignals(wasBlocked);

  d->MarkupsPlaceWidget->setCurrentNode( currentMarkupsNode );

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
vtkMRMLInteractionNode* qSlicerSimpleMarkupsWidget::interactionNode()const
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
void qSlicerSimpleMarkupsWidget::highlightNthControlPoint(int n)
{
  Q_D(qSlicerSimpleMarkupsWidget);
  if ( n >= 0 && n < d->MarkupsControlPointsTableWidget->rowCount() )
    {
    d->MarkupsControlPointsTableWidget->selectRow(n);
    d->MarkupsControlPointsTableWidget->setCurrentCell(n,0);
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
  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsNodeComboBox->currentNode() );
  this->setCurrentNode(currentMarkupsNode);

  if (d->EnterPlaceModeOnNodeChange)
    {
    d->MarkupsPlaceWidget->setPlaceModeEnabled(currentMarkupsNode!=nullptr);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onMarkupsNodeAdded( vtkMRMLNode* newNode )
{
  Q_D(qSlicerSimpleMarkupsWidget);
  if (d->MarkupsLogic == nullptr)
    {
    qCritical("qSlicerSimpleMarkupsWidget::onMarkupsNodeAdded failed: Markups module logic is invalid");
    return;
    }

  vtkMRMLMarkupsNode* newMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( newNode );
  if (newMarkupsNode->GetDisplayNode()==nullptr)
    {
    // Make sure there is an associated display node
    d->MarkupsLogic->AddNewDisplayNodeForMarkupsNode( newMarkupsNode );
    }
  d->MarkupsNodeComboBox->setCurrentNode( newMarkupsNode );
  this->setNodeColor( defaultNodeColor() );
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

  QPoint globalPosition = d->MarkupsControlPointsTableWidget->viewport()->mapToGlobal( position );

  QMenu* controlPointsMenu = new QMenu( d->MarkupsControlPointsTableWidget );
  QAction* deleteAction = new QAction( "Delete highlighted control points", controlPointsMenu );
  QAction* upAction = new QAction( "Move current control point up", controlPointsMenu );
  QAction* downAction = new QAction( "Move current control point down", controlPointsMenu );
  QAction* jumpAction = new QAction( "Jump slices to control point", controlPointsMenu );

  controlPointsMenu->addAction( deleteAction );
  controlPointsMenu->addAction( upAction );
  controlPointsMenu->addAction( downAction );
  controlPointsMenu->addAction( jumpAction );

  QAction* selectedAction = controlPointsMenu->exec( globalPosition );

  int currentControlPoint = d->MarkupsControlPointsTableWidget->currentRow();
  vtkMRMLMarkupsNode* currentNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsNodeComboBox->currentNode() );

  if ( currentNode == nullptr )
    {
    return;
    }

  // Only do this for non-null node
  if ( selectedAction == deleteAction )
    {
    QItemSelectionModel* selectionModel = d->MarkupsControlPointsTableWidget->selectionModel();
    std::vector< int > deleteControlPoints;
    // Need to find selected before removing because removing automatically refreshes the table
    for ( int i = 0; i < d->MarkupsControlPointsTableWidget->rowCount(); i++ )
      {
      if ( selectionModel->rowIntersectsSelection( i, d->MarkupsControlPointsTableWidget->rootIndex() ) )
        {
        deleteControlPoints.push_back( i );
        }
      }
    // Do this in batch mode
    int wasModifying = currentNode->StartModify();
    //Traversing this way should be more efficient and correct
    for ( int i = static_cast<int>(deleteControlPoints.size()) - 1; i >= 0; i-- )
      {
      // remove the point at that row
      currentNode->RemoveNthControlPoint(deleteControlPoints.at( static_cast<size_t>(i) ));
      }
    currentNode->EndModify(wasModifying);
    }


  if ( selectedAction == upAction )
    {
    if ( currentControlPoint > 0 )
      {
      currentNode->SwapControlPoints(currentControlPoint, currentControlPoint - 1 );
      }
    }

  if ( selectedAction == downAction )
    {
    if ( currentControlPoint < currentNode->GetNumberOfControlPoints() - 1 )
      {
      currentNode->SwapControlPoints( currentControlPoint, currentControlPoint + 1 );
      }
    }

  if ( selectedAction == jumpAction )
    {
    d->MarkupsLogic->JumpSlicesToNthPointInMarkup(this->currentNode()->GetID(), currentControlPoint, true /* centered */, d->ViewGroup);
    }

  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onMarkupsControlPointSelected(int row, int column)
{
  Q_UNUSED(column)
  Q_D(qSlicerSimpleMarkupsWidget);

  if (d->JumpToSliceEnabled)
    {
    vtkMRMLMarkupsNode *currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(this->currentNode());
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

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( this->currentNode() );

  if ( currentMarkupsNode == nullptr )
    {
    return;
    }

  // Find the control point's current properties
  double currentControlPointPosition[3] = { 0, 0, 0 };
  currentMarkupsNode->GetNthControlPointPosition( row, currentControlPointPosition );
  std::string currentControlPointLabel = currentMarkupsNode->GetNthControlPointLabel( row );

  // Find the entry that we changed
  QTableWidgetItem* qItem = d->MarkupsControlPointsTableWidget->item( row, column );
  QString qText = qItem->text();

  if ( column == CONTROL_POINT_LABEL_COLUMN )
    {
    currentMarkupsNode->SetNthControlPointLabel( row, qText.toStdString() );
    }

  // Check if the value can be converted to double is already performed implicitly
  double newControlPointPosition = qText.toDouble();

  // Change the position values
  if ( column == CONTROL_POINT_X_COLUMN )
    {
    currentControlPointPosition[ 0 ] = newControlPointPosition;
    }
  if ( column == CONTROL_POINT_Y_COLUMN )
    {
    currentControlPointPosition[ 1 ] = newControlPointPosition;
    }
  if ( column == CONTROL_POINT_Z_COLUMN )
    {
    currentControlPointPosition[ 2 ] = newControlPointPosition;
    }

  currentMarkupsNode->SetNthControlPointPosition( row, currentControlPointPosition );

  this->updateWidget(); // This may not be necessary the widget is updated whenever a control point is changed
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::updateWidget()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  if (d->MarkupsLogic == nullptr || this->mrmlScene() == nullptr)
    {
    qCritical("qSlicerSimpleMarkupsWidget::updateWidget failed: Markups module logic or scene is invalid");
    }

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsNodeComboBox->currentNode() );
  if ( currentMarkupsNode == nullptr || d->MarkupsLogic == nullptr)
    {
    d->MarkupsControlPointsTableWidget->clear();
    d->MarkupsControlPointsTableWidget->setRowCount( 0 );
    d->MarkupsControlPointsTableWidget->setColumnCount( 0 );
    d->MarkupsPlaceWidget->setEnabled(false);
    emit updateFinished();
    return;
    }

  d->MarkupsPlaceWidget->setEnabled(true);

  // Update the control points table
  bool wasBlockedTableWidget = d->MarkupsControlPointsTableWidget->blockSignals( true );

  if (d->MarkupsControlPointsTableWidget->rowCount()==currentMarkupsNode->GetNumberOfControlPoints())
    {
    // don't recreate the table if the number of items is not changed to preserve selection state
    double controlPointPosition[ 3 ] = { 0, 0, 0 };
    std::string controlPointLabel;
    for ( int i = 0; i < currentMarkupsNode->GetNumberOfControlPoints(); i++ )
      {
      controlPointLabel = currentMarkupsNode->GetNthControlPointLabel(i);
      currentMarkupsNode->GetNthControlPointPosition(i, controlPointPosition);
      d->MarkupsControlPointsTableWidget->item(i, CONTROL_POINT_LABEL_COLUMN)->setText(QString::fromStdString(controlPointLabel));
      d->MarkupsControlPointsTableWidget->item(i, CONTROL_POINT_X_COLUMN)->setText(QString::number( controlPointPosition[0], 'f', 3 ));
      d->MarkupsControlPointsTableWidget->item(i, CONTROL_POINT_Y_COLUMN)->setText(QString::number( controlPointPosition[1], 'f', 3 ));
      d->MarkupsControlPointsTableWidget->item(i, CONTROL_POINT_Z_COLUMN)->setText(QString::number( controlPointPosition[2], 'f', 3 ));
      }
    }
  else
    {
    d->MarkupsControlPointsTableWidget->clear();
    d->MarkupsControlPointsTableWidget->setRowCount( currentMarkupsNode->GetNumberOfControlPoints() );
    d->MarkupsControlPointsTableWidget->setColumnCount( CONTROL_POINT_COLUMNS );
    d->MarkupsControlPointsTableWidget->setHorizontalHeaderLabels( QStringList() << "Label" << "R" << "A" << "S" );
    d->MarkupsControlPointsTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    d->MarkupsControlPointsTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    d->MarkupsControlPointsTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    d->MarkupsControlPointsTableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    double controlPointPosition[ 3 ] = { 0, 0, 0 };
    std::string controlPointLabel;
    for ( int i = 0; i < currentMarkupsNode->GetNumberOfControlPoints(); i++ )
      {
      controlPointLabel = currentMarkupsNode->GetNthControlPointLabel( i );
      currentMarkupsNode->GetNthControlPointPosition( i, controlPointPosition );

      QTableWidgetItem* labelItem = new QTableWidgetItem( QString::fromStdString(controlPointLabel) );
      QTableWidgetItem* xItem = new QTableWidgetItem( QString::number( controlPointPosition[0], 'f', 3 ) );
      QTableWidgetItem* yItem = new QTableWidgetItem( QString::number( controlPointPosition[1], 'f', 3 ) );
      QTableWidgetItem* zItem = new QTableWidgetItem( QString::number( controlPointPosition[2], 'f', 3 ) );

      d->MarkupsControlPointsTableWidget->setItem( i, CONTROL_POINT_LABEL_COLUMN, labelItem );
      d->MarkupsControlPointsTableWidget->setItem( i, CONTROL_POINT_X_COLUMN, xItem );
      d->MarkupsControlPointsTableWidget->setItem( i, CONTROL_POINT_Y_COLUMN, yItem );
      d->MarkupsControlPointsTableWidget->setItem( i, CONTROL_POINT_Z_COLUMN, zItem );
      }
    }

  d->MarkupsControlPointsTableWidget->blockSignals( wasBlockedTableWidget );

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

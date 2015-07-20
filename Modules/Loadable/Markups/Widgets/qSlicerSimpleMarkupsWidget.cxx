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

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractCoreModule.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>

// Qt includes
#include <QtGui>

int FIDUCIAL_LABEL_COLUMN = 0;
int FIDUCIAL_X_COLUMN = 1;
int FIDUCIAL_Y_COLUMN = 2;
int FIDUCIAL_Z_COLUMN = 3;
int FIDUCIAL_COLUMNS = 4;


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
  QColor DefaultNodeColor;
  vtkSlicerMarkupsLogic* MarkupsLogic;
};

// --------------------------------------------------------------------------
qSlicerSimpleMarkupsWidgetPrivate::qSlicerSimpleMarkupsWidgetPrivate( qSlicerSimpleMarkupsWidget& object) : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerSimpleMarkupsWidgetPrivate::~qSlicerSimpleMarkupsWidgetPrivate()
{
}

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
  Q_D(qSlicerSimpleMarkupsWidget);

  this->setup();
}

//-----------------------------------------------------------------------------
qSlicerSimpleMarkupsWidget::~qSlicerSimpleMarkupsWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::setup()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  d->setupUi(this);

  d->DefaultNodeColor.setRgb(0.0,0.0,0.0);

  connect( d->MarkupsFiducialNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onMarkupsFiducialNodeChanged() ) );
  connect( d->MarkupsFiducialNodeComboBox, SIGNAL( nodeAddedByUser( vtkMRMLNode* ) ), this, SLOT( onMarkupsFiducialNodeAdded( vtkMRMLNode* ) ) );

  // Use the pressed signal (otherwise we can unpress buttons without clicking them)
  connect( d->ColorButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( onColorButtonChanged( QColor ) ) );

  connect( d->VisibilityButton, SIGNAL( clicked() ), this, SLOT( onVisibilityButtonClicked() ) );
  d->VisibilityButton->setIcon( QIcon( ":/Icons/Small/SlicerVisible.png" ) );
  connect( d->LockButton, SIGNAL( clicked() ), this, SLOT( onLockButtonClicked() ) );
  d->LockButton->setIcon( QIcon( ":/Icons/Small/SlicerUnlock.png" ) );
  connect( d->DeleteButton, SIGNAL( clicked() ), this, SLOT( onDeleteButtonClicked() ) );
  d->DeleteButton->setIcon( QIcon( ":/Icons/MarkupsDelete.png" ) );

  connect( d->ActiveButton, SIGNAL( toggled( bool ) ), this, SLOT( onActiveButtonClicked() ) );
  d->ActiveButton->setIcon( QApplication::style()->standardIcon( QStyle::SP_DialogApplyButton ) );

  connect( d->PlaceButton, SIGNAL( toggled( bool ) ), this, SLOT( onPlaceButtonClicked() ) ); 
  d->PlaceButton->setIcon( QIcon( ":/Icons/MarkupsMouseModePlace.png" ) );
  
  d->ExtraButton->setIcon( QIcon( ":/Icons/Ellipsis.png" ) );

  d->MarkupsFiducialTableWidget->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( d->MarkupsFiducialTableWidget, SIGNAL( customContextMenuRequested(const QPoint&) ), this, SLOT( onMarkupsFiducialTableContextMenu(const QPoint&) ) );
  connect( d->MarkupsFiducialTableWidget, SIGNAL( cellChanged( int, int ) ), this, SLOT( onMarkupsFiducialEdited( int, int ) ) );

}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::enter()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  // This cannot be called by the constructor, because Slicer may not exist when the constructor is called
  qSlicerAbstractCoreModule* MarkupsModule = qSlicerApplication::application()->moduleManager()->module( "Markups" );
  if ( MarkupsModule != NULL )
    {
    d->MarkupsLogic = vtkSlicerMarkupsLogic::SafeDownCast( MarkupsModule->logic() );
    }
  else
    {
    d->MarkupsLogic = NULL;
    }

  this->setMRMLScene( d->MarkupsLogic->GetMRMLScene() );

  // Connect to the selection singleton node - that way we can update the GUI if the Active node changes
  // Note that only the GUI cares about the active node (the logic and mrml don't)
  this->connectInteractionAndSelectionNodes();
  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::connectInteractionAndSelectionNodes()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLSelectionNode* selectionNode = vtkMRMLSelectionNode::SafeDownCast( this->mrmlScene()->GetNodeByID( d->MarkupsLogic->GetSelectionNodeID() ) );
  if ( selectionNode != NULL )
    {
    this->qvtkConnect( selectionNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
    }

  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast( this->mrmlScene()->GetNodeByID( "vtkMRMLInteractionNodeSingleton" ) );
  if ( interactionNode != NULL )
    {
    this->qvtkConnect( interactionNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
    }
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qSlicerSimpleMarkupsWidget::getCurrentNode()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  return d->MarkupsFiducialNodeComboBox->currentNode();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::setCurrentNode(vtkMRMLNode* currentNode)
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( currentNode );

  // Don't change the active fiducial list if the current node is changed programmatically
  disconnect( d->MarkupsFiducialNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onMarkupsFiducialNodeChanged() ) );
  d->MarkupsFiducialNodeComboBox->setCurrentNode( currentMarkupsNode );
  connect( d->MarkupsFiducialNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onMarkupsFiducialNodeChanged() ) );

  // Reconnect the appropriate nodes
  this->qvtkDisconnectAll();
  this->connectInteractionAndSelectionNodes();
  this->qvtkConnect( currentMarkupsNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  this->qvtkConnect( currentMarkupsNode, vtkMRMLMarkupsNode::MarkupAddedEvent, d->MarkupsFiducialTableWidget, SLOT( scrollToBottom() ) );

  this->updateWidget(); // Must call this to update widget even if the node hasn't changed - this will cause the active button and table to update
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::setNodeBaseName(QString newNodeBaseName)
{
  Q_D(qSlicerSimpleMarkupsWidget);

  d->MarkupsFiducialNodeComboBox->setBaseName(newNodeBaseName);
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::setDefaultNodeColor(QColor color)
{
  Q_D(qSlicerSimpleMarkupsWidget);

  d->DefaultNodeColor = color;
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::setNodeColor(QColor color)
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );
  if ( currentMarkupsNode == NULL )
    {
    return;
    }
  
  vtkMRMLDisplayNode* currentMarkupsDisplayNode = currentMarkupsNode->GetDisplayNode();
  if ( currentMarkupsDisplayNode == NULL )
    {
    return;
    }

  double rgbDoubleVector[3] = {color.redF(),color.greenF(),color.blueF()};
  currentMarkupsDisplayNode->SetColor( rgbDoubleVector );
  currentMarkupsDisplayNode->SetSelectedColor( rgbDoubleVector );
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::getNodeColor(QColor color)
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );
  if ( currentMarkupsNode == NULL )
    {
    return;
    }
  
  vtkMRMLDisplayNode* currentMarkupsDisplayNode = currentMarkupsNode->GetDisplayNode();
  if ( currentMarkupsDisplayNode == NULL )
    {
    return;
    }

  double rgbDoubleVector[3] = {0.0,0.0,0.0};
  currentMarkupsDisplayNode->GetSelectedColor(rgbDoubleVector);
  color.setRgb(rgbDoubleVector[0], rgbDoubleVector[1], rgbDoubleVector[2]);
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::highlightNthFiducial(int n)
{
  Q_D(qSlicerSimpleMarkupsWidget);

  if ( n >= 0 && n < d->MarkupsFiducialTableWidget->rowCount() )
    {
    d->MarkupsFiducialTableWidget->selectRow( n );
    }
  else
    {
    d->MarkupsFiducialTableWidget->clearSelection();
    }
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onColorButtonChanged(QColor color)
{
  Q_D(qSlicerSimpleMarkupsWidget);
  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );

  double colorDoubleVector[3] = {0.0,0.0,0.0};
  qMRMLUtils::qColorToColor( color, colorDoubleVector );

  if ( currentMarkupsNode != NULL && currentMarkupsNode->GetDisplayNode() != NULL )
    {
    currentMarkupsNode->GetDisplayNode()->SetSelectedColor( colorDoubleVector );
    }

  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onVisibilityButtonClicked()
{
  Q_D(qSlicerSimpleMarkupsWidget);
  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );

  if ( currentMarkupsNode != NULL && currentMarkupsNode->GetDisplayNode() != NULL )
    {
    currentMarkupsNode->GetDisplayNode()->SetVisibility( ! currentMarkupsNode->GetDisplayNode()->GetVisibility() );
    }

  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onLockButtonClicked()
{
  Q_D(qSlicerSimpleMarkupsWidget);
  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );

  if ( currentMarkupsNode != NULL )
    {
    currentMarkupsNode->SetLocked( ! currentMarkupsNode->GetLocked() );
    }

  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onDeleteButtonClicked()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );

  if ( currentMarkupsNode  == NULL )
    {
    return;
    }

  QItemSelectionModel* selectionModel = d->MarkupsFiducialTableWidget->selectionModel();
  std::vector< int > deleteFiducials;
  // Need to find selected before removing because removing automatically refreshes the table
  for ( int i = 0; i < d->MarkupsFiducialTableWidget->rowCount(); i++ )
    {
    if ( selectionModel->rowIntersectsSelection( i, d->MarkupsFiducialTableWidget->rootIndex() ) )
      {
      deleteFiducials.push_back( i );
      }
    }
  // Traversing this way should be more efficient and correct
  for ( int i = deleteFiducials.size() - 1; i >= 0; i-- )
    {
    currentMarkupsNode->RemoveMarkup( deleteFiducials.at( i ) );
    }

  this->updateWidget();
}    

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onPlaceButtonClicked()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );

  // Depending to the current state, change the activeness and placeness for the current markups node
  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast( this->mrmlScene()->GetNodeByID( "vtkMRMLInteractionNodeSingleton" ) );

  bool isActive = currentMarkupsNode != NULL && d->MarkupsLogic->GetActiveListID().compare( currentMarkupsNode->GetID() ) == 0;
  bool isPlace = interactionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place;

  if ( isPlace && isActive )
    {
    interactionNode->SetCurrentInteractionMode( vtkMRMLInteractionNode::ViewTransform );
    }
  else
    {
    interactionNode->SetCurrentInteractionMode( vtkMRMLInteractionNode::Place );
    }

  if ( currentMarkupsNode != NULL )
    {
    // If there are other widgets, they are responsible for updating themselves
    d->MarkupsLogic->SetActiveListID( currentMarkupsNode );
    }

  this->updateWidget();

  emit markupsFiducialPlaceModeChanged();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::activate()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );

  // Depending to the current state, change the activeness and placeness for the current markups node
  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast( this->mrmlScene()->GetNodeByID( "vtkMRMLInteractionNodeSingleton" ) );

  bool isActive = currentMarkupsNode != NULL && d->MarkupsLogic->GetActiveListID().compare( currentMarkupsNode->GetID() ) == 0;

  if ( ! isActive )
    {
    // Deactivate
    interactionNode->SetCurrentInteractionMode( vtkMRMLInteractionNode::ViewTransform );
    }

  if ( currentMarkupsNode != NULL )
    {
    // If there are other widgets, they are responsible for updating themselves
    d->MarkupsLogic->SetActiveListID( currentMarkupsNode );
    }

  this->updateWidget();

  emit markupsFiducialActivated();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onActiveButtonClicked()
{
  this->activate();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onMarkupsFiducialNodeChanged()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );

  // Reconnect the appropriate nodes
  this->qvtkDisconnectAll();
  this->connectInteractionAndSelectionNodes();
  this->qvtkConnect( currentMarkupsNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  this->qvtkConnect( currentMarkupsNode, vtkMRMLMarkupsNode::MarkupAddedEvent, d->MarkupsFiducialTableWidget, SLOT( scrollToBottom() ) );

  // Depending to the current state, change the activeness and placeness for the current markups node
  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast( this->mrmlScene()->GetNodeByID( "vtkMRMLInteractionNodeSingleton" ) );

  if ( currentMarkupsNode != NULL )
    {
    d->MarkupsLogic->SetActiveListID( currentMarkupsNode ); // If there are other widgets, they are responsible for updating themselves
    interactionNode->SetCurrentInteractionMode( vtkMRMLInteractionNode::Place );
    // interactionNode->SetPlaceModePersistence( true ); // Use whatever persistence the user has already set
    }
  else
    {
    interactionNode->SetCurrentInteractionMode( vtkMRMLInteractionNode::ViewTransform );
    }

  this->updateWidget();

  emit markupsFiducialNodeChanged();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onMarkupsFiducialNodeAdded( vtkMRMLNode* newNode )
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsFiducialNode* newMarkupsFiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( newNode );
  d->MarkupsLogic->AddNewDisplayNodeForMarkupsNode( newMarkupsFiducialNode ); // Make sure there is an associated display node
  d->MarkupsFiducialNodeComboBox->setCurrentNode( newMarkupsFiducialNode );
  this->setNodeColor( d->DefaultNodeColor );
  this->onMarkupsFiducialNodeChanged();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onMarkupsFiducialTableContextMenu(const QPoint& position)
{
  Q_D(qSlicerSimpleMarkupsWidget);

  QPoint globalPosition = d->MarkupsFiducialTableWidget->viewport()->mapToGlobal( position );

  QMenu* fiducialsMenu = new QMenu( d->MarkupsFiducialTableWidget );
  QAction* activateAction = new QAction( "Make fiducial list active", fiducialsMenu );
  QAction* deleteAction = new QAction( "Delete highlighted fiducials", fiducialsMenu );
  QAction* upAction = new QAction( "Move current fiducial up", fiducialsMenu );
  QAction* downAction = new QAction( "Move current fiducial down", fiducialsMenu );
  QAction* jumpAction = new QAction( "Jump slices to fiducial", fiducialsMenu );

  fiducialsMenu->addAction( activateAction );
  fiducialsMenu->addAction( deleteAction );
  fiducialsMenu->addAction( upAction );
  fiducialsMenu->addAction( downAction );
  fiducialsMenu->addAction( jumpAction );

  QAction* selectedAction = fiducialsMenu->exec( globalPosition );

  int currentFiducial = d->MarkupsFiducialTableWidget->currentRow();
  vtkMRMLMarkupsFiducialNode* currentNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );
  
  if ( currentNode == NULL )
    {
    return;
    }

  // Only do this for non-null node
  if ( selectedAction == activateAction )
    {
    d->MarkupsLogic->SetActiveListID( currentNode ); // If there are other widgets, they are responsible for updating themselves
    emit markupsFiducialNodeChanged();
    }

  if ( selectedAction == deleteAction )
    {
    QItemSelectionModel* selectionModel = d->MarkupsFiducialTableWidget->selectionModel();
    std::vector< int > deleteFiducials;
    // Need to find selected before removing because removing automatically refreshes the table
    for ( int i = 0; i < d->MarkupsFiducialTableWidget->rowCount(); i++ )
      {
      if ( selectionModel->rowIntersectsSelection( i, d->MarkupsFiducialTableWidget->rootIndex() ) )
        {
        deleteFiducials.push_back( i );
        }
      }
    //Traversing this way should be more efficient and correct
    for ( int i = deleteFiducials.size() - 1; i >= 0; i-- )
      {
      currentNode->RemoveMarkup( deleteFiducials.at( i ) );
      }
    }

  
  if ( selectedAction == upAction )
    {
    if ( currentFiducial > 0 )
      {
      currentNode->SwapMarkups( currentFiducial, currentFiducial - 1 );
      }
    }

  if ( selectedAction == downAction )
    {
    if ( currentFiducial < currentNode->GetNumberOfFiducials() - 1 )
      {
      currentNode->SwapMarkups( currentFiducial, currentFiducial + 1 );
      }
    }

  if ( selectedAction == jumpAction )
    {
    d->MarkupsLogic->JumpSlicesToNthPointInMarkup( this->getCurrentNode()->GetID(), currentFiducial );
    }

  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::onMarkupsFiducialEdited(int row, int column)
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsFiducialNode* currentMarkupsFiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( this->getCurrentNode() );

  if ( currentMarkupsFiducialNode == NULL )
    {
    return;
    }

  // Find the fiducial's current properties
  double currentFiducialPosition[3] = { 0, 0, 0 };
  currentMarkupsFiducialNode->GetNthFiducialPosition( row, currentFiducialPosition );
  std::string currentFiducialLabel = currentMarkupsFiducialNode->GetNthFiducialLabel( row );

  // Find the entry that we changed
  QTableWidgetItem* qItem = d->MarkupsFiducialTableWidget->item( row, column );
  QString qText = qItem->text();

  if ( column == FIDUCIAL_LABEL_COLUMN )
    {
    currentMarkupsFiducialNode->SetNthFiducialLabel( row, qText.toStdString() );
    }

  // Check if the value can be converted to double is already performed implicitly
  double newFiducialPosition = qText.toDouble();

  // Change the position values
  if ( column == FIDUCIAL_X_COLUMN )
    {
    currentFiducialPosition[ 0 ] = newFiducialPosition;
    }
  if ( column == FIDUCIAL_Y_COLUMN )
    {
    currentFiducialPosition[ 1 ] = newFiducialPosition;
    }
  if ( column == FIDUCIAL_Z_COLUMN )
    {
    currentFiducialPosition[ 2 ] = newFiducialPosition;
    }

  currentMarkupsFiducialNode->SetNthFiducialPositionFromArray( row, currentFiducialPosition );

  this->updateWidget(); // This may not be necessary the widget is updated whenever a fiducial is changed
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidget::updateWidget()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsFiducialNode* currentMarkupsFiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );
  if ( currentMarkupsFiducialNode == NULL )
    {
    d->MarkupsFiducialTableWidget->clear();
    d->MarkupsFiducialTableWidget->setRowCount( 0 );
    d->MarkupsFiducialTableWidget->setColumnCount( 0 );
    d->ActiveButton->setChecked( Qt::Unchecked );
    d->PlaceButton->setChecked( Qt::Unchecked );
    return;
    }

  // Set the button indicating if this list is active
  d->ActiveButton->blockSignals( true );
  d->PlaceButton->blockSignals( true );

  // Depending to the current state, change the activeness and placeness for the current markups node
  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast( this->mrmlScene()->GetNodeByID( "vtkMRMLInteractionNodeSingleton" ) );

  if ( currentMarkupsFiducialNode->GetDisplayNode() != NULL  )
    {
    double* color;
    QColor qColor;

    color = currentMarkupsFiducialNode->GetDisplayNode()->GetSelectedColor();
    qMRMLUtils::colorToQColor( color, qColor );

    d->ColorButton->setColor( qColor );
    }

  if ( d->MarkupsLogic->GetActiveListID().compare( currentMarkupsFiducialNode->GetID() ) == 0 )
    {
    d->ActiveButton->setChecked( Qt::Checked );
    }
  else
    {
    d->ActiveButton->setChecked( Qt::Unchecked );
    }

  if ( interactionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place && d->MarkupsLogic->GetActiveListID().compare( currentMarkupsFiducialNode->GetID() ) == 0 )
    {
    d->PlaceButton->setChecked( true );
    }
  else
    {
    d->PlaceButton->setChecked( false );
    }

  if ( currentMarkupsFiducialNode->GetDisplayNode() != NULL && currentMarkupsFiducialNode->GetDisplayNode()->GetVisibility() )
    {
    d->VisibilityButton->setIcon( QIcon( ":/Icons/Small/SlicerVisible.png" ) );
    }
  else
    {
    d->VisibilityButton->setIcon( QIcon( ":/Icons/Small/SlicerInvisible.png" ) );
    }

  if ( currentMarkupsFiducialNode->GetLocked() )
    {
    d->LockButton->setIcon( QIcon( ":/Icons/Small/SlicerLock.png" ) );
    }
  else
    {
    d->LockButton->setIcon( QIcon( ":/Icons/Small/SlicerUnlock.png" ) );
    }

  d->ActiveButton->blockSignals( false );
  d->PlaceButton->blockSignals( false );

  // Update the fiducials table
  d->MarkupsFiducialTableWidget->blockSignals( true );
 
  d->MarkupsFiducialTableWidget->clear();
  QStringList MarkupsTableHeaders;
  MarkupsTableHeaders << "Label" << "X" << "Y" << "Z";
  d->MarkupsFiducialTableWidget->setRowCount( currentMarkupsFiducialNode->GetNumberOfFiducials() );
  d->MarkupsFiducialTableWidget->setColumnCount( FIDUCIAL_COLUMNS );
  d->MarkupsFiducialTableWidget->setHorizontalHeaderLabels( MarkupsTableHeaders );
  d->MarkupsFiducialTableWidget->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  
  double fiducialPosition[ 3 ] = { 0, 0, 0 };
  std::string fiducialLabel = "";
  for ( int i = 0; i < currentMarkupsFiducialNode->GetNumberOfFiducials(); i++ )
    {
    fiducialLabel = currentMarkupsFiducialNode->GetNthFiducialLabel( i );
    currentMarkupsFiducialNode->GetNthFiducialPosition( i, fiducialPosition );

    QTableWidgetItem* labelItem = new QTableWidgetItem( QString::fromStdString( fiducialLabel ) );
    QTableWidgetItem* xItem = new QTableWidgetItem( QString::number( fiducialPosition[0], 'f', 3 ) );
    QTableWidgetItem* yItem = new QTableWidgetItem( QString::number( fiducialPosition[1], 'f', 3 ) );
    QTableWidgetItem* zItem = new QTableWidgetItem( QString::number( fiducialPosition[2], 'f', 3 ) );

    d->MarkupsFiducialTableWidget->setItem( i, 0, labelItem );
    d->MarkupsFiducialTableWidget->setItem( i, 1, xItem );
    d->MarkupsFiducialTableWidget->setItem( i, 2, yItem );
    d->MarkupsFiducialTableWidget->setItem( i, 3, zItem );
    }

  d->MarkupsFiducialTableWidget->blockSignals( false );

  emit updateFinished();
}

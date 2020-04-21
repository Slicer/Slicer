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
#include "qSlicerMarkupsPlaceWidget.h"

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
#include <vtkMRMLMarkupsFiducialNode.h>

// Qt includes
#include <QColor>
#include <QDebug>
#include <QList>
#include <QMenu>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerMarkupsPlaceWidgetPrivate
  : public Ui_qSlicerMarkupsPlaceWidget
{
  Q_DECLARE_PUBLIC(qSlicerMarkupsPlaceWidget);
protected:
  qSlicerMarkupsPlaceWidget* const q_ptr;

public:
  qSlicerMarkupsPlaceWidgetPrivate( qSlicerMarkupsPlaceWidget& object);
  ~qSlicerMarkupsPlaceWidgetPrivate();
  virtual void setupUi(qSlicerMarkupsPlaceWidget*);

public:
  vtkWeakPointer<vtkSlicerMarkupsLogic> MarkupsLogic;
  vtkWeakPointer<vtkMRMLMarkupsNode> CurrentMarkupsNode;
  vtkWeakPointer<vtkMRMLSelectionNode> SelectionNode;
  vtkWeakPointer<vtkMRMLInteractionNode> InteractionNode;
  QMenu* PlaceMenu;
  QMenu* DeleteMenu;
  qSlicerMarkupsPlaceWidget::PlaceMultipleMarkupsType PlaceMultipleMarkups;
  QList < QWidget* > OptionsWidgets;
  QColor DefaultNodeColor;
  bool DeleteMarkupsButtonVisible;
  bool DeleteAllMarkupsOptionVisible;
  bool LastSignaledPlaceModeEnabled; // if placeModeEnabled changes compared to this value then a activeMarkupsPlaceModeChanged signal will be emitted

};

// --------------------------------------------------------------------------
qSlicerMarkupsPlaceWidgetPrivate::qSlicerMarkupsPlaceWidgetPrivate( qSlicerMarkupsPlaceWidget& object)
  : q_ptr(&object)
{
  this->DeleteMarkupsButtonVisible = true;
  this->DeleteAllMarkupsOptionVisible = true;
  this->PlaceMultipleMarkups = qSlicerMarkupsPlaceWidget::ShowPlaceMultipleMarkupsOption;
  this->PlaceMenu = nullptr;
  this->DeleteMenu = nullptr;
  this->LastSignaledPlaceModeEnabled = false;
}

//-----------------------------------------------------------------------------
qSlicerMarkupsPlaceWidgetPrivate::~qSlicerMarkupsPlaceWidgetPrivate() = default;

// --------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidgetPrivate::setupUi(qSlicerMarkupsPlaceWidget* widget)
{
  this->Ui_qSlicerMarkupsPlaceWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerMarkupsPlaceWidget methods

//-----------------------------------------------------------------------------
qSlicerMarkupsPlaceWidget::qSlicerMarkupsPlaceWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerMarkupsPlaceWidgetPrivate(*this) )
{
  this->setup();
}

//-----------------------------------------------------------------------------
qSlicerMarkupsPlaceWidget::~qSlicerMarkupsPlaceWidget()
{
  this->setCurrentNode(nullptr);
  this->setInteractionNode(nullptr);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::setup()
{
  Q_D(qSlicerMarkupsPlaceWidget);

  // This cannot be called by the constructor, because Slicer may not exist when the constructor is called
  d->MarkupsLogic = nullptr;
  if (qSlicerApplication::application() != nullptr && qSlicerApplication::application()->moduleManager() != nullptr)
    {
    qSlicerAbstractCoreModule* markupsModule = qSlicerApplication::application()->moduleManager()->module( "Markups" );
    if ( markupsModule != nullptr )
      {
      d->MarkupsLogic = vtkSlicerMarkupsLogic::SafeDownCast( markupsModule->logic() );
      }
    }
  if (d->MarkupsLogic == nullptr)
    {
    qCritical() << Q_FUNC_INFO << ": Markups module is not found, some markup manipulation features will not be available";
    }
  d->setupUi(this);

  d->OptionsWidgets << d->ColorButton << d->PlaceButton << d->DeleteButton << d->MoreButton;

  d->DefaultNodeColor.setRgb(0.0,1.0,0.0); // displayed when no node is selected
  // Use the pressed signal (otherwise we can unpress buttons without clicking them)
  connect( d->ColorButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( onColorButtonChanged( QColor ) ) );

  d->PlaceMenu = new QMenu(tr("Place options"), d->PlaceButton);
  d->PlaceMenu->setObjectName("MoreMenu");
  d->PlaceMenu->addAction(d->ActionPersistent);
  QObject::connect(d->ActionPersistent, SIGNAL(toggled(bool)), this, SLOT(onPlacePersistent(bool)));
  if (d->PlaceMultipleMarkups == ShowPlaceMultipleMarkupsOption)
    {
    d->PlaceButton->setMenu(d->PlaceMenu);
    }
  connect( d->PlaceButton, SIGNAL(toggled(bool)), this, SLOT(setPlaceModeEnabled(bool)) );

  d->DeleteMenu = new QMenu(tr("Delete options"), d->DeleteButton);
  d->DeleteMenu->setObjectName("DeleteMenu");
  d->DeleteMenu->addAction(d->ActionDeleteAll);
  QObject::connect(d->ActionDeleteAll, SIGNAL(triggered()), this, SLOT(deleteAllPoints()));
  if (d->DeleteAllMarkupsOptionVisible)
    {
    d->DeleteButton->setMenu(d->DeleteMenu);
    }
  d->DeleteButton->setVisible(d->DeleteMarkupsButtonVisible);
  connect( d->DeleteButton, SIGNAL(clicked()), this, SLOT(deleteLastPoint()) );

  QMenu* moreMenu = new QMenu(tr("More options"), d->MoreButton);
  moreMenu->setObjectName("moreMenu");
  moreMenu->addAction(d->ActionVisibility);
  moreMenu->addAction(d->ActionLocked);
  QObject::connect(d->ActionVisibility, SIGNAL(triggered()), this, SLOT(onVisibilityButtonClicked()));
  QObject::connect(d->ActionLocked, SIGNAL(triggered()), this, SLOT(onLockedButtonClicked()));
  d->MoreButton->setMenu(moreMenu);

  updateWidget();
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qSlicerMarkupsPlaceWidget::currentNode() const
{
  Q_D(const qSlicerMarkupsPlaceWidget);
  return d->CurrentMarkupsNode;
}

//-----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* qSlicerMarkupsPlaceWidget::currentMarkupsFiducialNode() const
{
  Q_D(const qSlicerMarkupsPlaceWidget);
  return vtkMRMLMarkupsFiducialNode::SafeDownCast(d->CurrentMarkupsNode);
}

//-----------------------------------------------------------------------------
vtkMRMLMarkupsNode* qSlicerMarkupsPlaceWidget::currentMarkupsNode() const
{
  Q_D(const qSlicerMarkupsPlaceWidget);
  return d->CurrentMarkupsNode;
}

//-----------------------------------------------------------------------------
vtkMRMLInteractionNode* qSlicerMarkupsPlaceWidget::interactionNode()const
{
  Q_D(const qSlicerMarkupsPlaceWidget);
  return d->InteractionNode;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::setInteractionNode(vtkMRMLInteractionNode* interactionNode)
{
  Q_D(qSlicerMarkupsPlaceWidget);
  if (d->InteractionNode == interactionNode)
    {
    return;
    }
  this->qvtkReconnect(d->InteractionNode, interactionNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidget()));
  d->InteractionNode = interactionNode;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::setCurrentNode(vtkMRMLNode* currentNode)
{
  Q_D(qSlicerMarkupsPlaceWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(currentNode);
  if (currentMarkupsNode==d->CurrentMarkupsNode)
    {
    // not changed
    return;
    }

  // Reconnect the appropriate nodes
  this->qvtkReconnect(d->CurrentMarkupsNode, currentMarkupsNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidget()));
  this->qvtkReconnect(d->CurrentMarkupsNode, currentMarkupsNode, vtkMRMLMarkupsNode::PointAddedEvent, this, SLOT(updateWidget()));
  this->qvtkReconnect(d->CurrentMarkupsNode, currentMarkupsNode, vtkMRMLMarkupsNode::PointRemovedEvent, this, SLOT(updateWidget()));
  this->qvtkReconnect(d->CurrentMarkupsNode, currentMarkupsNode, vtkMRMLDisplayableNode::DisplayModifiedEvent, this, SLOT(updateWidget()));
  d->CurrentMarkupsNode = currentMarkupsNode;

  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::deleteLastPoint()
{
  vtkMRMLMarkupsNode* currentMarkupsNode = this->currentMarkupsNode();
  if ( currentMarkupsNode == nullptr )
    {
    return;
    }
  if (currentMarkupsNode->GetNumberOfControlPoints() < 1)
    {
    return;
    }
  currentMarkupsNode->RemoveNthControlPoint(currentMarkupsNode->GetNumberOfControlPoints() - 1);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::deleteLastMarkup()
{
  this->deleteLastPoint();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::deleteAllPoints()
{
  vtkMRMLMarkupsNode* currentMarkupsNode = this->currentMarkupsNode();
  if ( currentMarkupsNode == nullptr )
    {
    return;
    }

  currentMarkupsNode->RemoveAllControlPoints();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::deleteAllMarkups()
{
  this->deleteAllPoints();
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsPlaceWidget::currentNodeActive() const
{
  Q_D(const qSlicerMarkupsPlaceWidget);
  vtkMRMLMarkupsNode* currentMarkupsNode = this->currentMarkupsNode();
  if (d->MarkupsLogic == nullptr || this->mrmlScene() == nullptr ||
      currentMarkupsNode == nullptr || d->InteractionNode == nullptr ||
      d->SelectionNode == nullptr)
    {
    return false;
    }
  bool currentNodeActive = (d->MarkupsLogic->GetActiveListID().compare( currentMarkupsNode->GetID() ) == 0);
  const char* activePlaceNodeClassName = d->SelectionNode->GetActivePlaceNodeClassName();
  bool placeNodeClassNameMatches = activePlaceNodeClassName && std::string(activePlaceNodeClassName).compare(currentMarkupsNode->GetClassName())==0;
  return placeNodeClassNameMatches && currentNodeActive;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::setCurrentNodeActive(bool active)
{
  Q_D(qSlicerMarkupsPlaceWidget);
  if (d->MarkupsLogic == nullptr || this->mrmlScene() == nullptr || d->InteractionNode == nullptr)
    {
    if (active)
      {
      qCritical() << Q_FUNC_INFO << " failed: Markups module logic, scene, or interaction node is invalid";
      }
    return;
    }
  bool wasActive = this->currentNodeActive();
  if (wasActive!=active)
    {
    if (active)
      {
      d->MarkupsLogic->SetActiveListID(this->currentMarkupsNode());
      }
    else
      {
      d->MarkupsLogic->SetActiveListID(nullptr);
      d->InteractionNode->SetCurrentInteractionMode( vtkMRMLInteractionNode::ViewTransform );
      }
    }
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsPlaceWidget::placeModeEnabled() const
{
  Q_D(const qSlicerMarkupsPlaceWidget);
  if (!this->currentNodeActive())
    {
    return false;
    }
  vtkMRMLMarkupsNode* currentMarkupsNode = this->currentMarkupsNode();
  if (d->SelectionNode == nullptr || d->InteractionNode == nullptr || this->mrmlScene() == nullptr || currentMarkupsNode == nullptr)
    {
    return false;
    }
  bool placeMode = d->InteractionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place;
  const char* activePlaceNodeClassName = d->SelectionNode->GetActivePlaceNodeClassName();
  bool placeNodeClassNameMatches = activePlaceNodeClassName && std::string(activePlaceNodeClassName).compare(currentMarkupsNode->GetClassName()) == 0;
  return placeMode && placeNodeClassNameMatches;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::setPlaceModeEnabled(bool placeEnable)
{
  Q_D(qSlicerMarkupsPlaceWidget);
  if (d->MarkupsLogic == nullptr || this->mrmlScene() == nullptr ||
      d->InteractionNode == nullptr || this->currentMarkupsNode() == nullptr)
    {
    if (placeEnable)
      {
      qCritical() << Q_FUNC_INFO << " activate failed: Markups module logic, scene, or interaction node is invalid";
      }
    return;
    }
  bool wasActive = this->currentNodeActive();
  if ( placeEnable )
    {
    // activate and set place mode
    if (!wasActive)
      {
      d->MarkupsLogic->SetActiveListID(this->currentMarkupsNode());
      }
    if (d->PlaceMultipleMarkups == ForcePlaceSingleMarkup)
      {
      setPlaceModePersistency(false);
      }
    else if (d->PlaceMultipleMarkups == ForcePlaceMultipleMarkups)
      {
      setPlaceModePersistency(true);
      }
    d->InteractionNode->SetCurrentInteractionMode( vtkMRMLInteractionNode::Place );
    }
  else
    {
    // disable place mode
    d->InteractionNode->SetCurrentInteractionMode( vtkMRMLInteractionNode::ViewTransform );
    }
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsPlaceWidget::placeModePersistency() const
{
  Q_D(const qSlicerMarkupsPlaceWidget);
  if (d->InteractionNode == nullptr)
    {
    qCritical() << Q_FUNC_INFO << " failed: interactionNode is invalid";
    return false;
    }
  return d->InteractionNode->GetPlaceModePersistence();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::setPlaceModePersistency(bool persistent)
{
  Q_D(qSlicerMarkupsPlaceWidget);
  if (d->InteractionNode == nullptr)
    {
    qCritical() << Q_FUNC_INFO << " failed: interactionNode is invalid";
    return;
    }
  d->InteractionNode->SetPlaceModePersistence(persistent);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::updateWidget()
{
  Q_D(qSlicerMarkupsPlaceWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = this->currentMarkupsNode();
  if (d->MarkupsLogic == nullptr || this->mrmlScene() == nullptr ||
      d->InteractionNode == nullptr || currentMarkupsNode == nullptr)
    {
    d->ColorButton->setEnabled(false);
    d->PlaceButton->setEnabled(false);
    d->DeleteButton->setEnabled(false);
    d->MoreButton->setEnabled(false);
    bool wasBlockedColorButton = d->ColorButton->blockSignals(true);
    d->ColorButton->setColor(d->DefaultNodeColor);
    d->ColorButton->blockSignals(wasBlockedColorButton);
    if (d->LastSignaledPlaceModeEnabled)
      {
      emit activeMarkupsFiducialPlaceModeChanged(false);
      emit activeMarkupsPlaceModeChanged(false);
      d->LastSignaledPlaceModeEnabled = false;
      }
    return;
    }

  d->ColorButton->setEnabled(true);
  d->PlaceButton->setEnabled(true);
  d->DeleteButton->setEnabled(currentMarkupsNode->GetNumberOfControlPoints() > 0);
  d->MoreButton->setEnabled(true);

    // Set the button indicating if this list is active
  bool wasBlockedColorButton = d->ColorButton->blockSignals( true );
  bool wasBlockedVisibilityButton = d->ActionVisibility->blockSignals( true );
  bool wasBlockedLockButton = d->ActionLocked->blockSignals( true );

  if ( currentMarkupsNode->GetDisplayNode() != nullptr  )
    {
    double* color = currentMarkupsNode->GetDisplayNode()->GetSelectedColor();
    QColor qColor;
    qMRMLUtils::colorToQColor( color, qColor );
    d->ColorButton->setColor( qColor );
    }

  if ( currentMarkupsNode->GetLocked() )
    {
    d->ActionLocked->setIcon( QIcon( ":/Icons/Small/SlicerLock.png" ) );
    }
  else
    {
    d->ActionLocked->setIcon( QIcon( ":/Icons/Small/SlicerUnlock.png" ) );
    }

  d->ActionVisibility->setEnabled(currentMarkupsNode->GetDisplayNode() != nullptr);
  if (currentMarkupsNode->GetDisplayNode() != nullptr)
    {
    if (currentMarkupsNode->GetDisplayNode()->GetVisibility() )
      {
      d->ActionVisibility->setIcon( QIcon( ":/Icons/Small/SlicerVisible.png" ) );
      }
    else
      {
      d->ActionVisibility->setIcon( QIcon( ":/Icons/Small/SlicerInvisible.png" ) );
      }
    }

  d->ColorButton->blockSignals( wasBlockedColorButton );
  d->ActionVisibility->blockSignals( wasBlockedVisibilityButton);
  d->ActionLocked->blockSignals( wasBlockedLockButton );

  bool wasBlockedPlaceButton = d->PlaceButton->blockSignals( true );
  d->PlaceButton->setChecked(placeModeEnabled());
  d->PlaceButton->blockSignals( wasBlockedPlaceButton );

  bool wasBlockedPersistencyAction = d->ActionPersistent->blockSignals( true );
  d->ActionPersistent->setChecked(placeModePersistency());
  d->ActionPersistent->blockSignals( wasBlockedPersistencyAction );

  bool currentPlaceModeEnabled = placeModeEnabled();
  if (d->LastSignaledPlaceModeEnabled != currentPlaceModeEnabled)
      {
      emit activeMarkupsFiducialPlaceModeChanged(currentPlaceModeEnabled);
      emit activeMarkupsPlaceModeChanged(currentPlaceModeEnabled);
      d->LastSignaledPlaceModeEnabled = currentPlaceModeEnabled;
      }
}

//------------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerMarkupsPlaceWidget);

  this->Superclass::setMRMLScene(scene);

  vtkMRMLSelectionNode* selectionNode = nullptr;
  vtkMRMLInteractionNode *interactionNode = nullptr;

  if (d->MarkupsLogic != nullptr && d->MarkupsLogic->GetMRMLScene() != nullptr)
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast( d->MarkupsLogic->GetMRMLScene()->GetNodeByID( d->MarkupsLogic->GetSelectionNodeID() ) );
    interactionNode = vtkMRMLInteractionNode::SafeDownCast( d->MarkupsLogic->GetMRMLScene()->GetNodeByID( "vtkMRMLInteractionNodeSingleton" ) );
    }

  this->setInteractionNode(interactionNode);

  this->qvtkReconnect(d->SelectionNode, selectionNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidget()));
  d->SelectionNode = selectionNode;

  this->updateWidget();
}

//------------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::onPlacePersistent(bool enable)
{
  this->setPlaceModeEnabled(enable);
  this->setPlaceModePersistency(enable);
}

//-----------------------------------------------------------------------------
qSlicerMarkupsPlaceWidget::PlaceMultipleMarkupsType qSlicerMarkupsPlaceWidget::placeMultipleMarkups() const
{
  Q_D(const qSlicerMarkupsPlaceWidget);
  return d->PlaceMultipleMarkups;
}

//------------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::setPlaceMultipleMarkups(PlaceMultipleMarkupsType option)
{
  Q_D(qSlicerMarkupsPlaceWidget);
  d->PlaceMultipleMarkups = option;
  if (d->PlaceButton)
    {
    d->PlaceButton->setMenu(d->PlaceMultipleMarkups == ShowPlaceMultipleMarkupsOption ? d->PlaceMenu : nullptr);
    }
  if (this->placeModeEnabled())
    {
    if (d->PlaceMultipleMarkups == ForcePlaceSingleMarkup)
      {
      this->setPlaceModePersistency(false);
      }
    else if (d->PlaceMultipleMarkups == ForcePlaceMultipleMarkups)
      {
      this->setPlaceModePersistency(true);
      }
    }
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsPlaceWidget::deleteAllMarkupsOptionVisible() const
{
  Q_D(const qSlicerMarkupsPlaceWidget);
  return d->DeleteAllMarkupsOptionVisible;
}

//------------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::setDeleteAllMarkupsOptionVisible(bool visible)
{
  Q_D(qSlicerMarkupsPlaceWidget);
  d->DeleteAllMarkupsOptionVisible = visible;
  if (d->DeleteButton)
    {
    d->DeleteButton->setMenu(visible ? d->DeleteMenu : nullptr);
    }
}

//-----------------------------------------------------------------------------
QToolButton* qSlicerMarkupsPlaceWidget::placeButton() const
{
  Q_D(const qSlicerMarkupsPlaceWidget);
  return d->PlaceButton;
}

//-----------------------------------------------------------------------------
QToolButton* qSlicerMarkupsPlaceWidget::deleteButton() const
{
  Q_D(const qSlicerMarkupsPlaceWidget);
  return d->DeleteButton;
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsPlaceWidget::buttonsVisible() const
{
  Q_D(const qSlicerMarkupsPlaceWidget);
  foreach( QWidget *w, d->OptionsWidgets )
    {
    if (!w->isVisible())
      {
      return false;
      }
    }
  return true;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::setButtonsVisible(bool visible)
{
  Q_D(qSlicerMarkupsPlaceWidget);
  foreach( QWidget *w, d->OptionsWidgets )
    {
    w->setVisible(visible);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::setNodeColor(QColor color)
{
  vtkMRMLMarkupsNode* currentMarkupsNode = this->currentMarkupsNode();
  if ( currentMarkupsNode == nullptr )
    {
    return;
    }

  vtkMRMLDisplayNode* currentMarkupsDisplayNode = currentMarkupsNode->GetDisplayNode();
  if ( currentMarkupsDisplayNode == nullptr )
    {
    return;
    }

  double rgbDoubleVector[3] = {color.redF(),color.greenF(),color.blueF()};
  currentMarkupsDisplayNode->SetColor( rgbDoubleVector );
  currentMarkupsDisplayNode->SetSelectedColor( rgbDoubleVector );
}

//-----------------------------------------------------------------------------
QColor qSlicerMarkupsPlaceWidget::nodeColor() const
{
  Q_D(const qSlicerMarkupsPlaceWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = this->currentMarkupsNode();
  if ( currentMarkupsNode == nullptr )
    {
    return d->DefaultNodeColor;
    }

  vtkMRMLDisplayNode* currentMarkupsDisplayNode = currentMarkupsNode->GetDisplayNode();
  if ( currentMarkupsDisplayNode == nullptr )
    {
    return d->DefaultNodeColor;
    }

  QColor color;
  double rgbDoubleVector[3] = {0.0,0.0,0.0};
  currentMarkupsDisplayNode->GetSelectedColor(rgbDoubleVector);
  color.setRgb(static_cast<int>(rgbDoubleVector[0]),
               static_cast<int>(rgbDoubleVector[1]),
               static_cast<int>(rgbDoubleVector[2]));
  return color;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::setDefaultNodeColor(QColor color)
{
  Q_D(qSlicerMarkupsPlaceWidget);
  d->DefaultNodeColor = color;
}

//-----------------------------------------------------------------------------
QColor qSlicerMarkupsPlaceWidget::defaultNodeColor() const
{
  Q_D(const qSlicerMarkupsPlaceWidget);
  return d->DefaultNodeColor;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::onColorButtonChanged(QColor color)
{
  vtkMRMLMarkupsNode* currentMarkupsNode = this->currentMarkupsNode();
  if (currentMarkupsNode == nullptr || currentMarkupsNode->GetDisplayNode() == nullptr)
    {
    return;
    }
  double colorDoubleVector[3] = {0.0,0.0,0.0};
  qMRMLUtils::qColorToColor( color, colorDoubleVector );
  currentMarkupsNode->GetDisplayNode()->SetSelectedColor( colorDoubleVector );
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::onVisibilityButtonClicked()
{
  vtkMRMLMarkupsNode* currentMarkupsNode = this->currentMarkupsNode();
  if ( currentMarkupsNode == nullptr || currentMarkupsNode->GetDisplayNode() == nullptr )
    {
    return;
    }
  currentMarkupsNode->GetDisplayNode()->SetVisibility( ! currentMarkupsNode->GetDisplayNode()->GetVisibility() );
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsPlaceWidget::onLockedButtonClicked()
{
  vtkMRMLMarkupsNode* currentMarkupsNode = this->currentMarkupsNode();
  if ( currentMarkupsNode == nullptr )
    {
    return;
    }
  currentMarkupsNode->SetLocked( ! currentMarkupsNode->GetLocked() );
}

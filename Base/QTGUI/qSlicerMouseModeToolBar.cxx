/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
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
#include <QToolButton>

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLWindowLevelWidget.h>

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"
#include "qMRMLSliceView.h"
#include "qMRMLSliceWidget.h"
#include "qSlicerMouseModeToolBar_p.h"

// SlicerLogic includes
#include <vtkSlicerApplicationLogic.h>

//---------------------------------------------------------------------------
// qSlicerMouseModeToolBarPrivate methods

//---------------------------------------------------------------------------
qSlicerMouseModeToolBarPrivate::qSlicerMouseModeToolBarPrivate(qSlicerMouseModeToolBar& object)
  : q_ptr(&object)
{
  this->AdjustViewAction = nullptr;
  this->AdjustWindowLevelAction = nullptr;
  this->AdjustWindowLevelAdjustModeAction = nullptr;
  this->AdjustWindowLevelRegionModeAction = nullptr;
  this->AdjustWindowLevelCenteredRegionModeAction = nullptr;
  this->AdjustWindowLevelModeMapper = nullptr;
  this->AdjustWindowLevelMenu = nullptr;

  this->PlaceWidgetAction = nullptr;
  this->PlaceWidgetMenu = nullptr;

  this->PersistenceAction = nullptr;

  this->PlaceModesActionGroup = nullptr;
  this->InteractionModesActionGroup = nullptr;
  this->DefaultPlaceClassName = "vtkMRMLMarkupsFiducialNode";
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::init()
{
  Q_Q(qSlicerMouseModeToolBar);

  this->InteractionModesActionGroup = new QActionGroup(q);
  this->InteractionModesActionGroup->setExclusive(true);

  // Adjust view mode

  this->AdjustViewAction = new QAction(this);
  this->AdjustViewAction->setObjectName("AdjustViewAction");
  this->AdjustViewAction->setData(vtkMRMLInteractionNode::ViewTransform);
  this->AdjustViewAction->setToolTip(qSlicerMouseModeToolBar::tr("Translate/rotate view, adjust displayed objects"));
  this->AdjustViewAction->setIcon(QIcon(":/Icons/MouseViewTransformMode.png"));
  this->AdjustViewAction->setText(qSlicerMouseModeToolBar::tr("View"));
  this->AdjustViewAction->setCheckable(true);

  QObject::connect(this->AdjustViewAction, SIGNAL(toggled(bool)),
    q, SLOT(interactionModeActionTriggered(bool)));
  q->addAction(this->AdjustViewAction);
  this->InteractionModesActionGroup->addAction(this->AdjustViewAction);

  // Window/level mode

  QActionGroup* windowLevelModeActions = new QActionGroup(q);
  windowLevelModeActions->setExclusive(true);

  this->AdjustWindowLevelAdjustModeAction = new QAction(q);
  this->AdjustWindowLevelAdjustModeAction->setText(tr("Adjust"));
  this->AdjustWindowLevelAdjustModeAction->setToolTip(tr("Adjust window/level by click-and-drag in a slice viewer."));
  this->AdjustWindowLevelAdjustModeAction->setCheckable(true);

  this->AdjustWindowLevelRegionModeAction = new QAction(q);
  this->AdjustWindowLevelRegionModeAction->setText(tr("Select region"));
  this->AdjustWindowLevelRegionModeAction->setToolTip(
    tr("Set window level based on a rectangular region, specified by click-and-drag in a slice viewer. Click position is used as region corner."));
  this->AdjustWindowLevelRegionModeAction->setCheckable(true);

  this->AdjustWindowLevelCenteredRegionModeAction = new QAction(q);
  this->AdjustWindowLevelCenteredRegionModeAction->setText(tr("Select region - centered"));
  this->AdjustWindowLevelCenteredRegionModeAction->setToolTip(
    tr("Set window level based on a rectangular region, specified by click-and-drag in a slice viewer. Click position is used as region center."));
  this->AdjustWindowLevelCenteredRegionModeAction->setCheckable(true);

  windowLevelModeActions->addAction(this->AdjustWindowLevelAdjustModeAction);
  windowLevelModeActions->addAction(this->AdjustWindowLevelRegionModeAction);
  windowLevelModeActions->addAction(this->AdjustWindowLevelCenteredRegionModeAction);

  this->AdjustWindowLevelModeMapper = new ctkSignalMapper(q);
  this->AdjustWindowLevelModeMapper->setMapping(this->AdjustWindowLevelAdjustModeAction, vtkMRMLWindowLevelWidget::ModeAdjust);
  this->AdjustWindowLevelModeMapper->setMapping(this->AdjustWindowLevelRegionModeAction, vtkMRMLWindowLevelWidget::ModeRectangle);
  this->AdjustWindowLevelModeMapper->setMapping(this->AdjustWindowLevelCenteredRegionModeAction, vtkMRMLWindowLevelWidget::ModeRectangleCentered);
  QObject::connect(windowLevelModeActions, SIGNAL(triggered(QAction*)), this->AdjustWindowLevelModeMapper, SLOT(map(QAction*)));
  QObject::connect(this->AdjustWindowLevelModeMapper, SIGNAL(mapped(int)), q, SLOT(setAdjustWindowLevelMode(int)));

  // Menu
  this->AdjustWindowLevelMenu = new QMenu(tr("Adjust window/level"), q);
  this->AdjustWindowLevelMenu->addActions(windowLevelModeActions->actions());

  this->AdjustWindowLevelAction = new QAction(this);
  this->AdjustWindowLevelAction->setObjectName("AdjustWindowLevelAction");
  this->AdjustWindowLevelAction->setData(vtkMRMLInteractionNode::AdjustWindowLevel);
  this->AdjustWindowLevelAction->setToolTip(qSlicerMouseModeToolBar::tr(
    "Adjust window/level of volume by left-click-and-drag in slice views."
    " Hold down Ctrl/Cmd key for temporarily switch between adjustment and region-based setting."));
  this->AdjustWindowLevelAction->setIcon(QIcon(":/Icons/MouseWindowLevelMode.png"));
  this->AdjustWindowLevelAction->setText(qSlicerMouseModeToolBar::tr("Window/level"));
  this->AdjustWindowLevelAction->setCheckable(true);
  this->AdjustWindowLevelAction->setMenu(this->AdjustWindowLevelMenu);
  //this->AdjustWindowLevelAction->setPopupMode(QToolButton::MenuButtonPopup);

  QObject::connect(this->AdjustWindowLevelAction, SIGNAL(toggled(bool)),
    q, SLOT(interactionModeActionTriggered(bool)));
  q->addAction(this->AdjustWindowLevelAction);
  this->InteractionModesActionGroup->addAction(this->AdjustWindowLevelAction);



  // Place mode

  // persistence
  this->PersistenceAction = new QAction(q);
  this->PersistenceAction->setText(qSlicerMouseModeToolBar::tr("Persistent"));
  this->PersistenceAction->setToolTip(qSlicerMouseModeToolBar::tr("Switch between single place and persistent place modes."));
  this->PersistenceAction->setCheckable(true);
  this->PersistenceAction->setChecked(false);
  connect(this->PersistenceAction, SIGNAL(triggered(bool)),
          q, SLOT(setPersistence(bool)));

  this->PlaceModesActionGroup = new QActionGroup(q);
  this->PlaceModesActionGroup->setExclusive(true);
  // New actions should be added when interaction modes are registered with the scene.

  // popuplate the create and place menu, with persistence first
  this->PlaceWidgetMenu = new QMenu(qSlicerMouseModeToolBar::tr("Create and Place"), q);
  this->PlaceWidgetMenu->setObjectName("PlaceWidgetMenu");
  this->PlaceWidgetMenu->addAction(this->PersistenceAction);
  this->PlaceWidgetMenu->addSeparator();
  this->PlaceWidgetMenu->addActions(this->PlaceModesActionGroup->actions());
  this->PlaceWidgetMenu->addSeparator();

  this->PlaceWidgetAction = new QAction(this);
  this->PlaceWidgetAction->setObjectName("PlaceWidgetAction");
  this->PlaceWidgetAction->setData(vtkMRMLInteractionNode::Place);
  this->PlaceWidgetAction->setToolTip(qSlicerMouseModeToolBar::tr("Create and Place"));
  this->PlaceWidgetAction->setText(qSlicerMouseModeToolBar::tr("Place"));
  this->PlaceWidgetAction->setMenu(this->PlaceWidgetMenu);
  this->PlaceWidgetAction->setCheckable(true);

  QObject::connect(this->PlaceWidgetAction, SIGNAL(toggled(bool)),
    q, SLOT(interactionModeActionTriggered(bool)));
  q->addAction(this->PlaceWidgetAction);
  this->InteractionModesActionGroup->addAction(this->PlaceWidgetAction);
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_Q(qSlicerMouseModeToolBar);

  if (newScene == this->MRMLScene)
    {
    return;
    }

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::StartBatchProcessEvent,
                      this, SLOT(onMRMLSceneStartBatchProcess()));

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::EndBatchProcessEvent,
                      this, SLOT(onMRMLSceneEndBatchProcess()));

  this->MRMLScene = newScene;

  // watch for changes to the interaction, selection nodes so can update the widget
  q->setInteractionNode((this->MRMLAppLogic && this->MRMLScene) ? this->MRMLAppLogic->GetInteractionNode() : nullptr);

  vtkMRMLSelectionNode* selectionNode =
    (this->MRMLAppLogic && this->MRMLScene) ?
    this->MRMLAppLogic->GetSelectionNode() : nullptr;
  this->qvtkReconnect(selectionNode, vtkMRMLSelectionNode::ActivePlaceNodeClassNameChangedEvent,
                      this, SLOT(updateWidgetFromMRML()));
  this->qvtkReconnect(selectionNode, vtkMRMLSelectionNode::PlaceNodeClassNameListModifiedEvent,
                      this, SLOT(updateWidgetFromMRML()));

  // Update UI
  q->setEnabled(this->MRMLScene != nullptr);
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
QCursor qSlicerMouseModeToolBarPrivate::cursorFromIcon(QIcon& icon)
{
  QList<QSize> availableSizes = icon.availableSizes();
  if (availableSizes.size() > 0)
    {
    return QCursor(icon.pixmap(availableSizes[0]));
    }
  else
    {
    // use a default
    return QCursor(icon.pixmap(20));
    }
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::updateWidgetFromMRML()
{
  Q_Q(qSlicerMouseModeToolBar);
  vtkMRMLInteractionNode* interactionNode = q->interactionNode();
  if (!interactionNode)
    {
    qDebug() << "Mouse Mode ToolBar: no interaction node";
    q->setEnabled(false);
    return;
    }
  q->setEnabled(true);

  // Find action corresponding to current interaction mode
  int currentInteractionMode = interactionNode->GetCurrentInteractionMode();
  QAction* currentAction = nullptr;
  foreach(QAction* action, this->InteractionModesActionGroup->actions())
    {
    if (action->data().toInt() == currentInteractionMode)
      {
      currentAction = action;
      break;
      }
    }

  // Set action for current interaction mode checked
  if (currentAction)
    {
    currentAction->setChecked(true);
    }
  else
    {
    // uncheck all actions
    QAction* checkedAction = this->InteractionModesActionGroup->checkedAction();
    if (checkedAction)
      {
      checkedAction->setChecked(false);
      }
    }

  // Update place widget action

  this->updatePlaceWidgetMenuActionList();

  // Update persistence checkbox
  int persistence = interactionNode->GetPlaceModePersistence();
  this->PersistenceAction->setChecked(persistence != 0);

  // find the active place node class name and set it's corresponding action to be checked
  QString activePlaceNodeClassName;
  vtkMRMLSelectionNode *selectionNode = (this->MRMLAppLogic ? this->MRMLAppLogic->GetSelectionNode() : nullptr);
  if (selectionNode && selectionNode->GetActivePlaceNodeClassName())
    {
    activePlaceNodeClassName = selectionNode->GetActivePlaceNodeClassName();
    }
  // Update checked state of place actions
  if (activePlaceNodeClassName.isEmpty())
    {
    activePlaceNodeClassName = this->DefaultPlaceClassName;
    }
  foreach(QAction* action, this->PlaceModesActionGroup->actions())
    {
    if (action->data().toString() == activePlaceNodeClassName)
      {
      action->setChecked(true);
      this->PlaceWidgetAction->setIcon(action->icon());
      if (action->text().isEmpty())
        {
        this->PlaceWidgetAction->setText(qSlicerMouseModeToolBar::tr("Place"));
        }
      else
        {
        this->PlaceWidgetAction->setText(qSlicerMouseModeToolBar::tr("Place %1").arg(action->text()));
        }
      break;
      }
    }

  int adjustWindowLevelMode = vtkMRMLWindowLevelWidget::GetAdjustWindowLevelModeFromString(
    interactionNode->GetAttribute(vtkMRMLWindowLevelWidget::GetInteractionNodeAdjustWindowLevelModeAttributeName()));
  switch (adjustWindowLevelMode)
    {
    case vtkMRMLWindowLevelWidget::ModeRectangle:
      this->AdjustWindowLevelRegionModeAction->setChecked(true);
      break;
    case vtkMRMLWindowLevelWidget::ModeRectangleCentered:
      this->AdjustWindowLevelCenteredRegionModeAction->setChecked(true);
      break;
    case vtkMRMLWindowLevelWidget::ModeAdjust:
    default:
      this->AdjustWindowLevelAdjustModeAction->setChecked(true);
      break;
    }

  this->updateCursor();
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::updatePlaceWidgetMenuActionList()
{
  Q_Q(qSlicerMouseModeToolBar);
  vtkMRMLInteractionNode* interactionNode = q->interactionNode();
  if (!interactionNode)
    {
    return;
    }
  vtkMRMLSelectionNode *selectionNode =
    this->MRMLAppLogic ? this->MRMLAppLogic->GetSelectionNode() : nullptr;
  if (!selectionNode)
    {
    return;
    }

  // make sure that all the elements in the selection node have actions in the
  // create and place menu
  const int numClassNames = selectionNode->GetNumberOfPlaceNodeClassNamesInList();

  // if some were removed, clear out those actions first
  QList<QAction*> actionList = this->PlaceModesActionGroup->actions();
  int numActions = actionList.size();
  if (numClassNames < numActions)
    {
    // iterate over the action list and remove ones that aren't in the
    // selection node
    foreach(QAction *action, actionList)
      {
      if (selectionNode->PlaceNodeClassNameInList(
            action->data().toString().toStdString()) == -1)
        {
        // place node type not found, remove corresponding action
        this->PlaceModesActionGroup->removeAction(action);
        this->PlaceWidgetMenu->removeAction(action);
        }
      }
    // update the tool button from the updated action list
    actionList = this->PlaceWidgetMenu->actions();
    }

  // select the active one
  QString activePlace(selectionNode->GetActivePlaceNodeClassName());
  if (activePlace.isEmpty())
    {
    activePlace = this->DefaultPlaceClassName;
    }

  for (int i = 0; i < numClassNames; ++i)
    {
    QString placeNodeClassName = QString(selectionNode->GetPlaceNodeClassNameByIndex(i).c_str());
    QString placeNodeResource = QString(selectionNode->GetPlaceNodeResourceByIndex(i).c_str());
    QString placeNodeIconName = QString(selectionNode->GetPlaceNodeIconNameByIndex(i).c_str());

    QAction* action = q->actionFromPlaceNodeClassName(placeNodeClassName, this->PlaceWidgetMenu);
    if (!action)
      {
      // add it
      action = new QAction(this->PlaceWidgetMenu);
      connect(action, SIGNAL(triggered()),
        q, SLOT(switchPlaceMode()));
      this->PlaceWidgetAction->menu()->addAction(action);
      this->PlaceModesActionGroup->addAction(action);
      }
    // update it
    action->setObjectName(placeNodeClassName);
    action->setIcon(QIcon(placeNodeResource));
    if (action->icon().isNull())
      {
      qCritical() << "qSlicerMouseModeToolBarPrivate::updateWidgetFromSelectionNode - "
                  << "New action icon for class name " << placeNodeClassName << "is null. "
                  << "Resource:" << placeNodeResource;
      }
    action->setText(placeNodeIconName);
    action->setIconText(placeNodeIconName);
    QString tooltip = QString("Use mouse to Create-and-Place ") + placeNodeIconName;
    action->setToolTip(tooltip);
    // save the class name as data on the action
    action->setData(placeNodeClassName);
    action->setCheckable(true);
    }
}


//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::updateCursor()
{
  Q_Q(qSlicerMouseModeToolBar);

  if (!this->MRMLAppLogic)
    {
    qWarning() << "Mouse Mode Tool Bar not set up with application logic";
    return;
    }
  vtkMRMLInteractionNode* interactionNode = q->interactionNode();
  if (!interactionNode)
    {
    return;
    }

  // Use the action's icon as cursor
  // Except when in view mode (then use default cursor) or when in place mode
  // (then place node class is used).
  int currentInteractionMode = interactionNode->GetCurrentInteractionMode();
  if (currentInteractionMode != vtkMRMLInteractionNode::Place)
    {
    if (interactionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::ViewTransform)
      {
      q->changeCursorTo(QCursor());
      }
    else
      {
      // Find action corresponding to current interaction mode
      foreach(QAction* action, this->InteractionModesActionGroup->actions())
        {
        if (action->data().toInt() == currentInteractionMode)
          {
          QIcon icon = action->icon();
          q->changeCursorTo(this->cursorFromIcon(icon));
          break;
          }
        }
      }
    return;
    }

  const char* placeNodeClassName = nullptr;
  vtkMRMLSelectionNode *selectionNode =
    this->MRMLAppLogic ? this->MRMLAppLogic->GetSelectionNode() : nullptr;
  if (selectionNode)
    {
    placeNodeClassName = selectionNode->GetActivePlaceNodeClassName();
    }
  if (!placeNodeClassName)
    {
    q->changeCursorTo(QCursor());
    return;
    }

  // get the actions and check their data for the place node class name
  QList<QAction *> actions = this->PlaceModesActionGroup->actions();
  for (int i = 0; i < actions.size(); ++i)
    {
    QString thisClassName = actions.at(i)->data().toString();
    if (thisClassName.compare(placeNodeClassName) == 0)
      {
      // set this action checked
      actions.at(i)->setChecked(true);
      // update the cursor from the place node resource
      std::string resource = selectionNode->GetPlaceNodeResourceByClassName(std::string(placeNodeClassName));
      if (!resource.empty())
        {
        q->changeCursorTo(QCursor(QPixmap(resource.c_str()),-1,0));
        }
      else
        {
        QIcon icon = actions.at(i)->icon();
        q->changeCursorTo(this->cursorFromIcon(icon));
        }
      break;
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::onMRMLSceneStartBatchProcess()
{
  Q_Q(qSlicerMouseModeToolBar);
  q->setEnabled(false);
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::onMRMLSceneEndBatchProcess()
{
  Q_Q(qSlicerMouseModeToolBar);

  // re-enable in case it didn't get re-enabled for scene load
  q->setEnabled(true);

  q->setInteractionNode((this->MRMLAppLogic && this->MRMLScene) ? this->MRMLAppLogic->GetInteractionNode() : nullptr);

  // update the state from mrml
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::onActivePlaceNodeClassNameChangedEvent()
{
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::onPlaceNodeClassNameListModifiedEvent()
{
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
// qSlicerModuleSelectorToolBar methods

//---------------------------------------------------------------------------
qSlicerMouseModeToolBar::qSlicerMouseModeToolBar(const QString& title, QWidget* parentWidget)
  :Superclass(title, parentWidget)
  , d_ptr(new qSlicerMouseModeToolBarPrivate(*this))
{
  Q_D(qSlicerMouseModeToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qSlicerMouseModeToolBar::qSlicerMouseModeToolBar(QWidget* parentWidget):Superclass(parentWidget)
  , d_ptr(new qSlicerMouseModeToolBarPrivate(*this))
{
  Q_D(qSlicerMouseModeToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qSlicerMouseModeToolBar::~qSlicerMouseModeToolBar() = default;

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBar::setApplicationLogic(vtkSlicerApplicationLogic* appLogic)
{
  Q_D(qSlicerMouseModeToolBar);
  d->MRMLAppLogic = appLogic;
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBar::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qSlicerMouseModeToolBar);
  d->setMRMLScene(newScene);
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBar::switchToViewTransformMode()
{
  Q_D(qSlicerMouseModeToolBar);

  if (!d->MRMLAppLogic)
    {
    qWarning() << "Mouse Mode Tool Bar not set up with application logic";
    return;
    }
  vtkMRMLInteractionNode * intNode = this->interactionNode();
  if (!intNode)
    {
    qWarning() << "Mouse Mode Tool Bar not set up with application logic";
    return;
    }

  // update the interaction node, should trigger a cursor update
  intNode->SwitchToViewTransformMode();
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBar::changeCursorTo(QCursor cursor)
{
  if (!qSlicerApplication::application())
    {
    qWarning() << "changeCursorTo: can't get a qSlicerApplication";
    return;
    }
  qMRMLLayoutManager *layoutManager = qSlicerApplication::application()->layoutManager();

  if (!layoutManager)
    {
    return;
    }

  // Updated all mapped 3D viewers
  for (int i=0; i < layoutManager->threeDViewCount(); ++i)
    {
    qMRMLThreeDView* threeDView = layoutManager->threeDWidget(i)->threeDView();
    if (!threeDView->mrmlViewNode()->IsMappedInLayout())
      {
      continue;
      }
    // Update cursor only if view interaction node corresponds to the one associated with the mouse toolbar
    if (threeDView->mrmlViewNode()->GetInteractionNode() != this->interactionNode())
      {
      continue;
      }
    threeDView->setViewCursor(cursor);
    threeDView->setDefaultViewCursor(cursor);
    }

  // Updated all mapped slicer viewers
  foreach(const QString& viewerName, layoutManager->sliceViewNames())
    {
    qMRMLSliceView* sliceView = layoutManager->sliceWidget(viewerName)->sliceView();
    if (!sliceView->mrmlSliceNode()->IsMappedInLayout())
      {
      continue;
      }
    // Update cursor only if view interaction node corresponds to the one associated with the mouse toolbar
    if (sliceView->mrmlSliceNode()->GetInteractionNode() != this->interactionNode())
      {
      continue;
      }
    sliceView->setViewCursor(cursor);
    sliceView->setDefaultViewCursor(cursor);
    }
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBar::switchPlaceMode()
{
  Q_D(qSlicerMouseModeToolBar);

  if (!d->MRMLAppLogic)
    {
    qWarning() << "Mouse Mode Tool Bar not set up with application logic";
    return;
    }

  // get the currently checked action
  QString placeNodeClassName;
  QAction *thisAction = qobject_cast<QAction*>(sender());
  if (thisAction)
    {
    placeNodeClassName = thisAction->data().toString();
    }
  else
    {
    thisAction = d->PlaceModesActionGroup->checkedAction();
    if (thisAction)
      {
      placeNodeClassName = thisAction->data().toString();
      }
    }
  if (placeNodeClassName.isEmpty())
    {
    placeNodeClassName = this->defaultPlaceClassName();
    }
  // get selection node
  vtkMRMLSelectionNode *selectionNode = d->MRMLAppLogic->GetSelectionNode();
  if ( selectionNode )
    {
    QString previousPlaceNodeClassName = QString(selectionNode->GetActivePlaceNodeClassName());
    selectionNode->SetReferenceActivePlaceNodeClassName(placeNodeClassName.toUtf8());
    // update the interaction mode, which will trigger an update of the cursor
    vtkMRMLInteractionNode * interactionNode = this->interactionNode();
    if (interactionNode)
      {
      // is this a click on top of a single or persistent place mode?
      if (interactionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place &&
          placeNodeClassName.compare(previousPlaceNodeClassName) == 0)
        {
        this->switchToViewTransformMode();
        return;
        }
      if (d->PersistenceAction->isChecked())
        {
        interactionNode->SwitchToPersistentPlaceMode();
        }
      else
        {
        interactionNode->SwitchToSinglePlaceMode();
        }
      }
    else { qCritical() << "qSlicerMouseModeToolBar::switchPlaceMode: unable to get interaction node"; }
    }
  else
    {
    qCritical() << "qSlicerMouseModeToolBar::switchPlaceMode: unable to get selection node";
    }
}

//---------------------------------------------------------------------------
QAction* qSlicerMouseModeToolBar::actionFromPlaceNodeClassName(QString placeNodeClassName, QMenu *menu)
{
  foreach(QAction* action, menu->actions())
    {
    if (action->objectName() == placeNodeClassName)
      {
      return action;
      }
    }
  return nullptr;
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBar::setPersistence(bool persistent)
{
  vtkMRMLInteractionNode *interactionNode = this->interactionNode();
  if (interactionNode)
    {
    interactionNode->SetPlaceModePersistence(persistent ? 1 : 0);
    }
  else
    {
    qWarning() << __FUNCTION__ << ": no interaction node found to toggle.";
    }
}

//---------------------------------------------------------------------------
QString qSlicerMouseModeToolBar::defaultPlaceClassName()const
{
  Q_D(const qSlicerMouseModeToolBar);
  return d->DefaultPlaceClassName;
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBar::setDefaultPlaceClassName(const QString& className)
{
  Q_D(qSlicerMouseModeToolBar);
  d->DefaultPlaceClassName = className;
}

//-----------------------------------------------------------------------------
vtkMRMLInteractionNode* qSlicerMouseModeToolBar::interactionNode()const
{
  Q_D(const qSlicerMouseModeToolBar);
  return d->InteractionNode;
}

//-----------------------------------------------------------------------------
void qSlicerMouseModeToolBar::setInteractionNode(vtkMRMLInteractionNode* interactionNode)
{
  Q_D(qSlicerMouseModeToolBar);
  if (d->InteractionNode == interactionNode)
    {
    return;
    }
  d->qvtkReconnect(d->InteractionNode, interactionNode, vtkCommand::ModifiedEvent,
                   d, SLOT(updateWidgetFromMRML()));
  d->InteractionNode = interactionNode;
  d->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMouseModeToolBar::interactionModeActionTriggered(bool toggled)
{
  Q_D(qSlicerMouseModeToolBar);
  if (!toggled)
    {
    return;
    }
  QAction* sourceAction = qobject_cast<QAction*>(sender());
  if (!sourceAction)
    {
    return;
    }
  int selectedInteractionMode = sourceAction->data().toInt();
  if (!d->InteractionNode)
    {
    return;
    }
  d->InteractionNode->SetCurrentInteractionMode(selectedInteractionMode);

  // If no active place node class name is selected then use the default class
  if (d->InteractionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)
    {
    vtkMRMLSelectionNode* selectionNode = (d->MRMLAppLogic && d->MRMLScene) ?
      d->MRMLAppLogic->GetSelectionNode() : nullptr;
    if (selectionNode)
      {
      const char* currentPlaceNodeClassName = selectionNode->GetActivePlaceNodeClassName();
      if (!currentPlaceNodeClassName || strlen(currentPlaceNodeClassName) == 0)
        {
        selectionNode->SetReferenceActivePlaceNodeClassName(d->DefaultPlaceClassName.toUtf8());
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerMouseModeToolBar::setAdjustWindowLevelMode(int adjustWindowLevelMode)
{
  Q_D(qSlicerMouseModeToolBar);
  vtkMRMLInteractionNode* interactionNode = this->interactionNode();
  if (!interactionNode)
    {
    qDebug() << "setAdjustWindowLevelMode: no interaction node";
    return;
    }
  interactionNode->SetAttribute(vtkMRMLWindowLevelWidget::GetInteractionNodeAdjustWindowLevelModeAttributeName(),
    vtkMRMLWindowLevelWidget::GetAdjustWindowLevelModeAsString(adjustWindowLevelMode));
}

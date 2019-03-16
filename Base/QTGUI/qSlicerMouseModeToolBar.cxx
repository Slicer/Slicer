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

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"
#include "qMRMLSliceView.h"
#include "qMRMLSliceWidget.h"
#include "qSlicerMouseModeToolBar_p.h"

// SlicerLogic includes
#include <vtkSlicerApplicationLogic.h>

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>

//---------------------------------------------------------------------------
// qSlicerMouseModeToolBarPrivate methods

//---------------------------------------------------------------------------
qSlicerMouseModeToolBarPrivate::qSlicerMouseModeToolBarPrivate(qSlicerMouseModeToolBar& object)
  : q_ptr(&object)
{
  this->CreateAndPlaceToolButton = nullptr;
  this->CreateAndPlaceMenu = nullptr;

  this->PersistenceAction = nullptr;

  this->ActionGroup = nullptr;
  this->DefaultPlaceClassName = "vtkMRMLMarkupsFiducialNode";
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::init()
{
  Q_Q(qSlicerMouseModeToolBar);

  this->ActionGroup = new QActionGroup(q);
  this->ActionGroup->setExclusive(true);

  // new actions will be added when interaction modes are registered with the
  // scene

  // persistence
  this->PersistenceAction = new QAction(q);
  this->PersistenceAction->setText(QObject::tr("Persistent"));
  this->PersistenceAction->setToolTip(QObject::tr("Switch between single place and persistent place modes."));
  this->PersistenceAction->setCheckable(true);
  this->PersistenceAction->setChecked(false);
  connect(this->PersistenceAction, SIGNAL(triggered(bool)),
          q, SLOT(setPersistence(bool)));

  // popuplate the create and place menu, with persistence first
  this->CreateAndPlaceMenu = new QMenu(QObject::tr("Create and Place"), q);
  this->CreateAndPlaceMenu->setObjectName("CreateAndPlaceMenu");
  this->CreateAndPlaceMenu->addAction(this->PersistenceAction);
  this->CreateAndPlaceMenu->addSeparator();
  this->CreateAndPlaceMenu->addActions(this->ActionGroup->actions());
  this->CreateAndPlaceMenu->addSeparator();


  this->CreateAndPlaceToolButton = new QToolButton();
  this->CreateAndPlaceToolButton->setObjectName("CreateAndPlaceToolButton");
  this->CreateAndPlaceToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  this->CreateAndPlaceToolButton->setToolTip(QObject::tr("Create and Place"));
  this->CreateAndPlaceToolButton->setText(QObject::tr("Place"));
  this->CreateAndPlaceToolButton->setMenu(this->CreateAndPlaceMenu);
  this->CreateAndPlaceToolButton->setPopupMode(QToolButton::MenuButtonPopup);

  // set default action?


  q->addWidget(this->CreateAndPlaceToolButton);

  QObject::connect(q, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                   this->CreateAndPlaceToolButton,
                   SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));
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
                      this, SLOT(updateWidgetFromSelectionNode()));

  // Update UI
  q->setEnabled(this->MRMLScene != nullptr);
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::updateWidgetFromMRML()
{
  this->updateWidgetFromSelectionNode();
  this->updateWidgetFromInteractionNode();
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::updateWidgetFromSelectionNode()
{
  Q_Q(qSlicerMouseModeToolBar);
  vtkMRMLSelectionNode *selectionNode =
    this->MRMLAppLogic ? this->MRMLAppLogic->GetSelectionNode() : nullptr;
  if (!selectionNode)
    {
    qDebug() << "Mouse Mode ToolBar: no selection node";
    return;
    }

  // make sure that all the elements in the selection node have actions in the
  // create and place menu
  const int numClassNames = selectionNode->GetNumberOfPlaceNodeClassNamesInList();

  // if some were removed, clear out those actions first
  QList<QAction*> actionList = this->CreateAndPlaceMenu->actions();
  int numActions = actionList.size();
  if (numClassNames < numActions)
    {
    // iterate over the action list and remove ones that aren't in the
    // selection node
    for (int i = 0; i < actionList.size(); ++i)
      {
      QAction *action = actionList.at(i);
      QString actionText = action->text();
      // don't remove transform view or persistent or spacers
      if (actionText.compare("&Rotate") != 0 &&
          actionText.compare(QObject::tr("Persistent")) != 0 &&
          !actionText.isEmpty())
        {
        if (selectionNode->PlaceNodeClassNameInList(
              action->data().toString().toStdString()) == -1)
          {
          this->ActionGroup->removeAction(action);
          this->CreateAndPlaceMenu->removeAction(action);
          }
        }
      }
    // update the tool button from the updated action list
    actionList = this->CreateAndPlaceMenu->actions();
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

    QAction* action = q->actionFromPlaceNodeClassName(placeNodeClassName, this->CreateAndPlaceMenu);
    if (!action)
      {
      // add it
      QAction * newAction = new QAction(this->CreateAndPlaceMenu);
      newAction->setObjectName(placeNodeClassName);
      newAction->setIcon(QIcon(placeNodeResource));
      if (newAction->icon().isNull())
        {
        qCritical() << "qSlicerMouseModeToolBarPrivate::updateWidgetFromSelectionNode - "
                    << "New action icon for class name " << placeNodeClassName << "is null. "
                    << "Resource:" << placeNodeResource;
        }
      newAction->setText(placeNodeIconName);
      newAction->setIconText(placeNodeIconName);
      QString tooltip = QString("Use mouse to Create-and-Place ") + placeNodeIconName;
      newAction->setToolTip(tooltip);
      // save the class name as data on the action
      newAction->setData(placeNodeClassName);
      newAction->setCheckable(true);
      connect(newAction, SIGNAL(triggered()),
              q, SLOT(switchPlaceMode()));
      this->CreateAndPlaceToolButton->menu()->addAction(newAction);
      this->ActionGroup->addAction(newAction);
      action = newAction;
      }
    // if this new one is the default
    if (placeNodeClassName == activePlace)
      {
      // make it the default
      if (this->CreateAndPlaceToolButton->defaultAction()!=action)
        {
        this->CreateAndPlaceToolButton->setDefaultAction(action);
        // default action is changed, therefore we need to call
        // updateWidgetFromInteractionNode to make sure the mouse
        // cursor is set to the correct shape
        this->updateWidgetFromInteractionNode();
        }
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::updateWidgetFromInteractionNode()
{
  Q_Q(qSlicerMouseModeToolBar);
  vtkMRMLInteractionNode* interactionNode = q->interactionNode();
  if (!interactionNode)
    {
    qDebug() << "Mouse Mode ToolBar: no interaction node";
    return;
    }

  int persistence = interactionNode->GetPlaceModePersistence();
  this->PersistenceAction->setChecked(persistence != 0);

  int currentMouseMode = interactionNode->GetCurrentInteractionMode();
  switch (currentMouseMode)
    {
    case vtkMRMLInteractionNode::Place:
      {
      // find the active place node class name and set it's corresponding action to be checked
      vtkMRMLSelectionNode *selectionNode = this->MRMLAppLogic->GetSelectionNode();
      if ( selectionNode )
        {
        const char *activePlaceNodeClassName = selectionNode->GetActivePlaceNodeClassName();
        this->updateWidgetToPlace(activePlaceNodeClassName);
        }
      }
      break;
    case vtkMRMLInteractionNode::ViewTransform:
      // reset the widget to view transform, not supporting pick manipulate
      this->updateWidgetToPlace(nullptr);
      break;
    default:
      qWarning() << "qSlicerMouseModeToolBarPrivate::updateWidgetFromMRML - "
                    "unhandled MouseMode:" << currentMouseMode;
      break;
    }
}


//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::updateWidgetToPlace(const char *placeNodeClassName)
{
  Q_Q(qSlicerMouseModeToolBar);

  if (!this->MRMLAppLogic)
    {
    qWarning() << "Mouse Mode Tool Bar not set up with application logic";
    return;
    }

  if (!placeNodeClassName)
    {
    q->changeCursorTo(QCursor());
    q->switchToViewTransformMode();
    }
  else
    {
    // get the actions and check their data for the place node class name
    QList<QAction *> actions = this->ActionGroup->actions();
    for (int i = 0; i < actions.size(); ++i)
      {
      QString thisClassName = actions.at(i)->data().toString();
      if (thisClassName.compare(placeNodeClassName) == 0)
        {
        // set this action checked
        actions.at(i)->setChecked(true);
        // update the cursor from the place node resource
        vtkMRMLSelectionNode *selectionNode = this->MRMLAppLogic->GetSelectionNode();
        if ( selectionNode )
          {
          std::string resource = selectionNode->GetPlaceNodeResourceByClassName(std::string(placeNodeClassName));
          q->changeCursorTo(QCursor(QPixmap(resource.c_str()),-1,0));
          }
        else
          {
          // update from the icon, preserving size
          QList<QSize> availableSizes = actions.at(i)->icon().availableSizes();
          if (availableSizes.size() > 0)
            {
            q->changeCursorTo(QCursor(actions.at(i)->icon().pixmap(availableSizes[0])));
            }
          else
            {
            // use a default
            q->changeCursorTo(QCursor(actions.at(i)->icon().pixmap(20)));
            }
          }
        break;
        }
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
  this->updateWidgetFromSelectionNode();
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::onPlaceNodeClassNameListModifiedEvent()
{
  this->updateWidgetFromSelectionNode();
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
qSlicerMouseModeToolBar::~qSlicerMouseModeToolBar()
= default;

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

  vtkMRMLInteractionNode * interactionNode = this->interactionNode();
  if (interactionNode)
    {
    // update the interaction node, should trigger a cursor update
    interactionNode->SwitchToViewTransformMode();

    // uncheck all
    d->CreateAndPlaceToolButton->setChecked(false);
    QList<QAction*> actionList =  d->CreateAndPlaceMenu->actions();
    int numActions = actionList.size();
    for (int i = 0; i < numActions; i++)
      {
      QAction *action = actionList.at(i);
      QString actionText = action->text();
      if ( actionText.compare(QObject::tr("Persistent")) != 0  &&
          !actionText.isEmpty())
        {
        action->setChecked(false);
        }
      }
    // cancel all Place placements
    interactionNode->InvokeEvent(vtkMRMLInteractionNode::EndPlacementEvent);
    }
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
      return;
      }
    // Update cursor only if view interaction node corresponds to the one associated with the mouse toolbar
    if (threeDView->mrmlViewNode()->GetInteractionNode() != this->interactionNode())
      {
      continue;
      }
    threeDView->setViewCursor(cursor);
    }

  // Updated all mapped slicer viewers
  foreach(const QString& viewerName, layoutManager->sliceViewNames())
    {
    qMRMLSliceView* sliceView = layoutManager->sliceWidget(viewerName)->sliceView();
    if (!sliceView->mrmlSliceNode()->IsMappedInLayout())
      {
      return;
      }
    // Update cursor only if view interaction node corresponds to the one associated with the mouse toolbar
    if (sliceView->mrmlSliceNode()->GetInteractionNode() != this->interactionNode())
      {
      continue;
      }
    sliceView->setCursor(cursor);
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
    sliceView->VTKWidget()->setQVTKCursor(cursor);
#endif
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
  QAction *thisAction = d->CreateAndPlaceToolButton->menu()->activeAction();
  if (thisAction)
    {
    placeNodeClassName = thisAction->data().toString();
    }
  else
    {
    thisAction = d->ActionGroup->checkedAction();
    if (thisAction)
      {
      placeNodeClassName = thisAction->data().toString();
      }
    }
  if (placeNodeClassName.isEmpty())
    {
    qCritical() << "qSlicerMouseModeToolBar::switchPlaceMode: could not get active place node menu item!";
    return;
    }
  // get selection node
  vtkMRMLSelectionNode *selectionNode = d->MRMLAppLogic->GetSelectionNode();
  if ( selectionNode )
    {
    QString previousPlaceNodeClassName = QString(selectionNode->GetActivePlaceNodeClassName());
    selectionNode->SetReferenceActivePlaceNodeClassName(placeNodeClassName.toLatin1());
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
QString qSlicerMouseModeToolBar::activeActionText()
{
  Q_D(qSlicerMouseModeToolBar);

  QString activeActionText;

  QAction *defaultAction = d->CreateAndPlaceToolButton->defaultAction();
  if (defaultAction)
    {
    activeActionText = defaultAction->text();
    }

  return activeActionText;
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
                   d, SLOT(updateWidgetFromInteractionNode()));
  d->InteractionNode = interactionNode;
}

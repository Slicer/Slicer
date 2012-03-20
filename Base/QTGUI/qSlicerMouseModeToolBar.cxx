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
#include "qMRMLSliceWidget.h"
#include "qSlicerMouseModeToolBar_p.h"

// SlicerLogic includes
#include <vtkSlicerApplicationLogic.h>

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSelectionNode.h>

//---------------------------------------------------------------------------
// qSlicerMouseModeToolBarPrivate methods

//---------------------------------------------------------------------------
qSlicerMouseModeToolBarPrivate::qSlicerMouseModeToolBarPrivate(qSlicerMouseModeToolBar& object)
  : q_ptr(&object)
{
  this->CreateAndPlaceToolButton = 0;
  this->CreateAndPlaceMenu = 0;

  this->PersistenceCheckBox = 0;

  this->ViewTransformModeAction = 0;

  this->ActionGroup = 0;
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::init()
{
  Q_Q(qSlicerMouseModeToolBar);

  // RotateMode action
  this->ViewTransformModeAction = new QAction(q);
  this->ViewTransformModeAction->setIcon(QIcon(":/Icons/MouseRotateMode.png"));
  this->ViewTransformModeAction->setText("&Rotate");
  this->ViewTransformModeAction->setToolTip("Set the 3DViewer mouse mode to transform view");
  this->ViewTransformModeAction->setCheckable(true);
  connect(this->ViewTransformModeAction, SIGNAL(triggered()),
          q, SLOT(switchToViewTransformMode()));

  q->addAction(this->ViewTransformModeAction);

  // place once

  this->CreateAndPlaceMenu = new QMenu(QObject::tr("Create and Place"), q);

  this->CreateAndPlaceToolButton = new QToolButton();
  this->CreateAndPlaceToolButton->setCheckable(true);
//  this->CreateAndPlaceToolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  this->CreateAndPlaceToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  this->CreateAndPlaceToolButton->setToolTip(QObject::tr("Create and Place"));
  this->CreateAndPlaceToolButton->setText(QObject::tr("Place"));
  this->CreateAndPlaceToolButton->setMenu(this->CreateAndPlaceMenu);
  this->CreateAndPlaceToolButton->setPopupMode(QToolButton::MenuButtonPopup);
  QObject::connect( this->CreateAndPlaceMenu, SIGNAL(triggered(QAction*)),
                    this->CreateAndPlaceToolButton, SLOT(setDefaultAction(QAction*)));
  q->addWidget(this->CreateAndPlaceToolButton);
  QObject::connect(q, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                   this->CreateAndPlaceToolButton,
                   SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));

  // persistence
  this->PersistenceCheckBox = new QCheckBox(q);
  this->PersistenceCheckBox->setText(QObject::tr("Persistent"));
  this->PersistenceCheckBox->setToolTip(QObject::tr("Switch between single place and persistent place modes."));
  this->PersistenceCheckBox->setChecked(false);
  connect(this->PersistenceCheckBox, SIGNAL(stateChanged(int)), q,
                SLOT(onPersistenceCheckBoxStateChanged(int)));

  q->addWidget(this->PersistenceCheckBox);

  //q->addSeparator();




  QList<QAction*> actionList;
  // actionList << this->SinglePickModeAction <<
  // this->PersistentPickModeAction << this->PersistentPlaceModeAction << this->SinglePlaceModeAction
  actionList << this->ViewTransformModeAction;

  this->ActionGroup = new QActionGroup(q);

  foreach(QAction* action, actionList)
    {
    action->setCheckable(true);
    this->ActionGroup->addAction(action);
    }
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_Q(qSlicerMouseModeToolBar);

  if (newScene == this->MRMLScene)
    {
    return;
    }

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::StartCloseEvent,
                      this, SLOT(OnMRMLSceneStartClose()));

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::EndImportEvent,
                      this, SLOT(OnMRMLSceneEndImport()));

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::EndCloseEvent,
                      this, SLOT(OnMRMLSceneEndClose()));

  this->MRMLScene = newScene;

  // watch for changes to the interaction, selection nodes so can update the widget
  if (this->MRMLScene)
    {
    vtkMRMLInteractionNode * interactionNode = this->MRMLAppLogic->GetInteractionNode();
    if (interactionNode)
      {
      this->qvtkReconnect(interactionNode, vtkMRMLInteractionNode::InteractionModeChangedEvent,
                          this, SLOT(onInteractionNodeModeChangedEvent()));
      this->qvtkReconnect(interactionNode, vtkMRMLInteractionNode::InteractionModePersistenceChangedEvent,
                          this, SLOT(onInteractionNodeModePersistenceChanged()));
      }
    vtkMRMLSelectionNode *selectionNode = this->MRMLAppLogic->GetSelectionNode();
    if (selectionNode)
      {
      this->qvtkReconnect(selectionNode, vtkMRMLSelectionNode::ActiveAnnotationIDChangedEvent,
                          this, SLOT(onActiveAnnotationIDChangedEvent()));
      this->qvtkReconnect(selectionNode, vtkMRMLSelectionNode::AnnotationIDListModifiedEvent,
                          this, SLOT(onAnnotationIDListModifiedEvent()));
      }
    }
  // Update UI
  q->setEnabled(this->MRMLScene != 0);
  if (this->MRMLScene)
    {
    this->updateWidgetFromMRML();
    this->updateWidgetFromSelectionNode();
    }
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::updateWidgetFromMRML()
{
  Q_ASSERT(this->MRMLScene);

  this->onInteractionNodeModePersistenceChanged();

  vtkMRMLInteractionNode * interactionNode = this->MRMLAppLogic->GetInteractionNode();
  Q_ASSERT(interactionNode);

  int currentMouseMode = interactionNode->GetCurrentInteractionMode();
  switch (currentMouseMode)
    {
    case vtkMRMLInteractionNode::Place:
      {
      // find the active annotation id and set it's corresponding action to be checked
      vtkMRMLSelectionNode *selectionNode = this->MRMLAppLogic->GetSelectionNode();
      if ( selectionNode )
        {
        const char *activeAnnotationID = selectionNode->GetActiveAnnotationID();
        this->updateWidgetToAnnotation(activeAnnotationID);
        }
      }
      break;
    case vtkMRMLInteractionNode::ViewTransform:
      // reset the widget to view transform, not supporting pick manipulate
      this->updateWidgetToAnnotation(0);
      break;
    default:
      qWarning() << "qSlicerMouseModeToolBarPrivate::updateWidgetFromMRML - "
                    "unhandled MouseMode:" << currentMouseMode;
      break;
    }
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::updateWidgetFromSelectionNode()
{
  Q_Q(qSlicerMouseModeToolBar);
  Q_ASSERT(this->MRMLScene);
  // get the currently active annotation
  vtkMRMLSelectionNode *selectionNode = this->MRMLAppLogic->GetSelectionNode();
  if (!selectionNode)
    {
    return;
    }
  
  // make sure that all the elements in the selection node have actions in the
  // create and place menu
  int numIDs = selectionNode->GetNumberOfAnnotationIDsInList();

  // if some were removed, clear out those actions first
  QList<QAction*> actionList = this->CreateAndPlaceMenu->actions();
  int numActions = actionList.size();
  if (numIDs < numActions)
    {
    // iterate over the action list and remove ones that aren't in the
    // selection node
    for (int i = 0; i < actionList.size(); ++i)
      {
      QAction *action = actionList.at(i);
      QString actionText = action->text();
      actionText = actionText.prepend(QString("vtkMRMLAnnotation"));
      actionText = actionText.append(QString("Node"));
      const char *thisAnnotID = actionText.toLatin1();
      if (selectionNode->AnnotationIDInList(thisAnnotID) == -1)
        {
        this->ActionGroup->removeAction(action);
        this->CreateAndPlaceMenu->removeAction(action);
        }
      }
    // update the tool button from the updated action list
    actionList = this->CreateAndPlaceMenu->actions();
    if (actionList.size())
      {
      this->CreateAndPlaceToolButton->setDefaultAction(actionList.at(0));
      }
    else
      {
      this->CreateAndPlaceToolButton->setDefaultAction(0);
      this->CreateAndPlaceToolButton->setIcon(QIcon());
      this->CreateAndPlaceToolButton->setText(QString());
      }
    }
//  qDebug() << "\tnumIDs = " << numIDs;
  for (int i = 0; i < numIDs; i++)
    {
    QString annotationID = QString(selectionNode->GetAnnotationIDByIndex(i).c_str());
    QString annotationResource = QString(selectionNode->GetAnnotationResourceByIndex(i).c_str());
    QString annotationName = annotationID;
    annotationName = annotationName.remove(QString("vtkMRMLAnnotation"));
    annotationName = annotationName.remove(QString("Node"));
//    qDebug() << "\t" << i << ", id = " << annotationID << ", resource = " << annotationResource;

    bool inMenu = q->isActionTextInMenu(annotationName, this->CreateAndPlaceMenu);
    if (!inMenu)
      {
      // add it
      QAction * newAction = new QAction(this->CreateAndPlaceMenu);
      newAction->setIcon(QIcon(annotationResource));
      if (newAction->icon().isNull())
        {
        qCritical() << "qSlicerMouseModeToolBarPrivate::updateWidgetFromSelectionNode - "
                    << "New action icon for id " << annotationID << "is null. "
                    << "Resource:" << annotationResource;
        }
      newAction->setText(annotationName);
      newAction->setIconText(annotationName);
      QString tooltip = QString("Use mouse to Create-and-Place ") + annotationName;
      newAction->setToolTip(tooltip);
      // save the annotation id as data on the action
      newAction->setData(annotationID);
      newAction->setCheckable(true);
      connect(newAction, SIGNAL(triggered()),
              q, SLOT(switchPlaceMode()));
      this->CreateAndPlaceToolButton->menu()->addAction(newAction);
      this->ActionGroup->addAction(newAction);
      // is it a fiducial? or the only one? make it the default action
      if (annotationID == QString("vtkMRMLAnnotationFiducialNode") ||
          this->CreateAndPlaceMenu->actions().size() == 1)
        {
        this->CreateAndPlaceToolButton->setDefaultAction(newAction);
        }
//        qDebug() << "qSlicerMouseModeToolBarPrivate::updateWidgetFromSelectionNode - "
//                    "Added action for annotation id " << annotationID << ", "
//                    "name = " << annotationName << ", "
//                    "resource = " << annotationResource;
      }
    }
  // select the active one
  const char *activeAnnotationID = selectionNode->GetActiveAnnotationID();
  this->updateWidgetToAnnotation(activeAnnotationID);
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::updateWidgetToAnnotation(const char *annotationID)
{
  Q_Q(qSlicerMouseModeToolBar);

  if (!annotationID)
    {
    //qDebug() << "qSlicerMouseModeToolBarPrivate::updateWidgetToAnnotation: "
    //            "null active annotation id, resetting to view transform";
    this->ViewTransformModeAction->setChecked(true);
    q->changeCursorTo(QCursor());
    return;
    }
  // get the actions and check their data for the annotation id
  QList<QAction *> actions = this->ActionGroup->actions();
  for (int i = 0; i < actions.size(); ++i)
    {
    QString thisID = actions.at(i)->data().toString();
    if (thisID.compare(annotationID) == 0)
      {
      // set this action checked
      actions.at(i)->setChecked(true);
      //qDebug() << "qSlicerMouseModeToolBarPrivate::updateWidgetToAnnotation - "
      //            "Found active annotation: " << thisID;
      // update the cursor from the annotation id
      vtkMRMLSelectionNode *selectionNode = this->MRMLAppLogic->GetSelectionNode();
      if ( selectionNode )
        {
        std::string resource = selectionNode->GetAnnotationResourceByID(std::string(annotationID));
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

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::OnMRMLSceneStartClose()
{
  Q_Q(qSlicerMouseModeToolBar);
  q->setEnabled(false);
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::OnMRMLSceneEndImport()
{
  Q_Q(qSlicerMouseModeToolBar);

  // re-enable in case it didn't get re-enabled for scene load
  q->setEnabled(true);

  // update the state from mrml
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::OnMRMLSceneEndClose()
{
  Q_Q(qSlicerMouseModeToolBar);
  Q_ASSERT(this->MRMLScene);
  if (!this->MRMLScene || this->MRMLScene->IsBatchProcessing())
    {
    return;
    }
  // reenable it and update
  q->setEnabled(true);
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::onInteractionNodeModeChangedEvent()
{
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::onInteractionNodeModePersistenceChanged()
{
  //this->updateWidgetFromMRML();
  // get the place persistence mode from the interaction node
  Q_ASSERT(this->MRMLScene);
  vtkMRMLInteractionNode * interactionNode = this->MRMLAppLogic->GetInteractionNode();
  Q_ASSERT(interactionNode);
  int persistence = interactionNode->GetPlaceModePersistence();
  if (persistence)
    {
    if (!this->PersistenceCheckBox->isChecked())
      {
      this->PersistenceCheckBox->setChecked(true);
      }
    }
  else
    {
    if (this->PersistenceCheckBox->isChecked())
      {
      this->PersistenceCheckBox->setChecked(false);
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::onActiveAnnotationIDChangedEvent()
{
  //qDebug() << "qSlicerMouseModeToolBarPrivate::onActiveAnnotationIDChangedEvent";
  this->updateWidgetFromSelectionNode();
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::onAnnotationIDListModifiedEvent()
{
  //qDebug() << "qSlicerMouseModeToolBarPrivate::onAnnotationIDListModifiedEvent";
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
{
}

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

  vtkMRMLInteractionNode * interactionNode = d->MRMLAppLogic->GetInteractionNode();
  if (interactionNode)
    {
    //qDebug() << "qSlicerMouseModeToolBar::switchToViewTransformMode";

    interactionNode->SwitchToViewTransformMode();

    // reset cursor to default
    this->changeCursorTo(QCursor());

    d->ViewTransformModeAction->setChecked(true);

    // cancel all Annotation placements
    interactionNode->InvokeEvent(vtkMRMLInteractionNode::EndPlacementEvent);
    }
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBar::changeCursorTo(QCursor cursor)
{
  qMRMLLayoutManager *layoutManager = qSlicerApplication::application()->layoutManager();

  if (!layoutManager)
    {
    return;
    }

  // loop through all existing threeDViews
  for (int i=0; i < layoutManager->threeDViewCount(); ++i)
    {
    layoutManager->threeDWidget(i)->threeDView()->setCursor(cursor);
    }

  // the slice viewers
  qMRMLSliceWidget *greenSliceView = layoutManager->sliceWidget("Green");
  qMRMLSliceWidget *redSliceView = layoutManager->sliceWidget("Red");
  qMRMLSliceWidget *yellowSliceView = layoutManager->sliceWidget("Yellow");

  if (greenSliceView)
    {
    greenSliceView->setCursor(cursor);
    }
  if (redSliceView)
    {
    redSliceView->setCursor(cursor);
    }
  if (yellowSliceView)
    {
    yellowSliceView->setCursor(cursor);
    }

}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBar::switchPlaceMode()
{
  Q_D(qSlicerMouseModeToolBar);

  // get the currently checked action
  QString annotationID;
  QAction *thisAction = d->CreateAndPlaceToolButton->menu()->activeAction();
  if (thisAction)
    {
    annotationID = thisAction->text();
    //qDebug() << "qSlicerMouseModeToolBar::switchPlaceMode: got active action text " << annotationID;
    }
  else
    {
    thisAction = d->ActionGroup->checkedAction();
    if (thisAction)
      {
      annotationID = thisAction->data().toString();
      //qDebug() << "qSlicerMouseModeToolBar::switchPlaceMode: got action group checked action text id = " << annotationID;
      }
    }
  if (annotationID.isEmpty())
    {
    qCritical() << "qSlicerMouseModeToolBar::switchPlaceMode: could not get active annotation menu item!";
    return;
    }
  // get selection node
  vtkMRMLSelectionNode *selectionNode = d->MRMLAppLogic->GetSelectionNode();
  if ( selectionNode )
    {
    selectionNode->SetActiveAnnotationID(annotationID.toLatin1());
    std::string resource = selectionNode->GetAnnotationResourceByID(std::string(annotationID.toLatin1()));
    //qDebug() << "qSlicerMouseModeToolBar::switchPlaceMode: got resource " << resource.c_str();
    // change the cursor
    this->changeCursorTo(QCursor(QPixmap(resource.c_str()),-1,0));

    // update the interaction mode
    vtkMRMLInteractionNode * interactionNode = d->MRMLAppLogic->GetInteractionNode();
    if (interactionNode)
      {
      if (d->PersistenceCheckBox->isChecked())
        {
        //qDebug() << "qSlicerMouseModeToolBar::switchPlaceMode: switching to persistent place mode";
        interactionNode->SwitchToPersistentPlaceMode();
        }
      else
        {
        //qDebug() << "qSlicerMouseModeToolBar::switchPlaceMode: switching to single place mode";
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
bool qSlicerMouseModeToolBar::isActionTextInMenu(QString actionText, QMenu *menu)
{
  // get a list of all the actions currently in the menu
  QList<QAction*> actionList = menu->actions();
  int numActions = actionList.size();
  for (int i = 0; i < numActions; i++)
    {
//    qDebug() << "qSlicerMouseModeToolBar::isActionTextInMenu, testing action " << i
//             << " with text " << actionList[i] << " against text " << actionText;
    if (actionList[i]->text() == actionText)
      {
      return true;
      }
    }
  return false;
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBar::onPersistenceCheckBoxStateChanged(int state)
{
  Q_D(qSlicerMouseModeToolBar);

  vtkMRMLInteractionNode *interactionNode = d->MRMLAppLogic->GetInteractionNode();

  if (interactionNode)
    {
    if (state == Qt::Checked)
      {
      interactionNode->SetPlaceModePersistence(1);
      }
    else if (state == Qt::Unchecked)
      {
      interactionNode->SetPlaceModePersistence(0);
      }
    // ignoring Qt::PartiallyChecked, this checkbox is not tristate
    }
  else
    {
    qWarning() << "qSlicerMouseModeToolBar::onPersistenceCheckBoxStateChanged: "
                  "no interaction node found to set state to " << state;
    }
}

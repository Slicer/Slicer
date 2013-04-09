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
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceNode.h>

//---------------------------------------------------------------------------
// qSlicerMouseModeToolBarPrivate methods

//---------------------------------------------------------------------------
qSlicerMouseModeToolBarPrivate::qSlicerMouseModeToolBarPrivate(qSlicerMouseModeToolBar& object)
  : q_ptr(&object)
{
  this->CreateAndPlaceToolButton = 0;
  this->CreateAndPlaceMenu = 0;

  this->PersistenceAction = 0;

  this->ActionGroup = 0;
  this->DefaultAnnotation = "vtkMRMLAnnotationFiducialNode";
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
  vtkMRMLInteractionNode* interactionNode =
    (this->MRMLAppLogic && this->MRMLScene) ?
    this->MRMLAppLogic->GetInteractionNode() : 0;
  this->qvtkReconnect(interactionNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromInteractionNode()));
  //this->qvtkReconnect(interactionNode, vtkMRMLInteractionNode::InteractionModeChangedEvent,
  //                    this, SLOT(updateWidgetFromInteractionNode()));
  //this->qvtkReconnect(interactionNode, vtkMRMLInteractionNode::InteractionModePersistenceChangedEvent,
  //                    this, SLOT(updateWidgetFromInteractionNode()));

  vtkMRMLSelectionNode* selectionNode =
    (this->MRMLAppLogic && this->MRMLScene) ?
    this->MRMLAppLogic->GetSelectionNode() : 0;
  this->qvtkReconnect(selectionNode, vtkMRMLSelectionNode::ActiveAnnotationIDChangedEvent,
                      this, SLOT(updateWidgetFromSelectionNode()));
  //this->qvtkReconnect(selectionNode, vtkMRMLSelectionNode::ActiveAnnotationIDChangedEvent,
  //                    this, SLOT(onActiveAnnotationIDChangedEvent()));
  //this->qvtkReconnect(selectionNode, vtkMRMLSelectionNode::AnnotationIDListModifiedEvent,
  //                    this, SLOT(onAnnotationIDListModifiedEvent()));

  // Update UI
  q->setEnabled(this->MRMLScene != 0);
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
    this->MRMLAppLogic ? this->MRMLAppLogic->GetSelectionNode() : 0;
  if (!selectionNode)
    {
    qDebug() << "Mouse Mode ToolBar: no selection node";
    return;
    }

  // make sure that all the elements in the selection node have actions in the
  // create and place menu
  const int numIDs = selectionNode->GetNumberOfAnnotationIDsInList();

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
      // don't remove transform view or persistent or spacers
      if (actionText.compare("&Rotate") != 0 &&
          actionText.compare(QObject::tr("Persistent")) != 0 &&
          !actionText.isEmpty())
        {
        if (selectionNode->AnnotationIDInList(
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
//  qDebug() << "\tnumIDs = " << numIDs;

  // select the active one
  QString activeAnnotation(selectionNode->GetActiveAnnotationID());
  if (activeAnnotation.isEmpty())
    {
    activeAnnotation = this->DefaultAnnotation;
    }

  for (int i = 0; i < numIDs; ++i)
    {
    QString annotationID = QString(selectionNode->GetAnnotationIDByIndex(i).c_str());
    QString annotationResource = QString(selectionNode->GetAnnotationResourceByIndex(i).c_str());
    QString annotationName = annotationID;
    if (this->MRMLScene)
      {
      vtkSmartPointer<vtkMRMLNode> annotationNode;
      annotationNode.TakeReference(
        this->MRMLScene->CreateNodeByClass(annotationID.toLatin1()));
      if (annotationNode.GetPointer())
        {
        annotationName = annotationNode->GetNodeTagName();
        }
      }
    annotationName = annotationName.remove(QString("vtk"));
    annotationName = annotationName.remove(QString("MRML"));
    annotationName = annotationName.remove(QString("Annotation"));
    annotationName = annotationName.remove(QString("Node"));

//    qDebug() << "\t" << i << ", id = " << annotationID << ", resource = " << annotationResource;

    QAction* action = q->actionFromText(annotationName, this->CreateAndPlaceMenu);
    if (!action)
      {
      // add it
      QAction * newAction = new QAction(this->CreateAndPlaceMenu);
      newAction->setObjectName(annotationID);
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
      action = newAction;
      }
    // if this new one is the default
    if (annotationID == activeAnnotation)
      {
      // make it the default
      this->CreateAndPlaceToolButton->setDefaultAction(action);
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::updateWidgetFromInteractionNode()
{
  vtkMRMLInteractionNode* interactionNode =
    this->MRMLAppLogic ? this->MRMLAppLogic->GetInteractionNode() : 0;
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
void qSlicerMouseModeToolBarPrivate::updateWidgetToAnnotation(const char *annotationID)
{
  Q_Q(qSlicerMouseModeToolBar);

  if (!this->MRMLAppLogic)
    {
    qWarning() << "Mouse Mode Tool Bar not set up with application logic";
    return;
    }
    
  QAction *checkedAction = 0;
  if (!annotationID)
    {
    //qDebug() << "qSlicerMouseModeToolBarPrivate::updateWidgetToAnnotation: "
    //            "null active annotation id, resetting to view transform";
    q->changeCursorTo(QCursor());
    q->switchToViewTransformMode();
    }
  else
    {
    // get the actions and check their data for the annotation id
    QList<QAction *> actions = this->ActionGroup->actions();
    for (int i = 0; i < actions.size(); ++i)
      {
      QString thisID = actions.at(i)->data().toString();
      if (thisID.compare(annotationID) == 0)
        {
        // set this action checked
        actions.at(i)->setChecked(true);
        checkedAction = actions.at(i);
        //qDebug() << "qSlicerMouseModeToolBarPrivate::updateWidgetToAnnotation - "
        //            "Found active annotation: " << thisID;
        // update the cursor from the annotation id
        vtkMRMLSelectionNode *selectionNode = this->MRMLAppLogic->GetSelectionNode();
        if ( selectionNode )
          {
          std::string resource = selectionNode->GetAnnotationResourceByID(std::string(annotationID));
          //qDebug() << "qSlicerMouseModeToolBarPrivate::updateWidgetToAnnotation - updating cursor from selection node";
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

  // update the state from mrml
  this->updateWidgetFromMRML();
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

  if (!d->MRMLAppLogic)
    {
    qWarning() << "Mouse Mode Tool Bar not set up with application logic";
    return;
    }

  vtkMRMLInteractionNode * interactionNode = d->MRMLAppLogic->GetInteractionNode();
  if (interactionNode)
    {
    //qDebug() << "qSlicerMouseModeToolBar::switchToViewTransformMode";

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
    // cancel all Annotation placements
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
 
  // loop through all existing threeDViews
  for (int i=0; i < layoutManager->threeDViewCount(); ++i)
    {
    layoutManager->threeDWidget(i)->threeDView()->setCursor(cursor);
    }

  // the slice viewers
  vtkMRMLLayoutLogic *layoutLogic = layoutManager->layoutLogic();
  if (!layoutLogic)
    {
    return;
    }
  // the view nodes list is kept up to date with the currently mapped viewers
  vtkCollection *visibleViews = layoutLogic->GetViewNodes();
  // iterate through the view nodes, getting the layout name to get the slice
  // widget
  int numViews = visibleViews->GetNumberOfItems();
  for (int v = 0; v < numViews; v++)
    {
    // item 0 is usually a vtkMRMLViewNode for the 3d window
    vtkMRMLSliceNode *sliceNode = vtkMRMLSliceNode::SafeDownCast(visibleViews->GetItemAsObject(v));
    if (sliceNode)
      {    
      qMRMLSliceWidget *sliceView = layoutManager->sliceWidget(sliceNode->GetName());
      if (sliceView)
        {
        sliceView->setCursor(cursor);
        }
      }
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
  QString annotationID;
  QAction *thisAction = d->CreateAndPlaceToolButton->menu()->activeAction();
  if (thisAction)
    {
    annotationID = thisAction->data().toString();
//    qDebug() << "qSlicerMouseModeToolBar::switchPlaceMode: got active action data " << annotationID;
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
    QString previousAnnotationID = QString(selectionNode->GetActiveAnnotationID());
    //qDebug() << "switchPlaceMode: previous annotation id is " << qPrintable(previousAnnotationID) << ", changing to " <<  qPrintable(annotationID);
    selectionNode->SetReferenceActiveAnnotationID(annotationID.toLatin1());
    // update the interaction mode, which will trigger an update of the cursor
    vtkMRMLInteractionNode * interactionNode = d->MRMLAppLogic->GetInteractionNode();
    if (interactionNode)
      {
      // is this a click on top of a single or persistent place mode?
      ///qDebug() << "switchPlaceMode: interaction node current interaction mode = " << interactionNode->GetCurrentInteractionMode() << ", previous annotation id is " << qPrintable(previousAnnotationID) << ", new annotation id = " << qPrintable(annotationID);
      if (interactionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place &&
          annotationID.compare(previousAnnotationID) == 0)
        {
        //qDebug() << "switchPlaceMode: current interaction mode is place, bail out, switching to view transform mode now";
        this->switchToViewTransformMode();
        return;
        }
      if (d->PersistenceAction->isChecked())
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
QAction* qSlicerMouseModeToolBar::actionFromText(QString actionText, QMenu *menu)
{
  foreach(QAction* action, menu->actions())
    {
    if (action->text() == actionText)
      {
      return action;
      }
    }
  return 0;
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBar::setPersistence(bool persistent)
{
  Q_D(qSlicerMouseModeToolBar);

  vtkMRMLInteractionNode *interactionNode =
    d->MRMLAppLogic ? d->MRMLAppLogic->GetInteractionNode() : 0;

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
QString qSlicerMouseModeToolBar::defaultAnnotation()const
{
  Q_D(const qSlicerMouseModeToolBar);
  return d->DefaultAnnotation;
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBar::setDefaultAnnotation(const QString& annotation)
{
  Q_D(qSlicerMouseModeToolBar);
  d->DefaultAnnotation = annotation;
}

/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QIcon>
#include <QMenu>
#include <QActionGroup>
#include <QCursor>

// CTK includes
#include <ctkLogger.h>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qMRMLLayoutManager.h"
#include "qMRMLThreeDView.h"
#include "qMRMLSliceWidget.h"
#include "qSlicerMouseModeToolBar.h"
#include "qSlicerMouseModeToolBar_p.h"

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSelectionNode.h>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.base.qtgui.qSlicerMouseModeToolBar");
//--------------------------------------------------------------------------

//---------------------------------------------------------------------------
// qSlicerMouseModeToolBarPrivate methods

//---------------------------------------------------------------------------
qSlicerMouseModeToolBarPrivate::qSlicerMouseModeToolBarPrivate(qSlicerMouseModeToolBar& object)
  : q_ptr(&object)
{
  logger.setTrace();

  // MRMLApplicationLogic should be instantiated
  ///Q_ASSERT(qSlicerApplication::application()->mrmlApplicationLogic());
  this->MRMLAppLogic = qSlicerApplication::application() ? qSlicerApplication::application()->mrmlApplicationLogic(): 0;

  this->SinglePickModeAction = 0;
  this->PersistentPickModeAction = 0;
  this->SinglePlaceModeAction = 0;
  this->PersistentPlaceModeAction = 0;
  this->ViewTransformModeAction = 0;

  this->ActionGroup = 0;
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::init()
{
  Q_Q(qSlicerMouseModeToolBar);

  this->SinglePickModeAction = new QAction(q);
  this->SinglePickModeAction->setIcon(QIcon(":/Icons/MouseSinglePickMode.png"));
  this->SinglePickModeAction->setText("&Pick && Move(1)");
  this->SinglePickModeAction->setIconText("Move(1)");
  this->SinglePickModeAction->setToolTip("Use mouse to Pick-and-Manipulate one time.");
  connect(this->SinglePickModeAction, SIGNAL(triggered()),
          q, SLOT(switchToSinglePickMode()));

  this->PersistentPickModeAction = new QAction(q);
  this->PersistentPickModeAction->setIcon(QIcon(":/Icons/MousePickMode.png"));
  this->PersistentPickModeAction->setText("Pick && &Move");
  this->PersistentPickModeAction->setIconText("Move");
  this->PersistentPickModeAction->setToolTip("Use mouse to Pick-and-Manipulate persistently.");
  connect(this->PersistentPickModeAction, SIGNAL(triggered()),
          q, SLOT(switchToPersistentPickMode()));

  this->SinglePlaceModeAction = new QAction(q);
  this->SinglePlaceModeAction->setIcon(QIcon(":/Icons/MouseSinglePlaceMode.png"));
  this->SinglePlaceModeAction->setText("&Create && Place(1)");
  this->SinglePlaceModeAction->setIconText("Place(1)");
  this->SinglePlaceModeAction->setToolTip("Use mouse to Create-and-Place one time.");
  connect(this->SinglePlaceModeAction, SIGNAL(triggered()),
          q, SLOT(switchToSinglePlaceMode()));

  this->PersistentPlaceModeAction = new QAction(q);
  this->PersistentPlaceModeAction->setIcon(QIcon(":/Icons/MousePlaceMode.png"));
  this->PersistentPlaceModeAction->setText("Create && &Place");
  this->PersistentPlaceModeAction->setIconText("Place");
  this->PersistentPlaceModeAction->setToolTip("Use mouse to Create-and-Place persistently.");
  connect(this->PersistentPlaceModeAction, SIGNAL(triggered()),
          q, SLOT(switchToPersistentPlaceMode()));

  // RotateMode action
  this->ViewTransformModeAction = new QAction(q);
  this->ViewTransformModeAction->setIcon(QIcon(":/Icons/MouseRotateMode.png"));
  this->ViewTransformModeAction->setText("&Rotate"); 
  this->ViewTransformModeAction->setToolTip("Set the 3DViewer mouse mode to transform view");
  connect(this->ViewTransformModeAction, SIGNAL(triggered()),
          q, SLOT(switchToViewTransformMode()));

  QList<QAction*> actionList;
  actionList << this->SinglePickModeAction << this->PersistentPickModeAction
      << this->SinglePlaceModeAction << this->PersistentPlaceModeAction
      << this->ViewTransformModeAction;

  this->ActionGroup = new QActionGroup(q);

  foreach(QAction* action, actionList)
    {
    action->setCheckable(true);
    this->ActionGroup->addAction(action);
    q->addAction(action);
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

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::SceneAboutToBeClosedEvent,
                      this, SLOT(onMRMLSceneAboutToBeClosedEvent()));

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::SceneImportedEvent,
                      this, SLOT(onMRMLSceneImportedEvent()));

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::SceneClosedEvent,
                      this, SLOT(onMRMLSceneClosedEvent()));

  this->MRMLScene = newScene;

  // watch for changes to the interaction node so can update the widget
  if (this->MRMLScene)
    {
    vtkMRMLInteractionNode * interactionNode = this->MRMLAppLogic->GetInteractionNode();
    if (interactionNode)
      {
      this->qvtkReconnect(interactionNode, vtkMRMLInteractionNode::InteractionModeChangedEvent,
                          this, SLOT(onInteractionNodeModeChangedEvent()));
      this->qvtkReconnect(interactionNode, vtkMRMLInteractionNode::InteractionModePersistenceChangedEvent,
                          this, SLOT(onInteractionNodeModePersistenceChangedEvent()));
      }
    }
  // Update UI
  q->setEnabled(this->MRMLScene != 0);
  if (this->MRMLScene)
    {
    this->updateWidgetFromMRML();
    }
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::updateWidgetFromMRML()
{
  Q_Q(qSlicerMouseModeToolBar);
  Q_ASSERT(this->MRMLScene);
  vtkMRMLInteractionNode * interactionNode = this->MRMLAppLogic->GetInteractionNode();
  Q_ASSERT(interactionNode);

  int currentMouseMode = interactionNode->GetCurrentInteractionMode();
  switch(currentMouseMode)
    {
    case vtkMRMLInteractionNode::PickManipulate:
      if (interactionNode->GetPickModePersistence())
        {
        q->switchToPersistentPickMode();
        }
      else
        {
        q->switchToSinglePickMode();
        }
      break;
    case vtkMRMLInteractionNode::Place:
      if (interactionNode->GetPlaceModePersistence())
        {
        q->switchToPersistentPlaceMode();
        }
      else
        {
        q->switchToSinglePlaceMode();
        }
      break;
    case vtkMRMLInteractionNode::ViewTransform:
      q->switchToViewTransformMode();
      break;
    default:
      logger.warn(QString("updateWidgetFromMRML - Unknown MouseMode: %1").arg(currentMouseMode));
      break;
    }
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::onMRMLSceneAboutToBeClosedEvent()
{
  Q_Q(qSlicerMouseModeToolBar);
  q->setEnabled(false);
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::onMRMLSceneImportedEvent()
{
  Q_Q(qSlicerMouseModeToolBar);

  // re-enable in case it didn't get re-enabled for scene load
  q->setEnabled(true);
  
  // update the state from mrml
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::onMRMLSceneClosedEvent()
{
  Q_Q(qSlicerMouseModeToolBar);
  Q_ASSERT(this->MRMLScene);
  if (!this->MRMLScene || this->MRMLScene->GetIsUpdating())
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
void qSlicerMouseModeToolBarPrivate::onInteractionNodeModePersistenceChangedEvent()
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
qSlicerMouseModeToolBar::~qSlicerMouseModeToolBar()
{
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBar::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qSlicerMouseModeToolBar);
  d->setMRMLScene(newScene);
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBar::switchToPersistentPickMode()
{
  Q_D(qSlicerMouseModeToolBar);

  vtkMRMLInteractionNode * interactionNode = d->MRMLAppLogic->GetInteractionNode();
  if (interactionNode)
    {
    logger.trace("switchToPersistentPickMode");
    
    interactionNode->SwitchToPersistentPickMode();

    d->PersistentPickModeAction->setChecked(true);
    }
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBar::switchToSinglePickMode()
{
  Q_D(qSlicerMouseModeToolBar);

  vtkMRMLInteractionNode * interactionNode = d->MRMLAppLogic->GetInteractionNode();
  if (interactionNode)
    {
    logger.trace("switchToSinglePickMode");
    
    interactionNode->SwitchToSinglePickMode();

    d->SinglePickModeAction->setChecked(true);
    }
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBar::switchToPersistentPlaceMode()
{
  Q_D(qSlicerMouseModeToolBar);

  // check to see if there's a currently selected annotation node
  vtkMRMLSelectionNode *selectionNode = d->MRMLAppLogic->GetSelectionNode();
  if ( selectionNode )
    {
    if (selectionNode->GetActiveAnnotationID() == NULL ||
        strcmp(selectionNode->GetActiveAnnotationID(), "") == 0)
      {
      // set it to be fiducials
      selectionNode->SetActiveAnnotationID("vtkMRMLAnnotationFiducialNode");
      }
    }
  
  vtkMRMLInteractionNode * interactionNode = d->MRMLAppLogic->GetInteractionNode();
  if (interactionNode)
    {
    logger.trace("switchToPersistentPlaceMode");
    
    interactionNode->SwitchToPersistentPlaceMode();

    // set cursor to persistent place mode, -1,0 means the hot spot is at the top middle
    this->changeCursorTo(QCursor(QPixmap(":/Icons/MousePlaceMode.png"),-1,0));

    d->PersistentPlaceModeAction->setChecked(true);
    }
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBar::switchToSinglePlaceMode()
{
  Q_D(qSlicerMouseModeToolBar);

  // check to see if there's a currently selected annotation node
  vtkMRMLSelectionNode *selectionNode = d->MRMLAppLogic->GetSelectionNode();
  if ( selectionNode )
    {
    if (selectionNode->GetActiveAnnotationID() == NULL ||
        strcmp(selectionNode->GetActiveAnnotationID(), "") == 0)
      {
      // set it to be fiducials
      selectionNode->SetActiveAnnotationID("vtkMRMLAnnotationFiducialNode");
      }
    }
  
  vtkMRMLInteractionNode * interactionNode = d->MRMLAppLogic->GetInteractionNode();
  if (interactionNode)
    {
    logger.trace("switchToSinglePlaceMode");
    
    interactionNode->SwitchToSinglePlaceMode();

    // set cursor to single place mode, -1,0 means the hot spot is at the top middle
    this->changeCursorTo(QCursor(QPixmap(":/Icons/MouseSinglePlaceMode.png"),-1,0));

    d->SinglePlaceModeAction->setChecked(true);
    }
  
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBar::switchToViewTransformMode()
{
  Q_D(qSlicerMouseModeToolBar);

  vtkMRMLInteractionNode * interactionNode = d->MRMLAppLogic->GetInteractionNode();
  if (interactionNode)
    {
    logger.trace("switchToViewTransformMode");
    
    interactionNode->SwitchToViewTransformMode();
    
    // reset cursor to default
    this->changeCursorTo(QCursor());

    d->ViewTransformModeAction->setChecked(true);
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
    layoutManager->threeDView(i)->setCursor(cursor);
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





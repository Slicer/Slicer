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
#include <QMainWindow>

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

  QObject::connect(this->AdjustViewAction, SIGNAL(toggled(bool)), q, SLOT(interactionModeActionTriggered(bool)));
  q->addAction(this->AdjustViewAction);
  this->InteractionModesActionGroup->addAction(this->AdjustViewAction);

  // Window/level mode

  QActionGroup* windowLevelModeActions = new QActionGroup(q);
  windowLevelModeActions->setExclusive(true);

  this->AdjustWindowLevelAdjustModeAction = new QAction(q);
  this->AdjustWindowLevelAdjustModeAction->setText(qSlicerMouseModeToolBar::tr("Adjust"));
  this->AdjustWindowLevelAdjustModeAction->setToolTip(
    qSlicerMouseModeToolBar::tr("Adjust window/level by click-and-drag in a slice viewer."));
  this->AdjustWindowLevelAdjustModeAction->setCheckable(true);

  this->AdjustWindowLevelRegionModeAction = new QAction(q);
  this->AdjustWindowLevelRegionModeAction->setText(qSlicerMouseModeToolBar::tr("Select region"));
  this->AdjustWindowLevelRegionModeAction->setToolTip(
    qSlicerMouseModeToolBar::tr("Set window level based on a rectangular region, specified by click-and-drag in a "
                                "slice viewer. Click position is used as region corner."));
  this->AdjustWindowLevelRegionModeAction->setCheckable(true);

  this->AdjustWindowLevelCenteredRegionModeAction = new QAction(q);
  this->AdjustWindowLevelCenteredRegionModeAction->setText(qSlicerMouseModeToolBar::tr("Select region - centered"));
  this->AdjustWindowLevelCenteredRegionModeAction->setToolTip(
    qSlicerMouseModeToolBar::tr("Set window level based on a rectangular region, specified by click-and-drag in a "
                                "slice viewer. Click position is used as region center."));
  this->AdjustWindowLevelCenteredRegionModeAction->setCheckable(true);

  windowLevelModeActions->addAction(this->AdjustWindowLevelAdjustModeAction);
  windowLevelModeActions->addAction(this->AdjustWindowLevelRegionModeAction);
  windowLevelModeActions->addAction(this->AdjustWindowLevelCenteredRegionModeAction);

  this->AdjustWindowLevelModeMapper = new ctkSignalMapper(q);
  this->AdjustWindowLevelModeMapper->setMapping(this->AdjustWindowLevelAdjustModeAction,
                                                vtkMRMLWindowLevelWidget::ModeAdjust);
  this->AdjustWindowLevelModeMapper->setMapping(this->AdjustWindowLevelRegionModeAction,
                                                vtkMRMLWindowLevelWidget::ModeRectangle);
  this->AdjustWindowLevelModeMapper->setMapping(this->AdjustWindowLevelCenteredRegionModeAction,
                                                vtkMRMLWindowLevelWidget::ModeRectangleCentered);
  QObject::connect(
    windowLevelModeActions, SIGNAL(triggered(QAction*)), this->AdjustWindowLevelModeMapper, SLOT(map(QAction*)));
  QObject::connect(this->AdjustWindowLevelModeMapper, SIGNAL(mapped(int)), q, SLOT(setAdjustWindowLevelMode(int)));

  // Menu
  this->AdjustWindowLevelMenu = new QMenu(qSlicerMouseModeToolBar::tr("Adjust window/level"), q);
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
  // this->AdjustWindowLevelAction->setPopupMode(QToolButton::MenuButtonPopup);

  QObject::connect(this->AdjustWindowLevelAction, SIGNAL(toggled(bool)), q, SLOT(interactionModeActionTriggered(bool)));
  q->addAction(this->AdjustWindowLevelAction);
  this->InteractionModesActionGroup->addAction(this->AdjustWindowLevelAction);

  // Place mode
  this->ToolBarAction = new QAction(this);
  this->ToolBarAction->setObjectName("ToolBarAction");
  this->ToolBarAction->setToolTip(qSlicerMouseModeToolBar::tr("Toggle Markups Toolbar"));
  this->ToolBarAction->setText(qSlicerMouseModeToolBar::tr("Toggle Markups Toolbar"));
  this->ToolBarAction->setEnabled(true);
  this->ToolBarAction->setIcon(QIcon(":/Icons/MarkupsDisplayToolBar.png"));

  QObject::connect(this->ToolBarAction, SIGNAL(triggered()), q, SLOT(toggleMarkupsToolBar()));

  this->PlaceWidgetMenu = new QMenu(qSlicerMouseModeToolBar::tr("Place Menu"), q);
  this->PlaceWidgetMenu->setObjectName("PlaceWidgetMenu");
  this->PlaceWidgetMenu->addAction(this->ToolBarAction);

  this->PlaceWidgetAction = new QAction(this);
  this->PlaceWidgetAction->setObjectName("PlaceWidgetAction");
  this->PlaceWidgetAction->setData(vtkMRMLInteractionNode::Place);
  this->PlaceWidgetAction->setToolTip(qSlicerMouseModeToolBar::tr("Create and Place"));
  this->PlaceWidgetAction->setText(qSlicerMouseModeToolBar::tr("Place"));
  this->PlaceWidgetAction->setCheckable(true);
  this->PlaceWidgetAction->setEnabled(true);
  this->PlaceWidgetAction->setMenu(this->PlaceWidgetMenu);

  connect(this->PlaceWidgetAction, SIGNAL(triggered()), q, SLOT(switchPlaceMode()));
  this->InteractionModesActionGroup->addAction(this->PlaceWidgetAction);
  this->PlaceWidgetAction->setVisible(false);
  q->addAction(this->PlaceWidgetAction);

  q->addAction(this->ToolBarAction); // add Toggle Markups ToolBar action last

  this->PlaceWidgetToolBarAction = new QAction(this);
  this->PlaceWidgetToolBarAction->setObjectName("PlaceWidgetToolBarAction");
  this->PlaceWidgetToolBarAction->setToolTip(qSlicerMouseModeToolBar::tr("Toggle Markups Toolbar"));
  this->PlaceWidgetToolBarAction->setText(qSlicerMouseModeToolBar::tr("Toggle Markups Toolbar"));
  this->PlaceWidgetToolBarAction->setEnabled(true);
  this->PlaceWidgetToolBarAction->setIcon(QIcon(":/Icons/MarkupsDisplayToolBar.png"));

  QObject::connect(this->PlaceWidgetToolBarAction, SIGNAL(triggered()), q, SLOT(toggleMarkupsToolBar()));
  this->PlaceWidgetMenu->addAction(this->PlaceWidgetToolBarAction);
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_Q(qSlicerMouseModeToolBar);

  if (newScene == this->MRMLScene)
  {
    return;
  }

  this->qvtkReconnect(
    this->MRMLScene, newScene, vtkMRMLScene::StartBatchProcessEvent, this, SLOT(onMRMLSceneStartBatchProcess()));

  this->qvtkReconnect(
    this->MRMLScene, newScene, vtkMRMLScene::EndBatchProcessEvent, this, SLOT(onMRMLSceneEndBatchProcess()));

  this->MRMLScene = newScene;

  // watch for changes to the interaction, selection nodes so can update the widget
  q->setInteractionNode((this->MRMLAppLogic && this->MRMLScene) ? this->MRMLAppLogic->GetInteractionNode() : nullptr);

  vtkMRMLSelectionNode* selectionNode =
    (this->MRMLAppLogic && this->MRMLScene) ? this->MRMLAppLogic->GetSelectionNode() : nullptr;
  this->qvtkReconnect(
    selectionNode, vtkMRMLSelectionNode::ActivePlaceNodeClassNameChangedEvent, this, SLOT(updateWidgetFromMRML()));
  this->qvtkReconnect(
    selectionNode, vtkMRMLSelectionNode::PlaceNodeClassNameListModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  this->qvtkReconnect(
    selectionNode, vtkMRMLSelectionNode::ActivePlaceNodeIDChangedEvent, this, SLOT(updateWidgetFromMRML()));
  this->qvtkReconnect(
    selectionNode, vtkMRMLSelectionNode::ActivePlaceNodePlacementValidEvent, this, SLOT(updateWidgetFromMRML()));

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
  foreach (QAction* action, this->InteractionModesActionGroup->actions())
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
  this->updatePlaceWidget();

  // find the active place node class name and set it's corresponding action to be checked
  QString activePlaceNodeClassName;
  vtkMRMLSelectionNode* selectionNode = (this->MRMLAppLogic ? this->MRMLAppLogic->GetSelectionNode() : nullptr);
  if (selectionNode && selectionNode->GetActivePlaceNodeClassName())
  {
    activePlaceNodeClassName = selectionNode->GetActivePlaceNodeClassName();
  }
  // Update checked state of place actions
  if (activePlaceNodeClassName.isEmpty())
  {
    activePlaceNodeClassName = this->DefaultPlaceClassName;
  }

  int adjustWindowLevelMode = vtkMRMLWindowLevelWidget::GetAdjustWindowLevelModeFromString(
    interactionNode->GetAttribute(vtkMRMLWindowLevelWidget::GetInteractionNodeAdjustWindowLevelModeAttributeName()));
  switch (adjustWindowLevelMode)
  {
    case vtkMRMLWindowLevelWidget::ModeRectangle:
    {
      this->AdjustWindowLevelRegionModeAction->setChecked(true);
    }
    break;
    case vtkMRMLWindowLevelWidget::ModeRectangleCentered:
    {
      this->AdjustWindowLevelCenteredRegionModeAction->setChecked(true);
    }
    break;
    case vtkMRMLWindowLevelWidget::ModeAdjust:
    default:
    {
      this->AdjustWindowLevelAdjustModeAction->setChecked(true);
    }
    break;
  }
  this->updateCursor();
}

//---------------------------------------------------------------------------
void qSlicerMouseModeToolBarPrivate::updatePlaceWidget()
{
  Q_Q(qSlicerMouseModeToolBar);
  vtkMRMLInteractionNode* interactionNode = q->interactionNode();
  if (!interactionNode)
  {
    return;
  }
  vtkMRMLSelectionNode* selectionNode = this->MRMLAppLogic ? this->MRMLAppLogic->GetSelectionNode() : nullptr;
  if (!selectionNode)
  {
    return;
  }
  QString activePlaceNodeClassName = selectionNode->GetActivePlaceNodeClassName();
  bool validNodeForPlacement = selectionNode->GetActivePlaceNodePlacementValid();
  if (!validNodeForPlacement || activePlaceNodeClassName.isEmpty())
  {
    this->PlaceWidgetAction->setVisible(false);
    this->ToolBarAction->setVisible(true);
    return;
  }

  QString activePlaceNodeID = selectionNode->GetActivePlaceNodeID();
  if (activePlaceNodeID.isEmpty())
  {
    this->PlaceWidgetAction->setVisible(false);
    this->ToolBarAction->setVisible(true);
    return;
  }

  const int numClassNames = selectionNode->GetNumberOfPlaceNodeClassNamesInList();
  QString placeNodeResource;
  QString placeNodeIconName;
  for (int i = 0; i < numClassNames; ++i)
  {
    if (activePlaceNodeClassName == QString(selectionNode->GetPlaceNodeClassNameByIndex(i).c_str()))
    {
      placeNodeResource = QString(selectionNode->GetPlaceNodeResourceByIndex(i).c_str());
      placeNodeIconName = QString(selectionNode->GetPlaceNodeIconNameByIndex(i).c_str());
      break;
    }
  }
  this->ToolBarAction->setVisible(false);

  QIcon icon(placeNodeResource);
  if (icon.availableSizes().empty())
  {
    qWarning() << "Failed to load icon from resource " << placeNodeResource;
  }
  this->PlaceWidgetAction->setIcon(icon);
  this->PlaceWidgetAction->setText(placeNodeIconName);
  this->PlaceWidgetAction->setData(vtkMRMLInteractionNode::Place);
  QString tooltip = qSlicerMouseModeToolBar::tr("Place a control point");
  this->PlaceWidgetAction->setToolTip(tooltip);
  this->PlaceWidgetAction->setCheckable(true);

  connect(this->PlaceWidgetAction, SIGNAL(triggered()), q, SLOT(switchPlaceMode()));
  this->PlaceWidgetAction->setVisible(true);
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
      foreach (QAction* action, this->InteractionModesActionGroup->actions())
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
  vtkMRMLSelectionNode* selectionNode = this->MRMLAppLogic ? this->MRMLAppLogic->GetSelectionNode() : nullptr;
  if (selectionNode)
  {
    placeNodeClassName = selectionNode->GetActivePlaceNodeClassName();
  }
  if (!placeNodeClassName)
  {
    q->changeCursorTo(QCursor());
    return;
  }

  std::string resource = selectionNode->GetPlaceNodeResourceByClassName(std::string(placeNodeClassName));
  if (!resource.empty())
  {
    q->changeCursorTo(QCursor(QPixmap(resource.c_str()), -1, 0));
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
  : Superclass(title, parentWidget)
  , d_ptr(new qSlicerMouseModeToolBarPrivate(*this))
{
  Q_D(qSlicerMouseModeToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qSlicerMouseModeToolBar::qSlicerMouseModeToolBar(QWidget* parentWidget)
  : Superclass(parentWidget)
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
  vtkMRMLInteractionNode* intNode = this->interactionNode();
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
  qMRMLLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();

  if (!layoutManager)
  {
    return;
  }

  // Updated all mapped 3D viewers
  for (int i = 0; i < layoutManager->threeDViewCount(); ++i)
  {
    qMRMLThreeDView* threeDView = layoutManager->threeDWidget(i)->threeDView();

    // The cursor should be updated in all views, not only ones that are in the current layout.
    // If it is only updated for views that are mapped in the current layout, then the cursor will be incorrect
    // if we switch to a layout with different views.

    // Update cursor only if view interaction node corresponds to the one associated with the mouse toolbar
    if (threeDView->mrmlViewNode()->GetInteractionNode() != this->interactionNode())
    {
      continue;
    }
    threeDView->setViewCursor(cursor);
    threeDView->setDefaultViewCursor(cursor);
  }

  // Updated all mapped slicer viewers
  foreach (const QString& viewerName, layoutManager->sliceViewNames())
  {
    qMRMLSliceView* sliceView = layoutManager->sliceWidget(viewerName)->sliceView();

    // The cursor should be updated in all views, not only ones that are in the current layout.
    // If it is only updated for views that are mapped in the current layout, then the cursor will be incorrect
    // if we switch to a layout with different views.

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
  vtkMRMLInteractionNode* interactionNode = this->interactionNode();
  interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::Place);
}

//---------------------------------------------------------------------------
QAction* qSlicerMouseModeToolBar::actionFromPlaceNodeClassName(QString placeNodeClassName, QMenu* menu)
{
  foreach (QAction* action, menu->actions())
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
  vtkMRMLInteractionNode* interactionNode = this->interactionNode();
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
QString qSlicerMouseModeToolBar::defaultPlaceClassName() const
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
vtkMRMLInteractionNode* qSlicerMouseModeToolBar::interactionNode() const
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
  d->qvtkReconnect(d->InteractionNode, interactionNode, vtkCommand::ModifiedEvent, d, SLOT(updateWidgetFromMRML()));
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
    vtkMRMLSelectionNode* selectionNode =
      (d->MRMLAppLogic && d->MRMLScene) ? d->MRMLAppLogic->GetSelectionNode() : nullptr;
    if (selectionNode)
    {
      const char* currentPlaceNodeClassName = selectionNode->GetActivePlaceNodeClassName();
      if (!currentPlaceNodeClassName || strlen(currentPlaceNodeClassName) == 0)
      {
        selectionNode->SetReferenceActivePlaceNodeClassName(d->DefaultPlaceClassName.toUtf8());
      }
    }
  }
  d->updateWidgetFromMRML();
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

  // Activate window/level action when setting its mode.
  // This is done to save a button click and reduce user confusion, similarly how it is done elsewhere in Slicer
  // and other software, where adjusting an option of a feature activates that feature.
  d->AdjustWindowLevelAction->trigger();
}

//-----------------------------------------------------------------------------
void qSlicerMouseModeToolBar::toggleMarkupsToolBar()
{
  QMainWindow* mainWindow = qSlicerApplication::application()->mainWindow();
  if (mainWindow == nullptr)
  {
    qDebug("qSlicerMouseModeToolBar::toggleMarkupsToolBar: no main window is available, toolbar is not added");
    return;
  }
  foreach (QToolBar* toolBar, mainWindow->findChildren<QToolBar*>())
  {
    if (toolBar->objectName() == QString("MarkupsToolBar"))
    {
      bool visibility = toolBar->isVisible();
      toolBar->setVisible(!visibility);
    }
  }
}

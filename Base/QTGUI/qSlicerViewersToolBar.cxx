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

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qMRMLSliceWidget.h"
#include "qSlicerViewersToolBar_p.h"

// SlicerLogic includes
#include <vtkSlicerApplicationLogic.h>

// MRML includes
#include <vtkMRMLCrosshairNode.h>

//---------------------------------------------------------------------------
// qSlicerViewersToolBarPrivate methods

//---------------------------------------------------------------------------
qSlicerViewersToolBarPrivate::qSlicerViewersToolBarPrivate(qSlicerViewersToolBar& object)
  : q_ptr(&object)
{
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::init()
{
  Q_Q(qSlicerViewersToolBar);

  /// Crosshair
  ///

  // Style
  QActionGroup* crosshairJumpSlicesActions = new QActionGroup(q);
  crosshairJumpSlicesActions->setExclusive(true);

  this->CrosshairJumpSlicesDisabledAction = new QAction(q);
  this->CrosshairJumpSlicesDisabledAction->setText(tr("No jump slices"));
  this->CrosshairJumpSlicesDisabledAction->setToolTip(tr("Slice views are not repositioned when crosshair is moved."));
  this->CrosshairJumpSlicesDisabledAction->setCheckable(true);

  this->CrosshairJumpSlicesOffsetAction = new QAction(q);
  this->CrosshairJumpSlicesOffsetAction->setText(tr("Jump slices - offset"));
  this->CrosshairJumpSlicesOffsetAction->setToolTip(tr("Slice view planes are shifted to match crosshair position (even if crosshair is not displayed)."));
  this->CrosshairJumpSlicesOffsetAction->setCheckable(true);

  this->CrosshairJumpSlicesCenteredAction = new QAction(q);
  this->CrosshairJumpSlicesCenteredAction->setText(tr("Jump slices - centered"));
  this->CrosshairJumpSlicesCenteredAction->setToolTip(tr("Slice views are centered on crosshair position (even if crosshair is not displayed)."));
  this->CrosshairJumpSlicesCenteredAction->setCheckable(true);

  crosshairJumpSlicesActions->addAction(this->CrosshairJumpSlicesDisabledAction);
  crosshairJumpSlicesActions->addAction(this->CrosshairJumpSlicesOffsetAction);
  crosshairJumpSlicesActions->addAction(this->CrosshairJumpSlicesCenteredAction);

  this->CrosshairJumpSlicesMapper = new ctkSignalMapper(q);
  this->CrosshairJumpSlicesMapper->setMapping(this->CrosshairJumpSlicesDisabledAction, vtkMRMLCrosshairNode::NoAction);
  this->CrosshairJumpSlicesMapper->setMapping(this->CrosshairJumpSlicesOffsetAction, vtkMRMLCrosshairNode::OffsetJumpSlice);
  this->CrosshairJumpSlicesMapper->setMapping(this->CrosshairJumpSlicesCenteredAction, vtkMRMLCrosshairNode::CenteredJumpSlice);
  QObject::connect(crosshairJumpSlicesActions, SIGNAL(triggered(QAction*)), this->CrosshairJumpSlicesMapper, SLOT(map(QAction*)));
  QObject::connect(this->CrosshairJumpSlicesMapper, SIGNAL(mapped(int)), this, SLOT(setCrosshairJumpSlicesMode(int)));

  // Style
  QActionGroup* crosshairActions = new QActionGroup(q);
  crosshairActions->setExclusive(true);

  this->CrosshairNoAction = new QAction(q);
  this->CrosshairNoAction->setText(tr("No crosshair"));
  this->CrosshairNoAction->setToolTip(tr("No crosshair displayed."));
  this->CrosshairNoAction->setCheckable(true);

  this->CrosshairBasicAction = new QAction(q);
  this->CrosshairBasicAction->setText(tr("Basic crosshair"));
  this->CrosshairBasicAction->setToolTip(tr("Basic crosshair extending across the field of view with a small gap at the crosshair position."));
  this->CrosshairBasicAction->setCheckable(true);

  this->CrosshairBasicIntersectionAction = new QAction(q);
  this->CrosshairBasicIntersectionAction->setText(tr("Basic + intersection"));
  this->CrosshairBasicIntersectionAction->setToolTip(tr("Basic crosshair extending across the field of view."));
  this->CrosshairBasicIntersectionAction->setCheckable(true);

  this->CrosshairSmallBasicAction = new QAction(q);
  this->CrosshairSmallBasicAction->setText(tr("Small basic crosshair"));
  this->CrosshairSmallBasicAction->setToolTip(tr("Small crosshair with a small gap at the crosshair position."));
  this->CrosshairSmallBasicAction->setCheckable(true);

  this->CrosshairSmallBasicIntersectionAction = new QAction(q);
  this->CrosshairSmallBasicIntersectionAction->setText(tr("Small basic + intersection"));
  this->CrosshairSmallBasicIntersectionAction->setToolTip(tr("Small crosshair."));
  this->CrosshairSmallBasicIntersectionAction->setCheckable(true);

  crosshairActions->addAction(this->CrosshairNoAction);
  crosshairActions->addAction(this->CrosshairBasicAction);
  crosshairActions->addAction(this->CrosshairBasicIntersectionAction);
  crosshairActions->addAction(this->CrosshairSmallBasicAction);
  crosshairActions->addAction(this->CrosshairSmallBasicIntersectionAction);

  this->CrosshairMapper = new ctkSignalMapper(q);
  this->CrosshairMapper->setMapping(this->CrosshairNoAction,
                                    vtkMRMLCrosshairNode::NoCrosshair);
  this->CrosshairMapper->setMapping(this->CrosshairBasicAction,
                                    vtkMRMLCrosshairNode::ShowBasic);
  this->CrosshairMapper->setMapping(this->CrosshairBasicIntersectionAction,
                                    vtkMRMLCrosshairNode::ShowIntersection);
  this->CrosshairMapper->setMapping(this->CrosshairSmallBasicAction,
                                    vtkMRMLCrosshairNode::ShowSmallBasic);
  this->CrosshairMapper->setMapping(this->CrosshairSmallBasicIntersectionAction,
      vtkMRMLCrosshairNode::ShowSmallIntersection);
  QObject::connect(crosshairActions, SIGNAL(triggered(QAction*)),
                   this->CrosshairMapper, SLOT(map(QAction*)));
  QObject::connect(this->CrosshairMapper, SIGNAL(mapped(int)),
                   this, SLOT(setCrosshairMode(int)));

  // Thickness
  QActionGroup* crosshairThicknessActions = new QActionGroup(q);
  crosshairThicknessActions->setExclusive(true);

  this->CrosshairFineAction = new QAction(q);
  this->CrosshairFineAction->setText(tr("Fine crosshair"));
  this->CrosshairFineAction->setToolTip(tr("Fine crosshair."));
  this->CrosshairFineAction->setCheckable(true);

  this->CrosshairMediumAction = new QAction(q);
  this->CrosshairMediumAction->setText(tr("Medium crosshair"));
  this->CrosshairMediumAction->setToolTip(tr("Medium crosshair."));
  this->CrosshairMediumAction->setCheckable(true);

  this->CrosshairThickAction = new QAction(q);
  this->CrosshairThickAction->setText(tr("Thick crosshair"));
  this->CrosshairThickAction->setToolTip(tr("Thick crosshair."));
  this->CrosshairThickAction->setCheckable(true);

  crosshairThicknessActions->addAction(this->CrosshairFineAction);
  crosshairThicknessActions->addAction(this->CrosshairMediumAction);
  crosshairThicknessActions->addAction(this->CrosshairThickAction);
  this->CrosshairThicknessMapper = new ctkSignalMapper(q);
  this->CrosshairThicknessMapper->setMapping(this->CrosshairFineAction,
                                             vtkMRMLCrosshairNode::Fine);
  this->CrosshairThicknessMapper->setMapping(this->CrosshairMediumAction,
                                             vtkMRMLCrosshairNode::Medium);
  this->CrosshairThicknessMapper->setMapping(this->CrosshairThickAction,
                                             vtkMRMLCrosshairNode::Thick);
  QObject::connect(crosshairThicknessActions, SIGNAL(triggered(QAction*)),
                   this->CrosshairThicknessMapper, SLOT(map(QAction*)));
  QObject::connect(this->CrosshairThicknessMapper, SIGNAL(mapped(int)),
                   this, SLOT(setCrosshairThickness(int)));

  // Slice intersections visibility
  this->SliceIntersectionVisibleAction = new QAction(q);
  this->SliceIntersectionVisibleAction->setText(tr("Slice intersections"));
  this->SliceIntersectionVisibleAction->setToolTip(tr("Show how the other slice planes intersect each slice plane."));
  this->SliceIntersectionVisibleAction->setCheckable(true);
  QObject::connect(this->SliceIntersectionVisibleAction, SIGNAL(triggered(bool)),
                   this, SLOT(setSliceIntersectionVisibility(bool)));

  // Interactive slice intersections
  this->SliceInsersectionInteractiveAction = new QAction(q);
  this->SliceInsersectionInteractiveAction->setText(tr("Interaction"));
  this->SliceInsersectionInteractiveAction->setToolTip(tr("Show handles for slice interaction."));
  this->SliceInsersectionInteractiveAction->setCheckable(true);
  this->SliceInsersectionInteractiveAction->setEnabled(false);
  QObject::connect(this->SliceInsersectionInteractiveAction, SIGNAL(triggered(bool)),
    this, SLOT(setSliceIntersectionInteractive(bool)));

  // Interaction options

  this->SliceIntersectionTranslationEnabledAction = new QAction(q);
  this->SliceIntersectionTranslationEnabledAction->setText(tr("Translate"));
  this->SliceIntersectionTranslationEnabledAction->setToolTip(tr("Control visibility of translation handles for slice intersection."));
  this->SliceIntersectionTranslationEnabledAction->setCheckable(true);
  QObject::connect(this->SliceIntersectionTranslationEnabledAction, SIGNAL(triggered(bool)),
    this, SLOT(setSliceIntersectionTranslationEnabled(bool)));

  this->SliceIntersectionRotationEnabledAction = new QAction(q);
  this->SliceIntersectionRotationEnabledAction->setText(tr("Rotate"));
  this->SliceIntersectionRotationEnabledAction->setToolTip(tr("Control visibility of rotation handles for slice intersection."));
  this->SliceIntersectionRotationEnabledAction->setCheckable(true);
  QObject::connect(this->SliceIntersectionRotationEnabledAction, SIGNAL(triggered(bool)),
    this, SLOT(setSliceIntersectionRotationEnabled(bool)));

  this->SliceIntersectionInteractionModesMenu = new QMenu();
  this->SliceIntersectionInteractionModesMenu->setTitle(tr("Interaction options"));
  this->SliceIntersectionInteractionModesMenu->addAction(this->SliceIntersectionTranslationEnabledAction);
  this->SliceIntersectionInteractionModesMenu->addAction(this->SliceIntersectionRotationEnabledAction);

  // Menu
  this->CrosshairMenu = new QMenu(tr("Crosshair"), q);
  this->CrosshairMenu->addActions(crosshairJumpSlicesActions->actions());
  this->CrosshairMenu->addSeparator();
  this->CrosshairMenu->addActions(crosshairActions->actions());
  this->CrosshairMenu->addSeparator();
  this->CrosshairMenu->addActions(crosshairThicknessActions->actions());
  this->CrosshairMenu->addSeparator();
  this->CrosshairMenu->addAction(this->SliceIntersectionVisibleAction);
  this->CrosshairMenu->addAction(this->SliceInsersectionInteractiveAction);
  this->CrosshairMenu->addMenu(this->SliceIntersectionInteractionModesMenu);
  // Add connection to update slice intersection checkboxes before showing the dropdown menu
  QObject::connect(this->CrosshairMenu, SIGNAL(aboutToShow()),
    this, SLOT(updateWidgetFromMRML()));

  this->CrosshairToolButton = new QToolButton();
//  this->CrosshairToolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  this->CrosshairToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  this->CrosshairToolButton->setToolTip(tr("Crosshair"));
  this->CrosshairToolButton->setText(tr("Crosshair"));
  this->CrosshairToolButton->setMenu(this->CrosshairMenu);
  this->CrosshairToolButton->setPopupMode(QToolButton::MenuButtonPopup);

  // Default action
  this->CrosshairToggleAction = new QAction(q);
  this->CrosshairToggleAction->setIcon(QIcon(":/Icons/SlicesCrosshair.png"));
  this->CrosshairToggleAction->setCheckable(true);
  this->CrosshairToggleAction->setToolTip(tr(
    "Toggle crosshair visibility. Hold Shift key and move mouse in a view to set crosshair position."));
  this->CrosshairToggleAction->setText(tr("Crosshair"));
  this->CrosshairToolButton->setDefaultAction(this->CrosshairToggleAction);
  QObject::connect(this->CrosshairToggleAction, SIGNAL(toggled(bool)),
                   this, SLOT(setCrosshairEnabled(bool)));

  q->addWidget(this->CrosshairToolButton);
  QObject::connect(q, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                   this->CrosshairToolButton,
                   SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));

  /// Other controls
  ///
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setCrosshairJumpSlicesMode(int jumpSlicesMode)
{
//  Q_Q(qSlicerViewersToolBar);

  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->MRMLScene->GetNodesByClass("vtkMRMLCrosshairNode"));
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLCrosshairNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLCrosshairNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetCrosshairBehavior(jumpSlicesMode);
    }
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setCrosshairEnabled(bool enabled)
{
//  Q_Q(qSlicerViewersToolBar);

  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->MRMLScene->GetNodesByClass("vtkMRMLCrosshairNode"));
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLCrosshairNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLCrosshairNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    if (enabled)
      {
      node->SetCrosshairMode(this->CrosshairLastMode);
      }
    else
      {
      node->SetCrosshairMode(vtkMRMLCrosshairNode::NoCrosshair);
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setCrosshairMode(int mode)
{
//  Q_Q(qSlicerViewersToolBar);

  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->MRMLScene->GetNodesByClass("vtkMRMLCrosshairNode"));
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLCrosshairNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLCrosshairNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    // cache before carry
    if (mode != vtkMRMLCrosshairNode::NoCrosshair)
      {
      this->CrosshairLastMode = mode;
      }

      node->SetCrosshairMode(mode);
    }
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setCrosshairThickness(int thickness)
{
//  Q_Q(qSlicerViewersToolBar);

  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->MRMLScene->GetNodesByClass("vtkMRMLCrosshairNode"));
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLCrosshairNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLCrosshairNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetCrosshairThickness(thickness);
    }
}

// --------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setSliceIntersectionVisibility(bool visible)
{
  if (!this->MRMLAppLogic)
    {
    qWarning() << Q_FUNC_INFO << " failed: application logic is invalid";
    return;
    }
  this->MRMLAppLogic->SetSliceIntersectionEnabled(vtkMRMLApplicationLogic::SliceIntersectionVisibility, visible);
}

// --------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setSliceIntersectionInteractive(bool visible)
{
  if (!this->MRMLAppLogic)
    {
    qWarning() << Q_FUNC_INFO << " failed: application logic is invalid";
    return;
    }
  this->MRMLAppLogic->SetSliceIntersectionEnabled(vtkMRMLApplicationLogic::SliceIntersectionInteractive, visible);
}

// --------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setSliceIntersectionRotationEnabled(bool visible)
{
  if (!this->MRMLAppLogic)
    {
    qWarning() << Q_FUNC_INFO << " failed: application logic is invalid";
    return;
    }
  this->MRMLAppLogic->SetSliceIntersectionEnabled(vtkMRMLApplicationLogic::SliceIntersectionRotation, visible);
}

// --------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setSliceIntersectionTranslationEnabled(bool visible)
{
  if (!this->MRMLAppLogic)
    {
    qWarning() << Q_FUNC_INFO << " failed: application logic is invalid";
    return;
    }
  this->MRMLAppLogic->SetSliceIntersectionEnabled(vtkMRMLApplicationLogic::SliceIntersectionTranslation, visible);
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_Q(qSlicerViewersToolBar);

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

  if (this->MRMLScene)
    {
    // Watch the crosshairs
    vtkMRMLNode *node;
    vtkCollectionSimpleIterator it;
    vtkSmartPointer<vtkCollection> crosshairs;
    crosshairs.TakeReference(this->MRMLScene->GetNodesByClass("vtkMRMLCrosshairNode"));
    for (crosshairs->InitTraversal(it);
         (node = (vtkMRMLNode*)crosshairs->GetNextItemAsObject(it));)
      {
      vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairNode::SafeDownCast(node);
      if (crosshairNode)
        {
        this->qvtkReconnect(crosshairNode, vtkCommand::ModifiedEvent,
                          this, SLOT(onCrosshairNodeModeChangedEvent()));
        }
      }
    }

  // Update UI
  q->setEnabled(this->MRMLScene != nullptr);
  if (this->MRMLScene)
    {
    this->updateWidgetFromMRML();
    }
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::updateWidgetFromMRML()
{
  Q_ASSERT(this->MRMLScene);

  vtkMRMLNode *node;
  vtkCollectionSimpleIterator it;
  vtkMRMLCrosshairNode* crosshairNode = nullptr;
  vtkSmartPointer<vtkCollection> crosshairs;
  crosshairs.TakeReference(this->MRMLScene->GetNodesByClass("vtkMRMLCrosshairNode"));
  for (crosshairs->InitTraversal(it);
       (node = (vtkMRMLNode*)crosshairs->GetNextItemAsObject(it));)
    {
    crosshairNode = vtkMRMLCrosshairNode::SafeDownCast(node);
    if (crosshairNode  && crosshairNode->GetCrosshairName() == std::string("default"))
      {
      break;
      }
    }
  if (crosshairNode)
    {
    // toggle on/off, jump slices, style of crosshair
    //

    // jump slices
    if (this->CrosshairJumpSlicesMapper->mapping(crosshairNode->GetCrosshairBehavior()) != nullptr)
      {
      QAction* action = (QAction *)(this->CrosshairJumpSlicesMapper->mapping(crosshairNode->GetCrosshairBehavior()));
      if (action)
        {
        action->setChecked(true);
        }
      }

    // style of crosshair
    if (this->CrosshairMapper->mapping(crosshairNode->GetCrosshairMode()) != nullptr)
      {
      QAction* action = (QAction *)(this->CrosshairMapper->mapping(crosshairNode->GetCrosshairMode()));
      if (action)
        {
        action->setChecked(true);
        }
      }

    // thickness
    if (this->CrosshairThicknessMapper->mapping(crosshairNode->GetCrosshairThickness()) != nullptr)
      {
      QAction* action = (QAction *)(this->CrosshairThicknessMapper->mapping(crosshairNode->GetCrosshairThickness()));
      if (action)
        {
        action->setChecked(true);
        }
      }

    // cache the mode
    if (crosshairNode->GetCrosshairMode() != vtkMRMLCrosshairNode::NoCrosshair)
      {
      this->CrosshairLastMode = crosshairNode->GetCrosshairMode();
      }

    // on/off
    // Checking the crosshair button may trigger a crosshair enable/disable action
    // therefore this toggle action should be the last.
    if (this->CrosshairToolButton)
      {
      this->CrosshairToggleAction->setChecked( crosshairNode->GetCrosshairMode() != vtkMRMLCrosshairNode::NoCrosshair );
      }
    }

  if (this->MRMLAppLogic)
    {
    // Slicer intersection visibility
    this->SliceIntersectionVisibleAction->setChecked(
      this->MRMLAppLogic->GetSliceIntersectionEnabled(vtkMRMLApplicationLogic::SliceIntersectionVisibility));
    // Slicer intersection interactive
    this->SliceInsersectionInteractiveAction->setChecked(
      this->MRMLAppLogic->GetSliceIntersectionEnabled(vtkMRMLApplicationLogic::SliceIntersectionInteractive));
    this->SliceInsersectionInteractiveAction->setEnabled(this->SliceIntersectionVisibleAction->isChecked());
    // Interaction options
    this->SliceIntersectionInteractionModesMenu->setEnabled(this->SliceIntersectionVisibleAction->isChecked());
    this->SliceIntersectionTranslationEnabledAction->setChecked(
      this->MRMLAppLogic->GetSliceIntersectionEnabled(vtkMRMLApplicationLogic::SliceIntersectionTranslation));
    this->SliceIntersectionRotationEnabledAction->setChecked(
      this->MRMLAppLogic->GetSliceIntersectionEnabled(vtkMRMLApplicationLogic::SliceIntersectionRotation));
    }
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::OnMRMLSceneStartClose()
{
  Q_Q(qSlicerViewersToolBar);
  q->setEnabled(false);
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::OnMRMLSceneEndImport()
{
  Q_Q(qSlicerViewersToolBar);

  // re-enable in case it didn't get re-enabled for scene load
  q->setEnabled(true);

  // update the state from mrml
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::OnMRMLSceneEndClose()
{
  Q_Q(qSlicerViewersToolBar);
  Q_ASSERT(this->MRMLScene);
  if (!this->MRMLScene || this->MRMLScene->IsBatchProcessing())
    {
    return;
    }
  // re-enable it and update
  q->setEnabled(true);
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::onCrosshairNodeModeChangedEvent()
{
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::onSliceDisplayNodeChangedEvent()
{
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
// qSlicerModuleSelectorToolBar methods

//---------------------------------------------------------------------------
qSlicerViewersToolBar::qSlicerViewersToolBar(const QString& title, QWidget* parentWidget)
  :Superclass(title, parentWidget)
  , d_ptr(new qSlicerViewersToolBarPrivate(*this))
{
  Q_D(qSlicerViewersToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qSlicerViewersToolBar::qSlicerViewersToolBar(QWidget* parentWidget):Superclass(parentWidget)
  , d_ptr(new qSlicerViewersToolBarPrivate(*this))
{
  Q_D(qSlicerViewersToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qSlicerViewersToolBar::~qSlicerViewersToolBar() = default;

//---------------------------------------------------------------------------
void qSlicerViewersToolBar::setApplicationLogic(vtkSlicerApplicationLogic* appLogic)
{
  Q_D(qSlicerViewersToolBar);
  d->MRMLAppLogic = appLogic;
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBar::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qSlicerViewersToolBar);
  d->setMRMLScene(newScene);
}

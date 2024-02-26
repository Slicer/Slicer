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
#include "vtkMRMLSliceLogic.h"
#include <vtkMRMLModelNode.h>
#include <vtkMRMLSliceDisplayNode.h>

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
  this->CrosshairJumpSlicesDisabledAction->setText(qSlicerViewersToolBar::tr("No jump slices"));
  this->CrosshairJumpSlicesDisabledAction->setToolTip(
    qSlicerViewersToolBar::tr("Slice views are not repositioned when crosshair is moved."));
  this->CrosshairJumpSlicesDisabledAction->setCheckable(true);

  this->CrosshairJumpSlicesOffsetAction = new QAction(q);
  this->CrosshairJumpSlicesOffsetAction->setText(qSlicerViewersToolBar::tr("Jump slices - offset"));
  this->CrosshairJumpSlicesOffsetAction->setToolTip(qSlicerViewersToolBar::tr(
    "Slice view planes are shifted to match crosshair position (even if crosshair is not displayed)."));
  this->CrosshairJumpSlicesOffsetAction->setCheckable(true);

  this->CrosshairJumpSlicesCenteredAction = new QAction(q);
  this->CrosshairJumpSlicesCenteredAction->setText(qSlicerViewersToolBar::tr("Jump slices - centered"));
  this->CrosshairJumpSlicesCenteredAction->setToolTip(
    qSlicerViewersToolBar::tr("Slice views are centered on crosshair position (even if crosshair is not displayed)."));
  this->CrosshairJumpSlicesCenteredAction->setCheckable(true);

  crosshairJumpSlicesActions->addAction(this->CrosshairJumpSlicesDisabledAction);
  crosshairJumpSlicesActions->addAction(this->CrosshairJumpSlicesOffsetAction);
  crosshairJumpSlicesActions->addAction(this->CrosshairJumpSlicesCenteredAction);

  this->CrosshairJumpSlicesMapper = new ctkSignalMapper(q);
  this->CrosshairJumpSlicesMapper->setMapping(this->CrosshairJumpSlicesDisabledAction, vtkMRMLCrosshairNode::NoAction);
  this->CrosshairJumpSlicesMapper->setMapping(this->CrosshairJumpSlicesOffsetAction,
                                              vtkMRMLCrosshairNode::OffsetJumpSlice);
  this->CrosshairJumpSlicesMapper->setMapping(this->CrosshairJumpSlicesCenteredAction,
                                              vtkMRMLCrosshairNode::CenteredJumpSlice);
  QObject::connect(
    crosshairJumpSlicesActions, SIGNAL(triggered(QAction*)), this->CrosshairJumpSlicesMapper, SLOT(map(QAction*)));
  QObject::connect(this->CrosshairJumpSlicesMapper, SIGNAL(mapped(int)), this, SLOT(setCrosshairJumpSlicesMode(int)));

  // Crosshair Style
  QActionGroup* crosshairActions = new QActionGroup(q);
  crosshairActions->setExclusive(true);

  this->CrosshairNoAction = new QAction(q);
  this->CrosshairNoAction->setText(qSlicerViewersToolBar::tr("No crosshair"));
  this->CrosshairNoAction->setToolTip(qSlicerViewersToolBar::tr("No crosshair displayed."));
  this->CrosshairNoAction->setCheckable(true);

  this->CrosshairBasicAction = new QAction(q);
  this->CrosshairBasicAction->setText(qSlicerViewersToolBar::tr("Basic crosshair"));
  this->CrosshairBasicAction->setToolTip(qSlicerViewersToolBar::tr(
    "Basic crosshair extending across the field of view with a small gap at the crosshair position."));
  this->CrosshairBasicAction->setCheckable(true);

  this->CrosshairBasicIntersectionAction = new QAction(q);
  this->CrosshairBasicIntersectionAction->setText(qSlicerViewersToolBar::tr("Basic + intersection"));
  this->CrosshairBasicIntersectionAction->setToolTip(
    qSlicerViewersToolBar::tr("Basic crosshair extending across the field of view."));
  this->CrosshairBasicIntersectionAction->setCheckable(true);

  this->CrosshairSmallBasicAction = new QAction(q);
  this->CrosshairSmallBasicAction->setText(qSlicerViewersToolBar::tr("Small basic crosshair"));
  this->CrosshairSmallBasicAction->setToolTip(
    qSlicerViewersToolBar::tr("Small crosshair with a small gap at the crosshair position."));
  this->CrosshairSmallBasicAction->setCheckable(true);

  this->CrosshairSmallBasicIntersectionAction = new QAction(q);
  this->CrosshairSmallBasicIntersectionAction->setText(qSlicerViewersToolBar::tr("Small basic + intersection"));
  this->CrosshairSmallBasicIntersectionAction->setToolTip(qSlicerViewersToolBar::tr("Small crosshair."));
  this->CrosshairSmallBasicIntersectionAction->setCheckable(true);

  crosshairActions->addAction(this->CrosshairNoAction);
  crosshairActions->addAction(this->CrosshairBasicAction);
  crosshairActions->addAction(this->CrosshairBasicIntersectionAction);
  crosshairActions->addAction(this->CrosshairSmallBasicAction);
  crosshairActions->addAction(this->CrosshairSmallBasicIntersectionAction);

  this->CrosshairMapper = new ctkSignalMapper(q);
  this->CrosshairMapper->setMapping(this->CrosshairNoAction, vtkMRMLCrosshairNode::NoCrosshair);
  this->CrosshairMapper->setMapping(this->CrosshairBasicAction, vtkMRMLCrosshairNode::ShowBasic);
  this->CrosshairMapper->setMapping(this->CrosshairBasicIntersectionAction, vtkMRMLCrosshairNode::ShowIntersection);
  this->CrosshairMapper->setMapping(this->CrosshairSmallBasicAction, vtkMRMLCrosshairNode::ShowSmallBasic);
  this->CrosshairMapper->setMapping(this->CrosshairSmallBasicIntersectionAction,
                                    vtkMRMLCrosshairNode::ShowSmallIntersection);
  QObject::connect(crosshairActions, SIGNAL(triggered(QAction*)), this->CrosshairMapper, SLOT(map(QAction*)));
  QObject::connect(this->CrosshairMapper, SIGNAL(mapped(int)), this, SLOT(setCrosshairMode(int)));

  // Crosshair Thickness
  QActionGroup* crosshairThicknessActions = new QActionGroup(q);
  crosshairThicknessActions->setExclusive(true);

  this->CrosshairFineAction = new QAction(q);
  this->CrosshairFineAction->setText(qSlicerViewersToolBar::tr("Fine crosshair"));
  this->CrosshairFineAction->setToolTip(qSlicerViewersToolBar::tr("Fine crosshair."));
  this->CrosshairFineAction->setCheckable(true);

  this->CrosshairMediumAction = new QAction(q);
  this->CrosshairMediumAction->setText(qSlicerViewersToolBar::tr("Medium crosshair"));
  this->CrosshairMediumAction->setToolTip(qSlicerViewersToolBar::tr("Medium crosshair."));
  this->CrosshairMediumAction->setCheckable(true);

  this->CrosshairThickAction = new QAction(q);
  this->CrosshairThickAction->setText(qSlicerViewersToolBar::tr("Thick crosshair"));
  this->CrosshairThickAction->setToolTip(qSlicerViewersToolBar::tr("Thick crosshair."));
  this->CrosshairThickAction->setCheckable(true);

  crosshairThicknessActions->addAction(this->CrosshairFineAction);
  crosshairThicknessActions->addAction(this->CrosshairMediumAction);
  crosshairThicknessActions->addAction(this->CrosshairThickAction);
  this->CrosshairThicknessMapper = new ctkSignalMapper(q);
  this->CrosshairThicknessMapper->setMapping(this->CrosshairFineAction, vtkMRMLCrosshairNode::Fine);
  this->CrosshairThicknessMapper->setMapping(this->CrosshairMediumAction, vtkMRMLCrosshairNode::Medium);
  this->CrosshairThicknessMapper->setMapping(this->CrosshairThickAction, vtkMRMLCrosshairNode::Thick);
  QObject::connect(
    crosshairThicknessActions, SIGNAL(triggered(QAction*)), this->CrosshairThicknessMapper, SLOT(map(QAction*)));
  QObject::connect(this->CrosshairThicknessMapper, SIGNAL(mapped(int)), this, SLOT(setCrosshairThickness(int)));

  // Crosshair Menu
  this->CrosshairMenu = new QMenu(qSlicerViewersToolBar::tr("Crosshair"), q);
  this->CrosshairMenu->addActions(crosshairJumpSlicesActions->actions());
  this->CrosshairMenu->addSeparator();
  this->CrosshairMenu->addActions(crosshairActions->actions());
  this->CrosshairMenu->addSeparator();
  this->CrosshairMenu->addActions(crosshairThicknessActions->actions());

  // Crosshair ToolButton
  this->CrosshairToolButton = new QToolButton();
  this->CrosshairToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  this->CrosshairToolButton->setToolTip(qSlicerViewersToolBar::tr("Crosshair"));
  this->CrosshairToolButton->setText(qSlicerViewersToolBar::tr("Crosshair"));
  this->CrosshairToolButton->setMenu(this->CrosshairMenu);
  this->CrosshairToolButton->setPopupMode(QToolButton::MenuButtonPopup);

  // Default Crosshair action
  this->CrosshairToggleAction = new QAction(q);
  this->CrosshairToggleAction->setIcon(QIcon(":/Icons/SlicesCrosshair.png"));
  this->CrosshairToggleAction->setCheckable(true);
  this->CrosshairToggleAction->setToolTip(qSlicerViewersToolBar::tr(
    "Toggle crosshair visibility. Hold Shift key and move mouse in a view to set crosshair position."));
  this->CrosshairToggleAction->setText(qSlicerViewersToolBar::tr("Crosshair"));
  this->CrosshairToolButton->setDefaultAction(this->CrosshairToggleAction);
  QObject::connect(this->CrosshairToggleAction, SIGNAL(toggled(bool)), this, SLOT(setCrosshairEnabled(bool)));

  q->addWidget(this->CrosshairToolButton);
  QObject::connect(q,
                   SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                   this->CrosshairToolButton,
                   SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));

  // Slice Intersections Style
  QActionGroup* sliceIntersectionsActions = new QActionGroup(q);
  sliceIntersectionsActions->setExclusive(true);

  this->SliceIntersectionsFullIntersectionAction = new QAction(q);
  this->SliceIntersectionsFullIntersectionAction->setText(qSlicerViewersToolBar::tr("Full lines"));
  this->SliceIntersectionsFullIntersectionAction->setToolTip(
    qSlicerViewersToolBar::tr("Full slice intersection lines extending across the field of view."));
  this->SliceIntersectionsFullIntersectionAction->setCheckable(true);
  this->SliceIntersectionsFullIntersectionAction->setChecked(true);

  this->SliceIntersectionsSkipIntersectionAction = new QAction(q);
  this->SliceIntersectionsSkipIntersectionAction->setText(qSlicerViewersToolBar::tr("Skip line crossings"));
  this->SliceIntersectionsSkipIntersectionAction->setToolTip(qSlicerViewersToolBar::tr(
    "Slice intersection lines extending across the field of view with a gap at the intersection."));
  this->SliceIntersectionsSkipIntersectionAction->setCheckable(true);

  sliceIntersectionsActions->addAction(this->SliceIntersectionsFullIntersectionAction);
  sliceIntersectionsActions->addAction(this->SliceIntersectionsSkipIntersectionAction);

  this->SliceIntersectionsMapper = new ctkSignalMapper(q);
  this->SliceIntersectionsMapper->setMapping(this->SliceIntersectionsSkipIntersectionAction,
                                             vtkMRMLSliceDisplayNode::SkipLineCrossings);
  this->SliceIntersectionsMapper->setMapping(this->SliceIntersectionsFullIntersectionAction,
                                             vtkMRMLSliceDisplayNode::FullLines);
  QObject::connect(
    sliceIntersectionsActions, SIGNAL(triggered(QAction*)), this->SliceIntersectionsMapper, SLOT(map(QAction*)));
  QObject::connect(
    this->SliceIntersectionsMapper, SIGNAL(mapped(int)), this, SLOT(setIntersectingSlicesIntersectionMode(int)));

  // Slice Intersections Thickness
  QActionGroup* sliceIntersectionsThicknessActions = new QActionGroup(q);
  sliceIntersectionsThicknessActions->setExclusive(true);

  this->SliceIntersectionsFineAction = new QAction(q);
  this->SliceIntersectionsFineAction->setText(qSlicerViewersToolBar::tr("Fine lines"));
  this->SliceIntersectionsFineAction->setToolTip(qSlicerViewersToolBar::tr("Fine lines."));
  this->SliceIntersectionsFineAction->setCheckable(true);
  this->SliceIntersectionsFineAction->setChecked(true);

  this->SliceIntersectionsMediumAction = new QAction(q);
  this->SliceIntersectionsMediumAction->setText(qSlicerViewersToolBar::tr("Medium lines"));
  this->SliceIntersectionsMediumAction->setToolTip(qSlicerViewersToolBar::tr("Medium lines."));
  this->SliceIntersectionsMediumAction->setCheckable(true);

  this->SliceIntersectionsThickAction = new QAction(q);
  this->SliceIntersectionsThickAction->setText(qSlicerViewersToolBar::tr("Thick lines"));
  this->SliceIntersectionsThickAction->setToolTip(qSlicerViewersToolBar::tr("Thick lines."));
  this->SliceIntersectionsThickAction->setCheckable(true);

  sliceIntersectionsThicknessActions->addAction(this->SliceIntersectionsFineAction);
  sliceIntersectionsThicknessActions->addAction(this->SliceIntersectionsMediumAction);
  sliceIntersectionsThicknessActions->addAction(this->SliceIntersectionsThickAction);
  this->SliceIntersectionsThicknessMapper = new ctkSignalMapper(q);
  this->SliceIntersectionsThicknessMapper->setMapping(this->SliceIntersectionsFineAction,
                                                      vtkMRMLSliceDisplayNode::FineLines);
  this->SliceIntersectionsThicknessMapper->setMapping(this->SliceIntersectionsMediumAction,
                                                      vtkMRMLSliceDisplayNode::MediumLines);
  this->SliceIntersectionsThicknessMapper->setMapping(this->SliceIntersectionsThickAction,
                                                      vtkMRMLSliceDisplayNode::ThickLines);
  QObject::connect(sliceIntersectionsThicknessActions,
                   SIGNAL(triggered(QAction*)),
                   this->SliceIntersectionsThicknessMapper,
                   SLOT(map(QAction*)));
  QObject::connect(this->SliceIntersectionsThicknessMapper,
                   SIGNAL(mapped(int)),
                   this,
                   SLOT(setIntersectingSlicesLineThicknessMode(int)));

  // Interactive slice intersections
  this->IntersectingSlicesInteractiveAction = new QAction(q);
  this->IntersectingSlicesInteractiveAction->setText(qSlicerViewersToolBar::tr("Interaction"));
  this->IntersectingSlicesInteractiveAction->setToolTip(
    qSlicerViewersToolBar::tr("Show handles for slice interaction."));
  this->IntersectingSlicesInteractiveAction->setCheckable(true);
  QObject::connect(this->IntersectingSlicesInteractiveAction,
                   SIGNAL(triggered(bool)),
                   this,
                   SLOT(setIntersectingSlicesInteractive(bool)));

  // Interaction options
  this->IntersectingSlicesTranslationEnabledAction = new QAction(q);
  this->IntersectingSlicesTranslationEnabledAction->setText(qSlicerViewersToolBar::tr("Translate"));
  this->IntersectingSlicesTranslationEnabledAction->setToolTip(
    qSlicerViewersToolBar::tr("Control visibility of translation handles for slice intersection."));
  this->IntersectingSlicesTranslationEnabledAction->setCheckable(true);
  QObject::connect(this->IntersectingSlicesTranslationEnabledAction,
                   SIGNAL(triggered(bool)),
                   this,
                   SLOT(setIntersectingSlicesTranslationEnabled(bool)));

  this->IntersectingSlicesRotationEnabledAction = new QAction(q);
  this->IntersectingSlicesRotationEnabledAction->setText(qSlicerViewersToolBar::tr("Rotate"));
  this->IntersectingSlicesRotationEnabledAction->setToolTip(
    qSlicerViewersToolBar::tr("Control visibility of rotation handles for slice intersection."));
  this->IntersectingSlicesRotationEnabledAction->setCheckable(true);
  QObject::connect(this->IntersectingSlicesRotationEnabledAction,
                   SIGNAL(triggered(bool)),
                   this,
                   SLOT(setIntersectingSlicesRotationEnabled(bool)));

  this->IntersectingSlicesInteractionModesMenu = new QMenu();
  this->IntersectingSlicesInteractionModesMenu->setTitle(qSlicerViewersToolBar::tr("Interaction options"));
  this->IntersectingSlicesInteractionModesMenu->addAction(this->IntersectingSlicesTranslationEnabledAction);
  this->IntersectingSlicesInteractionModesMenu->addAction(this->IntersectingSlicesRotationEnabledAction);

  // Slice Intersections Menu
  this->SliceIntersectionsMenu = new QMenu(qSlicerViewersToolBar::tr("Slice intersections"), q);
  this->SliceIntersectionsMenu->addAction(this->IntersectingSlicesInteractiveAction);
  this->SliceIntersectionsMenu->addMenu(this->IntersectingSlicesInteractionModesMenu);
  this->SliceIntersectionsMenu->addSeparator();
  this->SliceIntersectionsMenu->addActions(sliceIntersectionsActions->actions());
  this->SliceIntersectionsMenu->addSeparator();
  this->SliceIntersectionsMenu->addActions(sliceIntersectionsThicknessActions->actions());

  // Add connection to update slice intersection checkboxes before showing the dropdown menu
  QObject::connect(this->SliceIntersectionsMenu, SIGNAL(aboutToShow()), this, SLOT(updateWidgetFromMRML()));

  // Slice Intersections ToolButton
  this->SliceIntersectionsToolButton = new QToolButton();
  this->SliceIntersectionsToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  this->SliceIntersectionsToolButton->setToolTip(qSlicerViewersToolBar::tr("Slice intersections"));
  this->SliceIntersectionsToolButton->setText(qSlicerViewersToolBar::tr("Slice intersections"));
  this->SliceIntersectionsToolButton->setMenu(this->SliceIntersectionsMenu);
  this->SliceIntersectionsToolButton->setPopupMode(QToolButton::MenuButtonPopup);

  // Default Slice intersections action
  this->IntersectingSlicesVisibleAction = new QAction(q);
  this->IntersectingSlicesVisibleAction->setIcon(QIcon(":/Icons/SliceIntersections.png"));
  this->IntersectingSlicesVisibleAction->setText(qSlicerViewersToolBar::tr("Slice intersections"));
  this->IntersectingSlicesVisibleAction->setToolTip(
    qSlicerViewersToolBar::tr("Show how the other slice planes intersect each slice plane."));
  this->IntersectingSlicesVisibleAction->setCheckable(true);
  this->SliceIntersectionsToolButton->setDefaultAction(this->IntersectingSlicesVisibleAction);
  QObject::connect(
    this->IntersectingSlicesVisibleAction, SIGNAL(triggered(bool)), this, SLOT(setIntersectingSlicesVisibility(bool)));

  q->addWidget(this->SliceIntersectionsToolButton);
  QObject::connect(q,
                   SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                   this->SliceIntersectionsToolButton,
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
  for (nodes->InitTraversal(it); (node = static_cast<vtkMRMLCrosshairNode*>(nodes->GetNextItemAsObject(it)));)
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
  for (nodes->InitTraversal(it); (node = static_cast<vtkMRMLCrosshairNode*>(nodes->GetNextItemAsObject(it)));)
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
  for (nodes->InitTraversal(it); (node = static_cast<vtkMRMLCrosshairNode*>(nodes->GetNextItemAsObject(it)));)
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
  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->MRMLScene->GetNodesByClass("vtkMRMLCrosshairNode"));
  if (!nodes.GetPointer())
  {
    return;
  }
  vtkMRMLCrosshairNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it); (node = static_cast<vtkMRMLCrosshairNode*>(nodes->GetNextItemAsObject(it)));)
  {
    node->SetCrosshairThickness(thickness);
  }
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setIntersectingSlicesLineThicknessMode(int mode)
{
  if (!this->MRMLAppLogic)
  {
    qWarning() << Q_FUNC_INFO << " failed: application logic is invalid";
    return;
  }
  this->MRMLAppLogic->SetIntersectingSlicesLineThicknessMode(mode);
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setIntersectingSlicesIntersectionMode(int mode)
{
  if (!this->MRMLAppLogic)
  {
    qWarning() << Q_FUNC_INFO << " failed: application logic is invalid";
    return;
  }
  this->MRMLAppLogic->SetIntersectingSlicesIntersectionMode(mode);
}

// --------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setIntersectingSlicesVisibility(bool visible)
{
  if (!this->MRMLAppLogic)
  {
    qWarning() << Q_FUNC_INFO << " failed: application logic is invalid";
    return;
  }
  this->MRMLAppLogic->SetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesVisibility, visible);
}

// --------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setIntersectingSlicesInteractive(bool visible)
{
  if (!this->MRMLAppLogic)
  {
    qWarning() << Q_FUNC_INFO << " failed: application logic is invalid";
    return;
  }
  this->MRMLAppLogic->SetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesInteractive, visible);
}

// --------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setIntersectingSlicesRotationEnabled(bool visible)
{
  if (!this->MRMLAppLogic)
  {
    qWarning() << Q_FUNC_INFO << " failed: application logic is invalid";
    return;
  }
  this->MRMLAppLogic->SetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesRotation, visible);
}

// --------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setIntersectingSlicesTranslationEnabled(bool visible)
{
  if (!this->MRMLAppLogic)
  {
    qWarning() << Q_FUNC_INFO << " failed: application logic is invalid";
    return;
  }
  this->MRMLAppLogic->SetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesTranslation, visible);
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_Q(qSlicerViewersToolBar);

  if (newScene == this->MRMLScene)
  {
    return;
  }

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::StartCloseEvent, this, SLOT(OnMRMLSceneStartClose()));

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::EndImportEvent, this, SLOT(OnMRMLSceneEndImport()));

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::EndCloseEvent, this, SLOT(OnMRMLSceneEndClose()));

  this->MRMLScene = newScene;

  if (this->MRMLScene)
  {
    // Watch the crosshairs
    vtkMRMLNode* node;
    vtkCollectionSimpleIterator it;
    vtkSmartPointer<vtkCollection> crosshairs;
    crosshairs.TakeReference(this->MRMLScene->GetNodesByClass("vtkMRMLCrosshairNode"));
    for (crosshairs->InitTraversal(it); (node = (vtkMRMLNode*)crosshairs->GetNextItemAsObject(it));)
    {
      vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairNode::SafeDownCast(node);
      if (crosshairNode)
      {
        this->qvtkReconnect(crosshairNode, vtkCommand::ModifiedEvent, this, SLOT(onCrosshairNodeModeChangedEvent()));
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

  vtkMRMLNode* node;
  vtkCollectionSimpleIterator it;
  vtkMRMLCrosshairNode* crosshairNode = nullptr;
  vtkSmartPointer<vtkCollection> crosshairs;
  crosshairs.TakeReference(this->MRMLScene->GetNodesByClass("vtkMRMLCrosshairNode"));
  for (crosshairs->InitTraversal(it); (node = (vtkMRMLNode*)crosshairs->GetNextItemAsObject(it));)
  {
    crosshairNode = vtkMRMLCrosshairNode::SafeDownCast(node);
    if (crosshairNode && crosshairNode->GetCrosshairName() == std::string("default"))
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
      QAction* action = (QAction*)(this->CrosshairJumpSlicesMapper->mapping(crosshairNode->GetCrosshairBehavior()));
      if (action)
      {
        action->setChecked(true);
      }
    }

    // style of crosshair
    if (this->CrosshairMapper->mapping(crosshairNode->GetCrosshairMode()) != nullptr)
    {
      QAction* action = (QAction*)(this->CrosshairMapper->mapping(crosshairNode->GetCrosshairMode()));
      if (action)
      {
        action->setChecked(true);
      }
    }

    // thickness
    if (this->CrosshairThicknessMapper->mapping(crosshairNode->GetCrosshairThickness()) != nullptr)
    {
      QAction* action = (QAction*)(this->CrosshairThicknessMapper->mapping(crosshairNode->GetCrosshairThickness()));
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
      this->CrosshairToggleAction->setChecked(crosshairNode->GetCrosshairMode() != vtkMRMLCrosshairNode::NoCrosshair);
    }
  }

  if (this->MRMLAppLogic)
  {
    // Slicer intersection visibility
    this->IntersectingSlicesVisibleAction->setChecked(
      this->MRMLAppLogic->GetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesVisibility));
    // Slicer intersection interactive
    this->IntersectingSlicesInteractiveAction->setChecked(
      this->MRMLAppLogic->GetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesInteractive));
    this->SliceIntersectionsFullIntersectionAction->setEnabled(this->IntersectingSlicesInteractiveAction->isChecked());
    this->SliceIntersectionsSkipIntersectionAction->setEnabled(this->IntersectingSlicesInteractiveAction->isChecked());
    this->SliceIntersectionsFineAction->setEnabled(this->IntersectingSlicesInteractiveAction->isChecked());
    this->SliceIntersectionsMediumAction->setEnabled(this->IntersectingSlicesInteractiveAction->isChecked());
    this->SliceIntersectionsThickAction->setEnabled(this->IntersectingSlicesInteractiveAction->isChecked());
    // Interaction options
    this->IntersectingSlicesInteractionModesMenu->setEnabled(this->IntersectingSlicesVisibleAction->isChecked());
    this->IntersectingSlicesTranslationEnabledAction->setChecked(
      this->MRMLAppLogic->GetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesTranslation));
    this->IntersectingSlicesRotationEnabledAction->setChecked(
      this->MRMLAppLogic->GetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesRotation));
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
  : Superclass(title, parentWidget)
  , d_ptr(new qSlicerViewersToolBarPrivate(*this))
{
  Q_D(qSlicerViewersToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qSlicerViewersToolBar::qSlicerViewersToolBar(QWidget* parentWidget)
  : Superclass(parentWidget)
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

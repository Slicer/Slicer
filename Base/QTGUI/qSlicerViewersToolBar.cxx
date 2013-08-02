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
#include <QToolButton>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qMRMLSliceWidget.h"
#include "qSlicerViewersToolBar_p.h"

// SlicerLogic includes
#include <vtkSlicerApplicationLogic.h>

// MRML includes
#include <vtkMRMLCrosshairNode.h>
#include <vtkMRMLSliceCompositeNode.h>

//---------------------------------------------------------------------------
// qSlicerViewersToolBarPrivate methods

//---------------------------------------------------------------------------
qSlicerViewersToolBarPrivate::qSlicerViewersToolBarPrivate(qSlicerViewersToolBar& object)
  : q_ptr(&object)
{
  this->CrosshairToolButton = 0;
  this->CrosshairMenu = 0;
  this->CrosshairNavigationAction = 0;

  this->CrosshairMapper = 0;
  this->CrosshairNoAction = 0;
  this->CrosshairBasicAction = 0;
  this->CrosshairBasicIntersectionAction = 0;
  this->CrosshairSmallBasicAction = 0;
  this->CrosshairSmallBasicIntersectionAction = 0;

  this->CrosshairThicknessMapper = 0;
  this->CrosshairFineAction = 0;
  this->CrosshairMediumAction = 0;
  this->CrosshairThickAction = 0;

  this->CrosshairSliceIntersectionsAction = 0;

  this->CrosshairToggleAction = 0;

  this->CrosshairLastMode = vtkMRMLCrosshairNode::ShowBasic;
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::init()
{
  Q_Q(qSlicerViewersToolBar);

  /// Crosshair
  ///

  // Navigation/Cross-referencing
  this->CrosshairNavigationAction = new QAction(q);
  this->CrosshairNavigationAction->setText(tr("Navigation"));
  this->CrosshairNavigationAction->setToolTip(tr("Toggle between crosshair navigation and cross-referencing"));
  this->CrosshairNavigationAction->setCheckable(true);
  QObject::connect(this->CrosshairNavigationAction, SIGNAL(triggered(bool)),
                   this, SLOT(setNavigation(bool)));

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
  this->CrosshairMapper->setMapping(this->CrosshairSmallBasicIntersectionAction,                                     vtkMRMLCrosshairNode::ShowSmallIntersection);
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

  // Slice intersections
  this->CrosshairSliceIntersectionsAction = new QAction(q);
  this->CrosshairSliceIntersectionsAction->setText(tr("Slice intersections"));
  this->CrosshairSliceIntersectionsAction->setToolTip(tr("Show how the other slice planes intersect each slice plane."));
  this->CrosshairSliceIntersectionsAction->setCheckable(true);
  QObject::connect(this->CrosshairSliceIntersectionsAction, SIGNAL(triggered(bool)),
                   this, SLOT(setSliceIntersectionVisible(bool)));

  // Menu
  this->CrosshairMenu = new QMenu(QObject::tr("Crosshair"), q);
  this->CrosshairMenu->addAction(this->CrosshairNavigationAction);
  this->CrosshairMenu->addSeparator();
  this->CrosshairMenu->addActions(crosshairActions->actions());
  this->CrosshairMenu->addSeparator();
  this->CrosshairMenu->addActions(crosshairThicknessActions->actions());
  this->CrosshairMenu->addSeparator();
  this->CrosshairMenu->addAction(this->CrosshairSliceIntersectionsAction);


  this->CrosshairToolButton = new QToolButton();
//  this->CrosshairToolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  this->CrosshairToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  this->CrosshairToolButton->setToolTip(QObject::tr("Crosshair"));
  this->CrosshairToolButton->setText(QObject::tr("Crosshair"));
  this->CrosshairToolButton->setMenu(this->CrosshairMenu);
  this->CrosshairToolButton->setPopupMode(QToolButton::MenuButtonPopup);

  // Default action
  this->CrosshairToggleAction = new QAction(q);
  this->CrosshairToggleAction->setIcon(QIcon(":/Icons/SlicesCrosshair.png"));
  this->CrosshairToggleAction->setCheckable(true);
  this->CrosshairToggleAction->setToolTip(QObject::tr("Toggle crosshair or set crosshair properties."));
  this->CrosshairToggleAction->setText(QObject::tr("Crosshair"));
  this->CrosshairToolButton->setDefaultAction(this->CrosshairToggleAction);
  QObject::connect(this->CrosshairToggleAction, SIGNAL(toggled(bool)),
                   this, SLOT(setCrosshairMode(bool)));

  q->addWidget(this->CrosshairToolButton);
  QObject::connect(q, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                   this->CrosshairToolButton,
                   SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));
  /// Other controls
  ///
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setNavigation(bool mode)
{
//  Q_Q(qSlicerViewersToolBar);

  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->MRMLScene->GetNodesByClass("vtkMRMLCrosshairNode"));
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLCrosshairNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLCrosshairNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetNavigation(mode);
    }
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setCrosshairMode(bool mode)
{
//  Q_Q(qSlicerViewersToolBar);

  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->MRMLScene->GetNodesByClass("vtkMRMLCrosshairNode"));
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLCrosshairNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLCrosshairNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    if (mode)
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
  vtkMRMLCrosshairNode* node = 0;
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
  vtkMRMLCrosshairNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLCrosshairNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetCrosshairThickness(thickness);
    }
}

// --------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::setSliceIntersectionVisible(bool visible)
{
  // Q_Q(qSlicerViewersToolBar);
  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->MRMLScene->GetNodesByClass("vtkMRMLSliceCompositeNode"));
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceCompositeNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceCompositeNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetSliceIntersectionVisibility(visible);
    }
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


    // Watch the Red SliceCompositeNodes for a change in the
    // SliceIntersectionVisibility state. There are potentially many
    // SliceCompositeNodes but only one menu option in the toolbar for
    // the state. So we just watch the Red viewer.
    qSlicerApplication *app = qSlicerApplication::application();
    qSlicerLayoutManager *layoutManager = app->layoutManager();
    if (layoutManager)
      {
      qMRMLSliceWidget *red = layoutManager->sliceWidget("Red");
      if (red)
        {
        vtkMRMLSliceCompositeNode *node = red->mrmlSliceCompositeNode();
        if (node)
          {
          this->qvtkReconnect(node, vtkCommand::ModifiedEvent,
                              this, SLOT(onSliceCompositeNodeChangedEvent()));
          }
        }
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

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::updateWidgetFromMRML()
{
  Q_ASSERT(this->MRMLScene);

  vtkMRMLNode *node;
  vtkCollectionSimpleIterator it;
  vtkMRMLCrosshairNode* crosshairNode = 0;
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
    // toggle on/off, navigation/cross-reference, style of crosshair
    //

    // on/off
    if (this->CrosshairToolButton)
      {
      this->CrosshairToggleAction->setChecked( crosshairNode->GetCrosshairMode() != vtkMRMLCrosshairNode::NoCrosshair );
      }

    // style of crosshair
    if (this->CrosshairMapper->mapping(crosshairNode->GetCrosshairMode()) != NULL)
      {
      QAction* action = (QAction *)(this->CrosshairMapper->mapping(crosshairNode->GetCrosshairMode()));
      if (action)
        {
        action->setChecked(true);
        }
      }

    // thickness
    if (this->CrosshairThicknessMapper->mapping(crosshairNode->GetCrosshairThickness()) != NULL)
      {
      QAction* action = (QAction *)(this->CrosshairThicknessMapper->mapping(crosshairNode->GetCrosshairThickness()));
      if (action)
        {
        action->setChecked(true);
        }
      }

    // navigation/cross-reference
    this->CrosshairNavigationAction->setChecked(crosshairNode->GetNavigation());

    // cache the mode
    if (crosshairNode->GetCrosshairMode() != vtkMRMLCrosshairNode::NoCrosshair)
      {
      this->CrosshairLastMode = crosshairNode->GetCrosshairMode();
      }

    }

  // toggle the slice intersections. this is harder to manage as there
  // are many SliceCompositeNodes but only one action. For now, let's
  // let changes in the Red Viewer control the state of this action
  qSlicerApplication *app = qSlicerApplication::application();
  qSlicerLayoutManager *layoutManager = app->layoutManager();
  if (layoutManager)
    {
    qMRMLSliceWidget *red = layoutManager->sliceWidget("Red");
    if (red)
      {
      vtkMRMLSliceCompositeNode *node = red->mrmlSliceCompositeNode();
      if (node)
        {
        this->CrosshairSliceIntersectionsAction->setChecked(node->GetSliceIntersectionVisibility());
        }
      }
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
  // reenable it and update
  q->setEnabled(true);
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::onCrosshairNodeModeChangedEvent()
{
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void qSlicerViewersToolBarPrivate::onSliceCompositeNodeChangedEvent()
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
qSlicerViewersToolBar::~qSlicerViewersToolBar()
{
}

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





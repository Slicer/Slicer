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
#include <QButtonGroup>
#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QVariant>
#include <QFileInfo>

// CTK includes
#include <ctkLogger.h>

// qMRMLWidgets includes
#include <qMRMLThreeDView.h>
#include <qMRMLSliceWidget.h>
#include <qMRMLSliceControllerWidget.h>
#include <qMRMLUtils.h>
#include <qMRMLNodeFactory.h>

// SlicerQt includes
#include "vtkSlicerConfigure.h"
#include "qMRMLLayoutManager.h"
#include "qMRMLLayoutManager_p.h"

// MRML includes
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceLogic.h>

// VTK includes
#include <vtkObject.h>
#include <vtkCollection.h>
#include <vtkSmartPointer.h>
#include <vtkIntArray.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorObserver.h>
#include <vtkCornerAnnotation.h>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.base.qtgui.qMRMLLayoutManager");
//--------------------------------------------------------------------------
  
//------------------------------------------------------------------------------
// qMRMLLayoutManagerPrivate methods

//------------------------------------------------------------------------------
qMRMLLayoutManagerPrivate::qMRMLLayoutManagerPrivate(qMRMLLayoutManager& object)
  : q_ptr(&object)
{
  this->MRMLScene = 0;
  this->MRMLLayoutNode = 0;
  this->ActiveMRMLThreeDViewNode = 0;
  this->MRMLSliceLogics = vtkCollection::New();
  this->SliceControllerButtonGroup = 0;
  this->GridLayout = 0;
  this->TargetWidget = 0;
  this->CurrentViewArrangement = vtkMRMLLayoutNode::SlicerLayoutNone;
  this->SavedCurrentViewArrangement = vtkMRMLLayoutNode::SlicerLayoutNone;
}

//------------------------------------------------------------------------------
qMRMLLayoutManagerPrivate::~qMRMLLayoutManagerPrivate()
{
  if (this->MRMLSliceLogics)
    {
    this->MRMLSliceLogics->RemoveAllItems();
    this->MRMLSliceLogics->Delete();
    }
}

//------------------------------------------------------------------------------
qMRMLThreeDView* qMRMLLayoutManagerPrivate::threeDView(int id)const
{
  Q_ASSERT(id >= 0);
  if (id >= this->ThreeDViewList.size())
    {
    return 0;
    }
  return this->ThreeDViewList.at(id);
}

//------------------------------------------------------------------------------
qMRMLThreeDView* qMRMLLayoutManagerPrivate::threeDView(vtkMRMLViewNode* node)const
{
  if (!node)
    {
    return 0;
    }
  foreach(qMRMLThreeDView* view, this->ThreeDViewList)
    {
    if (view->mrmlViewNode() == node)
      {
      return view;
      }
    }
  return 0;
}

//------------------------------------------------------------------------------
qMRMLThreeDView* qMRMLLayoutManagerPrivate::threeDViewCreateIfNeeded(int id)
{
  Q_ASSERT(id >= 0);
  qMRMLThreeDView* view = this->threeDView(id);
  while (!view)
    {
    this->createThreeDView();
    view = this->threeDView(id);
    }
  Q_ASSERT(view);
  return view;
}

//------------------------------------------------------------------------------
qMRMLSliceWidget* qMRMLLayoutManagerPrivate::sliceWidget(vtkMRMLSliceNode* node)const
{
  foreach(qMRMLSliceWidget* slice, this->SliceWidgetList)
    {
    if (slice->mrmlSliceNode() == node)
      {
      return slice;
      }
    }
  return 0;
}

//------------------------------------------------------------------------------
qMRMLSliceWidget* qMRMLLayoutManagerPrivate::sliceWidget(const QString& name)const
{
  foreach(qMRMLSliceWidget* slice, this->SliceWidgetList)
    {
    if (slice->sliceViewName() == name)
      {
      return slice;
      }
    }
  return 0;
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLLayoutManagerPrivate::viewNode(QWidget* widget)const
{
  if (qobject_cast<qMRMLSliceWidget*>(widget))
    {
    return qobject_cast<qMRMLSliceWidget*>(widget)->mrmlSliceNode();
    }
  if (qobject_cast<qMRMLThreeDView*>(widget))
    {
    return qobject_cast<qMRMLThreeDView*>(widget)->mrmlViewNode();
    }
  return 0;
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setMRMLScene(vtkMRMLScene* scene)
{
  if (this->MRMLScene == scene)
    {
    return;
    }

  this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::NodeAddedEvent, this,
    SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));

  this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::NodeRemovedEvent, this,
    SLOT(onNodeRemovedEvent(vtkObject*, vtkObject*)));

  this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneImportedEvent, this,
    SLOT(onSceneImportedEvent()));

  this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneAboutToBeClosedEvent, this,
    SLOT(onSceneAboutToBeClosedEvent()));

  this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneClosedEvent, this,
    SLOT(onSceneClosedEvent()));

  this->MRMLScene = scene;
  this->MRMLLayoutNode = 0;

  // update all the slice views and the 3D views
  foreach (qMRMLSliceWidget* sliceWidget, this->SliceWidgetList )
    {
    sliceWidget->setMRMLScene(this->MRMLScene);
    }
  foreach (qMRMLThreeDView* threeDView, this->ThreeDViewList )
    {
    threeDView->setMRMLScene(this->MRMLScene);
    }

  this->initialize();
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setMRMLLayoutNode(vtkMRMLLayoutNode* layoutNode)
{
  this->qvtkReconnect(this->MRMLLayoutNode, layoutNode, vtkCommand::ModifiedEvent,
                    this, SLOT(onLayoutNodeModifiedEvent(vtkObject*)));
  this->MRMLLayoutNode = layoutNode;
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setActiveMRMLThreeDViewNode(vtkMRMLViewNode * node)
{
  Q_Q(qMRMLLayoutManager);
  if (this->ActiveMRMLThreeDViewNode == node)
    {
    return;
    }

  this->ActiveMRMLThreeDViewNode = node;

  emit q->activeThreeDRendererChanged(q->activeThreeDRenderer());
  emit q->activeMRMLThreeDViewNodeChanged(this->ActiveMRMLThreeDViewNode);
}

// --------------------------------------------------------------------------
QWidget* qMRMLLayoutManagerPrivate::createSliceWidget(vtkMRMLSliceNode* sliceNode)
{
  if (!this->TargetWidget || !sliceNode || !this->MRMLScene)
    {// can't create a slice widget if there is no parent widget
    Q_ASSERT(this->MRMLScene);
    Q_ASSERT(sliceNode);
    return 0;
    }

  // there is a unique slice widget per node
  Q_ASSERT(!this->sliceWidget(sliceNode));

  qMRMLSliceWidget * sliceWidget =  new qMRMLSliceWidget(this->TargetWidget);
  sliceWidget->sliceController()->setControllerButtonGroup(this->SliceControllerButtonGroup);
  QString sliceLayoutName(sliceNode->GetLayoutName());
  sliceWidget->setSliceViewName(sliceLayoutName);
  sliceWidget->setMRMLScene(this->MRMLScene);
  sliceWidget->setMRMLSliceNode(sliceNode);
  sliceWidget->setSliceLogics(this->MRMLSliceLogics);

  this->SliceWidgetList.push_back(sliceWidget);
  this->MRMLSliceLogics->AddItem(sliceWidget->sliceLogic());
  logger.trace(QString("createSliceWidget - instantiated new qMRMLSliceWidget: %1")
               .arg(sliceLayoutName));

  logger.trace(QString("created %1").arg(sliceLayoutName));
  return sliceWidget;
}

// --------------------------------------------------------------------------
qMRMLThreeDView* qMRMLLayoutManagerPrivate::createThreeDView()
{
  int lastViewCount = this->ThreeDViewList.size();
  vtkMRMLNode* viewNode = qMRMLNodeFactory::createNode(this->MRMLScene, "vtkMRMLViewNode");
  // onNodeAdded should have created the threeDView, if not, then create it
  if (lastViewCount == this->ThreeDViewList.size())
    {
    this->createThreeDView(vtkMRMLViewNode::SafeDownCast(viewNode));
    }
  return this->threeDView(lastViewCount);
}

// --------------------------------------------------------------------------
qMRMLThreeDView* qMRMLLayoutManagerPrivate::createThreeDView(vtkMRMLViewNode* viewNode)
{
  if (!this->TargetWidget || !this->MRMLScene || !viewNode)
    {
    Q_ASSERT(this->MRMLScene);
    Q_ASSERT(viewNode);
    return 0;
    }

  // There must be a unique threedview per node
  Q_ASSERT(!this->threeDView(viewNode));

  qMRMLThreeDView* threeDView = 0;

  logger.trace("createThreeDView - instantiated new qMRMLThreeDView");
  threeDView = new qMRMLThreeDView(this->TargetWidget);
  threeDView->registerDisplayableManagers(this->ScriptedDisplayableManagerDirectory);
  threeDView->setMRMLScene(this->MRMLScene);
  threeDView->setMRMLViewNode(viewNode);

  this->ThreeDViewList.push_back(threeDView);

  return threeDView;
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::removeSliceView(vtkMRMLSliceNode* sliceNode)
{
  Q_ASSERT(sliceNode);

  qMRMLSliceWidget * sliceWidgetToDelete = this->sliceWidget(sliceNode);
  Q_ASSERT(sliceWidgetToDelete);

  // Remove slice widget
  this->SliceWidgetList.removeAll(sliceWidgetToDelete);
  delete sliceWidgetToDelete;

}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::removeThreeDView(vtkMRMLViewNode* viewNode)
{
  Q_ASSERT(viewNode);
  qMRMLThreeDView * threeDViewToDelete = this->threeDView(viewNode);

  // Remove threeDView
  if (threeDViewToDelete)
    {
    this->ThreeDViewList.removeAll(threeDViewToDelete);
    delete threeDViewToDelete;
    }
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::onNodeAddedEvent(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  Q_ASSERT(scene);
  Q_ASSERT(scene == this->MRMLScene);
  if (!this->MRMLScene || this->MRMLScene->GetIsUpdating())
    {
    return;
    }

  // Layout node
  vtkMRMLLayoutNode * layoutNode = vtkMRMLLayoutNode::SafeDownCast(node);
  if (layoutNode)
    {
    logger.trace("onLayoutNodeAddedEvent");
    // Only one Layout node is expected
    Q_ASSERT(this->MRMLLayoutNode == 0);
    if (this->MRMLLayoutNode != 0)
      {
      return;
      }
    this->setMRMLLayoutNode(layoutNode);
    }

  // View node
  vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(node);
  if (viewNode)
    {
    logger.trace(QString("onViewNodeAddedEvent - id: %1").arg(viewNode->GetID()));
    if (!this->threeDView(viewNode))
      {
      this->createThreeDView(viewNode);
      }
    }

  // Slice node
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  if (sliceNode)
    {
    QString layoutName = sliceNode->GetLayoutName();
    logger.trace(QString("onSliceNodeAddedEvent - layoutName: %1").arg(layoutName));
    Q_ASSERT(layoutName == "Red" || layoutName == "Yellow" || layoutName == "Green");
    if (!this->sliceWidget(sliceNode))
      {
      this->createSliceWidget(sliceNode);
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::onNodeRemovedEvent(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  Q_ASSERT(scene);
  Q_ASSERT(scene == this->MRMLScene);
  // Layout node
  vtkMRMLLayoutNode * layoutNode = vtkMRMLLayoutNode::SafeDownCast(node);
  if (layoutNode)
    {
    // The layout to be removed should be the same as the stored one
    Q_ASSERT(this->MRMLLayoutNode == layoutNode);
    this->setMRMLLayoutNode(0);
    }

  // View node
  vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(node);
  if (viewNode)
    {
    if (viewNode == this->ActiveMRMLThreeDViewNode)
      {
      this->setActiveMRMLThreeDViewNode(0); 
      }
    this->removeThreeDView(viewNode);
    }

  // Slice node
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  if (sliceNode)
    {
    this->removeSliceView(sliceNode);
    }
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::onSceneAboutToBeClosedEvent()
{
  logger.trace("onSceneAboutToBeClosedEvent");

  this->clearLayout(this->GridLayout);
  this->SavedCurrentViewArrangement = this->CurrentViewArrangement;
  this->CurrentViewArrangement = vtkMRMLLayoutNode::SlicerLayoutNone;
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::onSceneClosedEvent()
{
  logger.trace("onSceneClosedEvent");
  if (this->MRMLScene->GetIsConnecting())
    {
    // some more processing on the scene is happeninng, let's just wait until it
    // finishes.
    return;
    }
  // Since the loaded scene may not contain the required nodes, calling
  // initialize will make sure the LayoutNode, MRMLViewNode,
  // MRMLSliceNode exists.
  this->initialize();

  // Make sure the layoutNode arrangement match the LayoutManager one
  Q_ASSERT(this->MRMLLayoutNode);
  this->MRMLLayoutNode->SetViewArrangement(this->SavedCurrentViewArrangement);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::onSceneImportedEvent()
{
  logger.trace("onSceneImportedEvent");

  // Since the loaded scene may not contain the required node, calling initialize
  // will make sure the LayoutNode, MRMLViewNode, MRMLSliceNode exists.
  this->initialize();

  // Make sure the layoutNode arrangement match the LayoutManager one
  Q_ASSERT(this->MRMLLayoutNode);

  // Restore saved view arrangement
  if (this->MRMLLayoutNode &&
      this->MRMLLayoutNode->GetViewArrangement() == vtkMRMLLayoutNode::SlicerLayoutNone &&
      this->SavedCurrentViewArrangement != vtkMRMLLayoutNode::SlicerLayoutNone)
    {
    this->MRMLLayoutNode->SetViewArrangement(this->SavedCurrentViewArrangement);
    }
  else
    {
    this->onLayoutNodeModifiedEvent(this->MRMLLayoutNode);
    }
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::onLayoutNodeModifiedEvent(vtkObject* layoutNode)
{
  Q_Q(qMRMLLayoutManager);
  if (!this->MRMLScene || this->MRMLScene->GetIsUpdating())
    {
    return;
    }
  vtkMRMLLayoutNode * mrmlLayoutNode = vtkMRMLLayoutNode::SafeDownCast(layoutNode);

  if (!mrmlLayoutNode)
    {
    Q_ASSERT(mrmlLayoutNode);
    return;
    }
  q->setLayout(mrmlLayoutNode->GetViewArrangement());
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::initializeNode(vtkMRMLNode* node)
{
  Q_ASSERT(this->sender());
  QString sliceLogicName = this->sender()->property("SliceLogicName").toString();
  vtkMRMLSliceNode * sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  Q_ASSERT(sliceNode);
  // Note that SingletonTag and LayoutName are the same
  sliceNode->SetLayoutName(sliceLogicName.toLatin1()); 
  if (sliceLogicName == "Red")
    {
    sliceNode->SetOrientationToAxial();
    }
  else if(sliceLogicName == "Yellow")
    {
    sliceNode->SetOrientationToSagittal();
    }
  else if(sliceLogicName == "Green")
    {
    sliceNode->SetOrientationToCoronal();
    }
  else
    {
    sliceNode->SetOrientationToReformat();
    }
  sliceNode->SetName(sliceLogicName.toLower().append(
      sliceNode->GetOrientationString()).toLatin1());
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::initialize()
{
  if (!this->MRMLScene)
    {
    return;
    }
  // Create vtkMRMLViewNode if required
  int viewNodeCount = qMRMLUtils::countVisibleViewNode(this->MRMLScene);
  // It's ok to have more than 2 View nodes. Right now the user can't access
  // them but it might be supported later on. They could also create tones of
  // views from the Camera module if they wish.
  // Q_ASSERT(viewNodeCount >= 0 && viewNodeCount <= 2);
  if (viewNodeCount == 0)
    {
    vtkMRMLNode * node = qMRMLNodeFactory::createNode(this->MRMLScene, "vtkMRMLViewNode");
    Q_ASSERT(node);
    // For now, the active view is the first one
    // TODO It should be possible to change the activeview. For example,
    // the one having the focus. The LayoutNode should also store the current
    // active view
    this->setActiveMRMLThreeDViewNode(vtkMRMLViewNode::SafeDownCast(node));
    }
  else
    {
    // Maybe the nodes have been created a while ago, we need to associate a view to each of them
    std::vector<vtkMRMLNode*> viewNodes;
    this->MRMLScene->GetNodesByClass("vtkMRMLViewNode", viewNodes);
    for (unsigned int i = 0; i < viewNodes.size(); ++i)
      {
      vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(viewNodes[i]);
      if (!this->threeDView(viewNode))
        {
        this->createThreeDView(viewNode);
        }
      // For now, the active view is the first one
      if (i == 0)
        {
        this->setActiveMRMLThreeDViewNode(viewNode);
        }
      }
    }

  // Create "Red", "Yellow" and "Green" vtkMRMLSliceNode if required
  int sliceNodeCount = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLSliceNode");
  if (sliceNodeCount == 0)
    {
    QStringList sliceLogicNames;
    sliceLogicNames << "Red" << "Yellow" << "Green";
    qMRMLNodeFactory nodeFactory;
    nodeFactory.setMRMLScene(this->MRMLScene);
    connect(&nodeFactory, SIGNAL(nodeInitialized(vtkMRMLNode*)),
            this, SLOT(initializeNode(vtkMRMLNode*)));
    foreach(const QString& sliceLogicName, sliceLogicNames)
      {
      nodeFactory.setProperty("SliceLogicName", sliceLogicName);
      vtkMRMLNode * node = nodeFactory.createNode("vtkMRMLSliceNode");
      Q_ASSERT(node);
      Q_UNUSED(node);
      }
    }
  else
    {
    // Maybe the nodes have been created a while ago, we need to associate a view to each of them
    std::vector<vtkMRMLNode*> sliceNodes;
    this->MRMLScene->GetNodesByClass("vtkMRMLSliceNode", sliceNodes);
    for (unsigned int i = 0; i < sliceNodes.size();++i)
      {
      vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(sliceNodes[i]);
      if (!this->sliceWidget(sliceNode))
        {
        this->createSliceWidget(sliceNode);
        }
      }
    }

  // Create vtkMRMLLayoutNode if required
  this->MRMLScene->InitTraversal();
  vtkMRMLLayoutNode* layoutNode = vtkMRMLLayoutNode::SafeDownCast(
    this->MRMLScene->GetNextNodeByClass("vtkMRMLLayoutNode"));
  if (layoutNode == 0)
    {
    layoutNode = vtkMRMLLayoutNode::New();
    // the returned value of vtkMRMLScene::AddNode can be different from its
    // input when the input is a singleton node (vtkMRMLNode::SingletonTag is 1)
    // As we observe the MRML scene, this->MRMLLayoutNode will be set in
    // onNodeAdded
    vtkMRMLNode * nodeCreated = this->MRMLScene->AddNode(layoutNode);
    // as we checked that there was no vtkMRMLLayoutNode in the scene, the
    // returned node by vtkMRMLScene::AddNode() should be layoutNode
    Q_ASSERT(nodeCreated == layoutNode);
    Q_UNUSED(nodeCreated);
    layoutNode->Delete();
    layoutNode->SetViewArrangement(
      this->SavedCurrentViewArrangement != vtkMRMLLayoutNode::SlicerLayoutNone ?
      this->SavedCurrentViewArrangement : vtkMRMLLayoutNode::SlicerLayoutConventionalView);
    }
  else
    {
    // make sure there is just 1 node
    Q_ASSERT( this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLLayoutNode") == 1 );
    this->setMRMLLayoutNode(layoutNode);
    // relayout the items
    this->setLayoutInternal(layoutNode->GetViewArrangement());
    }
}

//------------------------------------------------------------------------------
bool qMRMLLayoutManagerPrivate::startUpdateLayout()
{
  if (!this->TargetWidget)
    {
    return false;
    }
  bool updatesEnabled = this->TargetWidget->updatesEnabled();
  this->TargetWidget->setUpdatesEnabled(false);
  return updatesEnabled;
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::endUpdateLayout(bool updatesEnabled)
{
  if (!this->TargetWidget)
    {
    return;
    }
  this->TargetWidget->setUpdatesEnabled(updatesEnabled);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::clearLayout(QLayout* layout)
{
  if (!layout)
    {
    return;
    }
  QLayoutItem * layoutItem = 0;
  while ((layoutItem = layout->takeAt(0)) != 0)
    {
    if (layoutItem->widget())
      {
      layoutItem->widget()->setVisible(false);
      vtkMRMLNode* node = this->viewNode(layoutItem->widget());
      if (node)
        {
        node->SetAttribute("MappedInLayout", "0");
        }
      layout->removeWidget(layoutItem->widget());
      }
    else if (layoutItem->layout())
      {
      this->clearLayout(layoutItem->layout());
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setLayoutInternal(int layout)
{
  bool updatesEnabled = true;
  if (this->TargetWidget)
    {
    updatesEnabled = this->startUpdateLayout();
    this->clearLayout(this->GridLayout);

    switch(layout)
      {
      case vtkMRMLLayoutNode::SlicerLayoutInitialView:
      case vtkMRMLLayoutNode::SlicerLayoutConventionalView:
        this->setConventionalView();
        break;
      case vtkMRMLLayoutNode::SlicerLayoutFourUpView:
        this->setFourUpView();
        break;
      case vtkMRMLLayoutNode::SlicerLayoutOneUp3DView:
        this->setOneUp3DView();
        break;
      case vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView:
        this->setOneUpSliceView("Red");
        break;
      case vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView:
        this->setOneUpSliceView("Yellow");
        break;
      case vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView:
        this->setOneUpSliceView("Green");
        break;
      case vtkMRMLLayoutNode::SlicerLayoutTabbed3DView:
        this->setTabbed3DView();
        break;
      case vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView:
        this->setTabbedSliceView();
        break;
      case vtkMRMLLayoutNode::SlicerLayoutLightboxView:
        this->setLightboxView();
        break;
      case vtkMRMLLayoutNode::SlicerLayoutCompareView:
        this->setCompareView();
        break;
      case vtkMRMLLayoutNode::SlicerLayoutSideBySideLightboxView:
        this->setSideBySideLightboxView();
        break;
      case vtkMRMLLayoutNode::SlicerLayoutDual3DView:
        this->setDual3DView();
        break;
      case vtkMRMLLayoutNode::SlicerLayoutNone:
        this->setNone();
        break;
      default:
        logger.warn(QString("Unknown view layout: %1").arg(layout));
        break;
      }
    }
  // Update LayoutNode
  this->CurrentViewArrangement = layout;
  if (this->MRMLLayoutNode)
    {
    this->MRMLLayoutNode->SetViewArrangement(layout);
    }

  if (this->TargetWidget)
    {
    this->endUpdateLayout(updatesEnabled);
    }
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setConventionalView()
{
  logger.trace(QString("switch to ConventionalView"));
  // First render view
  qMRMLThreeDView * renderView = this->threeDViewCreateIfNeeded(0);
  this->GridLayout->addWidget(renderView, 0, 0, 1, -1); // fromRow, fromColumn, rowSpan, columnSpan
  this->showWidget(renderView);

  // Red Slice Viewer
  qMRMLSliceWidget* redSliceView = this->sliceWidget("Red");
  this->GridLayout->addWidget(redSliceView, 1, 0);
  this->showWidget(redSliceView);

  // Yellow Slice Viewer
  qMRMLSliceWidget* yellowSliceView = this->sliceWidget("Yellow");
  this->GridLayout->addWidget(yellowSliceView, 1, 1);
  this->showWidget(yellowSliceView);

  // Green Slice Viewer
  qMRMLSliceWidget* greenSliceView = this->sliceWidget("Green");
  this->GridLayout->addWidget(greenSliceView, 1, 2);
  this->showWidget(greenSliceView);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setOneUp3DView()
{
  logger.trace(QString("switch to OneUp3DView"));
  // First render view
  qMRMLThreeDView * renderView = this->threeDViewCreateIfNeeded(0);
  this->GridLayout->addWidget(renderView, 0, 0, 1, -1); // fromRow, fromColumn, rowSpan, columnSpan
  this->showWidget(renderView);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setOneUpSliceView(const QString& sliceLayoutName)
{
  logger.trace(QString("switch to OneUpSlice%1").arg(sliceLayoutName));
  // Sanity checks
  Q_ASSERT(sliceLayoutName == "Red" || sliceLayoutName == "Yellow" || sliceLayoutName == "Green");
  if (sliceLayoutName != "Red" && sliceLayoutName != "Yellow" && sliceLayoutName != "Green")
    {
    qWarning() << "Slicer viewer name" << sliceLayoutName << "invalid !";
    return;
    }
  // Slice viewer
  qMRMLSliceWidget* sliceWidget = this->sliceWidget(sliceLayoutName);
  sliceWidget->setVisible(true);
  this->GridLayout->addWidget(sliceWidget, 1, 0);
  this->showWidget(sliceWidget);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setFourUpView()
{
  logger.trace(QString("switch to FourUpView"));
  // First render view
  qMRMLThreeDView * renderView = this->threeDViewCreateIfNeeded(0);
  this->GridLayout->addWidget(renderView, 0, 1); // fromRow, fromColumn, rowSpan, columnSpan
  this->showWidget(renderView);

  // Red Slice Viewer
  qMRMLSliceWidget* redSliceView = this->sliceWidget("Red");
  this->GridLayout->addWidget(redSliceView, 0, 0);
  this->showWidget(redSliceView);

  // Yellow Slice Viewer
  qMRMLSliceWidget* yellowSliceView = this->sliceWidget("Yellow");
  this->GridLayout->addWidget(yellowSliceView, 1, 0);
  this->showWidget(yellowSliceView);

  // Green Slice Viewer
  qMRMLSliceWidget* greenSliceView = this->sliceWidget("Green");
  this->GridLayout->addWidget(greenSliceView, 1, 1);
  this->showWidget(greenSliceView);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setTabbed3DView()
{
  logger.trace(QString("switch to Tabbed3DView"));
  qDebug() << "qMRMLLayoutManager::setTabbed3DView  not implemented";
  logger.debug("setTabbed3DView - Not Implemented");
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setTabbedSliceView()
{
  logger.trace(QString("switch to TabbedSliceView"));
  qDebug() << "qMRMLLayoutManager::setTabbedSliceView  not implemented";
  logger.debug("setTabbedSliceView - Not Implemented");
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setLightboxView()
{
  logger.trace(QString("switch to LightboxView"));
  qDebug() << "qMRMLLayoutManager::setLightboxView  not implemented";
  logger.debug("setLightboxView - Not Implemented");
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setCompareView()
{
  logger.trace(QString("switch to CompareView"));
  qDebug() << "qMRMLLayoutManager::setCompareView not implemented";
  logger.debug("setCompareView - Not Implemented");
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setSideBySideLightboxView()
{
  logger.trace(QString("switch to SideBySideLightboxView"));
  qDebug() << "qMRMLLayoutManager::setSideBySideLightboxView";
  logger.debug("setSideBySideLightboxView - Not Implemented");
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setDual3DView()
{
  logger.trace(QString("switch to Dual3DView"));
  // First render view
  qMRMLThreeDView * renderView = this->threeDViewCreateIfNeeded(0);
  this->GridLayout->addWidget(renderView, 0, 0); // fromRow, fromColumn, rowSpan, columnSpan
  this->showWidget(renderView);

  // Second render view
  qMRMLThreeDView * renderView2 = this->threeDViewCreateIfNeeded(1);
  this->GridLayout->addWidget(renderView2, 0, 1);
  this->showWidget(renderView2);

  // Add an horizontal layout to group the 3 sliceWidgets
  QHBoxLayout * sliceWidgetLayout = new QHBoxLayout();
  this->GridLayout->addLayout(sliceWidgetLayout, 1, 0, 1, 2); // fromRow, fromColumn, rowSpan, columnSpan

  // Red Slice Viewer
  qMRMLSliceWidget* redSliceView = this->sliceWidget("Red");
  sliceWidgetLayout->addWidget(redSliceView);
  this->showWidget(redSliceView);

  // Yellow Slice Viewer
  qMRMLSliceWidget* yellowSliceView = this->sliceWidget("Yellow");
  sliceWidgetLayout->addWidget(yellowSliceView);
  this->showWidget(yellowSliceView);

  // Green Slice Viewer
  qMRMLSliceWidget* greenSliceView = this->sliceWidget("Green");
  sliceWidgetLayout->addWidget(greenSliceView);
  this->showWidget(greenSliceView);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setNone()
{
  logger.trace(QString("switch to None"));
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::showWidget(QWidget* widget)
{
  Q_ASSERT(widget);
  Q_ASSERT(this->viewNode(widget));
  this->viewNode(widget)->SetAttribute("MappedInLayout", "1");
  widget->setVisible(true);
}


//------------------------------------------------------------------------------
// qMRMLLayoutManager methods

// --------------------------------------------------------------------------
qMRMLLayoutManager::qMRMLLayoutManager(QWidget* widget) : Superclass(widget)
  , d_ptr(new qMRMLLayoutManagerPrivate(*this))
{
  this->setViewport(widget);
}

// --------------------------------------------------------------------------
qMRMLLayoutManager::qMRMLLayoutManager(qMRMLLayoutManagerPrivate* pimpl, QWidget* widget)
  : Superclass(widget)
  , d_ptr(pimpl)
{
  this->setViewport(widget);
}

// --------------------------------------------------------------------------
qMRMLLayoutManager::~qMRMLLayoutManager()
{
}

// --------------------------------------------------------------------------
void qMRMLLayoutManager::setViewport(QWidget* widget)
{
  Q_D(qMRMLLayoutManager);
  if (widget == d->TargetWidget)
    {
    return;
    }

  QButtonGroup* buttonGroup = 0;
  QGridLayout* gridLayout = 0;
  if (widget)
    {
    if (!d->SliceControllerButtonGroup)
      {
      buttonGroup = new QButtonGroup(widget);
      buttonGroup->setExclusive(false);
      gridLayout = new QGridLayout(widget);
      gridLayout->setContentsMargins(0, 0, 0, 0);
      gridLayout->setSpacing(0);
      }
    else
      {
      buttonGroup = d->SliceControllerButtonGroup;
      gridLayout = d->GridLayout;
      }
    }
  foreach (qMRMLSliceWidget* sliceWidget, d->SliceWidgetList)
    {
    sliceWidget->sliceController()->setControllerButtonGroup(buttonGroup);
    }
  if (widget)
    {
    widget->setLayout(gridLayout);
    }

  if (buttonGroup != d->SliceControllerButtonGroup)
    {
    delete d->SliceControllerButtonGroup;
    d->SliceControllerButtonGroup= buttonGroup;
    // deleting the layout removes it from the old viewport
    delete d->GridLayout;
    d->GridLayout = gridLayout;
    }

  // set to new widget
  this->setParent(widget);
  d->TargetWidget = widget;

  // create necessary views if needed
  d->initialize();
}

// --------------------------------------------------------------------------
QWidget* qMRMLLayoutManager::viewport()const
{
  Q_D(const qMRMLLayoutManager);
  return d->TargetWidget;
}

//------------------------------------------------------------------------------
qMRMLSliceWidget* qMRMLLayoutManager::sliceWidget(const QString& name)const
{
  Q_D(const qMRMLLayoutManager);
  return d->sliceWidget(name);
}

//------------------------------------------------------------------------------
int qMRMLLayoutManager::threeDViewCount()const
{
  Q_D(const qMRMLLayoutManager);
  return d->ThreeDViewList.size();
}

//------------------------------------------------------------------------------
qMRMLThreeDView* qMRMLLayoutManager::threeDView(int id)const
{
  Q_D(const qMRMLLayoutManager);
  if(id < 0 || id >= d->ThreeDViewList.size())
    {
    return 0;
    }
  return d->threeDView(id);
}

//------------------------------------------------------------------------------
vtkCollection* qMRMLLayoutManager::mrmlSliceLogics()const
{
  Q_D(const qMRMLLayoutManager);
  return d->MRMLSliceLogics;
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLLayoutManager);
  d->setMRMLScene(scene);
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLLayoutManager::mrmlScene()const
{
  Q_D(const qMRMLLayoutManager);
  return d->MRMLScene;
}

//------------------------------------------------------------------------------
CTK_GET_CPP(qMRMLLayoutManager, vtkMRMLViewNode*,
            activeMRMLThreeDViewNode, ActiveMRMLThreeDViewNode)

//------------------------------------------------------------------------------
vtkRenderer* qMRMLLayoutManager::activeThreeDRenderer()const
{
  Q_D(const qMRMLLayoutManager);
  qMRMLThreeDView* activeThreeDView = d->threeDView(d->ActiveMRMLThreeDViewNode);
  return activeThreeDView ? activeThreeDView->renderer() : 0;
}

//------------------------------------------------------------------------------
int qMRMLLayoutManager::layout()const
{
  Q_D(const qMRMLLayoutManager);
  return d->CurrentViewArrangement;
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::setLayout(int layout)
{
  Q_D(qMRMLLayoutManager);
  if (d->CurrentViewArrangement == layout)
    {
    return;
    }
  d->setLayoutInternal(layout);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::switchToOneUpSliceView(const QString& sliceLayoutName)
{
  if (sliceLayoutName == "Red")
    {
    this->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
    }
  else if (sliceLayoutName == "Green")
    {
    this->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
    }
  else if (sliceLayoutName == "Yellow")
    {
    this->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView);
    }
  else
    {
    logger.warn(QString("Unknown view : %1").arg(sliceLayoutName));
    }
}

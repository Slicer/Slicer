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
#include "qMRMLLayoutWidget.h"
#include "qMRMLLayoutWidget_p.h"

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
static ctkLogger logger("org.slicer.base.qtgui.qMRMLLayoutWidget");
//--------------------------------------------------------------------------
  
//------------------------------------------------------------------------------
// qMRMLLayoutWidgetPrivate methods

//------------------------------------------------------------------------------
qMRMLLayoutWidgetPrivate::qMRMLLayoutWidgetPrivate(qMRMLLayoutWidget& object)
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
qMRMLLayoutWidgetPrivate::~qMRMLLayoutWidgetPrivate()
{
  if (this->MRMLSliceLogics)
    {
    this->MRMLSliceLogics->RemoveAllItems();
    this->MRMLSliceLogics->Delete();
    }
}

//------------------------------------------------------------------------------
qMRMLThreeDView* qMRMLLayoutWidgetPrivate::threeDView(int id)const
{
  Q_ASSERT(id >= 0);
  if (id >= this->ThreeDViewList.size())
    {
    return 0;
    }
  return this->ThreeDViewList.at(id);
}

//------------------------------------------------------------------------------
qMRMLThreeDView* qMRMLLayoutWidgetPrivate::threeDView(vtkMRMLViewNode* node)const
{
  Q_ASSERT(node);
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
qMRMLThreeDView* qMRMLLayoutWidgetPrivate::threeDViewCreateIfNeeded(int id)
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
qMRMLSliceWidget* qMRMLLayoutWidgetPrivate::sliceWidget(vtkMRMLSliceNode* node)const
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
qMRMLSliceWidget* qMRMLLayoutWidgetPrivate::sliceWidget(const QString& name)const
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
void qMRMLLayoutWidgetPrivate::setMRMLScene(vtkMRMLScene* scene)
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
void qMRMLLayoutWidgetPrivate::setMRMLLayoutNode(vtkMRMLLayoutNode* layoutNode)
{
  this->qvtkReconnect(this->MRMLLayoutNode, layoutNode, vtkCommand::ModifiedEvent,
                    this, SLOT(onLayoutNodeModifiedEvent(vtkObject*)));
  this->MRMLLayoutNode = layoutNode;
}

// --------------------------------------------------------------------------
void qMRMLLayoutWidgetPrivate::setActiveMRMLThreeDViewNode(vtkMRMLViewNode * node)
{
  Q_Q(qMRMLLayoutWidget);
  if (this->ActiveMRMLThreeDViewNode == node)
    {
    return;
    }
  this->ActiveMRMLThreeDViewNode = node;
  emit q->activeMRMLThreeDViewNodeChanged(this->ActiveMRMLThreeDViewNode);
}

// --------------------------------------------------------------------------
QWidget* qMRMLLayoutWidgetPrivate::createSliceWidget(vtkMRMLSliceNode* sliceNode)
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
  sliceWidget->registerDisplayableManagers(this->ScriptedDisplayableManagerDirectory);
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
qMRMLThreeDView* qMRMLLayoutWidgetPrivate::createThreeDView()
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
qMRMLThreeDView* qMRMLLayoutWidgetPrivate::createThreeDView(vtkMRMLViewNode* viewNode)
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
void qMRMLLayoutWidgetPrivate::removeSliceView(vtkMRMLSliceNode* sliceNode)
{
  Q_ASSERT(sliceNode);

  qMRMLSliceWidget * sliceWidgetToDelete = this->sliceWidget(sliceNode);
  Q_ASSERT(sliceWidgetToDelete);

  // Remove slice widget
  this->SliceWidgetList.removeAll(sliceWidgetToDelete);
  delete sliceWidgetToDelete;

}

// --------------------------------------------------------------------------
void qMRMLLayoutWidgetPrivate::removeThreeDView(vtkMRMLViewNode* viewNode)
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
void qMRMLLayoutWidgetPrivate::onNodeAddedEvent(vtkObject* scene, vtkObject* node)
{
  Q_ASSERT(scene);
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
    this->createThreeDView(viewNode);
    }

  // Slice node
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  if (sliceNode)
    {
    QString layoutName = sliceNode->GetLayoutName();
    logger.trace(QString("onSliceNodeAddedEvent - layoutName: %1").arg(layoutName));
    Q_ASSERT(layoutName == "Red" || layoutName == "Yellow" || layoutName == "Green");
    this->createSliceWidget(sliceNode);
    }
}

// --------------------------------------------------------------------------
void qMRMLLayoutWidgetPrivate::onNodeRemovedEvent(vtkObject* scene, vtkObject* node)
{
  Q_ASSERT(scene);

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
void qMRMLLayoutWidgetPrivate::onSceneAboutToBeClosedEvent()
{
  logger.trace("onSceneAboutToBeClosedEvent");

  this->clearLayout(this->GridLayout);
  this->SavedCurrentViewArrangement = this->CurrentViewArrangement;
  this->CurrentViewArrangement = vtkMRMLLayoutNode::SlicerLayoutNone;
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidgetPrivate::onSceneClosedEvent()
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
void qMRMLLayoutWidgetPrivate::onSceneImportedEvent()
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
void qMRMLLayoutWidgetPrivate::onLayoutNodeModifiedEvent(vtkObject* layoutNode)
{
  Q_Q(qMRMLLayoutWidget);
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

namespace
{
//------------------------------------------------------------------------------
struct vtkMRMLSliceNodeInitializer : public vtkMRMLNodeInitializer
{
  vtkMRMLSliceNodeInitializer(const QString& sliceLogicName):SliceLogicName(sliceLogicName){}
  virtual void operator()(vtkMRMLNode* node)const
    {
    vtkMRMLSliceNode * sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
    Q_ASSERT(sliceNode);
    // Note that SingletonTag and LayoutName are the same
    sliceNode->SetLayoutName(this->SliceLogicName.toLatin1()); 
    if (this->SliceLogicName == "Red")
      {
      sliceNode->SetOrientationToAxial();
      }
    else if(this->SliceLogicName == "Yellow")
      {
      sliceNode->SetOrientationToSagittal();
      }
    else if(this->SliceLogicName == "Green")
      {
      sliceNode->SetOrientationToCoronal();
      }
    else
      {
      sliceNode->SetOrientationToReformat();
      }
    sliceNode->SetName(this->SliceLogicName.toLower().append(
        sliceNode->GetOrientationString()).toLatin1());
    }
  QString SliceLogicName;
};
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidgetPrivate::initialize()
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
    foreach(const QString& sliceLogicName, sliceLogicNames)
      {
      vtkMRMLSliceNodeInitializer sliceNodeInitializer(sliceLogicName);
      vtkMRMLNode * node = qMRMLNodeFactory::createNode(this->MRMLScene,
                                                        "vtkMRMLSliceNode", sliceNodeInitializer);
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
bool qMRMLLayoutWidgetPrivate::startUpdateLayout()
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
void qMRMLLayoutWidgetPrivate::endUpdateLayout(bool updatesEnabled)
{
  if (!this->TargetWidget)
    {
    return;
    }
  this->TargetWidget->setUpdatesEnabled(updatesEnabled);
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidgetPrivate::clearLayout(QLayout* layout)
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
      layout->removeWidget(layoutItem->widget());
      }
    else if (layoutItem->layout())
      {
      this->clearLayout(layoutItem->layout());
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidgetPrivate::setLayoutInternal(int layout)
{
  bool updatesEnabled = true;
  if (this->TargetWidget)
    {
    updatesEnabled = this->startUpdateLayout();
    this->clearLayout(this->GridLayout);

    switch(layout)
      {
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
void qMRMLLayoutWidgetPrivate::setConventionalView()
{
  logger.trace(QString("switch to ConventionalView"));
  // First render view
  qMRMLThreeDView * renderView = this->threeDViewCreateIfNeeded(0);
  this->GridLayout->addWidget(renderView, 0, 0, 1, -1); // fromRow, fromColumn, rowSpan, columnSpan
  renderView->setVisible(true);

  // Red Slice Viewer
  qMRMLSliceWidget* redSliceView = this->sliceWidget("Red");
  this->GridLayout->addWidget(redSliceView, 1, 0);
  redSliceView->setVisible(true);

  // Yellow Slice Viewer
  qMRMLSliceWidget* yellowSliceView = this->sliceWidget("Yellow");
  this->GridLayout->addWidget(yellowSliceView, 1, 1);
  yellowSliceView->setVisible(true);

  // Green Slice Viewer
  qMRMLSliceWidget* greenSliceView = this->sliceWidget("Green");
  this->GridLayout->addWidget(greenSliceView, 1, 2);
  greenSliceView->setVisible(true);
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidgetPrivate::setOneUp3DView()
{
  logger.trace(QString("switch to OneUp3DView"));
  // First render view
  qMRMLThreeDView * renderView = this->threeDViewCreateIfNeeded(0);
  this->GridLayout->addWidget(renderView, 0, 0, 1, -1); // fromRow, fromColumn, rowSpan, columnSpan
  renderView->setVisible(true);
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidgetPrivate::setOneUpSliceView(const QString& sliceLayoutName)
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
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidgetPrivate::setFourUpView()
{
  logger.trace(QString("switch to FourUpView"));
  // First render view
  qMRMLThreeDView * renderView = this->threeDViewCreateIfNeeded(0);
  this->GridLayout->addWidget(renderView, 0, 1); // fromRow, fromColumn, rowSpan, columnSpan
  renderView->setVisible(true);

  // Red Slice Viewer
  qMRMLSliceWidget* redSliceView = this->sliceWidget("Red");
  this->GridLayout->addWidget(redSliceView, 0, 0);
  redSliceView->setVisible(true);

  // Yellow Slice Viewer
  qMRMLSliceWidget* yellowSliceView = this->sliceWidget("Yellow");
  this->GridLayout->addWidget(yellowSliceView, 1, 0);
  yellowSliceView->setVisible(true);

  // Green Slice Viewer
  qMRMLSliceWidget* greenSliceView = this->sliceWidget("Green");
  this->GridLayout->addWidget(greenSliceView, 1, 1);
  greenSliceView->setVisible(true);
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidgetPrivate::setTabbed3DView()
{
  logger.trace(QString("switch to Tabbed3DView"));
  qDebug() << "qMRMLLayoutWidget::setTabbed3DView  not implemented";
  logger.debug("setTabbed3DView - Not Implemented");
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidgetPrivate::setTabbedSliceView()
{
  logger.trace(QString("switch to TabbedSliceView"));
  qDebug() << "qMRMLLayoutWidget::setTabbedSliceView  not implemented";
  logger.debug("setTabbedSliceView - Not Implemented");
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidgetPrivate::setLightboxView()
{
  logger.trace(QString("switch to LightboxView"));
  qDebug() << "qMRMLLayoutWidget::setLightboxView  not implemented";
  logger.debug("setLightboxView - Not Implemented");
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidgetPrivate::setCompareView()
{
  logger.trace(QString("switch to CompareView"));
  qDebug() << "qMRMLLayoutWidget::setCompareView not implemented";
  logger.debug("setCompareView - Not Implemented");
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidgetPrivate::setSideBySideLightboxView()
{
  logger.trace(QString("switch to SideBySideLightboxView"));
  qDebug() << "qMRMLLayoutWidget::setSideBySideLightboxView";
  logger.debug("setSideBySideLightboxView - Not Implemented");
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidgetPrivate::setDual3DView()
{
  logger.trace(QString("switch to Dual3DView"));
  // First render view
  qMRMLThreeDView * renderView = this->threeDViewCreateIfNeeded(0);
  this->GridLayout->addWidget(renderView, 0, 0); // fromRow, fromColumn, rowSpan, columnSpan
  renderView->setVisible(true);

  // Second render view
  qMRMLThreeDView * renderView2 = this->threeDViewCreateIfNeeded(1);
  this->GridLayout->addWidget(renderView2, 0, 1);
  renderView2->setVisible(true);

  // Add an horizontal layout to group the 3 sliceWidgets
  QHBoxLayout * sliceWidgetLayout = new QHBoxLayout();
  this->GridLayout->addLayout(sliceWidgetLayout, 1, 0, 1, 2); // fromRow, fromColumn, rowSpan, columnSpan

  // Red Slice Viewer
  qMRMLSliceWidget* redSliceView = this->sliceWidget("Red");
  sliceWidgetLayout->addWidget(redSliceView);
  redSliceView->setVisible(true);

  // Yellow Slice Viewer
  qMRMLSliceWidget* yellowSliceView = this->sliceWidget("Yellow");
  sliceWidgetLayout->addWidget(yellowSliceView);
  yellowSliceView->setVisible(true);

  // Green Slice Viewer
  qMRMLSliceWidget* greenSliceView = this->sliceWidget("Green");
  sliceWidgetLayout->addWidget(greenSliceView);
  greenSliceView->setVisible(true);
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidgetPrivate::setNone()
{
  logger.trace(QString("switch to None"));
}

//------------------------------------------------------------------------------
// qMRMLLayoutWidget methods

// --------------------------------------------------------------------------
qMRMLLayoutWidget::qMRMLLayoutWidget(QWidget* widget) : Superclass(widget)
  , d_ptr(new qMRMLLayoutWidgetPrivate(*this))
{
  this->setViewport(widget);
}

// --------------------------------------------------------------------------
qMRMLLayoutWidget::qMRMLLayoutWidget(qMRMLLayoutWidgetPrivate* pimpl, QWidget* widget)
  : Superclass(widget)
  , d_ptr(pimpl)
{
  this->setViewport(widget);
}

// --------------------------------------------------------------------------
qMRMLLayoutWidget::~qMRMLLayoutWidget()
{
}

// --------------------------------------------------------------------------
void qMRMLLayoutWidget::setViewport(QWidget* widget)
{
  Q_D(qMRMLLayoutWidget);
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
QWidget* qMRMLLayoutWidget::viewport()const
{
  Q_D(const qMRMLLayoutWidget);
  return d->TargetWidget;
}

//------------------------------------------------------------------------------
qMRMLSliceWidget* qMRMLLayoutWidget::sliceWidget(const QString& name)const
{
  Q_D(const qMRMLLayoutWidget);
  return d->sliceWidget(name);
}

//------------------------------------------------------------------------------
int qMRMLLayoutWidget::threeDViewCount()const
{
  Q_D(const qMRMLLayoutWidget);
  return d->ThreeDViewList.size();
}

//------------------------------------------------------------------------------
qMRMLThreeDView* qMRMLLayoutWidget::threeDView(int id)const
{
  Q_D(const qMRMLLayoutWidget);
  if(id < 0 || id >= d->ThreeDViewList.size())
    {
    return 0;
    }
  return d->threeDView(id);
}

//------------------------------------------------------------------------------
vtkCollection* qMRMLLayoutWidget::mrmlSliceLogics()const
{
  Q_D(const qMRMLLayoutWidget);
  return d->MRMLSliceLogics;
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLLayoutWidget);
  d->setMRMLScene(scene);
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLLayoutWidget::mrmlScene()const
{
  Q_D(const qMRMLLayoutWidget);
  return d->MRMLScene;
}

//------------------------------------------------------------------------------
CTK_GET_CXX(qMRMLLayoutWidget, vtkMRMLViewNode*,
            activeMRMLThreeDViewNode, ActiveMRMLThreeDViewNode)

//------------------------------------------------------------------------------
int qMRMLLayoutWidget::layout()const
{
  Q_D(const qMRMLLayoutWidget);
  return d->CurrentViewArrangement;
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::setLayout(int layout)
{
  Q_D(qMRMLLayoutWidget);
  if (d->CurrentViewArrangement == layout)
    {
    return;
    }
  d->setLayoutInternal(layout);
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::switchToOneUpSliceView(const QString& sliceLayoutName)
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

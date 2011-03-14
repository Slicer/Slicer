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
#include <QFileInfo>
#include <QVariant>
#include <QWidget>

// CTK includes
#include <ctkLogger.h>
#include <ctkLayoutManager_p.h>

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

// MRMLLogic includes
#include <vtkMRMLLayoutLogic.h>

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
#include <vtkXMLDataElement.h>

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
  this->MRMLLayoutLogic = vtkMRMLLayoutLogic::New();
  this->ActiveMRMLThreeDViewNode = 0;
  this->MRMLSliceLogics = vtkCollection::New();
  this->SliceControllerButtonGroup = 0;
  //this->SavedCurrentViewArrangement = vtkMRMLLayoutNode::SlicerLayoutNone;
}

//------------------------------------------------------------------------------
qMRMLLayoutManagerPrivate::~qMRMLLayoutManagerPrivate()
{
  if (this->MRMLSliceLogics)
    {
    this->MRMLSliceLogics->RemoveAllItems();
    this->MRMLSliceLogics->Delete();
    }
  this->MRMLLayoutLogic->Delete();
  this->MRMLLayoutLogic = 0;
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::init()
{
  Q_Q(qMRMLLayoutManager);
  this->SliceControllerButtonGroup = new QButtonGroup(q);
  this->SliceControllerButtonGroup->setExclusive(false);
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
QWidget* qMRMLLayoutManagerPrivate::viewWidget(vtkMRMLNode* viewNode)const
{
  if (vtkMRMLSliceNode::SafeDownCast(viewNode))
    {
    return this->sliceWidget(vtkMRMLSliceNode::SafeDownCast(viewNode));
    }
  if (vtkMRMLViewNode::SafeDownCast(viewNode))
    {
    return this->threeDView(vtkMRMLViewNode::SafeDownCast(viewNode));
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
    vtkMRMLScene::SceneRestoredEvent, this,
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
  this->MRMLLayoutLogic->SetMRMLScene(this->MRMLScene);

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
  Q_Q(qMRMLLayoutManager);
  if (!q->viewport() || !sliceNode || !this->MRMLScene)
    {// can't create a slice widget if there is no parent widget
    Q_ASSERT(this->MRMLScene);
    Q_ASSERT(sliceNode);
    return 0;
    }

  // there is a unique slice widget per node
  Q_ASSERT(!this->sliceWidget(sliceNode));

  qMRMLSliceWidget * sliceWidget =  new qMRMLSliceWidget(q->viewport());
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
  Q_Q(qMRMLLayoutManager);
  if (!q->viewport() || !this->MRMLScene || !viewNode)
    {
    Q_ASSERT(this->MRMLScene);
    Q_ASSERT(viewNode);
    return 0;
    }

  // There must be a unique threedview per node
  Q_ASSERT(!this->threeDView(viewNode));

  qMRMLThreeDView* threeDView = 0;

  logger.trace("createThreeDView - instantiated new qMRMLThreeDView");
  threeDView = new qMRMLThreeDView(q->viewport());
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
  Q_Q(qMRMLLayoutManager);
  logger.trace("onSceneAboutToBeClosedEvent");

  q->clearLayout();
//  if (this->MRMLLayoutNode)
//    {
//    this->SavedCurrentViewArrangement = this->MRMLLayoutNode->GetViewArrangement();
//    }
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
  //int viewArrangement = this->SavedCurrentViewArrangement;
  // Since the loaded scene may not contain the required nodes, calling
  // initialize will make sure the LayoutNode, MRMLViewNode,
  // MRMLSliceNode exists.
  this->initialize();

  // Make sure the layoutNode arrangement match the LayoutManager one
  Q_ASSERT(this->MRMLLayoutNode);
  //this->MRMLLayoutNode->SetViewArrangement(viewArrangement);
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
//  if (this->MRMLLayoutNode &&
//      this->MRMLLayoutNode->GetViewArrangement() == vtkMRMLLayoutNode::SlicerLayoutNone &&
//      this->SavedCurrentViewArrangement != vtkMRMLLayoutNode::SlicerLayoutNone)
//    {
//    this->MRMLLayoutNode->SetViewArrangement(this->SavedCurrentViewArrangement);
//    }
//  else
//    {
    this->onLayoutNodeModifiedEvent(this->MRMLLayoutNode);
//    }
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::onLayoutNodeModifiedEvent(vtkObject* layoutNode)
{
  if (!this->MRMLScene || this->MRMLScene->GetIsUpdating())
    {
    return;
    }
  vtkMRMLLayoutNode * mrmlLayoutNode = vtkMRMLLayoutNode::SafeDownCast(layoutNode);
  Q_ASSERT(mrmlLayoutNode);
  this->setLayoutInternal(mrmlLayoutNode->GetViewArrangement());
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
    Q_ASSERT(viewNodeCount);
  // It's ok to have more than 2 View nodes. Right now the user can't access
  // them but it might be supported later on. They could also create tones of
  // views from the Camera module if they wish.
  // Q_ASSERT(viewNodeCount >= 0 && viewNodeCount <= 2);
  if (viewNodeCount)
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
  Q_ASSERT(sliceNodeCount);
  if (sliceNodeCount)
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
  Q_ASSERT(layoutNode);
  if (layoutNode)
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
  Q_Q(qMRMLLayoutManager);
  if (!q->viewport())
    {
    return false;
    }
  bool updatesEnabled = q->viewport()->updatesEnabled();
  q->viewport()->setUpdatesEnabled(false);
  return updatesEnabled;
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::endUpdateLayout(bool updatesEnabled)
{
  Q_Q(qMRMLLayoutManager);
  if (!q->viewport())
    {
    return;
    }
  q->viewport()->setUpdatesEnabled(updatesEnabled);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setLayoutInternal(int layout)
{
  Q_Q(qMRMLLayoutManager);
  // Update LayoutNode
  if (this->MRMLLayoutNode)
    {
    this->MRMLLayoutNode->SetViewArrangement(layout);
    }
  if (layout == vtkMRMLLayoutNode::SlicerLayoutCustomView)
    {
    return;
    }

  //this->SavedCurrentViewArrangement = layout;
  // TBD: modify the dom doc manually, don't create a new one
  QDomDocument newLayout;
  newLayout.setContent(QString(
    this->MRMLLayoutNode ?
    this->MRMLLayoutNode->GetCurrentViewArrangement() : ""));
  q->setLayout(newLayout);
  /*
  // TODO: find a way to remove that hack (only needed for the tests)
  if (q->viewport())
    {
    this->Viewport->repaint();
    }
  */
  emit q->layoutChanged(layout);
}

//------------------------------------------------------------------------------
vtkMRMLLayoutLogic::ViewAttributes qMRMLLayoutManagerPrivate::attributesFromXML(QDomElement viewElement)
{
  vtkMRMLLayoutLogic::ViewAttributes attributes;
  QDomNamedNodeMap elementAttributes = viewElement.attributes();
  const int attributeCount = elementAttributes.count();
  for (int i = 0; i < attributeCount; ++i)
    {
    QDomNode attribute = elementAttributes.item(i);
    attributes[attribute.nodeName().toStdString()] =
      viewElement.attribute(attribute.nodeName()).toStdString();
    }
  return attributes;
}

//------------------------------------------------------------------------------
// qMRMLLayoutManager methods

// --------------------------------------------------------------------------
qMRMLLayoutManager::qMRMLLayoutManager(QObject* parentObject)
  : Superclass(0, parentObject)
  , d_ptr(new qMRMLLayoutManagerPrivate(*this))
{
  Q_D(qMRMLLayoutManager);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLLayoutManager::qMRMLLayoutManager(QWidget* viewport, QObject* parentObject)
  : Superclass(viewport, parentObject)
  , d_ptr(new qMRMLLayoutManagerPrivate(*this))
{
  Q_D(qMRMLLayoutManager);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLLayoutManager::qMRMLLayoutManager(qMRMLLayoutManagerPrivate* pimpl,
                                       QWidget* viewport, QObject* parentObject)
  : Superclass(viewport, parentObject)
  , d_ptr(pimpl)
{
  Q_D(qMRMLLayoutManager);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLLayoutManager::~qMRMLLayoutManager()
{
}

// --------------------------------------------------------------------------
void qMRMLLayoutManager::onViewportChanged()
{
  Q_D(qMRMLLayoutManager);
  // create necessary views if needed
  d->initialize();
  this->ctkLayoutManager::onViewportChanged();
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
vtkMRMLLayoutLogic* qMRMLLayoutManager::layoutLogic()const
{
  Q_D(const qMRMLLayoutManager);
  return d->MRMLLayoutLogic;
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
  return d->MRMLLayoutNode ?
    d->MRMLLayoutNode->GetViewArrangement() : vtkMRMLLayoutNode::SlicerLayoutNone;//d->SavedCurrentViewArrangement;
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::setLayout(int layout)
{
  Q_D(qMRMLLayoutManager);
  if (this->layout() == layout)
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

//------------------------------------------------------------------------------
QWidget* qMRMLLayoutManager::viewFromXML(QDomElement viewElement)
{
  Q_D(qMRMLLayoutManager);
  // convert Qt xml element into vtkMRMLLayoutLogic attributes
  vtkMRMLLayoutLogic::ViewAttributes attributes = d->attributesFromXML(viewElement);
  vtkMRMLNode* viewNode = d->MRMLLayoutLogic->GetViewFromAttributes(attributes);
  if (!viewNode)
    {
    viewNode = d->MRMLLayoutLogic->CreateViewFromAttributes(attributes);
    if (viewNode)
      {
      this->mrmlScene()->AddNode(viewNode);
      viewNode->Delete();
      }
    }
  Q_ASSERT(viewNode);
  return d->viewWidget(viewNode);
}

//------------------------------------------------------------------------------
QList<QWidget*> qMRMLLayoutManager::viewsFromXML(QDomElement viewElement)
{
  Q_D(qMRMLLayoutManager);
  QList<QWidget*> res;
  // convert Qt xml element into vtkMRMLLayoutLogic attributes
  vtkMRMLLayoutLogic::ViewAttributes attributes = d->attributesFromXML(viewElement);
  vtkCollection* viewNodes = d->MRMLLayoutLogic->GetViewsFromAttributes(attributes);
  viewNodes->InitTraversal();
  for (vtkMRMLNode* node = 0;(node = vtkMRMLNode::SafeDownCast(viewNodes->GetNextItemAsObject()));)
    {
    res  << d->viewWidget(node);
    }
  viewNodes->Delete();
  return res;
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::setupView(QDomElement viewElement, QWidget*view)
{
  Q_D(qMRMLLayoutManager);
  this->ctkLayoutManager::setupView(viewElement, view);
  vtkMRMLNode* viewNode = d->viewNode(view);
  viewNode->SetAttribute("MappedInLayout", "1");
  view->setWindowTitle(viewNode->GetName());

}

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
#include <QWidget>

// CTK includes
#include <ctkLogger.h>
#include <ctkLayoutManager_p.h>

// qMRMLWidgets includes
#include "qMRMLLayoutManager.h"
#include "qMRMLLayoutManager_p.h"
#include <qMRMLSliceWidget.h>
#include <qMRMLSliceControllerWidget.h>
#include <qMRMLThreeDView.h>

// MRMLLogic includes
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLSliceLogic.h>

// MRML includes
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkObject.h>
#include <vtkCollection.h>
#include <vtkSmartPointer.h>

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

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setMRMLLayoutNode(vtkMRMLLayoutNode* layoutNode)
{
  this->qvtkReconnect(this->MRMLLayoutNode, layoutNode, vtkCommand::ModifiedEvent,
                    this, SLOT(onLayoutNodeModifiedEvent(vtkObject*)));
  this->MRMLLayoutNode = layoutNode;
  this->onLayoutNodeModifiedEvent(layoutNode);
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
#ifndef QT_NO_DEBUG
    // Note: The following has to be done since QColor::isValidColor is not available with Qt < 4.7
    bool startWithCompare = layoutName.startsWith("Compare");
    bool validColor = false;
    if(!startWithCompare)
      {
      // To avoid warning 'unknown color', let's check if the color is valid only if it doesn't
      // start with 'Compare'
      QColor c;
      c.setNamedColor(layoutName.toLower());
      validColor = c.isValid();
      }
    Q_ASSERT(startWithCompare || validColor);
#endif
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
  // remove the layout during closing.
  q->clearLayout();
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
  this->updateLayoutFromMRMLScene();
  Q_ASSERT(this->MRMLLayoutNode);
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
void qMRMLLayoutManagerPrivate::updateWidgetsFromViewNodes()
{
  if (!this->MRMLScene)
    {
    return;
    }
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

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::updateLayoutFromMRMLScene()
{
  this->setMRMLLayoutNode(this->MRMLLayoutLogic->GetLayoutNode());
}

/*
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
*/

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

  // TBD: modify the dom doc manually, don't create a new one
  QDomDocument newLayout;
  newLayout.setContent(QString(
    this->MRMLLayoutNode ?
    this->MRMLLayoutNode->GetCurrentViewArrangement() : ""));
  q->setLayout(newLayout);
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
  d->updateWidgetsFromViewNodes();
  d->updateLayoutFromMRMLScene();
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
  if (d->MRMLScene == scene)
    {
    return;
    }

  vtkMRMLScene* oldScene = d->MRMLScene;
  d->MRMLScene = scene;
  d->MRMLLayoutNode = 0;

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::SceneImportedEvent,
                   d, SLOT(updateWidgetsFromViewNodes()));
  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::SceneRestoredEvent,
                   d, SLOT(updateWidgetsFromViewNodes()));
  // We want to connect the logic to the scene first (before the following
  // qvtkReconnect); that way, anytime the scene is modified, the logic
  // callbacks will be called  before qMRMLLayoutManager and keep the scene
  // in a good state
  d->MRMLLayoutLogic->SetMRMLScene(d->MRMLScene);

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::NodeAddedEvent,
                   d, SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::NodeRemovedEvent,
                   d, SLOT(onNodeRemovedEvent(vtkObject*, vtkObject*)));

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::SceneImportedEvent,
                   d, SLOT(updateLayoutFromMRMLScene()));

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::SceneRestoredEvent,
                   d, SLOT(updateLayoutFromMRMLScene()));

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::SceneAboutToBeClosedEvent,
                   d, SLOT(onSceneAboutToBeClosedEvent()));

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::SceneClosedEvent,
                   d, SLOT(onSceneClosedEvent()));

  // update all the slice views and the 3D views
  foreach (qMRMLSliceWidget* sliceWidget, d->SliceWidgetList )
    {
    sliceWidget->setMRMLScene(d->MRMLScene);
    }
  foreach (qMRMLThreeDView* threeDView, d->ThreeDViewList )
    {
    threeDView->setMRMLScene(d->MRMLScene);
    }

  d->updateWidgetsFromViewNodes();
  d->updateLayoutFromMRMLScene();
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
    d->MRMLLayoutNode->GetViewArrangement() : vtkMRMLLayoutNode::SlicerLayoutNone;
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
QWidget* qMRMLLayoutManager::viewFromXML(QDomElement viewElement)
{
  Q_D(qMRMLLayoutManager);
  // convert Qt xml element into vtkMRMLLayoutLogic attributes
  vtkMRMLLayoutLogic::ViewAttributes attributes = d->attributesFromXML(viewElement);
  vtkMRMLNode* viewNode = d->MRMLLayoutLogic->GetViewFromAttributes(attributes);
  // the view should have been created automatically by the logic when the new
  // view arrangement is set
  Q_ASSERT(viewNode);
  QWidget* view = d->viewWidget(viewNode);
  Q_ASSERT(view);
  return view;
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

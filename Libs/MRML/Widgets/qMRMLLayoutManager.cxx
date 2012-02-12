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
#include <QButtonGroup>
#include <QDebug>

// MRMLWidgets includes
#include "qMRMLLayoutManager_p.h"
#include <qMRMLSliceWidget.h>
#include <qMRMLSliceControllerWidget.h>
#include <qMRMLChartView.h>
#include <qMRMLChartWidget.h>
#include <qMRMLThreeDView.h>
#include <qMRMLThreeDWidget.h>

// MRMLLogic includes
#include <vtkMRMLSliceLogic.h>

// MRML includes
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLChartViewNode.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkCollection.h>

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
  this->ActiveMRMLChartViewNode = 0;
  this->MRMLSliceLogics = vtkCollection::New();
  this->MRMLColorLogic = 0;
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
  q->setSpacing(1);
}

//------------------------------------------------------------------------------
qMRMLThreeDWidget* qMRMLLayoutManagerPrivate::threeDWidget(int id)const
{
  Q_ASSERT(id >= 0);
  if (id >= this->ThreeDWidgetList.size())
    {
    return 0;
    }
  return this->ThreeDWidgetList.at(id);
}

//------------------------------------------------------------------------------
qMRMLChartWidget* qMRMLLayoutManagerPrivate::chartWidget(int id)const
{
  Q_ASSERT(id >= 0);
  if (id >= this->ChartWidgetList.size())
    {
    return 0;
    }
  return this->ChartWidgetList.at(id);
}

//------------------------------------------------------------------------------
qMRMLThreeDWidget* qMRMLLayoutManagerPrivate::threeDWidget(vtkMRMLViewNode* node)const
{
  if (!node)
    {
    return 0;
    }
  foreach(qMRMLThreeDWidget* view, this->ThreeDWidgetList)
    {
    if (view->mrmlViewNode() == node)
      {
      return view;
      }
    }
  return 0;
}

//------------------------------------------------------------------------------
qMRMLChartWidget* qMRMLLayoutManagerPrivate::chartWidget(vtkMRMLChartViewNode* node)const
{
  if (!node)
    {
    return 0;
    }
  foreach(qMRMLChartWidget* view, this->ChartWidgetList)
    {
    if (view->mrmlChartViewNode() == node)
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
  if (qobject_cast<qMRMLThreeDWidget*>(widget))
    {
    return qobject_cast<qMRMLThreeDWidget*>(widget)->mrmlViewNode();
    }
  if (qobject_cast<qMRMLChartWidget*>(widget))
    {
    return qobject_cast<qMRMLChartWidget*>(widget)->mrmlChartViewNode();
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
    return this->threeDWidget(vtkMRMLViewNode::SafeDownCast(viewNode));
    }
  if (vtkMRMLChartViewNode::SafeDownCast(viewNode))
    {
    return this->chartWidget(vtkMRMLChartViewNode::SafeDownCast(viewNode));
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
void qMRMLLayoutManagerPrivate::setActiveMRMLChartViewNode(vtkMRMLChartViewNode * node)
{
  Q_Q(qMRMLLayoutManager);
  if (this->ActiveMRMLChartViewNode == node)
    {
    return;
    }

  this->ActiveMRMLChartViewNode = node;

  emit q->activeChartRendererChanged(q->activeChartRenderer());
  emit q->activeMRMLChartViewNodeChanged(this->ActiveMRMLChartViewNode);
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

  qMRMLSliceWidget * sliceWidget = new qMRMLSliceWidget(q->viewport());
  sliceWidget->sliceController()->setControllerButtonGroup(this->SliceControllerButtonGroup);
  QString sliceLayoutName(sliceNode->GetLayoutName());
  QString sliceLayoutLabel(sliceNode->GetLayoutLabel());
  QColor sliceLayoutColor = QColor::fromRgbF(sliceNode->GetLayoutColor()[0],
                                             sliceNode->GetLayoutColor()[1],
                                             sliceNode->GetLayoutColor()[2]);
  sliceWidget->setSliceViewName(sliceLayoutName);
  sliceWidget->setObjectName(QString("qMRMLSliceWidget" + sliceLayoutName));
  sliceWidget->setSliceViewLabel(sliceLayoutLabel);
  sliceWidget->setSliceViewColor(sliceLayoutColor);
  sliceWidget->setMRMLScene(this->MRMLScene);
  sliceWidget->setMRMLSliceNode(sliceNode);
  sliceWidget->setSliceLogics(this->MRMLSliceLogics);

  this->SliceWidgetList.push_back(sliceWidget);
  this->MRMLSliceLogics->AddItem(sliceWidget->sliceLogic());
  //qDebug() << "qMRMLLayoutManagerPrivate::createSliceWidget - Instantiated qMRMLSliceWidget"
  //         << sliceLayoutName;
  return sliceWidget;
}

// --------------------------------------------------------------------------
qMRMLThreeDWidget* qMRMLLayoutManagerPrivate::createThreeDWidget(vtkMRMLViewNode* viewNode)
{
  Q_Q(qMRMLLayoutManager);
  if (!q->viewport() || !this->MRMLScene || !viewNode)
    {
    Q_ASSERT(this->MRMLScene);
    Q_ASSERT(viewNode);
    return 0;
    }

  // There must be a unique ThreeDWidget per node
  Q_ASSERT(!this->threeDWidget(viewNode));

  qMRMLThreeDWidget* threeDWidget = new qMRMLThreeDWidget(q->viewport());
  threeDWidget->setViewLabel(viewNode->GetViewLabel());
  threeDWidget->setMRMLScene(this->MRMLScene);
  threeDWidget->setMRMLViewNode(viewNode);

  this->ThreeDWidgetList.push_back(threeDWidget);
  //qDebug() << "qMRMLLayoutManagerPrivate::createThreeDWidget - Instantiated qMRMLThreeDWidget";
  return threeDWidget;
}

// --------------------------------------------------------------------------
qMRMLChartWidget* qMRMLLayoutManagerPrivate::createChartWidget(vtkMRMLChartViewNode* viewNode)
{
  Q_Q(qMRMLLayoutManager);
  if (!q->viewport() || !this->MRMLScene || !viewNode)
    {
    Q_ASSERT(this->MRMLScene);
    Q_ASSERT(viewNode);
    return 0;
    }

  // There must be a unique ChartWidget per node
  Q_ASSERT(!this->chartWidget(viewNode));

  qMRMLChartWidget* chartWidget = 0;

  //logger.trace("createChartWidget - instantiated new qMRMLChartWidget");
  chartWidget = new qMRMLChartWidget(q->viewport());
  chartWidget->setViewLabel(viewNode->GetViewLabel());
  chartWidget->setColorLogic(this->MRMLColorLogic);
  chartWidget->setMRMLScene(this->MRMLScene);
  chartWidget->setMRMLChartViewNode(viewNode);

  this->ChartWidgetList.push_back(chartWidget);

  return chartWidget;
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
void qMRMLLayoutManagerPrivate::removeThreeDWidget(vtkMRMLViewNode* viewNode)
{
  Q_ASSERT(viewNode);
  qMRMLThreeDWidget * threeDWidgetToDelete = this->threeDWidget(viewNode);

  // Remove threeDView
  if (threeDWidgetToDelete)
    {
    this->ThreeDWidgetList.removeAll(threeDWidgetToDelete);
    delete threeDWidgetToDelete;
    }
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::removeChartWidget(vtkMRMLChartViewNode* viewNode)
{
  Q_ASSERT(viewNode);
  qMRMLChartWidget * chartWidgetToDelete = this->chartWidget(viewNode);

  // Remove threeDView
  if (chartWidgetToDelete)
    {
    this->ChartWidgetList.removeAll(chartWidgetToDelete);
    delete chartWidgetToDelete;
    }
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::onNodeAddedEvent(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  Q_ASSERT(scene);
  Q_ASSERT(scene == this->MRMLScene);
  if (!this->MRMLScene || this->MRMLScene->IsBatchProcessing())
    {
    return;
    }

  // Layout node
  vtkMRMLLayoutNode * layoutNode = vtkMRMLLayoutNode::SafeDownCast(node);
  if (layoutNode)
    {
    //qDebug() << "qMRMLLayoutManagerPrivate::onLayoutNodeAddedEvent";
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
    //qDebug() << "qMRMLLayoutManagerPrivate::onViewNodeAddedEvent - id:" << viewNode->GetID();
    if (!this->threeDWidget(viewNode))
      {
      this->createThreeDWidget(viewNode);
      }
    }

  // Chart-View node
  vtkMRMLChartViewNode* chartViewNode = vtkMRMLChartViewNode::SafeDownCast(node);
  if (chartViewNode)
    {
    //logger.trace(QString("onChartViewNodeAddedEvent - id: %1").arg(chartViewNode->GetID()));
    if (!this->chartWidget(chartViewNode))
      {
      this->createChartWidget(chartViewNode);
      }
    }

  // Slice node
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  if (sliceNode)
    {
    //QString layoutName = sliceNode->GetLayoutName();
    //qDebug() << "qMRMLLayoutManagerPrivate::onSliceNodeAddedEvent - layoutName:" << layoutName;
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
    this->removeThreeDWidget(viewNode);
    }

  // View node
  vtkMRMLChartViewNode* chartViewNode = vtkMRMLChartViewNode::SafeDownCast(node);
  if (chartViewNode)
    {
    if (chartViewNode == this->ActiveMRMLChartViewNode)
      {
      this->setActiveMRMLChartViewNode(0);
      }
    this->removeChartWidget(chartViewNode);
    }

  // Slice node
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  if (sliceNode)
    {
    this->removeSliceView(sliceNode);
    }
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::onSceneRestoredEvent()
{
  //qDebug() << "qMRMLLayoutManagerPrivate::onSceneRestoredEvent";

  if (this->MRMLLayoutNode)
    {
    // trigger an update to the layout
    this->MRMLLayoutNode->Modified();
    }
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::onSceneAboutToBeClosedEvent()
{
  Q_Q(qMRMLLayoutManager);
  //qDebug() << "qMRMLLayoutManagerPrivate::onSceneAboutToBeClosedEvent";
  // remove the layout during closing.
  q->clearLayout();
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::onSceneClosedEvent()
{
  //qDebug() << "qMRMLLayoutManagerPrivate::onSceneClosedEvent";
  if (this->MRMLScene->IsBatchProcessing())
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
  if (!this->MRMLScene || this->MRMLScene->IsBatchProcessing())
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
    if (!this->threeDWidget(viewNode))
      {
      this->createThreeDWidget(viewNode);
      }
    // For now, the active view is the first one
    if (i == 0)
      {
      this->setActiveMRMLThreeDViewNode(viewNode);
      }
    }

  // Maybe the chart-nodes have been created a while ago, we need to associate a chart-view to each of them
  std::vector<vtkMRMLNode*> chartViewNodes;
  this->MRMLScene->GetNodesByClass("vtkMRMLChartViewNode", chartViewNodes);
  for (unsigned int i = 0; i < chartViewNodes.size(); ++i)
    {
    vtkMRMLChartViewNode* chartViewNode = vtkMRMLChartViewNode::SafeDownCast(chartViewNodes[i]);
    if (!this->chartWidget(chartViewNode))
      {
      this->createChartWidget(chartViewNode);
      }
    // For now, the active view is the first one
    if (i == 0)
      {
      this->setActiveMRMLChartViewNode(chartViewNode);
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
    this->MRMLLayoutNode->GetCurrentLayoutDescription() : ""));
  q->setLayout(newLayout);

  emit q->layoutChanged(layout);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setLayoutNumberOfCompareViewRowsInternal(int num)
{
  // Set the number of viewers on the layout node. This will trigger a
  // callback to in qMRMLLayoutLogic to redefine the layouts for the
  // comparison modes.

  //Q_Q(qMRMLLayoutManager);
  // Update LayoutNode
  if (this->MRMLLayoutNode)
    {
    this->MRMLLayoutNode->SetNumberOfCompareViewRows(num);
    }
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setLayoutNumberOfCompareViewColumnsInternal(int num)
{
  // Set the number of viewers on the layout node. This will trigger a
  // callback to in qMRMLLayoutLogic to redefine the layouts for the
  // comparison modes.

  //Q_Q(qMRMLLayoutManager);
  // Update LayoutNode
  if (this->MRMLLayoutNode)
    {
    this->MRMLLayoutNode->SetNumberOfCompareViewColumns(num);
    }
}

//------------------------------------------------------------------------------
vtkMRMLLayoutLogic::ViewAttributes qMRMLLayoutManagerPrivate::attributesFromXML(QDomElement viewElement)const
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
vtkMRMLLayoutLogic::ViewProperties qMRMLLayoutManagerPrivate::propertiesFromXML(QDomElement viewElement)const
{
  vtkMRMLLayoutLogic::ViewProperties properties;
  for (QDomElement childElement = viewElement.firstChildElement();
       !childElement.isNull();
       childElement = childElement.nextSiblingElement())
    {
    properties.push_back(this->propertyFromXML(childElement));
    }
  return properties;
}

//------------------------------------------------------------------------------
vtkMRMLLayoutLogic::ViewProperty qMRMLLayoutManagerPrivate::propertyFromXML(QDomElement propertyElement)const
{
  vtkMRMLLayoutLogic::ViewProperty property;
  QDomNamedNodeMap elementAttributes = propertyElement.attributes();
  const int attributeCount = elementAttributes.count();
  for (int i = 0; i < attributeCount; ++i)
    {
    QDomNode attribute = elementAttributes.item(i);
    property[attribute.nodeName().toStdString()] =
      propertyElement.attribute(attribute.nodeName()).toStdString();
    }
  property["value"] = propertyElement.text().toStdString();
  return property;
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
  return d->ThreeDWidgetList.size();
}

//------------------------------------------------------------------------------
int qMRMLLayoutManager::chartViewCount()const
{
  Q_D(const qMRMLLayoutManager);
  return d->ChartWidgetList.size();
}

//------------------------------------------------------------------------------
qMRMLThreeDWidget* qMRMLLayoutManager::threeDWidget(int id)const
{
  Q_D(const qMRMLLayoutManager);
  if(id < 0 || id >= d->ThreeDWidgetList.size())
    {
    return 0;
    }
  return d->threeDWidget(id);
}

//------------------------------------------------------------------------------
qMRMLChartWidget* qMRMLLayoutManager::chartWidget(int id)const
{
  Q_D(const qMRMLLayoutManager);
  if(id < 0 || id >= d->ChartWidgetList.size())
    {
    return 0;
    }
  return d->chartWidget(id);
}

//------------------------------------------------------------------------------
vtkCollection* qMRMLLayoutManager::mrmlSliceLogics()const
{
  Q_D(const qMRMLLayoutManager);
  return d->MRMLSliceLogics;
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::setMRMLColorLogic(vtkMRMLColorLogic* colorLogic)
{
  Q_D(qMRMLLayoutManager);
  d->MRMLColorLogic = colorLogic;
  /// TODO: apply color logic to existing chart views
}

//------------------------------------------------------------------------------
vtkMRMLColorLogic* qMRMLLayoutManager::mrmlColorLogic()const
{
  Q_D(const qMRMLLayoutManager);
  return d->MRMLColorLogic;
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

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::EndBatchProcessEvent,
                   d, SLOT(updateWidgetsFromViewNodes()));
  // We want to connect the logic to the scene first (before the following
  // qvtkReconnect); that way, anytime the scene is modified, the logic
  // callbacks will be called  before qMRMLLayoutManager and keep the scene
  // in a good state
  d->MRMLLayoutLogic->SetMRMLScene(d->MRMLScene);

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::NodeAddedEvent,
                   d, SLOT(onNodeAddedEvent(vtkObject*,vtkObject*)));

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::NodeRemovedEvent,
                   d, SLOT(onNodeRemovedEvent(vtkObject*,vtkObject*)));

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::EndBatchProcessEvent,
                   d, SLOT(updateLayoutFromMRMLScene()));

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::EndRestoreEvent,
                   d, SLOT(onSceneRestoredEvent()));

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::StartCloseEvent,
                   d, SLOT(onSceneAboutToBeClosedEvent()));

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::EndCloseEvent,
                   d, SLOT(onSceneClosedEvent()));

  // update all the slice views and the 3D views
  foreach (qMRMLSliceWidget* sliceWidget, d->SliceWidgetList )
    {
    sliceWidget->setMRMLScene(d->MRMLScene);
    }
  foreach (qMRMLThreeDWidget* threeDWidget, d->ThreeDWidgetList )
    {
    threeDWidget->setMRMLScene(d->MRMLScene);
    }
  foreach (qMRMLChartWidget* chartWidget, d->ChartWidgetList )
    {
    chartWidget->setMRMLScene(d->MRMLScene);
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
CTK_GET_CPP(qMRMLLayoutManager, vtkMRMLChartViewNode*,
            activeMRMLChartViewNode, ActiveMRMLChartViewNode)

//------------------------------------------------------------------------------
vtkRenderer* qMRMLLayoutManager::activeThreeDRenderer()const
{
  Q_D(const qMRMLLayoutManager);
  qMRMLThreeDWidget* activeThreeDWidget = d->threeDWidget(d->ActiveMRMLThreeDViewNode);
  return activeThreeDWidget ? activeThreeDWidget->threeDView()->renderer() : 0;
}

//------------------------------------------------------------------------------
vtkRenderer* qMRMLLayoutManager::activeChartRenderer()const
{
  return 0;
  // Q_D(const qMRMLLayoutManager);
  // qMRMLChartWidget* activeChartWidget = d->chartWidget(d->ActiveMRMLChartViewNode);
  // return activeChartWidget ? activeChartWidget->chartView()->renderer() : 0;
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
void qMRMLLayoutManager::setLayoutNumberOfCompareViewRows(int num)
{
  Q_D(qMRMLLayoutManager);

  d->setLayoutNumberOfCompareViewRowsInternal(num);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::setLayoutNumberOfCompareViewColumns(int num)
{
  Q_D(qMRMLLayoutManager);

  d->setLayoutNumberOfCompareViewColumnsInternal(num);
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
  vtkMRMLLayoutLogic::ViewProperties properties = d->propertiesFromXML(viewElement);
  d->MRMLLayoutLogic->ApplyProperties(properties, viewNode, "relayout");
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


//------------------------------------------------------------------------------
QWidget* qMRMLLayoutManager::viewWidget(vtkMRMLNode* viewNode) const
{
  Q_D(const qMRMLLayoutManager);

  return d->viewWidget(viewNode);
}

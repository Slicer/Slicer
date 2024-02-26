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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QButtonGroup>
#include <QApplication>
#include <QDebug>
#include <QDockWidget>
#include <QMainWindow>
#include <QVBoxLayout>

// MRMLWidgets includes
#include <qMRMLWidgetsConfigure.h> // For MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
#include <qMRMLLayoutManager_p.h>
#include <qMRMLSliceView.h>
#include <qMRMLSliceWidget.h>
#include <qMRMLSliceControllerWidget.h>
#include <qMRMLTableView.h>
#include <qMRMLTableWidget.h>
#include <qMRMLPlotView.h>
#include <qMRMLPlotWidget.h>
#include <qMRMLThreeDView.h>
#include <qMRMLThreeDWidget.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLColorLogic.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLViewLogic.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>

// MRML includes
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTableViewNode.h>
#include <vtkMRMLPlotViewNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkCollection.h>

//------------------------------------------------------------------------------
// Factory methods

//------------------------------------------------------------------------------
// qMRMLLayoutThreeDViewFactory
//------------------------------------------------------------------------------
qMRMLLayoutThreeDViewFactory::qMRMLLayoutThreeDViewFactory(QObject* parent)
  : qMRMLLayoutViewFactory(parent)
{
  this->ViewLogics = vtkCollection::New();
}

//------------------------------------------------------------------------------
qMRMLLayoutThreeDViewFactory::~qMRMLLayoutThreeDViewFactory()
{
  this->ViewLogics->Delete();
  this->ViewLogics = nullptr;
}

//------------------------------------------------------------------------------
QString qMRMLLayoutThreeDViewFactory::viewClassName() const
{
  return "vtkMRMLViewNode";
}

//------------------------------------------------------------------------------
vtkCollection* qMRMLLayoutThreeDViewFactory::viewLogics() const
{
  return this->ViewLogics;
}

//------------------------------------------------------------------------------
QWidget* qMRMLLayoutThreeDViewFactory::createViewFromNode(vtkMRMLAbstractViewNode* viewNode)
{
  if (!viewNode || !this->layoutManager() || !this->layoutManager()->viewport())
  {
    Q_ASSERT(viewNode);
    return nullptr;
  }

  // There must be a unique ThreeDWidget per node
  Q_ASSERT(!this->viewWidget(viewNode));

  qMRMLThreeDWidget* threeDWidget = new qMRMLThreeDWidget(this->layoutManager()->viewport());
  threeDWidget->setObjectName(QString("ThreeDWidget%1").arg(viewNode->GetLayoutName()));
  threeDWidget->setMRMLScene(this->mrmlScene());
  threeDWidget->setMRMLViewNode(vtkMRMLViewNode::SafeDownCast(viewNode));

  this->viewLogics()->AddItem(threeDWidget->viewLogic());

  return threeDWidget;
}

//------------------------------------------------------------------------------
void qMRMLLayoutThreeDViewFactory::deleteView(vtkMRMLAbstractViewNode* viewNode)
{
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(this->viewWidget(viewNode));
  if (threeDWidget)
  {
    this->viewLogics()->RemoveItem(threeDWidget->viewLogic());
  }
  this->Superclass::deleteView(viewNode);
}

//------------------------------------------------------------------------------
// qMRMLLayoutTableViewFactory
//------------------------------------------------------------------------------
qMRMLLayoutTableViewFactory::qMRMLLayoutTableViewFactory(QObject* parent)
  : qMRMLLayoutViewFactory(parent)
{
}

//------------------------------------------------------------------------------
QString qMRMLLayoutTableViewFactory::viewClassName() const
{
  return "vtkMRMLTableViewNode";
}

//------------------------------------------------------------------------------
QWidget* qMRMLLayoutTableViewFactory::createViewFromNode(vtkMRMLAbstractViewNode* viewNode)
{
  if (!this->layoutManager() || !viewNode || !this->layoutManager()->viewport())
  {
    Q_ASSERT(viewNode);
    return nullptr;
  }

  // There must be a unique TableWidget per node
  Q_ASSERT(!this->viewWidget(viewNode));

  qMRMLTableWidget* tableWidget = new qMRMLTableWidget(this->layoutManager()->viewport());
  tableWidget->setObjectName(QString("qMRMLTableWidget%1").arg(viewNode->GetLayoutName()));
  tableWidget->setMRMLScene(this->mrmlScene());
  tableWidget->setMRMLTableViewNode(vtkMRMLTableViewNode::SafeDownCast(viewNode));

  return tableWidget;
}

//------------------------------------------------------------------------------
// qMRMLLayoutPlotViewFactory
//------------------------------------------------------------------------------
qMRMLLayoutPlotViewFactory::qMRMLLayoutPlotViewFactory(QObject* parent)
  : qMRMLLayoutViewFactory(parent)
{
}

//------------------------------------------------------------------------------
QString qMRMLLayoutPlotViewFactory::viewClassName() const
{
  return "vtkMRMLPlotViewNode";
}

//------------------------------------------------------------------------------
vtkMRMLColorLogic* qMRMLLayoutPlotViewFactory::colorLogic() const
{
  return this->ColorLogic;
}

//------------------------------------------------------------------------------
void qMRMLLayoutPlotViewFactory::setColorLogic(vtkMRMLColorLogic* colorLogic)
{
  this->ColorLogic = colorLogic;
  /*
  foreach(QWidget* view, this->registeredViews())
    {
    qMRMLPlotWidget* plotWidget = qobject_cast<qMRMLPlotWidget*>(view);
    plotWidget->setColorLogic(this->colorLogic());
    }
  */
}

//------------------------------------------------------------------------------
QWidget* qMRMLLayoutPlotViewFactory::createViewFromNode(vtkMRMLAbstractViewNode* viewNode)
{
  if (!this->layoutManager() || !viewNode || !this->layoutManager()->viewport())
  {
    Q_ASSERT(viewNode);
    return nullptr;
  }

  // There must be a unique plot widget per node
  Q_ASSERT(!this->viewWidget(viewNode));

  qMRMLPlotWidget* plotWidget = new qMRMLPlotWidget(this->layoutManager()->viewport());
  plotWidget->setObjectName(QString("qMRMLPlotWidget%1").arg(viewNode->GetLayoutName()));
  plotWidget->setMRMLScene(this->mrmlScene());
  plotWidget->setMRMLPlotViewNode(vtkMRMLPlotViewNode::SafeDownCast(viewNode));

  return plotWidget;
}

//------------------------------------------------------------------------------
// qMRMLLayoutSliceViewFactory
//------------------------------------------------------------------------------
qMRMLLayoutSliceViewFactory::qMRMLLayoutSliceViewFactory(QObject* parent)
  : qMRMLLayoutViewFactory(parent)
{
  this->SliceControllerButtonGroup = new QButtonGroup(nullptr);
  this->SliceControllerButtonGroup->setParent(this);
  this->SliceControllerButtonGroup->setExclusive(false);
  this->SliceLogics = vtkCollection::New();
}

//------------------------------------------------------------------------------
qMRMLLayoutSliceViewFactory::~qMRMLLayoutSliceViewFactory()
{
  this->SliceLogics->Delete();
  this->SliceLogics = nullptr;
}

// --------------------------------------------------------------------------
QString qMRMLLayoutSliceViewFactory::viewClassName() const
{
  return "vtkMRMLSliceNode";
}

//------------------------------------------------------------------------------
vtkCollection* qMRMLLayoutSliceViewFactory::sliceLogics() const
{
  return this->SliceLogics;
}

// --------------------------------------------------------------------------
QWidget* qMRMLLayoutSliceViewFactory::createViewFromNode(vtkMRMLAbstractViewNode* viewNode)
{
  if (!this->layoutManager() || !viewNode || !this->layoutManager()->viewport())
  { // can't create a slice widget if there is no parent widget
    Q_ASSERT(viewNode);
    return nullptr;
  }

  // there is a unique slice widget per node
  Q_ASSERT(!this->viewWidget(viewNode));

  qMRMLSliceWidget* sliceWidget = new qMRMLSliceWidget(this->layoutManager()->viewport());

  // Set slice logic before setting the slice node in the widget
  // to allow displayable managers to use the slice logic during initialization
  // For example, without this the color legend displayable manager would not be able to correctly
  // initialize new views when switching to a new view layout that has more slice views.
  sliceWidget->setSliceLogics(this->sliceLogics());
  this->sliceLogics()->AddItem(sliceWidget->sliceLogic());

  sliceWidget->sliceController()->setControllerButtonGroup(this->SliceControllerButtonGroup);
  sliceWidget->setObjectName(QString("qMRMLSliceWidget%1").arg(viewNode->GetLayoutName()));
  // set slice node before setting the scene to allow using slice node names in the slice transform, display, and model
  // nodes
  sliceWidget->setMRMLSliceNode(vtkMRMLSliceNode::SafeDownCast(viewNode));
  sliceWidget->setMRMLScene(this->mrmlScene());

  return sliceWidget;
}

// --------------------------------------------------------------------------
void qMRMLLayoutSliceViewFactory::deleteView(vtkMRMLAbstractViewNode* viewNode)
{
  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(this->viewWidget(viewNode));
  if (sliceWidget)
  {
    this->sliceLogics()->RemoveItem(sliceWidget->sliceLogic());
  }
  this->Superclass::deleteView(viewNode);
}

//------------------------------------------------------------------------------
// qMRMLLayoutManagerPrivate methods

//------------------------------------------------------------------------------
qMRMLLayoutManagerPrivate::qMRMLLayoutManagerPrivate(qMRMLLayoutManager& object)
  : q_ptr(&object)
{
  this->Enabled = true;
  this->MRMLScene = nullptr;
  this->MRMLLayoutNode = nullptr;
  this->MRMLLayoutLogic = vtkMRMLLayoutLogic::New();
  this->ActiveMRMLThreeDViewNode = nullptr;
  this->ActiveMRMLTableViewNode = nullptr;
  this->ActiveMRMLPlotViewNode = nullptr;
  // this->SavedCurrentViewArrangement = vtkMRMLLayoutNode::SlicerLayoutNone;
}

//------------------------------------------------------------------------------
qMRMLLayoutManagerPrivate::~qMRMLLayoutManagerPrivate()
{
  Q_Q(qMRMLLayoutManager);
  this->MRMLLayoutLogic->Delete();
  this->MRMLLayoutLogic = nullptr;
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::init()
{
  Q_Q(qMRMLLayoutManager);

  q->setSpacing(1);

  qMRMLLayoutThreeDViewFactory* threeDViewFactory = new qMRMLLayoutThreeDViewFactory;
  q->registerViewFactory(threeDViewFactory);

  qMRMLLayoutSliceViewFactory* sliceViewFactory = new qMRMLLayoutSliceViewFactory;
  q->registerViewFactory(sliceViewFactory);

  qMRMLLayoutTableViewFactory* tableViewFactory = new qMRMLLayoutTableViewFactory;
  q->registerViewFactory(tableViewFactory);

  qMRMLLayoutPlotViewFactory* plotViewFactory = new qMRMLLayoutPlotViewFactory;
  q->registerViewFactory(plotViewFactory);
}

//------------------------------------------------------------------------------
qMRMLThreeDWidget* qMRMLLayoutManagerPrivate::threeDWidget(vtkMRMLViewNode* node) const
{
  Q_Q(const qMRMLLayoutManager);
  return qobject_cast<qMRMLThreeDWidget*>(q->mrmlViewFactory("vtkMRMLViewNode")->viewWidget(node));
}

//------------------------------------------------------------------------------
qMRMLSliceWidget* qMRMLLayoutManagerPrivate::sliceWidget(vtkMRMLSliceNode* node) const
{
  Q_Q(const qMRMLLayoutManager);
  return qobject_cast<qMRMLSliceWidget*>(q->mrmlViewFactory("vtkMRMLSliceNode")->viewWidget(node));
}

//------------------------------------------------------------------------------
qMRMLTableWidget* qMRMLLayoutManagerPrivate::tableWidget(vtkMRMLTableViewNode* node) const
{
  Q_Q(const qMRMLLayoutManager);
  return qobject_cast<qMRMLTableWidget*>(q->mrmlViewFactory("vtkMRMLTableViewNode")->viewWidget(node));
}

//------------------------------------------------------------------------------
qMRMLPlotWidget* qMRMLLayoutManagerPrivate::plotWidget(vtkMRMLPlotViewNode* node) const
{
  Q_Q(const qMRMLLayoutManager);
  return qobject_cast<qMRMLPlotWidget*>(q->mrmlViewFactory("vtkMRMLPlotViewNode")->viewWidget(node));
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLLayoutManagerPrivate::viewNode(QWidget* widget) const
{
  if (qobject_cast<qMRMLSliceWidget*>(widget))
  {
    return qobject_cast<qMRMLSliceWidget*>(widget)->mrmlSliceNode();
  }
  if (qobject_cast<qMRMLThreeDWidget*>(widget))
  {
    return qobject_cast<qMRMLThreeDWidget*>(widget)->mrmlViewNode();
  }
  if (qobject_cast<qMRMLTableWidget*>(widget))
  {
    return qobject_cast<qMRMLTableWidget*>(widget)->mrmlTableViewNode();
  }
  if (qobject_cast<qMRMLPlotWidget*>(widget))
  {
    return qobject_cast<qMRMLPlotWidget*>(widget)->mrmlPlotViewNode();
  }
  return nullptr;
}

//------------------------------------------------------------------------------
QWidget* qMRMLLayoutManagerPrivate::viewWidget(vtkMRMLNode* viewNode) const
{
  Q_Q(const qMRMLLayoutManager);
  if (!vtkMRMLAbstractViewNode::SafeDownCast(viewNode))
  {
    return nullptr;
  }
  QWidget* widget = nullptr;
  if (vtkMRMLSliceNode::SafeDownCast(viewNode))
  {
    widget = this->sliceWidget(vtkMRMLSliceNode::SafeDownCast(viewNode));
  }
  if (vtkMRMLViewNode::SafeDownCast(viewNode))
  {
    widget = this->threeDWidget(vtkMRMLViewNode::SafeDownCast(viewNode));
  }
  if (vtkMRMLTableViewNode::SafeDownCast(viewNode))
  {
    widget = this->tableWidget(vtkMRMLTableViewNode::SafeDownCast(viewNode));
  }
  if (vtkMRMLPlotViewNode::SafeDownCast(viewNode))
  {
    widget = this->plotWidget(vtkMRMLPlotViewNode::SafeDownCast(viewNode));
  }
  return widget ? widget
                : q->mrmlViewFactory(QString::fromUtf8(viewNode->GetClassName()))
                    ->viewWidget(vtkMRMLAbstractViewNode::SafeDownCast(viewNode));
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setMRMLLayoutNode(vtkMRMLLayoutNode* layoutNode)
{
  this->qvtkReconnect(
    this->MRMLLayoutNode, layoutNode, vtkCommand::ModifiedEvent, this, SLOT(onLayoutNodeModifiedEvent(vtkObject*)));
  this->MRMLLayoutNode = layoutNode;
  this->onLayoutNodeModifiedEvent(layoutNode);
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setActiveMRMLThreeDViewNode(vtkMRMLViewNode* node)
{
  Q_Q(qMRMLLayoutManager);
  QObject::connect(q->mrmlViewFactory("vtkMRMLViewNode"),
                   SIGNAL(activeViewNodeChanged(vtkMRMLAbstractViewNode*)),
                   this,
                   SLOT(onActiveThreeDViewNodeChanged(vtkMRMLAbstractViewNode*)),
                   Qt::UniqueConnection);

  q->mrmlViewFactory("vtkMRMLViewNode")->setActiveViewNode(node);
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate ::onActiveThreeDViewNodeChanged(vtkMRMLAbstractViewNode* node)
{
  Q_Q(qMRMLLayoutManager);
  emit q->activeThreeDRendererChanged(q->mrmlViewFactory("vtkMRMLViewNode")->activeRenderer());
  emit q->activeMRMLThreeDViewNodeChanged(vtkMRMLViewNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setActiveMRMLTableViewNode(vtkMRMLTableViewNode* node)
{
  Q_Q(qMRMLLayoutManager);
  QObject::connect(q->mrmlViewFactory("vtkMRMLTableViewNode"),
                   SIGNAL(activeViewNodeChanged(vtkMRMLAbstractViewNode*)),
                   this,
                   SLOT(onActiveTableViewNodeChanged(vtkMRMLAbstractViewNode*)),
                   Qt::UniqueConnection);
  q->mrmlViewFactory("vtkMRMLTableViewNode")->setActiveViewNode(node);
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate ::onActiveTableViewNodeChanged(vtkMRMLAbstractViewNode* node)
{
  Q_Q(qMRMLLayoutManager);
  emit q->activeTableRendererChanged(q->mrmlViewFactory("vtkMRMLTableViewNode")->activeRenderer());
  emit q->activeMRMLTableViewNodeChanged(vtkMRMLTableViewNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setActiveMRMLPlotViewNode(vtkMRMLPlotViewNode* node)
{
  Q_Q(qMRMLLayoutManager);
  QObject::connect(q->mrmlViewFactory("vtkMRMLPlotViewNode"),
                   SIGNAL(activeViewNodeChanged(vtkMRMLAbstractViewNode*)),
                   this,
                   SLOT(onActivePlotViewNodeChanged(vtkMRMLAbstractViewNode*)),
                   Qt::UniqueConnection);
  q->mrmlViewFactory("vtkMRMLPlotViewNode")->setActiveViewNode(node);
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::onActivePlotViewNodeChanged(vtkMRMLAbstractViewNode* node)
{
  Q_Q(qMRMLLayoutManager);
  emit q->activePlotRendererChanged(q->mrmlViewFactory("vtkMRMLPlotViewNode")->activeRenderer());
  emit q->activeMRMLPlotViewNodeChanged(vtkMRMLPlotViewNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::onNodeAddedEvent(vtkObject* scene, vtkObject* node)
{
  Q_Q(qMRMLLayoutManager);
  Q_UNUSED(scene);
  Q_ASSERT(scene);
  Q_ASSERT(scene == this->MRMLScene);
  if (!this->MRMLScene || this->MRMLScene->IsBatchProcessing())
  {
    return;
  }

  // Layout node added
  vtkMRMLLayoutNode* layoutNode = vtkMRMLLayoutNode::SafeDownCast(node);
  if (layoutNode)
  {
    // qDebug() << "qMRMLLayoutManagerPrivate::onLayoutNodeAddedEvent";
    //  Only one Layout node is expected
    Q_ASSERT(this->MRMLLayoutNode == nullptr);
    if (this->MRMLLayoutNode != nullptr)
    {
      return;
    }
    this->setMRMLLayoutNode(layoutNode);
  }

  // View node added
  vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(node);
  if (viewNode)
  {
    // No explicit parent layout node means that view is handled by the main Slicer layout
    if (!viewNode->GetParentLayoutNode())
    {
      foreach (qMRMLLayoutViewFactory* mrmlViewFactory, q->mrmlViewFactories())
      {
        mrmlViewFactory->onViewNodeAdded(viewNode);
      }
    }
  }
  else if (node->IsA("vtkMRMLSegmentationNode"))
  {
    // Show segmentation section in slice view controller if the first segmentation
    // node has been added to the scene
    this->updateSegmentationControls();
  }
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::onNodeRemovedEvent(vtkObject* scene, vtkObject* node)
{
  Q_Q(qMRMLLayoutManager);
  Q_UNUSED(scene);
  Q_ASSERT(scene);
  Q_ASSERT(scene == this->MRMLScene);
  // Layout node
  vtkMRMLLayoutNode* layoutNode = vtkMRMLLayoutNode::SafeDownCast(node);
  if (layoutNode)
  {
    // The layout to be removed should be the same as the stored one
    Q_ASSERT(this->MRMLLayoutNode == layoutNode);
    this->setMRMLLayoutNode(nullptr);
  }
  vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(node);
  if (viewNode)
  {
    foreach (qMRMLLayoutViewFactory* mrmlViewFactory, q->mrmlViewFactories())
    {
      mrmlViewFactory->onViewNodeRemoved(viewNode);
    }
  }
  else if (node->IsA("vtkMRMLSegmentationNode"))
  {
    this->updateSegmentationControls();
  }
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::onSceneRestoredEvent()
{
  // qDebug() << "qMRMLLayoutManagerPrivate::onSceneRestoredEvent";

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
  if (!this->Enabled)
  {
    return;
  }
  // remove the layout during closing.
  q->clearLayout();
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::onSceneClosedEvent()
{
  // qDebug() << "qMRMLLayoutManagerPrivate::onSceneClosedEvent";
  if (this->MRMLScene->IsBatchProcessing())
  {
    // Some more processing on the scene is happening, let's just wait until it
    // finishes.
    return;
  }

  // Since the loaded scene may not contain the required nodes, calling
  // initialize will make sure the LayoutNode, MRMLViewNode,
  // MRMLSliceNode exists.
  this->updateLayoutFromMRMLScene();
  Q_ASSERT(this->MRMLLayoutNode);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::onLayoutNodeModifiedEvent(vtkObject* vtkNotUsed(layoutNode))
{
  if (!this->MRMLScene || this->MRMLScene->IsBatchProcessing() || !this->Enabled)
  {
    return;
  }
  this->updateLayoutInternal();
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::updateLayoutFromMRMLScene()
{
  Q_Q(qMRMLLayoutManager);
  foreach (qMRMLLayoutViewFactory* viewFactory, q->mrmlViewFactories())
  {
    viewFactory->onSceneModified();
  }
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
void qMRMLLayoutManagerPrivate::updateLayoutInternal()
{
  Q_Q(qMRMLLayoutManager);
  int layout = this->MRMLLayoutNode ? this->MRMLLayoutNode->GetViewArrangement() : vtkMRMLLayoutNode::SlicerLayoutNone;

  if (layout == vtkMRMLLayoutNode::SlicerLayoutCustomView)
  {
    return;
  }

  QDomDocument newLayout;

  int numberOfMaximizedViewNodes = (this->MRMLLayoutNode ? this->MRMLLayoutNode->GetNumberOfMaximizedViewNodes() : 0);
  if (numberOfMaximizedViewNodes)
  {
    std::string maximizedLayoutDescription = this->MRMLLayoutNode->GetCurrentLayoutDescription();
    // Rewrite each viewport that has maximized view node.
    for (int maximizedViewNodeIndex = 0; maximizedViewNodeIndex < numberOfMaximizedViewNodes; maximizedViewNodeIndex++)
    {
      vtkMRMLAbstractViewNode* maximizedViewNode = this->MRMLLayoutNode->GetMaximizedViewNode(maximizedViewNodeIndex);
      if (!maximizedViewNode)
      {
        continue;
      }
      maximizedLayoutDescription =
        this->MRMLLayoutLogic->GetMaximizedViewLayoutDescription(maximizedViewNode, maximizedLayoutDescription.c_str());
    }
    newLayout.setContent(QString::fromStdString(maximizedLayoutDescription));
  }
  else
  {
    // Normal (non-maximized view)
    // TBD: modify the dom doc manually, don't create a new one
    newLayout.setContent(QString(this->MRMLLayoutNode ? this->MRMLLayoutNode->GetCurrentLayoutDescription() : ""));
  }

  q->setLayout(newLayout);
  emit q->layoutChanged(layout);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setLayoutNumberOfCompareViewRowsInternal(int num)
{
  // Set the number of viewers on the layout node. This will trigger a
  // callback to in qMRMLLayoutLogic to redefine the layouts for the
  // comparison modes.

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

  // Update LayoutNode
  if (this->MRMLLayoutNode)
  {
    this->MRMLLayoutNode->SetNumberOfCompareViewColumns(num);
  }
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::updateSegmentationControls()
{
  Q_Q(qMRMLLayoutManager);

  foreach (const QString& viewName, q->sliceViewNames())
  {
    q->sliceWidget(viewName)->sliceController()->updateSegmentationControlsVisibility();
  }
}

//------------------------------------------------------------------------------
// qMRMLLayoutManager methods

// --------------------------------------------------------------------------
qMRMLLayoutManager::qMRMLLayoutManager(QObject* parentObject)
  : Superclass(nullptr, parentObject)
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
qMRMLLayoutManager::qMRMLLayoutManager(qMRMLLayoutManagerPrivate* pimpl, QWidget* viewport, QObject* parentObject)
  : Superclass(viewport, parentObject)
  , d_ptr(pimpl)
{
  Q_D(qMRMLLayoutManager);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLLayoutManager::~qMRMLLayoutManager()
{
  Q_D(qMRMLLayoutManager);

  // Erase all views (must happen before deleting viewports)
  foreach (ctkLayoutViewFactory* viewFactory, this->registeredViewFactories())
  {
    qMRMLLayoutViewFactory* mrmlViewFactory = qobject_cast<qMRMLLayoutViewFactory*>(viewFactory);
    if (mrmlViewFactory)
    {
      mrmlViewFactory->setMRMLScene(nullptr);
    }
  }

  // Delete detached viewports if they are not owned by the application mainWindow
  foreach (qMRMLLayoutManagerPrivate::ViewportInfo viewport, d->DetachedViewports)
  {
    if (viewport.Window && !viewport.Window->parent())
    {
      delete viewport.Window;
    }
  }
  d->DetachedViewports.clear();
}

// --------------------------------------------------------------------------
bool qMRMLLayoutManager::isEnabled() const
{
  Q_D(const qMRMLLayoutManager);
  return d->Enabled;
}

// --------------------------------------------------------------------------
void qMRMLLayoutManager::setEnabled(bool enable)
{
  Q_D(qMRMLLayoutManager);
  d->Enabled = enable;
}

// --------------------------------------------------------------------------
QList<qMRMLLayoutViewFactory*> qMRMLLayoutManager::mrmlViewFactories() const
{
  QList<qMRMLLayoutViewFactory*> res;
  foreach (ctkLayoutViewFactory* viewFactory, this->registeredViewFactories())
  {
    qMRMLLayoutViewFactory* mrmlViewFactory = qobject_cast<qMRMLLayoutViewFactory*>(viewFactory);
    if (mrmlViewFactory)
    {
      res << mrmlViewFactory;
    }
  }
  return res;
}

// --------------------------------------------------------------------------
qMRMLLayoutViewFactory* qMRMLLayoutManager ::mrmlViewFactory(const QString& viewClassName) const
{
  foreach (qMRMLLayoutViewFactory* viewFactory, this->mrmlViewFactories())
  {
    if (viewFactory->viewClassName() == viewClassName)
    {
      return viewFactory;
    }
  }
  return nullptr;
}

// --------------------------------------------------------------------------
void qMRMLLayoutManager ::registerViewFactory(ctkLayoutViewFactory* viewFactory)
{
  this->Superclass::registerViewFactory(viewFactory);
  qMRMLLayoutViewFactory* mrmlViewFactory = qobject_cast<qMRMLLayoutViewFactory*>(viewFactory);
  if (mrmlViewFactory)
  {
    mrmlViewFactory->setLayoutManager(this);
    mrmlViewFactory->setMRMLScene(this->mrmlScene());
  }
}

// --------------------------------------------------------------------------
void qMRMLLayoutManager::onViewportChanged()
{
  Q_D(qMRMLLayoutManager);
  d->updateLayoutFromMRMLScene();
  this->Superclass::onViewportChanged();
}

//------------------------------------------------------------------------------
qMRMLSliceWidget* qMRMLLayoutManager::sliceWidget(const QString& name) const
{
  qMRMLLayoutViewFactory* viewFactory = this->mrmlViewFactory("vtkMRMLSliceNode");
  if (!viewFactory)
  {
    return nullptr;
  }
  return qobject_cast<qMRMLSliceWidget*>(viewFactory->viewWidget(name));
}

//------------------------------------------------------------------------------
QStringList qMRMLLayoutManager::sliceViewNames() const
{
  qMRMLLayoutViewFactory* viewFactory = this->mrmlViewFactory("vtkMRMLSliceNode");
  if (!viewFactory)
  {
    return QStringList();
  }
  return viewFactory->viewNodeNames();
}

//------------------------------------------------------------------------------
int qMRMLLayoutManager::threeDViewCount() const
{
  qMRMLLayoutViewFactory* viewFactory = this->mrmlViewFactory("vtkMRMLViewNode");
  if (!viewFactory)
  {
    return 0;
  }
  return viewFactory->viewCount();
}

//------------------------------------------------------------------------------
int qMRMLLayoutManager::tableViewCount() const
{
  qMRMLLayoutViewFactory* viewFactory = this->mrmlViewFactory("vtkMRMLTableViewNode");
  if (!viewFactory)
  {
    return 0;
  }
  return viewFactory->viewCount();
}

//------------------------------------------------------------------------------
int qMRMLLayoutManager::plotViewCount() const
{
  qMRMLLayoutViewFactory* viewFactory = this->mrmlViewFactory("vtkMRMLPlotViewNode");
  if (!viewFactory)
  {
    return 0;
  }
  return viewFactory->viewCount();
}

//------------------------------------------------------------------------------
qMRMLThreeDWidget* qMRMLLayoutManager::threeDWidget(int id) const
{
  qMRMLLayoutViewFactory* viewFactory = this->mrmlViewFactory("vtkMRMLViewNode");
  if (!viewFactory)
  {
    return nullptr;
  }
  return qobject_cast<qMRMLThreeDWidget*>(viewFactory->viewWidget(id));
}

//------------------------------------------------------------------------------
qMRMLThreeDWidget* qMRMLLayoutManager::threeDWidget(const QString& name) const
{
  qMRMLLayoutViewFactory* viewFactory = this->mrmlViewFactory("vtkMRMLViewNode");
  if (!viewFactory)
  {
    return nullptr;
  }
  return qobject_cast<qMRMLThreeDWidget*>(viewFactory->viewWidget(name));
}

//------------------------------------------------------------------------------
qMRMLTableWidget* qMRMLLayoutManager::tableWidget(int id) const
{
  qMRMLLayoutViewFactory* viewFactory = this->mrmlViewFactory("vtkMRMLTableViewNode");
  if (!viewFactory)
  {
    return nullptr;
  }
  return qobject_cast<qMRMLTableWidget*>(viewFactory->viewWidget(id));
}

//------------------------------------------------------------------------------
qMRMLPlotWidget* qMRMLLayoutManager::plotWidget(int id) const
{
  qMRMLLayoutViewFactory* viewFactory = this->mrmlViewFactory("vtkMRMLPlotViewNode");
  if (!viewFactory)
  {
    return nullptr;
  }
  return qobject_cast<qMRMLPlotWidget*>(viewFactory->viewWidget(id));
}

//------------------------------------------------------------------------------
vtkCollection* qMRMLLayoutManager::mrmlSliceLogics() const
{
  qMRMLLayoutSliceViewFactory* viewFactory =
    qobject_cast<qMRMLLayoutSliceViewFactory*>(this->mrmlViewFactory("vtkMRMLSliceNode"));
  if (!viewFactory)
  {
    return nullptr;
  }
  return viewFactory->sliceLogics();
}

//------------------------------------------------------------------------------
vtkCollection* qMRMLLayoutManager::mrmlViewLogics() const
{
  qMRMLLayoutThreeDViewFactory* viewFactory =
    qobject_cast<qMRMLLayoutThreeDViewFactory*>(this->mrmlViewFactory("vtkMRMLViewNode"));
  if (!viewFactory)
  {
    return nullptr;
  }
  return viewFactory->viewLogics();
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::setMRMLColorLogic(vtkMRMLColorLogic* colorLogic)
{
  qMRMLLayoutPlotViewFactory* viewFactory =
    qobject_cast<qMRMLLayoutPlotViewFactory*>(this->mrmlViewFactory("vtkMRMLPlotViewNode"));
  if (!viewFactory)
  {
    return;
  }
  viewFactory->setColorLogic(colorLogic);
}

//------------------------------------------------------------------------------
vtkMRMLColorLogic* qMRMLLayoutManager::mrmlColorLogic() const
{
  qMRMLLayoutPlotViewFactory* viewFactory =
    qobject_cast<qMRMLLayoutPlotViewFactory*>(this->mrmlViewFactory("vtkMRMLPlotViewNode"));
  if (!viewFactory)
  {
    return nullptr;
  }
  return viewFactory->colorLogic();
}

//------------------------------------------------------------------------------
vtkMRMLLayoutLogic* qMRMLLayoutManager::layoutLogic() const
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
  d->MRMLLayoutNode = nullptr;

  // We want to connect the logic to the scene first (before the following
  // qvtkReconnect); that way, anytime the scene is modified, the logic
  // callbacks will be called  before qMRMLLayoutManager and keep the scene
  // in a good state
  d->MRMLLayoutLogic->SetMRMLScene(d->MRMLScene);

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::NodeAddedEvent, d, SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));

  d->qvtkReconnect(
    oldScene, scene, vtkMRMLScene::NodeRemovedEvent, d, SLOT(onNodeRemovedEvent(vtkObject*, vtkObject*)));

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::EndBatchProcessEvent, d, SLOT(updateLayoutFromMRMLScene()));

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::EndBatchProcessEvent, d, SLOT(updateSegmentationControls()));

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::EndRestoreEvent, d, SLOT(onSceneRestoredEvent()));

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::StartCloseEvent, d, SLOT(onSceneAboutToBeClosedEvent()));

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::EndCloseEvent, d, SLOT(onSceneClosedEvent()));

  foreach (qMRMLLayoutViewFactory* viewFactory, this->mrmlViewFactories())
  {
    viewFactory->setMRMLScene(d->MRMLScene);
  }
  d->updateLayoutFromMRMLScene();
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLLayoutManager::mrmlScene() const
{
  Q_D(const qMRMLLayoutManager);
  return d->MRMLScene;
}

//------------------------------------------------------------------------------
vtkMRMLViewNode* qMRMLLayoutManager::activeMRMLThreeDViewNode() const
{
  return vtkMRMLViewNode::SafeDownCast(this->mrmlViewFactory("vtkMRMLViewNode")->activeViewNode());
}

//------------------------------------------------------------------------------
vtkMRMLTableViewNode* qMRMLLayoutManager::activeMRMLTableViewNode() const
{
  return vtkMRMLTableViewNode::SafeDownCast(this->mrmlViewFactory("vtkMRMLTableViewNode")->activeViewNode());
}

//------------------------------------------------------------------------------
vtkMRMLPlotViewNode* qMRMLLayoutManager::activeMRMLPlotViewNode() const
{
  return vtkMRMLPlotViewNode::SafeDownCast(this->mrmlViewFactory("vtkMRMLPlotViewNode")->activeViewNode());
}

//------------------------------------------------------------------------------
vtkRenderer* qMRMLLayoutManager::activeThreeDRenderer() const
{
  return this->mrmlViewFactory("vtkMRMLViewNode")->activeRenderer();
}

//------------------------------------------------------------------------------
vtkRenderer* qMRMLLayoutManager::activeTableRenderer() const
{
  return this->mrmlViewFactory("vtkMRMLTableViewNode")->activeRenderer();
}

//------------------------------------------------------------------------------
vtkRenderer* qMRMLLayoutManager::activePlotRenderer() const
{
  return this->mrmlViewFactory("vtkMRMLPlotViewNode")->activeRenderer();
}

//------------------------------------------------------------------------------
int qMRMLLayoutManager::layout() const
{
  Q_D(const qMRMLLayoutManager);
  return d->MRMLLayoutNode ? d->MRMLLayoutNode->GetViewArrangement() : vtkMRMLLayoutNode::SlicerLayoutNone;
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::setLayout(int layout)
{
  Q_D(qMRMLLayoutManager);
  if (this->layout() == layout)
  {
    return;
  }
  // Update LayoutNode
  if (d->MRMLLayoutNode)
  {
    if (!d->MRMLLayoutNode->IsLayoutDescription(layout))
    {
      layout = vtkMRMLLayoutNode::SlicerLayoutConventionalView;
    }
    d->MRMLLayoutNode->RemoveAllMaximizedViewNodes();
    d->MRMLLayoutNode->SetViewArrangement(layout);
  }
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::addMaximizedViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  Q_D(qMRMLLayoutManager);
  if (!d->MRMLLayoutNode)
  {
    return;
  }
  d->MRMLLayoutNode->AddMaximizedViewNode(viewNode);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::removeMaximizedViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  Q_D(qMRMLLayoutManager);
  if (!d->MRMLLayoutNode)
  {
    return;
  }
  d->MRMLLayoutNode->RemoveMaximizedViewNode(viewNode);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::removeAllMaximizedViewNodes()
{
  Q_D(qMRMLLayoutManager);
  if (!d->MRMLLayoutNode)
  {
    return;
  }
  d->MRMLLayoutNode->RemoveAllMaximizedViewNodes();
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
void qMRMLLayoutManager::resetThreeDViews()
{
  for (int idx = 0; idx < this->threeDViewCount(); ++idx)
  {
    qMRMLThreeDView* threeDView = this->threeDWidget(idx)->threeDView();
    threeDView->resetFocalPoint();
    threeDView->resetCamera();
  }
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::resetSliceViews()
{
  foreach (const QString& viewName, this->sliceViewNames())
  {
    this->sliceWidget(viewName)->sliceController()->fitSliceToBackground();
  }
}

//------------------------------------------------------------------------------
QWidget* qMRMLLayoutManager::viewWidget(vtkMRMLNode* viewNode) const
{
  Q_D(const qMRMLLayoutManager);

  return d->viewWidget(viewNode);
}

//------------------------------------------------------------------------------
QList<QWidget*> qMRMLLayoutManager::viewWidgets() const
{
  Q_D(const qMRMLLayoutManager);
  QList<QWidget*> viewWidgets;
  for (qMRMLLayoutViewFactory* factory : this->mrmlViewFactories())
  {
    for (int i = 0; i < factory->viewCount(); ++i)
    {
      viewWidgets.append(factory->viewWidget(i));
    }
  }
  return viewWidgets;
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::setRenderPaused(bool pause)
{
  // Note: views that are instantiated between pauseRender() calls will not be affected
  // by the specified pause state
  Q_D(qMRMLLayoutManager);

  QList<QWidget*> viewWidgets = this->viewWidgets();
  for (QWidget* widget : viewWidgets)
  {
    qMRMLAbstractViewWidget* viewWidget = qobject_cast<qMRMLAbstractViewWidget*>(widget);
    if (viewWidget)
    {
      viewWidget->setRenderPaused(pause);
    }
  }

  if (pause)
  {
    d->AllViewsPauseRenderCount++;
  }
  else
  {
    d->AllViewsPauseRenderCount--;
    if (d->AllViewsPauseRenderCount < 0)
    {
      qWarning() << Q_FUNC_INFO << "Cannot resume rendering on all views, pause render count is already 0";
      d->AllViewsPauseRenderCount = 0;
    }
  }
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::pauseRender()
{
  this->setRenderPaused(true);
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::resumeRender()
{
  this->setRenderPaused(false);
}

//------------------------------------------------------------------------------
int qMRMLLayoutManager::allViewsPauseRenderCount()
{
  Q_D(qMRMLLayoutManager);
  return d->AllViewsPauseRenderCount;
}

//-----------------------------------------------------------------------------
void qMRMLLayoutManager::onViewportUsageChanged(const QString& viewportName)
{
  Q_D(qMRMLLayoutManager);
  QWidget* viewport = this->viewport(viewportName);
  if (!viewport)
  {
    return;
  }
  QWidget* viewportWindow = qobject_cast<QWidget*>(viewport->parent());
  if (!viewportWindow)
  {
    return;
  }
  // Show/hide and save/restore geometry if it is detached viewport managed by this class
  if (d->DetachedViewports.contains(viewportName))
  {
    bool used = this->isViewportUsedInLayout(viewportName);
    QByteArray& geometry = d->DetachedViewports[viewportName].LastSavedWindowGeometry;
    if (used)
    {
      viewportWindow->setVisible(true);
      if (!geometry.isEmpty())
      {
        viewportWindow->restoreGeometry(geometry);
      }
    }
    else
    {
      // save geometry and hide if detached window
      geometry = viewportWindow->saveGeometry();
      viewportWindow->setVisible(false);
    }
  }
}

//-----------------------------------------------------------------------------
QWidget* qMRMLLayoutManager::createViewport(const QDomElement& layoutElement, const QString& viewportName)
{
  Q_D(qMRMLLayoutManager);
  // This method is called by the ctkLayoutManager when a viewport widget is needed by a layout and the
  // viewport is not available yet.
  if (!d->DetachedViewports.contains(viewportName))
  {
    qMRMLLayoutManagerPrivate::ViewportInfo viewportInfo;

    // Get application mainwindow
    QMainWindow* mainWindow = nullptr;
    foreach (QWidget* widget, qApp->topLevelWidgets())
    {
      mainWindow = qobject_cast<QMainWindow*>(widget);
      if (mainWindow)
      {
        break;
      }
    }

    // Use mainWindow as parent to link the viewport to the mainWindow: bring to the top and minimize
    // along with the viewport (and also copy application name and icon by default).
    bool dockable = (layoutElement.attribute("dockable", "true").toLower() == "true");
    if (mainWindow && dockable)
    {
      QDockWidget* dockWidget = new QDockWidget(mainWindow);
      dockWidget->setObjectName(viewportName);
      dockWidget->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable
                              | QDockWidget::DockWidgetFloatable);
      viewportInfo.Window = dockWidget;
      QFrame* frame = new QFrame(dockWidget);
      frame->setFrameStyle(QFrame::Box);
      viewportInfo.Viewport = frame;
      dockWidget->setWidget(frame);
      QString dockPosition = layoutElement.attribute("dockPosition", "floating").toLower();
      if (dockPosition == "floating")
      {
        dockWidget->setFloating(true);
      }
      else if (dockPosition == "top")
      {
        mainWindow->addDockWidget(Qt::TopDockWidgetArea, dockWidget);
      }
      else if (dockPosition == "bottom")
      {
        mainWindow->addDockWidget(Qt::BottomDockWidgetArea, dockWidget);
      }
      else if (dockPosition == "left")
      {
        mainWindow->addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
      }
      else if (dockPosition == "right")
      {
        mainWindow->addDockWidget(Qt::RightDockWidgetArea, dockWidget);
      }
      else if (dockPosition == "bottom-left")
      {
        mainWindow->addDockWidget(mainWindow->corner(Qt::BottomLeftCorner), dockWidget);
      }
      else if (dockPosition == "bottom-right")
      {
        mainWindow->addDockWidget(mainWindow->corner(Qt::BottomRightCorner), dockWidget);
      }
      else if (dockPosition == "top-left")
      {
        mainWindow->addDockWidget(mainWindow->corner(Qt::TopLeftCorner), dockWidget);
      }
      else if (dockPosition == "top-right")
      {
        mainWindow->addDockWidget(mainWindow->corner(Qt::TopRightCorner), dockWidget);
      }
      else
      {
        qWarning()
          << "Unknown dockPosition in layout XML:" << dockPosition
          << " - valid values are floating, top, bottom, left, right, bottom-left, bottom-right, top-left, top-right";
        dockWidget->setFloating(true);
      }
    }
    else
    {
      viewportInfo.Window = new QWidget(mainWindow, Qt::Window | Qt::WindowMaximizeButtonHint);
      viewportInfo.Window->setObjectName(viewportName);
      if (!mainWindow)
      {
        viewportInfo.Window->setWindowIcon(QIcon(":/Icons/Medium/DesktopIcon.png"));
      }
      QVBoxLayout* layout = new QVBoxLayout(viewportInfo.Window);
      viewportInfo.Viewport = new QWidget(viewportInfo.Window);
      layout->addWidget(viewportInfo.Viewport);
    }

    QString label = layoutElement.attribute("label");
    if (!label.isEmpty())
    {
      // TODO: Translate label (see https://github.com/Slicer/Slicer/issues/6647)
      viewportInfo.Window->setWindowTitle(
        QCoreApplication::translate("vtkMRMLLayoutLogic", label.toStdString().c_str()));
    }

    d->DetachedViewports.insert(viewportName, viewportInfo);
  }
  return d->DetachedViewports.value(viewportName).Viewport;
}

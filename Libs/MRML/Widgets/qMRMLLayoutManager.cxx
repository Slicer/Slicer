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
#include <QDebug>

// MRMLWidgets includes
#include <qMRMLWidgetsConfigure.h> // For MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
#include <qMRMLLayoutManager_p.h>
#include <qMRMLSliceView.h>
#include <qMRMLSliceWidget.h>
#include <qMRMLSliceControllerWidget.h>
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
#include <qMRMLChartView.h>
#include <qMRMLChartWidget.h>
#endif
#include <qMRMLTableView.h>
#include <qMRMLTableWidget.h>
#include <qMRMLPlotView.h>
#include <qMRMLPlotWidget.h>
#include <qMRMLThreeDView.h>
#include <qMRMLThreeDWidget.h>

// MRMLLogic includes
#include <vtkMRMLColorLogic.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLViewLogic.h>

// MRML includes
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLChartViewNode.h>
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
  this->setViewLogics(nullptr);
}

//------------------------------------------------------------------------------
QString qMRMLLayoutThreeDViewFactory::viewClassName()const
{
  return "vtkMRMLViewNode";
}

//------------------------------------------------------------------------------
vtkCollection* qMRMLLayoutThreeDViewFactory::viewLogics() const
{
  return this->ViewLogics;
}

//------------------------------------------------------------------------------
void qMRMLLayoutThreeDViewFactory::setViewLogics(vtkCollection* viewLogics)
{
  if (viewLogics == this->ViewLogics)
    {
    return;
    }
  if (this->ViewLogics)
    {
    for (int viewIndex = 0; viewIndex < this->viewCount(); ++viewIndex)
      {
      qMRMLThreeDWidget* threeDWidget =
        qobject_cast<qMRMLThreeDWidget*>(viewWidget(viewIndex));
      vtkMRMLViewLogic* viewLogic =
        threeDWidget ? threeDWidget->viewLogic() : nullptr;
      if (viewLogics)
        {
        viewLogics->AddItem(viewLogic);
        }
      this->ViewLogics->RemoveItem(viewLogic);
      }
    this->ViewLogics->Delete();
    }
  this->ViewLogics = viewLogics;
  if (this->ViewLogics)
    {
    this->ViewLogics->Register(this->ViewLogics);
    }
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
  threeDWidget->setObjectName(QString("ThreeDWidget%1").arg(viewNode->GetLayoutLabel()));
  threeDWidget->setViewLabel(viewNode->GetLayoutLabel());
  QColor layoutColor = QColor::fromRgbF(viewNode->GetLayoutColor()[0],
                                        viewNode->GetLayoutColor()[1],
                                        viewNode->GetLayoutColor()[2]);
  threeDWidget->setViewColor(layoutColor);
  threeDWidget->setMRMLScene(this->mrmlScene());
  vtkMRMLViewNode* threeDViewNode = vtkMRMLViewNode::SafeDownCast(viewNode);
  threeDWidget->setMRMLViewNode(threeDViewNode);

  threeDWidget->setViewLogics(this->viewLogics());

  this->viewLogics()->AddItem(threeDWidget->viewLogic());

  return threeDWidget;
}

//------------------------------------------------------------------------------
void qMRMLLayoutThreeDViewFactory::deleteView(vtkMRMLAbstractViewNode* viewNode)
{
  qMRMLThreeDWidget* threeDWidget =
    qobject_cast<qMRMLThreeDWidget*>(this->viewWidget(viewNode));
  if (threeDWidget)
    {
    this->viewLogics()->RemoveItem(threeDWidget->viewLogic());
    }
  this->Superclass::deleteView(viewNode);
}

//------------------------------------------------------------------------------
// qMRMLLayoutChartViewFactory
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
//------------------------------------------------------------------------------
qMRMLLayoutChartViewFactory::qMRMLLayoutChartViewFactory(QObject* parent)
  : qMRMLLayoutViewFactory(parent)
{
}

//------------------------------------------------------------------------------
QString qMRMLLayoutChartViewFactory::viewClassName()const
{
  return "vtkMRMLChartViewNode";
}

//------------------------------------------------------------------------------
vtkMRMLColorLogic* qMRMLLayoutChartViewFactory::colorLogic()const
{
  return this->ColorLogic;
}

//------------------------------------------------------------------------------
void qMRMLLayoutChartViewFactory::setColorLogic(vtkMRMLColorLogic* colorLogic)
{
  this->ColorLogic = colorLogic;
  foreach(QWidget* view, this->registeredViews())
    {
    qMRMLChartWidget* chartWidget = qobject_cast<qMRMLChartWidget*>(view);
    Q_ASSERT(chartWidget);
    chartWidget->setColorLogic(this->colorLogic());
    }
}

//------------------------------------------------------------------------------
QWidget* qMRMLLayoutChartViewFactory::createViewFromNode(vtkMRMLAbstractViewNode* viewNode)
{
  if (!this->layoutManager() || !viewNode || !this->layoutManager()->viewport())
    {
    Q_ASSERT(viewNode);
    return nullptr;
    }

  // There must be a unique ChartWidget per node
  Q_ASSERT(!this->viewWidget(viewNode));

  qMRMLChartWidget* chartWidget = new qMRMLChartWidget(this->layoutManager()->viewport());
  QString layoutName(viewNode->GetLayoutName());
  chartWidget->setObjectName(QString("qMRMLChartWidget" + layoutName));
  chartWidget->setViewLabel(viewNode->GetLayoutLabel());
  /// \todo move color logic to view factory.
  chartWidget->setColorLogic(this->colorLogic());
  chartWidget->setMRMLScene(this->mrmlScene());
  vtkMRMLChartViewNode* chartNode = vtkMRMLChartViewNode::SafeDownCast(viewNode);
  chartWidget->setMRMLChartViewNode(chartNode);

  return chartWidget;
}
#endif

//------------------------------------------------------------------------------
// qMRMLLayoutTableViewFactory
//------------------------------------------------------------------------------
qMRMLLayoutTableViewFactory::qMRMLLayoutTableViewFactory(QObject* parent)
  : qMRMLLayoutViewFactory(parent)
{
}

//------------------------------------------------------------------------------
QString qMRMLLayoutTableViewFactory::viewClassName()const
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
  QString layoutName(viewNode->GetLayoutName());
  tableWidget->setObjectName(QString("qMRMLTableWidget" + layoutName));
  tableWidget->setViewLabel(viewNode->GetLayoutLabel());
  tableWidget->setMRMLScene(this->mrmlScene());
  vtkMRMLTableViewNode* tableNode = vtkMRMLTableViewNode::SafeDownCast(viewNode);
  tableWidget->setMRMLTableViewNode(tableNode);

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
  QString layoutName(viewNode->GetLayoutName());
  plotWidget->setObjectName(QString("qMRMLPlotWidget" + layoutName));
  plotWidget->setViewLabel(viewNode->GetLayoutLabel());
  plotWidget->setMRMLScene(this->mrmlScene());
  vtkMRMLPlotViewNode* plotViewNode = vtkMRMLPlotViewNode::SafeDownCast(viewNode);
  plotWidget->setMRMLPlotViewNode(plotViewNode);

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
  this->setSliceLogics(nullptr);
}

// --------------------------------------------------------------------------
QString qMRMLLayoutSliceViewFactory::viewClassName()const
{
  return "vtkMRMLSliceNode";
}

//------------------------------------------------------------------------------
vtkCollection* qMRMLLayoutSliceViewFactory::sliceLogics()const
{
  return this->SliceLogics;
}

// --------------------------------------------------------------------------
void qMRMLLayoutSliceViewFactory::setSliceLogics(vtkCollection* sliceLogics)
{
  if (sliceLogics == this->SliceLogics)
    {
    return;
    }
  if (this->SliceLogics)
    {
    for (int i = 0; i < this->viewCount(); ++i)
      {
      qMRMLSliceWidget* sliceWidget =
        qobject_cast<qMRMLSliceWidget*>(viewWidget(i));
      vtkMRMLSliceLogic* sliceLogic =
        sliceWidget ? sliceWidget->sliceLogic() : nullptr;
      if (sliceLogics)
        {
        sliceLogics->AddItem(sliceLogic);
        }
      this->SliceLogics->RemoveItem(sliceLogic);
      }
    this->SliceLogics->Delete();
    }
  this->SliceLogics = sliceLogics;
  if (this->SliceLogics)
    {
    this->SliceLogics->Register(this->SliceLogics);
    }
}

// --------------------------------------------------------------------------
QWidget* qMRMLLayoutSliceViewFactory::createViewFromNode(vtkMRMLAbstractViewNode* viewNode)
{
  if (!this->layoutManager() || !viewNode || !this->layoutManager()->viewport())
    {// can't create a slice widget if there is no parent widget
    Q_ASSERT(viewNode);
    return nullptr;
    }

  // there is a unique slice widget per node
  Q_ASSERT(!this->viewWidget(viewNode));

  qMRMLSliceWidget * sliceWidget = new qMRMLSliceWidget(this->layoutManager()->viewport());
  sliceWidget->sliceController()->setControllerButtonGroup(this->SliceControllerButtonGroup);
  QString sliceLayoutName(viewNode->GetLayoutName());
  QString sliceViewLabel(viewNode->GetLayoutLabel());
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(viewNode);
  QColor sliceLayoutColor = QColor::fromRgbF(sliceNode->GetLayoutColor()[0],
                                             sliceNode->GetLayoutColor()[1],
                                             sliceNode->GetLayoutColor()[2]);
  sliceWidget->setSliceViewName(sliceLayoutName);
  sliceWidget->setObjectName(QString("qMRMLSliceWidget" + sliceLayoutName));
  sliceWidget->setSliceViewLabel(sliceViewLabel);
  sliceWidget->setSliceViewColor(sliceLayoutColor);
  sliceWidget->setMRMLScene(this->mrmlScene());
  sliceWidget->setMRMLSliceNode(sliceNode);
  sliceWidget->setSliceLogics(this->sliceLogics());

  this->sliceLogics()->AddItem(sliceWidget->sliceLogic());

  return sliceWidget;
}

// --------------------------------------------------------------------------
void qMRMLLayoutSliceViewFactory::deleteView(vtkMRMLAbstractViewNode* viewNode)
{
  qMRMLSliceWidget* sliceWidget =
    qobject_cast<qMRMLSliceWidget*>(this->viewWidget(viewNode));
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
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
  this->ActiveMRMLChartViewNode = nullptr;
#endif
  this->ActiveMRMLTableViewNode = nullptr;
  this->ActiveMRMLPlotViewNode = nullptr;
  //this->SavedCurrentViewArrangement = vtkMRMLLayoutNode::SlicerLayoutNone;
}

//------------------------------------------------------------------------------
qMRMLLayoutManagerPrivate::~qMRMLLayoutManagerPrivate()
{
  this->MRMLLayoutLogic->Delete();
  this->MRMLLayoutLogic = nullptr;
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::init()
{
  Q_Q(qMRMLLayoutManager);

  q->setSpacing(1);

  qMRMLLayoutThreeDViewFactory* threeDViewFactory =
    new qMRMLLayoutThreeDViewFactory;
  q->registerViewFactory(threeDViewFactory);

  qMRMLLayoutSliceViewFactory* sliceViewFactory =
    new qMRMLLayoutSliceViewFactory;
  q->registerViewFactory(sliceViewFactory);

#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
  qMRMLLayoutChartViewFactory* chartViewFactory =
    new qMRMLLayoutChartViewFactory;
  q->registerViewFactory(chartViewFactory);
#endif

  qMRMLLayoutTableViewFactory* tableViewFactory =
    new qMRMLLayoutTableViewFactory;
  q->registerViewFactory(tableViewFactory);

  qMRMLLayoutPlotViewFactory* plotViewFactory =
    new qMRMLLayoutPlotViewFactory;
  q->registerViewFactory(plotViewFactory);
}

//------------------------------------------------------------------------------
qMRMLThreeDWidget* qMRMLLayoutManagerPrivate::threeDWidget(vtkMRMLViewNode* node)const
{
  Q_Q(const qMRMLLayoutManager);
  return qobject_cast<qMRMLThreeDWidget*>(
        q->mrmlViewFactory("vtkMRMLViewNode")->viewWidget(node));
}

//------------------------------------------------------------------------------
qMRMLChartWidget* qMRMLLayoutManagerPrivate::chartWidget(vtkMRMLChartViewNode* node)const
{
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
  Q_Q(const qMRMLLayoutManager);
  return qobject_cast<qMRMLChartWidget*>(
        q->mrmlViewFactory("vtkMRMLChartViewNode")->viewWidget(node));
#else
  Q_UNUSED(node);
  return nullptr;
#endif
}

//------------------------------------------------------------------------------
qMRMLSliceWidget* qMRMLLayoutManagerPrivate::sliceWidget(vtkMRMLSliceNode* node)const
{
  Q_Q(const qMRMLLayoutManager);
  return qobject_cast<qMRMLSliceWidget*>(
        q->mrmlViewFactory("vtkMRMLSliceNode")->viewWidget(node));
}

//------------------------------------------------------------------------------
qMRMLTableWidget* qMRMLLayoutManagerPrivate::tableWidget(vtkMRMLTableViewNode* node)const
{
  Q_Q(const qMRMLLayoutManager);
  return qobject_cast<qMRMLTableWidget*>(
              q->mrmlViewFactory("vtkMRMLTableViewNode")->viewWidget(node));
}

//------------------------------------------------------------------------------
qMRMLPlotWidget *qMRMLLayoutManagerPrivate::plotWidget(vtkMRMLPlotViewNode *node) const
{
  Q_Q(const qMRMLLayoutManager);
  return qobject_cast<qMRMLPlotWidget*>(
              q->mrmlViewFactory("vtkMRMLPlotViewNode")->viewWidget(node));
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
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
  if (qobject_cast<qMRMLChartWidget*>(widget))
    {
    return qobject_cast<qMRMLChartWidget*>(widget)->mrmlChartViewNode();
    }
#endif
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
QWidget* qMRMLLayoutManagerPrivate::viewWidget(vtkMRMLNode* viewNode)const
{
  Q_Q(const qMRMLLayoutManager);
  if (!viewNode)
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
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
  if (vtkMRMLChartViewNode::SafeDownCast(viewNode))
    {
    widget = this->chartWidget(vtkMRMLChartViewNode::SafeDownCast(viewNode));
    }
#endif
  if (vtkMRMLTableViewNode::SafeDownCast(viewNode))
    {
    widget = this->tableWidget(vtkMRMLTableViewNode::SafeDownCast(viewNode));
    }
  if (vtkMRMLPlotViewNode::SafeDownCast(viewNode))
    {
    widget = this->plotWidget(vtkMRMLPlotViewNode::SafeDownCast(viewNode));
    }
  return widget ? widget : q->mrmlViewFactory(
        QString::fromUtf8(viewNode->GetClassName()))->viewWidget(
        vtkMRMLAbstractViewNode::SafeDownCast(viewNode));
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
void qMRMLLayoutManagerPrivate::setActiveMRMLThreeDViewNode(vtkMRMLViewNode* node)
{
  Q_Q(qMRMLLayoutManager);
  QObject::connect(q->mrmlViewFactory("vtkMRMLViewNode"),
                   SIGNAL(activeViewNodeChanged(vtkMRMLAbstractViewNode*)),
                   this, SLOT(onActiveThreeDViewNodeChanged(vtkMRMLAbstractViewNode*)),
                   Qt::UniqueConnection);

  q->mrmlViewFactory("vtkMRMLViewNode")->setActiveViewNode(node);
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate
::onActiveThreeDViewNodeChanged(vtkMRMLAbstractViewNode* node)
{
  Q_Q(qMRMLLayoutManager);
  emit q->activeThreeDRendererChanged(
    q->mrmlViewFactory("vtkMRMLViewNode")->activeRenderer());
  emit q->activeMRMLThreeDViewNodeChanged(
    vtkMRMLViewNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setActiveMRMLChartViewNode(vtkMRMLChartViewNode* node)
{
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
  Q_Q(qMRMLLayoutManager);
  QObject::connect(q->mrmlViewFactory("vtkMRMLChartViewNode"),
                   SIGNAL(activeViewNodeChanged(vtkMRMLAbstractViewNode*)),
                   this, SLOT(onActiveChartViewNodeChanged(vtkMRMLAbstractViewNode*)),
                   Qt::UniqueConnection);
  q->mrmlViewFactory("vtkMRMLChartViewNode")->setActiveViewNode(node);
#else
  Q_UNUSED(node);
#endif
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate
::onActiveChartViewNodeChanged(vtkMRMLAbstractViewNode* node)
{
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
  Q_Q(qMRMLLayoutManager);
  emit q->activeChartRendererChanged(
    q->mrmlViewFactory("vtkMRMLChartViewNode")->activeRenderer());
  emit q->activeMRMLChartViewNodeChanged(
    vtkMRMLChartViewNode::SafeDownCast(node));
#else
  Q_UNUSED(node);
#endif
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setActiveMRMLTableViewNode(vtkMRMLTableViewNode* node)
{
  Q_Q(qMRMLLayoutManager);
  QObject::connect(q->mrmlViewFactory("vtkMRMLTableViewNode"),
                   SIGNAL(activeViewNodeChanged(vtkMRMLAbstractViewNode*)),
                   this, SLOT(onActiveTableViewNodeChanged(vtkMRMLAbstractViewNode*)),
                   Qt::UniqueConnection);
  q->mrmlViewFactory("vtkMRMLTableViewNode")->setActiveViewNode(node);
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate
::onActiveTableViewNodeChanged(vtkMRMLAbstractViewNode* node)
{
  Q_Q(qMRMLLayoutManager);
  emit q->activeTableRendererChanged(
    q->mrmlViewFactory("vtkMRMLTableViewNode")->activeRenderer());
  emit q->activeMRMLTableViewNodeChanged(
                vtkMRMLTableViewNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::setActiveMRMLPlotViewNode(vtkMRMLPlotViewNode* node)
{
  Q_Q(qMRMLLayoutManager);
  QObject::connect(q->mrmlViewFactory("vtkMRMLPlotViewNode"),
                   SIGNAL(activeViewNodeChanged(vtkMRMLAbstractViewNode*)),
                   this, SLOT(onActivePlotViewNodeChanged(vtkMRMLAbstractViewNode*)),
                   Qt::UniqueConnection);
  q->mrmlViewFactory("vtkMRMLPlotViewNode")->setActiveViewNode(node);
}

// --------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::
onActivePlotViewNodeChanged(vtkMRMLAbstractViewNode* node)
{
  Q_Q(qMRMLLayoutManager);
  emit q->activePlotRendererChanged(
    q->mrmlViewFactory("vtkMRMLPlotViewNode")->activeRenderer());
  emit q->activeMRMLPlotViewNodeChanged(
                vtkMRMLPlotViewNode::SafeDownCast(node));
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
    //qDebug() << "qMRMLLayoutManagerPrivate::onLayoutNodeAddedEvent";
    // Only one Layout node is expected
    Q_ASSERT(this->MRMLLayoutNode == nullptr);
    if (this->MRMLLayoutNode != nullptr)
      {
      return;
      }
    this->setMRMLLayoutNode(layoutNode);
    }

  // View node added
  vtkMRMLAbstractViewNode* viewNode =
    vtkMRMLAbstractViewNode::SafeDownCast(node);
  if (viewNode)
    {
    // No explicit parent layout node means that view is handled by the main Slicer layout
    if (!viewNode->GetParentLayoutNode())
      {
      foreach(qMRMLLayoutViewFactory* mrmlViewFactory, q->mrmlViewFactories())
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
  vtkMRMLAbstractViewNode* viewNode =
    vtkMRMLAbstractViewNode::SafeDownCast(node);
  if (viewNode)
    {
    foreach(qMRMLLayoutViewFactory* mrmlViewFactory, q->mrmlViewFactories())
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
  //qDebug() << "qMRMLLayoutManagerPrivate::onSceneClosedEvent";
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
  if (!this->MRMLScene ||
      this->MRMLScene->IsBatchProcessing() ||
      !this->Enabled)
    {
    return;
    }
  this->updateLayoutInternal();
}

//------------------------------------------------------------------------------
void qMRMLLayoutManagerPrivate::updateLayoutFromMRMLScene()
{
  Q_Q(qMRMLLayoutManager);
  foreach(qMRMLLayoutViewFactory* viewFactory, q->mrmlViewFactories())
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
  int layout = this->MRMLLayoutNode ?
    this->MRMLLayoutNode->GetViewArrangement() :
    vtkMRMLLayoutNode::SlicerLayoutNone;

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

  foreach(const QString& viewName, q->sliceViewNames())
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
qMRMLLayoutManager::qMRMLLayoutManager(qMRMLLayoutManagerPrivate* pimpl,
                                       QWidget* viewport, QObject* parentObject)
  : Superclass(viewport, parentObject)
  , d_ptr(pimpl)
{
  Q_D(qMRMLLayoutManager);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLLayoutManager::~qMRMLLayoutManager() = default;

// --------------------------------------------------------------------------
bool qMRMLLayoutManager::isEnabled()const
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
QList<qMRMLLayoutViewFactory*> qMRMLLayoutManager::mrmlViewFactories()const
{
  QList<qMRMLLayoutViewFactory*> res;
  foreach(ctkLayoutViewFactory* viewFactory, this->registeredViewFactories())
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
qMRMLLayoutViewFactory* qMRMLLayoutManager
::mrmlViewFactory(const QString& viewClassName)const
{
  foreach(qMRMLLayoutViewFactory* viewFactory, this->mrmlViewFactories())
    {
    if (viewFactory->viewClassName() == viewClassName)
      {
      return viewFactory;
      }
    }
  return nullptr;
}

// --------------------------------------------------------------------------
void qMRMLLayoutManager
::registerViewFactory(ctkLayoutViewFactory* viewFactory)
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
qMRMLSliceWidget* qMRMLLayoutManager::sliceWidget(const QString& name)const
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
int qMRMLLayoutManager::threeDViewCount()const
{
  qMRMLLayoutViewFactory* viewFactory = this->mrmlViewFactory("vtkMRMLViewNode");
  if (!viewFactory)
    {
    return 0;
    }
  return viewFactory->viewCount();
}

//------------------------------------------------------------------------------
int qMRMLLayoutManager::chartViewCount()const
{
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
  qMRMLLayoutViewFactory* viewFactory = this->mrmlViewFactory("vtkMRMLChartViewNode");
  if (!viewFactory)
    {
    return 0;
    }
  return viewFactory->viewCount();
#else
  return 0;
#endif
}

//------------------------------------------------------------------------------
int qMRMLLayoutManager::tableViewCount()const
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
qMRMLThreeDWidget* qMRMLLayoutManager::threeDWidget(int id)const
{
  qMRMLLayoutViewFactory* viewFactory = this->mrmlViewFactory("vtkMRMLViewNode");
  if (!viewFactory)
    {
    return nullptr;
    }
  return qobject_cast<qMRMLThreeDWidget*>(viewFactory->viewWidget(id));
}

//------------------------------------------------------------------------------
qMRMLThreeDWidget* qMRMLLayoutManager::threeDWidget(const QString& name)const
{
  qMRMLLayoutViewFactory* viewFactory = this->mrmlViewFactory("vtkMRMLViewNode");
  if (!viewFactory)
    {
    return nullptr;
    }
  return qobject_cast<qMRMLThreeDWidget*>(viewFactory->viewWidget(name));
}

//------------------------------------------------------------------------------
qMRMLChartWidget* qMRMLLayoutManager::chartWidget(int id)const
{
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
  qMRMLLayoutViewFactory* viewFactory = this->mrmlViewFactory("vtkMRMLChartViewNode");
  if (!viewFactory)
    {
    return nullptr;
    }
  return qobject_cast<qMRMLChartWidget*>(viewFactory->viewWidget(id));
#else
  Q_UNUSED(id);
  return nullptr;
#endif
}

//------------------------------------------------------------------------------
qMRMLTableWidget* qMRMLLayoutManager::tableWidget(int id)const
{
  qMRMLLayoutViewFactory* viewFactory = this->mrmlViewFactory("vtkMRMLTableViewNode");
  if (!viewFactory)
    {
    return nullptr;
    }
  return qobject_cast<qMRMLTableWidget*>(viewFactory->viewWidget(id));
}

//------------------------------------------------------------------------------
qMRMLPlotWidget *qMRMLLayoutManager::plotWidget(int id)const
{
  qMRMLLayoutViewFactory* viewFactory = this->mrmlViewFactory("vtkMRMLPlotViewNode");
  if (!viewFactory)
    {
    return nullptr;
    }
  return qobject_cast<qMRMLPlotWidget*>(viewFactory->viewWidget(id));
}

//------------------------------------------------------------------------------
vtkCollection* qMRMLLayoutManager::mrmlSliceLogics()const
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
vtkCollection *qMRMLLayoutManager::mrmlViewLogics() const
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
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
  qMRMLLayoutChartViewFactory* viewChartFactory =
    qobject_cast<qMRMLLayoutChartViewFactory*>(this->mrmlViewFactory("vtkMRMLChartViewNode"));
  if (!viewChartFactory)
    {
    return;
    }
  viewChartFactory->setColorLogic(colorLogic);
#else
  Q_UNUSED(colorLogic);
#endif
}

//------------------------------------------------------------------------------
vtkMRMLColorLogic* qMRMLLayoutManager::mrmlColorLogic()const
{
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
  qMRMLLayoutChartViewFactory* viewFactory =
    qobject_cast<qMRMLLayoutChartViewFactory*>(this->mrmlViewFactory("vtkMRMLChartViewNode"));
  if (!viewFactory)
    {
    return nullptr;
    }
  return viewFactory->colorLogic();
#else
  return nullptr;
#endif
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
  d->MRMLLayoutNode = nullptr;

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

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::EndBatchProcessEvent,
                   d, SLOT(updateSegmentationControls()));

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::EndRestoreEvent,
                   d, SLOT(onSceneRestoredEvent()));

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::StartCloseEvent,
                   d, SLOT(onSceneAboutToBeClosedEvent()));

  d->qvtkReconnect(oldScene, scene, vtkMRMLScene::EndCloseEvent,
                   d, SLOT(onSceneClosedEvent()));

  foreach(qMRMLLayoutViewFactory* viewFactory, this->mrmlViewFactories())
    {
    viewFactory->setMRMLScene(d->MRMLScene);
    }
  d->updateLayoutFromMRMLScene();
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLLayoutManager::mrmlScene()const
{
  Q_D(const qMRMLLayoutManager);
  return d->MRMLScene;
}

//------------------------------------------------------------------------------
vtkMRMLViewNode* qMRMLLayoutManager::activeMRMLThreeDViewNode()const
{
  return vtkMRMLViewNode::SafeDownCast(
    this->mrmlViewFactory("vtkMRMLViewNode")->activeViewNode());
}

//------------------------------------------------------------------------------
vtkMRMLChartViewNode* qMRMLLayoutManager::activeMRMLChartViewNode()const
{
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
  return vtkMRMLChartViewNode::SafeDownCast(
    this->mrmlViewFactory("vtkMRMLChartViewNode")->activeViewNode());
#else
  return nullptr;
#endif
}

//------------------------------------------------------------------------------
vtkMRMLTableViewNode* qMRMLLayoutManager::activeMRMLTableViewNode()const
{
  return vtkMRMLTableViewNode::SafeDownCast(
    this->mrmlViewFactory("vtkMRMLTableViewNode")->activeViewNode());
}

//------------------------------------------------------------------------------
vtkMRMLPlotViewNode *qMRMLLayoutManager::activeMRMLPlotViewNode()const
{
  return vtkMRMLPlotViewNode::SafeDownCast(
    this->mrmlViewFactory("vtkMRMLPlotViewNode")->activeViewNode());
}

//------------------------------------------------------------------------------
vtkRenderer* qMRMLLayoutManager::activeThreeDRenderer()const
{
  return this->mrmlViewFactory("vtkMRMLViewNode")->activeRenderer();
}

//------------------------------------------------------------------------------
vtkRenderer* qMRMLLayoutManager::activeChartRenderer()const
{
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
  return this->mrmlViewFactory("vtkMRMLChartViewNode")->activeRenderer();
#else
  return nullptr;
#endif
}

//------------------------------------------------------------------------------
vtkRenderer* qMRMLLayoutManager::activeTableRenderer()const
{
  return this->mrmlViewFactory("vtkMRMLTableViewNode")->activeRenderer();
}

//------------------------------------------------------------------------------
vtkRenderer* qMRMLLayoutManager::activePlotRenderer()const
{
  return this->mrmlViewFactory("vtkMRMLPlotViewNode")->activeRenderer();
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
  // Update LayoutNode
  if (d->MRMLLayoutNode)
    {
    if (!d->MRMLLayoutNode->IsLayoutDescription(layout))
      {
      layout = vtkMRMLLayoutNode::SlicerLayoutConventionalView;
      }
    d->MRMLLayoutNode->SetViewArrangement(layout);
    }
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
  for(int idx = 0; idx < this->threeDViewCount(); ++idx)
    {
    qMRMLThreeDView* threeDView = this->threeDWidget(idx)->threeDView();
    threeDView->resetFocalPoint();
    threeDView->resetCamera();
    }
}

//------------------------------------------------------------------------------
void qMRMLLayoutManager::resetSliceViews()
{
  foreach(const QString& viewName, this->sliceViewNames())
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
void qMRMLLayoutManager::setRenderPaused(bool pause)
{
  // Note: views that are instantiated between pauseRender() calls will not be affected
  // by the specified pause state
  Q_D(qMRMLLayoutManager);
  qMRMLLayoutViewFactory* sliceViewFactory = this->mrmlViewFactory("vtkMRMLSliceNode");
  foreach(const QString& viewName, sliceViewFactory->viewNodeNames())
    {
    ctkVTKAbstractView* view = this->sliceWidget(viewName)->sliceView();
    view->setRenderPaused(pause);
    }

  qMRMLLayoutViewFactory* threeDViewFactory = this->mrmlViewFactory("vtkMRMLViewNode");
  foreach(const QString& viewName, threeDViewFactory->viewNodeNames())
    {
    ctkVTKAbstractView* view = this->threeDWidget(viewName)->threeDView();
    view->setRenderPaused(pause);
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

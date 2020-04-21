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

// MRMLWidgets includes
#include <qMRMLSliceWidget.h>
#include <qMRMLSliceControllerWidget.h>
#include <qMRMLThreeDWidget.h>
#include <qMRMLThreeDViewControllerWidget.h>
#include <qMRMLPlotWidget.h>
#include <qMRMLPlotViewControllerWidget.h>

// Slicer includes
#include "qSlicerViewControllersModuleWidget.h"
#include "ui_qSlicerViewControllersModuleWidget.h"
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"

// MRML includes
#include "vtkMRMLPlotViewNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLViewNode.h"

// MRMLLogic includes
#include "vtkMRMLLayoutLogic.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerViewControllersModuleWidgetPrivate:
    public Ui_qSlicerViewControllersModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerViewControllersModuleWidget);
protected:
  qSlicerViewControllersModuleWidget* const q_ptr;

public:
  qSlicerViewControllersModuleWidgetPrivate(qSlicerViewControllersModuleWidget& obj);
  virtual ~qSlicerViewControllersModuleWidgetPrivate();

  /// Create a Controller for a Node and pack in the widget
  void createController(vtkMRMLNode *n, qSlicerLayoutManager *lm);

  /// Remove the Controller for a Node from the widget
  void removeController(vtkMRMLNode *n);

  typedef std::map<vtkSmartPointer<vtkMRMLNode>, qMRMLViewControllerBar* > ControllerMapType;
  ControllerMapType ControllerMap;

protected:
};

//-----------------------------------------------------------------------------
qSlicerViewControllersModuleWidgetPrivate::qSlicerViewControllersModuleWidgetPrivate(qSlicerViewControllersModuleWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerViewControllersModuleWidgetPrivate::~qSlicerViewControllersModuleWidgetPrivate() = default;

//-----------------------------------------------------------------------------
void
qSlicerViewControllersModuleWidgetPrivate::createController(vtkMRMLNode *n, qSlicerLayoutManager *layoutManager)
{
  Q_Q(qSlicerViewControllersModuleWidget);

  if (this->ControllerMap.find(n) != this->ControllerMap.end())
    {
    qDebug() << "qSlicerViewControllersModuleWidgetPrivate::createController - Node already added to module";
    return;
    }

  // create the ControllerWidget and wire it to the appropriate node
  qMRMLViewControllerBar *barWidget = nullptr;
  vtkMRMLSliceNode *sn = vtkMRMLSliceNode::SafeDownCast(n);
  if (sn)
    {
    qMRMLSliceControllerWidget *widget =
      new qMRMLSliceControllerWidget(this->SliceControllersCollapsibleButton);
    widget->setSliceViewName( sn->GetName() ); // call before setting slice node
    widget->setSliceViewLabel( sn->GetLayoutLabel() );
    QColor layoutColor = QColor::fromRgbF(sn->GetLayoutColor()[0],
                                          sn->GetLayoutColor()[1],
                                          sn->GetLayoutColor()[2]);
    widget->setSliceViewColor( layoutColor );
    widget->setMRMLSliceNode( sn );
    widget->setLayoutBehavior( qMRMLViewControllerBar::Panel );

    // SliceControllerWidget needs to know the SliceLogic(s)
    qMRMLSliceWidget *sliceWidget = layoutManager->sliceWidget(sn->GetLayoutName());
    widget->setSliceLogics(layoutManager->mrmlSliceLogics());
    widget->setSliceLogic(sliceWidget->sliceController()->sliceLogic());

    // add the widget to the display
    this->SliceControllersLayout->addWidget(widget);

    barWidget = widget;
    }

  vtkMRMLViewNode *vn = vtkMRMLViewNode::SafeDownCast(n);
  if (vn)
    {
    qMRMLThreeDViewControllerWidget *widget =
      new qMRMLThreeDViewControllerWidget(this->ThreeDViewControllersCollapsibleButton);
    widget->setViewLabel( vn->GetLayoutLabel() );
    widget->setMRMLViewNode( vn );
    widget->setLayoutBehavior( qMRMLViewControllerBar::Panel );

    // ThreeDViewController needs to now the ThreeDView
    qMRMLThreeDWidget *viewWidget = dynamic_cast<qMRMLThreeDWidget*>(layoutManager->viewWidget( vn ));
    if (viewWidget)
      {
      widget->setThreeDView( viewWidget->threeDView() );
      widget->setMRMLScene(q->mrmlScene());
      // qMRMLThreeDViewControllerWidget needs to know the ViewLogic(s)
      widget->setViewLogics(layoutManager->mrmlViewLogics());
      widget->setViewLogic(viewWidget->threeDController()->viewLogic());
      }

    // add the widget to the display
    this->ThreeDViewControllersLayout->addWidget(widget);

    barWidget = widget;
    }

  vtkMRMLPlotViewNode *pn = vtkMRMLPlotViewNode::SafeDownCast(n);
  if (pn)
    {
    qMRMLPlotViewControllerWidget *widget =
      new qMRMLPlotViewControllerWidget(this->PlotViewControllersCollapsibleButton);
    widget->setMRMLPlotViewNode( pn );
    widget->setLayoutBehavior( qMRMLViewControllerBar::Panel );

    // PlotViewController needs to now the PlotView
    qMRMLPlotWidget *viewWidget = dynamic_cast<qMRMLPlotWidget*>(layoutManager->viewWidget( pn ));
    if (viewWidget)
      {
      widget->setPlotView( viewWidget->plotView() ) ;
      widget->setMRMLPlotViewNode(pn);
      widget->setMRMLScene(q->mrmlScene());
      }

    // add the widget to the display
    this->PlotViewControllersLayout->addWidget(widget);

    barWidget = widget;
    }

  // cache the widget. we'll clean this up on the NodeRemovedEvent
  this->ControllerMap[n] = barWidget;
}

//-----------------------------------------------------------------------------
void
qSlicerViewControllersModuleWidgetPrivate::removeController(vtkMRMLNode *n)
{
  // find the widget for the SliceNode
  ControllerMapType::iterator cit = this->ControllerMap.find(n);
  if (cit == this->ControllerMap.end())
    {
    qDebug() << "qSlicerViewControllersModuleWidgetPrivate::removeController - Node has no Controller managed by this module.";
    return;
    }

  // unpack the widget
  vtkMRMLSliceNode *sn = vtkMRMLSliceNode::SafeDownCast(n);
  if (sn)
    {
    SliceControllersLayout->removeWidget((*cit).second);
    }

  vtkMRMLViewNode *vn = vtkMRMLViewNode::SafeDownCast(n);
  if (vn)
    {
    ThreeDViewControllersLayout->removeWidget((*cit).second);
    }

  vtkMRMLPlotViewNode *pn = vtkMRMLPlotViewNode::SafeDownCast(n);
  if (pn)
    {
    PlotViewControllersLayout->removeWidget((*cit).second);
    }

  // delete the widget
  delete (*cit).second;

  // remove entry from the map
  this->ControllerMap.erase(cit);
}


//-----------------------------------------------------------------------------
qSlicerViewControllersModuleWidget::qSlicerViewControllersModuleWidget(QWidget* _parentWidget)
  : Superclass(_parentWidget),
    d_ptr(new qSlicerViewControllersModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerViewControllersModuleWidget::~qSlicerViewControllersModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerViewControllersModuleWidget::setup()
{
  Q_D(qSlicerViewControllersModuleWidget);
  d->setupUi(this);

  connect(d->MRMLViewNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(onAdvancedViewNodeChanged(vtkMRMLNode*)));

  d->AdvancedCollapsibleButton->setCollapsed(true);
}

//-----------------------------------------------------------------------------
void qSlicerViewControllersModuleWidget::setMRMLScene(vtkMRMLScene *newScene)
{
  Q_D(qSlicerViewControllersModuleWidget);

  vtkMRMLScene* oldScene = this->mrmlScene();

  this->Superclass::setMRMLScene(newScene);

  qSlicerApplication * app = qSlicerApplication::application();
  if (!app)
    {
    return;
    }
  qSlicerLayoutManager * layoutManager = app->layoutManager();
  if (!layoutManager)
    {
    return;
    }

  // Search the scene for the available view nodes and create a
  // Controller and connect it up
  std::vector<vtkMRMLNode*> sliceNodes;
  newScene->GetNodesByClass("vtkMRMLSliceNode", sliceNodes);
  for (std::vector< vtkMRMLNode* >::iterator sliceNodeIt = sliceNodes.begin(); sliceNodeIt != sliceNodes.end(); ++sliceNodeIt)
    {
    vtkMRMLSliceNode *snode = vtkMRMLSliceNode::SafeDownCast(*sliceNodeIt);
    if (snode)
      {
      d->createController(snode, layoutManager);
      }
    }

  std::vector<vtkMRMLNode*> threeDNodes;
  newScene->GetNodesByClass("vtkMRMLViewNode", threeDNodes);
  for (std::vector< vtkMRMLNode* >::iterator threeDNodeIt = threeDNodes.begin(); threeDNodeIt != threeDNodes.end(); ++threeDNodeIt)
    {
    vtkMRMLViewNode *vnode = vtkMRMLViewNode::SafeDownCast(*threeDNodeIt);
    if (vnode)
      {
      d->createController(vnode, layoutManager);
      }
    }

  std::vector<vtkMRMLNode*> plotNodes;
  newScene->GetNodesByClass("vtkMRMLPlotViewNode", plotNodes);
  for (std::vector< vtkMRMLNode* >::iterator plotNodeIt = plotNodes.begin(); plotNodeIt != plotNodes.end(); ++plotNodeIt)
    {
    vtkMRMLPlotViewNode *pnode = vtkMRMLPlotViewNode::SafeDownCast(*plotNodeIt);
    if (pnode)
      {
      d->createController(pnode, layoutManager);
      }
    }

  // Need to listen for any new slice or view nodes being added
  this->qvtkReconnect(oldScene, newScene, vtkMRMLScene::NodeAddedEvent,
                      this, SLOT(onNodeAddedEvent(vtkObject*,vtkObject*)));

  // Need to listen for any slice or view nodes being removed
  this->qvtkReconnect(oldScene, newScene, vtkMRMLScene::NodeRemovedEvent,
                      this, SLOT(onNodeRemovedEvent(vtkObject*,vtkObject*)));

  // Listen to changes in the Layout so we only show controllers for
  // the visible nodes
  QObject::connect(layoutManager, SIGNAL(layoutChanged(int)), this,
                   SLOT(onLayoutChanged(int)));

}

// --------------------------------------------------------------------------
void qSlicerViewControllersModuleWidget::onNodeAddedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerViewControllersModuleWidget);

  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }

  qSlicerApplication * app = qSlicerApplication::application();
  if (!app)
    {
    return;
    }
  qSlicerLayoutManager * layoutManager = app->layoutManager();
  if (!layoutManager)
    {
    return;
    }

  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  if (sliceNode)
    {
    //QString layoutName = sliceNode->GetLayoutName();
    //qDebug() << "qSlicerViewControllersModuleWidget::onNodeAddedEvent - layoutName:" << layoutName;

    // create the slice controller
    d->createController(sliceNode, layoutManager);
    }

  vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(node);
  if (viewNode)
    {
    //QString layoutName = viewNode->GetName();
    //qDebug() << "qSlicerViewControllersModuleWidget::onNodeAddedEvent - layoutName:" << layoutName;

    // create the view controller
    d->createController(viewNode, layoutManager);
    }

  vtkMRMLPlotViewNode* plotViewNode = vtkMRMLPlotViewNode::SafeDownCast(node);
  if (plotViewNode)
    {
    //QString layoutName = viewNode->GetName();
    //qDebug() << "qSlicerViewControllersModuleWidget::onNodeAddedEvent - layoutName:" << layoutName;

    // create the view controller
    d->createController(plotViewNode, layoutManager);
    }
}

// --------------------------------------------------------------------------
void qSlicerViewControllersModuleWidget::onNodeRemovedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerViewControllersModuleWidget);

  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }

  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  if (sliceNode)
    {
    QString layoutName = sliceNode->GetLayoutName();
    qDebug() << "qSlicerViewControllersModuleWidget::onNodeRemovedEvent - layoutName:" << layoutName;

    // destroy the slice controller
    d->removeController(sliceNode);
    }

  vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(node);
  if (viewNode)
    {
    QString layoutName = viewNode->GetName();
    qDebug() << "qSlicerViewControllersModuleWidget::onNodeRemovedEvent - layoutName:" << layoutName;

    // destroy the view controller
    d->removeController(viewNode);
    }

  vtkMRMLPlotViewNode* plotViewNode = vtkMRMLPlotViewNode::SafeDownCast(node);
  if (plotViewNode)
    {
    QString layoutName = plotViewNode->GetName();
    qDebug() << "qSlicerViewControllersModuleWidget::onNodeRemovedEvent - layoutName:" << layoutName;

    // destroy the view controller
    d->removeController(plotViewNode);
    }
}

// --------------------------------------------------------------------------
void qSlicerViewControllersModuleWidget::onLayoutChanged(int)
{
  Q_D(qSlicerViewControllersModuleWidget);

  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }

  //qDebug() << "qSlicerViewControllersModuleWidget::onLayoutChanged";

  // add the controllers for any newly visible SliceNodes and remove
  // the controllers for any SliceNodes no longer visible

  qSlicerApplication * app = qSlicerApplication::application();
  if (!app)
    {
    return;
    }
  qSlicerLayoutManager * layoutManager = app->layoutManager();
  if (!layoutManager)
    {
    return;
    }

  vtkMRMLLayoutLogic *layoutLogic = layoutManager->layoutLogic();
  vtkCollection *visibleViews = layoutLogic->GetViewNodes();

  // hide Controllers for Nodes not currently visible in
  // the layout
  qSlicerViewControllersModuleWidgetPrivate::ControllerMapType::iterator cit;
  for (cit = d->ControllerMap.begin(); cit != d->ControllerMap.end(); ++cit)
    {
    // is mananaged Node not currently displayed in the layout?
    if (!visibleViews->IsItemPresent((*cit).first))
      {
      // hide it
      (*cit).second->hide();
      }
    }

  // show Controllers for Nodes not currently being managed
  // by this widget
  vtkObject *v = nullptr;
  vtkCollectionSimpleIterator it;
  for (visibleViews->InitTraversal(it);
    (v = visibleViews->GetNextItemAsObject(it));)
    {
    vtkMRMLNode *vn = vtkMRMLNode::SafeDownCast(v);
    if (vn)
      {
      // find the controller
      qSlicerViewControllersModuleWidgetPrivate::ControllerMapType::iterator cit = d->ControllerMap.find(vn);
      if (cit != d->ControllerMap.end())
        {
        // show it
        (*cit).second->show();
        }
      }
    }
}

// --------------------------------------------------------------------------
void qSlicerViewControllersModuleWidget::onAdvancedViewNodeChanged(vtkMRMLNode* viewNode)
{
  Q_D(qSlicerViewControllersModuleWidget);
  // Only show widget corresponding to selected view node type
  d->MRMLSliceInformationWidget->setVisible(vtkMRMLSliceNode::SafeDownCast(viewNode) != nullptr);
  d->MRMLThreeDViewInformationWidget->setVisible(vtkMRMLViewNode::SafeDownCast(viewNode) != nullptr);
}

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

// CTK includes
#include <ctkLogger.h>

// VTK includes
#include <vtkSmartPointer.h>

// qMRMLWidgets includes
#include <qMRMLSliceWidget.h>
#include <qMRMLSliceControllerWidget.h>
#include <qMRMLThreeDWidget.h>
#include <qMRMLThreeDViewControllerWidget.h>

// SlicerQt includes
#include "qSlicerSliceControllerModuleWidget.h"
#include "ui_qSlicerSliceControllerModule.h"
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"

// MRML includes
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLViewNode.h"

// MRMLLogic includes
#include "vtkMRMLLayoutLogic.h"

// STL include

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.base.qtcoremodules.qSlicerSliceControllerModuleWidget");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class qSlicerSliceControllerModuleWidgetPrivate:
    public Ui_qSlicerSliceControllerModule
{
  Q_DECLARE_PUBLIC(qSlicerSliceControllerModuleWidget);
protected:
  qSlicerSliceControllerModuleWidget* const q_ptr;

public:
  qSlicerSliceControllerModuleWidgetPrivate(qSlicerSliceControllerModuleWidget& obj);
  virtual ~qSlicerSliceControllerModuleWidgetPrivate();
  
  /// Create a Controller for a Node and pack in the widget
  void createController(vtkMRMLNode *n, qSlicerLayoutManager *lm);

  /// Remove the Controller for a Node from the widget
  void removeController(vtkMRMLNode *n);
  
  typedef std::map<vtkSmartPointer<vtkMRMLNode>, qMRMLViewControllerBar* > ControllerMapType;
  ControllerMapType ControllerMap;

protected:
};

//-----------------------------------------------------------------------------
qSlicerSliceControllerModuleWidgetPrivate::qSlicerSliceControllerModuleWidgetPrivate(qSlicerSliceControllerModuleWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerSliceControllerModuleWidgetPrivate::~qSlicerSliceControllerModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
void 
qSlicerSliceControllerModuleWidgetPrivate::createController(vtkMRMLNode *n, qSlicerLayoutManager *layoutManager)
{
  Q_Q(qSlicerSliceControllerModuleWidget);

  if (this->ControllerMap.find(n) != this->ControllerMap.end())
    {
    logger.trace(QString("createController - Node already added to module"));
    return;
    }

  // create the ControllerWidget and wire it to the appropriate node
  qMRMLViewControllerBar *barWidget;
  vtkMRMLSliceNode *sn = vtkMRMLSliceNode::SafeDownCast(n);
  if (sn)
    {
    qMRMLSliceControllerWidget *widget =
      new qMRMLSliceControllerWidget(this->SliceControllersCollapsibleButton);
    widget->setSliceViewName( sn->GetName() ); // call before setting slice node
    widget->setSliceViewLabel( sn->GetLayoutLabel() );
    widget->setSliceViewColor( sn->GetLayoutColor() );
    widget->setMRMLSliceNode( sn );
    widget->setLayoutBehavior( qMRMLViewControllerBar::Panel );

    // SliceControllerWidget needs to know the SliceLogic(s)
    qMRMLSliceWidget *sliceWidget = layoutManager->sliceWidget(sn->GetLayoutName());
    widget->setSliceLogics(layoutManager->mrmlSliceLogics());
    widget->setSliceLogic(sliceWidget->sliceController()->sliceLogic());
    
    // add the widget to the display
    SliceControllersLayout->addWidget(widget);

    barWidget = widget;
    }

  vtkMRMLViewNode *vn = vtkMRMLViewNode::SafeDownCast(n);
  if (vn)
    {
    qMRMLThreeDViewControllerWidget *widget =
      new qMRMLThreeDViewControllerWidget(this->ThreeDViewControllersCollapsibleButton);
    widget->setViewLabel( vn->GetViewLabel() );
    widget->setMRMLViewNode( vn );
    widget->setLayoutBehavior( qMRMLViewControllerBar::Panel );
    
    // ThreeDViewController needs to now the ThreeDView
    qMRMLThreeDWidget *viewWidget = dynamic_cast<qMRMLThreeDWidget*>(layoutManager->viewWidget( vn ));
    if (viewWidget)
      {
      widget->setThreeDView( viewWidget->threeDView() );
      }
    
    // add the widget to the display
    ThreeDViewControllersLayout->addWidget(widget);
    
    barWidget = widget;
    }

  // cache the widget. we'll clean this up on the NodeRemovedEvent
  this->ControllerMap[n] = barWidget;
}

//-----------------------------------------------------------------------------
void 
qSlicerSliceControllerModuleWidgetPrivate::removeController(vtkMRMLNode *n)
{
  // find the widget for the SliceNode
  ControllerMapType::iterator cit = this->ControllerMap.find(n);
  if (cit == this->ControllerMap.end())
    {
    logger.trace(QString("removeController - Node has no Controller managed by this module."));
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

  // delete the widget
  delete (*cit).second;

  // remove entry from the map
  this->ControllerMap.erase(cit);
}


//-----------------------------------------------------------------------------
qSlicerSliceControllerModuleWidget::qSlicerSliceControllerModuleWidget(QWidget* _parentWidget)
  : Superclass(_parentWidget),
    d_ptr(new qSlicerSliceControllerModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerSliceControllerModuleWidget::~qSlicerSliceControllerModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerSliceControllerModuleWidget::setup()
{
  Q_D(qSlicerSliceControllerModuleWidget);
  d->setupUi(this);
  d->SliceInformationCollapsibleButton->setCollapsed(true);
}

//-----------------------------------------------------------------------------
void qSlicerSliceControllerModuleWidget::setMRMLScene(vtkMRMLScene *newScene)
{
  Q_D(qSlicerSliceControllerModuleWidget);

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
  newScene->InitTraversal();
  for (vtkMRMLNode *sn = NULL; (sn=newScene->GetNextNodeByClass("vtkMRMLSliceNode"));)
    {
    vtkMRMLSliceNode *snode = vtkMRMLSliceNode::SafeDownCast(sn);
    if (snode)
      {
      d->createController(snode, layoutManager);
      }
    }

  newScene->InitTraversal();
  for (vtkMRMLNode *sn = NULL; (sn=newScene->GetNextNodeByClass("vtkMRMLViewNode"));)
    {
    vtkMRMLViewNode *vnode = vtkMRMLViewNode::SafeDownCast(sn);
    if (vnode)
      {
      d->createController(vnode, layoutManager);
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
void qSlicerSliceControllerModuleWidget::onNodeAddedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerSliceControllerModuleWidget);

  if (!this->mrmlScene() || this->mrmlScene()->GetIsUpdating())
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
    QString layoutName = sliceNode->GetLayoutName();
    logger.trace(QString("onNodeAddedEvent - layoutName: %1").arg(layoutName));

    // create the slice controller
    d->createController(sliceNode, layoutManager);
    }

  vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(node);
  if (viewNode)
    {
    QString layoutName = viewNode->GetName();
    logger.trace(QString("onNodeAddedEvent - layoutName: %1").arg(layoutName));

    // create the view controller
    d->createController(viewNode, layoutManager);
    }
}

// --------------------------------------------------------------------------
void qSlicerSliceControllerModuleWidget::onNodeRemovedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerSliceControllerModuleWidget);

  if (!this->mrmlScene() || this->mrmlScene()->GetIsUpdating())
    {
    return;
    }

  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  if (sliceNode)
    {
    QString layoutName = sliceNode->GetLayoutName();
    logger.trace(QString("onNodeRemovedEvent - layoutName: %1").arg(layoutName));
                                             
    // destroy the slice controller
    d->removeController(sliceNode);
    }

  vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(node);
  if (sliceNode)
    {
    QString layoutName = viewNode->GetName();
    logger.trace(QString("onNodeRemovedEvent - layoutName: %1").arg(layoutName));
                                             
    // destroy the view controller
    d->removeController(viewNode);
    }
}

void qSlicerSliceControllerModuleWidget::onLayoutChanged(int)
{
  Q_D(qSlicerSliceControllerModuleWidget);

  if (!this->mrmlScene() || this->mrmlScene()->GetIsUpdating())
    {
    return;
    }

  logger.trace(QString("onLayoutChanged"));

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
  vtkObject *v;

  // hide Controllers for Nodes not currently visible in
  // the layout
  qSlicerSliceControllerModuleWidgetPrivate::ControllerMapType::iterator cit;
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
  for (visibleViews->InitTraversal(); (v = visibleViews->GetNextItemAsObject());)
    {
    vtkMRMLNode *vn = vtkMRMLNode::SafeDownCast(v);
    if (vn)
      {
      // find the controller
      qSlicerSliceControllerModuleWidgetPrivate::ControllerMapType::iterator cit = d->ControllerMap.find(vn);
      if (cit != d->ControllerMap.end())
        {
        // show it
        (*cit).second->show();
        }
      }
    }
}


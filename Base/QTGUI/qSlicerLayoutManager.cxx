
// Qt includes
#include <QButtonGroup>
#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QWidget>
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
#include "qSlicerLayoutManager.h"
#include "qSlicerLayoutManager_p.h"

// MRML includes
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkSmartPointer.h>
#include <vtkIntArray.h>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.base.qtgui.qSlicerLayoutManager");
//--------------------------------------------------------------------------
  
//------------------------------------------------------------------------------
// qSlicerLayoutManagerPrivate methods

//------------------------------------------------------------------------------
qSlicerLayoutManagerPrivate::qSlicerLayoutManagerPrivate()
{
  this->MRMLScene = 0;
  this->MRMLLayoutNode = 0;
  this->MRMLSliceLogics = vtkCollection::New();
  this->SliceControllerButtonGroup = 0;
  this->GridLayout = 0;
  this->TargetWidget = 0;
  this->CurrentViewArrangement = vtkMRMLLayoutNode::SlicerLayoutNone;
  this->SavedCurrentViewArrangement = vtkMRMLLayoutNode::SlicerLayoutNone;
}

//------------------------------------------------------------------------------
qSlicerLayoutManagerPrivate::~qSlicerLayoutManagerPrivate()
{
  if (this->MRMLSliceLogics)
    {
    this->MRMLSliceLogics->RemoveAllItems();
    this->MRMLSliceLogics->Delete();
    }
}

//------------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::setMRMLScene(vtkMRMLScene* scene)
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
  foreach (qMRMLSliceWidget* sliceWidget, this->SliceViewMap.values() )
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
void qSlicerLayoutManagerPrivate::setMRMLLayoutNode(vtkMRMLLayoutNode* layoutNode)
{
  this->qvtkReconnect(this->MRMLLayoutNode, layoutNode, vtkCommand::ModifiedEvent,
                    this, SLOT(onLayoutNodeModifiedEvent(vtkObject*)));
  this->MRMLLayoutNode = layoutNode;
}


// --------------------------------------------------------------------------
QWidget* qSlicerLayoutManagerPrivate::createSliceView(vtkMRMLSliceNode* sliceNode)
{
  Q_ASSERT(this->MRMLScene);
  Q_ASSERT(sliceNode);

  if (!this->TargetWidget)
    {
    return 0;
    }

  qMRMLSliceWidget * sliceView = 0;
  const QString sliceViewName = sliceNode->GetLayoutName();

  if (this->SliceViewMap.contains(sliceViewName))
    {
    sliceView = this->SliceViewMap[sliceViewName];

    if (sliceView->mrmlSliceNode() != sliceNode)
      {
      sliceView->setMRMLSliceNode(sliceNode);
      }
    logger.trace(
        QString("createSliceView - return existing qMRMLSliceWidget: %1").arg(sliceViewName));
    }
  else
    {
    sliceView = new qMRMLSliceWidget(this->TargetWidget);
    sliceView->registerDisplayableManagers(this->ScriptedDisplayableManagerDirectory);
    sliceView->sliceController()->setControllerButtonGroup(this->SliceControllerButtonGroup);
    sliceView->setSliceViewName(sliceViewName);
    sliceView->setMRMLScene(this->MRMLScene);
    sliceView->setMRMLSliceNode(sliceNode);

    this->SliceViewMap[sliceViewName] = sliceView;
    this->MRMLSliceLogics->AddItem(sliceView->sliceLogic());
    logger.trace(
        QString("createSliceView - instantiated new qMRMLSliceWidget: %1").arg(sliceViewName));

    // Note: Python code shouldn't be added to the layout manager itself !

////#ifdef Slicer3_USE_PYTHONQT
//    // Register this slice view with the python layer
//    qSlicerPythonManager *py = qSlicerApplication::application()->pythonManager();
//    py->executeString(QString("slicer.sliceView%1 = _sliceView()").arg(sliceViewName));
//    QString instName = QString("slicer.sliceView%1.%2");
//    py->addVTKSlicerObject(
//      instName.arg(sliceViewName, "mrmlScene").toLatin1().constData(),
//      this->MRMLScene);
//    py->addVTKSlicerObject(
//      instName.arg(sliceViewName, "sliceLogic").toLatin1().constData(),
//      sliceView->sliceController()->sliceLogic());
//    py->addVTKObject(
//      instName.arg(sliceViewName, "renderWindow").toLatin1().constData(),
//      sliceView->sliceView()->renderWindow());
//    //py->addVTKObject(
//      //instName.arg(sliceViewName, "interactor").toLatin1().constData(),
//      //sliceView->interactor());
//    py->addVTKObject(
//      instName.arg(sliceViewName, "interactorStyle").toLatin1().constData(),
//      sliceView->sliceView()->interactorStyle());
//    //py->addVTKObject(
//      //instName.arg(sliceViewName, "cornerAnnotation").toLatin1().constData(),
//      //sliceView->cornerAnnotation());
//    py->executeString(QString("registerScriptedDisplayableManagers('%1')").arg(sliceViewName));
////#endif
    }

  return sliceView;
}

// --------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::removeSliceView(vtkMRMLSliceNode* sliceNode)
{
  // removeSliceView doesn't delete anything, it just removes the node from the view
  Q_ASSERT(sliceNode);

  QString sliceViewName = QString::fromLatin1(sliceNode->GetLayoutName());
  Q_ASSERT(!sliceViewName.isEmpty());

  if (!this->SliceViewMap.contains(sliceViewName))
    {
    return;
    }
  qMRMLSliceWidget * sliceViewWidget = this->SliceViewMap.value(sliceViewName);
  Q_ASSERT(sliceViewWidget);

  sliceViewWidget->setMRMLSliceNode(0);
}

// --------------------------------------------------------------------------
QWidget* qSlicerLayoutManagerPrivate::createThreeDView(vtkMRMLViewNode* viewNode)
{
  Q_ASSERT(this->MRMLScene);
  Q_ASSERT(viewNode);

  if (!this->TargetWidget)
    {
    return 0;
    }

  qMRMLThreeDView* threeDView = 0;
  
  int viewNodeIndex = this->MRMLViewNodeList.indexOf(viewNode);
  if (viewNodeIndex != -1)
    {
    Q_ASSERT(0 >= viewNodeIndex && viewNodeIndex < this->ThreeDViewList.size());
    threeDView = this->ThreeDViewList[viewNodeIndex];
    logger.trace("createThreeDView - return existing qMRMLThreeDView");
    if (threeDView->mrmlViewNode() != viewNode)
      {
      threeDView->setMRMLViewNode(viewNode);
      }
    }
  else
    {
    logger.trace("createThreeDView - instantiated new qMRMLThreeDView");
    threeDView = new qMRMLThreeDView(this->TargetWidget);
    threeDView->registerDisplayableManagers(this->ScriptedDisplayableManagerDirectory);
    threeDView->setMRMLScene(this->MRMLScene);
    threeDView->setMRMLViewNode(viewNode);

    //QObject::connect(qSlicerApplication::application(),
    //                SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
    //                threeDView,
    //                SLOT(setMRMLScene(vtkMRMLScene*)));

    this->ThreeDViewList.push_back(threeDView);
    this->MRMLViewNodeList.push_back(viewNode);
    }

  return threeDView;
}

// --------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::removeThreeDView(vtkMRMLViewNode* viewNode)
{
  Q_ASSERT(viewNode);
  int indexViewNode = this->MRMLViewNodeList.indexOf(viewNode);

  // The MRMLViewNode is expected to be in the list
  if (indexViewNode < 0)
    {
    return;
    }

  // The 'indexViewNode' should also be a valid ThreeDViewList index
  Q_ASSERT(indexViewNode < this->ThreeDViewList.size());

  qMRMLThreeDView * threeDView = this->ThreeDViewList.at(indexViewNode);

  // The MRMLViewNode associated with the corresponding ThreeRenderView should match
  Q_ASSERT(viewNode == threeDView->mrmlViewNode());

  // Remove threeDView
  this->MRMLViewNodeList.removeAt(indexViewNode);
  this->ThreeDViewList.removeAt(indexViewNode);
  delete threeDView;
}

// --------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::onNodeAddedEvent(vtkObject* scene, vtkObject* node)
{
  Q_ASSERT(scene);

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
    this->createSliceView(sliceNode);
    }
}

// --------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::onNodeRemovedEvent(vtkObject* scene, vtkObject* node)
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
void qSlicerLayoutManagerPrivate::onSceneAboutToBeClosedEvent()
{
  logger.trace("onSceneAboutToBeClosedEvent");

  this->clearLayout(this->GridLayout);
  this->SavedCurrentViewArrangement = this->CurrentViewArrangement;
  this->CurrentViewArrangement = vtkMRMLLayoutNode::SlicerLayoutNone;
}

//------------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::onSceneClosedEvent()
{
  logger.trace("onSceneClosedEvent");
  if (!this->MRMLScene->GetIsConnecting())
    {
    // Since the loaded scene may not contain the required node, calling initialize 
    // will make sure the LayoutNode, MRMLViewNode, MRMLSliceNode exists.
    this->initialize();

    // Make sure the layoutNode arrangement match the LayoutManager one
    Q_ASSERT(this->MRMLLayoutNode);
    this->MRMLLayoutNode->SetViewArrangement(this->SavedCurrentViewArrangement);
    }
}

//------------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::onSceneImportedEvent()
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
void qSlicerLayoutManagerPrivate::onLayoutNodeModifiedEvent(vtkObject* layoutNode)
{
  CTK_P(qSlicerLayoutManager);

  vtkMRMLLayoutNode * mrmlLayoutNode = vtkMRMLLayoutNode::SafeDownCast(layoutNode);

  if (!mrmlLayoutNode)
    {
    Q_ASSERT(mrmlLayoutNode);
    return;
    }
  p->setLayout(mrmlLayoutNode->GetViewArrangement());
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
void qSlicerLayoutManagerPrivate::initialize()
{
  if (!this->MRMLScene)
    {
    return;
    }
  // Create vtkMRMLViewNode if required
  int viewNodeCount = qMRMLUtils::countVisibleViewNode(this->MRMLScene);
  Q_ASSERT(viewNodeCount >= 0 && viewNodeCount <= 2);
  if (viewNodeCount == 0)
    {
    vtkMRMLNode * node = qMRMLNodeFactory::createNode(this->MRMLScene, "vtkMRMLViewNode");
    Q_ASSERT(node);
    Q_UNUSED(node);
    }
  else
    {
    // maybe the nodes have been created a while ago, we need to associate a view to each of them
    std::vector<vtkMRMLNode*> viewNodes;
    this->MRMLScene->GetNodesByClass("vtkMRMLViewNode", viewNodes);
    for (unsigned int i = 0; i < viewNodes.size();++i)
      {
      vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(viewNodes[i]);
      int index = this->MRMLViewNodeList.indexOf(viewNode);
      if (index < 0 && viewNode->GetVisibility())
        {
        this->createThreeDView(viewNode);
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
    // maybe the nodes have been created a while ago, we need to associate a view to each of them
    std::vector<vtkMRMLNode*> sliceNodes;
    this->MRMLScene->GetNodesByClass("vtkMRMLSliceNode", sliceNodes);
    for (unsigned int i = 0; i < sliceNodes.size();++i)
      {
      vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(sliceNodes[i]);
      QString sliceViewName = QString::fromLatin1(sliceNode->GetLayoutName());
      if (!this->SliceViewMap.contains(sliceViewName))
        {
        this->createSliceView(sliceNode);
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
bool qSlicerLayoutManagerPrivate::startUpdateLayout()
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
void qSlicerLayoutManagerPrivate::endUpdateLayout(bool updatesEnabled)
{
  if (!this->TargetWidget)
    {
    return;
    }
  this->TargetWidget->setUpdatesEnabled(updatesEnabled);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::clearLayout(QLayout* layout)
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
qMRMLThreeDView* qSlicerLayoutManagerPrivate::threeDView(int id)
{
  Q_ASSERT(id >=0 && id < this->ThreeDViewList.size());
  return this->ThreeDViewList.at(id);
}

//------------------------------------------------------------------------------
qMRMLSliceWidget* qSlicerLayoutManagerPrivate::sliceView(const QString& name)
{
  Q_ASSERT(this->SliceViewMap.contains(name));
  qMRMLSliceWidget* sliceView = this->SliceViewMap[name];
  Q_ASSERT(sliceView);
  return sliceView;
}

//------------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::setLayoutInternal(int layout)
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
      case vtkMRMLLayoutNode::SlicerLayoutSideBySideCompareView:
        this->setSideBySideCompareView();
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
void qSlicerLayoutManagerPrivate::setConventionalView()
{
  logger.trace(QString("switch to ConventionalView"));
  // First render view
  qMRMLThreeDView * renderView = this->threeDView(0);
  this->GridLayout->addWidget(renderView, 0, 0, 1, -1); // fromRow, fromColumn, rowSpan, columnSpan
  renderView->setVisible(true);

  // Red Slice Viewer
  qMRMLSliceWidget* redSliceView = this->sliceView("Red");
  this->GridLayout->addWidget(redSliceView, 1, 0);
  redSliceView->setVisible(true);

  // Yellow Slice Viewer
  qMRMLSliceWidget* yellowSliceView = this->sliceView("Yellow");
  this->GridLayout->addWidget(yellowSliceView, 1, 1);
  yellowSliceView->setVisible(true);

  // Green Slice Viewer
  qMRMLSliceWidget* greenSliceView = this->sliceView("Green");
  this->GridLayout->addWidget(greenSliceView, 1, 2);
  greenSliceView->setVisible(true);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::setOneUp3DView()
{
  logger.trace(QString("switch to OneUp3DView"));
  // First render view
  qMRMLThreeDView * renderView = this->threeDView(0);
  this->GridLayout->addWidget(renderView, 0, 0, 1, -1); // fromRow, fromColumn, rowSpan, columnSpan
  renderView->setVisible(true);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::setOneUpSliceView(const QString& sliceViewName)
{
  logger.trace(QString("switch to OneUpSlice%1").arg(sliceViewName));
  // Sanity checks
  Q_ASSERT(sliceViewName == "Red" || sliceViewName == "Yellow" || sliceViewName == "Green");
  if (sliceViewName != "Red" && sliceViewName != "Yellow" && sliceViewName != "Green")
    {
    qWarning() << "Slicer viewer name" << sliceViewName << "invalid !";
    return;
    }
  // Slice viewer
  qMRMLSliceWidget* sliceView = this->sliceView(sliceViewName);
  sliceView->setVisible(true);
  this->GridLayout->addWidget(sliceView, 1, 0);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::setFourUpView()
{
  logger.trace(QString("switch to FourUpView"));
  // First render view
  qMRMLThreeDView * renderView = this->threeDView(0);
  this->GridLayout->addWidget(renderView, 0, 1); // fromRow, fromColumn, rowSpan, columnSpan
  renderView->setVisible(true);

  // Red Slice Viewer
  qMRMLSliceWidget* redSliceView = this->sliceView("Red");
  this->GridLayout->addWidget(redSliceView, 0, 0);
  redSliceView->setVisible(true);

  // Yellow Slice Viewer
  qMRMLSliceWidget* yellowSliceView = this->sliceView("Yellow");
  this->GridLayout->addWidget(yellowSliceView, 1, 0);
  yellowSliceView->setVisible(true);

  // Green Slice Viewer
  qMRMLSliceWidget* greenSliceView = this->sliceView("Green");
  this->GridLayout->addWidget(greenSliceView, 1, 1);
  greenSliceView->setVisible(true);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::setTabbed3DView()
{
  logger.trace(QString("switch to Tabbed3DView"));
  qDebug() << "qSlicerLayoutManager::setTabbed3DView  not implemented";
  logger.debug("setTabbed3DView - Not Implemented");
}

//------------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::setTabbedSliceView()
{
  logger.trace(QString("switch to TabbedSliceView"));
  qDebug() << "qSlicerLayoutManager::setTabbedSliceView  not implemented";
  logger.debug("setTabbedSliceView - Not Implemented");
}

//------------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::setLightboxView()
{
  logger.trace(QString("switch to LightboxView"));
  qDebug() << "qSlicerLayoutManager::setLightboxView  not implemented";
  logger.debug("setLightboxView - Not Implemented");
}

//------------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::setCompareView()
{
  logger.trace(QString("switch to CompareView"));
  qDebug() << "qSlicerLayoutManager::setCompareView not implemented";
  logger.debug("setCompareView - Not Implemented");
}

//------------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::setSideBySideCompareView()
{
  logger.trace(QString("switch to SideBySideCompareView"));
  qDebug() << "qSlicerLayoutManager::setSideBySideCompareView";
  logger.debug("setSideBySideCompareView - Not Implemented");
}

//------------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::setDual3DView()
{
  logger.trace(QString("switch to Dual3DView"));
  // First render view
  qMRMLThreeDView * renderView = this->threeDView(0);
  this->GridLayout->addWidget(renderView, 0, 0); // fromRow, fromColumn, rowSpan, columnSpan
  renderView->setVisible(true);

  // Create a second RenderView if needed
  int viewNodeCount = qMRMLUtils::countVisibleViewNode(this->MRMLScene);
  Q_ASSERT(viewNodeCount >= 0 && viewNodeCount <= 2);
  if (viewNodeCount == 1)
    {
    vtkMRMLNode * node = qMRMLNodeFactory::createNode(this->MRMLScene, "vtkMRMLViewNode");
    Q_ASSERT(node);
    Q_UNUSED(node);
    }

  // Second render view
  qMRMLThreeDView * renderView2 = this->threeDView(1);
  this->GridLayout->addWidget(renderView2, 0, 1);
  renderView2->setVisible(true);

  // Add an horizontal layout to group the 3 sliceViews
  QHBoxLayout * sliceViewLayout = new QHBoxLayout();
  this->GridLayout->addLayout(sliceViewLayout, 1, 0, 1, 2); // fromRow, fromColumn, rowSpan, columnSpan

  // Red Slice Viewer
  qMRMLSliceWidget* redSliceView = this->sliceView("Red");
  sliceViewLayout->addWidget(redSliceView);
  redSliceView->setVisible(true);

  // Yellow Slice Viewer
  qMRMLSliceWidget* yellowSliceView = this->sliceView("Yellow");
  sliceViewLayout->addWidget(yellowSliceView);
  yellowSliceView->setVisible(true);

  // Green Slice Viewer
  qMRMLSliceWidget* greenSliceView = this->sliceView("Green");
  sliceViewLayout->addWidget(greenSliceView);
  greenSliceView->setVisible(true);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::setNone()
{
  logger.trace(QString("switch to None"));
}

//------------------------------------------------------------------------------
// qSlicerLayoutManager methods

// --------------------------------------------------------------------------
qSlicerLayoutManager::qSlicerLayoutManager(QWidget* widget) : Superclass(widget)
{
  CTK_INIT_PRIVATE(qSlicerLayoutManager);
  this->setViewport(widget);
}

// --------------------------------------------------------------------------
void qSlicerLayoutManager::setViewport(QWidget* widget)
{
  CTK_D(qSlicerLayoutManager);
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
  foreach (qMRMLSliceWidget* sliceWidget, d->SliceViewMap.values())
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
QWidget* qSlicerLayoutManager::viewport()const
{
  CTK_D(const qSlicerLayoutManager);
  return d->TargetWidget;
}

//------------------------------------------------------------------------------
qMRMLSliceWidget* qSlicerLayoutManager::sliceView(const QString& name)
{
  CTK_D(qSlicerLayoutManager);
  if (!d->SliceViewMap.contains(name))
    {
    return 0;
    }
  return d->SliceViewMap[name];
}

//------------------------------------------------------------------------------
int qSlicerLayoutManager::threeDViewCount()
{
  CTK_D(qSlicerLayoutManager);
  return d->ThreeDViewList.size();
}

//------------------------------------------------------------------------------
qMRMLThreeDView* qSlicerLayoutManager::threeDView(int id)
{
  CTK_D(qSlicerLayoutManager);
  if(id < 0 || id >= d->ThreeDViewList.size())
    {
    return 0;
    }
  return d->threeDView(id);
}

//------------------------------------------------------------------------------
vtkCollection* qSlicerLayoutManager::mrmlSliceLogics()const
{
  CTK_D(const qSlicerLayoutManager);
  return d->MRMLSliceLogics;
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::setMRMLScene(vtkMRMLScene* scene)
{
  CTK_D(qSlicerLayoutManager);
  d->setMRMLScene(scene);
}

//------------------------------------------------------------------------------
vtkMRMLScene* qSlicerLayoutManager::mrmlScene()const
{
  CTK_D(const qSlicerLayoutManager);
  return d->MRMLScene;
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::setScriptedDisplayableManagerDirectory(
    const QString& scriptedDisplayableManagerDirectory)
{
#ifdef MRMLDisplayableManager_USE_PYTHON
  CTK_D(qSlicerLayoutManager);

  Q_ASSERT(QFileInfo(scriptedDisplayableManagerDirectory).isDir());
  d->ScriptedDisplayableManagerDirectory = scriptedDisplayableManagerDirectory;
#endif
}

//------------------------------------------------------------------------------
int qSlicerLayoutManager::layout()const
{
  CTK_D(const qSlicerLayoutManager);
  return d->CurrentViewArrangement;
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::setLayout(int layout)
{
  CTK_D(qSlicerLayoutManager);
  if (d->CurrentViewArrangement == layout)
    {
    return;
    }
  d->setLayoutInternal(layout);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToOneUpSliceView(const QString& sliceViewName)
{
  if (sliceViewName == "Red")
    {
    this->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
    }
  else if (sliceViewName == "Green")
    {
    this->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
    }
  else if (sliceViewName == "Yellow")
    {
    this->setLayout(vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView);
    }
  else
    {
    logger.warn(QString("Unknown view : %1").arg(sliceViewName));
    }
}

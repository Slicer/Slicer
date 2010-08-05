
// Qt includes
#include <QButtonGroup>
#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QWidget>

// CTK includes
#include <ctkLogger.h>

// qMRMLWidgets includes
#include <qMRMLThreeDView.h>
#include <qMRMLSliceViewWidget.h>
#include <qMRMLSliceControllerWidget.h>
#include <qMRMLUtils.h>
#include <qMRMLNodeFactory.h>

// SlicerQt includes
#include "qSlicerLayoutManager.h"
#include "qSlicerLayoutManager_p.h"
#include "qSlicerApplication.h"

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
  this->GridLayout = 0;
  this->TargetWidget = 0;
  this->CurrentViewArrangement = vtkMRMLLayoutNode::SlicerLayoutNone;
  this->SavedCurrentViewArrangement = vtkMRMLLayoutNode::SlicerLayoutNone;
  this->UpdatingMRMLLayoutNode = false;
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
}

// --------------------------------------------------------------------------
QWidget* qSlicerLayoutManagerPrivate::createSliceView(const QString& sliceViewName,
                                                      vtkMRMLSliceNode* sliceNode)
{
  Q_ASSERT(this->MRMLScene);
  Q_ASSERT(sliceNode);

  qMRMLSliceViewWidget * sliceView = 0;

  if (this->SliceViewMap.contains(sliceViewName))
    {
    sliceView = this->SliceViewMap[sliceViewName];

    if (sliceView->mrmlSliceNode() != sliceNode)
      {
      sliceView->setMRMLSliceNode(sliceNode);
      }
    logger.trace(
        QString("createSliceView - return existing qMRMLSliceViewWidget: %1").arg(sliceViewName));
    }
  else
    {
    sliceView = new qMRMLSliceViewWidget(this->TargetWidget);
    sliceView->sliceController()->setControllerButtonGroup(this->SliceControllerButtonGroup);
    sliceView->setSliceViewName(sliceViewName);
    sliceView->setMRMLScene(this->MRMLScene);
    sliceView->setMRMLSliceNode(sliceNode);

    QObject::connect(qSlicerApplication::application(),
                    SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                    sliceView,
                    SLOT(setMRMLScene(vtkMRMLScene*)));

    this->SliceViewMap[sliceViewName] = sliceView;
    this->MRMLSliceLogics->AddItem(sliceView->sliceLogic());
    logger.trace(
        QString("createSliceView - instantiated new qMRMLSliceViewWidget: %1").arg(sliceViewName));
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

  // The sliceNode name is expected to be in the map
  Q_ASSERT(this->SliceViewMap.contains(sliceViewName));

  qMRMLSliceViewWidget * sliceViewWidget = this->SliceViewMap.value(sliceViewName);
  Q_ASSERT(sliceViewWidget);

  sliceViewWidget->setMRMLSliceNode(0);

}

// --------------------------------------------------------------------------
QWidget* qSlicerLayoutManagerPrivate::createThreeDView(vtkMRMLViewNode* viewNode)
{
  Q_ASSERT(this->MRMLScene);
  Q_ASSERT(viewNode);

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
    threeDView->setMRMLScene(this->MRMLScene);
    threeDView->setMRMLViewNode(viewNode);

    QObject::connect(qSlicerApplication::application(),
                    SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                    threeDView,
                    SLOT(setMRMLScene(vtkMRMLScene*)));

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
  Q_ASSERT(indexViewNode >= 0);

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
    
    this->qvtkConnect(layoutNode, vtkCommand::ModifiedEvent,
      this, SLOT(onLayoutNodeModifiedEvent(vtkObject*)));

    this->MRMLLayoutNode = layoutNode;
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
    this->createSliceView(layoutName, sliceNode);
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

    this->qvtkDisconnect(layoutNode, vtkCommand::ModifiedEvent,
      this, SLOT(onLayoutNodeModifiedEvent(vtkObject*)));
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
  //this->MRMLLayoutNode->SetViewArrangement(this->SavedCurrentViewArrangement);
  this->onLayoutNodeModifiedEvent(this->MRMLLayoutNode);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::onLayoutNodeModifiedEvent(vtkObject* layoutNode)
{
  CTK_P(qSlicerLayoutManager);

  // If the node is updated in one of the "switchTo*" slot
  if (this->UpdatingMRMLLayoutNode)
    {
    return;
    }
  
  vtkMRMLLayoutNode * mrmlLayoutNode = vtkMRMLLayoutNode::SafeDownCast(layoutNode);
  
  Q_ASSERT(mrmlLayoutNode);
  if (!mrmlLayoutNode)
    {
    return;
    }

  if (this->CurrentViewArrangement == mrmlLayoutNode->GetViewArrangement())
    {
    return;
    }

  #define MRMLLayoutManager_arrangement_case(_NAME)                                \
  case vtkMRMLLayoutNode::SlicerLayout##_NAME:                                     \
    p->switchTo##_NAME();                                                          \
    logger.trace(QString("onLayoutNodeModifiedEvent - switch to %1").arg(#_NAME)); \
    this->CurrentViewArrangement = vtkMRMLLayoutNode::SlicerLayout##_NAME;         \
    break;

  switch(mrmlLayoutNode->GetViewArrangement())
    {
    MRMLLayoutManager_arrangement_case(ConventionalView);
    MRMLLayoutManager_arrangement_case(FourUpView);
    MRMLLayoutManager_arrangement_case(OneUp3DView);
    MRMLLayoutManager_arrangement_case(OneUpRedSliceView);
    MRMLLayoutManager_arrangement_case(OneUpYellowSliceView);
    MRMLLayoutManager_arrangement_case(OneUpGreenSliceView);
    MRMLLayoutManager_arrangement_case(Tabbed3DView);
    MRMLLayoutManager_arrangement_case(TabbedSliceView);
    MRMLLayoutManager_arrangement_case(LightboxView);
    MRMLLayoutManager_arrangement_case(CompareView);
    MRMLLayoutManager_arrangement_case(SideBySideCompareView);
    MRMLLayoutManager_arrangement_case(Dual3DView);
    MRMLLayoutManager_arrangement_case(None);
    default:
      logger.warn(QString("Unknown view layout: %1").arg(mrmlLayoutNode->GetViewArrangement()));
      break;
    }

  #undef MRMLLayoutManager_arrangement_case
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
  Q_ASSERT(this->MRMLScene);
  
  // Create vtkMRMLLayoutNode if required
  int layoutNodeCount = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLLayoutNode");
  Q_ASSERT(layoutNodeCount == 0 || layoutNodeCount == 1);
  if (layoutNodeCount == 0)
    {
    VTK_CREATE(vtkMRMLLayoutNode, layoutNode);
    vtkMRMLNode * nodeCreated = this->MRMLScene->AddNode(layoutNode);
    Q_ASSERT(nodeCreated);
    Q_UNUSED(nodeCreated);
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
}

////------------------------------------------------------------------------------
//void qSlicerLayoutManagerPrivate::saveCurrentViewArrangement()
//{
//  this->SavedCurrentViewArrangement = this->CurrentViewArrangement;
//}
//
////------------------------------------------------------------------------------
//void qSlicerLayoutManagerPrivate::restoreCurrentViewArrangement()
//{
//
//}

//------------------------------------------------------------------------------
bool qSlicerLayoutManagerPrivate::startUpdateLayout()
{
  bool updatesEnabled = this->TargetWidget->updatesEnabled();
  this->TargetWidget->setUpdatesEnabled(false);
  return updatesEnabled;
}

//------------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::endUpdateLayout(bool updatesEnabled)
{
  this->TargetWidget->setUpdatesEnabled(updatesEnabled);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::clearLayout(QLayout* layout)
{
  Q_ASSERT(layout);
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
qMRMLSliceViewWidget* qSlicerLayoutManagerPrivate::sliceView(const QString& name)
{
  Q_ASSERT(this->SliceViewMap.contains(name));
  qMRMLSliceViewWidget* sliceView = this->SliceViewMap[name];
  Q_ASSERT(sliceView);
  return sliceView;
}

//------------------------------------------------------------------------------
// qSlicerLayoutManager methods

// --------------------------------------------------------------------------
qSlicerLayoutManager::qSlicerLayoutManager(QWidget* widget) : Superclass(widget)
{
  Q_ASSERT(widget);
  CTK_INIT_PRIVATE(qSlicerLayoutManager);
  CTK_D(qSlicerLayoutManager);
  d->SliceControllerButtonGroup = new QButtonGroup(widget);
  d->SliceControllerButtonGroup->setExclusive(false);
  d->GridLayout = new QGridLayout(widget);
  d->GridLayout->setContentsMargins(6, 6, 6, 6);
  d->GridLayout->setSpacing(0);
  d->TargetWidget = widget;
}

//------------------------------------------------------------------------------
qMRMLSliceViewWidget* qSlicerLayoutManager::sliceView(const QString& name)
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
  d->initialize();

  // Default layout
  this->switchToConventionalView();
}

//------------------------------------------------------------------------------
// Convenient macros
#define qSlicerLayoutManager_updateLayoutNode(_NAME)                             \
  d->CurrentViewArrangement = vtkMRMLLayoutNode::SlicerLayout##_NAME;            \
  d->UpdatingMRMLLayoutNode = true;                                              \
  d->MRMLLayoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayout##_NAME); \
  d->UpdatingMRMLLayoutNode = false;

#define qSlicerLayoutManager_returnIfMatchCurrentView(_NAME)                      \
  if (!ctk_d()->MRMLScene)                                                        \
    {                                                                             \
    return;                                                                       \
    }                                                                             \
  if (ctk_d()->CurrentViewArrangement == vtkMRMLLayoutNode::SlicerLayout##_NAME)  \
    {                                                                             \
    return;                                                                       \
    }                                                                             \
    logger.trace(QString("switch to %1").arg(#_NAME));

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToConventionalView()
{
  CTK_D(qSlicerLayoutManager);
  qSlicerLayoutManager_returnIfMatchCurrentView(ConventionalView);
  d->initialize();
  bool updatesEnabled = d->startUpdateLayout();
  d->clearLayout(d->GridLayout);

  // First render view
  qMRMLThreeDView * renderView = d->threeDView(0);
  d->GridLayout->addWidget(renderView, 0, 0, 1, -1); // fromRow, fromColumn, rowSpan, columnSpan
  renderView->setVisible(true);

  // Red Slice Viewer
  qMRMLSliceViewWidget* redSliceView = d->sliceView("Red");
  d->GridLayout->addWidget(redSliceView, 1, 0);
  redSliceView->setVisible(true);

  // Yellow Slice Viewer
  qMRMLSliceViewWidget* yellowSliceView = d->sliceView("Yellow");
  d->GridLayout->addWidget(yellowSliceView, 1, 1);
  yellowSliceView->setVisible(true);

  // Green Slice Viewer
  qMRMLSliceViewWidget* greenSliceView = d->sliceView("Green");
  d->GridLayout->addWidget(greenSliceView, 1, 2);
  greenSliceView->setVisible(true);

  // Update LayoutNode
  qSlicerLayoutManager_updateLayoutNode(ConventionalView);

  d->endUpdateLayout(updatesEnabled);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToOneUp3DView()
{
  CTK_D(qSlicerLayoutManager);
  qSlicerLayoutManager_returnIfMatchCurrentView(OneUp3DView);
  d->initialize();

  bool updatesEnabled = d->startUpdateLayout();
  d->clearLayout(d->GridLayout);

  // First render view
  qMRMLThreeDView * renderView = d->threeDView(0);
  d->GridLayout->addWidget(renderView, 0, 0, 1, -1); // fromRow, fromColumn, rowSpan, columnSpan
  renderView->setVisible(true);

  // Update LayoutNode
  qSlicerLayoutManager_updateLayoutNode(OneUp3DView);
  
  d->endUpdateLayout(updatesEnabled);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToOneUpRedSliceView()
{
  qSlicerLayoutManager_returnIfMatchCurrentView(OneUpRedSliceView);
  this->switchToOneUpSliceView("Red");
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToOneUpGreenSliceView()
{
  qSlicerLayoutManager_returnIfMatchCurrentView(OneUpGreenSliceView);
  this->switchToOneUpSliceView("Green");
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToOneUpYellowSliceView()
{
  qSlicerLayoutManager_returnIfMatchCurrentView(OneUpYellowSliceView);
  this->switchToOneUpSliceView("Yellow");
}
//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToOneUpSliceView(const QString& sliceViewName)
{
  // Sanity checks
  Q_ASSERT(sliceViewName == "Red" || sliceViewName == "Yellow" || sliceViewName == "Green");
  if (sliceViewName != "Red" && sliceViewName != "Yellow" && sliceViewName != "Green")
    {
    qWarning() << "Slicer viewer name" << sliceViewName << "invalid !";
    return;
    }
  
  CTK_D(qSlicerLayoutManager);
  d->initialize();

  bool updatesEnabled = d->startUpdateLayout();
  d->clearLayout(d->GridLayout);

  // Slice viewer
  qMRMLSliceViewWidget* sliceView = d->sliceView(sliceViewName);
  sliceView->setVisible(true);
  d->GridLayout->addWidget(sliceView, 1, 0);
    
  // Update LayoutNode
  qSlicerLayoutManager_updateLayoutNode(OneUpSliceView);

  d->endUpdateLayout(updatesEnabled);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToFourUpView()
{
  CTK_D(qSlicerLayoutManager);
  qSlicerLayoutManager_returnIfMatchCurrentView(FourUpView);
  d->initialize();

  bool updatesEnabled = d->startUpdateLayout();
  d->clearLayout(d->GridLayout);

  // First render view
  qMRMLThreeDView * renderView = d->threeDView(0);
  d->GridLayout->addWidget(renderView, 0, 1); // fromRow, fromColumn, rowSpan, columnSpan
  renderView->setVisible(true);

  // Red Slice Viewer
  qMRMLSliceViewWidget* redSliceView = d->sliceView("Red");
  d->GridLayout->addWidget(redSliceView, 0, 0);
  redSliceView->setVisible(true);

  // Yellow Slice Viewer
  qMRMLSliceViewWidget* yellowSliceView = d->sliceView("Yellow");
  d->GridLayout->addWidget(yellowSliceView, 1, 0);
  yellowSliceView->setVisible(true);

  // Green Slice Viewer
  qMRMLSliceViewWidget* greenSliceView = d->sliceView("Green");
  d->GridLayout->addWidget(greenSliceView, 1, 1);
  greenSliceView->setVisible(true);

  // Update LayoutNode
  qSlicerLayoutManager_updateLayoutNode(FourUpView);

  d->endUpdateLayout(updatesEnabled);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToTabbed3DView()
{
  CTK_D(qSlicerLayoutManager);
  qSlicerLayoutManager_returnIfMatchCurrentView(Tabbed3DView);
  d->initialize();

  bool updatesEnabled = d->startUpdateLayout();
  d->clearLayout(d->GridLayout);

  logger.debug("switchToTabbed3DView - Not Implemented");

  // Update LayoutNode
  qSlicerLayoutManager_updateLayoutNode(Tabbed3DView);
  
  d->endUpdateLayout(updatesEnabled);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToTabbedSliceView()
{
  qDebug() << "qSlicerLayoutManager::switchToTabbedSliceView";
  CTK_D(qSlicerLayoutManager);
  qSlicerLayoutManager_returnIfMatchCurrentView(TabbedSliceView);
  d->initialize();

  bool updatesEnabled = d->startUpdateLayout();
  d->clearLayout(d->GridLayout);

  logger.debug("switchToTabbedSliceView - Not Implemented");

  // Update LayoutNode
  qSlicerLayoutManager_updateLayoutNode(TabbedSliceView);

  d->endUpdateLayout(updatesEnabled);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToLightboxView()
{
  qDebug() << "qSlicerLayoutManager::switchToLightboxView";
  CTK_D(qSlicerLayoutManager);
  qSlicerLayoutManager_returnIfMatchCurrentView(LightboxView);
  d->initialize();

  bool updatesEnabled = d->startUpdateLayout();
  d->clearLayout(d->GridLayout);

  logger.debug("switchToLightboxView - Not Implemented");

  // Update LayoutNode
  qSlicerLayoutManager_updateLayoutNode(LightboxView);

  d->endUpdateLayout(updatesEnabled);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToCompareView()
{
  qDebug() << "qSlicerLayoutManager::switchToCompareView";
  CTK_D(qSlicerLayoutManager);
  qSlicerLayoutManager_returnIfMatchCurrentView(CompareView);
  d->initialize();

  bool updatesEnabled = d->startUpdateLayout();
  d->clearLayout(d->GridLayout);

  logger.debug("switchToCompareView - Not Implemented");

  // Update LayoutNode
  qSlicerLayoutManager_updateLayoutNode(CompareView);

  d->endUpdateLayout(updatesEnabled);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToSideBySideCompareView()
{
  qDebug() << "qSlicerLayoutManager::switchToSideBySideCompareView";
  CTK_D(qSlicerLayoutManager);
  qSlicerLayoutManager_returnIfMatchCurrentView(SideBySideCompareView);
  d->initialize();

  bool updatesEnabled = d->startUpdateLayout();
  d->clearLayout(d->GridLayout);

  logger.debug("switchToSideBySideCompareView - Not Implemented");

  // Update LayoutNode
  qSlicerLayoutManager_updateLayoutNode(SideBySideCompareView);

  d->endUpdateLayout(updatesEnabled);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToDual3DView()
{
  CTK_D(qSlicerLayoutManager);
  qSlicerLayoutManager_returnIfMatchCurrentView(Dual3DView);
  d->initialize();

  bool updatesEnabled = d->startUpdateLayout();
  d->clearLayout(d->GridLayout);

  // First render view
  qMRMLThreeDView * renderView = d->threeDView(0);
  d->GridLayout->addWidget(renderView, 0, 0); // fromRow, fromColumn, rowSpan, columnSpan
  renderView->setVisible(true);
  
  // Create a second RenderView if needed
  int viewNodeCount = qMRMLUtils::countVisibleViewNode(d->MRMLScene);
  Q_ASSERT(viewNodeCount >= 0 && viewNodeCount <= 2);
  if (viewNodeCount == 1)
    {
    vtkMRMLNode * node = qMRMLNodeFactory::createNode(d->MRMLScene, "vtkMRMLViewNode");
    Q_ASSERT(node);
    Q_UNUSED(node);
    }

  // Second render view
  qMRMLThreeDView * renderView2 = d->threeDView(1);
  d->GridLayout->addWidget(renderView2, 0, 1);
  renderView2->setVisible(true);

  // Add an horizontal layout to group the 3 sliceViews
  QHBoxLayout * sliceViewLayout = new QHBoxLayout();
  d->GridLayout->addLayout(sliceViewLayout, 1, 0, 1, 2); // fromRow, fromColumn, rowSpan, columnSpan

  // Red Slice Viewer
  qMRMLSliceViewWidget* redSliceView = d->sliceView("Red");
  sliceViewLayout->addWidget(redSliceView);
  redSliceView->setVisible(true);

  // Yellow Slice Viewer
  qMRMLSliceViewWidget* yellowSliceView = d->sliceView("Yellow");
  sliceViewLayout->addWidget(yellowSliceView);
  yellowSliceView->setVisible(true);

  // Green Slice Viewer
  qMRMLSliceViewWidget* greenSliceView = d->sliceView("Green");
  sliceViewLayout->addWidget(greenSliceView);
  greenSliceView->setVisible(true);

  // Update LayoutNode
  qSlicerLayoutManager_updateLayoutNode(Dual3DView);

  d->endUpdateLayout(updatesEnabled);
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToNone()
{
  CTK_D(qSlicerLayoutManager);
  qSlicerLayoutManager_returnIfMatchCurrentView(None);
  d->clearLayout(d->GridLayout);

  // Update LayoutNode
  qSlicerLayoutManager_updateLayoutNode(None);
}

#undef qSlicerLayoutManager_updateLayoutNode


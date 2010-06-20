
// Qt includes
#include <QDebug>
#include <QGridLayout>
#include <QWidget>

// CTK includes
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLThreeDRenderView.h"
#include "qMRMLUtils.h"
#include "qMRMLNodeFactory.h"

// SlicerQt includes
#include "qSlicerLayoutManager.h"
#include "qSlicerLayoutManager_p.h"
#include "qSlicerApplication.h"
#include "qSlicerSliceViewWidget.h"

// MRML includes
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
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
  this->GridLayout = 0;
  this->TargetWidget = 0;
  this->CurrentViewArrangement = vtkMRMLLayoutNode::SlicerLayoutNone;
  this->UpdatingMRMLLayoutNode = false;
  logger.setTrace();
}

//------------------------------------------------------------------------------
qSlicerLayoutManagerPrivate::~qSlicerLayoutManagerPrivate()
{
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

//   this->qvtkReconnect(
//     this->MRMLScene, scene,
//     vtkMRMLScene::SceneCloseEvent, this,
//     SLOT(onSceneCloseEvent()));
  
  this->MRMLScene = scene;
}

// --------------------------------------------------------------------------
QWidget* qSlicerLayoutManagerPrivate::createSliceView(const QString& sliceViewName,
                                                      vtkMRMLSliceNode* sliceNode)
{
  Q_ASSERT(this->MRMLScene);
  Q_ASSERT(sliceNode);

  qSlicerSliceViewWidget * sliceView = 0;
  
  //QString widgetId = QLatin1String(sliceNode->GetID());

  if (this->SliceViewMap.contains(sliceViewName))
    {
    sliceView = this->SliceViewMap[sliceViewName];
    if (sliceView->mrmlSliceNode() != sliceNode)
      {
      sliceView->setMRMLSliceNode(sliceNode);
      }
    logger.trace(
        QString("createSliceView - return existing sliceView: %1").arg(sliceViewName));
    }
  else
    {
    sliceView = new qSlicerSliceViewWidget(sliceViewName, this->TargetWidget);
    sliceView->setMRMLScene(this->MRMLScene);
    sliceView->setMRMLSliceNode(sliceNode);

    QObject::connect(qSlicerApplication::application(),
                    SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                    sliceView,
                    SLOT(setMRMLScene(vtkMRMLScene*)));

    this->SliceViewMap[sliceViewName] = sliceView;
    logger.trace(
        QString("createSliceView - instanciated new sliceView: %1").arg(sliceViewName));
    }

  return sliceView;
}

// --------------------------------------------------------------------------
QWidget* qSlicerLayoutManagerPrivate::createThreeDRenderView(const QString& renderViewName,
                                                             vtkMRMLViewNode* viewNode)
{
  Q_ASSERT(this->MRMLScene);
  Q_ASSERT(viewNode);

  qMRMLThreeDRenderView* threeDRenderView = 0;
  
  if (this->ThreeDRenderViewMap.contains(renderViewName))
    {
    threeDRenderView = this->ThreeDRenderViewMap[renderViewName];
    if (threeDRenderView->mrmlViewNode() != viewNode)
      {
      threeDRenderView->setMRMLViewNode(viewNode);
      }
    }
  else
    {
    threeDRenderView = new qMRMLThreeDRenderView(this->TargetWidget);
    threeDRenderView->setMRMLScene(this->MRMLScene);
    threeDRenderView->setMRMLViewNode(viewNode);

    QObject::connect(qSlicerApplication::application(),
                    SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                    threeDRenderView,
                    SLOT(setMRMLScene(vtkMRMLScene*)));

    this->ThreeDRenderViewMap[renderViewName] = threeDRenderView;
    }

  return threeDRenderView;
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
    QString name = QString("RenderView#%1").arg(this->ThreeDRenderViewMap.count() + 1);
    logger.trace(QString("onViewNodeAddedEvent - name: %1").arg(name));
    this->createThreeDRenderView(name, viewNode);
    }

  // Slice node
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  if (sliceNode)
    {
    QString name = sliceNode->GetName();
    logger.trace(QString("onSliceNodeAddedEvent - name: %1").arg(name));
    Q_ASSERT(name == "red" || name == "yellow" || name == "green");
    this->createSliceView(name, sliceNode);
    }
}

// --------------------------------------------------------------------------
void qSlicerLayoutManagerPrivate::onNodeRemovedEvent(vtkObject* scene, vtkObject* node)
{
  Q_ASSERT(scene);
  
  vtkMRMLLayoutNode * layoutNode = vtkMRMLLayoutNode::SafeDownCast(node);
  if (!layoutNode)
    {
    return;
    }

  // The layout to be removed should be the same as the stored one
  Q_ASSERT(this->MRMLLayoutNode == layoutNode);
    
  this->qvtkDisconnect(layoutNode, vtkCommand::ModifiedEvent,
    this, SLOT(onLayoutNodeModifiedEvent(vtkObject*)));
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

  #define MRMLLayoutManager_arrangement_case(_NAME)                                  \
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
      qWarning() << "Unknown view layout:" << mrmlLayoutNode->GetViewArrangement();
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
    sliceNode->SetName(this->SliceLogicName.toLatin1());
    sliceNode->SetLayoutName(this->SliceLogicName.toLatin1());
    sliceNode->SetSingletonTag(this->SliceLogicName.toLatin1());
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

  // Create "red", "yellow" and "green" vtkMRMLSliceNode if required
  int sliceNodeCount = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLSliceNode");
  if (sliceNodeCount == 0)
    {
    QStringList sliceLogicNames;
    sliceLogicNames << "red" << "yellow" << "green";
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
void qSlicerLayoutManagerPrivate::clearLayout()
{
  QLayoutItem * layoutItem = 0;
  while ((layoutItem = this->GridLayout->takeAt(0)) != 0)
    {
    layoutItem->widget()->setVisible(false);
    this->GridLayout->removeWidget(layoutItem->widget());
    }
}

//------------------------------------------------------------------------------
qMRMLThreeDRenderView* qSlicerLayoutManagerPrivate::threeDRenderView(const QString& name)
{
  Q_ASSERT(this->ThreeDRenderViewMap.contains(name));
  qMRMLThreeDRenderView* renderView = this->ThreeDRenderViewMap[name];
  Q_ASSERT(renderView);
  return renderView;
}

//------------------------------------------------------------------------------
qSlicerSliceViewWidget* qSlicerLayoutManagerPrivate::sliceView(const QString& name)
{
  Q_ASSERT(this->SliceViewMap.contains(name));
  qSlicerSliceViewWidget* sliceView = this->SliceViewMap[name];
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
  d->GridLayout = new QGridLayout(widget);
  d->GridLayout->setContentsMargins(6, 6, 6, 6);
  d->GridLayout->setSpacing(0);
  d->TargetWidget = widget;
}

//------------------------------------------------------------------------------
int qSlicerLayoutManager::threeDRenderViewCount()
{
  CTK_D(qSlicerLayoutManager);
  return d->ThreeDRenderViewMap.count();
}

//------------------------------------------------------------------------------
qMRMLThreeDRenderView* qSlicerLayoutManager::threeDRenderView(int id)
{
  CTK_D(qSlicerLayoutManager);
  
  if (id <= 0 || id > d->ThreeDRenderViewMap.count())
    {
    return 0;
    }
    
  QString name = "RenderView#%1";
  if (d->ThreeDRenderViewMap.contains(name.arg(id)))
    {
    return 0;
    }
  return d->ThreeDRenderViewMap[name.arg(id)];
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
  d->UpdatingMRMLLayoutNode = true;                                              \
  d->MRMLLayoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayout##_NAME); \
  d->UpdatingMRMLLayoutNode = false;

#define qSlicerLayoutManager_returnIfMatchCurrentView(_NAME)                                    \
  if (ctk_d()->MRMLLayoutNode->GetViewArrangement() == vtkMRMLLayoutNode::SlicerLayout##_NAME)  \
    {                                                                                           \
    return;                                                                                     \
    }

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToConventionalView()
{
  CTK_D(qSlicerLayoutManager);
  qSlicerLayoutManager_returnIfMatchCurrentView(ConventionalView);
  d->initialize();
  bool updatesEnabled = d->startUpdateLayout();
  d->clearLayout();

  // First render view
  qMRMLThreeDRenderView * renderView = d->threeDRenderView("RenderView#1");
  d->GridLayout->addWidget(renderView, 0, 0, 1, -1); // fromRow, fromColumn, rowSpan, columnSpan
  renderView->setVisible(true);

  // Red Slice Viewer
  qSlicerSliceViewWidget* redSliceView = d->sliceView("red");
  d->GridLayout->addWidget(redSliceView, 1, 0);
  redSliceView->setVisible(true);

  // Yellow Slice Viewer
  qSlicerSliceViewWidget* yellowSliceView = d->sliceView("yellow");
  d->GridLayout->addWidget(yellowSliceView, 1, 1);
  yellowSliceView->setVisible(true);

  // Green Slice Viewer
  qSlicerSliceViewWidget* greenSliceView = d->sliceView("green");
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
  d->clearLayout();

  // First render view
  qMRMLThreeDRenderView * renderView = d->threeDRenderView("RenderView#1");
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
  this->switchToOneUpSliceView("red");
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToOneUpGreenSliceView()
{
  qSlicerLayoutManager_returnIfMatchCurrentView(OneUpGreenSliceView);
  this->switchToOneUpSliceView("green");
}

//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToOneUpYellowSliceView()
{
  qSlicerLayoutManager_returnIfMatchCurrentView(OneUpYellowSliceView);
  this->switchToOneUpSliceView("yellow");
}
//------------------------------------------------------------------------------
void qSlicerLayoutManager::switchToOneUpSliceView(const QString& sliceViewName)
{
  // Sanity checks
  Q_ASSERT(sliceViewName == "red" || sliceViewName == "yellow" || sliceViewName == "green");
  if (sliceViewName != "red" && sliceViewName != "yellow" && sliceViewName != "green")
    {
    qWarning() << "Slicer viewer name" << sliceViewName << "invalid !";
    return;
    }
  
  CTK_D(qSlicerLayoutManager);
  d->initialize();

  bool updatesEnabled = d->startUpdateLayout();
  d->clearLayout();

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
  d->clearLayout();

  // First render view
  qMRMLThreeDRenderView * renderView = d->threeDRenderView("RenderView#1");
  d->GridLayout->addWidget(renderView, 0, 0); // fromRow, fromColumn, rowSpan, columnSpan
  renderView->setVisible(true);

  // Red Slice Viewer
  qMRMLSliceViewWidget* redSliceView = d->sliceView("red");
  d->GridLayout->addWidget(redSliceView, 0, 1);
  redSliceView->setVisible(true);

  // Yellow Slice Viewer
  qMRMLSliceViewWidget* yellowSliceView = d->sliceView("yellow");
  d->GridLayout->addWidget(yellowSliceView, 1, 0);
  yellowSliceView->setVisible(true);

  // Green Slice Viewer
  qMRMLSliceViewWidget* greenSliceView = d->sliceView("green");
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
  d->clearLayout();

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
  d->clearLayout();

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
  d->clearLayout();

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
  d->clearLayout();

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
  d->clearLayout();

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
  d->clearLayout();

  // First render view
  qMRMLThreeDRenderView * renderView = d->threeDRenderView("RenderView#1");
  d->GridLayout->addWidget(renderView, 0, 0); // fromRow, fromColumn, rowSpan, columnSpan
  renderView->setVisible(true);
  
  // Create a second RenderView if needed
  int viewNodeCount = qMRMLUtils::countVisibleViewNode(d->MRMLScene);
  Q_ASSERT(viewNodeCount >= 0 && viewNodeCount <= 2);
  if (viewNodeCount == 1)
    {
    vtkMRMLNode * node = qMRMLNodeFactory::createNode(d->MRMLScene, "vtkMRMLViewNode");
    Q_ASSERT(node);
    }

  // Second render view
  qMRMLThreeDRenderView * renderView2 = d->threeDRenderView("RenderView#2");
  d->GridLayout->addWidget(renderView2, 0, 1);
  renderView2->setVisible(true);

  // Red Slice Viewer
  qMRMLSliceViewWidget* redSliceView = d->sliceView("red");
  d->GridLayout->addWidget(redSliceView, 0, 1);
  redSliceView->setVisible(true);

  // Yellow Slice Viewer
  qMRMLSliceViewWidget* yellowSliceView = d->sliceView("yellow");
  d->GridLayout->addWidget(yellowSliceView, 1, 0);
  yellowSliceView->setVisible(true);

  // Green Slice Viewer
  qMRMLSliceViewWidget* greenSliceView = d->sliceView("green");
  d->GridLayout->addWidget(greenSliceView, 1, 1);
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
  d->clearLayout();

  // Update LayoutNode
  qSlicerLayoutManager_updateLayoutNode(None);
}

#undef qSlicerLayoutManager_updateLayoutNode

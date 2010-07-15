
// Qt includes
#include <QDebug>

// CTK includes
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLThreeDRenderView.h"
#include "qMRMLThreeDRenderView_p.h"

// MRMLDisplayableManager includes
#include <vtkMRMLDisplayableManagerFactory.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkDisplayableManagerInteractorStyle.h>
#include <vtkMRMLCameraDisplayableManager.h>
#include <vtkMRMLViewDisplayableManager.h>
#include <vtkDisplayableManagerInteractorStyle.h>

// MRML includes
#include <vtkMRMLViewNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkRenderWindowInteractor.h>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLThreeDRenderView");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLThreeDRenderViewPrivate methods

//---------------------------------------------------------------------------
qMRMLThreeDRenderViewPrivate::qMRMLThreeDRenderViewPrivate()
{
  logger.setTrace();

  this->MRMLScene = 0;
  this->MRMLViewNode = 0;
}

//---------------------------------------------------------------------------
qMRMLThreeDRenderViewPrivate::~qMRMLThreeDRenderViewPrivate()
{
  if (this->DisplayableManagerGroup)
    {
    this->DisplayableManagerGroup->Delete();
    }
}

//---------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  if (newScene == this->MRMLScene)
    {
    return;
    }

  this->qvtkReconnect(
    this->MRMLScene, newScene,
    vtkMRMLScene::SceneAboutToBeClosedEvent, this, SLOT(onSceneAboutToBeClosedEvent()));

//  this->qvtkReconnect(
//    this->MRMLScene, newScene,
//    vtkMRMLScene::SceneClosedEvent, this, SLOT(onSceneClosedEvent()));

  this->qvtkReconnect(
    this->MRMLScene, newScene,
    vtkMRMLScene::SceneLoadStartEvent, this, SLOT(onSceneLoadStartEvent()));

  this->qvtkReconnect(
    this->MRMLScene, newScene,
    vtkMRMLScene::SceneLoadEndEvent, this, SLOT(onSceneLoadEndEvent()));
//
//  this->qvtkReconnect(
//    this->MRMLScene, newScene,
//    vtkMRMLScene::NodeAddedEvent, this,
//    SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));
//
//  this->qvtkReconnect(
//    this->MRMLScene, newScene,
//    vtkMRMLScene::NodeRemovedEvent, this,
//    SLOT(onNodeRemovedEvent(vtkObject*, vtkObject*)));
//
//  this->qvtkReconnect(
//    this->MRMLScene, newScene,
//    vtkMRMLScene::SceneRestoredEvent, this, SLOT(onSceneRestoredEvent()));

  this->MRMLScene = newScene;
}

//---------------------------------------------------------------------------
//vtkMRMLCameraNode* qMRMLThreeDRenderViewPrivate::lookUpMRMLCameraNode(vtkMRMLViewNode* viewNode)
//{
//  Q_ASSERT(viewNode);
//
//  QString viewNodeID(viewNode->GetID());
//  Q_ASSERT(!viewNodeID.isEmpty());
//
//  std::vector<vtkMRMLNode*> cameraNodes;
//  int cameraNodeCount = this->MRMLScene->GetNodesByClass("vtkMRMLCameraNode", cameraNodes);
//  for (int n=0; n < cameraNodeCount; n++)
//    {
//    vtkMRMLCameraNode* cameraNode = vtkMRMLCameraNode::SafeDownCast(cameraNodes[n]);
//    Q_ASSERT(cameraNode);
//    QString activeTag(cameraNode->GetActiveTag());
//    if (activeTag == viewNodeID)
//      {
//      Q_ASSERT(this->MRMLScene->GetNodeByID(cameraNode->GetID()));
//      return cameraNode;
//      }
//    }
//  return 0;
//}

// --------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::onSceneAboutToBeClosedEvent()
{
  logger.trace("onSceneAboutToBeClosedEvent");
  CTK_P(qMRMLThreeDRenderView);
  p->setRenderEnabled(false);
}
//
//// --------------------------------------------------------------------------
//void qMRMLThreeDRenderViewPrivate::onSceneClosedEvent()
//{
//  logger.trace("onSceneClosedEvent");
//}
//
// --------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::onSceneLoadStartEvent()
{
  CTK_P(qMRMLThreeDRenderView);
  p->setRenderEnabled(false);
}
//
// --------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::onSceneLoadEndEvent()
{
  CTK_P(qMRMLThreeDRenderView);
  p->setRenderEnabled(true);
  //p->scheduleRender();
}
//
//// --------------------------------------------------------------------------
//void qMRMLThreeDRenderViewPrivate::onSceneRestoredEvent()
//{
//  logger.trace("onSceneRestoredEvent");
//}
  
// --------------------------------------------------------------------------
//void qMRMLThreeDRenderViewPrivate::onNodeAddedEvent(vtkObject* scene, vtkObject* node)
//{
//  Q_ASSERT(scene);
//
//  vtkMRMLNode * mrmlNode = vtkMRMLNode::SafeDownCast(node);
//  Q_ASSERT(mrmlNode);
//  if (!mrmlNode)
//    {
//    return;
//    }
//}

// --------------------------------------------------------------------------
//void qMRMLThreeDRenderViewPrivate::onNodeRemovedEvent(vtkObject* scene, vtkObject* node)
//{
//  Q_ASSERT(scene);
//
//  vtkMRMLNode * mrmlNode = vtkMRMLNode::SafeDownCast(node);
//  Q_ASSERT(mrmlNode);
//  if (!mrmlNode)
//    {
//    return;
//    }
//}

// --------------------------------------------------------------------------
// qMRMLThreeDRenderView methods

// --------------------------------------------------------------------------
qMRMLThreeDRenderView::qMRMLThreeDRenderView(QWidget* _parent) : Superclass(_parent)
{
  CTK_INIT_PRIVATE(qMRMLThreeDRenderView);
  CTK_D(qMRMLThreeDRenderView);
  VTK_CREATE(vtkDisplayableManagerInteractorStyle, interactorStyle);
  this->interactor()->SetInteractorStyle(interactorStyle);

  // Register Displayable Managers
  VTK_CREATE(vtkMRMLDisplayableManagerFactory, factory);
  factory->RegisterDisplayableManager("vtkMRMLCameraDisplayableManager");
  factory->RegisterDisplayableManager("vtkMRMLViewDisplayableManager");

  d->DisplayableManagerGroup = factory->InstantiateDisplayableManagers(this->renderer());
  Q_ASSERT(d->DisplayableManagerGroup);

  // Observe displayable manager group to catch RequestRender events
  d->qvtkConnect(d->DisplayableManagerGroup, vtkCommand::UpdateEvent,
                 this, SLOT(scheduleRender()));

  this->setRenderEnabled(true);
}

//------------------------------------------------------------------------------
void qMRMLThreeDRenderView::setMRMLScene(vtkMRMLScene* newScene)
{
  CTK_D(qMRMLThreeDRenderView);
  if (d->MRMLScene == newScene)
    {
    return;
    }
  d->setMRMLScene(newScene);
  if (d->MRMLViewNode && newScene == d->MRMLViewNode->GetScene())
    {
    return;
    }
  this->setMRMLViewNode(0);
}

//---------------------------------------------------------------------------
void qMRMLThreeDRenderView::setMRMLViewNode(vtkMRMLViewNode* newViewNode)
{
  CTK_D(qMRMLThreeDRenderView);
  if (d->MRMLViewNode == newViewNode)
    {
    return;
    }

  d->DisplayableManagerGroup->SetMRMLViewNode(newViewNode);
  d->MRMLViewNode = newViewNode;
}

//---------------------------------------------------------------------------
CTK_GET_CXX(qMRMLThreeDRenderView, vtkMRMLViewNode*, mrmlViewNode, MRMLViewNode);


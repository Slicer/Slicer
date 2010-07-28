
// Qt includes
#include <QDebug>

// CTK includes
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDView_p.h"

// MRMLDisplayableManager includes
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkThreeDViewInteractorStyle.h>
#include <vtkMRMLCameraDisplayableManager.h>
#include <vtkMRMLViewDisplayableManager.h>

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
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLThreeDView");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLThreeDViewPrivate methods

//---------------------------------------------------------------------------
qMRMLThreeDViewPrivate::qMRMLThreeDViewPrivate()
{
  logger.setTrace();

  this->MRMLScene = 0;
  this->MRMLViewNode = 0;
}

//---------------------------------------------------------------------------
qMRMLThreeDViewPrivate::~qMRMLThreeDViewPrivate()
{
  if (this->DisplayableManagerGroup)
    {
    this->DisplayableManagerGroup->Delete();
    }
}

//---------------------------------------------------------------------------
void qMRMLThreeDViewPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  if (newScene == this->MRMLScene)
    {
    return;
    }

  this->qvtkReconnect(
    this->MRMLScene, newScene,
    vtkMRMLScene::SceneAboutToBeClosedEvent, this, SLOT(onSceneAboutToBeClosedEvent()));

  this->qvtkReconnect(
    this->MRMLScene, newScene,
    vtkMRMLScene::SceneAboutToBeImportedEvent, this, SLOT(onSceneAboutToBeImportedEvent()));

  this->qvtkReconnect(
    this->MRMLScene, newScene,
    vtkMRMLScene::SceneImportedEvent, this, SLOT(onSceneImportedEvent()));
//
//  this->qvtkReconnect(
//    this->MRMLScene, newScene,
//    vtkMRMLScene::SceneRestoredEvent, this, SLOT(onSceneRestoredEvent()));

  this->MRMLScene = newScene;
}

//---------------------------------------------------------------------------
//vtkMRMLCameraNode* qMRMLThreeDViewPrivate::lookUpMRMLCameraNode(vtkMRMLViewNode* viewNode)
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
void qMRMLThreeDViewPrivate::onSceneAboutToBeClosedEvent()
{
  logger.trace("onSceneAboutToBeClosedEvent");
  CTK_P(qMRMLThreeDView);
  p->setRenderEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewPrivate::onSceneAboutToBeImportedEvent()
{
  logger.trace("onSceneAboutToBeImportedEvent");
  CTK_P(qMRMLThreeDView);
  p->setRenderEnabled(false);
}
//
// --------------------------------------------------------------------------
void qMRMLThreeDViewPrivate::onSceneImportedEvent()
{
  logger.trace("onSceneImportedEvent");
  CTK_P(qMRMLThreeDView);
  p->setRenderEnabled(true);
  //p->scheduleRender();
}
//
//// --------------------------------------------------------------------------
//void qMRMLThreeDViewPrivate::onSceneRestoredEvent()
//{
//  logger.trace("onSceneRestoredEvent");
//}

// --------------------------------------------------------------------------
// qMRMLThreeDView methods

// --------------------------------------------------------------------------
qMRMLThreeDView::qMRMLThreeDView(QWidget* _parent) : Superclass(_parent)
{
  CTK_INIT_PRIVATE(qMRMLThreeDView);
  CTK_D(qMRMLThreeDView);
  VTK_CREATE(vtkThreeDViewInteractorStyle, interactorStyle);
  this->interactor()->SetInteractorStyle(interactorStyle);

  // Register Displayable Managers
  vtkMRMLThreeDViewDisplayableManagerFactory* factory = vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance();
  QStringList displayableManagers;
  displayableManagers << "vtkMRMLCameraDisplayableManager"
      << "vtkMRMLViewDisplayableManager"
      << "vtkMRMLModelDisplayableManager";
  foreach(const QString displayableManagerName, displayableManagers)
    {
    if (!factory->IsDisplayableManagerRegistered(displayableManagerName.toLatin1()))
      {
      factory->RegisterDisplayableManager(displayableManagerName.toLatin1());
      }
    }

  d->DisplayableManagerGroup = factory->InstantiateDisplayableManagers(this->renderer());
  Q_ASSERT(d->DisplayableManagerGroup);

  // Observe displayable manager group to catch RequestRender events
  d->qvtkConnect(d->DisplayableManagerGroup, vtkCommand::UpdateEvent,
                 this, SLOT(scheduleRender()));

  // Set default background color
  this->setBackgroundColor(QColor::fromRgbF(0.701960784314, 0.701960784314, 0.905882352941));

  // Hide orientation widget
  this->setOrientationWidgetVisible(false);

  this->setRenderEnabled(true);
}

//------------------------------------------------------------------------------
void qMRMLThreeDView::setMRMLScene(vtkMRMLScene* newScene)
{
  CTK_D(qMRMLThreeDView);
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
void qMRMLThreeDView::setMRMLViewNode(vtkMRMLViewNode* newViewNode)
{
  CTK_D(qMRMLThreeDView);
  if (d->MRMLViewNode == newViewNode)
    {
    return;
    }

  // Enable/disable widget
  this->setDisabled(newViewNode == 0);

  d->DisplayableManagerGroup->SetMRMLViewNode(newViewNode);
  d->MRMLViewNode = newViewNode;
}

//---------------------------------------------------------------------------
CTK_GET_CXX(qMRMLThreeDView, vtkMRMLViewNode*, mrmlViewNode, MRMLViewNode);


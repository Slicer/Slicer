/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QFileInfo>

// CTK includes
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDView_p.h"

// MRMLDisplayableManager includes
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkThreeDViewInteractorStyle.h>

// MRML includes
#include <vtkMRMLViewNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

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
  this->DisplayableManagerGroup = 0;
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
void qMRMLThreeDViewPrivate::onMRMLViewNodeModifiedEvent()
{
  CTK_P(qMRMLThreeDView);
  p->setAnimationIntervalMs(this->MRMLViewNode->GetAnimationMs());
  p->setPitchRollYawIncrement(this->MRMLViewNode->GetRotateDegrees());
  p->setSpinIncrement(this->MRMLViewNode->GetSpinDegrees());
  p->setRockIncrement(this->MRMLViewNode->GetRockCount());
  p->setRockLength(this->MRMLViewNode->GetRockLength());
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewPrivate::onResetFocalPointRequestedEvent()
{
  CTK_P(qMRMLThreeDView);

  // Save current visiblity state of Box and AxisLabel
  bool savedBoxVisibile = this->MRMLViewNode->GetBoxVisible();
  bool savedAxisLabelVisible = this->MRMLViewNode->GetAxisLabelsVisible();

  // Hide Box and AxisLabel
  this->MRMLViewNode->SetBoxVisible(0);
  this->MRMLViewNode->SetAxisLabelsVisible(0);

  p->resetFocalPoint();

  // Restore visibility state
  this->MRMLViewNode->SetBoxVisible(savedBoxVisibile);
  this->MRMLViewNode->SetAxisLabelsVisible(savedAxisLabelVisible);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewPrivate::onAnimationModeEvent()
{
  CTK_P(qMRMLThreeDView);
  if (this->MRMLViewNode->GetAnimationMode() == vtkMRMLViewNode::Spin)
    {
    p->setSpinEnabled(true);
    }
  else if (this->MRMLViewNode->GetAnimationMode() == vtkMRMLViewNode::Rock)
    {
    p->setRockEnabled(true);
    }
  else
    {
    p->setRockEnabled(false);
    p->setSpinEnabled(false);
    }
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewPrivate::rockView()
{
  qDebug() << "rockView";
}

// --------------------------------------------------------------------------
// qMRMLThreeDView methods

// --------------------------------------------------------------------------
qMRMLThreeDView::qMRMLThreeDView(QWidget* _parent) : Superclass(_parent)
{
  CTK_INIT_PRIVATE(qMRMLThreeDView);
  VTK_CREATE(vtkThreeDViewInteractorStyle, interactorStyle);
  this->interactor()->SetInteractorStyle(interactorStyle);

  // Set default background color
  this->setBackgroundColor(QColor::fromRgbF(0.701960784314, 0.701960784314, 0.905882352941));

  // Hide orientation widget
  this->setOrientationWidgetVisible(false);

  this->setZoomFactor(0.05);

  this->setPitchDirection(ctkVTKRenderView::PitchUp);
  this->setRollDirection(ctkVTKRenderView::RollRight);
  this->setYawDirection(ctkVTKRenderView::YawLeft);

  this->setRenderEnabled(true);
}

//------------------------------------------------------------------------------
void qMRMLThreeDView::registerDisplayableManagers(const QString& scriptedDisplayableManagerDirectory)
{
  CTK_D(qMRMLThreeDView);

  QStringList displayableManagers;
  displayableManagers << "vtkMRMLCameraDisplayableManager"
      << "vtkMRMLViewDisplayableManager"
      << "vtkMRMLModelDisplayableManager";

#ifdef Slicer3_USE_PYTHONQT
  QFileInfo dirInfo(scriptedDisplayableManagerDirectory);
  if (dirInfo.isDir())
    {
    displayableManagers << QString("%1/vtkScriptedExampleDisplayableManager.py").arg(
        scriptedDisplayableManagerDirectory);
    }
  else
    {
    logger.error(QString("registerDisplayableManagers - directory %1 doesn't exists !").
                 arg(scriptedDisplayableManagerDirectory));
    }
#endif

  // Register Displayable Managers
  vtkMRMLThreeDViewDisplayableManagerFactory* factory = vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance();
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

  d->DisplayableManagerGroup->SetMRMLDisplayableNode(newViewNode);

  d->qvtkReconnect(
    d->MRMLViewNode, newViewNode,
    vtkCommand::ModifiedEvent, d, SLOT(onMRMLViewNodeModifiedEvent()));

  d->qvtkReconnect(
    d->MRMLViewNode, newViewNode,
    vtkMRMLViewNode::PitchViewRequestedEvent, this, SLOT(pitch()));

  d->qvtkReconnect(
    d->MRMLViewNode, newViewNode,
    vtkMRMLViewNode::RollViewRequestedEvent, this, SLOT(roll()));

  d->qvtkReconnect(
    d->MRMLViewNode, newViewNode,
    vtkMRMLViewNode::YawViewRequestedEvent, this, SLOT(yaw()));

  d->qvtkReconnect(
    d->MRMLViewNode, newViewNode,
    vtkMRMLViewNode::ZoomInRequestedEvent, this, SLOT(zoomIn()));

  d->qvtkReconnect(
    d->MRMLViewNode, newViewNode,
    vtkMRMLViewNode::ZoomOutRequestedEvent, this, SLOT(zoomOut()));

  d->qvtkReconnect(
    d->MRMLViewNode, newViewNode,
    vtkMRMLViewNode::ResetFocalPointRequestedEvent, d, SLOT(onResetFocalPointRequestedEvent()));

  d->qvtkReconnect(
    d->MRMLViewNode, newViewNode,
    vtkMRMLViewNode::AnimationModeEvent, d, SLOT(onAnimationModeEvent()));

  d->MRMLViewNode = newViewNode;

  if (d->MRMLViewNode)
    {
    d->onMRMLViewNodeModifiedEvent();
    d->onAnimationModeEvent();
    }
}

//---------------------------------------------------------------------------
CTK_GET_CXX(qMRMLThreeDView, vtkMRMLViewNode*, mrmlViewNode, MRMLViewNode);


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
#include <ctkAxesWidget.h>

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
qMRMLThreeDViewPrivate::qMRMLThreeDViewPrivate(qMRMLThreeDView& object)
  : q_ptr(&object)
{
  this->DisplayableManagerGroup = 0;
  this->MRMLScene = 0;
  this->MRMLViewNode = 0;
  this->IgnoreScriptedDisplayableManagers = false;
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
  Q_Q(qMRMLThreeDView);
  q->setRenderEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewPrivate::onSceneAboutToBeImportedEvent()
{
  logger.trace("onSceneAboutToBeImportedEvent");
  Q_Q(qMRMLThreeDView);
  q->setRenderEnabled(false);
}
//
// --------------------------------------------------------------------------
void qMRMLThreeDViewPrivate::onSceneImportedEvent()
{
  logger.trace("onSceneImportedEvent");
  Q_Q(qMRMLThreeDView);
  q->setRenderEnabled(true);
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
  Q_Q(qMRMLThreeDView);
  q->setAnimationIntervalMs(this->MRMLViewNode->GetAnimationMs());
  q->setPitchRollYawIncrement(this->MRMLViewNode->GetRotateDegrees());
  q->setSpinIncrement(this->MRMLViewNode->GetSpinDegrees());
  q->setRockIncrement(this->MRMLViewNode->GetRockCount());
  q->setRockLength(this->MRMLViewNode->GetRockLength());
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewPrivate::onResetFocalPointRequestedEvent()
{
  Q_Q(qMRMLThreeDView);

  // Save current visiblity state of Box and AxisLabel
  bool savedBoxVisibile = this->MRMLViewNode->GetBoxVisible();
  bool savedAxisLabelVisible = this->MRMLViewNode->GetAxisLabelsVisible();

  // Hide Box and AxisLabel
  this->MRMLViewNode->SetBoxVisible(0);
  this->MRMLViewNode->SetAxisLabelsVisible(0);

  q->resetFocalPoint();

  // Restore visibility state
  this->MRMLViewNode->SetBoxVisible(savedBoxVisibile);
  this->MRMLViewNode->SetAxisLabelsVisible(savedAxisLabelVisible);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewPrivate::onAnimationModeEvent()
{
  Q_Q(qMRMLThreeDView);
  if (this->MRMLViewNode->GetAnimationMode() == vtkMRMLViewNode::Spin)
    {
    q->setSpinEnabled(true);
    }
  else if (this->MRMLViewNode->GetAnimationMode() == vtkMRMLViewNode::Rock)
    {
    q->setRockEnabled(true);
    }
  else
    {
    q->setRockEnabled(false);
    q->setSpinEnabled(false);
    }
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewPrivate::rockView()
{
  qDebug() << "rockView";
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewPrivate::onLookFromAxisEvent(vtkObject* node, void* axis)
{
  Q_Q(qMRMLThreeDView);
  Q_UNUSED(node);
  Q_ASSERT(this->MRMLViewNode == node);
  ctkAxesWidget::Axis lookFrom = *reinterpret_cast<ctkAxesWidget::Axis*>(axis);
  Q_ASSERT(lookFrom == ctkAxesWidget::None ||
           lookFrom == ctkAxesWidget::Right ||
           lookFrom == ctkAxesWidget::Left ||
           lookFrom == ctkAxesWidget::Anterior ||
           lookFrom == ctkAxesWidget::Posterior ||
           lookFrom == ctkAxesWidget::Superior ||
           lookFrom == ctkAxesWidget::Inferior);
  double fov = this->MRMLViewNode->GetFieldOfView();
  Q_ASSERT(fov >= 0.0);
  q->lookFromAxis(lookFrom, fov);
}

// --------------------------------------------------------------------------
// qMRMLThreeDView methods

// --------------------------------------------------------------------------
qMRMLThreeDView::qMRMLThreeDView(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLThreeDViewPrivate(*this))
{
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

  // true by default this->setRenderEnabled(true);
}

// --------------------------------------------------------------------------
qMRMLThreeDView::~qMRMLThreeDView()
{
}

//------------------------------------------------------------------------------
void qMRMLThreeDView::registerDisplayableManagers(const QString& scriptedDisplayableManagerDirectory)
{
  Q_D(qMRMLThreeDView);

  QStringList displayableManagers;
  displayableManagers << "vtkMRMLCameraDisplayableManager"
      << "vtkMRMLViewDisplayableManager"
      << "vtkMRMLModelDisplayableManager";

#ifdef Slicer_USE_PYTHONQT
  if (!d->IgnoreScriptedDisplayableManagers)
    {
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
    }
#else
  Q_UNUSED(scriptedDisplayableManagerDirectory);
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
CTK_SET_CPP(qMRMLThreeDView, bool, setIgnoreScriptedDisplayableManagers, IgnoreScriptedDisplayableManagers);

//------------------------------------------------------------------------------
void qMRMLThreeDView::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLThreeDView);
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
  Q_D(qMRMLThreeDView);
  if (d->MRMLViewNode == newViewNode)
    {
    return;
    }

  // Enable/disable widget
  this->setDisabled(newViewNode == 0);

  if (d->DisplayableManagerGroup)
    {
    d->DisplayableManagerGroup->SetMRMLDisplayableNode(newViewNode);
    }

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

  d->qvtkReconnect(
    d->MRMLViewNode, newViewNode,
    vtkMRMLViewNode::LookFromAxisRequestedEvent, d, SLOT(onLookFromAxisEvent(vtkObject*, void*)));

  d->MRMLViewNode = newViewNode;

  if (d->MRMLViewNode)
    {
    d->onMRMLViewNodeModifiedEvent();
    d->onAnimationModeEvent();
    }
}

//---------------------------------------------------------------------------
vtkMRMLViewNode* qMRMLThreeDView::mrmlViewNode()const
{
  Q_D(const qMRMLThreeDView);
  return d->MRMLViewNode;
}


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
#include <QDropEvent>
#include <QEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMimeData>
#include <QToolButton>

// CTK includes
#include <ctkAxesWidget.h>

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLThreeDView_p.h"
#include "qMRMLUtils.h"

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractDisplayableManager.h>
#include <vtkMRMLCameraDisplayableManager.h>
#include <vtkMRMLCrosshairDisplayableManager.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include <vtkMRMLThreeDViewInteractorStyle.h>

// MRML includes
#include <vtkMRMLViewNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLCrosshairNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkInteractorStyle3D.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>

//--------------------------------------------------------------------------
// qMRMLThreeDViewPrivate methods

//---------------------------------------------------------------------------
qMRMLThreeDViewPrivate::qMRMLThreeDViewPrivate(qMRMLThreeDView& object)
  : ctkVTKRenderViewPrivate(object)
{
  this->DisplayableManagerGroup = nullptr;
  this->InteractorObserver = vtkMRMLThreeDViewInteractorStyle::New();
  this->MRMLScene = nullptr;
  this->MRMLViewNode = nullptr;

  // The depth format must be Fixed32 for the volume mapper to successfully copy the depth texture
  this->ShadowsRenderPass->SetDepthFormat(vtkTextureObject::Fixed32);
  this->ShadowsRenderPass->SetDelegatePass(BasicRenderPass);
}

//---------------------------------------------------------------------------
qMRMLThreeDViewPrivate::~qMRMLThreeDViewPrivate()
{
  if (this->DisplayableManagerGroup)
  {
    this->DisplayableManagerGroup->Delete();
  }
  if (this->InteractorObserver)
  {
    this->InteractorObserver->Delete();
  }
}

//---------------------------------------------------------------------------
void qMRMLThreeDViewPrivate::init()
{
  Q_Q(qMRMLThreeDView);

  this->ctkVTKRenderViewPrivate::init();

  q->setRenderEnabled(this->MRMLScene != nullptr);

  vtkNew<vtkInteractorStyle3D> interactorStyle;
  q->interactor()->SetInteractorStyle(interactorStyle.GetPointer());

  // Set default background color
  q->setBackgroundColor(QColor::fromRgbF(vtkMRMLViewNode::defaultBackgroundColor()[0],
                                         vtkMRMLViewNode::defaultBackgroundColor()[1],
                                         vtkMRMLViewNode::defaultBackgroundColor()[2]));

  q->setGradientBackground(true);

  // Hide orientation widget
  q->setOrientationWidgetVisible(false);

  q->setZoomFactor(0.05);

  q->setPitchDirection(ctkVTKRenderView::PitchUp);
  q->setRollDirection(ctkVTKRenderView::RollRight);
  q->setYawDirection(ctkVTKRenderView::YawLeft);

  this->initDisplayableManagers();
}

//---------------------------------------------------------------------------
void qMRMLThreeDViewPrivate::initDisplayableManagers()
{
  Q_Q(qMRMLThreeDView);
  vtkMRMLThreeDViewDisplayableManagerFactory* factory = vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance();

  QStringList displayableManagers;
  displayableManagers << "vtkMRMLCameraDisplayableManager"
                      << "vtkMRMLViewDisplayableManager"
                      << "vtkMRMLModelDisplayableManager"
                      << "vtkMRMLThreeDReformatDisplayableManager"
                      << "vtkMRMLCrosshairDisplayableManager3D"
                      << "vtkMRMLOrientationMarkerDisplayableManager"
                      << "vtkMRMLRulerDisplayableManager";
  foreach (const QString& displayableManager, displayableManagers)
  {
    if (!factory->IsDisplayableManagerRegistered(displayableManager.toUtf8()))
    {
      factory->RegisterDisplayableManager(displayableManager.toUtf8());
    }
  }

  this->DisplayableManagerGroup = factory->InstantiateDisplayableManagers(q->renderer());
  this->InteractorObserver->SetDisplayableManagers(this->DisplayableManagerGroup);
  // Observe displayable manager group to catch RequestRender events
  this->qvtkConnect(this->DisplayableManagerGroup, vtkCommand::UpdateEvent, q, SLOT(scheduleRender()));
}

//---------------------------------------------------------------------------
void qMRMLThreeDViewPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_Q(qMRMLThreeDView);
  if (newScene == this->MRMLScene)
  {
    return;
  }

  this->qvtkReconnect(
    this->MRMLScene, newScene, vtkMRMLScene::StartBatchProcessEvent, this, SLOT(onSceneStartProcessing()));

  this->qvtkReconnect(
    this->MRMLScene, newScene, vtkMRMLScene::EndBatchProcessEvent, this, SLOT(onSceneEndProcessing()));

  this->MRMLScene = newScene;
  q->setRenderEnabled(this->MRMLScene != nullptr && !this->MRMLScene->IsBatchProcessing());
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewPrivate::onSceneStartProcessing()
{
  // qDebug() << "qMRMLThreeDViewPrivate::onSceneStartProcessing";
  Q_Q(qMRMLThreeDView);
  q->setRenderEnabled(false);
}

//
// --------------------------------------------------------------------------
void qMRMLThreeDViewPrivate::onSceneEndProcessing()
{
  // qDebug() << "qMRMLThreeDViewPrivate::onSceneImportedEvent";
  Q_Q(qMRMLThreeDView);
  q->setRenderEnabled(true);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewPrivate::updateWidgetFromMRML()
{
  Q_Q(qMRMLThreeDView);
  if (!this->MRMLViewNode)
  {
    return;
  }
  q->setAnimationIntervalMs(this->MRMLViewNode->GetAnimationMs());
  q->setPitchRollYawIncrement(this->MRMLViewNode->GetRotateDegrees());
  q->setSpinIncrement(this->MRMLViewNode->GetSpinDegrees());
  q->setRockIncrement(this->MRMLViewNode->GetRockCount());
  q->setRockLength(this->MRMLViewNode->GetRockLength());

  q->setSpinEnabled(this->MRMLViewNode->GetAnimationMode() == vtkMRMLViewNode::Spin);
  q->setRockEnabled(this->MRMLViewNode->GetAnimationMode() == vtkMRMLViewNode::Rock);

  q->setUseDepthPeeling(this->MRMLViewNode->GetUseDepthPeeling() != 0);
  q->setFPSVisible(this->MRMLViewNode->GetFPSVisible() != 0);

  q->setShadowsVisibility(this->MRMLViewNode->GetShadowsVisibility());
  q->setAmbientShadowsSizeScale(this->MRMLViewNode->GetAmbientShadowsSizeScale());
  q->setAmbientShadowsVolumeOpacityThreshold(this->MRMLViewNode->GetAmbientShadowsVolumeOpacityThreshold());
}

// --------------------------------------------------------------------------
// qMRMLThreeDView methods

// --------------------------------------------------------------------------
namespace
{
void ClickCallbackFunction(vtkObject* vtkNotUsed(caller),
                           long unsigned int eventId,
                           void* clientData,
                           void* vtkNotUsed(callData))
{
  qMRMLThreeDView* threeDView = reinterpret_cast<qMRMLThreeDView*>(clientData);
  vtkMRMLCameraNode* cam = threeDView->cameraNode();
  if (!cam)
  {
    qCritical() << "qMRMLThreeDView::mouseMoveEvent: can not retrieve camera node.";
    return;
  }

  switch (eventId)
  {
    case vtkCommand::MouseWheelForwardEvent:
    {
      cam->InvokeCustomModifiedEvent(vtkMRMLCameraNode::CameraInteractionEvent);
    }
    break;
    case vtkCommand::MouseWheelBackwardEvent:
    {
      cam->InvokeCustomModifiedEvent(vtkMRMLCameraNode::CameraInteractionEvent);
    }
    break;
    case vtkCommand::InteractionEvent:
    {
      cam->InvokeCustomModifiedEvent(vtkMRMLCameraNode::CameraInteractionEvent);
    }
    break;
    case vtkCommand::KeyPressEvent:
    {
      cam->InvokeCustomModifiedEvent(vtkMRMLCameraNode::CameraInteractionEvent);
    }
    break;
  }
}
} // namespace

// --------------------------------------------------------------------------
qMRMLThreeDView::qMRMLThreeDView(QWidget* _parent)
  : Superclass(new qMRMLThreeDViewPrivate(*this), _parent)
{
  Q_D(qMRMLThreeDView);
  d->init();
  setAcceptDrops(true);

  vtkRenderWindowInteractor* renderWindowInteractor = this->interactor();

  vtkSmartPointer<vtkCallbackCommand> clickCallback = vtkSmartPointer<vtkCallbackCommand>::New();
  clickCallback->SetClientData(this);
  clickCallback->SetCallback(ClickCallbackFunction);

  renderWindowInteractor->AddObserver(vtkCommand::MouseWheelForwardEvent, clickCallback);
  renderWindowInteractor->AddObserver(vtkCommand::MouseWheelBackwardEvent, clickCallback);
  renderWindowInteractor->AddObserver(vtkCommand::InteractionEvent, clickCallback);
  renderWindowInteractor->AddObserver(vtkCommand::KeyPressEvent, clickCallback);
}

// --------------------------------------------------------------------------
qMRMLThreeDView::~qMRMLThreeDView() = default;

//------------------------------------------------------------------------------
void qMRMLThreeDView::setInteractor(vtkRenderWindowInteractor* interactor)
{
  Q_D(qMRMLThreeDView);
  this->Superclass::setInteractor(interactor);
  d->InteractorObserver->SetInteractor(interactor);
}

//------------------------------------------------------------------------------
vtkMRMLThreeDViewInteractorStyle* qMRMLThreeDView::interactorObserver() const
{
  Q_D(const qMRMLThreeDView);
  return d->InteractorObserver;
}

//------------------------------------------------------------------------------
void qMRMLThreeDView::addDisplayableManager(const QString& displayableManagerName)
{
  Q_D(qMRMLThreeDView);
  vtkSmartPointer<vtkMRMLAbstractDisplayableManager> displayableManager;
  displayableManager.TakeReference(
    vtkMRMLDisplayableManagerGroup::InstantiateDisplayableManager(displayableManagerName.toUtf8()));
  d->DisplayableManagerGroup->AddDisplayableManager(displayableManager);
}

//------------------------------------------------------------------------------
vtkMRMLCameraNode* qMRMLThreeDView::cameraNode()
{
  vtkMRMLCameraDisplayableManager* cameraDM = vtkMRMLCameraDisplayableManager::SafeDownCast(
    this->displayableManagerByClassName("vtkMRMLCameraDisplayableManager"));
  if (!cameraDM)
  {
    return nullptr;
  }

  vtkMRMLCameraNode* cam = cameraDM->GetCameraNode();
  return cam;
}

//------------------------------------------------------------------------------
void qMRMLThreeDView::rotateToViewAxis(unsigned int axisId)
{
  vtkMRMLCameraNode* cam = this->cameraNode();
  if (!cam)
  {
    qCritical() << "qMRMLThreeDView::rotateToViewAxis: can not retrieve camera node.";
    return;
  }

  switch (axisId)
  {
    case 0:
      cam->RotateTo(vtkMRMLCameraNode::Left);
      break;
    case 1:
      cam->RotateTo(vtkMRMLCameraNode::Right);
      break;
    case 2:
      cam->RotateTo(vtkMRMLCameraNode::Posterior);
      break;
    case 3:
      cam->RotateTo(vtkMRMLCameraNode::Anterior);
      break;
    case 4:
      cam->RotateTo(vtkMRMLCameraNode::Inferior);
      break;
    case 5:
      cam->RotateTo(vtkMRMLCameraNode::Superior);
      break;
    default:
      qWarning() << "qMRMLThreeDView::rotateToViewAxis: " << axisId << " is not a valid axis id (0 to 5 : "
                 << "-X, +X, -Y, +Y, -Z, +Z).";
      break;
  }
}

//------------------------------------------------------------------------------
void qMRMLThreeDView::rotateToViewAxis(const std::string& axisLabel)
{
  Q_D(qMRMLThreeDView);
  if (!d->MRMLViewNode)
  {
    qCritical() << "qMRMLThreeDView::rotateToViewAxis: no valid view node.";
    return;
  }

  for (int i = 0; i < vtkMRMLAbstractViewNode::AxisLabelsCount; ++i)
  {
    if (axisLabel == std::string(d->MRMLViewNode->GetAxisLabel(i)))
    {
      this->rotateToViewAxis(i);
      return;
    }
  }
  qWarning() << "qMRMLThreeDView::rotateToViewAxis: " << QString(axisLabel.c_str()) << "is not a valid axis label.";
}

//------------------------------------------------------------------------------
void qMRMLThreeDView ::resetCamera(bool resetRotation, bool resetTranslation, bool resetDistance)
{
  vtkMRMLCameraNode* cam = this->cameraNode();
  if (!cam)
  {
    qCritical() << "qMRMLThreeDView::resetCamera: can not retrieve camera node.";
    return;
  }

  cam->Reset(resetRotation, resetTranslation, resetDistance, this->renderer());
}

//------------------------------------------------------------------------------
void qMRMLThreeDView::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLThreeDView);
  d->setMRMLScene(newScene);

  if (d->MRMLViewNode && newScene != d->MRMLViewNode->GetScene())
  {
    this->setMRMLViewNode(nullptr);
  }
}

//---------------------------------------------------------------------------
void qMRMLThreeDView::setMRMLViewNode(vtkMRMLViewNode* newViewNode)
{
  Q_D(qMRMLThreeDView);
  if (d->MRMLViewNode == newViewNode)
  {
    return;
  }

  d->qvtkReconnect(d->MRMLViewNode, newViewNode, vtkCommand::ModifiedEvent, d, SLOT(updateWidgetFromMRML()));

  d->MRMLViewNode = newViewNode;
  d->DisplayableManagerGroup->SetMRMLDisplayableNode(newViewNode);

  d->updateWidgetFromMRML();
  // Enable/disable widget
  this->setEnabled(newViewNode != nullptr);
}

//---------------------------------------------------------------------------
vtkMRMLViewNode* qMRMLThreeDView::mrmlViewNode() const
{
  Q_D(const qMRMLThreeDView);
  return d->MRMLViewNode;
}

// --------------------------------------------------------------------------
void qMRMLThreeDView::resetFocalPoint()
{
  Q_D(qMRMLThreeDView);

  bool savedBoxVisibile = true;
  bool savedAxisLabelVisible = true;
  if (d->MRMLViewNode)
  {
    // Save current visibility state of Box and AxisLabel
    savedBoxVisibile = d->MRMLViewNode->GetBoxVisible();
    savedAxisLabelVisible = d->MRMLViewNode->GetAxisLabelsVisible();

    int wasModifying = d->MRMLViewNode->StartModify();
    // Hide Box and AxisLabel so they don't get taken into account when computing
    // the view boundaries
    d->MRMLViewNode->SetBoxVisible(0);
    d->MRMLViewNode->SetAxisLabelsVisible(0);
    d->MRMLViewNode->EndModify(wasModifying);
  }

  // Exclude crosshair from focal point computation
  vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairDisplayableManager::FindCrosshairNode(d->MRMLScene);
  int crosshairMode = 0;
  if (crosshairNode)
  {
    crosshairMode = crosshairNode->GetCrosshairMode();
    crosshairNode->SetCrosshairMode(vtkMRMLCrosshairNode::NoCrosshair);
  }

  // Superclass resets the camera.
  this->Superclass::resetFocalPoint();

  if (d->MRMLViewNode)
  {
    // Restore visibility state
    int wasModifying = d->MRMLViewNode->StartModify();
    d->MRMLViewNode->SetBoxVisible(savedBoxVisibile);
    d->MRMLViewNode->SetAxisLabelsVisible(savedAxisLabelVisible);
    d->MRMLViewNode->EndModify(wasModifying);
    // Inform the displayable manager that the view is reset, so it can
    // update the box/labels bounds.
    d->MRMLViewNode->InvokeEvent(vtkMRMLViewNode::ResetFocalPointRequestedEvent);
  }

  if (crosshairNode)
  {
    crosshairNode->SetCrosshairMode(crosshairMode);
  }

  if (this->renderer())
  {
    this->renderer()->ResetCameraClippingRange();
  }
}

//------------------------------------------------------------------------------
void qMRMLThreeDView::getDisplayableManagers(vtkCollection* displayableManagers)
{
  Q_D(qMRMLThreeDView);

  if (!displayableManagers)
  {
    return;
  }
  int num = d->DisplayableManagerGroup->GetDisplayableManagerCount();
  for (int n = 0; n < num; n++)
  {
    displayableManagers->AddItem(d->DisplayableManagerGroup->GetNthDisplayableManager(n));
  }
}

//------------------------------------------------------------------------------
vtkMRMLAbstractDisplayableManager* qMRMLThreeDView::displayableManagerByClassName(const char* className)
{
  Q_D(qMRMLThreeDView);
  return d->DisplayableManagerGroup->GetDisplayableManagerByClassName(className);
}

// --------------------------------------------------------------------------
void qMRMLThreeDView::setViewCursor(const QCursor& cursor)
{
  this->setCursor(cursor);
  if (this->VTKWidget() != nullptr)
  {
    this->VTKWidget()->setCursor(cursor); // TODO: test if cursor settings works
  }
}

// --------------------------------------------------------------------------
void qMRMLThreeDView::unsetViewCursor()
{
  this->unsetCursor();
  if (this->VTKWidget() != nullptr)
  {
    // TODO: it would be better to restore default cursor, but QVTKOpenGLNativeWidget
    // API does not have an accessor method to the default cursor.
    this->VTKWidget()->setCursor(QCursor(Qt::ArrowCursor)); // TODO: test if cursor settings works
  }
}

// --------------------------------------------------------------------------
void qMRMLThreeDView::setDefaultViewCursor(const QCursor& cursor)
{
  if (this->VTKWidget() != nullptr)
  {
    this->VTKWidget()->setDefaultCursor(cursor); // TODO: test if cursor settings works
  }
}

//---------------------------------------------------------------------------
void qMRMLThreeDView::dragEnterEvent(QDragEnterEvent* event)
{
  Q_D(qMRMLThreeDView);
  vtkNew<vtkIdList> shItemIdList;
  qMRMLUtils::mimeDataToSubjectHierarchyItemIDs(event->mimeData(), shItemIdList);
  if (shItemIdList->GetNumberOfIds() > 0)
  {
    event->accept();
    return;
  }
  Superclass::dragEnterEvent(event);
}

//-----------------------------------------------------------------------------
void qMRMLThreeDView::dropEvent(QDropEvent* event)
{
  Q_D(qMRMLThreeDView);
  vtkNew<vtkIdList> shItemIdList;
  qMRMLUtils::mimeDataToSubjectHierarchyItemIDs(event->mimeData(), shItemIdList);
  if (!shItemIdList->GetNumberOfIds())
  {
    return;
  }
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(d->MRMLScene);
  if (!shNode)
  {
    qWarning() << Q_FUNC_INFO << " failed: invalid subject hierarchy node";
    return;
  }
  shNode->ShowItemsInView(shItemIdList, this->mrmlViewNode());
}

//------------------------------------------------------------------------------
bool qMRMLThreeDView::shadowsVisibility() const
{
  Q_D(const qMRMLThreeDView);
  vtkRenderer* renderer = this->renderer();
  if (!renderer)
  {
    return false;
  }
  return (renderer->GetPass() == d->ShadowsRenderPass);
}

//------------------------------------------------------------------------------
void qMRMLThreeDView::setShadowsVisibility(bool visibility)
{
  Q_D(const qMRMLThreeDView);
  vtkRenderer* renderer = this->renderer();
  if (!renderer)
  {
    return;
  }
  if (visibility)
  {
    renderer->SetPass(d->ShadowsRenderPass);
  }
  else
  {
    renderer->SetPass(nullptr);
  }
}

//------------------------------------------------------------------------------
double qMRMLThreeDView::ambientShadowsSizeScale() const
{
  Q_D(const qMRMLThreeDView);
  // Compute sizeScale from bias by inverting computation implemented in setAmbientShadowsSizeScale.
  double bias = d->ShadowsRenderPass->GetBias();
  double sceneSize = bias / 0.001;
  double sizeScale = log(sceneSize / 100.0);
  return sizeScale;
}

//------------------------------------------------------------------------------
void qMRMLThreeDView::setAmbientShadowsSizeScale(double sizeScale)
{
  Q_D(const qMRMLThreeDView);
  // SizeScale = 0.0 corresponds to 100mm scene size
  double sceneSize = 100.0 * pow(10, sizeScale);
  // Bias and radius are from example in https://blog.kitware.com/ssao/
  // These values have been tested on different kind of meshes and volumes and found to work well.
  d->ShadowsRenderPass->SetBias(0.001 * sceneSize); // how much distance difference will be made visible
  d->ShadowsRenderPass->SetRadius(0.1 * sceneSize); // determines the spread of shadows cast by ambient occlusion
  d->ShadowsRenderPass->SetBlur(true);              // reduce noise
  d->ShadowsRenderPass->SetKernelSize(320);         // larger kernel size reduces noise pattern in the darkened region
}

//------------------------------------------------------------------------------
double qMRMLThreeDView::ambientShadowsVolumeOpacityThreshold() const
{
  Q_D(const qMRMLThreeDView);
  return d->ShadowsRenderPass->GetVolumeOpacityThreshold();
}

//------------------------------------------------------------------------------
void qMRMLThreeDView::setAmbientShadowsVolumeOpacityThreshold(double opacityThreshold)
{
  Q_D(const qMRMLThreeDView);

  d->ShadowsRenderPass->SetVolumeOpacityThreshold(opacityThreshold);
}

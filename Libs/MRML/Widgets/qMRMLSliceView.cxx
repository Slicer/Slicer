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

  This file was originally developed by Julien Finet, Kitware Inc.
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
#include <QUrl>

// CTK includes
#include <ctkAxesWidget.h>
#include <ctkPopupWidget.h>

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLSliceView_p.h"
#include "qMRMLUtils.h"

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractDisplayableManager.h>
#include <vtkMRMLCrosshairDisplayableManager.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
#include <vtkMRMLScalarBarDisplayableManager.h>
#include <vtkMRMLSliceViewInteractorStyle.h>

// MRML includes
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleUser.h>
#include <vtkSmartPointer.h>

//--------------------------------------------------------------------------
// qMRMLSliceViewPrivate methods

//---------------------------------------------------------------------------
qMRMLSliceViewPrivate::qMRMLSliceViewPrivate(qMRMLSliceView& object)
  : ctkVTKSliceViewPrivate(object)
{
  this->DisplayableManagerGroup = nullptr;
  this->InteractorObserver = vtkMRMLSliceViewInteractorStyle::New();
  this->MRMLScene = nullptr;
  this->MRMLSliceNode = nullptr;
  this->InactiveBoxColor = QColor(95, 95, 113);
}

//---------------------------------------------------------------------------
qMRMLSliceViewPrivate::~qMRMLSliceViewPrivate()
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
void qMRMLSliceViewPrivate::init()
{
  Q_Q(qMRMLSliceView);

  this->ctkVTKSliceViewPrivate::init();

  q->setRenderEnabled(this->MRMLScene != nullptr);

  vtkNew<vtkInteractorStyleUser> interactorStyle;

  q->interactor()->SetInteractorStyle(interactorStyle.GetPointer());

  this->initDisplayableManagers();

  // Force an initial render to ensure that the render window creates an OpenGL
  // context. If operations that require a context--such as hardware
  // picking--are performed when no context exists, OpenGL errors will occur.
  // When using the VTK OpenGL2 backend the errors may be followed by a
  // segfault. Such a scenario can occur when the app is started using a layout
  // that hides one or more of the slice views.
  q->renderWindow()->Render();
}

//---------------------------------------------------------------------------
void qMRMLSliceViewPrivate::initDisplayableManagers()
{
  Q_Q(qMRMLSliceView);

  vtkMRMLSliceViewDisplayableManagerFactory* factory = vtkMRMLSliceViewDisplayableManagerFactory::GetInstance();

  QStringList displayableManagers;
  displayableManagers << "vtkMRMLVolumeGlyphSliceDisplayableManager";
  displayableManagers << "vtkMRMLModelSliceDisplayableManager";
  displayableManagers << "vtkMRMLCrosshairDisplayableManager";
  displayableManagers << "vtkMRMLOrientationMarkerDisplayableManager";
  displayableManagers << "vtkMRMLRulerDisplayableManager";
  displayableManagers << "vtkMRMLScalarBarDisplayableManager";
  for (const QString& displayableManager : displayableManagers)
  {
    if (!factory->IsDisplayableManagerRegistered(displayableManager.toUtf8()))
    {
      factory->RegisterDisplayableManager(displayableManager.toUtf8());
    }
  }

  this->DisplayableManagerGroup = factory->InstantiateDisplayableManagers(q->lightBoxRendererManager()->GetRenderer(0));

  this->InteractorObserver->SetDisplayableManagers(this->DisplayableManagerGroup);
  // Observe displayable manager group to catch RequestRender events
  q->qvtkConnect(this->DisplayableManagerGroup, vtkCommand::UpdateEvent, q, SLOT(scheduleRender()));
}

//---------------------------------------------------------------------------
void qMRMLSliceViewPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_Q(qMRMLSliceView);
  if (newScene == this->MRMLScene)
  {
    return;
  }

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::StartBatchProcessEvent, this, SLOT(onSceneStartProcessing()));

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::EndBatchProcessEvent, this, SLOT(onSceneEndProcessing()));

  this->MRMLScene = newScene;
  q->setRenderEnabled(this->MRMLScene != nullptr && !this->MRMLScene->IsBatchProcessing());
}

// --------------------------------------------------------------------------
void qMRMLSliceViewPrivate::onSceneStartProcessing()
{
  Q_Q(qMRMLSliceView);
  q->setRenderEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLSliceViewPrivate::onSceneEndProcessing()
{
  Q_Q(qMRMLSliceView);
  q->setRenderEnabled(true);
}

// --------------------------------------------------------------------------
// qMRMLSliceView methods

// --------------------------------------------------------------------------
qMRMLSliceView::qMRMLSliceView(QWidget* _parent)
  : Superclass(new qMRMLSliceViewPrivate(*this), _parent)
{
  Q_D(qMRMLSliceView);
  d->init();
  setAcceptDrops(true);
}

// --------------------------------------------------------------------------
qMRMLSliceView::~qMRMLSliceView() = default;

//------------------------------------------------------------------------------
void qMRMLSliceView::setInteractor(vtkRenderWindowInteractor* interactor)
{
  Q_D(qMRMLSliceView);
  this->Superclass::setInteractor(interactor);
  d->InteractorObserver->SetInteractor(interactor);
}

//------------------------------------------------------------------------------
vtkMRMLSliceViewInteractorStyle* qMRMLSliceView::interactorObserver() const
{
  Q_D(const qMRMLSliceView);
  return d->InteractorObserver;
}

//------------------------------------------------------------------------------
void qMRMLSliceView::addDisplayableManager(const QString& displayableManagerName)
{
  Q_D(qMRMLSliceView);
  vtkSmartPointer<vtkMRMLAbstractDisplayableManager> displayableManager;
  displayableManager.TakeReference(vtkMRMLDisplayableManagerGroup::InstantiateDisplayableManager(displayableManagerName.toUtf8()));
  d->DisplayableManagerGroup->AddDisplayableManager(displayableManager);
}

//------------------------------------------------------------------------------
void qMRMLSliceView::getDisplayableManagers(vtkCollection* displayableManagers)
{
  Q_D(qMRMLSliceView);

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
vtkMRMLAbstractDisplayableManager* qMRMLSliceView::displayableManagerByClassName(const char* className)
{
  Q_D(qMRMLSliceView);
  return d->DisplayableManagerGroup->GetDisplayableManagerByClassName(className);
}

//------------------------------------------------------------------------------
void qMRMLSliceView::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLSliceView);
  d->setMRMLScene(newScene);

  if (d->MRMLSliceNode && newScene != d->MRMLSliceNode->GetScene())
  {
    this->setMRMLSliceNode(nullptr);
  }
}

//---------------------------------------------------------------------------
void qMRMLSliceView::setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode)
{
  Q_D(qMRMLSliceView);
  if (d->MRMLSliceNode == newSliceNode)
  {
    return;
  }

  d->MRMLSliceNode = newSliceNode;

  d->DisplayableManagerGroup->SetMRMLDisplayableNode(newSliceNode);

  // Enable/disable widget
  this->setEnabled(newSliceNode != nullptr);
}

//---------------------------------------------------------------------------
vtkMRMLSliceNode* qMRMLSliceView::mrmlSliceNode() const
{
  Q_D(const qMRMLSliceView);
  return d->MRMLSliceNode;
}

//---------------------------------------------------------------------------
vtkMRMLSliceViewInteractorStyle* qMRMLSliceView::sliceViewInteractorStyle() const
{
  qWarning("qMRMLSliceView::sliceViewInteractorStyle is deprecated. Use interactorObserver instead.");
  return this->interactorObserver();
}

// --------------------------------------------------------------------------
QList<double> qMRMLSliceView::convertDeviceToXYZ(const QList<int>& xy) const
{
  Q_D(const qMRMLSliceView);

  // Grab a displayable manager that is derived from
  // AbstractSliceViewDisplayableManager, like the CrosshairDisplayableManager
  vtkMRMLCrosshairDisplayableManager* cmgr =
    vtkMRMLCrosshairDisplayableManager::SafeDownCast(d->DisplayableManagerGroup->GetDisplayableManagerByClassName("vtkMRMLCrosshairDisplayableManager"));
  if (cmgr)
  {
    double xyz[3];
    cmgr->ConvertDeviceToXYZ(xy[0], xy[1], xyz);
    QList<double> ret;
    ret << xyz[0] << xyz[1] << xyz[2];
    return ret;
  }

  QList<double> ret;
  ret << 0. << 0. << 0.;
  return ret;
}

// --------------------------------------------------------------------------
QList<double> qMRMLSliceView::convertRASToXYZ(const QList<double>& ras) const
{
  Q_D(const qMRMLSliceView);

  // Grab a displayable manager that is derived from
  // AbstractSliceViewDisplayableManager, like the CrosshairDisplayableManager
  vtkMRMLCrosshairDisplayableManager* cmgr =
    vtkMRMLCrosshairDisplayableManager::SafeDownCast(d->DisplayableManagerGroup->GetDisplayableManagerByClassName("vtkMRMLCrosshairDisplayableManager"));
  if (cmgr)
  {
    double rasv[3], xyz[3];
    rasv[0] = ras[0];
    rasv[1] = ras[1];
    rasv[2] = ras[2];
    cmgr->ConvertRASToXYZ(rasv, xyz);
    QList<double> ret;
    ret << xyz[0] << xyz[1] << xyz[2];
    return ret;
  }

  QList<double> ret;
  ret << 0. << 0. << 0.;
  return ret;
}

// --------------------------------------------------------------------------
QList<double> qMRMLSliceView::convertXYZToRAS(const QList<double>& xyz) const
{
  Q_D(const qMRMLSliceView);

  // Grab a displayable manager that is derived from
  // AbstractSliceViewDisplayableManager, like the CrosshairDisplayableManager
  vtkMRMLCrosshairDisplayableManager* cmgr =
    vtkMRMLCrosshairDisplayableManager::SafeDownCast(d->DisplayableManagerGroup->GetDisplayableManagerByClassName("vtkMRMLCrosshairDisplayableManager"));
  if (cmgr)
  {
    double xyzv[3], ras[3];
    xyzv[0] = xyz[0];
    xyzv[1] = xyz[1];
    xyzv[2] = xyz[2];
    cmgr->ConvertXYZToRAS(xyzv, ras);
    QList<double> ret;
    ret << ras[0] << ras[1] << ras[2];
    return ret;
  }

  QList<double> ret;
  ret << 0. << 0. << 0.;
  return ret;
}

// --------------------------------------------------------------------------
void qMRMLSliceView::setViewCursor(const QCursor& cursor)
{
  this->setCursor(cursor);
  if (this->VTKWidget() != nullptr)
  {
    this->VTKWidget()->setCursor(cursor); // TODO: test if cursor settings works
  }
}

// --------------------------------------------------------------------------
void qMRMLSliceView::unsetViewCursor()
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
void qMRMLSliceView::setDefaultViewCursor(const QCursor& cursor)
{
  if (this->VTKWidget() != nullptr)
  {
    this->VTKWidget()->setDefaultCursor(cursor); // TODO: test if cursor settings works
  }
}

//---------------------------------------------------------------------------
void qMRMLSliceView::dragEnterEvent(QDragEnterEvent* event)
{
  Q_D(qMRMLSliceView);
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
void qMRMLSliceView::dropEvent(QDropEvent* event)
{
  Q_D(qMRMLSliceView);
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
  shNode->ShowItemsInView(shItemIdList, this->mrmlSliceNode());
}

//---------------------------------------------------------------------------
vtkMRMLDisplayableManagerGroup* qMRMLSliceView::displayableManagerGroup() const
{
  Q_D(const qMRMLSliceView);
  return d->DisplayableManagerGroup;
}

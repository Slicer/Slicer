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
#include <QEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QToolButton>

// CTK includes
#include <ctkAxesWidget.h>
#include <ctkPopupWidget.h>
#include <vtkLightBoxRendererManager.h>

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLSliceView_p.h"

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractDisplayableManager.h>
#include <vtkMRMLCrosshairDisplayableManager.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLLightBoxRendererManagerProxy.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
#include <vtkSliceViewInteractorStyle.h>

// MRML includes
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

//--------------------------------------------------------------------------
// qMRMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy class

//--------------------------------------------------------------------------
// vtkInternalLightBoxRendereManagerProxy methods
//vtkStandardNewMacro(qMRMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy );

//---------------------------------------------------------------------------
// Using the vtkStandardNewMacro results in a compiler error about
// vtkInstantiatorqMRMLSliceWidgetPrivate has not been declared. This
// seems to be due to how the macro uses the type passed into the
// vtkStandardNewMacro as both a type and a classname string. Below,
// we do the equivalent to the vtkStandardNewMacro but use the full
// path to the type where needed and the scoped name elsewhere.
qMRMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy *
qMRMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkInternalLightBoxRendererManagerProxy");
  if (ret)
    {
    return static_cast<qMRMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy*>(ret);
    }
  return new qMRMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy;
}

//---------------------------------------------------------------------------
qMRMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy::vtkInternalLightBoxRendererManagerProxy()
{
  this->LightBoxRendererManager = 0;
}

//---------------------------------------------------------------------------
qMRMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy::~vtkInternalLightBoxRendererManagerProxy()
{
  this->LightBoxRendererManager = 0;
}

//---------------------------------------------------------------------------
vtkRenderer* qMRMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy::GetRenderer(int id)
{
  if (this->LightBoxRendererManager)
    {
    return this->LightBoxRendererManager->GetRenderer(id);
    }
  return NULL;
}

//---------------------------------------------------------------------------
void qMRMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy::SetLightBoxRendererManager(vtkLightBoxRendererManager *mgr)
{
  this->LightBoxRendererManager = mgr;
}

//--------------------------------------------------------------------------
// qMRMLSliceViewPrivate methods

//---------------------------------------------------------------------------
qMRMLSliceViewPrivate::qMRMLSliceViewPrivate(qMRMLSliceView& object)
  : q_ptr(&object)
{
  this->DisplayableManagerGroup = 0;
  this->MRMLScene = 0;
  this->MRMLSliceNode = 0;
  this->InactiveBoxColor = QColor(95, 95, 113);
  this->LightBoxRendererManagerProxy = vtkInternalLightBoxRendererManagerProxy::New();
}

//---------------------------------------------------------------------------
qMRMLSliceViewPrivate::~qMRMLSliceViewPrivate()
{
  if (this->DisplayableManagerGroup)
    {
    this->DisplayableManagerGroup->Delete();
    }
  if (this->LightBoxRendererManagerProxy)
    {
    this->LightBoxRendererManagerProxy->Delete();
    }
}

//---------------------------------------------------------------------------
void qMRMLSliceViewPrivate::init()
{
  Q_Q(qMRMLSliceView);

  // Highligh first RenderWindowItem
  q->setHighlightedBoxColor(this->InactiveBoxColor);

  q->setRenderEnabled(this->MRMLScene != 0);

  vtkNew<vtkSliceViewInteractorStyle> interactorStyle;
  q->interactor()->SetInteractorStyle(interactorStyle.GetPointer());

  this->LightBoxRendererManagerProxy->SetLightBoxRendererManager(
    q->lightBoxRendererManager());
  this->initDisplayableManagers();
}

//---------------------------------------------------------------------------
void qMRMLSliceViewPrivate::initDisplayableManagers()
{
  Q_Q(qMRMLSliceView);

  vtkMRMLSliceViewDisplayableManagerFactory* factory
    = vtkMRMLSliceViewDisplayableManagerFactory::GetInstance();

  QStringList displayableManagers;
  displayableManagers << "vtkMRMLVolumeGlyphSliceDisplayableManager";
  displayableManagers << "vtkMRMLModelSliceDisplayableManager";
  displayableManagers << "vtkMRMLCrosshairDisplayableManager";
  foreach(const QString& displayableManager, displayableManagers)
    {
    if (!factory->IsDisplayableManagerRegistered(displayableManager.toLatin1()))
      {
      factory->RegisterDisplayableManager(displayableManager.toLatin1());
      }
    }

  this->DisplayableManagerGroup
    = factory->InstantiateDisplayableManagers(
      q->lightBoxRendererManager()->GetRenderer(0));
  // Observe displayable manager group to catch RequestRender events
  q->qvtkConnect(this->DisplayableManagerGroup, vtkCommand::UpdateEvent,
                 q, SLOT(scheduleRender()));

  // pass the lightbox manager proxy onto the display managers
  this->DisplayableManagerGroup->SetLightBoxRendererManagerProxy(this->LightBoxRendererManagerProxy);

}

//---------------------------------------------------------------------------
void qMRMLSliceViewPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_Q(qMRMLSliceView);
  if (newScene == this->MRMLScene)
    {
    return;
    }

  this->qvtkReconnect(
    this->MRMLScene, newScene,
    vtkMRMLScene::StartBatchProcessEvent, this, SLOT(onSceneStartProcessing()));

  this->qvtkReconnect(
    this->MRMLScene, newScene,
    vtkMRMLScene::EndBatchProcessEvent, this, SLOT(onSceneEndProcessing()));

  this->MRMLScene = newScene;
  q->setRenderEnabled(
    this->MRMLScene != 0 && !this->MRMLScene->IsBatchProcessing());
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
void qMRMLSliceViewPrivate::updateWidgetFromMRML()
{
  Q_Q(qMRMLSliceView);
  if (!this->MRMLSliceNode)
    {
    return;
    }
  q->lightBoxRendererManager()->SetRenderWindowLayout(
    this->MRMLSliceNode->GetLayoutGridRows(),
    this->MRMLSliceNode->GetLayoutGridColumns());
  bool displayLightboxBorders =
    this->MRMLSliceNode->GetLayoutGridRows() != 1 ||
    this->MRMLSliceNode->GetLayoutGridColumns() != 1;
  q->lightBoxRendererManager()->SetHighlighted(0, 0, displayLightboxBorders);
}

// --------------------------------------------------------------------------
// qMRMLSliceView methods

// --------------------------------------------------------------------------
qMRMLSliceView::qMRMLSliceView(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLSliceViewPrivate(*this))
{
  Q_D(qMRMLSliceView);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLSliceView::~qMRMLSliceView()
{
}

//------------------------------------------------------------------------------
void qMRMLSliceView::addDisplayableManager(const QString& displayableManagerName)
{
  Q_D(qMRMLSliceView);
  vtkSmartPointer<vtkMRMLAbstractDisplayableManager> displayableManager;
  displayableManager.TakeReference(
    vtkMRMLDisplayableManagerGroup::InstantiateDisplayableManager(
      displayableManagerName.toLatin1()));
  d->DisplayableManagerGroup->AddDisplayableManager(displayableManager);
}

//------------------------------------------------------------------------------
void qMRMLSliceView::getDisplayableManagers(vtkCollection *displayableManagers)
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
void qMRMLSliceView::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLSliceView);
  d->setMRMLScene(newScene);

  if (d->MRMLSliceNode && newScene != d->MRMLSliceNode->GetScene())
    {
    this->setMRMLSliceNode(0);
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

  d->qvtkReconnect(
    d->MRMLSliceNode, newSliceNode,
    vtkCommand::ModifiedEvent, d, SLOT(updateWidgetFromMRML()));

  d->MRMLSliceNode = newSliceNode;
  d->DisplayableManagerGroup->SetMRMLDisplayableNode(newSliceNode);

  d->updateWidgetFromMRML();
  // Enable/disable widget
  this->setEnabled(newSliceNode != 0);
}

//---------------------------------------------------------------------------
vtkMRMLSliceNode* qMRMLSliceView::mrmlSliceNode()const
{
  Q_D(const qMRMLSliceView);
  return d->MRMLSliceNode;
}

//---------------------------------------------------------------------------
vtkSliceViewInteractorStyle* qMRMLSliceView::sliceViewInteractorStyle()const
{
  return vtkSliceViewInteractorStyle::SafeDownCast(this->interactorStyle());
}

// --------------------------------------------------------------------------
QList<double> qMRMLSliceView::convertDeviceToXYZ(const QList<int>& xy)
{
  Q_D(qMRMLSliceView);

  // Grab a displayable manager that is derived from
  // AbstractSliceViewDisplayableManager, like the CrosshairDisplayableManager
  vtkMRMLCrosshairDisplayableManager *cmgr =
    vtkMRMLCrosshairDisplayableManager::SafeDownCast(
      d->DisplayableManagerGroup->GetDisplayableManagerByClassName(
        "vtkMRMLCrosshairDisplayableManager"));
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
QList<double> qMRMLSliceView::convertRASToXYZ(const QList<double>& ras)
{
  Q_D(qMRMLSliceView);

  // Grab a displayable manager that is derived from
  // AbstractSliceViewDisplayableManager, like the CrosshairDisplayableManager
  vtkMRMLCrosshairDisplayableManager *cmgr =
    vtkMRMLCrosshairDisplayableManager::SafeDownCast(
      d->DisplayableManagerGroup->GetDisplayableManagerByClassName(
        "vtkMRMLCrosshairDisplayableManager"));
  if (cmgr)
    {
    double rasv[3], xyz[3];
    rasv[0] = ras[0]; rasv[1] = ras[1]; rasv[2] = ras[2];
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
QList<double> qMRMLSliceView::convertXYZToRAS(const QList<double>& xyz)
{
  Q_D(qMRMLSliceView);

  // Grab a displayable manager that is derived from
  // AbstractSliceViewDisplayableManager, like the CrosshairDisplayableManager
  vtkMRMLCrosshairDisplayableManager *cmgr =
    vtkMRMLCrosshairDisplayableManager::SafeDownCast(
      d->DisplayableManagerGroup->GetDisplayableManagerByClassName(
        "vtkMRMLCrosshairDisplayableManager"));
  if (cmgr)
    {
    double xyzv[3], ras[3];
    xyzv[0] = xyz[0]; xyzv[1] = xyz[1]; xyzv[2] = xyz[2];
    cmgr->ConvertXYZToRAS(xyzv, ras);
    QList<double> ret;
    ret << ras[0] << ras[1] << ras[2];
    return ret;
    }

  QList<double> ret;
  ret << 0. << 0. << 0.;
  return ret;
}

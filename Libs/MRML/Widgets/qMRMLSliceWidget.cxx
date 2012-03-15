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

// CTK includes
#include <vtkLightBoxRendererManager.h>

// qMRML includes
#include "qMRMLSliceWidget_p.h"

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
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

//--------------------------------------------------------------------------
// qMRMLSliceWidgetPrivate::vtkInternalLightBoxRendererManagerProxy class

//---------------------------------------------------------------------------
class qMRMLSliceWidgetPrivate::vtkInternalLightBoxRendererManagerProxy :
  public vtkMRMLLightBoxRendererManagerProxy
{
public:
  static vtkInternalLightBoxRendererManagerProxy* New();
  vtkTypeRevisionMacro(vtkInternalLightBoxRendererManagerProxy,
                       vtkMRMLLightBoxRendererManagerProxy);


  /// Method to query the mapping from an id of a LightBox frame to
  /// the Renderer for that frame
  virtual vtkRenderer *GetRenderer(int id);

  /// Method to set the real LightBoxManager
  virtual void SetLightBoxRendererManager(vtkLightBoxRendererManager *mgr);

protected:
  vtkInternalLightBoxRendererManagerProxy();
  virtual ~vtkInternalLightBoxRendererManagerProxy();

private:
  vtkInternalLightBoxRendererManagerProxy(const vtkInternalLightBoxRendererManagerProxy&); // Not implemented
  void operator=(const vtkInternalLightBoxRendererManagerProxy&);                    // Not implemented

  vtkWeakPointer<vtkLightBoxRendererManager> LightBoxRendererManager;

};

//--------------------------------------------------------------------------
// vtkInternalLightBoxRendereManagerProxy methods
//vtkStandardNewMacro(qMRMLSliceWidgetPrivate::vtkInternalLightBoxRendererManagerProxy );
vtkCxxRevisionMacro(qMRMLSliceWidgetPrivate::vtkInternalLightBoxRendererManagerProxy, "$Revision: 13525 $");

//---------------------------------------------------------------------------
// Using the vtkStandardNewMacro results in a compiler error about
// vtkInstantiatorqMRMLSliceWidgetPrivate has not been declared. This
// seems to be due to how the macro uses the type passed into the
// vtkStandardNewMacro as both a type and a classname string. Below,
// we do the equivalent to the vtkStandardNewMacro but use the full
// path to the type where needed and the scoped name elsewhere.
qMRMLSliceWidgetPrivate::vtkInternalLightBoxRendererManagerProxy *
qMRMLSliceWidgetPrivate::vtkInternalLightBoxRendererManagerProxy::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkInternalLightBoxRendererManagerProxy");
  if (ret)
    {
    return static_cast<qMRMLSliceWidgetPrivate::vtkInternalLightBoxRendererManagerProxy*>(ret);
    }
  return new qMRMLSliceWidgetPrivate::vtkInternalLightBoxRendererManagerProxy;
}

//---------------------------------------------------------------------------
qMRMLSliceWidgetPrivate::vtkInternalLightBoxRendererManagerProxy::vtkInternalLightBoxRendererManagerProxy()
{
  this->LightBoxRendererManager = 0;
}

//---------------------------------------------------------------------------
qMRMLSliceWidgetPrivate::vtkInternalLightBoxRendererManagerProxy::~vtkInternalLightBoxRendererManagerProxy()
{
  this->LightBoxRendererManager = 0;
}

//---------------------------------------------------------------------------
vtkRenderer* qMRMLSliceWidgetPrivate::vtkInternalLightBoxRendererManagerProxy::GetRenderer(int id)
{
  if (this->LightBoxRendererManager)
    {
    return this->LightBoxRendererManager->GetRenderer(id);
    }
  return NULL;
}

//---------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::vtkInternalLightBoxRendererManagerProxy::SetLightBoxRendererManager(vtkLightBoxRendererManager *mgr)
{
  this->LightBoxRendererManager = mgr;
}


//--------------------------------------------------------------------------
// qMRMLSliceViewPrivate methods

//---------------------------------------------------------------------------
qMRMLSliceWidgetPrivate::qMRMLSliceWidgetPrivate(qMRMLSliceWidget& object)
  : q_ptr(&object)
{
  this->DisplayableManagerGroup = 0;
  this->MRMLSliceNode = 0;
  this->InactiveBoxColor = QColor(95, 95, 113);
  this->LightBoxRendererManagerProxy = vtkInternalLightBoxRendererManagerProxy::New();
}

//---------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::init()
{
  Q_Q(qMRMLSliceWidget);
  this->setupUi(q);

  // Highligh first RenderWindowItem
  this->VTKSliceView->setHighlightedBoxColor(this->InactiveBoxColor);
  //this->VTKSliceView->findChild<QVTKWidget*>()->installEventFilter(q);

  vtkNew<vtkSliceViewInteractorStyle> interactorStyle;
  interactorStyle->SetSliceLogic(this->SliceController->sliceLogic());
  this->VTKSliceView->interactor()->SetInteractorStyle(interactorStyle.GetPointer());

  connect(this->VTKSliceView, SIGNAL(resized(QSize)),
          this->SliceController, SLOT(setSliceViewSize(QSize)));

  connect(this->SliceController, SIGNAL(imageDataChanged(vtkImageData*)),
          this, SLOT(setImageData(vtkImageData*)));
  connect(this->SliceController, SIGNAL(renderRequested()),
          this->VTKSliceView, SLOT(scheduleRender()), Qt::QueuedConnection);

  this->LightBoxRendererManagerProxy->SetLightBoxRendererManager(this->VTKSliceView->lightBoxRendererManager());
  this->initDisplayableManagers();
}

//---------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::initDisplayableManagers()
{
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
      this->VTKSliceView->lightBoxRendererManager()->GetRenderer(0));
  // Observe displayable manager group to catch RequestRender events
  this->qvtkConnect(this->DisplayableManagerGroup, vtkCommand::UpdateEvent,
                    this->VTKSliceView, SLOT(scheduleRender()));

  // pass the lightbox manager proxy on the CrosshairDisplayableManager
  vtkMRMLCrosshairDisplayableManager *cmgr = vtkMRMLCrosshairDisplayableManager::SafeDownCast(this->DisplayableManagerGroup->GetDisplayableManagerByClassName("vtkMRMLCrosshairDisplayableManager"));
  if (cmgr)
    {
    cmgr->SetLightBoxRendererManagerProxy(this->LightBoxRendererManagerProxy);
    }
}

//---------------------------------------------------------------------------
qMRMLSliceWidgetPrivate::~qMRMLSliceWidgetPrivate()
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

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::startProcessing()
{
  this->VTKSliceView->setRenderEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::endProcessing()
{
  this->VTKSliceView->setRenderEnabled(true);

  // When a scene is closed, we need to reconfigure the SliceNode to
  // the size of the widget.
  QRect rect = this->VTKSliceView->geometry();
  this->SliceController->setSliceViewSize(QSize(rect.width(), rect.height()));
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::setImageData(vtkImageData * imageData)
{
  //qDebug() << "qMRMLSliceWidgetPrivate::setImageData";
  this->VTKSliceView->setImageData(imageData);
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::updateWidgetFromMRMLSliceNode()
{
  if (!this->MRMLSliceNode)
    {
    return;
    }
  this->VTKSliceView->lightBoxRendererManager()->SetRenderWindowLayout(
    this->MRMLSliceNode->GetLayoutGridRows(),
    this->MRMLSliceNode->GetLayoutGridColumns());
  bool displayLightboxBorders =
    this->MRMLSliceNode->GetLayoutGridRows() != 1 ||
    this->MRMLSliceNode->GetLayoutGridColumns() != 1;
  this->VTKSliceView->lightBoxRendererManager()->SetHighlighted(0, 0, displayLightboxBorders);
}

// --------------------------------------------------------------------------
// qMRMLSliceView methods

// --------------------------------------------------------------------------
qMRMLSliceWidget::qMRMLSliceWidget(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLSliceWidgetPrivate(*this))
{
  Q_D(qMRMLSliceWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLSliceWidget::~qMRMLSliceWidget()
{
}

//------------------------------------------------------------------------------
void qMRMLSliceWidget::addDisplayableManager(const QString& displayableManagerName)
{
  Q_D(qMRMLSliceWidget);
  vtkSmartPointer<vtkMRMLAbstractDisplayableManager> displayableManager;
  displayableManager.TakeReference(
    vtkMRMLDisplayableManagerGroup::InstantiateDisplayableManager(
      displayableManagerName.toLatin1()));
  d->DisplayableManagerGroup->AddDisplayableManager(displayableManager);
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLSliceWidget);
  if (newScene == this->mrmlScene())
    {
    return;
    }

  d->qvtkReconnect(
    this->mrmlScene(), newScene,
    vtkMRMLScene::StartBatchProcessEvent, d, SLOT(startProcessing()));

  d->qvtkReconnect(
    this->mrmlScene(), newScene,
    vtkMRMLScene::EndBatchProcessEvent, d, SLOT(endProcessing()));

  this->Superclass::setMRMLScene(newScene);
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode)
{
  Q_D(qMRMLSliceWidget);
  if (newSliceNode == d->MRMLSliceNode)
    {
    return;
    }

  d->qvtkReconnect(d->MRMLSliceNode, newSliceNode, vtkCommand::ModifiedEvent,
                   d, SLOT(updateWidgetFromMRMLSliceNode()));

  d->MRMLSliceNode = newSliceNode;
  d->DisplayableManagerGroup->SetMRMLDisplayableNode(newSliceNode);
  d->SliceController->setMRMLSliceNode(newSliceNode);

  d->updateWidgetFromMRMLSliceNode();
}

//---------------------------------------------------------------------------
vtkMRMLSliceCompositeNode* qMRMLSliceWidget::mrmlSliceCompositeNode()const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceController->mrmlSliceCompositeNode();
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setSliceViewName(const QString& newSliceViewName)
{
  Q_D(qMRMLSliceWidget);
  d->SliceController->setSliceViewName(newSliceViewName);

  // QColor sliceViewColor =
  //   qMRMLSliceControllerWidget::sliceViewColor(newSliceViewName);

//Don't apply the color of the slice to the highlight box
//  double highlightedBoxColor[3];
//  highlightedBoxColor[0] = sliceViewColor.redF();
//  highlightedBoxColor[1] = sliceViewColor.greenF();
//  highlightedBoxColor[2] = sliceViewColor.blueF();
//  // Set the color associated with the highlightedBox
//  d->VTKSliceView->setHighlightedBoxColor(highlightedBoxColor);
}

//---------------------------------------------------------------------------
QString qMRMLSliceWidget::sliceViewName()const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceController->sliceViewName();
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setSliceViewLabel(const QString& newSliceViewLabel)
{
  Q_D(qMRMLSliceWidget);
  d->SliceController->setSliceViewLabel(newSliceViewLabel);
}

//---------------------------------------------------------------------------
QString qMRMLSliceWidget::sliceViewLabel()const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceController->sliceViewLabel();
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setSliceViewColor(const QColor& newSliceViewColor)
{
  Q_D(qMRMLSliceWidget);
  d->SliceController->setSliceViewColor(newSliceViewColor);
}

//---------------------------------------------------------------------------
QColor qMRMLSliceWidget::sliceViewColor()const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceController->sliceViewColor();
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setSliceOrientation(const QString& orientation)
{
  Q_D(qMRMLSliceWidget);
  d->SliceController->setSliceOrientation(orientation);
}

//---------------------------------------------------------------------------
QString qMRMLSliceWidget::sliceOrientation()const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceController->sliceOrientation();
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setImageData(vtkImageData* newImageData)
{
  Q_D(qMRMLSliceWidget);
  d->SliceController->setImageData(newImageData);
}

//---------------------------------------------------------------------------
vtkImageData* qMRMLSliceWidget::imageData() const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceController->imageData();
}

//---------------------------------------------------------------------------
vtkInteractorObserver* qMRMLSliceWidget::interactorStyle()const
{
  return this->sliceView()->interactorStyle();
}

//---------------------------------------------------------------------------
vtkCornerAnnotation* qMRMLSliceWidget::overlayCornerAnnotation()const
{
  return this->sliceView()->overlayCornerAnnotation();
}

//---------------------------------------------------------------------------
vtkMRMLSliceNode* qMRMLSliceWidget::mrmlSliceNode()const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceController->mrmlSliceNode();
}

//---------------------------------------------------------------------------
vtkMRMLSliceLogic* qMRMLSliceWidget::sliceLogic()const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceController->sliceLogic();
}

// --------------------------------------------------------------------------
void qMRMLSliceWidget::fitSliceToBackground()
{
  Q_D(qMRMLSliceWidget);
  d->SliceController->fitSliceToBackground();
}

// --------------------------------------------------------------------------
const ctkVTKSliceView* qMRMLSliceWidget::sliceView()const
{
  Q_D(const qMRMLSliceWidget);
  return d->VTKSliceView;
}

// --------------------------------------------------------------------------
qMRMLSliceControllerWidget* qMRMLSliceWidget::sliceController()const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceController;
}

// --------------------------------------------------------------------------
void qMRMLSliceWidget::setSliceLogics(vtkCollection* logics)
{
  Q_D(qMRMLSliceWidget);
  d->SliceController->setSliceLogics(logics);
}

//// --------------------------------------------------------------------------
//bool qMRMLSliceWidget::eventFilter(QObject* object, QEvent* event)
//{
//  Q_D(qMRMLSliceWidget);
//  if (this->isAncestorOf(qobject_cast<QWidget*>(object)))
//    {
//    if (event->type() == QEvent::FocusIn)
//      {
//      d->VTKSliceView->setHighlightedBoxColor(Qt::white);
//      d->VTKSliceView->scheduleRender();
//      }
//    else if (event->type() == QEvent::FocusOut)
//      {
//      d->VTKSliceView->setHighlightedBoxColor(d->InactiveBoxColor);
//      d->VTKSliceView->scheduleRender();
//      }
//    }
//  return this->Superclass::eventFilter(object, event);
//}


// --------------------------------------------------------------------------
QList<double> qMRMLSliceWidget::convertDeviceToXYZ(const QList<int>& xy)
{
  Q_D(qMRMLSliceWidget);

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
QList<double> qMRMLSliceWidget::convertRASToXYZ(const QList<double>& ras)
{
  Q_D(qMRMLSliceWidget);

  // Grab a displayable manager that is derived from
  // AbstractSliceViewDisplayableManager, like the CrosshairDisplayableManager
  vtkMRMLCrosshairDisplayableManager *cmgr = vtkMRMLCrosshairDisplayableManager::SafeDownCast(d->DisplayableManagerGroup->GetDisplayableManagerByClassName("vtkMRMLCrosshairDisplayableManager"));
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
QList<double> qMRMLSliceWidget::convertXYZToRAS(const QList<double>& xyz)
{
  Q_D(qMRMLSliceWidget);

  // Grab a displayable manager that is derived from
  // AbstractSliceViewDisplayableManager, like the CrosshairDisplayableManager
  vtkMRMLCrosshairDisplayableManager *cmgr = vtkMRMLCrosshairDisplayableManager::SafeDownCast(d->DisplayableManagerGroup->GetDisplayableManagerByClassName("vtkMRMLCrosshairDisplayableManager"));
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

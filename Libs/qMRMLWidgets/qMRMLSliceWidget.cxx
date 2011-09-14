/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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

// CTK includes
#include <ctkLogger.h>
#include <vtkLightBoxRendererManager.h>

// qMRML includes
#include "qMRMLSliceWidget_p.h"

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractDisplayableManager.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
//#include <vtkSliceViewInteractorStyle.h>

// MRML includes
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLSliceWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLSliceViewPrivate methods

//---------------------------------------------------------------------------
qMRMLSliceWidgetPrivate::qMRMLSliceWidgetPrivate(qMRMLSliceWidget& object)
  : q_ptr(&object)
{
  this->DisplayableManagerGroup = 0;
  this->MRMLSliceNode = 0;
}

//---------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::init()
{
  Q_Q(qMRMLSliceWidget);
  this->setupUi(q);

  // Highligh first RenderWindowItem
  this->VTKSliceView->setHighlightedBoxColor(QColor(102, 102, 119));
  this->VTKSliceView->lightBoxRendererManager()->SetHighlighted(0, 0, true);
  this->VTKSliceView->findChild<QVTKWidget*>()->installEventFilter(q);

  connect(this->VTKSliceView, SIGNAL(resized(QSize)),
          this->SliceController, SLOT(setSliceViewSize(QSize)));

  connect(this->SliceController, SIGNAL(imageDataChanged(vtkImageData*)),
          this, SLOT(setImageData(vtkImageData*)));
  connect(this->SliceController, SIGNAL(renderRequested()),
          this->VTKSliceView, SLOT(scheduleRender()));

  this->initDisplayableManagers();
}

//---------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::initDisplayableManagers()
{
  vtkMRMLSliceViewDisplayableManagerFactory* factory
    = vtkMRMLSliceViewDisplayableManagerFactory::GetInstance();

  QStringList displayableManagers;
  displayableManagers << "vtkMRMLSliceModelDisplayableManager";
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
}

//---------------------------------------------------------------------------
qMRMLSliceWidgetPrivate::~qMRMLSliceWidgetPrivate()
{
  if (this->DisplayableManagerGroup)
    {
    this->DisplayableManagerGroup->Delete();
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::onSceneAboutToBeClosedEvent()
{
  this->VTKSliceView->setRenderEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::onSceneClosedEvent()
{
  Q_Q(qMRMLSliceWidget);
  if (!q->mrmlScene()->GetIsUpdating())
    {
    this->VTKSliceView->setRenderEnabled(true);
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::onSceneAboutToBeImportedEvent()
{
  this->VTKSliceView->setRenderEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::onSceneImportedEvent()
{
  this->VTKSliceView->setRenderEnabled(true);
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::onSceneRestoredEvent()
{
  this->VTKSliceView->setRenderEnabled(true);
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::setImageData(vtkImageData * imageData)
{
  logger.trace("setImageData");
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
    vtkMRMLScene::SceneAboutToBeClosedEvent, d, SLOT(onSceneAboutToBeClosedEvent()));

  d->qvtkReconnect(
    this->mrmlScene(), newScene,
    vtkMRMLScene::SceneClosedEvent, d, SLOT(onSceneClosedEvent()));

  d->qvtkReconnect(
    this->mrmlScene(), newScene,
    vtkMRMLScene::SceneAboutToBeImportedEvent, d, SLOT(onSceneAboutToBeImportedEvent()));

  d->qvtkReconnect(
    this->mrmlScene(), newScene,
    vtkMRMLScene::SceneImportedEvent, d, SLOT(onSceneImportedEvent()));

  d->qvtkReconnect(
    this->mrmlScene(), newScene,
    vtkMRMLScene::SceneRestoredEvent, d, SLOT(onSceneRestoredEvent()));

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

namespace
{
//---------------------------------------------------------------------------
bool slicer_qcolor_is_valid_color(const QString& colorName)
{
#if QT_VERSION < 0x040700
  if (colorName.startsWith("compare"))
    {
    return false;
    }
  QColor c;
  c.setNamedColor(colorName);
  return c.isValid();
#else
  return QColor::isValidColor(colorName);
#endif
}
} // end namespace

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setSliceViewName(const QString& newSliceViewName)
{
  Q_D(qMRMLSliceWidget);
  d->SliceController->setSliceViewName(newSliceViewName);

  QColor sliceViewColor =
    qMRMLSliceControllerWidget::sliceViewColor(newSliceViewName);

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

// --------------------------------------------------------------------------
bool qMRMLSliceWidget::eventFilter(QObject* object, QEvent* event)
{
  Q_D(qMRMLSliceWidget);
  if (this->isAncestorOf(qobject_cast<QWidget*>(object)))
    {
    if (event->type() == QEvent::FocusIn)
      {
      d->VTKSliceView->setHighlightedBoxColor(Qt::white);
      d->VTKSliceView->scheduleRender();
      }
    else if (event->type() == QEvent::FocusOut)
      {
      d->VTKSliceView->setHighlightedBoxColor(QColor(102, 102, 119));
      d->VTKSliceView->scheduleRender();
      }
    }
  return this->Superclass::eventFilter(object, event);
}

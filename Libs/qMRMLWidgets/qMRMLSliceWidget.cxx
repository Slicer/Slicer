
// Qt includes
#include <QDebug>
#include <QMenu>
#include <QFileInfo>

// CTK includes
#include <ctkVTKSliceView.h>
#include <ctkLogger.h>
#include <vtkLightBoxRendererManager.h>

// qMRML includes
#include "qMRMLSliceWidget.h"
#include "qMRMLSliceWidget_p.h"

// MRMLDisplayableManager includes
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
#include <vtkMRMLDisplayableManagerGroup.h>
//#include <vtkSliceViewInteractorStyle.h>

// MRML includes
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkImageData.h>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLSliceWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLSliceViewPrivate methods

//---------------------------------------------------------------------------
qMRMLSliceWidgetPrivate::qMRMLSliceWidgetPrivate()
{
  this->DisplayableManagerGroup = 0;
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
  CTK_P(qMRMLSliceWidget);
  if (!p->mrmlScene()->GetIsUpdating())
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
void qMRMLSliceWidgetPrivate::onImageDataModified(vtkImageData * imageData)
{
  logger.trace("onImageDataModifiedEvent");
  this->VTKSliceView->setImageData(imageData);
  this->VTKSliceView->scheduleRender();
}

// --------------------------------------------------------------------------
// qMRMLSliceView methods

// --------------------------------------------------------------------------
qMRMLSliceWidget::qMRMLSliceWidget(QWidget* _parent) : Superclass(_parent)
{
  CTK_INIT_PRIVATE(qMRMLSliceWidget);
  CTK_D(qMRMLSliceWidget);
  d->setupUi(this);

  connect(d->VTKSliceView, SIGNAL(resized(const QSize&,const QSize&)),
          d->SliceController, SLOT(setSliceViewSize(const QSize&)));

  connect(d->SliceController,
          SIGNAL(imageDataModified(vtkImageData*)), d,
          SLOT(onImageDataModified(vtkImageData*)));
}

//------------------------------------------------------------------------------
void qMRMLSliceWidget::registerDisplayableManagers(const QString& scriptedDisplayableManagerDirectory)
{
  CTK_D(qMRMLSliceWidget);

  QStringList displayableManagers;
  //displayableManagers << "vtkSliceDisplayableManager";

#ifdef MRMLDisplayableManager_USE_PYTHON
  QFileInfo dirInfo(scriptedDisplayableManagerDirectory);
  Q_ASSERT(dirInfo.isDir());
  if (dirInfo.isDir())
    {
    //displayableManagers<< QString("%1/vtkScriptedExampleDisplayableManager.py").
    //  arg(scriptedDisplayableManagerDirectory);
    }
  else
    {
    logger.error(QString("registerDisplayableManagers - directory %1 doesn't exists !").
                 arg(scriptedDisplayableManagerDirectory));
    }
#endif

  // Register Displayable Managers
  vtkMRMLSliceViewDisplayableManagerFactory* factory =
      vtkMRMLSliceViewDisplayableManagerFactory::GetInstance();

  foreach(const QString displayableManagerName, displayableManagers)
    {
    if (!factory->IsDisplayableManagerRegistered(displayableManagerName.toLatin1()))
      {
      factory->RegisterDisplayableManager(displayableManagerName.toLatin1());
      }
    }

  d->DisplayableManagerGroup = factory->InstantiateDisplayableManagers(
      d->VTKSliceView->lightBoxRendererManager()->GetRenderer(0));
  Q_ASSERT(d->DisplayableManagerGroup);

  // Observe displayable manager group to catch RequestRender events
  d->qvtkConnect(d->DisplayableManagerGroup, vtkCommand::UpdateEvent,
                 d->VTKSliceView, SLOT(scheduleRender()));
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  CTK_D(qMRMLSliceWidget);
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
  CTK_D(qMRMLSliceWidget);
  d->DisplayableManagerGroup->SetMRMLDisplayableNode(newSliceNode);
  d->SliceController->setMRMLSliceNode(newSliceNode);
}

//---------------------------------------------------------------------------
vtkMRMLSliceCompositeNode* qMRMLSliceWidget::mrmlSliceCompositeNode()const
{
  return ctk_d()->SliceController->mrmlSliceCompositeNode();
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setSliceViewName(const QString& newSliceViewName)
{
  ctk_d()->SliceController->setSliceViewName(newSliceViewName);
}

//---------------------------------------------------------------------------
QString qMRMLSliceWidget::sliceViewName()const
{
  return ctk_d()->SliceController->sliceViewName();
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setSliceOrientation(const QString& orientation)
{
  ctk_d()->SliceController->setSliceOrientation(orientation);
}

//---------------------------------------------------------------------------
QString qMRMLSliceWidget::sliceOrientation()const
{
  return ctk_d()->SliceController->sliceOrientation();
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setImageData(vtkImageData* newImageData)
{
  ctk_d()->SliceController->setImageData(newImageData);
}

//---------------------------------------------------------------------------
vtkImageData* qMRMLSliceWidget::imageData() const
{
  return ctk_d()->SliceController->imageData();
}

//---------------------------------------------------------------------------
vtkInteractorObserver* qMRMLSliceWidget::interactorStyle()const
{
  return this->sliceView()->interactorStyle();
}

//---------------------------------------------------------------------------
vtkMRMLSliceNode* qMRMLSliceWidget::mrmlSliceNode()const
{
  return ctk_d()->SliceController->mrmlSliceNode();
}

//---------------------------------------------------------------------------
vtkMRMLSliceLogic* qMRMLSliceWidget::sliceLogic()const
{
  return ctk_d()->SliceController->sliceLogic();
}

// --------------------------------------------------------------------------
void qMRMLSliceWidget::fitSliceToBackground()
{
  ctk_d()->SliceController->fitSliceToBackground();
}

// --------------------------------------------------------------------------
CTK_GET_CXX(qMRMLSliceWidget, ctkVTKSliceView*, sliceView, VTKSliceView);
CTK_GET_CXX(qMRMLSliceWidget, qMRMLSliceControllerWidget*, sliceController, SliceController);


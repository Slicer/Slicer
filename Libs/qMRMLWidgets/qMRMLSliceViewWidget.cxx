
// Qt includes
// #include <QStringList>
#include <QDebug>
#include <QMenu>

// CTK includes
#include <ctkVTKSliceView.h>
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLSliceViewWidget.h"
#include "qMRMLSliceViewWidget_p.h"

// MRML includes
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkImageData.h>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLSliceViewWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLSliceViewPrivate methods

//---------------------------------------------------------------------------
qMRMLSliceViewWidgetPrivate::qMRMLSliceViewWidgetPrivate()
{
}

//---------------------------------------------------------------------------
qMRMLSliceViewWidgetPrivate::~qMRMLSliceViewWidgetPrivate()
{
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onSceneAboutToBeClosedEvent()
{
  this->VTKSliceView->setRenderEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onSceneClosedEvent()
{
  CTK_P(qMRMLSliceViewWidget);
  if (!p->mrmlScene()->GetIsUpdating())
    {
    this->VTKSliceView->setRenderEnabled(true);
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onSceneAboutToBeImportedEvent()
{
  this->VTKSliceView->setRenderEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onSceneImportedEvent()
{
  this->VTKSliceView->setRenderEnabled(true);
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onSceneRestoredEvent()
{
  this->VTKSliceView->setRenderEnabled(true);
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onImageDataModified(vtkImageData * imageData)
{
  logger.trace("onImageDataModifiedEvent");
  this->VTKSliceView->setImageData(imageData);
  this->VTKSliceView->scheduleRender();
}

// --------------------------------------------------------------------------
// qMRMLSliceView methods

// --------------------------------------------------------------------------
qMRMLSliceViewWidget::qMRMLSliceViewWidget(QWidget* _parent) : Superclass(_parent)
{
  CTK_INIT_PRIVATE(qMRMLSliceViewWidget);
  CTK_D(qMRMLSliceViewWidget);
  d->setupUi(this);

  connect(d->VTKSliceView, SIGNAL(resized(const QSize&,const QSize&)),
          d->SliceController, SLOT(setSliceViewSize(const QSize&)));

  connect(d->SliceController,
          SIGNAL(imageDataModified(vtkImageData*)), d,
          SLOT(onImageDataModified(vtkImageData*)));
}

//---------------------------------------------------------------------------
void qMRMLSliceViewWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  CTK_D(qMRMLSliceViewWidget);
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
void qMRMLSliceViewWidget::setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode)
{
  ctk_d()->SliceController->setMRMLSliceNode(newSliceNode);
}

//---------------------------------------------------------------------------
vtkMRMLSliceCompositeNode* qMRMLSliceViewWidget::mrmlSliceCompositeNode()const
{
  return ctk_d()->SliceController->mrmlSliceCompositeNode();
}

//---------------------------------------------------------------------------
void qMRMLSliceViewWidget::setSliceViewName(const QString& newSliceViewName)
{
  ctk_d()->SliceController->setSliceViewName(newSliceViewName);
}

//---------------------------------------------------------------------------
QString qMRMLSliceViewWidget::sliceViewName()const
{
  return ctk_d()->SliceController->sliceViewName();
}

//---------------------------------------------------------------------------
void qMRMLSliceViewWidget::setSliceOrientation(const QString& orientation)
{
  ctk_d()->SliceController->setSliceOrientation(orientation);
}

//---------------------------------------------------------------------------
QString qMRMLSliceViewWidget::sliceOrientation()const
{
  return ctk_d()->SliceController->sliceOrientation();
}

//---------------------------------------------------------------------------
void qMRMLSliceViewWidget::setImageData(vtkImageData* newImageData)
{
  ctk_d()->SliceController->setImageData(newImageData);
}

//---------------------------------------------------------------------------
vtkImageData* qMRMLSliceViewWidget::imageData() const
{
  return ctk_d()->SliceController->imageData();
}

//---------------------------------------------------------------------------
vtkMRMLSliceNode* qMRMLSliceViewWidget::mrmlSliceNode()const
{
  return ctk_d()->SliceController->mrmlSliceNode();
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidget::fitSliceToBackground()
{
  ctk_d()->SliceController->fitSliceToBackground();
}

// --------------------------------------------------------------------------
CTK_GET_CXX(qMRMLSliceViewWidget, ctkVTKSliceView*, sliceView, VTKSliceView);
CTK_GET_CXX(qMRMLSliceViewWidget, qMRMLSliceControllerWidget*, sliceController, SliceController);


// Qt includes
#include <QDebug>

// CTK includes
#include <ctkLogger.h>
#include <ctkVTKSliceView.h>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkIntArray.h>

// Slicer includes
#include "qSlicerSliceViewWidget.h"
#include "qSlicerSliceViewWidget_p.h"

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.base.qtgui.qSlicerSliceView");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qSlicerSliceViewWidgetPrivate methods

//---------------------------------------------------------------------------
qSlicerSliceViewWidgetPrivate::qSlicerSliceViewWidgetPrivate()
{
  logger.setTrace();

  this->SliceLogic = vtkSmartPointer<vtkMRMLSliceLogic>::New();

  this->qvtkConnect(this->SliceLogic,
                    vtkCommand::ModifiedEvent, this, SLOT(onSliceLogicModifiedEvent()));
}

//---------------------------------------------------------------------------
qSlicerSliceViewWidgetPrivate::~qSlicerSliceViewWidgetPrivate()
{
}

// --------------------------------------------------------------------------
void qSlicerSliceViewWidgetPrivate::onSliceLogicModifiedEvent()
{
  CTK_P(qSlicerSliceViewWidget);

  if (p->mrmlSliceCompositeNode() != this->SliceLogic->GetSliceCompositeNode())
    {
    p->setMRMLSliceCompositeNode(this->SliceLogic->GetSliceCompositeNode());
    }

  if (this->ImageData != this->SliceLogic->GetImageData())
    {
    logger.trace("onSliceLogicModifiedEvent - reconnect onImageDataModifiedEvent slot");
    this->qvtkReconnect(this->ImageData, this->SliceLogic->GetImageData(),
                      vtkCommand::ModifiedEvent, this, SLOT(onImageDataModifiedEvent()));
    this->ImageData = this->SliceLogic->GetImageData();

    this->onImageDataModifiedEvent();
    }
  
  // Set the scale increments to match the z spacing (rotated into slice space)
  const double * sliceSpacing = 0;
  sliceSpacing = this->SliceLogic->GetLowestVolumeSliceSpacing();
  Q_ASSERT(sliceSpacing);
  double offsetResolution = sliceSpacing[2];
  p->setSliceOffsetResolution(offsetResolution);

  // Set slice offset range to match the field of view
  // Calculate the number of slices in the current range
  double sliceBounds[6] = {0, 0, 0, 0, 0, 0};
  this->SliceLogic->GetLowestVolumeSliceBounds(sliceBounds);
  p->setSliceOffsetRange(sliceBounds[4], sliceBounds[5]);
}

// --------------------------------------------------------------------------
void qSlicerSliceViewWidgetPrivate::onImageDataModifiedEvent()
{
  CTK_P(qSlicerSliceViewWidget);

  logger.trace("onImageDataModifiedEvent");

  p->setImageData(this->ImageData);

  p->scheduleRender();
}

// --------------------------------------------------------------------------
// qSlicerSliceView methods

// --------------------------------------------------------------------------
qSlicerSliceViewWidget::qSlicerSliceViewWidget(const QString& sliceViewName, QWidget* _parent) :
    Superclass(sliceViewName, _parent)
{
  CTK_INIT_PRIVATE(qSlicerSliceViewWidget);
  //CTK_D(qSlicerSliceView);
}

//------------------------------------------------------------------------------
void qSlicerSliceViewWidget::setMRMLScene(vtkMRMLScene* scene)
{
  if (scene == this->mrmlScene())
    {
    return;
    }

  this->qMRMLSliceViewWidget::setMRMLScene(scene);
}

//------------------------------------------------------------------------------
void qSlicerSliceViewWidget::setMRMLSliceNode(vtkMRMLSliceNode* sliceNode)
{
  CTK_D(qSlicerSliceViewWidget);
  Q_ASSERT(this->mrmlScene());

  if (sliceNode == this->mrmlSliceNode())
    {
    return;
    }

  // List of events the slice logics should listen
  VTK_CREATE(vtkIntArray, events);
  events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
  events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosingEvent);
  events->InsertNextValue(vtkMRMLScene::SceneRestoredEvent);
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

  // Initialize logic
  d->SliceLogic->SetMRMLScene(this->mrmlScene());
  d->SliceLogic->SetSliceNode(sliceNode);
  d->SliceLogic->ProcessLogicEvents();
  d->SliceLogic->ProcessMRMLEvents(this->mrmlScene(), vtkCommand::ModifiedEvent, NULL);
  d->SliceLogic->SetAndObserveMRMLSceneEvents(this->mrmlScene(), events);

  this->qMRMLSliceViewWidget::setMRMLSliceNode(sliceNode);
}

// --------------------------------------------------------------------------
void qSlicerSliceViewWidget::onSliceOffsetValueChanged(double value)
{
  logger.trace(QString("onSliceOffsetValueChanged: %1").arg(value));
  ctk_d()->SliceLogic->SetSliceOffset(value);
}

/*
//------------------------------------------------------------------------------
void qSlicerSliceViewWidget::resizeEvent(QResizeEvent * event)
{
  CTK_D(qSlicerSliceViewWidget);

  this->qMRMLSliceViewWidget::resizeEvent(event);
}
*/


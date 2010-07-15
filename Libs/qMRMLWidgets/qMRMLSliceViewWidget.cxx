
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

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLSliceViewWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLSliceViewPrivate methods

//---------------------------------------------------------------------------
qMRMLSliceViewWidgetPrivate::qMRMLSliceViewWidgetPrivate()
{
  this->SliceLogic = vtkSmartPointer<vtkMRMLSliceLogic>::New();
  
  this->qvtkConnect(this->SliceLogic,
                    vtkCommand::ModifiedEvent, this, SLOT(onSliceLogicModifiedEvent()));
  
  this->MRMLSliceNode = 0;
  this->MRMLSliceCompositeNode = 0;

  this->SliceOrientationToDescription["Axial"]    = QLatin1String("I <-----> S");
  this->SliceOrientationToDescription["Sagittal"] = QLatin1String("L <-----> R");
  this->SliceOrientationToDescription["Coronal"]  = QLatin1String("P <----> A");
  this->SliceOrientationToDescription["Reformat"] = QLatin1String("Oblique");
}

//---------------------------------------------------------------------------
qMRMLSliceViewWidgetPrivate::~qMRMLSliceViewWidgetPrivate()
{
}

//---------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::setupUi(qMRMLWidget* widget)
{
  CTK_P(qMRMLSliceViewWidget);

  this->Ui_qMRMLSliceViewWidget::setupUi(widget);
  
  // Set LabelMapSelector attributes
  this->LabelMapSelector->addAttribute("vtkMRMLVolumeNode", "LabelMap", "1");

  // Connect Orientation selector
  this->connect(this->OrientationSelector, SIGNAL(currentIndexChanged(QString)),
                p, SLOT(setSliceOrientation(QString)));

  // Connect Foreground layer selector
  this->connect(this->ForegroundLayerNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                SLOT(onForegroundLayerNodeSelected(vtkMRMLNode*)));

  // Connect Background layer selector
  this->connect(this->BackgroundLayerNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                SLOT(onBackgroundLayerNodeSelected(vtkMRMLNode*)));

  // Connect Label map selector
  this->connect(this->LabelMapSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                SLOT(onLabelMapNodeSelected(vtkMRMLNode*)));

  // Connect Slice offset slider
  this->connect(this->SliceOffsetSlider, SIGNAL(valueIsChanging(double)),
                p, SLOT(setSliceOffsetValue(double)));
                
}

//---------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  CTK_P(qMRMLSliceViewWidget);
  if (newScene == p->mrmlScene())
    {
    return; 
    }
    
  this->qvtkReconnect(
    p->mrmlScene(), newScene,
    vtkMRMLScene::SceneAboutToBeClosedEvent, this, SLOT(onSceneAboutToBeClosedEvent()));

  this->qvtkReconnect(
    p->mrmlScene(), newScene,
    vtkMRMLScene::SceneClosedEvent, this, SLOT(onSceneClosedEvent()));

  this->qvtkReconnect(
    p->mrmlScene(), newScene,
    vtkMRMLScene::SceneAboutToBeImportedEvent, this, SLOT(onSceneAboutToBeImportedEvent()));

  this->qvtkReconnect(
    p->mrmlScene(), newScene,
    vtkMRMLScene::SceneImportedEvent, this, SLOT(onSceneImportedEvent()));
    
  this->qvtkReconnect(
    p->mrmlScene(), newScene,
    vtkMRMLScene::NodeAddedEvent, this,
    SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));
                      
  this->qvtkReconnect(
    p->mrmlScene(), newScene,
    vtkMRMLScene::NodeRemovedEvent, this,
    SLOT(onNodeRemovedEvent(vtkObject*, vtkObject*)));

  this->qvtkReconnect(
    p->mrmlScene(), newScene,
    vtkMRMLScene::SceneRestoredEvent, this, SLOT(onSceneRestoredEvent()));

  p->setMRMLScene(newScene);
}

//---------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode)
{
  CTK_P(qMRMLSliceViewWidget);
  
  if (newSliceNode == this->MRMLSliceNode)
    {
    return; 
    }
    
  // List of events the slice logics should listen
  VTK_CREATE(vtkIntArray, events);
  events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneAboutToBeClosedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneRestoredEvent);
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

  // Initialize logic
  this->SliceLogic->SetMRMLScene(p->mrmlScene());
  this->SliceLogic->SetSliceNode(newSliceNode);
  this->SliceLogic->SetName(this->SliceViewName.toLatin1());
  this->SliceLogic->ProcessLogicEvents();
  this->SliceLogic->ProcessMRMLEvents(p->mrmlScene(), vtkCommand::ModifiedEvent, NULL);
  this->SliceLogic->SetAndObserveMRMLSceneEvents(p->mrmlScene(), events);

  this->qvtkReconnect(this->MRMLSliceNode, newSliceNode, vtkCommand::ModifiedEvent, 
                      this, SLOT(updateWidgetFromMRMLSliceNode()));
    
  this->MRMLSliceNode = newSliceNode;

  // Update widget state given the new node
  this->updateWidgetFromMRMLSliceNode();
  
}

//---------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::setImageData(vtkImageData* newImageData)
{
  if (this->ImageData == newImageData)
    {
    return;
    }

  logger.trace("setImageData");
  
  this->qvtkReconnect(this->ImageData, newImageData,
                      vtkCommand::ModifiedEvent, this, SLOT(onImageDataModifiedEvent()));

  this->ImageData = newImageData;

  // Since new layers have been associated with the current MRML Slice Node,
  // let's update the widget state to reflect these changes
  //this->updateWidgetFromMRMLSliceNode();
  
  this->onImageDataModifiedEvent();
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::setupMoreOptionMenu()
{
  QMenu * menu = new QMenu(this->SliceMoreOptionButton);
  Q_UNUSED(menu);
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onSceneAboutToBeClosedEvent()
{
  qDebug() << "qMRMLSliceViewPrivate::onSceneAboutToBeClosedEvent";
  this->VTKSliceView->setRenderEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onSceneClosedEvent()
{
  qDebug() << "qMRMLSliceViewPrivate::onSceneClosedEvent";
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onSceneAboutToBeImportedEvent()
{
  qDebug() << "qMRMLSliceViewPrivate::onSceneAboutToBeImportedEvent";
  this->VTKSliceView->setRenderEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onSceneImportedEvent()
{
  qDebug() << "qMRMLSliceViewPrivate::onSceneImportedEvent";
  this->VTKSliceView->setRenderEnabled(true);
  this->VTKSliceView->scheduleRender();
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onSceneRestoredEvent()
{
  qDebug() << "qMRMLSliceViewPrivate::onSceneRestoredEvent";
}
  
// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onNodeAddedEvent(vtkObject* scene, vtkObject* node)
{
  Q_ASSERT(scene);
  
  vtkMRMLNode * mrmlNode = vtkMRMLNode::SafeDownCast(node);
  Q_ASSERT(mrmlNode);
  if (!mrmlNode)
    {
    return;
    }

  qDebug() << "qMRMLSliceViewPrivate::onNodeAddedEvent=>" << mrmlNode->GetClassName();
  
//   if (mrmlNode->IsA("vtkMRMLDisplayableNode"))
//     {
//     }
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onNodeRemovedEvent(vtkObject* scene, vtkObject* node)
{
  Q_ASSERT(scene);
  
  vtkMRMLNode * mrmlNode = vtkMRMLNode::SafeDownCast(node);
  Q_ASSERT(mrmlNode);
  if (!mrmlNode)
    {
    return;
    }

  qDebug() << "qMRMLSliceViewPrivate::onNodeRemovedEvent=>" << mrmlNode->GetClassName();
  
//   if (mrmlNode->IsA("vtkMRMLDisplayableNode"))
//     {
//     }
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::updateWidgetFromMRMLSliceNode()
{
  CTK_P(qMRMLSliceViewWidget);
  Q_ASSERT(this->MRMLSliceCompositeNode);

  logger.trace("updateWidgetFromMRMLSliceNode");

  // Update orientation selector state
  int index = this->OrientationSelector->findText(
      QString::fromStdString(this->MRMLSliceNode->GetOrientationString()));
  Q_ASSERT(index>=0 && index <=4);
  this->OrientationSelector->setCurrentIndex(index);

  // Update slice offset slider tooltip
  this->SliceOffsetSlider->setToolTip(
      this->SliceOrientationToDescription[
          QString::fromStdString(this->MRMLSliceNode->GetOrientationString())]);

  // Update "foreground layer" node selector
  this->ForegroundLayerNodeSelector->setCurrentNode(
      p->mrmlScene()->GetNodeByID(this->MRMLSliceCompositeNode->GetForegroundVolumeID()));

  // Update "background layer" node selector
  this->BackgroundLayerNodeSelector->setCurrentNode(
      p->mrmlScene()->GetNodeByID(this->MRMLSliceCompositeNode->GetBackgroundVolumeID()));

  // Update "label map" node selector
  this->LabelMapSelector->setCurrentNode(
      p->mrmlScene()->GetNodeByID(this->MRMLSliceCompositeNode->GetLabelVolumeID()));
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onForegroundLayerNodeSelected(vtkMRMLNode * node)
{
  CTK_P(qMRMLSliceViewWidget);
  logger.trace(QString("sliceView: %1 - ForegroundLayerNodeSelected").arg(p->sliceOrientation()));

  if (!this->MRMLSliceCompositeNode)
    {
    return;
    }
  this->MRMLSliceCompositeNode->SetForegroundVolumeID(node ? node->GetID() : 0);
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onBackgroundLayerNodeSelected(vtkMRMLNode * node)
{
  CTK_P(qMRMLSliceViewWidget);
  logger.trace(QString("sliceView: %1 - BackgroundLayerNodeSelected").arg(p->sliceOrientation()));

  if (!this->MRMLSliceCompositeNode)
    {
    return;
    }
  this->MRMLSliceCompositeNode->SetBackgroundVolumeID(node ? node->GetID() : 0);
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onLabelMapNodeSelected(vtkMRMLNode * node)
{
  CTK_P(qMRMLSliceViewWidget);
  logger.trace(QString("sliceView: %1 - LabelMapNodeSelected").arg(p->sliceOrientation()));

  if (!this->MRMLSliceCompositeNode)
    {
    return;
    }
  this->MRMLSliceCompositeNode->SetLabelVolumeID(node ? node->GetID() : 0);
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onSliceLogicModifiedEvent()
{
  CTK_P(qMRMLSliceViewWidget);

  if (p->mrmlSliceCompositeNode() != this->SliceLogic->GetSliceCompositeNode())
    {
    p->setMRMLSliceCompositeNode(this->SliceLogic->GetSliceCompositeNode());
    }

  if (this->ImageData != this->SliceLogic->GetImageData())
    {
    logger.trace("onSliceLogicModifiedEvent - reconnect onImageDataModifiedEvent slot");
    p->setImageData(this->SliceLogic->GetImageData());
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
void qMRMLSliceViewWidgetPrivate::onImageDataModifiedEvent()
{
  logger.trace("onImageDataModifiedEvent");
  this->VTKSliceView->setImageData(this->ImageData); 
  //ctk_p()->fitSliceToBackground();
  this->VTKSliceView->scheduleRender();
}

// --------------------------------------------------------------------------
// qMRMLSliceView methods

// --------------------------------------------------------------------------
qMRMLSliceViewWidget::qMRMLSliceViewWidget(const QString& _sliceViewName, QWidget* _parent) :
    Superclass(_parent)
{
  CTK_INIT_PRIVATE(qMRMLSliceViewWidget);
  CTK_D(qMRMLSliceViewWidget);
  d->SliceViewName = _sliceViewName;
  d->setupUi(this);
  d->PushButtonSliceViewName->setText(_sliceViewName);
}

//------------------------------------------------------------------------------
void qMRMLSliceViewWidget::setMRMLScene(vtkMRMLScene* scene)
{
  this->qMRMLWidget::setMRMLScene(scene);
}

//---------------------------------------------------------------------------
void qMRMLSliceViewWidget::setMRMLSliceNode(vtkMRMLSliceNode* sliceNode)
{
  ctk_d()->setMRMLSliceNode(sliceNode);
}

//---------------------------------------------------------------------------
CTK_SET_CXX(qMRMLSliceViewWidget, vtkMRMLSliceCompositeNode*,
            setMRMLSliceCompositeNode, MRMLSliceCompositeNode);
CTK_GET_CXX(qMRMLSliceViewWidget, vtkMRMLSliceCompositeNode*,
            mrmlSliceCompositeNode, MRMLSliceCompositeNode);

//---------------------------------------------------------------------------
CTK_GET_CXX(qMRMLSliceViewWidget, QString, sliceViewName, SliceViewName);

//---------------------------------------------------------------------------
void qMRMLSliceViewWidget::setSliceOffsetRange(double min, double max)
{
  ctk_d()->SliceOffsetSlider->setRange(min, max);
}

//---------------------------------------------------------------------------
void qMRMLSliceViewWidget::setSliceOffsetResolution(double resolution)
{
  ctk_d()->SliceOffsetSlider->setSingleStep(resolution);
}

//---------------------------------------------------------------------------
void qMRMLSliceViewWidget::setImageData(vtkImageData* newImageData)
{
  ctk_d()->setImageData(newImageData);
}

//---------------------------------------------------------------------------
CTK_GET_CXX(qMRMLSliceViewWidget, vtkImageData*, imageData, ImageData);
CTK_GET_CXX(qMRMLSliceViewWidget, vtkMRMLSliceNode*, mrmlSliceNode, MRMLSliceNode);

// --------------------------------------------------------------------------
void qMRMLSliceViewWidget::setSliceOffsetValue(double value)
{
  CTK_D(qMRMLSliceViewWidget);
  logger.trace(QString("setSliceOffsetValue: %1").arg(value));
  d->SliceLogic->SetSliceOffset(value);
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidget::fitSliceToBackground()
{
  CTK_D(qMRMLSliceViewWidget);
  Q_ASSERT(d->SliceLogic->GetSliceNode());
  int width = d->VTKSliceView->size().width();
  int height = d->VTKSliceView->size().height();
  logger.trace(QString("fitSliceToBackground - size(%1, %2)").arg(width).arg(height));
  d->SliceLogic->FitSliceToAll(width, height);
  d->SliceLogic->GetSliceNode()->UpdateMatrices();
}

//---------------------------------------------------------------------------
QString qMRMLSliceViewWidget::sliceOrientation()
{
  return ctk_d()->OrientationSelector->currentText();
}

//---------------------------------------------------------------------------
void qMRMLSliceViewWidget::setSliceOrientation(const QString& orientation)
{
  CTK_D(qMRMLSliceViewWidget);

#ifndef QT_NO_DEBUG
  QStringList expectedOrientation;
  expectedOrientation << "Axial" << "Sagittal" << "Coronal" << "Reformat";
  Q_ASSERT(expectedOrientation.contains(orientation));
#endif

  d->MRMLSliceNode->SetOrientationString(orientation.toLatin1());
}

// --------------------------------------------------------------------------
CTK_GET_CXX(qMRMLSliceViewWidget, ctkVTKSliceView*, sliceView, VTKSliceView);

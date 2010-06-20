
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
  logger.setTrace();

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
                p, SLOT(onSliceOffsetValueChanged(double)));
}

//---------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::setMRMLScene(vtkMRMLScene* scene)
{
  CTK_P(qMRMLSliceViewWidget);
  if (scene == p->mrmlScene())
    {
    return; 
    }
    
  this->qvtkReconnect(
    p->mrmlScene(), scene,
    vtkMRMLScene::SceneClosingEvent, this, SLOT(onSceneClosingEvent()));

  this->qvtkReconnect(
    p->mrmlScene(), scene,
    vtkMRMLScene::SceneCloseEvent, this, SLOT(onSceneCloseEvent()));

  this->qvtkReconnect(
    p->mrmlScene(), scene,
    vtkMRMLScene::SceneLoadStartEvent, this, SLOT(onSceneLoadStartEvent()));

  this->qvtkReconnect(
    p->mrmlScene(), scene,
    vtkMRMLScene::SceneLoadEndEvent, this, SLOT(onSceneLoadEndEvent()));
    
  this->qvtkReconnect(
    p->mrmlScene(), scene,
    vtkMRMLScene::NodeAddedEvent, this,
    SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));
                      
  this->qvtkReconnect(
    p->mrmlScene(), scene,
    vtkMRMLScene::NodeRemovedEvent, this,
    SLOT(onNodeRemovedEvent(vtkObject*, vtkObject*)));

  this->qvtkReconnect(
    p->mrmlScene(), scene,
    vtkMRMLScene::SceneRestoredEvent, this, SLOT(onSceneRestoredEvent()));

  p->setMRMLScene(scene);
}

//---------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::setMRMLSliceNode(vtkMRMLSliceNode* sliceNode)
{
  if (sliceNode == this->MRMLSliceNode)
    {
    return; 
    }

  this->qvtkReconnect(this->MRMLSliceNode, sliceNode, vtkCommand::ModifiedEvent, this,
                      SLOT(onMRMLSliceNodeModifiedEvent()));
    
  this->MRMLSliceNode = sliceNode;
  
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::setupMoreOptionMenu()
{
  QMenu * menu = new QMenu(this->SliceMoreOptionButton);
  Q_UNUSED(menu);
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onSceneClosingEvent()
{
  qDebug() << "qMRMLSliceViewPrivate::onSceneClosingEvent";
  this->VTKSliceView->setRenderEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onSceneCloseEvent()
{
  qDebug() << "qMRMLSliceViewPrivate::onSceneCloseEvent";
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onSceneLoadStartEvent()
{
  qDebug() << "qMRMLSliceViewPrivate::onSceneLoadStartEvent";
  this->VTKSliceView->setRenderEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidgetPrivate::onSceneLoadEndEvent()
{
  qDebug() << "qMRMLSliceViewPrivate::onSceneLoadEndEvent";
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
void qMRMLSliceViewWidgetPrivate::onMRMLSliceNodeModifiedEvent()
{
  CTK_P(qMRMLSliceViewWidget);
  Q_ASSERT(this->MRMLSliceCompositeNode);

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
  CTK_D(qMRMLSliceViewWidget);
  if (d->ImageData == newImageData)
    {
    return;
    }

  d->VTKSliceView->setImageData(newImageData);

  d->ImageData = newImageData;
}

//---------------------------------------------------------------------------
CTK_GET_CXX(qMRMLSliceViewWidget, vtkImageData*, imageData, ImageData);
CTK_GET_CXX(qMRMLSliceViewWidget, vtkMRMLSliceNode*, mrmlSliceNode, MRMLSliceNode);

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
  expectedOrientation << "Axial" << "Sagital" << "Coronal" << "Reformat";
  Q_ASSERT(expectedOrientation.contains(orientation));
#endif

  d->MRMLSliceNode->SetOrientationString(orientation.toLatin1());
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidget::scheduleRender()
{
  ctk_d()->VTKSliceView->scheduleRender();
}

// --------------------------------------------------------------------------
void qMRMLSliceViewWidget::onSliceOffsetValueChanged(double value)
{
  Q_UNUSED(value);
}

// --------------------------------------------------------------------------
CTK_GET_CXX(qMRMLSliceViewWidget, ctkVTKSliceView*, sliceView, VTKSliceView);

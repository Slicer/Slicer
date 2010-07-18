
// Qt includes
#include <QDebug>
#include <QMenu>
#include <QProxyStyle>

// CTK includes
#include <ctkVTKSliceView.h>
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLSliceControllerWidget.h"
#include "qMRMLSliceControllerWidget_p.h"

// MRML includes
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkImageData.h>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLSliceControllerWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLSliceViewPrivate methods

//---------------------------------------------------------------------------
qMRMLSliceControllerWidgetPrivate::qMRMLSliceControllerWidgetPrivate()
{
  this->SliceLogic = vtkSmartPointer<vtkMRMLSliceLogic>::New();
  
  this->qvtkConnect(this->SliceLogic,
                    vtkCommand::ModifiedEvent, this, SLOT(onSliceLogicModifiedEvent()));
  
  this->MRMLSliceNode = 0;
  this->MRMLSliceCompositeNode = 0;

  this->SliceOrientation = "Axial";

  this->SliceOrientationToDescription["Axial"]    = QLatin1String("I <-----> S");
  this->SliceOrientationToDescription["Sagittal"] = QLatin1String("L <-----> R");
  this->SliceOrientationToDescription["Coronal"]  = QLatin1String("P <----> A");
  this->SliceOrientationToDescription["Reformat"] = QLatin1String("Oblique");
}

//---------------------------------------------------------------------------
qMRMLSliceControllerWidgetPrivate::~qMRMLSliceControllerWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
namespace
{
class qMRMLSliceCollapsibleButtonStyle:public QProxyStyle
{
public:
  virtual void drawControl(ControlElement ce, const QStyleOption * opt,
                           QPainter * p, const QWidget * widget = 0) const
  {
    this->QProxyStyle::drawControl(ce, opt, p, widget);
    this->QProxyStyle::drawPrimitive(QStyle::PE_IndicatorArrowUp, opt, p, widget);
  }
};
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupUi(qMRMLWidget* widget)
{
  CTK_P(qMRMLSliceControllerWidget);

  this->Ui_qMRMLSliceControllerWidget::setupUi(widget);

  // Set a ProxyStyle responsible for drawing the arrow
  //this->SliceCollapsibleButton->setStyle(new qMRMLSliceCollapsibleButtonStyle);
  
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

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::onImageDataModifiedEvent()
{
  CTK_P(qMRMLSliceControllerWidget);
  logger.trace("onImageDataModifiedEvent");
  emit p->imageDataModified(this->ImageData);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupMoreOptionMenu()
{
  QMenu * menu = new QMenu(this->SliceMoreOptionButton);
  Q_UNUSED(menu);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::updateWidgetFromMRMLSliceNode()
{
  CTK_P(qMRMLSliceControllerWidget);
  Q_ASSERT(this->MRMLSliceNode);
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
void qMRMLSliceControllerWidgetPrivate::onForegroundLayerNodeSelected(vtkMRMLNode * node)
{
  CTK_P(qMRMLSliceControllerWidget);
  logger.trace(QString("sliceView: %1 - ForegroundLayerNodeSelected").arg(p->sliceOrientation()));

  if (!this->MRMLSliceCompositeNode)
    {
    return;
    }
  this->MRMLSliceCompositeNode->SetForegroundVolumeID(node ? node->GetID() : 0);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::onBackgroundLayerNodeSelected(vtkMRMLNode * node)
{
  CTK_P(qMRMLSliceControllerWidget);
  logger.trace(QString("sliceView: %1 - BackgroundLayerNodeSelected").arg(p->sliceOrientation()));

  if (!this->MRMLSliceCompositeNode)
    {
    return;
    }
  this->MRMLSliceCompositeNode->SetBackgroundVolumeID(node ? node->GetID() : 0);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::onLabelMapNodeSelected(vtkMRMLNode * node)
{
  CTK_P(qMRMLSliceControllerWidget);
  logger.trace(QString("sliceView: %1 - LabelMapNodeSelected").arg(p->sliceOrientation()));

  if (!this->MRMLSliceCompositeNode)
    {
    return;
    }
  this->MRMLSliceCompositeNode->SetLabelVolumeID(node ? node->GetID() : 0);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::onSliceLogicModifiedEvent()
{
  CTK_P(qMRMLSliceControllerWidget);

  if (this->MRMLSliceCompositeNode != this->SliceLogic->GetSliceCompositeNode())
    {
    this->MRMLSliceCompositeNode = this->SliceLogic->GetSliceCompositeNode();
    }

  if (this->ImageData != this->SliceLogic->GetImageData())
    {
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
// qMRMLSliceView methods

// --------------------------------------------------------------------------
qMRMLSliceControllerWidget::qMRMLSliceControllerWidget(QWidget* _parent) : Superclass(_parent)
{
  CTK_INIT_PRIVATE(qMRMLSliceControllerWidget);
  CTK_D(qMRMLSliceControllerWidget);
  d->setupUi(this);
  this->setSliceViewName("Red");
}

//---------------------------------------------------------------------------
CTK_GET_CXX(qMRMLSliceControllerWidget, vtkMRMLSliceNode*, mrmlSliceNode, MRMLSliceNode);

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode)
{
  CTK_D(qMRMLSliceControllerWidget);

  if (newSliceNode == d->MRMLSliceNode)
    {
    return;
    }

  // Enable/disable widget
  this->setDisabled(newSliceNode == 0);

  // Initialize logic
  if (!d->SliceLogic->IsInitialized())
    {
    d->SliceLogic->Initialize(d->SliceViewName.toLatin1(), this->mrmlScene(), newSliceNode);
    }

  d->MRMLSliceCompositeNode = d->SliceLogic->GetSliceCompositeNode();

  d->qvtkReconnect(d->MRMLSliceNode, newSliceNode, vtkCommand::ModifiedEvent,
                   d, SLOT(updateWidgetFromMRMLSliceNode()));

  d->MRMLSliceNode = newSliceNode;

  // Update widget state given the new node
  if (d->MRMLSliceNode)
    {
    d->updateWidgetFromMRMLSliceNode();
    }
}

//---------------------------------------------------------------------------
CTK_GET_CXX(qMRMLSliceControllerWidget, vtkMRMLSliceLogic*, sliceLogic, SliceLogic);

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceLogic(vtkMRMLSliceLogic * newSliceLogic)
{
  CTK_D(qMRMLSliceControllerWidget);
  if (d->SliceLogic == newSliceLogic)
    {
    return;
    }

  d->SliceLogic = newSliceLogic;

  this->setMRMLSliceNode(d->SliceLogic->GetSliceNode());
}

//---------------------------------------------------------------------------
CTK_GET_CXX(qMRMLSliceControllerWidget, vtkMRMLSliceCompositeNode*,
            mrmlSliceCompositeNode, MRMLSliceCompositeNode);

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceViewSize(const QSize& newSize)
{
  CTK_D(qMRMLSliceControllerWidget);
  if (d->VTKSliceViewSize.isNull() || d->VTKSliceViewSize == newSize)
    {
    return;
    }
  logger.trace(QString("setSliceViewSize - newSize(%1, %2)").
               arg(newSize.width()).arg(newSize.height()));
  d->VTKSliceViewSize = newSize;
  this->fitSliceToBackground();
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceViewName(const QString& newSliceViewName)
{
  CTK_D(qMRMLSliceControllerWidget);

  if (d->MRMLSliceNode)
    {
    logger.error("setSliceViewName should be called before setMRMLSliceNode !");
    return;
    }

  if (d->SliceViewName == newSliceViewName)
    {
    return;
    }

  QPalette palette;
  // If name matches either 'Red, 'Green' or 'Yellow' set the corresponding color
  // set Orange otherwise
  if (newSliceViewName == "Red")
    {
    QColor red;
    red.setRgbF(0.952941176471, 0.290196078431, 0.2);
    palette = QPalette(red);
    }
  else if (newSliceViewName == "Green")
    {
    QColor green;
    green.setRgbF(0.43137254902, 0.690196078431, 0.294117647059);
    palette = QPalette(green);
    }
  else if (newSliceViewName == "Yellow")
    {
    QColor yellow;
    yellow.setRgbF(0.929411764706, 0.835294117647, 0.298039215686);
    palette = QPalette(yellow);
    }
  else
    {
    // Default slice view color
    QColor orange;
    orange.setRgbF(0.882352941176, 0.439215686275, 0.0705882352941);
    palette = QPalette(orange);
    }
  d->SliceCollapsibleButton->setPalette(palette);

  if (d->SliceLogic)
    {
    d->SliceLogic->SetName(newSliceViewName.toLatin1());
    }

  d->SliceViewName = newSliceViewName;
}

//---------------------------------------------------------------------------
CTK_GET_CXX(qMRMLSliceControllerWidget, QString, sliceViewName, SliceViewName);

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setImageData(vtkImageData* newImageData)
{
  CTK_D(qMRMLSliceControllerWidget);

  if (d->ImageData == newImageData)
    {
    return;
    }

  logger.trace("setImageData - Reconnect ModifiedEvent on ImageData");

  d->qvtkReconnect(d->ImageData, newImageData,
                   vtkCommand::ModifiedEvent, d, SLOT(onImageDataModifiedEvent()));

  d->ImageData = newImageData;

  // Since new layers have been associated with the current MRML Slice Node,
  // let's update the widget state to reflect these changes
  //this->updateWidgetFromMRMLSliceNode();

  d->onImageDataModifiedEvent();
}

//---------------------------------------------------------------------------
CTK_GET_CXX(qMRMLSliceControllerWidget, vtkImageData*, imageData, ImageData);

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceOffsetRange(double min, double max)
{
  ctk_d()->SliceOffsetSlider->setRange(min, max);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceOffsetResolution(double resolution)
{
  ctk_d()->SliceOffsetSlider->setSingleStep(resolution);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceOffsetValue(double value)
{
  CTK_D(qMRMLSliceControllerWidget);
  logger.trace(QString("setSliceOffsetValue: %1").arg(value));
  d->SliceLogic->SetSliceOffset(value);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::fitSliceToBackground()
{
  CTK_D(qMRMLSliceControllerWidget);
  if (!d->SliceLogic->GetSliceNode())
    {
    logger.warn("fitSliceToBackground - Failed because SliceLogic->GetSliceNode() is NULL");
    return;
    }
  int width = d->VTKSliceViewSize.width();
  int height = d->VTKSliceViewSize.height();
  logger.trace(QString("fitSliceToBackground - size(%1, %2)").arg(width).arg(height));
  d->SliceLogic->SetSliceViewSize(width, height);
  d->SliceLogic->FitSliceToAll();
  d->SliceLogic->GetSliceNode()->UpdateMatrices();
}

//---------------------------------------------------------------------------
QString qMRMLSliceControllerWidget::sliceOrientation()
{
  return ctk_d()->OrientationSelector->currentText();
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceOrientation(const QString& orientation)
{
  CTK_D(qMRMLSliceControllerWidget);

#ifndef QT_NO_DEBUG
  QStringList expectedOrientation;
  expectedOrientation << "Axial" << "Sagittal" << "Coronal" << "Reformat";
  Q_ASSERT(expectedOrientation.contains(orientation));
#endif

  if (d->MRMLSliceNode)
    {
    d->MRMLSliceNode->SetOrientationString(orientation.toLatin1());
    }
}

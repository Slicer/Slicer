#include "qSlicerScalarVolumeDisplayWidget.h"
#include "ui_qSlicerScalarVolumeDisplayWidget.h"

// Qt includes
#include <QDebug>

// CTK includes
#include <ctkVTKColorTransferFunction.h>
#include <ctkTransferFunctionGradientItem.h>
#include <ctkTransferFunctionScene.h>
#include <ctkTransferFunctionBarsItem.h>
#include <ctkVTKHistogram.h>

// MRML includes
#include "vtkMRMLScalarVolumeNode.h"

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerScalarVolumeDisplayWidgetPrivate: public ctkPrivate<qSlicerScalarVolumeDisplayWidget>,
                                          public Ui_qSlicerScalarVolumeDisplayWidget
{
public:
  qSlicerScalarVolumeDisplayWidgetPrivate();
  ~qSlicerScalarVolumeDisplayWidgetPrivate();
  void init();
  void updateTransferFunction();
  ctkVTKHistogram* Histogram;
  vtkSmartPointer<vtkColorTransferFunction> ColorTransferFunction;
};

//-----------------------------------------------------------------------------
qSlicerScalarVolumeDisplayWidgetPrivate::qSlicerScalarVolumeDisplayWidgetPrivate()
{
  this->Histogram = new ctkVTKHistogram();
  this->ColorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
}

//-----------------------------------------------------------------------------
qSlicerScalarVolumeDisplayWidgetPrivate::~qSlicerScalarVolumeDisplayWidgetPrivate()
{
  delete this->Histogram;
}
//-----------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidgetPrivate::init()
{
  CTK_P(qSlicerScalarVolumeDisplayWidget);

  this->setupUi(p);

  ctkTransferFunctionScene* scene = qobject_cast<ctkTransferFunctionScene*>(
    this->TransferFunctionWidget->scene());
  // Transfer Function
  ctkVTKColorTransferFunction* transferFunction =
    new ctkVTKColorTransferFunction(this->ColorTransferFunction);

  ctkTransferFunctionGradientItem* gradientItem =
    new ctkTransferFunctionGradientItem(transferFunction);
  scene->addItem(gradientItem);
  // Histogram
  //scene->setTransferFunction(this->Histogram);
  ctkTransferFunctionBarsItem* barsItem =
    new ctkTransferFunctionBarsItem(this->Histogram);
  barsItem->setBarWidth(1.);
  QColor highlight = p->palette().color(QPalette::Normal, QPalette::Highlight);
  //highlight.setAlphaF(0.5); not correctly supported... strange
  barsItem->setBarColor(highlight);
  scene->addItem(barsItem);
}

// --------------------------------------------------------------------------
qSlicerScalarVolumeDisplayWidget::qSlicerScalarVolumeDisplayWidget(QWidget* _parent) : Superclass(_parent)
{
  CTK_INIT_PRIVATE(qSlicerScalarVolumeDisplayWidget);
  CTK_D(qSlicerScalarVolumeDisplayWidget);
  d->init();

  // disable as there is not MRML Node associated with the widget
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::setMRMLVolumeNode(vtkMRMLNode* node)
{
  this->setMRMLVolumeNode(vtkMRMLScalarVolumeNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::setMRMLVolumeNode(vtkMRMLScalarVolumeNode* volumeNode)
{
  CTK_D(qSlicerScalarVolumeDisplayWidget);
  d->MRMLWindowLevelWidget->setMRMLVolumeNode(volumeNode);
  d->MRMLVolumeThresholdWidget->setMRMLVolumeNode(volumeNode);
  d->Histogram->setDataArray(volumeNode->GetImageData()->GetPointData()->GetScalars());
  d->Histogram->build();
  d->updateTransferFunction();
  this->setEnabled(volumeNode != 0);
}

//----------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidgetPrivate::updateTransferFunction()
{
  vtkMRMLVolumeNode* volumeNode = this->MRMLWindowLevelWidget->mrmlVolumeNode();
  Q_ASSERT(volumeNode == this->MRMLVolumeThresholdWidget->mrmlVolumeNode());
  vtkImageData* imageData = volumeNode ? volumeNode->GetImageData() : 0;
  if (imageData == 0)
    {
    this->ColorTransferFunction->RemoveAllPoints();
    return;
    }
  double range[2] = {0,255};
  imageData->GetScalarRange(range);
  // AdjustRange call will take out points that are outside of the new
  // range, but it needs the points to be there in order to work, so call
  // RemoveAllPoints after it's done
  this->ColorTransferFunction->AdjustRange(range);
  this->ColorTransferFunction->RemoveAllPoints();

  double low   = this->MRMLVolumeThresholdWidget->lowerThreshold();
  double upper = this->MRMLVolumeThresholdWidget->upperThreshold();
  double min = this->MRMLWindowLevelWidget->level() - 0.5 * this->MRMLWindowLevelWidget->window();
  double max = this->MRMLWindowLevelWidget->level() + 0.5 * this->MRMLWindowLevelWidget->window();
  double minVal = 0;
  double maxVal = 1;

  this->ColorTransferFunction->SetColorSpaceToRGB();

  if (low >= max || upper <= min)
    {
    this->ColorTransferFunction->AddRGBPoint(range[0], 0, 0, 0);
    this->ColorTransferFunction->AddRGBPoint(range[1], 0, 0, 0);
    }
  else
    {
    max = qMax(min+0.001, max);
    low = qMax(range[0] + 0.001, low);
    min = qMax(range[0] + 0.001, min);
    upper = qMin(range[1] - 0.001, upper);

    if (min <= low)
      {
      minVal = (low - min)/(max - min);
      min = low + 0.001;
      }

    if (max >= upper)
      {
      maxVal = (upper - min)/(max-min);
      max = upper - 0.001;
      }

    this->ColorTransferFunction->AddRGBPoint(range[0], 0, 0, 0);
    this->ColorTransferFunction->AddRGBPoint(low, 0, 0, 0);
    this->ColorTransferFunction->AddRGBPoint(min, minVal, minVal, minVal);
    this->ColorTransferFunction->AddRGBPoint(max, maxVal, maxVal, maxVal);
    this->ColorTransferFunction->AddRGBPoint(upper, maxVal, maxVal, maxVal);
    if (upper+0.001 < range[1])
      {
      this->ColorTransferFunction->AddRGBPoint(upper+0.001, 0, 0, 0);
      this->ColorTransferFunction->AddRGBPoint(range[1], 0, 0, 0);
      }
    }

  this->ColorTransferFunction->SetAlpha(1.0);
  this->ColorTransferFunction->Build();
}

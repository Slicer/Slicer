#include "qSlicerScalarVolumeDisplayWidget.h"
#include "ui_qSlicerScalarVolumeDisplayWidget.h"

// Qt includes
#include <QDebug>

// CTK includes
#include <ctkTransferFunctionScene.h>
#include <ctkTransferFunctionBarsItem.h>
#include <ctkVTKHistogram.h>

// MRML includes
#include "vtkMRMLScalarVolumeNode.h"

// VTK includes
#include <vtkPointData.h>

//-----------------------------------------------------------------------------
class qSlicerScalarVolumeDisplayWidgetPrivate: public ctkPrivate<qSlicerScalarVolumeDisplayWidget>,
                                          public Ui_qSlicerScalarVolumeDisplayWidget
{
public:
  qSlicerScalarVolumeDisplayWidgetPrivate();
  ~qSlicerScalarVolumeDisplayWidgetPrivate();
  void init();
  ctkVTKHistogram* Histogram;
};

//-----------------------------------------------------------------------------
qSlicerScalarVolumeDisplayWidgetPrivate::qSlicerScalarVolumeDisplayWidgetPrivate()
{
  this->Histogram = new ctkVTKHistogram();
}

//-----------------------------------------------------------------------------
qSlicerScalarVolumeDisplayWidgetPrivate::~qSlicerScalarVolumeDisplayWidgetPrivate()
{
  delete this->Histogram;
  this->Histogram = 0;
}
//-----------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidgetPrivate::init()
{
  CTK_P(qSlicerScalarVolumeDisplayWidget);
  this->setupUi(p);
  qobject_cast<ctkTransferFunctionScene*>(this->TransferFunctionWidget->scene())
    ->setTransferFunction(this->Histogram);
  ctkTransferFunctionBarsItem* barsItem = 
    new ctkTransferFunctionBarsItem(this->Histogram);
  barsItem->setBarWidth(1.);
  this->TransferFunctionWidget->scene()->addItem(barsItem);
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
  this->setEnabled(volumeNode != 0);
}

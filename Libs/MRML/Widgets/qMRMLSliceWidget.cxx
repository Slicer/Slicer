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

// qMRML includes
#include "qMRMLSliceWidget_p.h"

// MRMLDisplayableManager includes
#include <vtkSliceViewInteractorStyle.h>

// MRML includes
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkCollection.h>
#include <vtkWeakPointer.h>


//--------------------------------------------------------------------------
// qMRMLSliceWidgetPrivate methods

//---------------------------------------------------------------------------
qMRMLSliceWidgetPrivate::qMRMLSliceWidgetPrivate(qMRMLSliceWidget& object)
  : q_ptr(&object)
{
}

//---------------------------------------------------------------------------
qMRMLSliceWidgetPrivate::~qMRMLSliceWidgetPrivate()
{
}

//---------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::init()
{
  Q_Q(qMRMLSliceWidget);
  this->setupUi(q);

  this->SliceView->sliceViewInteractorStyle()
    ->SetSliceLogic(this->SliceController->sliceLogic());

  connect(this->SliceView, SIGNAL(resized(QSize)),
          this->SliceController, SLOT(setSliceViewSize(QSize)));

  connect(this->SliceController, SIGNAL(imageDataChanged(vtkImageData*)),
          this, SLOT(setImageData(vtkImageData*)));
  connect(this->SliceController, SIGNAL(renderRequested()),
          this->SliceView, SLOT(scheduleRender()), Qt::QueuedConnection);
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::endProcessing()
{
  // When a scene is closed, we need to reconfigure the SliceNode to
  // the size of the widget.
  QRect rect = this->SliceView->geometry();
  this->SliceController->setSliceViewSize(QSize(rect.width(), rect.height()));
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::setImageData(vtkImageData * imageData)
{
  //qDebug() << "qMRMLSliceWidgetPrivate::setImageData";
  this->SliceView->setImageData(imageData);
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

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLSliceWidget);

  this->Superclass::setMRMLScene(newScene);

  d->qvtkReconnect(
    this->mrmlScene(), newScene,
    vtkMRMLScene::EndBatchProcessEvent, d, SLOT(endProcessing()));
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode)
{
  Q_D(qMRMLSliceWidget);

  d->SliceView->setMRMLSliceNode(newSliceNode);
  d->SliceController->setMRMLSliceNode(newSliceNode);
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
//  d->SliceView->setHighlightedBoxColor(highlightedBoxColor);
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
const qMRMLSliceView* qMRMLSliceWidget::sliceView()const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceView;
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

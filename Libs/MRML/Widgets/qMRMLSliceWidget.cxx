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
#include <QApplication>
#include <QMainWindow>
#include <QWindow>

// qMRML includes
#include "qMRMLSliceWidget_p.h"

// MRMLDisplayableManager includes
#include <vtkMRMLSliceViewInteractorStyle.h>

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
qMRMLSliceWidgetPrivate::~qMRMLSliceWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::init()
{
  Q_Q(qMRMLSliceWidget);
  this->setupUi(q);

  this->SliceView->sliceViewInteractorStyle()
    ->SetSliceLogic(this->SliceController->sliceLogic());

  connect(this->SliceView, SIGNAL(resized(QSize)),
          this, SLOT(setSliceViewSize(QSize)));

  connect(this->SliceController, SIGNAL(imageDataConnectionChanged(vtkAlgorithmOutput*)),
          this, SLOT(setImageDataConnection(vtkAlgorithmOutput*)));
  connect(this->SliceController, SIGNAL(renderRequested()),
          this->SliceView, SLOT(scheduleRender()), Qt::QueuedConnection);
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::setSliceViewSize(const QSize& size)
{
  QSizeF scaledSizeF = QSizeF(size) * this->SliceView->devicePixelRatioF();
  this->SliceController->setSliceViewSize(scaledSizeF.toSize());
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::resetSliceViewSize()
{
  this->setSliceViewSize(this->SliceView->size());
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::endProcessing()
{
  // When a scene is closed, we need to reconfigure the SliceNode to
  // the size of the widget.
  this->setSliceViewSize(this->SliceView->size());
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::setImageDataConnection(vtkAlgorithmOutput * imageDataConnection)
{
  //qDebug() << "qMRMLSliceWidgetPrivate::setImageDataConnection";
  this->SliceView->setImageDataConnection(imageDataConnection);
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
qMRMLSliceWidget::qMRMLSliceWidget(qMRMLSliceWidgetPrivate* pimpl,
                                   QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(pimpl)
{
  // Note: You are responsible to call init() in the constructor of derived class.
}

// --------------------------------------------------------------------------
qMRMLSliceWidget::~qMRMLSliceWidget() = default;

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
  d->SliceController->setMRMLSliceNode(newSliceNode);
  d->SliceView->setMRMLSliceNode(newSliceNode);
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
void qMRMLSliceWidget::setImageDataConnection(vtkAlgorithmOutput* newImageDataConnection)
{
  Q_D(qMRMLSliceWidget);
  d->SliceController->setImageDataConnection(newImageDataConnection);
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* qMRMLSliceWidget::imageDataConnection() const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceController->imageDataConnection();
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
qMRMLSliceView* qMRMLSliceWidget::sliceView()const
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

// --------------------------------------------------------------------------
void qMRMLSliceWidget::showEvent(QShowEvent* event)
{
  Superclass::showEvent(event);

  Q_D(qMRMLSliceWidget);

  // Reset slice view size when screen changes to account for a possible change
  // in the device pixel ratio.
  QWindow* window = nullptr;
  foreach(QWidget* widget, qApp->topLevelWidgets())
    {
    QMainWindow* mainWindow = qobject_cast<QMainWindow*>(widget);
    if (mainWindow)
      {
      window = mainWindow->windowHandle();
      break;
      }
    }
  if (window)
    {
    connect(window, SIGNAL(screenChanged(QScreen*)),
            d, SLOT(resetSliceViewSize()), Qt::UniqueConnection);
    }
}

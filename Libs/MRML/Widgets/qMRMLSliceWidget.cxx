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
#include "qMRMLSliceView.h"

// MRMLDisplayableManager includes
#include <vtkMRMLSliceViewInteractorStyle.h>

// MRML includes
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLScene.h>

// MRML logic includes
#include <vtkMRMLSliceLogic.h>

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

  vtkMRMLSliceLogic* sliceLogic = this->SliceController->sliceLogic();

  this->SliceVerticalController->setSliceLogic(sliceLogic);
  this->SliceView->interactorObserver()->SetSliceLogic(sliceLogic);

  connect(this->SliceView, SIGNAL(resized(QSize)), this, SLOT(setSliceViewSize(QSize)));

  connect(this->SliceController,
          SIGNAL(imageDataConnectionChanged(vtkAlgorithmOutput*)),
          this,
          SLOT(setImageDataConnection(vtkAlgorithmOutput*)));
  connect(
    this->SliceController, SIGNAL(renderRequested()), this->SliceView, SLOT(scheduleRender()), Qt::QueuedConnection);
  connect(this->SliceVerticalController,
          SIGNAL(renderRequested()),
          this->SliceView,
          SLOT(scheduleRender()),
          Qt::QueuedConnection);

  this->updateSliceOffsetSliderOrientation();
}

// --------------------------------------------------------------------------
void qMRMLSliceWidgetPrivate::updateSliceOffsetSliderOrientation()
{
  bool horizontal = (this->SliceOffsetSliderOrientation == Qt::Horizontal);
  this->SliceController->setShowSliceOffsetSlider(horizontal);
  this->SliceVerticalController->setShowSliceOffsetSlider(!horizontal);
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
void qMRMLSliceWidgetPrivate::setImageDataConnection(vtkAlgorithmOutput* imageDataConnection)
{
  // qDebug() << "qMRMLSliceWidgetPrivate::setImageDataConnection";
  this->SliceView->setImageDataConnection(imageDataConnection);
}

// --------------------------------------------------------------------------
// qMRMLSliceView methods

// --------------------------------------------------------------------------
qMRMLSliceWidget::qMRMLSliceWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qMRMLSliceWidgetPrivate(*this))
{
  Q_D(qMRMLSliceWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLSliceWidget::qMRMLSliceWidget(qMRMLSliceWidgetPrivate* pimpl, QWidget* _parent)
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

  // In SliceController and  SliceVerticalController widgets
  // the scene is set by signals defined in the .ui file.

  d->qvtkReconnect(this->mrmlScene(), newScene, vtkMRMLScene::EndBatchProcessEvent, d, SLOT(endProcessing()));
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode)
{
  Q_D(qMRMLSliceWidget);
  d->SliceController->setMRMLSliceNode(newSliceNode);
  d->SliceVerticalController->setMRMLSliceNode(newSliceNode);
  d->SliceView->setMRMLSliceNode(newSliceNode);
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setMRMLAbstractViewNode(vtkMRMLAbstractViewNode* newViewNode)
{
  Q_D(qMRMLSliceWidget);
  vtkMRMLSliceNode* sliceViewNode = vtkMRMLSliceNode::SafeDownCast(newViewNode);
  if (newViewNode && !sliceViewNode)
  {
    qWarning() << Q_FUNC_INFO << " failed: Invalid view node type " << newViewNode->GetClassName()
               << ". Expected node type: vtkMRMLSliceNode";
  }
  this->setMRMLSliceNode(sliceViewNode);
}

//---------------------------------------------------------------------------
vtkMRMLSliceCompositeNode* qMRMLSliceWidget::mrmlSliceCompositeNode() const
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
QString qMRMLSliceWidget::sliceViewName() const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceController->sliceViewName();
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setSliceViewLabel(const QString& newSliceViewLabel)
{
  Q_D(qMRMLSliceWidget);
  this->setViewLabel(newSliceViewLabel);
}

//---------------------------------------------------------------------------
QString qMRMLSliceWidget::sliceViewLabel() const
{
  Q_D(const qMRMLSliceWidget);
  return this->viewLabel();
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setSliceViewColor(const QColor& newSliceViewColor)
{
  Q_D(const qMRMLSliceWidget);
  this->setViewColor(newSliceViewColor);
}

//---------------------------------------------------------------------------
QColor qMRMLSliceWidget::sliceViewColor() const
{
  Q_D(const qMRMLSliceWidget);
  return this->viewColor();
}

//---------------------------------------------------------------------------
void qMRMLSliceWidget::setSliceOrientation(const QString& orientation)
{
  Q_D(qMRMLSliceWidget);
  d->SliceController->setSliceOrientation(orientation);
}

//---------------------------------------------------------------------------
QString qMRMLSliceWidget::sliceOrientation() const
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
vtkInteractorObserver* qMRMLSliceWidget::interactorStyle() const
{
  return this->sliceView()->interactorStyle();
}

//---------------------------------------------------------------------------
vtkCornerAnnotation* qMRMLSliceWidget::overlayCornerAnnotation() const
{
  return this->sliceView()->overlayCornerAnnotation();
}

//---------------------------------------------------------------------------
vtkMRMLSliceNode* qMRMLSliceWidget::mrmlSliceNode() const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceController->mrmlSliceNode();
}

//---------------------------------------------------------------------------
vtkMRMLAbstractViewNode* qMRMLSliceWidget::mrmlAbstractViewNode() const
{
  Q_D(const qMRMLSliceWidget);
  return this->mrmlSliceNode();
}

//---------------------------------------------------------------------------
vtkMRMLSliceLogic* qMRMLSliceWidget::sliceLogic() const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceController->sliceLogic();
}

//---------------------------------------------------------------------------
vtkMRMLAbstractLogic* qMRMLSliceWidget::logic() const
{
  Q_D(const qMRMLSliceWidget);
  return this->sliceLogic();
}

// --------------------------------------------------------------------------
void qMRMLSliceWidget::fitSliceToBackground()
{
  Q_D(qMRMLSliceWidget);
  d->SliceController->fitSliceToBackground();
}

// --------------------------------------------------------------------------
void qMRMLSliceWidget::setSliceOffsetSliderOrientation(Qt::Orientation orientation)
{
  Q_D(qMRMLSliceWidget);
  d->SliceOffsetSliderOrientation = orientation;
  d->updateSliceOffsetSliderOrientation();
}

// --------------------------------------------------------------------------
qMRMLSliceView* qMRMLSliceWidget::sliceView() const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceView;
}

// --------------------------------------------------------------------------
QWidget* qMRMLSliceWidget::viewWidget() const
{
  Q_D(const qMRMLSliceWidget);
  return this->sliceView();
}

// --------------------------------------------------------------------------
Qt::Orientation qMRMLSliceWidget::sliceOffsetSliderOrientation() const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceOffsetSliderOrientation;
}

// --------------------------------------------------------------------------
qMRMLSliceControllerWidget* qMRMLSliceWidget::sliceController() const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceController;
}

// --------------------------------------------------------------------------
qMRMLViewControllerBar* qMRMLSliceWidget::controllerWidget() const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceController;
}

// --------------------------------------------------------------------------
qMRMLSliceVerticalControllerWidget* qMRMLSliceWidget::sliceVerticalController() const
{
  Q_D(const qMRMLSliceWidget);
  return d->SliceVerticalController;
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
  foreach (QWidget* widget, qApp->topLevelWidgets())
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
    connect(window, SIGNAL(screenChanged(QScreen*)), d, SLOT(resetSliceViewSize()), Qt::UniqueConnection);
  }
}

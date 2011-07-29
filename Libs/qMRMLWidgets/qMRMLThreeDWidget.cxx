/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QToolButton>

// CTK includes
#include <ctkLogger.h>
#include <ctkPopupWidget.h>

// qMRML includes
#include "qMRMLThreeDViewControllerWidget.h"
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLThreeDWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLSliceViewPrivate
class qMRMLThreeDWidgetPrivate
  : public QObject
{
  Q_DECLARE_PUBLIC(qMRMLThreeDWidget);
protected:
  qMRMLThreeDWidget* const q_ptr;
public:
  qMRMLThreeDWidgetPrivate(qMRMLThreeDWidget& object);
  ~qMRMLThreeDWidgetPrivate();
  
  void init();

  qMRMLThreeDView*       ThreeDView;
  QToolButton*           PinButton;
  ctkPopupWidget*        PopupWidget;
  qMRMLThreeDViewControllerWidget* ThreeDController;
};


//---------------------------------------------------------------------------
qMRMLThreeDWidgetPrivate::qMRMLThreeDWidgetPrivate(qMRMLThreeDWidget& object)
  : q_ptr(&object)
{
  this->ThreeDView = 0;
  this->PinButton = 0;
  this->PopupWidget = 0;
  this->ThreeDController = 0;
}

//---------------------------------------------------------------------------
qMRMLThreeDWidgetPrivate::~qMRMLThreeDWidgetPrivate()
{
  if (this->PopupWidget)
    {
    delete this->PopupWidget;
    this->PopupWidget = 0;
    }
}

//---------------------------------------------------------------------------
void qMRMLThreeDWidgetPrivate::init()
{
  Q_Q(qMRMLThreeDWidget);
  this->ThreeDView = new qMRMLThreeDView;
  
  QVBoxLayout* layout = new QVBoxLayout(q);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(this->ThreeDView);
  
  this->PinButton = new QToolButton(q);
  this->PinButton->setCheckable(true);
  this->PinButton->setAutoRaise(true);
  this->PinButton->setFixedSize(20, 20);
  QIcon pushPinIcon;
  pushPinIcon.addFile(":/Icons/PushPinIn.png", QSize(), QIcon::Normal, QIcon::On);
  pushPinIcon.addFile(":/Icons/PushPinOut.png", QSize(), QIcon::Normal, QIcon::Off);
  this->PinButton->setIcon(pushPinIcon);
  this->PinButton->setAutoFillBackground(true);
  this->PinButton->move(q->width() - this->PinButton->width(), 0);

  this->PopupWidget = new ctkPopupWidget;
  this->PopupWidget->setBaseWidget(this->PinButton);
  this->PopupWidget->setAlignment(Qt::AlignTop | Qt::AlignRight);
  this->PopupWidget->setOrientation(Qt::Horizontal | Qt::Vertical);
  this->PopupWidget->setHorizontalDirection(Qt::RightToLeft);
  
  QObject::connect(this->PinButton, SIGNAL(toggled(bool)),
                   this->PopupWidget, SLOT(pinPopup(bool)));

  this->ThreeDController = new qMRMLThreeDViewControllerWidget;
  this->ThreeDController->setThreeDView(this->ThreeDView);

  QToolButton* controllerPinButton = new QToolButton(this->ThreeDController);
  controllerPinButton->setCheckable(true);
  controllerPinButton->setAutoRaise(true);
  controllerPinButton->setIcon(pushPinIcon);
  //controllerPinButton->setFixedSize(this->PinButton->size());
  QObject::connect(controllerPinButton, SIGNAL(toggled(bool)),
                   this->PinButton, SLOT(setChecked(bool)));
  QObject::connect(this->PinButton, SIGNAL(toggled(bool)),
                   controllerPinButton, SLOT(setChecked(bool)));
  qobject_cast<QGridLayout*>(qobject_cast<QHBoxLayout*>(this->ThreeDController->layout())->itemAt(1)
    ->layout())->addWidget(controllerPinButton, 0,3, Qt::AlignRight | Qt::AlignTop);

  QHBoxLayout* popupLayout = new QHBoxLayout(this->PopupWidget);
  popupLayout->setContentsMargins(0, 0, 0, 0);
  popupLayout->addWidget(this->ThreeDController);

  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->ThreeDView, SLOT(setMRMLScene(vtkMRMLScene*)));
}

// --------------------------------------------------------------------------
// qMRMLThreeDWidget methods

// --------------------------------------------------------------------------
qMRMLThreeDWidget::qMRMLThreeDWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLThreeDWidgetPrivate(*this))
{
  Q_D(qMRMLThreeDWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLThreeDWidget::~qMRMLThreeDWidget()
{
}

// --------------------------------------------------------------------------
void qMRMLThreeDWidget::addDisplayableManager(const QString& dManager)
{
  Q_D(qMRMLThreeDWidget);
  d->ThreeDView->addDisplayableManager(dManager);
}

// --------------------------------------------------------------------------
void qMRMLThreeDWidget::setMRMLViewNode(vtkMRMLViewNode* newViewNode)
{
  Q_D(qMRMLThreeDWidget);
  d->ThreeDView->setMRMLViewNode(newViewNode);
  d->ThreeDController->setMRMLViewNode(newViewNode);
}

// --------------------------------------------------------------------------
vtkMRMLViewNode* qMRMLThreeDWidget::mrmlViewNode()const
{
  Q_D(const qMRMLThreeDWidget);
  return d->ThreeDView->mrmlViewNode();
}

// --------------------------------------------------------------------------
qMRMLThreeDView* qMRMLThreeDWidget::threeDView()const
{
  Q_D(const qMRMLThreeDWidget);
  return d->ThreeDView;
}

// --------------------------------------------------------------------------
void qMRMLThreeDWidget::resizeEvent(QResizeEvent* event)
{
  Q_D(qMRMLThreeDWidget);
  d->PinButton->move(event->size().width() - d->PinButton->width(), 0);
  this->Superclass::resizeEvent(event);
}

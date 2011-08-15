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
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

// CTK includes
#include <ctkLogger.h>
#include <ctkPopupWidget.h>

// qMRML includes
#include "qMRMLViewControllerBar_p.h"

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLViewControllerBar");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLViewControllerBarPrivate methods

//---------------------------------------------------------------------------
qMRMLViewControllerBarPrivate::qMRMLViewControllerBarPrivate(
  qMRMLViewControllerBar& object)
  : QObject(0)
  , q_ptr(&object)
{
  this->PinButton = 0;
  this->ViewLabel = 0;
  this->PopupWidget = 0;
}

//---------------------------------------------------------------------------
qMRMLViewControllerBarPrivate::~qMRMLViewControllerBarPrivate()
{
  delete this->PopupWidget;
  this->PopupWidget = 0;
}

//---------------------------------------------------------------------------
void qMRMLViewControllerBarPrivate::init()
{
  Q_Q(qMRMLViewControllerBar);
  this->setParent(q);

  // necessary to colorize the bar
  q->setAutoFillBackground(true);

  this->PopupWidget = new ctkPopupWidget;
  this->setupPopupUi();

  QHBoxLayout* barLayout = new QHBoxLayout(q);
  barLayout->setSpacing(2);
  QMargins margins = barLayout->contentsMargins();
  margins.setTop(0);
  margins.setBottom(0);
  barLayout->setContentsMargins(margins);

  this->PinButton = new QToolButton(q);
  this->PinButton->setCheckable(true);
  this->PinButton->setAutoRaise(true);
  this->PinButton->setFixedSize(15, 15);
  QIcon pushPinIcon;
  pushPinIcon.addFile(":/Icons/PushPinIn.png", QSize(), QIcon::Normal, QIcon::On);
  pushPinIcon.addFile(":/Icons/PushPinOut.png", QSize(), QIcon::Normal, QIcon::Off);
  this->PinButton->setIcon(pushPinIcon);
  QObject::connect(this->PinButton, SIGNAL(toggled(bool)),
                   this->PopupWidget, SLOT(pinPopup(bool)));
  this->PinButton->installEventFilter(this);
  barLayout->addWidget(this->PinButton);

  this->ViewLabel = new QLabel(q);
  this->ViewLabel->setAlignment(Qt::AlignHCenter | Qt::AlignCenter);
  this->ViewLabel->setMinimumWidth(this->ViewLabel->fontMetrics().width("XX"));
  this->ViewLabel->setAutoFillBackground(true);
  barLayout->addWidget(this->ViewLabel);

  barLayout->addSpacing(5);
}

//---------------------------------------------------------------------------
void qMRMLViewControllerBarPrivate::setupPopupUi()
{
  Q_Q(qMRMLViewControllerBar);
  this->PopupWidget->setBaseWidget(q);
  this->PopupWidget->setAutoShow(false);
  this->PopupWidget->setAutoHide(true);
  this->PopupWidget->setOrientation(Qt::Vertical);
}

//---------------------------------------------------------------------------
bool qMRMLViewControllerBarPrivate::eventFilter(QObject* object, QEvent* event)
{
  if (object == this->PinButton && event->type() == QEvent::Enter)
    {
    this->PopupWidget->showPopup();
    }
  return this->QObject::eventFilter(object, event);
}

//---------------------------------------------------------------------------
void qMRMLViewControllerBarPrivate::setColor(QColor barColor)
{
  Q_Q(qMRMLViewControllerBar);

  QPalette palette = q->palette();
  QLinearGradient gradient(QPointF(0.,0.), QPointF(0.,1.));
  gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
  gradient.setColorAt(0., barColor.lighter(102));
  gradient.setColorAt(0.1, barColor);
  gradient.setColorAt(0.8, barColor.darker(102));
  gradient.setColorAt(1., barColor.darker(108));
  palette.setBrush(QPalette::Window, gradient);
  q->setPalette(palette);

  QPalette labelPalette( barColor.lighter(130));
  this->ViewLabel->setPalette(labelPalette);
}

// --------------------------------------------------------------------------
// qMRMLViewControllerBar methods

// --------------------------------------------------------------------------
qMRMLViewControllerBar::qMRMLViewControllerBar(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLViewControllerBarPrivate(*this))
{
  Q_D(qMRMLViewControllerBar);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLViewControllerBar::qMRMLViewControllerBar(qMRMLViewControllerBarPrivate* pimpl, QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(pimpl)
{
  // init() should be called in the subclass constructor
}

// --------------------------------------------------------------------------
qMRMLViewControllerBar::~qMRMLViewControllerBar()
{
}

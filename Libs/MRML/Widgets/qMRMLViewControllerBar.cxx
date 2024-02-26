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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>

// CTK includes
#include <ctkPopupWidget.h>

// VTK includes
#include <vtkCommand.h>

// qMRML includes
#include "qMRMLViewControllerBar_p.h"

// MRML includes
#include "vtkMRMLLayoutNode.h"
#include "vtkMRMLAbstractViewNode.h"
#include "vtkMRMLScene.h"

//--------------------------------------------------------------------------
// qMRMLViewControllerBarPrivate methods

//---------------------------------------------------------------------------
qMRMLViewControllerBarPrivate::qMRMLViewControllerBarPrivate(qMRMLViewControllerBar& object)
  : QObject(nullptr)
  , q_ptr(&object)
{
}

//---------------------------------------------------------------------------
qMRMLViewControllerBarPrivate::~qMRMLViewControllerBarPrivate()
{
  delete this->PopupWidget;
  this->PopupWidget = nullptr;
  delete this->BarLayout;
  this->BarLayout = nullptr;
  delete this->BarWidget;
  this->BarWidget = nullptr;
  delete this->ControllerLayout;
  this->ControllerLayout = nullptr;
}

//---------------------------------------------------------------------------
void qMRMLViewControllerBarPrivate::init()
{
  Q_Q(qMRMLViewControllerBar);
  // Widget contains
  // VBoxLayout (ControllerLayout)
  //    Widget (BarWidget)
  //        HBoxLayout (BarLayout)
  //    Optional PopupWidget (packed under BarWidget if ControllerBar is a panel)

  this->setParent(q);

  this->BarWidget = new QWidget(q);
  this->BarWidget->setObjectName("BarWidget");
  this->BarWidget->setAutoFillBackground(true); // color the bar

  this->ControllerLayout = new QVBoxLayout(q);
  this->ControllerLayout->setSpacing(2);
  QMargins cmargins = this->ControllerLayout->contentsMargins();
  cmargins.setTop(0);
  cmargins.setBottom(0);
  cmargins.setLeft(0);
  cmargins.setRight(0);
  this->ControllerLayout->setContentsMargins(cmargins);

  this->BarLayout = new QHBoxLayout();
  this->BarLayout->setSpacing(2);
  QMargins margins = this->BarLayout->contentsMargins();
  margins.setTop(0);
  margins.setBottom(0);
  this->BarLayout->setContentsMargins(margins);

  this->PopupWidget = new ctkPopupWidget(q);
  this->setupPopupUi();

  this->PinButton = new QToolButton(q);
  this->PinButton->setObjectName("PinButton");
  this->PinButton->setCheckable(true);
  this->PinButton->setAutoRaise(true);
  QIcon pushPinIcon;
  pushPinIcon.addFile(":/Icons/PushPinIn.png", QSize(), QIcon::Normal, QIcon::On);
  pushPinIcon.addFile(":/Icons/PushPinOut.png", QSize(), QIcon::Normal, QIcon::Off);
  this->PinButton->setIcon(pushPinIcon);
  QObject::connect(this->PinButton, SIGNAL(toggled(bool)), this->PopupWidget, SLOT(pinPopup(bool)));
  this->PinButton->installEventFilter(this);
  this->BarLayout->addWidget(this->PinButton);

  this->ViewLabel = new QLabel(q);
  this->ViewLabel->setObjectName("ViewLabel");
  this->ViewLabel->setAlignment(Qt::AlignHCenter | Qt::AlignCenter);
  // Slice controller background color is independent from the color palette, therefore the color of text and controls
  // are hardcoded to black
  this->ViewLabel->setStyleSheet("color: black; background-color: transparent;");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
  this->ViewLabel->setMinimumWidth(this->ViewLabel->fontMetrics().horizontalAdvance("XX"));
#else
  this->ViewLabel->setMinimumWidth(this->ViewLabel->fontMetrics().width("XX"));
#endif
  this->BarLayout->addWidget(this->ViewLabel);

  this->ViewMaximizeIcon = QIcon(":Icons/ViewMaximize.png");
  this->ViewRestoreIcon = QIcon(":Icons/ViewRestore.png");

  this->MaximizeViewButton = new QToolButton(q);
  this->MaximizeViewButton->setObjectName("MaximizeViewButton");
  this->MaximizeViewButton->setToolTip(tr("Maximize/restore view"));
  this->MaximizeViewButton->setAutoRaise(true);
  this->MaximizeViewButton->setIcon(this->ViewMaximizeIcon);
  this->BarLayout->addWidget(this->MaximizeViewButton);
  QObject::connect(this->MaximizeViewButton, SIGNAL(clicked()), q, SLOT(maximizeView()));

  this->BarLayout->addSpacing(5);

  this->BarWidget->setLayout(this->BarLayout);
  this->ControllerLayout->addWidget(this->BarWidget);
  q->setLayout(this->ControllerLayout);
}

//---------------------------------------------------------------------------
void qMRMLViewControllerBarPrivate::setupPopupUi()
{
  this->PopupWidget->setAutoShow(false);
  this->PopupWidget->setAutoHide(true);
  this->PopupWidget->setOrientation(Qt::Vertical);
  this->PopupWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
}

//---------------------------------------------------------------------------
bool qMRMLViewControllerBarPrivate::eventFilter(QObject* object, QEvent* event)
{
  if (object == this->PinButton && event->type() == QEvent::Enter)
  {
    if (this->LayoutBehavior == qMRMLViewControllerBar::Popup)
    {
      this->PopupWidget->showPopup();
    }
  }
  return this->QObject::eventFilter(object, event);
}

//---------------------------------------------------------------------------
void qMRMLViewControllerBarPrivate::setColor(QColor barColor)
{
  // Q_Q(qMRMLViewControllerBar);
  this->BarColor = barColor;

  QPalette palette = this->BarWidget->palette();
  QLinearGradient gradient(QPointF(0., 0.), QPointF(0., 1.));
  gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
  // Light gradient
  // gradient.setColorAt(0., barColor.lighter(102));
  // gradient.setColorAt(0.1, barColor);
  // gradient.setColorAt(0.8, barColor.darker(102));
  // gradient.setColorAt(1., barColor.darker(108));
  // Glass effect
  // gradient.setColorAt(0., barColor.lighter(120));
  // gradient.setColorAt(0.498, barColor.darker(118));
  // gradient.setColorAt(0.5, barColor.darker(145));
  // gradient.setColorAt(1., barColor.darker(165));
  // Dialog effect
  int hue = barColor.hue();
  int sat = barColor.saturation();
  double valueCoef = static_cast<double>(barColor.value()) / 240.;
  gradient.setColorAt(0.0, QColor::fromHsv(hue, sat, qMin(255., 358. * valueCoef)));
  gradient.setColorAt(0.15, QColor::fromHsv(hue, sat, qMin(255., 224 * valueCoef)));
  gradient.setColorAt(0.24, QColor::fromHsv(hue, sat, qMin(255., 213 * valueCoef)));
  gradient.setColorAt(0.65, QColor::fromHsv(hue, sat, qMin(255., 255 * valueCoef)));
  gradient.setColorAt(0.71, QColor::fromHsv(hue, sat, qMin(255., 246 * valueCoef)));
  gradient.setColorAt(0.85, QColor::fromHsv(hue, sat, qMin(255., 253 * valueCoef)));
  gradient.setColorAt(1.0, QColor::fromHsv(hue, sat, qMin(255., 191 * valueCoef)));
  palette.setBrush(QPalette::Window, gradient);
  palette.setBrush(QPalette::Text, Qt::black);
  this->BarWidget->setPalette(palette);
}

//---------------------------------------------------------------------------
QColor qMRMLViewControllerBarPrivate::color() const
{
  return this->BarColor;
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
qMRMLViewControllerBar::~qMRMLViewControllerBar() = default;

// --------------------------------------------------------------------------
void qMRMLViewControllerBar::setLayoutBehavior(LayoutBehavior behavior)
{
  Q_D(qMRMLViewControllerBar);

  d->LayoutBehavior = behavior;

  if (d->PopupWidget)
  {
    if (behavior == qMRMLViewControllerBar::Popup)
    {
      d->PopupWidget->setAutoShow(false);
      d->PopupWidget->setAutoHide(true);
      d->ControllerLayout->removeWidget(d->PopupWidget);
      d->PopupWidget->setActive(true);
      d->PopupWidget->setWindowFlags(d->PopupWidget->windowFlags() | Qt::ToolTip);
      d->PinButton->show();
    }
    else
    {
      d->PopupWidget->setAutoShow(false);
      d->PopupWidget->setAutoHide(false);
      d->ControllerLayout->addWidget(d->PopupWidget);
      d->PinButton->hide();
      // d->PinButton->setDown(1);
      d->PopupWidget->setActive(false);
      d->PopupWidget->setWindowFlags(d->PopupWidget->windowFlags() & ~Qt::ToolTip);
      d->PopupWidget->pinPopup(true);
    }
  }
}

// --------------------------------------------------------------------------
QLayout* qMRMLViewControllerBar::barLayout()
{
  Q_D(qMRMLViewControllerBar);

  return d->BarLayout;
}

// --------------------------------------------------------------------------
QWidget* qMRMLViewControllerBar::barWidget()
{
  Q_D(qMRMLViewControllerBar);

  return d->BarWidget;
}

// --------------------------------------------------------------------------
QToolButton* qMRMLViewControllerBar::pinButton()
{
  Q_D(qMRMLViewControllerBar);
  return d->PinButton;
}

// --------------------------------------------------------------------------
QLabel* qMRMLViewControllerBar::viewLabel()
{
  Q_D(qMRMLViewControllerBar);
  return d->ViewLabel;
}

// --------------------------------------------------------------------------
void qMRMLViewControllerBar::maximizeView()
{
  Q_D(qMRMLViewControllerBar);
  if (!this->mrmlViewNode())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid view node.";
    return;
  }

  bool isMaximized = false;
  bool canBeMaximized = false;
  vtkMRMLLayoutNode* layoutNode = this->mrmlViewNode()->GetMaximizedState(isMaximized, canBeMaximized);
  if (!layoutNode || !canBeMaximized)
  {
    return;
  }

  if (isMaximized)
  {
    layoutNode->RemoveMaximizedViewNode(this->mrmlViewNode());
  }
  else
  {
    layoutNode->AddMaximizedViewNode(this->mrmlViewNode());
  }
}

//---------------------------------------------------------------------------
void qMRMLViewControllerBar::setMRMLViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  Q_D(qMRMLViewControllerBar);
  this->qvtkReconnect(d->ViewNode, viewNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRMLView()));
  d->ViewNode = viewNode;
  this->updateWidgetFromMRMLView();
}

//---------------------------------------------------------------------------
vtkMRMLAbstractViewNode* qMRMLViewControllerBar::mrmlViewNode() const
{
  Q_D(const qMRMLViewControllerBar);
  return d->ViewNode;
}

//---------------------------------------------------------------------------
void qMRMLViewControllerBar::updateWidgetFromMRMLView()
{
  Q_D(qMRMLViewControllerBar);

  // The view node gets modified whenever it is added/remoeved from the layout
  // therefore we can update the maximize button status here (there is no need
  // to observe the layout).
  bool isMaximized = false;
  bool canBeMaximized = false;
  if (d->ViewNode)
  {
    d->ViewNode->GetMaximizedState(isMaximized, canBeMaximized);
  }

  d->MaximizeViewButton->setVisible(canBeMaximized && d->ShowMaximizeViewButton);
  if (canBeMaximized && d->ShowMaximizeViewButton)
  {
    if (isMaximized)
    {
      d->MaximizeViewButton->setToolTip(tr("Restore view layout"));
      d->MaximizeViewButton->setIcon(d->ViewRestoreIcon);
    }
    else
    {
      d->MaximizeViewButton->setToolTip(tr("Maximize view"));
      d->MaximizeViewButton->setIcon(d->ViewMaximizeIcon);
    }
  }
}

//-----------------------------------------------------------------------------
bool qMRMLViewControllerBar::showMaximizeViewButton() const
{
  Q_D(const qMRMLViewControllerBar);
  return d->ShowMaximizeViewButton;
}

//-----------------------------------------------------------------------------
void qMRMLViewControllerBar::setShowMaximizeViewButton(bool show)
{
  Q_D(qMRMLViewControllerBar);
  d->ShowMaximizeViewButton = show;
  this->updateWidgetFromMRMLView();
}

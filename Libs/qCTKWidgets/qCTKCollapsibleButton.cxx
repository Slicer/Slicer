/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#include "qCTKCollapsibleButton.h"

#include <QApplication>
#include <QCleanlooksStyle>
#include <QDebug>
#include <QLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QStyle>
#include <QStyleOptionButton>
#include <QStyleOptionFrameV3>

//-----------------------------------------------------------------------------
class qCTKCollapsibleButtonPrivate : public qCTKPrivate<qCTKCollapsibleButton>
{
public:
  QCTK_DECLARE_PUBLIC(qCTKCollapsibleButton);
  void init();

  bool     Collapsed;

  // Contents frame
  QFrame::Shape  ContentsFrameShape;
  QFrame::Shadow ContentsFrameShadow;
  int            ContentsLineWidth;
  int            ContentsMidLineWidth;

  int      CollapsedHeight;
  bool     ExclusiveMouseOver;
  bool     LookOffWhenChecked;

  int      MaximumHeight;  // use carefully
};

//-----------------------------------------------------------------------------
void qCTKCollapsibleButtonPrivate::init()
{
  QCTK_P(qCTKCollapsibleButton);
  p->setCheckable(true);
  // checked and Collapsed are synchronized: checked != Collapsed
  p->setChecked(true);

  this->Collapsed = false;

  this->ContentsFrameShape = QFrame::NoFrame;
  this->ContentsFrameShadow = QFrame::Plain;
  this->ContentsLineWidth = 1;
  this->ContentsMidLineWidth = 0;

  this->CollapsedHeight = 10;
  this->ExclusiveMouseOver = false;  
  this->LookOffWhenChecked = true; // set as a prop ?
  
  this->MaximumHeight = p->maximumHeight();

  p->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,
                               QSizePolicy::Preferred, 
                               QSizePolicy::DefaultType));
  p->setContentsMargins(0, p->buttonSizeHint().height(),0 , 0);

  QObject::connect(p, SIGNAL(toggled(bool)),
                   p, SLOT(onToggled(bool)));
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleButton::initStyleOption(QStyleOptionButton* option)const
{
  QCTK_D(const qCTKCollapsibleButton);
  if (option == 0)
    {
    return;
    }
  option->initFrom(this);

  if (this->isDown() )
    {
    option->state |= QStyle::State_Sunken;
    }
  if (this->isChecked() && !d->LookOffWhenChecked)
    {
    option->state |= QStyle::State_On;
    }
  if (!this->isDown())
    {
    option->state |= QStyle::State_Raised;
    }

  option->text = this->text();
  option->icon = this->icon();
  option->iconSize = QSize(this->style()->pixelMetric(QStyle::PM_IndicatorWidth, option, this),
                       this->style()->pixelMetric(QStyle::PM_IndicatorHeight, option, this));
  int buttonHeight = this->buttonSizeHint().height();//qMax(this->fontMetrics().height(), option->iconSize.height());
  option->rect.setHeight(buttonHeight);
}

//-----------------------------------------------------------------------------
qCTKCollapsibleButton::qCTKCollapsibleButton(QWidget* _parent)
  :QAbstractButton(_parent)
{
  QCTK_INIT_PRIVATE(qCTKCollapsibleButton);
  qctk_d()->init();
}

//-----------------------------------------------------------------------------
qCTKCollapsibleButton::qCTKCollapsibleButton(const QString& title, QWidget* _parent)
  :QAbstractButton(_parent)
{
  QCTK_INIT_PRIVATE(qCTKCollapsibleButton);
  qctk_d()->init();
  this->setText(title);
}

//-----------------------------------------------------------------------------
qCTKCollapsibleButton::~qCTKCollapsibleButton()
{
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleButton::setCollapsed(bool c)
{
  if (!this->isCheckable())
    {
    // not sure if one should handle this case...
    this->collapse(c);
    return;
    }
  this->setChecked(!c);
}

//-----------------------------------------------------------------------------
bool qCTKCollapsibleButton::collapsed()const
{
  return qctk_d()->Collapsed;
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleButton::setCollapsedHeight(int h)
{
  qctk_d()->CollapsedHeight = h;
  this->updateGeometry();
}

//-----------------------------------------------------------------------------
int qCTKCollapsibleButton::collapsedHeight()const
{
  return qctk_d()->CollapsedHeight;
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleButton::onToggled(bool checked)
{
  if (this->isCheckable())
    {
    this->collapse(!checked);
    }
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleButton::collapse(bool c)
{
  QCTK_D(qCTKCollapsibleButton);
  if (c == d->Collapsed)
    {
    return;
    }

  d->Collapsed = c;

  // we do that here as setVisible calls will correctly refresh the widget
  if (c)
    {
    d->MaximumHeight = this->maximumHeight();
    this->setMaximumHeight(this->sizeHint().height());
    //this->updateGeometry();
    }
  else
    {
    // restore maximumheight
    this->setMaximumHeight(d->MaximumHeight);
    this->updateGeometry();
    }

  QObjectList childList = this->children();
  for (int i = 0; i < childList.size(); ++i)
    {
    QObject *o = childList.at(i);
    if (!o->isWidgetType())
      {
      continue;
      }
    QWidget *w = static_cast<QWidget *>(o);
    w->setVisible(!c);
    }
  // this might be too many updates...
  QWidget* _parent = this->parentWidget();
  if (!d->Collapsed && (!_parent || !_parent->layout()))
    {
    this->resize(this->sizeHint());
    }
  else
    {
    this->updateGeometry();
    }
  //this->update(QRect(QPoint(0,0), this->sizeHint()));
  //this->repaint(QRect(QPoint(0,0), this->sizeHint()));
  emit contentsCollapsed(c);
}

//-----------------------------------------------------------------------------
QFrame::Shape qCTKCollapsibleButton::contentsFrameShape() const
{
  return qctk_d()->ContentsFrameShape;
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleButton::setContentsFrameShape(QFrame::Shape s)
{
  qctk_d()->ContentsFrameShape = s;
}

//-----------------------------------------------------------------------------
QFrame::Shadow qCTKCollapsibleButton::contentsFrameShadow() const
{
  return qctk_d()->ContentsFrameShadow;
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleButton::setContentsFrameShadow(QFrame::Shadow s)
{
  qctk_d()->ContentsFrameShadow = s;
}

//-----------------------------------------------------------------------------
int qCTKCollapsibleButton:: contentsLineWidth() const
{
  return qctk_d()->ContentsLineWidth;
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleButton::setContentsLineWidth(int w)
{
  qctk_d()->ContentsLineWidth = w;
}

//-----------------------------------------------------------------------------
int qCTKCollapsibleButton::contentsMidLineWidth() const
{
  return qctk_d()->ContentsMidLineWidth;
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleButton::setContentsMidLineWidth(int w)
{
  qctk_d()->ContentsMidLineWidth = w;
}

//-----------------------------------------------------------------------------
QSize qCTKCollapsibleButton::buttonSizeHint()const
{
  int w = 0, h = 0;

  QStyleOptionButton opt;
  opt.initFrom(this);
  
  // indicator
  QSize indicatorSize = QSize(style()->pixelMetric(QStyle::PM_IndicatorWidth, &opt, this),
                              style()->pixelMetric(QStyle::PM_IndicatorHeight, &opt, this));
  int indicatorSpacing = style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing, &opt, this);
  int ih = indicatorSize.height();
  int iw = indicatorSize.width() + indicatorSpacing;
  w += iw;
  h = qMax(h, ih);
  
  // text 
  QString string(this->text());
  bool empty = string.isEmpty();
  if (empty)
    {
    string = QString::fromLatin1("XXXX");
    }
  QFontMetrics fm = this->fontMetrics();
  QSize sz = fm.size(Qt::TextShowMnemonic, string);
  if(!empty || !w)
    {
    w += sz.width();
    }
  h = qMax(h, sz.height());
  //opt.rect.setSize(QSize(w, h)); // PM_MenuButtonIndicator depends on the height
  QSize buttonSize = (style()->sizeFromContents(QStyle::CT_PushButton, &opt, QSize(w, h), this).
                      expandedTo(QApplication::globalStrut()));
  return buttonSize;
}

//-----------------------------------------------------------------------------
QSize qCTKCollapsibleButton::minimumSizeHint()const
{
  QCTK_D(const qCTKCollapsibleButton);
  QSize buttonSize = this->buttonSizeHint();
  if (d->Collapsed)
    {
    return buttonSize + QSize(0,d->CollapsedHeight);
    }
  // open
  if (this->layout() == 0)
    {// no layout, means the button is empty ?
    return buttonSize;
    }
  QSize s = this->QAbstractButton::minimumSizeHint(); 
  return s.expandedTo(buttonSize);
}

//-----------------------------------------------------------------------------
QSize qCTKCollapsibleButton::sizeHint()const
{
  QCTK_D(const qCTKCollapsibleButton);
  QSize buttonSize = this->buttonSizeHint();
  if (d->Collapsed)
    {
    return buttonSize + QSize(0,d->CollapsedHeight);
    }
  // open
  // QAbstractButton works well only if a layout is set
  QSize s = this->QAbstractButton::sizeHint(); 
  return s.expandedTo(buttonSize + QSize(0, d->CollapsedHeight));
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleButton::paintEvent(QPaintEvent * _event)
{
  QCTK_D(qCTKCollapsibleButton);

  QPainter p(this);
  // Draw Button
  QStyleOptionButton opt;
  this->initStyleOption(&opt);

  // We don't want to have the highlight effect on the button when mouse is
  // over a child. We want the highlight effect only when the mouse is just
  // over itself.
  // same as this->underMouse()
  bool exclusiveMouseOver = false;
  if (opt.state & QStyle::State_MouseOver)
    {
    QRect buttonRect = opt.rect;
    QList<QWidget*> _children = this->findChildren<QWidget*>();
    QList<QWidget*>::ConstIterator it;
    for (it = _children.constBegin(); it != _children.constEnd(); ++it ) 
      {
      if ((*it)->underMouse())
        {
        // the mouse has been moved from the collapsible button to one 
        // of its children. The paint event rect is the child rect, this
        // is why we have to request another paint event to redraw the 
        // button to remove the highlight effect.
        if (!_event->rect().contains(buttonRect))
          {// repaint the button rect.
          this->update(buttonRect);
          }
        opt.state &= ~QStyle::State_MouseOver;
        exclusiveMouseOver = true;
        break;
        }
      }
    if (d->ExclusiveMouseOver && !exclusiveMouseOver)
      {
      // the mouse is over the widget, but not over the children. As it 
      // has been de-highlighted in the past, we should refresh the button
      // rect to re-highlight the button.
      if (!_event->rect().contains(buttonRect))
        {// repaint the button rect.
        this->update(buttonRect);
        }
      }
    }
  d->ExclusiveMouseOver = exclusiveMouseOver;
  QSize indicatorSize = QSize(style()->pixelMetric(QStyle::PM_IndicatorWidth, &opt, this),
                              style()->pixelMetric(QStyle::PM_IndicatorHeight, &opt, this));
  opt.iconSize = indicatorSize;
  style()->drawControl(QStyle::CE_PushButtonBevel, &opt, &p, this);
  int buttonHeight = opt.rect.height();
  // Draw Indicator
  QStyleOption indicatorOpt;
  indicatorOpt.init(this);
  indicatorOpt.rect = QRect((buttonHeight - indicatorSize.width()) / 2, 
                            (buttonHeight - indicatorSize.height()) / 2,
                            indicatorSize.width(), indicatorSize.height());
  if (d->Collapsed)
    {
    style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &indicatorOpt, &p, this);
    }
  else
    {
    style()->drawPrimitive(QStyle::PE_IndicatorArrowUp, &indicatorOpt, &p, this);
    }
  // Draw Text
  int indicatorSpacing = style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing, &opt, this);
  opt.rect.setLeft( indicatorOpt.rect.right() + indicatorSpacing);
  uint tf = Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::AlignLeft;
  style()->drawItemText(&p, opt.rect, tf, opt.palette, (opt.state & QStyle::State_Enabled),
                        opt.text, QPalette::ButtonText);
  // Draw Frame around contents
  QStyleOptionFrameV3 f;
  f.init(this);
  // HACK: on some styles, the frame doesn't exactly touch the button.
  // this is because the button has some kind of extra border. 
  if (qobject_cast<QCleanlooksStyle*>(this->style()) != 0)
    {
    f.rect.setTop(buttonHeight - 1);
    }
  else
    {
    f.rect.setTop(buttonHeight);
    }
  f.frameShape = d->ContentsFrameShape;
  switch (d->ContentsFrameShadow)
    {
    case QFrame::Sunken:
      f.state |= QStyle::State_Sunken;
      break;
    case QFrame::Raised:
      f.state |= QStyle::State_Raised;
      break;
    default:
    case QFrame::Plain:
      break;
    }
  f.lineWidth = d->ContentsLineWidth;
  f.midLineWidth = d->ContentsMidLineWidth;
  style()->drawControl(QStyle::CE_ShapedFrame, &f, &p, this);
}

//-----------------------------------------------------------------------------
bool qCTKCollapsibleButton::hitButton(const QPoint & _pos)const
{
  QStyleOptionButton opt;
  this->initStyleOption(&opt);
  return opt.rect.contains(_pos);
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleButton::childEvent(QChildEvent* c)
{
  if(c && c->type() == QEvent::ChildAdded)
    {
    if (c->child() && c->child()->isWidgetType())
      {
      QWidget *w = static_cast<QWidget*>(c->child());
      w->setVisible(!qctk_d()->Collapsed);
      }
    }
  QWidget::childEvent(c);
}

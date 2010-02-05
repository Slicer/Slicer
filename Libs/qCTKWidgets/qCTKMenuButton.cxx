/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#include "qCTKMenuButton.h"

#include <QApplication>
#include <QCleanlooksStyle>
#include <QDebug>
#include <QDesktopWidget>
#include <QLayout>
#include <QMouseEvent>
#include <QMenu>
#include <QPainter>
#include <QPointer>
#include <QPushButton>
#include <QStyle>
#include <QStyleOptionButton>
#include <QStylePainter>
#include <QToolBar>

//-----------------------------------------------------------------------------
class qCTKMenuButtonPrivate : public qCTKPrivate<qCTKMenuButton>
{
public:
  QCTK_DECLARE_PUBLIC(qCTKMenuButton);
  void init();
  QRect indicatorRect() const;

  QPointer<QMenu> ExtraMenu;
};


//-----------------------------------------------------------------------------
void qCTKMenuButtonPrivate::init()
{
}

//-----------------------------------------------------------------------------
QRect qCTKMenuButtonPrivate::indicatorRect()const 
{
  QCTK_P(const qCTKMenuButton);
  
  QStyleOptionButton option;
  p->initStyleOption(&option);

  QRect downArrowRect = p->style()->visualRect(option.direction, option.rect, option.rect);
  downArrowRect.setRect(downArrowRect.right() - 13, downArrowRect.top(),
                        14, downArrowRect.height());
  downArrowRect = p->style()->visualRect(option.direction, option.rect, downArrowRect);
  return downArrowRect;
}

//-----------------------------------------------------------------------------
qCTKMenuButton::qCTKMenuButton(QWidget* _parent)
  :QPushButton(_parent)
{
  QCTK_INIT_PRIVATE(qCTKMenuButton);
  qctk_d()->init();
}

//-----------------------------------------------------------------------------
qCTKMenuButton::qCTKMenuButton(const QString& title, QWidget* _parent)
  :QPushButton(title, _parent)
{
  QCTK_INIT_PRIVATE(qCTKMenuButton);
  qctk_d()->init();
}

//-----------------------------------------------------------------------------
qCTKMenuButton::~qCTKMenuButton()
{
}

//-----------------------------------------------------------------------------
void qCTKMenuButton::setExtraMenu(QMenu* extramenu)
{
  QCTK_D(qCTKMenuButton);
  if (d->ExtraMenu)
    {
    this->removeAction(d->ExtraMenu->menuAction());
    }
  d->ExtraMenu = extramenu;
  if (d->ExtraMenu)
    {
    this->addAction(d->ExtraMenu->menuAction());
    }
  update();
  updateGeometry();
}

//-----------------------------------------------------------------------------
QMenu* qCTKMenuButton::extraMenu() const
{
    QCTK_D(const qCTKMenuButton);
    return d->ExtraMenu;
}

//-----------------------------------------------------------------------------
QSize qCTKMenuButton::minimumSizeHint()const
{
  QSize min = QPushButton::minimumSizeHint();
  return QSize(min.width() + 13, min.height());
}

//-----------------------------------------------------------------------------
QSize qCTKMenuButton::sizeHint()const
{
  QSize buttonSizeHint = QPushButton::sizeHint();
  return QSize(buttonSizeHint.width() + 13, buttonSizeHint.height());
}

//-----------------------------------------------------------------------------
void qCTKMenuButton::paintEvent(QPaintEvent * _event)
{
  Q_UNUSED(_event);
  QCTK_D(qCTKMenuButton);
  QStylePainter painter(this);
  QStyleOptionButton option;
  initStyleOption(&option);

  // Draw button
  option.features &= ~QStyleOptionButton::HasMenu;
  painter.drawControl(QStyle::CE_PushButtonBevel, option);
  // is PE_PanelButtonCommand better ?
  //painter.drawPrimitive(QStyle::PE_PanelButtonCommand, option);
  QRect downArrowRect = d->indicatorRect();
  if (option.state & QStyle::State_Sunken)
    {    
    // if the button is down, draw the part under the indicator up
    QPixmap cache = QPixmap(option.rect.size());
    cache.fill(Qt::transparent);
    QPainter cachePainter(&cache);
    option.state &= ~QStyle::State_Sunken;
    option.state |= QStyle::State_Raised;
    //option.state &= ~QStyle::State_HasFocus;
    option.state &= ~QStyle::State_MouseOver;
    this->style()->drawControl(QStyle::CE_PushButtonBevel, &option, &cachePainter, this);
    painter.drawItemPixmap(downArrowRect, Qt::AlignLeft | Qt::AlignTop, cache.copy(downArrowRect)); 
    }

  // Separator
  // Freely inspired by the painting of CC_ComboBox in qcleanlooksstyle.cpp 
  
  QColor buttonColor = this->palette().button().color();
  QColor darkColor;
  darkColor.setHsv(buttonColor.hue(),
                   qMin(255, (int)(buttonColor.saturation()*1.9)),
                   qMin(255, (int)(buttonColor.value()*0.7)));
  painter.setPen(buttonColor.darker(130));
  int borderSize = 2;
  painter.drawLine(QPoint(downArrowRect.left() - 1, downArrowRect.top() + borderSize),
                   QPoint(downArrowRect.left() - 1, downArrowRect.bottom() - borderSize));
  painter.setPen(this->palette().light().color());
  painter.drawLine(QPoint(downArrowRect.left(), downArrowRect.top() + borderSize),
                   QPoint(downArrowRect.left(), downArrowRect.bottom() - borderSize));
  // Draw arrow
  QStyleOption indicatorOpt;
  indicatorOpt.init(this);
  indicatorOpt.rect = downArrowRect.adjusted(borderSize, borderSize, -borderSize, -borderSize);
  painter.drawPrimitive(QStyle::PE_IndicatorArrowDown, indicatorOpt);

  // Draw Icon & Text
  option.rect.setRight( downArrowRect.left());
  painter.drawControl(QStyle::CE_PushButtonLabel, option);
}

//-----------------------------------------------------------------------------
bool qCTKMenuButton::hitButton(const QPoint & _pos)const
{
  QCTK_D(const qCTKMenuButton);
  return !d->indicatorRect().contains(_pos) 
    && this->QPushButton::hitButton(_pos);
}

//-----------------------------------------------------------------------------
void qCTKMenuButton::initStyleOption(QStyleOptionButton* option)const
{
  this->QPushButton::initStyleOption(option);
}

//-----------------------------------------------------------------------------
void qCTKMenuButton::mousePressEvent(QMouseEvent *e)
{
  QCTK_D(qCTKMenuButton);
  this->QPushButton::mousePressEvent(e);
  if (e->isAccepted())
    {
    return;
    }
  if (d->indicatorRect().contains(e->pos()))
    {
    this->showExtraMenu();
    e->accept();
    }
}

//-----------------------------------------------------------------------------
void qCTKMenuButton::showExtraMenu()
{
  QCTK_D(qCTKMenuButton);
  if (!d->ExtraMenu)
    {
    return;
    }

  d->ExtraMenu->setNoReplayFor(this);
  bool horizontal = true;
#if !defined(QT_NO_TOOLBAR)
  QToolBar *tb = qobject_cast<QToolBar*>(this->parent());
  if (tb && tb->orientation() == Qt::Vertical)
    {
    horizontal = false;
    }
#endif
  QWidgetItem item(this);
  QRect _rect = item.geometry();
  _rect.setRect(_rect.x() - this->x(), _rect.y() - this->y(), _rect.width(), _rect.height());

  QSize menuSize = d->ExtraMenu->sizeHint();
  QPoint globalPos = this->mapToGlobal(_rect.topLeft());
  int _x = globalPos.x();
  int _y = globalPos.y();
  if (horizontal) 
    {
    if (globalPos.y() + _rect.height() + menuSize.height() <= QApplication::desktop()->height()) 
      {
      _y += _rect.height();
      } 
    else 
      {
      _y -= menuSize.height();
      }
    if (this->layoutDirection() == Qt::RightToLeft)
      {
      _x += _rect.width() - menuSize.width();
      }
    } 
  else 
    {
    if (globalPos.x() + _rect.width() + d->ExtraMenu->sizeHint().width() <= QApplication::desktop()->width())
      {
      _x += _rect.width();
      }
    else
      {
      _x -= menuSize.width();
      }
    }
  // waits until the menu is executed...
  d->ExtraMenu->exec(QPoint(_x, _y));
}

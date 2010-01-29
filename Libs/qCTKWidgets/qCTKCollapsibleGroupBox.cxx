/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#include "qCTKCollapsibleGroupBox.h"

#include <QDebug>
#include <QChildEvent>
#include <QMouseEvent>
#include <QStylePainter>
#include <QStyleOptionGroupBox>
#include <QStyle>

#if QT_VERSION >= 0x040600
#include <QProxyStyle>

class qCTKCollapsibleGroupBoxStyle:public QProxyStyle
{
  public:

  virtual void drawPrimitive(PrimitiveElement pe, const QStyleOption * opt, QPainter * p, const QWidget * widget = 0) const
  {
    if (pe == QStyle::PE_IndicatorCheckBox)
      {
      const QGroupBox* groupBox= qobject_cast<const QGroupBox*>(widget);
      if (groupBox)
        {
        this->QProxyStyle::drawPrimitive(groupBox->isChecked() ? QStyle::PE_IndicatorArrowUp : QStyle::PE_IndicatorArrowDown, opt, p, widget);
        return;
        }
      }
    this->QProxyStyle::drawPrimitive(pe, opt, p, widget);
  }
};
#else
  
#endif

qCTKCollapsibleGroupBox::qCTKCollapsibleGroupBox(QWidget* _parent)
  :QGroupBox(_parent)
{
  this->setCheckable(true);
  connect(this, SIGNAL(toggled(bool)), this, SLOT(expand(bool)));

  this->MaxHeight = this->maximumHeight();
#if QT_VERSION >= 0x040600
  this->setStyle(new qCTKCollapsibleGroupBoxStyle);
#else
  this->setStyleSheet(
    "qCTKCollapsibleGroupBox::indicator:checked{"
    "image: url(:/Icons/expand-up.png);}"
    "qCTKCollapsibleGroupBox::indicator:unchecked{"
    "image: url(:/Icons/expand-down.png);}");
#endif
}

qCTKCollapsibleGroupBox::~qCTKCollapsibleGroupBox()
{

}

void qCTKCollapsibleGroupBox::expand(bool _expand)
{
  if (!_expand)
    {
    this->OldSize = this->size();
    }

  QObjectList childList = this->children();
  for (int i = 0; i < childList.size(); ++i) 
    {
    QObject *o = childList.at(i);
    if (o && o->isWidgetType()) 
      {
      QWidget *w = static_cast<QWidget *>(o);
      if ( w )
        {
        w->setVisible(_expand);
        }
      }
    }
  
  if (_expand)
    {
    this->setMaximumHeight(this->MaxHeight);
    this->resize(this->OldSize);
    }
  else
    {
    this->MaxHeight = this->maximumHeight();
    this->setMaximumHeight(22);
    }
}

void qCTKCollapsibleGroupBox::childEvent(QChildEvent* c)
{
  if(c && c->type() == QEvent::ChildAdded)
    {
    if (c->child() && c->child()->isWidgetType())
      {
      QWidget *w = static_cast<QWidget*>(c->child());
      w->setVisible(this->isChecked());
      }
    }
  QGroupBox::childEvent(c);
}

#if QT_VERSION < 0x040600
void qCTKCollapsibleGroupBox::paintEvent(QPaintEvent* e)
{
  this->QGroupBox::paintEvent(e);
  
  QStylePainter paint(this);
  QStyleOptionGroupBox option;
  initStyleOption(&option);
  option.activeSubControls &= !QStyle::SC_GroupBoxCheckBox;
  paint.drawComplexControl(QStyle::CC_GroupBox, option);
  
}

void qCTKCollapsibleGroupBox::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }
    // no animation
}

void qCTKCollapsibleGroupBox::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    QStyleOptionGroupBox box;
    initStyleOption(&box);
    box.activeSubControls &= !QStyle::SC_GroupBoxCheckBox;
    QStyle::SubControl released = style()->hitTestComplexControl(QStyle::CC_GroupBox, &box,
                                                                 event->pos(), this);
    bool toggle = this->isCheckable() && (released == QStyle::SC_GroupBoxLabel
                                   || released == QStyle::SC_GroupBoxCheckBox);
    if (toggle)
      {
      this->setChecked(!this->isChecked());
      }
}

#endif

QSize qCTKCollapsibleGroupBox::minimumSizeHint() const
{
  //qDebug() << "qCTKCollapsibleGroupBox::minimumSizeHint::" << this->QGroupBox::minimumSizeHint() ;
  return this->QGroupBox::minimumSizeHint();
}

QSize qCTKCollapsibleGroupBox::sizeHint() const
{
  //qDebug() << "qCTKCollapsibleGroupBox::sizeHint::" << this->QGroupBox::sizeHint() ;
  return this->QGroupBox::sizeHint();
}

int qCTKCollapsibleGroupBox::heightForWidth(int w) const
{
  //qDebug() << "qCTKCollapsibleGroupBox::heightForWidth::" << this->QGroupBox::heightForWidth(w) ;
  return this->QGroupBox::heightForWidth(w);
}

void qCTKCollapsibleGroupBox::resizeEvent ( QResizeEvent * _event )
{
  //qDebug() << "qCTKCollapsibleGroupBox::resizeEvent::" << _event->oldSize() << _event->size() ;
  return this->QGroupBox::resizeEvent(_event);
}

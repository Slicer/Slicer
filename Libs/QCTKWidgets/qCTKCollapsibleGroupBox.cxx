#include "qCTKCollapsibleGroupBox.h"

#include <QDebug>
#include <QChildEvent>
#include <QMouseEvent>
#include <QStylePainter>
#include <QStyleOptionGroupBox>
#include <QStyle>

qCTKCollapsibleGroupBox::qCTKCollapsibleGroupBox(QWidget* parent)
  :QGroupBox(parent)
{
  this->setCheckable(true);
  connect(this, SIGNAL(toggled(bool)), this, SLOT(expand(bool)));

  this->MaxHeight = this->maximumHeight();

  this->setStyleSheet(
    "qCTKCollapsibleGroupBox::indicator:checked{"
    "image: url(:/Icons/expand-up.png);}"
    "qCTKCollapsibleGroupBox::indicator:unchecked{"
    "image: url(:/Icons/expand-down.png);}");
}

qCTKCollapsibleGroupBox::~qCTKCollapsibleGroupBox()
{

}

void qCTKCollapsibleGroupBox::expand(bool expand)
{
  if (!expand)
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
        w->setVisible(expand);
        }
      }
    }
  
  if (expand)
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
      w->setVisible(!this->isChecked());
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

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
  this->setChecked(false);
  connect(this, SIGNAL(toggled(bool)), this, SLOT(collapse(bool)));

  this->Height = this->height();

// #if QT_VERSION < 0x040600
//   // pb when the group box is empty.
//   this->setMinimumHeight(20);
// #endif

  this->setStyleSheet(
    "qCTKCollapsibleGroupBox::indicator:checked{"
    "image: url(:/Icons/expand-down.png);}"
    "qCTKCollapsibleGroupBox::indicator:unchecked{"
    "image: url(:/Icons/expand-up.png);}");
}

qCTKCollapsibleGroupBox::~qCTKCollapsibleGroupBox()
{

}

void qCTKCollapsibleGroupBox::collapse(bool hide)
{
  QObjectList childList = this->children();
  for (int i = 0; i < childList.size(); ++i) 
    {
    QObject *o = childList.at(i);
    if (o && o->isWidgetType()) 
      {
      QWidget *w = static_cast<QWidget *>(o);
      if ( w )
        {
        w->setVisible(!hide);
        }
      }
    }
  
  if (hide)
    {
    this->Height = this->maximumHeight();
    this->setMaximumHeight(22);
    }
  else
    {
    this->setMaximumHeight(this->Height);
    this->resize(this->sizeHint());
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
/*
#include <QTreeWidget>
#include <QVBoxLayout>
#include <iostream>


qCTKCollapsibleGroupBox::qCTKCollapsibleGroupBox(QWidget* parent)
  :QFrame(parent)
{
  QVBoxLayout* verticalLayout = new QVBoxLayout(this);
  this->GroupBoxHeader = new QTreeWidget(this);
  this->GroupBoxHeader->setColumnCount(1);
  this->GroupBoxHeader->setHeaderHidden(true);
  this->GroupBoxHeader->setSelectionMode(QAbstractItemView::NoSelection);
  this->GroupBoxHeader->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->GroupBoxHeader->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  QTreeWidgetItem* item = 0;
  item = new QTreeWidgetItem(item, QStringList(QString("GroupBox")));
  item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
  this->GroupBoxHeader->insertTopLevelItem(0, item);
  verticalLayout->addWidget(this->GroupBoxHeader);
  this->setLayout(verticalLayout);
  this->GroupBoxHeader->setMaximumHeight(
    this->GroupBoxHeader->sizeHintForRow(0) + 
    2 * this->GroupBoxHeader->frameWidth());
  this->GroupBoxHeader->setFrameStyle(QFrame::Panel | QFrame::Raised);
  this->GroupBoxHeader->setBackgroundRole(QPalette::Window);
  this->GroupBoxHeader->setFocusPolicy(Qt::NoFocus);

  verticalLayout->setStretch(0, 0);
  verticalLayout->setStretch(1, 1);
  verticalLayout->setContentsMargins(0, 0, 0, 0);

  this->setFrameShape(QFrame::Box);
  this->setFrameShadow(QFrame::Sunken);

  connect(this->GroupBoxHeader, SIGNAL(itemExpanded(QTreeWidgetItem*)),
          this, SLOT(expandChildren()));
  connect(this->GroupBoxHeader, SIGNAL(itemCollapsed(QTreeWidgetItem*)),
          this, SLOT(collapseChildren()));
}

qCTKCollapsibleGroupBox::~qCTKCollapsibleGroupBox()
{

}

void qCTKCollapsibleGroupBox::setTitle(QString title)
{
  this->GroupBoxHeader->topLevelItem(0)->setText(0, title);
}

QString qCTKCollapsibleGroupBox::title() const
{
  return this->GroupBoxHeader->topLevelItem(0)->text(0);
}

void qCTKCollapsibleGroupBox::expand()
{
  this->GroupBoxHeader->expandItem(
    this->GroupBoxHeader->topLevelItem(0));
}

void qCTKCollapsibleGroupBox::collapse()
{
  this->GroupBoxHeader->collapseItem(
    this->GroupBoxHeader->topLevelItem(0));  
}

void qCTKCollapsibleGroupBox::expandChildren()
{                                             
  int childrenCount = this->layout()->count();
  for (int i = 0; i < childrenCount; ++i)
    {
    QWidget* item = this->layout()->itemAt(i)->widget();
    if (item && item != this->GroupBoxHeader)
      {
      item->hide();
      }
    }
}

void qCTKCollapsibleGroupBox::collapseChildren()
{
  int childrenCount = this->layout()->count();
  for (int i = 0; i < childrenCount; ++i)
    {
    QWidget* item = this->layout()->itemAt(i)->widget();
    if (item && item != this->GroupBoxHeader)
      {
      item->show();
      }
    }
}
*/

#include "qCTKCollapsibleWidget.h"

#include <QStyle>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QResizeEvent>

struct qCTKCollapsibleWidget::qInternal
{
  bool  Collapsed;
  QIcon OnIcon;
  QIcon OffIcon;

  bool  CollapseChildren;
  QSize OldSize;
  int   MaxHeight;
  int   CollapsedHeight;

  QPushButton* Header;
  QStackedWidget *StackWidget;

};

qCTKCollapsibleWidget::qCTKCollapsibleWidget(QWidget* parent)
  :QFrame(parent)
{
  this->Internal = new qCTKCollapsibleWidget::qInternal;
  this->Internal->Collapsed = false;
  this->Internal->Header = new QPushButton("CollapsibleWidget",this);
  this->Internal->StackWidget = new QStackedWidget(this);
  this->Internal->StackWidget->addWidget(new QWidget);
  
  QVBoxLayout* layout = new QVBoxLayout();
  layout->addWidget(this->Internal->Header);
  layout->addWidget(this->Internal->StackWidget);
  this->setLayout(layout);
  
  connect(this->Internal->Header, SIGNAL(clicked()),
          this, SLOT(toggleCollapse()));

  this->Internal->CollapseChildren = true;
  this->Internal->CollapsedHeight = 0;

  this->Internal->MaxHeight = this->maximumHeight();

  // Customization
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  this->Internal->Header->setStyleSheet("text-align: left;");
  this->Internal->Header->setFixedHeight(20);
  
  //QIcon headerIcon;
  //headerIcon.addFile(":/Icons/expand-up.png",QSize(), QIcon::Normal, QIcon::Off);
  //headerIcon.addFile(":/Icons/expand-down.png",QSize(), QIcon::Normal, QIcon::On);
  
  this->Internal->OffIcon.addFile(":/Icons/expand-up.png");
  this->Internal->OnIcon.addFile(":/Icons/expand-down.png");
  this->Internal->Header->setIcon(this->Internal->OffIcon);
  /*
  QIcon arrowIcon = 
    this->style()->standardIcon(QStyle::SP_ArrowUp);
  this->Header->setIcon(arrowIcon);
  */

  // Slicer Custom
  this->Internal->CollapsedHeight = 10;
}

qCTKCollapsibleWidget::~qCTKCollapsibleWidget()
{
  delete this->Internal;
}


QWidget *qCTKCollapsibleWidget::widget()
{
  return this->Internal->StackWidget->widget(0);
}

void qCTKCollapsibleWidget::setWidget(QWidget *newWidget)
{
  // we force to have only 1 page in the QStackedWidget
  // because we don't really want a QStackedWidget. We 
  // use a QStackedWidget because of some QDesigner issues.
  this->Internal->StackWidget->removeWidget(
    this->Internal->StackWidget->widget(0));
  newWidget->setParent(this->Internal->StackWidget);
  this->Internal->StackWidget->addWidget(newWidget);
}

void qCTKCollapsibleWidget::setTitle(QString t)
{
  this->Internal->Header->setText(t);
}

QString qCTKCollapsibleWidget::title()const
{
  return this->Internal->Header->text();
}

void qCTKCollapsibleWidget::setCollapsed(bool c)
{
  this->collapse(c);
}

bool qCTKCollapsibleWidget::collapsed()const
{
  return this->Internal->Collapsed;
}

void qCTKCollapsibleWidget::toggleCollapse()
{
  this->collapse(!this->Internal->Collapsed);
}

void qCTKCollapsibleWidget::setCollapseChildren(bool c)
{
  this->Internal->CollapseChildren = c;
}

bool qCTKCollapsibleWidget::collapseChildren()const
{
  return this->Internal->CollapseChildren;
}

void qCTKCollapsibleWidget::setCollapsedHeight(int h)
{
  this->Internal->CollapsedHeight = h;
}

int qCTKCollapsibleWidget::collapsedHeight()const
{
  return this->Internal->CollapsedHeight;
}

void qCTKCollapsibleWidget::collapse(bool c)
{
  if (c == this->Internal->Collapsed)
    {
    return;
    }
  if (c)
    {
    this->Internal->OldSize = this->size();
    }
  QSize newSize = this->Internal->OldSize;
  if (this->Internal->CollapseChildren)
    {
    this->widget()->setHidden(c);
    }
  else
    {
    this->Internal->StackWidget->setHidden(c);
    }
  if (c)
    {
    if (this->Internal->CollapseChildren)
      {
      int top, bottom, left, right;
      this->Internal->StackWidget->getContentsMargins(&left, &top, &right, &bottom);
      int stackWidgetHeight = top + bottom + this->Internal->CollapsedHeight;
      this->Internal->StackWidget->setMaximumHeight(stackWidgetHeight);
      int top2, bottom2;
      this->getContentsMargins(0, &top2, 0, &bottom2);
      this->Internal->MaxHeight = this->maximumHeight();
      this->setMaximumHeight(top2 + this->Internal->Header->size().height() + 
                             this->layout()->spacing() + stackWidgetHeight + 
                             bottom2);
      }
    else
      {
      newSize.setHeight(this->sizeHint().height());
      this->resize(newSize);
      }
    }
  else
    {
    this->Internal->StackWidget->setMaximumHeight(QWIDGETSIZE_MAX);
    this->setMaximumHeight(this->Internal->MaxHeight);
    this->resize(newSize);
    //qDebug() << "Resize: " << this->Internal->Header->text() << " " << newSize ;
    }
  this->Internal->Collapsed = c;
  this->Internal->Header->setIcon(c ? this->Internal->OnIcon : 
                                  this->Internal->OffIcon);
  emit contentsCollapsed(c);
}

QFrame::Shape qCTKCollapsibleWidget::contentsFrameShape() const
{
  return this->Internal->StackWidget->frameShape();
}

void qCTKCollapsibleWidget::setContentsFrameShape(QFrame::Shape s)
{
  this->Internal->StackWidget->setFrameShape(s);
}

QFrame::Shadow qCTKCollapsibleWidget::contentsFrameShadow() const
{
  return this->Internal->StackWidget->frameShadow();
}

void qCTKCollapsibleWidget::setContentsFrameShadow(QFrame::Shadow s)
{
  this->Internal->StackWidget->setFrameShadow(s);
}
  
int qCTKCollapsibleWidget:: contentsLineWidth() const
{
  return this->Internal->StackWidget->lineWidth();
}

void qCTKCollapsibleWidget::setContentsLineWidth(int w)
{
  this->Internal->StackWidget->setLineWidth(w);
}

int qCTKCollapsibleWidget::contentsMidLineWidth() const
{
  return this->Internal->StackWidget->midLineWidth();
}

void qCTKCollapsibleWidget::setContentsMidLineWidth(int w)
{
  this->Internal->StackWidget->setMidLineWidth(w);
}

QSize qCTKCollapsibleWidget::minimumSizeHint()const
{
  //qDebug() << "qCTKCollapsibleWidget::" << __FUNCTION__ << ":: " 
  //         << this->Internal->Header->text() << " " << this->QWidget::minimumSizeHint();
  return this->QWidget::minimumSizeHint();
}

QSize qCTKCollapsibleWidget::sizeHint()const
{
  //qDebug() << "qCTKCollapsibleWidget::" << __FUNCTION__ << "::"
  //         << this->Internal->Header->text() << " " << this->QWidget::sizeHint();
  return this->QWidget::sizeHint();
}

int qCTKCollapsibleWidget::heightForWidth(int w) const
{
  //qDebug() << "qCTKCollapsibleWidget::heightForWidth::" << this->QWidget::heightForWidth(w) ;
  return this->QWidget::heightForWidth(w);
}

void qCTKCollapsibleWidget::resizeEvent ( QResizeEvent * event )
{
  //qDebug() << "qCTKCollapsibleWidget::resizeEvent::" << event->oldSize() << " " << event->size() ;
  return this->QWidget::resizeEvent(event);
}

#include "qCTKCollapsibleWidget2.h"

#include <QStyle>
#include <QPushButton>
#include <QDebug>
#include <QPainter>
#include <QStyleOptionButton>
#include <QApplication>
#include <QStyleOptionFrameV3>
#include <QMouseEvent>
#include <QLayout>

//-----------------------------------------------------------------------------
struct qCTKCollapsibleWidget2::qInternal
{
  qInternal( QWidget* widget);
  void init();
  void initStyleOption(QStyleOptionButton& opt);

  qCTKCollapsibleWidget2* Widget;

  bool     Collapsed;
  int      CollapsedHeight;
  QString  Title;

  // Contents frame
  QFrame::Shape  ContentsFrameShape;
  QFrame::Shadow ContentsFrameShadow;
  int            ContentsLineWidth;
  int            ContentsMidLineWidth;

  // Is the button pressed
  bool PressedControl;
  int  MaximumHeight;
};

//-----------------------------------------------------------------------------
qCTKCollapsibleWidget2::qInternal::qInternal(QWidget* widget)
{
  this->Widget = qobject_cast<qCTKCollapsibleWidget2*>(widget);
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleWidget2::qInternal::init()
{
  this->Title = "GroupBox";
  this->Collapsed = false;
  this->CollapsedHeight = 10;

  this->ContentsFrameShape = QFrame::NoFrame;
  this->ContentsFrameShadow = QFrame::Plain;
  this->ContentsLineWidth = 1;
  this->ContentsMidLineWidth = 0;

  this->PressedControl = 0;
  this->MaximumHeight = this->Widget->maximumHeight();

  QStyleOptionButton opt;
  this->initStyleOption(opt);
  this->Widget->setContentsMargins(0, opt.rect.height(),0 , 0);
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleWidget2::qInternal::initStyleOption(QStyleOptionButton& opt)
{
  opt.init(this->Widget);
  if (!this->PressedControl)
    {
    opt.state |= QStyle::State_Raised;
    }
  else
    {
    opt.state |= QStyle::State_Sunken;
    }
  opt.state &= ~QStyle::State_HasFocus;
  opt.text = this->Title;
  opt.iconSize = QSize(this->Widget->style()->pixelMetric(QStyle::PM_IndicatorWidth, &opt, this->Widget),
                       this->Widget->style()->pixelMetric(QStyle::PM_IndicatorHeight, &opt, this->Widget));
  int buttonHeight = qMax(this->Widget->fontMetrics().height(), opt.iconSize.height());
  opt.rect.setHeight(buttonHeight);
}

//-----------------------------------------------------------------------------
qCTKCollapsibleWidget2::qCTKCollapsibleWidget2(QWidget* parent)
  :QWidget(parent)
{
  this->Internal = new qCTKCollapsibleWidget2::qInternal(this);
  this->Internal->init();
}

//-----------------------------------------------------------------------------
qCTKCollapsibleWidget2::qCTKCollapsibleWidget2(const QString& title, QWidget* parent)
  :QWidget(parent)
{
  this->Internal = new qCTKCollapsibleWidget2::qInternal(this);
  this->Internal->init();
  this->setTitle(title);
}

//-----------------------------------------------------------------------------
qCTKCollapsibleWidget2::~qCTKCollapsibleWidget2()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleWidget2::setTitle(QString t)
{
  this->Internal->Title = t;
}

//-----------------------------------------------------------------------------
QString qCTKCollapsibleWidget2::title()const
{
  return this->Internal->Title;
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleWidget2::setCollapsed(bool c)
{
  this->collapse(c);
}

//-----------------------------------------------------------------------------
bool qCTKCollapsibleWidget2::collapsed()const
{
  return this->Internal->Collapsed;
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleWidget2::toggleCollapse()
{
  this->collapse(!this->Internal->Collapsed);
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleWidget2::setCollapsedHeight(int h)
{
  this->Internal->CollapsedHeight = h;
}

//-----------------------------------------------------------------------------
int qCTKCollapsibleWidget2::collapsedHeight()const
{
  return this->Internal->CollapsedHeight;
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleWidget2::collapse(bool c)
{
  if (c == this->Internal->Collapsed)
    {
    return;
    }

  this->Internal->Collapsed = c;

  // we do that here as setVisible calls will correctly refresh the widget
  if (c)
    {
    this->Internal->MaximumHeight = this->maximumHeight();

    QStyleOptionButton opt;
    this->Internal->initStyleOption(opt);
    this->setMaximumHeight(this->Internal->CollapsedHeight + opt.rect.height());
    this->updateGeometry();
    }
  else
    {
    // restore maximumheight
    this->setMaximumHeight(this->Internal->MaximumHeight);
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
  this->updateGeometry();
  this->update(QRect(QPoint(0,0), this->sizeHint()));
  this->repaint(QRect(QPoint(0,0), this->sizeHint()));
  emit contentsCollapsed(c);
}

//-----------------------------------------------------------------------------
QFrame::Shape qCTKCollapsibleWidget2::contentsFrameShape() const
{
  return this->Internal->ContentsFrameShape;
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleWidget2::setContentsFrameShape(QFrame::Shape s)
{
  this->Internal->ContentsFrameShape = s;
}

//-----------------------------------------------------------------------------
QFrame::Shadow qCTKCollapsibleWidget2::contentsFrameShadow() const
{
  return this->Internal->ContentsFrameShadow;
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleWidget2::setContentsFrameShadow(QFrame::Shadow s)
{
  this->Internal->ContentsFrameShadow = s;
}

//-----------------------------------------------------------------------------
int qCTKCollapsibleWidget2:: contentsLineWidth() const
{
  return this->Internal->ContentsLineWidth;
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleWidget2::setContentsLineWidth(int w)
{
  this->Internal->ContentsLineWidth = w;
}

//-----------------------------------------------------------------------------
int qCTKCollapsibleWidget2::contentsMidLineWidth() const
{
  return this->Internal->ContentsMidLineWidth;
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleWidget2::setContentsMidLineWidth(int w)
{
  this->Internal->ContentsMidLineWidth = w;
}

//-----------------------------------------------------------------------------
QSize qCTKCollapsibleWidget2::minimumSizeHint()const
{
  // frame
  QSize s = this->QWidget::minimumSizeHint(); 
  if (!this->Internal->Collapsed)
    {
    return s;
    }
    
  int w = 0, h = 0;

  QStyleOptionButton opt;
  this->Internal->initStyleOption(opt);
  
  // indicator
  QSize indicatorSize = QSize(style()->pixelMetric(QStyle::PM_IndicatorWidth, &opt, this),
                              style()->pixelMetric(QStyle::PM_IndicatorHeight, &opt, this));
  int indicatorSpacing = style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing, &opt, this);
  int ih = indicatorSize.height();
  int iw = indicatorSize.width() + indicatorSpacing;
  w += iw;
  h = qMax(h, ih);
  
  // text 
  QString string(this->Internal->Title);
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
  if(!empty || !h)
    {
    h = qMax(h, sz.height());
    }
  opt.rect.setSize(QSize(w, h)); // PM_MenuButtonIndicator depends on the height
  QSize buttonSize = (style()->sizeFromContents(QStyle::CT_PushButton, &opt, QSize(w, h), this).
                      expandedTo(QApplication::globalStrut()));
  return buttonSize + QSize(0,s.height());
}

//-----------------------------------------------------------------------------
QSize qCTKCollapsibleWidget2::sizeHint()const
{
  return this->QWidget::sizeHint();
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleWidget2::paintEvent(QPaintEvent * event)
{
  QPainter p(this);
  // Draw Button
  QStyleOptionButton opt;
  this->Internal->initStyleOption(opt);
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
  if (this->Internal->Collapsed)
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
  f.rect.setTop(buttonHeight);
  f.frameShape = this->Internal->ContentsFrameShape;
  switch (this->Internal->ContentsFrameShadow)
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
  f.lineWidth = this->Internal->ContentsLineWidth;
  f.midLineWidth = this->Internal->ContentsMidLineWidth;
  style()->drawControl(QStyle::CE_ShapedFrame, &f, &p, this);
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleWidget2::mousePressEvent(QMouseEvent* event)
{
  if (event->button() != Qt::LeftButton)
    {
    event->ignore();
    return;
    }

  QStyleOptionButton opt;
  this->Internal->initStyleOption(opt);
  this->Internal->PressedControl = opt.rect.contains(event->pos());
  if (this->Internal->PressedControl)
    {
    this->update(opt.rect);
    }
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleWidget2::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() != Qt::LeftButton)
    {
    event->ignore();
    return;
    }
  if (!this->Internal->PressedControl)
    {
    return;
    }

  this->Internal->PressedControl = false;
  QStyleOptionButton opt;
  this->Internal->initStyleOption(opt);
  if (opt.rect.contains(event->pos()))
    {
    this->toggleCollapse();
    }
  else
    {
    this->update(opt.rect);
    }
}

void qCTKCollapsibleWidget2::childEvent(QChildEvent* c)
{
  if(c && c->type() == QEvent::ChildAdded)
    {
    if (c->child() && c->child()->isWidgetType())
      {
      QWidget *w = static_cast<QWidget*>(c->child());
      w->setVisible(!this->Internal->Collapsed);
      }
    }
  QWidget::childEvent(c);
}

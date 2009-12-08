#include "qCTKCollapsibleButton.h"

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
class qCTKCollapsibleButtonPrivate : public qCTKPrivate<qCTKCollapsibleButton>
{
public:
  QCTK_DECLARE_PUBLIC(qCTKCollapsibleButton);
  void init();

  bool     Collapsed;
  int      CollapsedHeight;

  // Contents frame
  QFrame::Shape  ContentsFrameShape;
  QFrame::Shadow ContentsFrameShadow;
  int            ContentsLineWidth;
  int            ContentsMidLineWidth;

  int      MaximumHeight;
};

//-----------------------------------------------------------------------------
void qCTKCollapsibleButtonPrivate::init()
{
  QCTK_P(qCTKCollapsibleButton);
  this->Collapsed = false;
  this->CollapsedHeight = 10;

  this->ContentsFrameShape = QFrame::NoFrame;
  this->ContentsFrameShadow = QFrame::Plain;
  this->ContentsLineWidth = 1;
  this->ContentsMidLineWidth = 0;

  this->MaximumHeight = p->maximumHeight();

  p->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,
                              QSizePolicy::Preferred, 
                              QSizePolicy::DefaultType));

  QStyleOptionButton opt;
  p->initStyleOption(&opt);
  p->setContentsMargins(0, opt.rect.height(),0 , 0);

  QObject::connect(p, SIGNAL(clicked(bool)),
                   p, SLOT(onClicked(bool)));
                   
  QObject::connect(p, SIGNAL(toggled(bool)),
                   p, SLOT(onClicked(bool)));
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleButton::initStyleOption(QStyleOptionButton* option)const
{
  if (option == 0)
    {
    return;
    }
  option->initFrom(this);
  if (this->isDown())
    {
    option->state |= QStyle::State_Sunken;
    }
  if (this->isChecked())
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
  int buttonHeight = qMax(this->fontMetrics().height(), option->iconSize.height());
  option->rect.setHeight(buttonHeight);
}

//-----------------------------------------------------------------------------
qCTKCollapsibleButton::qCTKCollapsibleButton(QWidget* parent)
  :QAbstractButton(parent)
{
  QCTK_INIT_PRIVATE(qCTKCollapsibleButton);
  qctk_d()->init();
}

//-----------------------------------------------------------------------------
qCTKCollapsibleButton::qCTKCollapsibleButton(const QString& title, QWidget* parent)
  :QAbstractButton(parent)
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
  this->collapse(c);
}

//-----------------------------------------------------------------------------
bool qCTKCollapsibleButton::collapsed()const
{
  return qctk_d()->Collapsed;
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleButton::toggleCollapse()
{
  this->collapse(!qctk_d()->Collapsed);
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
void qCTKCollapsibleButton::onClicked(bool checked)
{
  if (this->isCheckable())
    {
    this->collapse(!checked);
    }
  else
    {
    this->toggleCollapse();
    }
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleButton::collapse(bool c)
{
  QCTK_D(qCTKCollapsibleButton);
  if (c == qctk_d()->Collapsed)
    {
    return;
    }

  d->Collapsed = c;

  // we do that here as setVisible calls will correctly refresh the widget
  if (c)
    {
    d->MaximumHeight = this->maximumHeight();

    QStyleOptionButton opt;
    this->initStyleOption(&opt);
    this->setMaximumHeight(d->CollapsedHeight + opt.rect.height());
    this->updateGeometry();
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
  QWidget* parent = this->parentWidget();
  if (!d->Collapsed && (!parent || !parent->layout()))
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
  this->initStyleOption(&opt);
  
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
  if(!empty || !h)
    {
    h = qMax(h, sz.height());
    }
  opt.rect.setSize(QSize(w, h)); // PM_MenuButtonIndicator depends on the height
  QSize buttonSize = (style()->sizeFromContents(QStyle::CT_PushButton, &opt, QSize(w, h), this).
                      expandedTo(QApplication::globalStrut()));
  return buttonSize;
}

//-----------------------------------------------------------------------------
QSize qCTKCollapsibleButton::minimumSizeHint()const
{
  QSize s = this->QAbstractButton::minimumSizeHint(); 
  QSize buttonSize = this->buttonSizeHint();
  if (qctk_d()->Collapsed)
    {
    return buttonSize.expandedTo(s);
    }
  // open
  if (this->layout() == 0)
    {// no layout, means the button is empty
    return buttonSize;
    }
  return s.expandedTo(buttonSize);
}

//-----------------------------------------------------------------------------
QSize qCTKCollapsibleButton::sizeHint()const
{
  // frame
  QSize s = this->QAbstractButton::sizeHint(); 
  if (!qctk_d()->Collapsed)
    {
    return s;
    }
    
  QSize buttonSize = this->buttonSizeHint();
  return buttonSize + QSize(0,s.height());
}

//-----------------------------------------------------------------------------
void qCTKCollapsibleButton::paintEvent(QPaintEvent * event)
{
  QPainter p(this);
  // Draw Button
  QStyleOptionButton opt;
  this->initStyleOption(&opt);
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
  if (qctk_d()->Collapsed)
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
  f.frameShape = qctk_d()->ContentsFrameShape;
  switch (qctk_d()->ContentsFrameShadow)
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
  f.lineWidth = qctk_d()->ContentsLineWidth;
  f.midLineWidth = qctk_d()->ContentsMidLineWidth;
  style()->drawControl(QStyle::CE_ShapedFrame, &f, &p, this);
}

//-----------------------------------------------------------------------------
bool qCTKCollapsibleButton::hitButton(const QPoint & pos)const
{
  QStyleOptionButton opt;
  this->initStyleOption(&opt);
  return opt.rect.contains(pos);
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

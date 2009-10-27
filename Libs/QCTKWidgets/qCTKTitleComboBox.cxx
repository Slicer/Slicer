#include "qCTKTitleComboBox.h"

#include <QStylePainter>
#include <QApplication>
#include <QDebug>

qCTKTitleComboBox::qCTKTitleComboBox(QWidget* parent)
  : QComboBox(parent)
{
  this->Title = "Select an item...";
}

qCTKTitleComboBox::~qCTKTitleComboBox()
{
  
}

void qCTKTitleComboBox::setTitle(const QString& title)
{
  this->Title = title;
  this->SizeHint = QSize();
  this->updateGeometry();
}

QString qCTKTitleComboBox::title()const
{
  return this->Title;
}
  
void qCTKTitleComboBox::setIcon(const QIcon& icon)
{
  this->Icon = icon;
  this->SizeHint = QSize();
  this->updateGeometry();
}

QIcon qCTKTitleComboBox::icon()const
{
  return this->Icon;
}

void qCTKTitleComboBox::paintEvent(QPaintEvent*)
{
  QStylePainter painter(this);
  painter.setPen(palette().color(QPalette::Text));

  // draw the combobox frame, focusrect and selected etc.
  QStyleOptionComboBox opt;
  initStyleOption(&opt);

  opt.currentText = this->Title;
  opt.currentIcon = this->Icon;
  if (!this->Icon.isNull())
    {
    QList<QSize> sizes = this->Icon.availableSizes();
    // TODO: pick the best size (based on the pixelMetric maybe)
    //int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);
    opt.iconSize = sizes[0];
    }

  painter.drawComplexControl(QStyle::CC_ComboBox, opt);
  // draw the icon and text
  painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

QSize qCTKTitleComboBox::minimumSizeHint() const
{
  return this->recomputeSizeHint(this->SizeHint);
}

/*!
    \reimp

    This implementation caches the size hint to avoid resizing when
    the contents change dynamically. To invalidate the cached value
    change the \l sizeAdjustPolicy.
*/
QSize qCTKTitleComboBox::sizeHint() const
{
  return this->recomputeSizeHint(this->SizeHint);
}

QSize qCTKTitleComboBox::recomputeSizeHint(QSize &sh) const
{
  if (sh.isValid()) 
    {
    return sh.expandedTo(QApplication::globalStrut());
    }
  const QFontMetrics &fm = this->fontMetrics();
  if (!this->Icon.isNull()) 
    {
    QList<QSize> sizes = this->Icon.availableSizes();
    // TODO: pick the best size (based on the pixelMetric maybe)
    QSize iconSize = sizes[0];
    sh.setWidth(qMax(sh.width(), fm.boundingRect(this->Title).width() + iconSize.width() + 4));
    }
  else
    {
    sh.setWidth(qMax(sh.width(), fm.boundingRect(this->Title).width()));
    }

  // height
  sh.setHeight(qMax(fm.lineSpacing(), 14) + 2);
  if (!this->Icon.isNull()) 
    {
    QList<QSize> sizes = this->Icon.availableSizes();
    // TODO: pick the best size (based on the pixelMetric maybe)
    QSize iconSize = sizes[0];
    sh.setHeight(qMax(sh.height(), iconSize.height() + 2));
    }

  // add style and strut values
  QStyleOptionComboBox opt;
  this->initStyleOption(&opt);
  sh = this->style()->sizeFromContents(QStyle::CT_ComboBox, &opt, sh, this);

  return sh.expandedTo(QApplication::globalStrut());
}

#include "qCTKTitleComboBox.h"

#include <QStylePainter>
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
}

QString qCTKTitleComboBox::title()const
{
  return this->Title;
}
  
void qCTKTitleComboBox::setIcon(const QIcon& icon)
{
  this->Icon = icon;
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
  QList<QSize> sizes = this->Icon.availableSizes();
  // TODO: pick the best size (based on the pixelMetric maybe)
  //int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);
  opt.iconSize = sizes[0];

  painter.drawComplexControl(QStyle::CC_ComboBox, opt);
  // draw the icon and text
  painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

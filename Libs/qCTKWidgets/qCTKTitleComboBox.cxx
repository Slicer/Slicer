/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#include "qCTKTitleComboBox.h"

#include <QStylePainter>
#include <QApplication>
#include <QDebug>

// -------------------------------------------------------------------------
qCTKTitleComboBox::qCTKTitleComboBox(QWidget* _parent)
  : QComboBox(_parent)
{
  this->Title = "Select an item...";
}

// -------------------------------------------------------------------------
qCTKTitleComboBox::~qCTKTitleComboBox()
{
  
}

// -------------------------------------------------------------------------
void qCTKTitleComboBox::setTitle(const QString& _title)
{
  this->Title = _title;
  this->SizeHint = QSize();
  this->updateGeometry();
}

// -------------------------------------------------------------------------
QString qCTKTitleComboBox::title()const
{
  return this->Title;
}

// -------------------------------------------------------------------------
void qCTKTitleComboBox::setIcon(const QIcon& _icon)
{
  this->Icon = _icon;
  this->SizeHint = QSize();
  this->updateGeometry();
}

// -------------------------------------------------------------------------
QIcon qCTKTitleComboBox::icon()const
{
  return this->Icon;
}

// -------------------------------------------------------------------------
void qCTKTitleComboBox::paintEvent(QPaintEvent*)
{
  QStylePainter painter(this);
  painter.setPen(palette().color(QPalette::Text));

  QStyleOptionComboBox opt;
  initStyleOption(&opt);

  opt.currentText = this->Title;
  opt.currentIcon = this->Icon;
  if (!this->Icon.isNull() && !this->Icon.availableSizes().empty())
    {
    QList<QSize> sizes = this->Icon.availableSizes();
    // TODO: pick the best size (based on the pixelMetric maybe)
    //int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);
    opt.iconSize = sizes[0];
    }

  // draw the combobox frame, focusrect and selected etc.
  painter.drawComplexControl(QStyle::CC_ComboBox, opt);
  // draw the icon and text
  painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

// -------------------------------------------------------------------------
QSize qCTKTitleComboBox::minimumSizeHint() const
{
  return this->recomputeSizeHint(this->SizeHint);
}

// -------------------------------------------------------------------------
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

// -------------------------------------------------------------------------
QSize qCTKTitleComboBox::recomputeSizeHint(QSize &sh) const
{
  if (sh.isValid()) 
    {
    return sh.expandedTo(QApplication::globalStrut());
    }
  const QFontMetrics &fm = this->fontMetrics();
  QList<QSize> iconSizes = this->Icon.availableSizes();
  if (!this->Icon.isNull() && !iconSizes.empty()) 
    {
    // TODO: pick the best size (based on the pixelMetric maybe)
    QSize _iconSize = iconSizes[0];
    sh.setWidth(qMax(sh.width(), fm.boundingRect(this->Title).width() + _iconSize.width() + 4));
    }
  else
    {
    sh.setWidth(qMax(sh.width(), fm.boundingRect(this->Title).width()));
    }

  // height
  sh.setHeight(qMax(fm.lineSpacing(), 14) + 2);
  if (!this->Icon.isNull() && !iconSizes.empty()) 
    {
    // TODO: pick the best size (based on the pixelMetric maybe)
    QSize _iconSize = iconSizes[0];
    sh.setHeight(qMax(sh.height(), _iconSize.height() + 2));
    }

  // add style and strut values
  QStyleOptionComboBox opt;
  this->initStyleOption(&opt);
  sh = this->style()->sizeFromContents(QStyle::CT_ComboBox, &opt, sh, this);

  return sh.expandedTo(QApplication::globalStrut());
}

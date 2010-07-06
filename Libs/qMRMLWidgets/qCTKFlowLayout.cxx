/****************************************************************************
 **
 ** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 ** All rights reserved.
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL$
 ** No Commercial Usage
 ** This file contains pre-release code and may not be distributed.
 ** You may use this file in accordance with the terms and conditions
 ** contained in the Technology Preview License Agreement accompanying
 ** this package.
 **
 ** GNU Lesser General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU Lesser
 ** General Public License version 2.1 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.LGPL included in the
 ** packaging of this file.  Please review the following information to
 ** ensure the GNU Lesser General Public License version 2.1 requirements
 ** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 **
 ** In addition, as a special exception, Nokia gives you certain additional
 ** rights.  These rights are described in the Nokia Qt LGPL Exception
 ** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
 **
 ** If you have questions regarding the use of this file, please contact
 ** Nokia at qt-info@nokia.com.
 **
 **
 **
 **
 **
 **
 **
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#include "qCTKFlowLayout.h"

#include <QWidget>
#include <QDebug>

//-----------------------------------------------------------------------------
qCTKFlowLayout::qCTKFlowLayout(QWidget *_parent, int _margin, int hSpacing, int vSpacing)
    : QLayout(_parent), HSpace(hSpacing), VSpace(vSpacing)
{
  this->setContentsMargins(_margin, _margin, _margin, _margin);
}

//-----------------------------------------------------------------------------
qCTKFlowLayout::qCTKFlowLayout(int _margin, int hSpacing, int vSpacing)
    : HSpace(hSpacing), VSpace(vSpacing)
{
  this->setContentsMargins(_margin, _margin, _margin, _margin);
}

//-----------------------------------------------------------------------------
qCTKFlowLayout::~qCTKFlowLayout()
{
  QLayoutItem *item;
  while ((item = this->takeAt(0)))
      delete item;
}

//-----------------------------------------------------------------------------
void qCTKFlowLayout::addItem(QLayoutItem *item)
{
  this->ItemList.append(item);
}

//-----------------------------------------------------------------------------
int qCTKFlowLayout::horizontalSpacing() const
{
  if (this->HSpace >= 0)
    {
    return this->HSpace;
    }
  else
    {
    return this->smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
    }
}

//-----------------------------------------------------------------------------
int qCTKFlowLayout::verticalSpacing() const
{
  if (this->VSpace >= 0)
    {
    return this->VSpace;
    }
  else
    {
    return this->smartSpacing(QStyle::PM_LayoutVerticalSpacing);
    }
}

//-----------------------------------------------------------------------------
int qCTKFlowLayout::count() const
{
  return this->ItemList.size();
}

//-----------------------------------------------------------------------------
QLayoutItem *qCTKFlowLayout::itemAt(int index) const
{
  return this->ItemList.value(index);
}

//-----------------------------------------------------------------------------
QLayoutItem *qCTKFlowLayout::takeAt(int index)
{
  if (index >= 0 && index < this->ItemList.size())
    return this->ItemList.takeAt(index);
  else
    return 0;
}

//-----------------------------------------------------------------------------
Qt::Orientations qCTKFlowLayout::expandingDirections() const
{
  // FIXME ?
  return 0;
}

//-----------------------------------------------------------------------------
bool qCTKFlowLayout::hasHeightForWidth() const
{
  return true;
}

//-----------------------------------------------------------------------------
int qCTKFlowLayout::heightForWidth(int width) const
{
  int height = this->doLayout(QRect(0, 0, width, 0), true);
  return height;
}

//-----------------------------------------------------------------------------
void qCTKFlowLayout::setGeometry(const QRect &rect)
{
  this->QLayout::setGeometry(rect);
  this->doLayout(rect, false);
}

//-----------------------------------------------------------------------------
QSize qCTKFlowLayout::sizeHint() const
{
  QSize size = QSize(0,0);
  QLayoutItem *item;
  foreach (item, this->ItemList)
    {
    QSize itemSize = item->sizeHint();
    // FIME: add option to let the user choose what he would prefer:
    // large width + short height or short wight + large height or ...
    size.rwidth() += itemSize.width();
    size.rheight() = qMax(itemSize.height(), size.height());
    }
  size += QSize((this->ItemList.count()-1) * this->horizontalSpacing(), 0);
  int left, top, right, bottom;
  this->getContentsMargins(&left, &top, &right, &bottom);
  size += QSize(left+right, top+bottom);
  return size;
}

//-----------------------------------------------------------------------------
QSize qCTKFlowLayout::minimumSize() const
{
  QSize size;
  QLayoutItem *item;
  foreach (item, this->ItemList)
    {
    size = size.expandedTo(item->minimumSize());
    }
  int left, top, right, bottom;
  this->getContentsMargins(&left, &top, &right, &bottom);
  size += QSize(left+right, top+bottom);
  return size;
}

//-----------------------------------------------------------------------------
int qCTKFlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
  int left, top, right, bottom;
  this->getContentsMargins(&left, &top, &right, &bottom);
  QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
  int x = effectiveRect.x();
  int y = effectiveRect.y();
  int lineHeight = 0;

  QLayoutItem *item;
  foreach (item, this->ItemList)
    {
    QWidget *wid = item->widget();
    int spaceX = this->horizontalSpacing();
    if (spaceX == -1)
        spaceX = wid->style()->layoutSpacing(
            QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
    int spaceY = this->verticalSpacing();
    if (spaceY == -1)
        spaceY = wid->style()->layoutSpacing(
            QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
    int nextX = x + item->sizeHint().width() + spaceX;
    if (nextX - spaceX > (effectiveRect.right() + 1) && lineHeight > 0)
      {
      x = effectiveRect.x();
      y = y + lineHeight + spaceY;
      nextX = x + item->sizeHint().width() + spaceX;
      lineHeight = 0;
      }

    if (!testOnly)
        item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

    x = nextX;
    lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
  return y + lineHeight - rect.y() + bottom;
}

//-----------------------------------------------------------------------------
int qCTKFlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
  QObject *_parent = this->parent();
  if (!_parent)
    {
    return -1;
    }
  else if (_parent->isWidgetType())
    {
    QWidget *pw = static_cast<QWidget *>(_parent);
    return pw->style()->pixelMetric(pm, 0, pw);
    }
  else
    {
    return static_cast<QLayout *>(_parent)->spacing();
    }
}

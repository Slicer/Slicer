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

//-----------------------------------------------------------------------------
qCTKFlowLayout::qCTKFlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
  setContentsMargins(margin, margin, margin, margin);
}

//-----------------------------------------------------------------------------
qCTKFlowLayout::qCTKFlowLayout(int margin, int hSpacing, int vSpacing)
    : m_hSpace(hSpacing), m_vSpace(vSpacing)
{
  setContentsMargins(margin, margin, margin, margin);
}

//-----------------------------------------------------------------------------
qCTKFlowLayout::~qCTKFlowLayout()
{
  QLayoutItem *item;
  while ((item = takeAt(0)))
      delete item;
}

//-----------------------------------------------------------------------------
void qCTKFlowLayout::addItem(QLayoutItem *item)
{
  itemList.append(item);
}

//-----------------------------------------------------------------------------
int qCTKFlowLayout::horizontalSpacing() const
{
  if (m_hSpace >= 0)
    {
    return m_hSpace;
    }
  else
    {
    return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
    }
}

//-----------------------------------------------------------------------------
int qCTKFlowLayout::verticalSpacing() const
{
  if (m_vSpace >= 0)
    {
    return m_vSpace;
    }
  else
    {
    return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
    }
}

//-----------------------------------------------------------------------------
int qCTKFlowLayout::count() const
{
  return itemList.size();
}

//-----------------------------------------------------------------------------
QLayoutItem *qCTKFlowLayout::itemAt(int index) const
{
  return itemList.value(index);
}

//-----------------------------------------------------------------------------
QLayoutItem *qCTKFlowLayout::takeAt(int index)
{
  if (index >= 0 && index < itemList.size())
    return itemList.takeAt(index);
  else
    return 0;
}

//-----------------------------------------------------------------------------
Qt::Orientations qCTKFlowLayout::expandingDirections() const
{
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
  int height = doLayout(QRect(0, 0, width, 0), true);
  return height;
}

//-----------------------------------------------------------------------------
void qCTKFlowLayout::setGeometry(const QRect &rect)
{
  QLayout::setGeometry(rect);
  doLayout(rect, false);
}

//-----------------------------------------------------------------------------
QSize qCTKFlowLayout::sizeHint() const
{
  return minimumSize();
}

//-----------------------------------------------------------------------------
QSize qCTKFlowLayout::minimumSize() const
{
  QSize size;
  QLayoutItem *item;
  foreach (item, itemList)
    size = size.expandedTo(item->minimumSize());

  size += QSize(2*margin(), 2*margin());
  return size;
}

//-----------------------------------------------------------------------------
int qCTKFlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
  int left, top, right, bottom;
  getContentsMargins(&left, &top, &right, &bottom);
  QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
  int x = effectiveRect.x();
  int y = effectiveRect.y();
  int lineHeight = 0;

  QLayoutItem *item;
  foreach (item, itemList)
    {
    QWidget *wid = item->widget();
    int spaceX = horizontalSpacing();
    if (spaceX == -1)
        spaceX = wid->style()->layoutSpacing(
            QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
    int spaceY = verticalSpacing();
    if (spaceY == -1)
        spaceY = wid->style()->layoutSpacing(
            QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
    int nextX = x + item->sizeHint().width() + spaceX;
    if (nextX - spaceX > effectiveRect.right() && lineHeight > 0)
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
  QObject *parent = this->parent();
  if (!parent)
    {
    return -1;
    }
  else if (parent->isWidgetType())
    {
    QWidget *pw = static_cast<QWidget *>(parent);
    return pw->style()->pixelMetric(pm, 0, pw);
    }
  else
    {
    return static_cast<QLayout *>(parent)->spacing();
    }
}

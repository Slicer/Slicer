/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Benjamin Long, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

//QT include
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTextOption>

//include
#include "qMRMLNodeComboBoxDelegate.h"

// --------------------------------------------------------------------------
qMRMLNodeComboBoxDelegate::qMRMLNodeComboBoxDelegate(QObject *parent)
  : QItemDelegate(parent)
{
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBoxDelegate::drawDisplay(QPainter *painter,
                                            const QStyleOptionViewItem &option,
                                            const QRect &rect,
                                            const QString &text) const
{
   QStyleOptionViewItem option2 = option;
   if (option.state & QStyle::State_HasFocus &&
       !(option.state & QStyle::State_Selected))
     {
     option2.state |= QStyle::State_Selected;
     QColor unselectedHighlight = option2.palette.color(QPalette::Highlight);
     unselectedHighlight.setAlpha(110);
     QColor unselectedHighlightedText = option2.palette.color(QPalette::HighlightedText).darker(30);
     option2.palette.setColor(QPalette::Highlight, unselectedHighlight);
     option2.palette.setColor(QPalette::HighlightedText, unselectedHighlightedText);
     }
   this->QItemDelegate::drawDisplay(painter, option2, rect, text);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBoxDelegate::drawFocus(QPainter *painter,
                                          const QStyleOptionViewItem &option,
                                          const QRect &rect) const
{
  Q_UNUSED(painter);
  Q_UNUSED(option);
  Q_UNUSED(rect);
  return;
}

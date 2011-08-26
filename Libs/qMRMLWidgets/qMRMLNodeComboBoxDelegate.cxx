//QT include
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTextOption>

//include
#include "qMRMLNodeComboBoxDelegate.h"

qMRMLNodeComboBoxDelegate::qMRMLNodeComboBoxDelegate(QObject *parent) : QItemDelegate(parent)
{

}
void qMRMLNodeComboBoxDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
                                const QRect &rect, const QString &text) const
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

void qMRMLNodeComboBoxDelegate::drawFocus(QPainter *painter,const QStyleOptionViewItem &option,
                       const QRect &rect) const
{
  Q_UNUSED(painter);
  Q_UNUSED(option);
  Q_UNUSED(rect);
  return;
}

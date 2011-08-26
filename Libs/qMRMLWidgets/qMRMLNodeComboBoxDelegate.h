#ifndef QMRMLNODECOMBOBOXDELEGATE_H
#define QMRMLNODECOMBOBOXDELEGATE_H

//QT include
#include <QItemDelegate>

class qMRMLNodeComboBoxDelegate : public QItemDelegate
{
  Q_OBJECT
public :
  explicit qMRMLNodeComboBoxDelegate(QObject* parent = 0);

 // virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
  virtual void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
                           const QRect &rect, const QString &text) const;
  virtual void drawFocus(QPainter *painter, const QStyleOptionViewItem &option,
                         const QRect &rect) const;
};

#endif // QMRMLNODECOMBOBOXDELEGATE_H

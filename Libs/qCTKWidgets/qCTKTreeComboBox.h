#ifndef __qCTKTreeComboBox_h
#define __qCTKTreeComboBox_h

#include<QComboBox>

// Description:
// ComboBox that displays the items as a tree view.
// See below for a use case:
//    qCTKTreeComboBox combo;
//    QStandardItemModel model;
//    model.appendRow(new QStandardItem("Test1"));
//    model.item(0)->appendRow(new QStandardItem("Test1.1"));
//    model.item(0)->appendRow(new QStandardItem("Test1.2"));
//    model.item(0)->appendRow(new QStandardItem("Test1.3"));
//    model.appendRow(new QStandardItem("Test2"));
//    model.appendRow(new QStandardItem("Test3"));
//    combo.setModel(&model);
//    combo.show();
//
class qCTKTreeComboBox : public QComboBox
{
  Q_OBJECT
public:
  qCTKTreeComboBox(QWidget* parent = 0);
  virtual ~qCTKTreeComboBox();

  virtual bool eventFilter(QObject* object, QEvent* event);
  virtual void showPopup();
  virtual void hidePopup();

protected:
  virtual void paintEvent(QPaintEvent*);
protected slots:
  void onExpanded(const QModelIndex&);
  void onCollapsed(const QModelIndex&);
private:
  struct qInternal;
  qInternal* Internal;
};

#endif

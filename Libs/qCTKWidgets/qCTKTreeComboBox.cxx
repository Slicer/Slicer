#include "qCTKTreeComboBox.h"

#include <QEvent>
#include <QMouseEvent>
#include <QModelIndex>
#include <QTreeView>
#include <QDebug>

struct qCTKTreeComboBox::qInternal
{
  bool SkipNextHide;
  bool ResetPopupSize;
  void init()
  {
    this->SkipNextHide = false;
    this->ResetPopupSize = false;
  }
};

qCTKTreeComboBox::qCTKTreeComboBox(QWidget* parent)
  :QComboBox(parent)
{
  this->Internal = new qCTKTreeComboBox::qInternal;
  this->Internal->init();
  QTreeView* treeView = new QTreeView(this);
  treeView->setHeaderHidden(true);
  this->setView(treeView);
  // we install the filter AFTER the QComboBox installed it.
  // so that our eventFilter will be called first
  this->view()->viewport()->installEventFilter(this);
  connect(treeView, SIGNAL(collapsed(const QModelIndex&)),
          this, SLOT(onCollapsed(const QModelIndex&)));
  connect(treeView, SIGNAL(expanded(const QModelIndex&)),
          this, SLOT(onExpanded(const QModelIndex&)));
}

qCTKTreeComboBox::~qCTKTreeComboBox()
{
  delete this->Internal;
}

bool qCTKTreeComboBox::eventFilter(QObject* object, QEvent* event)
{
  bool res = false;
  if (event->type() == QEvent::MouseButtonRelease && 
      object == this->view()->viewport())
    {
    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event); 
    QModelIndex index = this->view()->indexAt(mouseEvent->pos());
    // do we click the branch (+ or -) or the item itself ?
    if (this->view()->model()->hasChildren(index) && 
        (index.flags() & Qt::ItemIsSelectable) &&
        !this->view()->visualRect(index).contains(mouseEvent->pos()))
      {//qDebug() << "Set skip on";
      // if the branch is clicked, then we don't want to close the 
      // popup. (we don't want to select the item, just expand it.)
      // of course, all that doesn't apply with unselectable items, as
      // they won't close the popup.
      this->Internal->SkipNextHide = true;
      }

    // we want to get rid of an odd behavior. 
    // If the user highlight a selectable item and then 
    // click on the branch of an unselectable item while keeping the 
    // previous selection. The popup would be normally closed in that
    // case. We don't want that.
    if ( this->view()->model()->hasChildren(index) && 
        !(index.flags() & Qt::ItemIsSelectable) &&         
        !this->view()->visualRect(index).contains(mouseEvent->pos()))
      {//qDebug() << "eat";
      // eat the event, don't go to the QComboBox event filters.
      res = true;
      }

    if (this->Internal->ResetPopupSize)
      {
      this->Internal->ResetPopupSize = false;
      //this->QComboBox::showPopup();
      }
    }
  return res;
}
 
void qCTKTreeComboBox::showPopup()
{
  this->setRootModelIndex(QModelIndex());
  this->QComboBox::showPopup();
}
 
void qCTKTreeComboBox::hidePopup()
{
  if (this->Internal->SkipNextHide)
    {// don't hide the popup if the selected item is a parent.
    this->Internal->SkipNextHide = false;
    //this->setCurrentIndex(-1);
    //qDebug() << "skip";
    //this->QComboBox::showPopup();
    }
  else
    {
    QModelIndex currentIndex = this->view()->currentIndex();
    //qDebug() << "qCTKTreeComboBox::hidePopup() " << currentIndex << " " << currentIndex.row();
    //qDebug() << "before: " << this->currentIndex() << this->view()->currentIndex();
    this->QComboBox::hidePopup();
    //qDebug() << "after: " << this->currentIndex() << this->view()->currentIndex();
    this->setRootModelIndex(currentIndex.parent());
    this->setCurrentIndex(currentIndex.row());
    //qDebug() << "after2: " << this->currentIndex() << this->view()->currentIndex();
    }
}
 
void qCTKTreeComboBox::onCollapsed(const QModelIndex& index)
{
  if (this->view()->currentIndex().parent() == index)
    {
    // in the case the current item is a child of the collapsed/expanded item.
    // we don't want to resize the popup as it would undo the collapsed item.
    return;
    }
  this->Internal->ResetPopupSize = true;
}

void qCTKTreeComboBox::onExpanded(const QModelIndex& index)
{
  this->Internal->ResetPopupSize = true;
}

void qCTKTreeComboBox::paintEvent(QPaintEvent *p)
{
  //qDebug() << __FUNCTION__ << " " << this->currentText() << " " << this->currentIndex() ;
  //qDebug() << this->itemText(0) << this->itemText(1);
  this->QComboBox::paintEvent(p);
}

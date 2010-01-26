/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#include "qCTKTreeComboBox.h"

#include <QEvent>
#include <QMouseEvent>
#include <QModelIndex>
#include <QTreeView>
#include <QDebug>

// -------------------------------------------------------------------------
class qCTKTreeComboBoxPrivate: public qCTKPrivate<qCTKTreeComboBox>
{
public:
  bool SkipNextHide;
  bool ResetPopupSize;
  
  void init()
  {
    this->SkipNextHide = false;
    this->ResetPopupSize = false;
  }
};

// -------------------------------------------------------------------------
qCTKTreeComboBox::qCTKTreeComboBox(QWidget* _parent):Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qCTKTreeComboBox);
  QCTK_D(qCTKTreeComboBox);
  
  d->init();
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

// -------------------------------------------------------------------------
bool qCTKTreeComboBox::eventFilter(QObject* object, QEvent* _event)
{
  QCTK_D(qCTKTreeComboBox);
  
  bool res = false;
  if (_event->type() == QEvent::MouseButtonRelease && 
      object == this->view()->viewport())
    {
    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(_event); 
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
      d->SkipNextHide = true;
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

    if (d->ResetPopupSize)
      {
      d->ResetPopupSize = false;
      //this->QComboBox::showPopup();
      }
    }
  return res;
}

// -------------------------------------------------------------------------
void qCTKTreeComboBox::showPopup()
{
  this->setRootModelIndex(QModelIndex());
  this->QComboBox::showPopup();
}

// -------------------------------------------------------------------------
void qCTKTreeComboBox::hidePopup()
{
  QCTK_D(qCTKTreeComboBox);
  
  if (d->SkipNextHide)
    {// don't hide the popup if the selected item is a parent.
    d->SkipNextHide = false;
    //this->setCurrentIndex(-1);
    //qDebug() << "skip";
    //this->QComboBox::showPopup();
    }
  else
    {
    QModelIndex _currentIndex = this->view()->currentIndex();
    //qDebug() << "qCTKTreeComboBox::hidePopup() " << _currentIndex << " " << _currentIndex.row();
    //qDebug() << "before: " << this->currentIndex() << this->view()->currentIndex();
    this->QComboBox::hidePopup();
    //qDebug() << "after: " << this->currentIndex() << this->view()->currentIndex();
    this->setRootModelIndex(_currentIndex.parent());
    this->setCurrentIndex(_currentIndex.row());
    //qDebug() << "after2: " << this->currentIndex() << this->view()->currentIndex();
    }
}

// -------------------------------------------------------------------------
void qCTKTreeComboBox::onCollapsed(const QModelIndex& index)
{
  QCTK_D(qCTKTreeComboBox);
  
  if (this->view()->currentIndex().parent() == index)
    {
    // in the case the current item is a child of the collapsed/expanded item.
    // we don't want to resize the popup as it would undo the collapsed item.
    return;
    }
  d->ResetPopupSize = true;
}

// -------------------------------------------------------------------------
void qCTKTreeComboBox::onExpanded(const QModelIndex& /*index*/)
{
  qctk_d()->ResetPopupSize = true;
}

// -------------------------------------------------------------------------
void qCTKTreeComboBox::paintEvent(QPaintEvent *p)
{
  //qDebug() << __FUNCTION__ << " " << this->currentText() << " " << this->currentIndex() ;
  //qDebug() << this->itemText(0) << this->itemText(1);
  this->QComboBox::paintEvent(p);
}

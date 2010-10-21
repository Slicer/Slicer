/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/


#include "qCTKAddRemoveComboBox.h"
#include "qCTKAddRemoveComboBox_p.h"

#include <QDebug>

// --------------------------------------------------------------------------
// qCTKAddRemoveComboBoxPrivate methods

// --------------------------------------------------------------------------
qCTKAddRemoveComboBoxPrivate::qCTKAddRemoveComboBoxPrivate()
{
  this->EmptyText = "None";

  this->ActionItemsCount = 4;
  
  this->AddText = "Add";
  this->RemoveText = "Remove";
  this->EditText = "Edit";

  this->AddingEmptyItem = false;
  this->AddingActionItems = false;

  this->RemovingEmptyItem = false;
  this->RemovingActionItems = false;

  this->AddEnabled = true;
  this->RemoveEnabled = true;
  this->EditEnabled = true;

  this->HasEmptyItem = false;
  this->ActionItemsIndex = -1;
  this->PushButtonEnabled = true;
  
  this->CurrentItemIndex = 0;
  this->RestoringSelectedItem = false; 
}
    
// --------------------------------------------------------------------------
void qCTKAddRemoveComboBoxPrivate::insertActionItems()
{
  if (this->ActionItemsIndex == -1)
    {
    this->AddingActionItems = true;
    int actionItemIndex = this->ComboBox->count();
    this->ComboBox->insertSeparator(actionItemIndex);
    this->ComboBox->addItem(this->AddText);
    this->ComboBox->addItem(this->RemoveText);
    this->ComboBox->addItem(this->EditText);
    this->ActionItemsIndex = actionItemIndex; 
    this->AddingActionItems = false;
    }
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBoxPrivate::removeActionItems()
{
  if (this->ActionItemsIndex > 0)
    {
    this->RemovingActionItems = true;
    while(this->ComboBox->count() >= this->ActionItemsIndex)
      {
      this->ComboBox->removeItem(this->ComboBox->count());
      }
    this->RemovingActionItems = false;
    this->ActionItemsIndex = -1;
    }
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBoxPrivate::insertEmptyItem()
{
  if (!this->HasEmptyItem )
    {
    this->AddingEmptyItem = true;
    this->ComboBox->insertItem(0, this->EmptyText);
    this->AddingEmptyItem = false;
    this->HasEmptyItem = true;
    }
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBoxPrivate::connectComboBox(QComboBox* comboBox)
{
  QCTK_P(qCTKAddRemoveComboBox);
  QObject::connect(comboBox, SIGNAL(activated(int)),
                this, SLOT(activated(int)));
  QObject::connect(comboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(currentIndexChanged(int)));
  /*
  this->connect(qctk_d()->ComboBox->model(),
  SIGNAL(rowsAboutToBeInserted(const QModelIndex & _parent, int start, int end )),
  SLOT(onRowsAboutToBeInserted(const QModelIndex & _parent, int start, int end )));
  */
  QObject::connect(comboBox->model(),
                SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
                p, SLOT(onRowsAboutToBeRemoved(const QModelIndex & , int , int  )));

  QObject::connect(comboBox->model(),
                SIGNAL(rowsInserted(const QModelIndex &, int, int )),
                p, SLOT(onRowsInserted(const QModelIndex &, int, int)));
  QObject::connect(comboBox->model(),
                SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                p, SLOT(onRowsRemoved(const QModelIndex &, int, int )));
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBoxPrivate::currentIndexChanged(int index)
{
  QCTK_P(qCTKAddRemoveComboBox);
  if (!(this->AddingActionItems ||
        this->AddingEmptyItem ||
        this->RemovingActionItems ||
        this->RemovingEmptyItem))
    {
    Q_ASSERT(index < this->ComboBox->count());

    // If index corresponds to an action item, emit the corresponding signal
    if (!this->PushButtonEnabled && index >= this->ActionItemsIndex)
      {
      // Restore the item previously selected
      this->RestoringSelectedItem = true;
      this->ComboBox->setCurrentIndex(this->CurrentItemIndex);
      this->RestoringSelectedItem = false;
      
      if (index == this->ActionItemsIndex + 1) // Add action
        {
        qDebug() << "onAdd";
        emit p->onAdd();
        }
      else if (index == this->ActionItemsIndex + 2) // Remove action
        {
        qDebug() << "onRemove";
        emit p->onRemove();
        }
      else if (index == this->ActionItemsIndex + 3) // Edit action
        {
        qDebug() << "onEdit";
        emit p->onEdit();
        }
      }
    else
      {
      emit p->currentIndexChanged(index);
      }
    }
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBoxPrivate::activated(int index)
{
  QCTK_P(qCTKAddRemoveComboBox);
  if (!(this->AddingActionItems ||
        this->AddingEmptyItem ||
        this->RemovingActionItems ||
        this->RemovingEmptyItem ||
        this->RestoringSelectedItem))
    {
    Q_ASSERT(index < this->ComboBox->count());
    
    // If index corresponds to an action item, emit the corresponding signal
    if (!this->PushButtonEnabled && index >= this->ActionItemsIndex)
      {
      // Restore the item previously selected
      this->RestoringSelectedItem = true;
      this->ComboBox->setCurrentIndex(this->CurrentItemIndex);
      this->RestoringSelectedItem = false;
      
      if (index == this->ActionItemsIndex + 1) // Add action
        {
        qDebug() << "onAdd";
        emit p->onAdd();
        }
      else if (index == this->ActionItemsIndex + 2) // Remove action
        {
        qDebug() << "onRemove";
        emit p->onRemove();
        }
      else if (index == this->ActionItemsIndex + 3) // Edit action
        {
        qDebug() << "onEdit";
        emit p->onEdit();
        }
      }
    else
      {
      // Update CurrentItemIndex
      this->CurrentItemIndex = index;  
      emit p->activated(index);
      }
    }
}

// --------------------------------------------------------------------------
// qCTKAddRemoveComboBox methods

// --------------------------------------------------------------------------
qCTKAddRemoveComboBox::qCTKAddRemoveComboBox(QWidget* _parent) : Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qCTKAddRemoveComboBox);
  QCTK_D(qCTKAddRemoveComboBox);
  d->setupUi(this);
  
  // connect
  d->connectComboBox(d->ComboBox);
    
  this->connect(d->AddPushButton, SIGNAL(pressed()), SLOT(onAdd()));
  this->connect(d->RemovePushButton, SIGNAL(pressed()), SLOT(onRemove()));
  this->connect(d->EditPushButton, SIGNAL(pressed()), SLOT(onEdit()));

  // Add default 'empty item'
  d->insertEmptyItem();

  // By default, add the combo box actions
  //d->insertActionItems();
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::printAdditionalInfo()
{
  QCTK_D(qCTKAddRemoveComboBox);
  qDebug() << "qCTKAddRemoveComboBox:" << this << endl
           << " EmptyText:" << d->EmptyText << endl
           << " ActionItemsCount:" << d->ActionItemsCount << endl
           << " AddText:" << d->AddText << endl
           << " RemoveText:" << d->RemoveText << endl
           << " EditText:" << d->EditText << endl
           << " AddingEmptyItem:" << d->AddingEmptyItem << endl
           << " AddingActionItems:" << d->AddingActionItems << endl
           << " RemovingEmptyItem:" << d->RemovingEmptyItem << endl
           << " RemovingActionItems:" << d->RemovingActionItems << endl
           << " AddEnabled:" << d->AddEnabled << endl
           << " RemoveEnabled:" << d->RemoveEnabled << endl
           << " EditEnabled:" << d->EditEnabled << endl
           << " HasEmptyItem:" << d->HasEmptyItem << endl
           << " ActionItemsIndex:" << d->ActionItemsIndex << endl
           << " PushButtonEnabled:" << d->PushButtonEnabled;
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setComboBox(QComboBox* comboBox)
{
  QCTK_D(qCTKAddRemoveComboBox);
  if ((comboBox == d->ComboBox) ||
      comboBox->count())
    {
    return;
    }
  
  QLayoutItem* oldComboBoxItem = this->layout()->takeAt(0);
  QComboBox* oldComboBox = qobject_cast<QComboBox*>(oldComboBoxItem->widget());
  comboBox->setSizePolicy(oldComboBox->sizePolicy());
  comboBox->setEnabled(this->comboBoxEnabled());

  delete oldComboBoxItem;

  dynamic_cast<QBoxLayout*>(this->layout())->insertWidget(0, comboBox);
  d->connectComboBox(comboBox);
  d->ComboBox = comboBox;
  delete oldComboBox;

  // Add default 'empty item'
  d->insertEmptyItem();

  if (!d->PushButtonEnabled)
    {
    // Add action items
    d->insertActionItems();
    }
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setEmptyText(const QString& text)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    {
    Q_ASSERT(d->ComboBox->count() == 1);
    this->setItemText(0, text);
    }
  d->EmptyText = text;
}

// --------------------------------------------------------------------------
QCTK_GET_CPP(qCTKAddRemoveComboBox, QString, emptyText, EmptyText);

// --------------------------------------------------------------------------
QCTK_SET_CPP(qCTKAddRemoveComboBox, const QString&, setAddText, AddText);
QCTK_GET_CPP(qCTKAddRemoveComboBox, QString, addText, AddText);

// --------------------------------------------------------------------------
QCTK_SET_CPP(qCTKAddRemoveComboBox, const QString&, setRemoveText, RemoveText);
QCTK_GET_CPP(qCTKAddRemoveComboBox, QString, removeText, RemoveText);

// --------------------------------------------------------------------------
QCTK_SET_CPP(qCTKAddRemoveComboBox, const QString&, setEditText, EditText);
QCTK_GET_CPP(qCTKAddRemoveComboBox, QString, editText, EditText);

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onRowsInserted(const QModelIndex & _parent, int start, int end)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (_parent != d->ComboBox->rootModelIndex())
    {// Rows that are to be added in the model are not displayed by the combobox
    return;
    }
    
  if (d->HasEmptyItem && !d->AddingEmptyItem && !d->AddingActionItems)
    {
    // Remove the Empty item as some real items have been added
    d->HasEmptyItem = false;
    d->RemovingEmptyItem = true;
    d->ComboBox->removeItem(start == 0 ? end + 1 : 0);
    d->RemovingEmptyItem = false;
    
    if (d->RemoveEnabled)
      {
      d->RemovePushButton->setEnabled(true);
      }
    if (d->EditEnabled)
      {
      d->EditPushButton->setEnabled(true);
      }
    // Since we just removed the empty item, we need to shift the start/end items if needed
    if (start > 0 )
      {
      --start;
      --end;
      }

    // Decrement ActionItemsIndex
    d->ActionItemsIndex--;
    }

  // If pushButtons are disabled and if an item has been added,
  // let's update the increment ActionItemsIndex
  if (!d->PushButtonEnabled && !d->AddingEmptyItem && !d->AddingActionItems)
    {
    Q_ASSERT(d->ActionItemsIndex != -1);
    d->ActionItemsIndex++;
    }

  // Emit signal only if the items added are *NOT* the Empty item or the action items
  if (!(d->AddingEmptyItem || d->AddingActionItems))
    {
    for (int i = start; i <= end; ++i)
      {
      emit this->itemAdded(i);
      }
    }
 }

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onRowsAboutToBeRemoved(const QModelIndex & _parent, int start, int end)
{
  QCTK_D(qCTKAddRemoveComboBox);

  if (_parent != d->ComboBox->rootModelIndex())
    {//rows that are to be added in the model are not displayed by the combobox
    return;
    }

  // if the user try to remove the Empty item or the action items, don't send event
  if (d->RemovingEmptyItem || d->RemovingActionItems)
    {
    return;
    }
  for (int i = start; i <= end; ++i)
    {
    emit this->itemAboutToBeRemoved(i);
    }
}


// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onRowsRemoved(const QModelIndex & _parent, int start, int end)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (_parent != d->ComboBox->rootModelIndex())
    {//rows that are to be added in the model are not displayed by the combobox
    return;
    }
  
  // the combobox is now empty, add the EmptyItem and action items if needed
  if (d->ComboBox->count() == 0)
    {
    // Add default 'empty item'
    d->insertEmptyItem();

    // Add action items
    if (!d->PushButtonEnabled)
      {
      d->ActionItemsIndex = -1;
      d->insertActionItems();
      }
   
    if (d->RemoveEnabled)
      {
      d->RemovePushButton->setEnabled(false);
      }
    if (d->EditEnabled)
      {
      d->EditPushButton->setEnabled(false);
      }
    }
  else
    {
    // If pushButtons are disabled, 
    // let's update the current ActionItemsIndex, since an item has been added, let's decrement it
    if (!d->PushButtonEnabled && !d->RemovingEmptyItem && !d->RemovingActionItems)
      {
      Q_ASSERT(d->ActionItemsIndex != -1);
      d->ActionItemsIndex--;
      }
    }

  if (!(d->RemovingEmptyItem || d->RemovingActionItems))
    {
    for (int i = start; i <= end; ++i)
      {
      emit this->itemRemoved(i);
      }
    }
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setComboBoxEnabled(bool enable)
{
  qctk_d()->ComboBox->setEnabled(enable);
}

// --------------------------------------------------------------------------
bool qCTKAddRemoveComboBox::comboBoxEnabled()const
{
  //const cast as I'm not sure why isEnabledTo doesn't take a const
  return qctk_d()->ComboBox->isEnabledTo(const_cast<qCTKAddRemoveComboBox*>(this));
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setAddEnabled(bool enable)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  d->AddPushButton->setEnabled(enable);
  d->AddEnabled = enable;
}

// --------------------------------------------------------------------------
bool qCTKAddRemoveComboBox::addEnabled()const
{
  return qctk_d()->AddEnabled;
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setRemoveEnabled(bool enable)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (this->count() > 0)
    {
    d->RemovePushButton->setEnabled(enable);
    }
  d->RemoveEnabled = enable;
}

// --------------------------------------------------------------------------
bool qCTKAddRemoveComboBox::removeEnabled()const
{
  return qctk_d()->RemoveEnabled;
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setEditEnabled(bool enable)
{
  QCTK_D(qCTKAddRemoveComboBox);

  bool shouldSetEnabled = false;
  if (d->ActionItemsIndex == -1)
    {
    shouldSetEnabled = (this->count() > 0); 
    }
  else
    {
    shouldSetEnabled  = ((this->count() - d->ActionItemsCount) > 0);
    }
  if (shouldSetEnabled)
    { 
    d->EditPushButton->setEnabled(enable);
    }
  d->EditEnabled = enable;
}

// --------------------------------------------------------------------------
bool qCTKAddRemoveComboBox::editEnabled()const
{
  return qctk_d()->EditEnabled;
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setPushButtonsEnabled(bool enabled)
{
  QCTK_D(qCTKAddRemoveComboBox);

  if (d->PushButtonEnabled == enabled)
    {
    return;
    }

  // Update buttons state
  QList<QWidget*> buttons;
  buttons << d->AddPushButton << d->RemovePushButton << d->EditPushButton;

  foreach(QWidget* w, buttons)
    {
    w->setDisabled(enabled);
    w->setVisible(enabled);
    }

  if (!enabled)
    {
    d->insertActionItems();
    }
  else
    {
    d->removeActionItems();
    }

  d->PushButtonEnabled = enabled;
}

// --------------------------------------------------------------------------
QCTK_GET_CPP(qCTKAddRemoveComboBox, bool, pushButtonsEnabled, PushButtonEnabled)

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onAdd()
{
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onRemove()
{
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onEdit()
{
}

// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::count()const
{
  QCTK_D(const qCTKAddRemoveComboBox);
  int itemCount = d->ComboBox->count();
  if (d->ActionItemsIndex > 0)
    {
    itemCount = itemCount - d->ActionItemsCount;
    }
  return (d->HasEmptyItem ? 0 : itemCount);
}

// --------------------------------------------------------------------------
QCTK_GET_CPP(qCTKAddRemoveComboBox, bool, empty, HasEmptyItem);

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setCurrentIndex(int index)
{
  QCTK_D(qCTKAddRemoveComboBox);
  Q_ASSERT(d->ActionItemsIndex > 0 ? index < d->ActionItemsIndex : true);
  return d->ComboBox->setCurrentIndex(index);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::insertItem(int index, const QString &text, const QVariant &userDataVariable)
{
  QCTK_D(qCTKAddRemoveComboBox);
  Q_ASSERT(d->ActionItemsIndex > 0 ? index <= d->ActionItemsIndex : true);
  //qDebug() << __FUNCTION__ << " " << index <<  " " << text << " " << userDataVariable ;
  d->ComboBox->insertItem(index, text, userDataVariable);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::insertItem(int index, const QIcon &icon, const QString &text, const QVariant &userDataVariable)
{
  QCTK_D(qCTKAddRemoveComboBox);
  Q_ASSERT(d->ActionItemsIndex > 0 ? index <= d->ActionItemsIndex : true);
  d->ComboBox->insertItem(index, icon, text, userDataVariable);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::insertItems(int index, const QStringList &texts)
{
  QCTK_D(qCTKAddRemoveComboBox);
  Q_ASSERT(d->ActionItemsIndex > 0 ? index <= d->ActionItemsIndex : true);
  d->ComboBox->insertItems(index, texts);
}
  
// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::findText(const QString & text, Qt::MatchFlags flags)const
{
  QCTK_D(const qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    { // if the scene is empty, don't even try to find the text (it could be the
      // one of the EmptyText prop.
    return -1;
    }
  int index = d->ComboBox->findText(text, flags);
  if (d->ActionItemsIndex > 0 && index >= d->ActionItemsIndex)
    {
    index = -1;
    }
  return index;
}

// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::findData(const QVariant &dataVariable, int role, Qt::MatchFlags flags)const
{
  QCTK_D(const qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    { // if the scene is empty, don't even try to find the dataVariable
    return -1;
    }
  int index = d->ComboBox->findData(dataVariable, role, flags);
  if (d->ActionItemsIndex > 0 && index >= d->ActionItemsIndex)
    {
    index = -1;
    }
  return index;
}

// --------------------------------------------------------------------------
QString qCTKAddRemoveComboBox::itemText(int index) const
{
  QCTK_D(const qCTKAddRemoveComboBox);

  // For now, let's return the real text.
  
  //Q_ASSERT(d->ActionItemsIndex > 0 ? index < d->ActionItemsIndex : true);
  
  //if (d->HasEmptyItem)
  //  {
  //  return QString();
  //  }
  return d->ComboBox->itemText(index);
}
  
// --------------------------------------------------------------------------
QVariant qCTKAddRemoveComboBox::itemData(int index, int role) const
{
  QCTK_D(const qCTKAddRemoveComboBox);
  Q_ASSERT(d->ActionItemsIndex > 0 ? index < d->ActionItemsIndex : true);
  
  if (d->HasEmptyItem)
    {
    return QVariant();
    }
  return d->ComboBox->itemData(index,role);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setItemText(int index, const QString& text)
{
  QCTK_D(qCTKAddRemoveComboBox);
  Q_ASSERT(d->ActionItemsIndex > 0 ? index < d->ActionItemsIndex : true);
  
  if (d->HasEmptyItem)
    {
    return;
    }
  return d->ComboBox->setItemText(index, text);
}
  
// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setItemData(int index, const QVariant& dataVariable, int role)
{
  QCTK_D(qCTKAddRemoveComboBox);
  Q_ASSERT(d->ActionItemsIndex > 0 ? index < d->ActionItemsIndex : true);
  
  if (d->HasEmptyItem)
    {
    return;
    }
  d->ComboBox->setItemData(index, dataVariable, role);
}

// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::currentIndex() const
{
  QCTK_D(const qCTKAddRemoveComboBox);
  
  return d->HasEmptyItem ? -1 : d->ComboBox->currentIndex();
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::removeItem(int index)
{
  QCTK_D(qCTKAddRemoveComboBox);
  Q_ASSERT(d->ActionItemsIndex > 0 ? index < d->ActionItemsIndex : true);
  
  if (d->HasEmptyItem)
    {
    return; 
    }
  d->ComboBox->removeItem(index);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::clear()
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    {
    return;
    }
  d->ComboBox->clear();
}

// --------------------------------------------------------------------------
QModelIndex qCTKAddRemoveComboBox::rootModelIndex()const
{
  return qctk_d()->ComboBox->rootModelIndex();
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setRootModelIndex(const QModelIndex& root)
{
  qctk_d()->ComboBox->setRootModelIndex(root);
}

// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::modelColumn()const
{
  return qctk_d()->ComboBox->modelColumn();
}

// --------------------------------------------------------------------------
QAbstractItemModel* qCTKAddRemoveComboBox::model()const
{
  return qctk_d()->ComboBox->model();
}

